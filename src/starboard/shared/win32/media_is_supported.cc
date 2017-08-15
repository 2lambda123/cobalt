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

#include "starboard/log.h"
#include "starboard/media.h"
#include "starboard/string.h"

// TODO: Fill this in for DRM.
SB_EXPORT bool SbMediaIsSupported(SbMediaVideoCodec video_codec,
                                  SbMediaAudioCodec audio_codec,
                                  const char* key_system) {
  SB_UNREFERENCED_PARAMETER(video_codec);
  SB_UNREFERENCED_PARAMETER(audio_codec);
  SB_UNREFERENCED_PARAMETER(key_system);
  return 0 == SbStringCompareAll(key_system, "com.youtube.playready");
}
