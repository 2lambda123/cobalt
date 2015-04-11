/*
 * Copyright 2012 Google Inc. All Rights Reserved.
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

#ifndef SRC_DIAL_SERVICE_HANDLER_H_
#define SRC_DIAL_SERVICE_HANDLER_H_

#include <map>
#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/callback_forward.h"
#include "net/http/http_response_headers.h"

namespace net {

class HttpServerRequestInfo;

class HttpServerResponseInfo {
 public:
  int response_code;
  std::string mime_type;
  std::string body;
  std::vector<std::string> headers;
};

class DialServiceHandler {
 public:
  typedef base::Callback<void(scoped_ptr<HttpServerResponseInfo>, bool)>
      CompletionCB;
  virtual ~DialServiceHandler() {}
  virtual bool handleRequest(const std::string& path,
                             const HttpServerRequestInfo& request,
                             const CompletionCB& completion_cb) = 0;
  virtual const std::string service_name() const  = 0;
};

} // namespace net

#endif  // SRC_DIAL_SERVICE_HANDLER_H_

