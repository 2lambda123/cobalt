// Copyright 2017 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef STARBOARD_SHARED_WIN32_DECODE_TARGET_INTERNAL_H_
#define STARBOARD_SHARED_WIN32_DECODE_TARGET_INTERNAL_H_

#include "starboard/common/ref_counted.h"
#include "starboard/decode_target.h"
#include "starboard/shared/win32/media_common.h"

struct SbDecodeTargetPrivate {
  // Publicly accessible information about the decode target.
  SbDecodeTargetInfo info;
  ::starboard::shared::win32::VideoFramePtr frame;
  explicit SbDecodeTargetPrivate(starboard::shared::win32::VideoFramePtr frame);
  ~SbDecodeTargetPrivate();
};

#endif  // STARBOARD_SHARED_WIN32_DECODE_TARGET_INTERNAL_H_
