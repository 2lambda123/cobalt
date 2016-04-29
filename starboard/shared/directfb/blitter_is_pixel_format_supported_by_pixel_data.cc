// Copyright 2016 Google Inc. All Rights Reserved.
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

#include <directfb.h>

#include "starboard/blitter.h"
#include "starboard/log.h"
#include "starboard/shared/directfb/blitter_internal.h"

bool SbBlitterIsPixelFormatSupportedByPixelData(
    SbBlitterDevice device,
    SbBlitterPixelDataFormat pixel_format,
    SbBlitterAlphaFormat alpha_format) {
  if (!SbBlitterIsDeviceValid(device)) {
    SB_DLOG(ERROR) << __FUNCTION__ << ": Invalid device.";
    return false;
  }

  // DirectFB supports ARGB but not RGBA.  This implementation currently only
  // supports incoming pixel data in unpremultiplied alpha format.
  return alpha_format == kSbBlitterAlphaFormatUnpremultiplied &&
         (pixel_format == kSbBlitterPixelDataFormatARGB8 ||
          pixel_format == kSbBlitterPixelDataFormatA8);
}
