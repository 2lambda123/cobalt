// Copyright 2008, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//    * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//    * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "net/base/upload_data_stream.h"

namespace net {

UploadDataStream::UploadDataStream(const UploadData* data)
    : data_(data),
      next_element_handle_(INVALID_HANDLE_VALUE),
      total_size_(data->GetContentLength()) {
  Reset();
  FillBuf();
}

UploadDataStream::~UploadDataStream() {
  if (next_element_handle_ != INVALID_HANDLE_VALUE)
    CloseHandle(next_element_handle_);
}

void UploadDataStream::DidConsume(size_t num_bytes) {
  DCHECK(num_bytes <= buf_len_);

  buf_len_ -= num_bytes;
  if (buf_len_)
    memmove(buf_, buf_ + num_bytes, buf_len_);

  FillBuf();

  current_position_ += num_bytes;
}

void UploadDataStream::Reset() {
  if (next_element_handle_ != INVALID_HANDLE_VALUE) {
    CloseHandle(next_element_handle_);
    next_element_handle_ = INVALID_HANDLE_VALUE;
  }
  buf_len_ = 0;
  next_element_ = data_->elements().begin();
  next_element_offset_ = 0;
  next_element_remaining_ = 0;
  current_position_ = 0;
}

void UploadDataStream::FillBuf() {
  std::vector<UploadData::Element>::const_iterator end =
      data_->elements().end();

  while (buf_len_ < kBufSize && next_element_ != end) {
    bool advance_to_next_element = false;

    size_t size_remaining = kBufSize - buf_len_;
    if ((*next_element_).type() == UploadData::TYPE_BYTES) {
      const std::vector<char>& d = (*next_element_).bytes();
      size_t count = d.size() - next_element_offset_;

      size_t bytes_copied = std::min(count, size_remaining);

      memcpy(buf_ + buf_len_, &d[next_element_offset_], bytes_copied);
      buf_len_ += bytes_copied;

      if (bytes_copied == count) {
        advance_to_next_element = true;
      } else {
        next_element_offset_ += bytes_copied;
      }
    } else {
      DCHECK((*next_element_).type() == UploadData::TYPE_FILE);

      if (next_element_handle_ == INVALID_HANDLE_VALUE) {
        next_element_handle_ = CreateFile((*next_element_).file_path().c_str(),
                                          GENERIC_READ,
                                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                                          NULL, OPEN_EXISTING,
                                          FILE_ATTRIBUTE_NORMAL, NULL);
        // If the file does not exist, that's technically okay.. we'll just
        // upload an empty file.  This is for consistency with Mozilla.
        DLOG_IF(WARNING, next_element_handle_ == INVALID_HANDLE_VALUE) <<
            "Unable to open file \"" << (*next_element_).file_path() <<
            "\" for reading: " << GetLastError();

        next_element_remaining_ = (*next_element_).file_range_length();

        if ((*next_element_).file_range_offset()) {
          LARGE_INTEGER offset;
          offset.QuadPart = (*next_element_).file_range_offset();
          if (!SetFilePointerEx(next_element_handle_, offset,
                                NULL, FILE_BEGIN)) {
            DLOG(WARNING) <<
                "Unable to set file position for file \"" <<
                (*next_element_).file_path() << "\": " << GetLastError();
            next_element_remaining_ = 0;
          }
        }
      }

      // ReadFile will happily fail if given an invalid handle.
      BOOL ok = FALSE;
      DWORD bytes_read = 0;
      uint64 amount_to_read = std::min(static_cast<uint64>(size_remaining),
                                       next_element_remaining_);
      if ((amount_to_read > 0) &&
          (ok = ReadFile(next_element_handle_, buf_ + buf_len_,
                         static_cast<DWORD>(amount_to_read), &bytes_read,
                         NULL))) {
        buf_len_ += bytes_read;
        next_element_remaining_ -= bytes_read;
      }

      if (!ok || bytes_read == 0)
        advance_to_next_element = true;
    }

    if (advance_to_next_element) {
      ++next_element_;
      next_element_offset_ = 0;
      if (next_element_handle_ != INVALID_HANDLE_VALUE) {
        CloseHandle(next_element_handle_);
        next_element_handle_ = INVALID_HANDLE_VALUE;
      }
    }
  }
}

}  // namespace net
