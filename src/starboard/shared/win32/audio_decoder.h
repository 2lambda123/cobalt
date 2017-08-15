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

#ifndef STARBOARD_SHARED_WIN32_AUDIO_DECODER_H_
#define STARBOARD_SHARED_WIN32_AUDIO_DECODER_H_

#include "starboard/common/ref_counted.h"
#include "starboard/common/scoped_ptr.h"
#include "starboard/configuration.h"
#include "starboard/media.h"
#include "starboard/shared/internal_only.h"
#include "starboard/shared/starboard/player/decoded_audio_internal.h"
#include "starboard/shared/starboard/player/filter/audio_decoder_internal.h"
#include "starboard/shared/starboard/player/job_queue.h"
#include "starboard/shared/win32/atomic_queue.h"
#include "starboard/shared/win32/audio_decoder_thread.h"
#include "starboard/shared/win32/media_common.h"
#include "starboard/shared/win32/win32_decoder_impl.h"

namespace starboard {
namespace shared {
namespace win32 {

using JobQueue = ::starboard::shared::starboard::player::JobQueue;
using JobOwner = JobQueue::JobOwner;

class AudioDecoder
    : public ::starboard::shared::starboard::player::filter::AudioDecoder,
      private JobOwner,
      private AudioDecodedCallback {
 public:
  AudioDecoder(SbMediaAudioCodec audio_codec,
               const SbMediaAudioHeader& audio_header);
  ~AudioDecoder() SB_OVERRIDE;

  void Decode(const scoped_refptr<InputBuffer>& input_buffer,
              const Closure& consumed_cb) SB_OVERRIDE;
  void WriteEndOfStream() SB_OVERRIDE;
  scoped_refptr<DecodedAudio> Read() SB_OVERRIDE;
  void Reset() SB_OVERRIDE;
  SbMediaAudioSampleType GetSampleType() const SB_OVERRIDE;
  int GetSamplesPerSecond() const SB_OVERRIDE;

  void Initialize(const Closure& output_cb) SB_OVERRIDE;
  SbMediaAudioFrameStorageType GetStorageType() const SB_OVERRIDE {
    return kSbMediaAudioFrameStorageTypeInterleaved;
  }
  void OnAudioDecoded(DecodedAudioPtr data) SB_OVERRIDE;

 private:
  class CallbackScheduler;
  SbMediaAudioHeader audio_header_;
  SbMediaAudioSampleType sample_type_;
  SbMediaAudioCodec audio_codec_;
  bool stream_ended_;

  AtomicQueue<DecodedAudioPtr> decoded_data_;
  scoped_ptr<AudioDecoder::CallbackScheduler> callback_scheduler_;
  scoped_ptr<AbstractWin32AudioDecoder> decoder_impl_;
  scoped_ptr<AudioDecoderThread> decoder_thread_;
  Closure output_cb_;

  ::starboard::Mutex mutex_;
};

}  // namespace win32
}  // namespace shared
}  // namespace starboard

#endif  // STARBOARD_SHARED_WIN32_AUDIO_DECODER_H_
