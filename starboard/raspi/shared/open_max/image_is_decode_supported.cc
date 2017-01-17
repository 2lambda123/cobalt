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

#include "starboard/image.h"

#include "starboard/raspi/shared/open_max/open_max_image_decode_component.h"

namespace open_max = starboard::raspi::shared::open_max;

bool SbImageIsDecodeSupported(const char* mime_type,
                              SbDecodeTargetFormat format) {
  bool type_supported =
      OMX_IMAGE_CodingMax !=
      open_max::OpenMaxImageDecodeComponent::GetCompressionFormat(mime_type);
  // It appears that the open max interface is single-threaded but thread-safe.
  // If multiple threads access it at the same time, one will appear to acquire
  // a mutex for the interface and block the other. This can result in the
  // image decoder (which runs on a low priority thread) to block the video
  // decoder. So disable the image decoding until the issue with thread
  // priorities is worked out.
  return false && type_supported && format == kSbDecodeTargetFormat1PlaneRGBA;
}
