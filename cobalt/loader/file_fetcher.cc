/*
 * Copyright 2015 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cobalt/loader/file_fetcher.h"

#include "base/bind.h"
#include "base/file_util.h"
#include "base/file_util_proxy.h"
#include "base/path_service.h"

namespace cobalt {
namespace loader {

FileFetcher::FileFetcher(const FilePath& file_path, const Options& options,
                         Handler* handler)
    : Fetcher(handler),
      buffer_size_(options.buffer_size),
      file_(base::kInvalidPlatformFileValue),
      file_offset_(0),
      io_message_loop_(options.io_message_loop),
      weak_ptr_factory_(this) {
  DCHECK_GT(buffer_size_, 0);

  // Get the actual file path by prepending DIR_SOURCE_ROOT to the file path.
  FilePath actual_file_path;
  PathService::Get(base::DIR_SOURCE_ROOT, &actual_file_path);
  actual_file_path = actual_file_path.Append(file_path);

  // Trigger fetching in the given message loop.
  base::FileUtilProxy::CreateOrOpen(
      io_message_loop_, actual_file_path,
      base::PLATFORM_FILE_OPEN | base::PLATFORM_FILE_READ,
      base::Bind(&FileFetcher::DidCreateOrOpen,
                 weak_ptr_factory_.GetWeakPtr()));
}

FileFetcher::~FileFetcher() {
  DCHECK(thread_checker_.CalledOnValidThread());
  CloseFile();
}

void FileFetcher::ReadNextChunk() {
  base::FileUtilProxy::Read(
      io_message_loop_, file_, file_offset_, buffer_size_,
      base::Bind(&FileFetcher::DidRead, weak_ptr_factory_.GetWeakPtr()));
}

void FileFetcher::CloseFile() {
  if (file_ != base::kInvalidPlatformFileValue) {
    base::ClosePlatformFile(file_);
    file_ = base::kInvalidPlatformFileValue;
  }
}

const std::string& FileFetcher::PlatformFileErrorToString(
    base::PlatformFileError error) {
  static const std::string kPlatformFileOk = "PLATFORM_FILE_OK";
  static const std::string kPlatformFileErrorNotFound =
      "PLATFORM_FILE_ERROR_NOT_FOUND";
  static const std::string kPlatformFileErrorInUse =
      "PLATFORM_FILE_ERROR_IN_USE";
  static const std::string kPlatformFileErrorAccessDenied =
      "PLATFORM_FILE_ERROR_ACCESS_DENIED";
  static const std::string kPlatformFileErrorSecurity =
      "PLATFORM_FILE_ERROR_SECURITY";
  static const std::string kPlatformFileErrorInvalidUrl =
      "PLATFORM_FILE_ERROR_INVALID_URL";
  static const std::string kPlatformFileErrorAbort =
      "PLATFORM_FILE_ERROR_ABORT";
  static const std::string kPlatformFileErrorNotDefined =
      "PlatformError not defined";

  switch (error) {
    case base::PLATFORM_FILE_OK:
      return kPlatformFileOk;
    case base::PLATFORM_FILE_ERROR_NOT_FOUND:
      return kPlatformFileErrorNotFound;
    case base::PLATFORM_FILE_ERROR_IN_USE:
      return kPlatformFileErrorInUse;
    case base::PLATFORM_FILE_ERROR_ACCESS_DENIED:
      return kPlatformFileErrorAccessDenied;
    case base::PLATFORM_FILE_ERROR_SECURITY:
      return kPlatformFileErrorSecurity;
    case base::PLATFORM_FILE_ERROR_INVALID_URL:
      return kPlatformFileErrorInvalidUrl;
    case base::PLATFORM_FILE_ERROR_ABORT:
      return kPlatformFileErrorAbort;
    default:
      break;
  }
  return kPlatformFileErrorNotDefined;
}

void FileFetcher::DidCreateOrOpen(base::PlatformFileError error,
                                  base::PassPlatformFile file,
                                  bool /*created*/) {
  DCHECK(thread_checker_.CalledOnValidThread());
  if (error != base::PLATFORM_FILE_OK) {
    handler()->OnError(PlatformFileErrorToString(error));
    return;
  }

  file_ = file.ReleaseValue();
  file_offset_ = 0;
  ReadNextChunk();
}

void FileFetcher::DidRead(base::PlatformFileError error, const char* data,
                          int num_bytes_read) {
  DCHECK(thread_checker_.CalledOnValidThread());

  if (error != base::PLATFORM_FILE_OK) {
    handler()->OnError(PlatformFileErrorToString(error));
    return;
  }

  if (!num_bytes_read) {
    handler()->OnDone();
    return;
  }

  handler()->OnReceived(data, num_bytes_read);

  file_offset_ += num_bytes_read;
  ReadNextChunk();
}

}  // namespace loader
}  // namespace cobalt
