// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/disk_cache/net_log_parameters.h"

#include "base/bind.h"
#include "base/logging.h"
#include "base/string_number_conversions.h"
#include "base/values.h"
#include "net/base/net_errors.h"
#include "net/disk_cache/disk_cache.h"

namespace {

Value* NetLogEntryCreationCallback(const disk_cache::Entry* entry,
                                   bool created,
                                   net::NetLog::LogLevel /* log_level */) {
  DictionaryValue* dict = new DictionaryValue();
  dict->SetString("key", entry->GetKey());
  dict->SetBoolean("created", created);
  return dict;
}

Value* NetLogReadWriteDataCallback(int index,
                                   int offset,
                                   int buf_len,
                                   bool truncate,
                                   net::NetLog::LogLevel /* log_level */) {
  DictionaryValue* dict = new DictionaryValue();
  dict->SetInteger("index", index);
  dict->SetInteger("offset", offset);
  dict->SetInteger("buf_len", buf_len);
  if (truncate)
    dict->SetBoolean("truncate", truncate);
  return dict;
}

Value* NetLogReadWriteCompleteCallback(int bytes_copied,
                                       net::NetLog::LogLevel /* log_level */) {
  DCHECK_NE(bytes_copied, net::ERR_IO_PENDING);
  DictionaryValue* dict = new DictionaryValue();
  if (bytes_copied < 0) {
    dict->SetInteger("net_error", bytes_copied);
  } else {
    dict->SetInteger("bytes_copied", bytes_copied);
  }
  return dict;
}

Value* NetLogSparseOperationCallback(int64 offset,
                                     int buff_len,
                                     net::NetLog::LogLevel /* log_level */) {
  DictionaryValue* dict = new DictionaryValue();
  // Values can only be created with at most 32-bit integers.  Using a string
  // instead circumvents that restriction.
  dict->SetString("offset", base::Int64ToString(offset));
  dict->SetInteger("buff_len", buff_len);
  return dict;
}

Value* NetLogSparseReadWriteCallback(const net::NetLog::Source& source,
                                     int child_len,
                                     net::NetLog::LogLevel /* log_level */) {
  DictionaryValue* dict = new DictionaryValue();
  source.AddToEventParameters(dict);
  dict->SetInteger("child_len", child_len);
  return dict;
}

Value* NetLogGetAvailableRangeResultCallback(
    int64 start,
    int result,
    net::NetLog::LogLevel /* log_level */) {
  DictionaryValue* dict = new DictionaryValue();
  if (result > 0) {
    dict->SetInteger("length", result);
    dict->SetString("start",  base::Int64ToString(start));
  } else {
    dict->SetInteger("net_error", result);
  }
  return dict;
}

}  // namespace

namespace disk_cache {

net::NetLog::ParametersCallback CreateNetLogEntryCreationCallback(
    const Entry* entry,
    bool created) {
  DCHECK(entry);
  return base::Bind(&NetLogEntryCreationCallback, entry, created);
}

net::NetLog::ParametersCallback CreateNetLogReadWriteDataCallback(
    int index,
    int offset,
    int buf_len,
    bool truncate) {
  return base::Bind(&NetLogReadWriteDataCallback,
                    index, offset, buf_len, truncate);
}

net::NetLog::ParametersCallback CreateNetLogReadWriteCompleteCallback(
    int bytes_copied) {
  return base::Bind(&NetLogReadWriteCompleteCallback, bytes_copied);
}

net::NetLog::ParametersCallback CreateNetLogSparseOperationCallback(
    int64 offset,
    int buff_len) {
  return base::Bind(&NetLogSparseOperationCallback, offset, buff_len);
}

net::NetLog::ParametersCallback CreateNetLogSparseReadWriteCallback(
    const net::NetLog::Source& source,
    int child_len) {
  return base::Bind(&NetLogSparseReadWriteCallback, source, child_len);
}

net::NetLog::ParametersCallback CreateNetLogGetAvailableRangeResultCallback(
    int64 start,
    int result) {
  return base::Bind(&NetLogGetAvailableRangeResultCallback, start, result);
}

}  // namespace disk_cache
