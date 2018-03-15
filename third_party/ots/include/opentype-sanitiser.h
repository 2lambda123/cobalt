// Copyright (c) 2009-2017 The OTS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OPENTYPE_SANITISER_H_
#define OPENTYPE_SANITISER_H_

#if !defined(STARBOARD)
#include <cstring>
#define MEMCPY_OPENTYPE_SANITISER std::memcpy
#else
#include "starboard/memory.h"
#define MEMCPY_OPENTYPE_SANITISER SbMemoryCopy
#endif

#if defined(STARBOARD)
#include "starboard/byte_swap.h"
#define NTOHL_OPENTYPE_SANITISER(x) SB_NET_TO_HOST_U32(x)
#define NTOHS_OPENTYPE_SANITISER(x) SB_NET_TO_HOST_U16(x)
#define HTONL_OPENTYPE_SANITISER(x) SB_HOST_TO_NET_U32(x)
#define HTONS_OPENTYPE_SANITISER(x) SB_HOST_TO_NET_U16(x)
#elif defined(_WIN32)
#include <stdlib.h>
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#define NTOHL_OPENTYPE_SANITISER(x) _byteswap_ulong (x)
#define NTOHS_OPENTYPE_SANITISER(x) _byteswap_ushort (x)
#define HTONL_OPENTYPE_SANITISER(x) _byteswap_ulong (x)
#define HTONS_OPENTYPE_SANITISER(x) _byteswap_ushort (x)
#else
#include <arpa/inet.h>
#include <stdint.h>
#define NTOHL_OPENTYPE_SANITISER(x) ntohl (x)
#define NTOHS_OPENTYPE_SANITISER(x) ntohs (x)
#define HTONL_OPENTYPE_SANITISER(x) htonl (x)
#define HTONS_OPENTYPE_SANITISER(x) htons (x)
#endif

#include <sys/types.h>

#include <algorithm>
#include <cassert>
#include <cstddef>

#define OTS_TAG(c1,c2,c3,c4) ((uint32_t)((((uint8_t)(c1))<<24)|(((uint8_t)(c2))<<16)|(((uint8_t)(c3))<<8)|((uint8_t)(c4))))
#define OTS_UNTAG(tag)       ((char)((tag)>>24)), ((char)((tag)>>16)), ((char)((tag)>>8)), ((char)(tag))

namespace ots {

// -----------------------------------------------------------------------------
// This is an interface for an abstract stream class which is used for writing
// the serialised results out.
// -----------------------------------------------------------------------------
class OTSStream {
 public:
  OTSStream() : chksum_(0) {}

  virtual ~OTSStream() {}

  // This should be implemented to perform the actual write.
  virtual bool WriteRaw(const void *data, size_t length) = 0;

  bool Write(const void *data, size_t length) {
    if (!length) return false;

    const size_t orig_length = length;
    size_t offset = 0;

    size_t chksum_offset = Tell() & 3;
    if (chksum_offset) {
      const size_t l = std::min(length, static_cast<size_t>(4) - chksum_offset);
      uint32_t tmp = 0;
      MEMCPY_OPENTYPE_SANITISER(reinterpret_cast<uint8_t *>(&tmp) + chksum_offset, data, l);
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4267)  // possible loss of data
#endif
      chksum_ += NTOHL_OPENTYPE_SANITISER(tmp);
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
      length -= l;
      offset += l;
    }

    while (length >= 4) {
      uint32_t tmp;
      MEMCPY_OPENTYPE_SANITISER(&tmp, reinterpret_cast<const uint8_t *>(data) + offset,
        sizeof(uint32_t));
      chksum_ += NTOHL_OPENTYPE_SANITISER(tmp);
      length -= 4;
      offset += 4;
    }

    if (length) {
      if (length > 4) return false;  // not reached
      uint32_t tmp = 0;
      MEMCPY_OPENTYPE_SANITISER(&tmp,
                  reinterpret_cast<const uint8_t*>(data) + offset, length);
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4267)  // possible loss of data
#elif defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#endif
      chksum_ += NTOHL_OPENTYPE_SANITISER(tmp);
#if defined(_MSC_VER)
#pragma warning(pop)
#elif defined(__clang__)
#pragma clang diagnostic pop
#endif
    }

