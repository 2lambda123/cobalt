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

#ifndef STARBOARD_SHARED_WIN32_WIN32_VIDEO_DECODER_H_
#define STARBOARD_SHARED_WIN32_WIN32_VIDEO_DECODER_H_

#include <deque>
#include <vector>

#include "starboard/common/ref_counted.h"
#include "starboard/common/scoped_ptr.h"
#include "starboard/media.h"
#include "starboard/shared/starboard/player/decoded_audio_internal.h"
#include "starboard/shared/starboard/player/video_frame_internal.h"
#include "starboard/shared/win32/media_common.h"
#include "starboard/types.h"

namespace starboard {
namespace shared {
namespace win32 {

// VideoDecoder for Win32.
class AbstractWin32VideoDecoder {
 public:
  static scoped_ptr<AbstractWin32VideoDecoder> Create(SbMediaVideoCodec codec);
  virtual ~AbstractWin32VideoDecoder() {}

  virtual bool TryWrite(const scoped_refptr<InputBuffer>& buff) = 0;
  virtual void WriteEndOfStream() = 0;
  virtual VideoFramePtr ProcessAndRead() = 0;

 private:
  virtual void EnsureVideoDecoderCreated() = 0;
};

}  // namespace win32
}  // namespace shared
}  // namespace starboard

#endif  // STARBOARD_SHARED_WIN32_WIN32_VIDEO_DECODER_H_
