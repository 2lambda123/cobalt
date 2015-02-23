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

#ifndef DOM_MEDIA_ERROR_H_
#define DOM_MEDIA_ERROR_H_

#include "cobalt/script/wrappable.h"

namespace cobalt {
namespace dom {

// The MediaError represents a media element error with an error code.
//   http://www.w3.org/TR/html5/embedded-content-0.html#mediaerror
class MediaError : public script::Wrappable {
 public:
  // Web API: MediaError
  //
  enum Code {
    kMediaErrAborted = 1,
    kMediaErrNetwork = 2,
    kMediaErrDecode = 3,
    kMediaErrSrcNotSupported = 4,
  };

  Code code() const { return code_; }

 private:
  Code code_;
};

}  // namespace dom
}  // namespace cobalt

#endif  // DOM_MEDIA_ERROR_H_
