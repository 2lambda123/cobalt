// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/url_request/url_request_context.h"

#include "base/string_util.h"
#include "net/base/cookie_store.h"
#include "net/base/host_resolver.h"

URLRequestContext::URLRequestContext()
    : net_log_(NULL),
      http_transaction_factory_(NULL),
      ftp_transaction_factory_(NULL),
      http_auth_handler_factory_(NULL),
      network_delegate_(NULL),
      cookie_policy_(NULL),
      transport_security_state_(NULL),
      is_main_(false) {
}

const std::string& URLRequestContext::GetUserAgent(const GURL& url) const {
  return EmptyString();
}

URLRequestContext::~URLRequestContext() {
}
