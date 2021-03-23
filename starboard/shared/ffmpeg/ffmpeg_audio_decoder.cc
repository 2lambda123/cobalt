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

#include "starboard/shared/ffmpeg/ffmpeg_audio_decoder.h"

#include "starboard/audio_sink.h"
#include "starboard/log.h"
#include "starboard/memory.h"

namespace starboard {
namespace shared {
namespace ffmpeg {

namespace {

SbMediaAudioSampleType GetSupportedSampleType() {
  if (SbAudioSinkIsAudioSampleTypeSupported(kSbMediaAudioSampleTypeFloat32)) {
    return kSbMediaAudioSampleTypeFloat32;
  }
  return kSbMediaAudioSampleTypeInt16;
}

// The required output format and the output of the ffmpeg decoder can be
// different.  In this case libavresample is used to convert the ffmpeg output
// into the required format.
void ConvertSamples(int source_sample_format,
                    int target_sample_format,
                    int channel_layout,
                    int sample_rate,
                    int samples_per_channel,
                    uint8_t** input_buffer,
                    uint8_t* output_buffer) {
  AVAudioResampleContext* context = avresample_alloc_context();
  SB_DCHECK(context != NULL);

  av_opt_set_int(context, "in_channel_layout", channel_layout, 0);
  av_opt_set_int(context, "out_channel_layout", channel_layout, 0);
  av_opt_set_int(context, "in_sample_rate", sample_rate, 0);
  av_opt_set_int(context, "out_sample_rate", sample_rate, 0);
  av_opt_set_int(context, "in_sample_fmt", source_sample_format, 0);
  av_opt_set_int(context, "out_sample_fmt", target_sample_format, 0);
  av_opt_set_int(context, "internal_sample_fmt", source_sample_format, 0);

  int result = avresample_open(context);
  SB_DCHECK(!result);

  int samples_resampled =
      avresample_convert(context, &output_buffer, 1024, samples_per_channel,
                         input_buffer, 0, samples_per_channel);
  SB_DCHECK(samples_resampled == samples_per_channel);

  avresample_close(context);
  av_free(context);
}

AVCodecID GetFfmpegCodecIdByMediaCodec(SbMediaAudioCodec audio_codec) {
  switch (audio_codec) {
    case kSbMediaAudioCodecAac:
      return AV_CODEC_ID_AAC;
    case kSbMediaAudioCodecOpus:
      return AV_CODEC_ID_OPUS;
    default:
      return AV_CODEC_ID_NONE;
  }
}

}  // namespace

AudioDecoder::AudioDecoder(SbMediaAudioCodec audio_codec,
                           const SbMediaAudioHeader& audio_header)
    : audio_codec_(audio_codec),
      sample_type_(GetSupportedSampleType()),
      codec_context_(NULL),
      av_frame_(NULL),
      stream_ended_(false),
      audio_header_(audio_header) {
  SB_DCHECK(GetFfmpegCodecIdByMediaCodec(audio_codec) != AV_CODEC_ID_NONE)
      << "Unsupported audio codec " << audio_codec;

  InitializeCodec();
}

AudioDecoder::~AudioDecoder() {
  TeardownCodec();
}

void AudioDecoder::Decode(const InputBuffer& input_buffer) {
  SB_CHECK(codec_context_ != NULL);

  if (stream_ended_) {
    SB_LOG(ERROR) << "Decode() is called after WriteEndOfStream() is called.";
    return;
  }

  AVPacket packet;
  av_init_packet(&packet);
  packet.data = const_cast<uint8_t*>(input_buffer.data());
  packet.size = input_buffer.size();

#if LIBAVUTIL_VERSION_MAJOR > 52
  av_frame_unref(av_frame_);
#else   // LIBAVUTIL_VERSION_MAJOR > 52
  avcodec_get_frame_defaults(av_frame_);
#endif  // LIBAVUTIL_VERSION_MAJOR > 52
  int frame_decoded = 0;
  int result =
      avcodec_decode_audio4(codec_context_, av_frame_, &frame_decoded, &packet);
  if (result != input_buffer.size() || frame_decoded != 1) {
    // TODO: Consider fill it with silence.
    SB_DLOG(WARNING) << "avcodec_decode_audio4() failed with result: " << result
                     << " with input buffer size: " << input_buffer.size()
                     << " and frame decoded: " << frame_decoded;
    return;
  }

  int decoded_audio_size = av_samples_get_buffer_size(
      NULL, codec_context_->channels, av_frame_->nb_samples,
      codec_context_->sample_fmt, 1);
  audio_header_.samples_per_second = codec_context_->sample_rate;

  if (decoded_audio_size > 0) {
    scoped_refptr<DecodedAudio> decoded_audio = new DecodedAudio(
        input_buffer.pts(),
        codec_context_->channels * av_frame_->nb_samples *
            (sample_type_ == kSbMediaAudioSampleTypeInt16 ? 2 : 4));
    if (codec_context_->sample_fmt == codec_context_->request_sample_fmt) {
      SbMemoryCopy(decoded_audio->buffer(), *av_frame_->extended_data,
                   decoded_audio->size());
    } else {
      ConvertSamples(codec_context_->sample_fmt,
                     codec_context_->request_sample_fmt,
                     codec_context_->channel_layout,
                     audio_header_.samples_per_second, av_frame_->nb_samples,
                     av_frame_->extended_data, decoded_audio->buffer());
    }
    decoded_audios_.push(decoded_audio);
  } else {
    // TODO: Consider fill it with silence.
    SB_LOG(ERROR) << "Decoded audio frame is empty.";
  }
}

void AudioDecoder::WriteEndOfStream() {
  // AAC has no dependent frames so we needn't flush the decoder.  Set the flag
  // to ensure that Decode() is not called when the stream is ended.
  stream_ended_ = true;
  // Put EOS into the queue.
  decoded_audios_.push(new DecodedAudio);
}

scoped_refptr<AudioDecoder::DecodedAudio> AudioDecoder::Read() {
  scoped_refptr<DecodedAudio> result;
  if (!decoded_audios_.empty()) {
    result = decoded_audios_.front();
    decoded_audios_.pop();
  }
  return result;
}

void AudioDecoder::Reset() {
  stream_ended_ = false;
  while (!decoded_audios_.empty()) {
    decoded_audios_.pop();
  }
}

SbMediaAudioSampleType AudioDecoder::GetSampleType() const {
  return sample_type_;
}

int AudioDecoder::GetSamplesPerSecond() const {
  return audio_header_.samples_per_second;
}

void AudioDecoder::InitializeCodec() {
  InitializeFfmpeg();

  codec_context_ = avcodec_alloc_context3(NULL);

  if (codec_context_ == NULL) {
    SB_LOG(ERROR) << "Unable to allocate ffmpeg codec context";
    return;
  }

  codec_context_->codec_type = AVMEDIA_TYPE_AUDIO;
  codec_context_->codec_id = GetFfmpegCodecIdByMediaCodec(audio_codec_);
  // Request_sample_fmt is set by us, but sample_fmt is set by the decoder.
  if (sample_type_ == kSbMediaAudioSampleTypeInt16) {
    codec_context_->request_sample_fmt = AV_SAMPLE_FMT_S16;
  } else {
    codec_context_->request_sample_fmt = AV_SAMPLE_FMT_FLT;
  }

  codec_context_->channels = audio_header_.number_of_channels;
  codec_context_->sample_rate = audio_header_.samples_per_second;

  codec_context_->extradata = NULL;
  codec_context_->extradata_size = 0;

  AVCodec* codec = avcodec_find_decoder(codec_context_->codec_id);

  if (codec == NULL) {
    SB_LOG(ERROR) << "Unable to allocate ffmpeg codec context";
    TeardownCodec();
    return;
  }

  int rv = OpenCodec(codec_context_, codec);
  if (rv < 0) {
    SB_LOG(ERROR) << "Unable to open codec";
    TeardownCodec();
    return;
  }

#if LIBAVUTIL_VERSION_MAJOR > 52
  av_frame_ = av_frame_alloc();
#else   // LIBAVUTIL_VERSION_MAJOR > 52
  av_frame_ = avcodec_alloc_frame();
#endif  // LIBAVUTIL_VERSION_MAJOR > 52
  if (av_frame_ == NULL) {
    SB_LOG(ERROR) << "Unable to allocate audio frame";
    TeardownCodec();
  }
}

void AudioDecoder::TeardownCodec() {
  if (codec_context_) {
    CloseCodec(codec_context_);
    av_free(codec_context_);
    codec_context_ = NULL;
  }
  if (av_frame_) {
    av_free(av_frame_);
    av_frame_ = NULL;
  }
}

}  // namespace ffmpeg
}  // namespace shared
}  // namespace starboard