    return WriteRaw(data, orig_length);
  }

  virtual bool Seek(off_t position) = 0;
  virtual off_t Tell() const = 0;

  virtual bool Pad(size_t bytes) {
    static const uint32_t kZero = 0;
    while (bytes >= 4) {
      if (!Write(&kZero, 4)) return false;
      bytes -= 4;
    }
    while (bytes) {
      static const uint8_t kZerob = 0;
      if (!Write(&kZerob, 1)) return false;
      bytes--;
    }
    return true;
  }

  bool WriteU8(uint8_t v) {
    return Write(&v, sizeof(v));
  }

  bool WriteU16(uint16_t v) {
    v = HTONS_OPENTYPE_SANITISER(v);
    return Write(&v, sizeof(v));
  }

  bool WriteS16(int16_t v) {
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4365)  // signed/unsigned mismatch
#endif
    v = HTONS_OPENTYPE_SANITISER(v);
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
    return Write(&v, sizeof(v));
  }

  bool WriteU24(uint32_t v) {
    v = HTONL_OPENTYPE_SANITISER(v);
    return Write(reinterpret_cast<uint8_t*>(&v)+1, 3);
  }

  bool WriteU32(uint32_t v) {
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4365)  // signed/unsigned mismatch
#endif
    v = HTONL_OPENTYPE_SANITISER(v);
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
    return Write(&v, sizeof(v));
  }

  bool WriteS32(int32_t v) {
    v = HTONL_OPENTYPE_SANITISER(v);
    return Write(&v, sizeof(v));
  }

  bool WriteR64(uint64_t v) {
    return Write(&v, sizeof(v));
  }

  void ResetChecksum() {
    assert((Tell() & 3) == 0);
    chksum_ = 0;
  }

  uint32_t chksum() const {
    return chksum_;
  }

 protected:
  uint32_t chksum_;
};

#ifdef __GCC__
#define MSGFUNC_FMT_ATTR __attribute__((format(printf, 2, 3)))
#else
#define MSGFUNC_FMT_ATTR
#endif

enum TableAction {
  TABLE_ACTION_DEFAULT,  // Use OTS's default action for that table
  TABLE_ACTION_SANITIZE, // Sanitize the table, potentially dropping it
  TABLE_ACTION_PASSTHRU, // Serialize the table unchanged
  TABLE_ACTION_DROP      // Drop the table
};

class OTSContext {
  public:
    OTSContext() {}
    virtual ~OTSContext() {}

    // Process a given OpenType file and write out a sanitized version
    //   output: a pointer to an object implementing the OTSStream interface. The
    //     sanitisied output will be written to this. In the even of a failure,
    //     partial output may have been written.
    //   input: the OpenType file
    //   length: the size, in bytes, of |input|
    //   index: if the input is a font collection and index is specified, then
    //     the corresponding font will be returned, otherwise the whole
    //     collection. Ignored for non-collection fonts.
    bool Process(OTSStream *output, const uint8_t *input, size_t length, uint32_t index = -1);

    // This function will be called when OTS is reporting an error.
    //   level: the severity of the generated message:
    //     0: error messages in case OTS fails to sanitize the font.
    //     1: warning messages about issue OTS fixed in the sanitized font.
    virtual void Message(int /* level */, const char* /* format */, ...) MSGFUNC_FMT_ATTR {}

    // This function will be called when OTS needs to decide what to do for a
    // font table.
    //   tag: table tag formed with OTS_TAG() macro
    virtual TableAction GetTableAction(uint32_t /* tag */) { return ots::TABLE_ACTION_DEFAULT; }
};

}  // namespace ots

#undef MEMCPY_OPENTYPE_SANITISER
#undef NTOHL_OPENTYPE_SANITISER
#undef NTOHS_OPENTYPE_SANITISER
#undef HTONL_OPENTYPE_SANITISER
#undef HTONS_OPENTYPE_SANITISER

#endif  // OPENTYPE_SANITISER_H_
