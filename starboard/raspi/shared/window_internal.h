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

#ifndef STARBOARD_RASPI_SHARED_WINDOW_INTERNAL_H_
#define STARBOARD_RASPI_SHARED_WINDOW_INTERNAL_H_

#include <bcm_host.h>
#include <EGL/egl.h>

#include "starboard/shared/internal_only.h"
#include "starboard/window.h"

struct SbWindowPrivate {
  SbWindowPrivate(DISPMANX_DISPLAY_HANDLE_T display,
                  const SbWindowOptions* options);
  ~SbWindowPrivate();

  DISPMANX_DISPLAY_HANDLE_T display;
  DISPMANX_ELEMENT_HANDLE_T element;
  EGL_DISPMANX_WINDOW_T window;
};

#endif  // STARBOARD_RASPI_SHARED_WINDOW_INTERNAL_H_
