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

#include <map>

#include "base/bind.h"
#include "base/callback_helpers.h"
#include "base/logging.h"
#include "base/message_loop.h"
#include "base/synchronization/lock.h"
#include "base/time.h"
#include "media/base/audio_decoder_config.h"
#include "media/base/bind_to_loop.h"
#include "media/base/channel_layout.h"
#include "media/base/decoder_buffer.h"
#include "media/base/demuxer.h"
#include "media/base/demuxer_stream.h"
#include "media/base/filter_collection.h"
#include "media/base/media_export.h"
#include "media/base/media_log.h"
#include "media/base/pipeline.h"
#include "media/base/pipeline_status.h"
#include "media/base/ranges.h"
#include "starboard/player.h"
#include "ui/gfx/size.h"

namespace media {

#if SB_HAS(PLAYER)

using base::Time;
using base::TimeDelta;

namespace {

TimeDelta SbMediaTimeToTimeDelta(SbMediaTime timestamp) {
  return TimeDelta::FromMicroseconds(timestamp * Time::kMicrosecondsPerSecond /
                                     kSbMediaTimeSecond);
}

SbMediaTime TimeDeltaToSbMediaTime(TimeDelta timedelta) {
  return timedelta.InMicroseconds() * kSbMediaTimeSecond /
         Time::kMicrosecondsPerSecond;
}

// SbPlayerPipeline is a PipelineBase implementation that uses the SbPlayer
// interface internally.
class MEDIA_EXPORT SbPlayerPipeline : public Pipeline, public DemuxerHost {
 public:
  // Constructs a media pipeline that will execute on |message_loop|.
  SbPlayerPipeline(const scoped_refptr<base::MessageLoopProxy>& message_loop,
                   MediaLog* media_log);
  ~SbPlayerPipeline() OVERRIDE;

  void Start(scoped_ptr<FilterCollection> filter_collection,
             const PipelineStatusCB& ended_cb,
             const PipelineStatusCB& error_cb,
             const PipelineStatusCB& seek_cb,
             const BufferingStateCB& buffering_state_cb,
             const base::Closure& duration_change_cb) OVERRIDE;

  void Stop(const base::Closure& stop_cb) OVERRIDE;
  void Seek(TimeDelta time, const PipelineStatusCB& seek_cb);
  bool HasAudio() const OVERRIDE;
  bool HasVideo() const OVERRIDE;

  float GetPlaybackRate() const OVERRIDE;
  void SetPlaybackRate(float playback_rate) OVERRIDE;
  float GetVolume() const OVERRIDE;
  void SetVolume(float volume) OVERRIDE;

  TimeDelta GetMediaTime() const OVERRIDE;
  Ranges<TimeDelta> GetBufferedTimeRanges() OVERRIDE;
  TimeDelta GetMediaDuration() const OVERRIDE;
  int64 GetTotalBytes() const OVERRIDE;
  void GetNaturalVideoSize(gfx::Size* out_size) const OVERRIDE;

  bool DidLoadingProgress() const OVERRIDE;
  PipelineStatistics GetStatistics() const OVERRIDE;

 private:
  // DataSourceHost (by way of DemuxerHost) implementation.
  void SetTotalBytes(int64 total_bytes) OVERRIDE;
  void AddBufferedByteRange(int64 start, int64 end) OVERRIDE;
  void AddBufferedTimeRange(TimeDelta start, TimeDelta end) OVERRIDE;

  // DemuxerHost implementaion.
  void SetDuration(TimeDelta duration) OVERRIDE;
  void OnDemuxerError(PipelineStatus error) OVERRIDE;

  void OnDemuxerInitialized(PipelineStatus status);
  void OnDemuxerSeeked(PipelineStatus status);
  void OnDemuxerStopped();
  void OnDemuxerStreamRead(DemuxerStream::Type type,
                           int ticket,
                           DemuxerStream::Status status,
                           const scoped_refptr<DecoderBuffer>& buffer);

  void OnDecoderStatus(SbMediaType type,
                       SbPlayerDecoderState state,
                       int ticket);
  void OnPlayerStatus(SbPlayerState state, int ticket);
  void OnDeallocateSample(void* sample_buffer);

  static void DecoderStatusCB(SbPlayer player,
                              void* context,
                              SbMediaType type,
                              SbPlayerDecoderState state,
                              int ticket);
  static void PlayerStatusCB(SbPlayer player,
                             void* context,
                             SbPlayerState state,
                             int ticket);
  static void DeallocateSampleCB(SbPlayer player,
                                 void* context,
                                 void* sample_buffer);

  // Message loop used to execute pipeline tasks.
  scoped_refptr<base::MessageLoopProxy> message_loop_;

  // MediaLog to which to log events.
  scoped_refptr<MediaLog> media_log_;

  // Lock used to serialize access for the following data members.
  mutable base::Lock lock_;

  // Whether or not the pipeline is running.
  bool running_;

  // Amount of available buffered data.  Set by filters.
  Ranges<int64> buffered_byte_ranges_;
  Ranges<TimeDelta> buffered_time_ranges_;

  // True when AddBufferedByteRange() has been called more recently than
  // DidLoadingProgress().
  mutable bool did_loading_progress_;

  // Total size of the media.  Set by filters.
  int64 total_bytes_;

  // Video's natural width and height.  Set by filters.
  gfx::Size natural_size_;

  // Current volume level (from 0.0f to 1.0f).  This value is set immediately
  // via SetVolume() and a task is dispatched on the message loop to notify the
  // filters.
  float volume_;

  // Current playback rate (>= 0.0f).  This value is set immediately via
  // SetPlaybackRate() and a task is dispatched on the message loop to notify
  // the filters.
  float playback_rate_;

  // Status of the pipeline.  Initialized to PIPELINE_OK which indicates that
  // the pipeline is operating correctly. Any other value indicates that the
  // pipeline is stopped or is stopping.  Clients can call the Stop() method to
  // reset the pipeline state, and restore this to PIPELINE_OK.
  PipelineStatus status_;

  // Whether the media contains rendered audio and video streams.
  // TODO(fischman,scherkus): replace these with checks for
  // {audio,video}_decoder_ once extraction of {Audio,Video}Decoder from the
  // Filter heirarchy is done.
  bool has_audio_;
  bool has_video_;

  int ticket_;

  // The following data members are only accessed by tasks posted to
  // |message_loop_|.
  scoped_ptr<FilterCollection> filter_collection_;

  // Temporary callback used for Start() and Seek().
  PipelineStatusCB seek_cb_;
  SbMediaTime seek_time_;

  // Temporary callback used for Stop().
  base::Closure stop_cb_;

  // Permanent callbacks passed in via Start().
  PipelineStatusCB ended_cb_;
  PipelineStatusCB error_cb_;
  BufferingStateCB buffering_state_cb_;
  base::Closure duration_change_cb_;

  // Demuxer reference used for setting the preload value.
  scoped_refptr<Demuxer> demuxer_;
  bool audio_read_in_progress_;
  bool video_read_in_progress_;
  TimeDelta duration_;

  PipelineStatistics statistics_;

  SbPlayer player_;

  std::map<const void*, scoped_refptr<DecoderBuffer> > decoding_buffers_;

  DISALLOW_COPY_AND_ASSIGN(SbPlayerPipeline);
};

SbPlayerPipeline::SbPlayerPipeline(
    const scoped_refptr<base::MessageLoopProxy>& message_loop,
    MediaLog* media_log)
    : message_loop_(message_loop),
      media_log_(media_log),
      total_bytes_(0),
      natural_size_(0, 0),
      volume_(1.f),
      playback_rate_(0.f),
      has_audio_(false),
      has_video_(false),
      ticket_(SB_PLAYER_INITIAL_TICKET),
      audio_read_in_progress_(false),
      video_read_in_progress_(false),
      player_(kSbPlayerInvalid) {}

SbPlayerPipeline::~SbPlayerPipeline() {
  DCHECK(player_ == kSbPlayerInvalid);
}

void SbPlayerPipeline::Start(scoped_ptr<FilterCollection> filter_collection,
                             const PipelineStatusCB& ended_cb,
                             const PipelineStatusCB& error_cb,
                             const PipelineStatusCB& seek_cb,
                             const BufferingStateCB& buffering_state_cb,
                             const base::Closure& duration_change_cb) {
  if (!message_loop_->BelongsToCurrentThread()) {
    message_loop_->PostTask(
        FROM_HERE,
        base::Bind(&SbPlayerPipeline::Start, this,
                   base::Passed(&filter_collection), ended_cb, error_cb,
                   seek_cb, buffering_state_cb, duration_change_cb));
    return;
  }

  filter_collection_ = filter_collection.Pass();
  ended_cb_ = ended_cb;
  error_cb_ = error_cb;
  seek_cb_ = seek_cb;
  buffering_state_cb_ = buffering_state_cb;
  duration_change_cb_ = duration_change_cb;

  demuxer_ = filter_collection_->GetDemuxer();
  demuxer_->Initialize(
      this, BindToCurrentLoop(
                base::Bind(&SbPlayerPipeline::OnDemuxerInitialized, this)));
}

void SbPlayerPipeline::Stop(const base::Closure& stop_cb) {
  if (!message_loop_->BelongsToCurrentThread()) {
    message_loop_->PostTask(FROM_HERE,
                            base::Bind(&SbPlayerPipeline::Stop, this, stop_cb));
    return;
  }

  DCHECK(stop_cb_.is_null());
  DCHECK(!stop_cb.is_null());

  if (SbPlayerIsValid(player_)) {
    SbPlayerDestroy(player_);
    player_ = kSbPlayerInvalid;
  }
  if (demuxer_) {
    stop_cb_ = stop_cb;
    demuxer_->Stop(base::Bind(&SbPlayerPipeline::OnDemuxerStopped, this));
  } else {
    stop_cb.Run();
  }
}

void SbPlayerPipeline::Seek(TimeDelta time, const PipelineStatusCB& seek_cb) {
  if (!message_loop_->BelongsToCurrentThread()) {
    message_loop_->PostTask(
        FROM_HERE, base::Bind(&SbPlayerPipeline::Seek, this, time, seek_cb));
    return;
  }

  if (!SbPlayerIsValid(player_)) {
    seek_cb.Run(PIPELINE_ERROR_INVALID_STATE);
  }

  DCHECK(seek_cb_.is_null());
  DCHECK(!seek_cb.is_null());

  // Increase |ticket_| so all upcoming need data requests from the SbPlayer
  // are ignored.
  ++ticket_;
  seek_cb_ = seek_cb;
  seek_time_ = TimeDeltaToSbMediaTime(time);
  demuxer_->Seek(time, BindToCurrentLoop(base::Bind(
                           &SbPlayerPipeline::OnDemuxerSeeked, this)));
}

bool SbPlayerPipeline::HasAudio() const {
  base::AutoLock auto_lock(lock_);
  return has_audio_;
}

bool SbPlayerPipeline::HasVideo() const {
  base::AutoLock auto_lock(lock_);
  return has_video_;
}

float SbPlayerPipeline::GetPlaybackRate() const {
  base::AutoLock auto_lock(lock_);
  return playback_rate_;
}

void SbPlayerPipeline::SetPlaybackRate(float playback_rate) {
  if (playback_rate != 0.0f && playback_rate != 1.0f)
    return;

  base::AutoLock auto_lock(lock_);
  playback_rate_ = playback_rate;
  if (SbPlayerIsValid(player_)) {
    SbPlayerSetPause(player_, playback_rate_ == 0.0);
  }
}

float SbPlayerPipeline::GetVolume() const {
  base::AutoLock auto_lock(lock_);
  return volume_;
}

void SbPlayerPipeline::SetVolume(float volume) {
  if (volume < 0.0f || volume > 1.0f)
    return;

  base::AutoLock auto_lock(lock_);
  volume_ = volume;
  if (SbPlayerIsValid(player_)) {
    // SbPlayerSetVolume(player_, volume_);
  }
}

TimeDelta SbPlayerPipeline::GetMediaTime() const {
  base::AutoLock auto_lock(lock_);

  if (!SbPlayerIsValid(player_)) {
    return TimeDelta();
  }
  if (!seek_cb_.is_null()) {
    return SbMediaTimeToTimeDelta(seek_time_);
  }
  SbPlayerInfo info;
  SbPlayerGetInfo(player_, &info);
  return SbMediaTimeToTimeDelta(info.current_media_pts);
}

Ranges<TimeDelta> SbPlayerPipeline::GetBufferedTimeRanges() {
  base::AutoLock auto_lock(lock_);
  Ranges<TimeDelta> time_ranges;
  for (size_t i = 0; i < buffered_time_ranges_.size(); ++i) {
    time_ranges.Add(buffered_time_ranges_.start(i),
                    buffered_time_ranges_.end(i));
  }
  NOTIMPLEMENTED();
  /*if (clock_->Duration() == TimeDelta() || total_bytes_ == 0)
    return time_ranges;
  for (size_t i = 0; i < buffered_byte_ranges_.size(); ++i) {
    TimeDelta start = TimeForByteOffset_Locked(buffered_byte_ranges_.start(i));
    TimeDelta end = TimeForByteOffset_Locked(buffered_byte_ranges_.end(i));
    // Cap approximated buffered time at the length of the video.
    end = std::min(end, clock_->Duration());
    time_ranges.Add(start, end);
  }*/

  return time_ranges;
}

TimeDelta SbPlayerPipeline::GetMediaDuration() const {
  base::AutoLock auto_lock(lock_);
  return duration_;
}

int64 SbPlayerPipeline::GetTotalBytes() const {
  base::AutoLock auto_lock(lock_);
  return total_bytes_;
}

void SbPlayerPipeline::GetNaturalVideoSize(gfx::Size* out_size) const {
  CHECK(out_size);
  base::AutoLock auto_lock(lock_);
  *out_size = natural_size_;
}

bool SbPlayerPipeline::DidLoadingProgress() const {
  base::AutoLock auto_lock(lock_);
  bool ret = did_loading_progress_;
  did_loading_progress_ = false;
  return ret;
}

PipelineStatistics SbPlayerPipeline::GetStatistics() const {
  base::AutoLock auto_lock(lock_);
  return statistics_;
}

void SbPlayerPipeline::SetTotalBytes(int64 total_bytes) {
  base::AutoLock auto_lock(lock_);
  total_bytes_ = total_bytes;
}

void SbPlayerPipeline::SetDuration(TimeDelta duration) {
  base::AutoLock auto_lock(lock_);
  duration_ = duration;
  duration_change_cb_.Run();
}

void SbPlayerPipeline::OnDemuxerError(PipelineStatus error) {
  NOTIMPLEMENTED();
}

void SbPlayerPipeline::AddBufferedByteRange(int64 start, int64 end) {
  base::AutoLock auto_lock(lock_);
  buffered_byte_ranges_.Add(start, end);
  did_loading_progress_ = true;
}

void SbPlayerPipeline::AddBufferedTimeRange(TimeDelta start, TimeDelta end) {
  base::AutoLock auto_lock(lock_);
  buffered_time_ranges_.Add(start, end);
  did_loading_progress_ = true;
}

void SbPlayerPipeline::OnDemuxerInitialized(PipelineStatus status) {
  DCHECK(message_loop_->BelongsToCurrentThread());

  base::AutoLock auto_lock(lock_);
  has_audio_ = demuxer_->GetStream(DemuxerStream::AUDIO) != NULL;
  DCHECK(has_audio_);
  has_video_ = demuxer_->GetStream(DemuxerStream::VIDEO) != NULL;

  buffering_state_cb_.Run(kHaveMetadata);

  NOTIMPLEMENTED() << "Dynamically determinate codecs";

  const AudioDecoderConfig& config =
      demuxer_->GetStream(DemuxerStream::AUDIO)->audio_decoder_config();
  SbMediaAudioHeader audio_header;
  audio_header.format_tag = 0x00ff;
  audio_header.number_of_channels =
      ChannelLayoutToChannelCount(config.channel_layout());
  audio_header.samples_per_second = config.samples_per_second();
  audio_header.average_bytes_per_second = 1;
  audio_header.block_alignment = 4;
  audio_header.bits_per_sample = config.bits_per_channel();
  audio_header.audio_specific_config_size = 0;
  player_ =
      SbPlayerCreate(kSbMediaVideoCodecH264, kSbMediaAudioCodecAac,
                     SB_PLAYER_NO_DURATION, NULL, &audio_header,
                     DeallocateSampleCB, DecoderStatusCB, PlayerStatusCB, this);
}

void SbPlayerPipeline::OnDemuxerSeeked(PipelineStatus status) {
  DCHECK(message_loop_->BelongsToCurrentThread());

  if (status == PIPELINE_OK) {
    DCHECK(!audio_read_in_progress_);
    DCHECK(!video_read_in_progress_);
    ++ticket_;
    SbPlayerSeek(player_, seek_time_, ticket_);
  }
}

void SbPlayerPipeline::OnDemuxerStopped() {
  base::AutoLock auto_lock(lock_);
  base::ResetAndReturn(&stop_cb_).Run();
}

void SbPlayerPipeline::OnDemuxerStreamRead(
    DemuxerStream::Type type,
    int ticket,
    DemuxerStream::Status status,
    const scoped_refptr<DecoderBuffer>& buffer) {
  DCHECK(type == DemuxerStream::AUDIO || type == DemuxerStream::VIDEO)
      << "Unsupported DemuxerStream::Type " << type;

  if (!message_loop_->BelongsToCurrentThread()) {
    message_loop_->PostTask(
        FROM_HERE, base::Bind(&SbPlayerPipeline::OnDemuxerStreamRead, this,
                              type, ticket, status, buffer));
    return;
  }

  if (ticket != ticket_) {
    if (status == DemuxerStream::kConfigChanged) {
      scoped_refptr<DemuxerStream> stream = demuxer_->GetStream(type);
      if (type == DemuxerStream::AUDIO) {
        stream->audio_decoder_config();
      } else {
        stream->video_decoder_config();
      }
    }
    return;
  }

  // In case if Stop() has been called.
  if (!SbPlayerIsValid(player_)) {
    return;
  }

  if (status == DemuxerStream::kAborted) {
    if (type == DemuxerStream::AUDIO) {
      DCHECK(audio_read_in_progress_);
      audio_read_in_progress_ = false;
    } else {
      DCHECK(video_read_in_progress_);
      video_read_in_progress_ = false;
    }
    if (!seek_cb_.is_null()) {
      buffering_state_cb_.Run(kPrerollCompleted);
      base::ResetAndReturn(&seek_cb_).Run(PIPELINE_OK);
    }
    return;
  }

  scoped_refptr<DemuxerStream> stream = demuxer_->GetStream(type);
  if (type == DemuxerStream::AUDIO) {
    if (status == DemuxerStream::kConfigChanged) {
      NOTIMPLEMENTED() << "Update audio decoder config";
      stream->audio_decoder_config();
      stream->Read(base::Bind(&SbPlayerPipeline::OnDemuxerStreamRead, this,
                              type, ticket));
      return;
    }
    audio_read_in_progress_ = false;
    if (buffer->IsEndOfStream()) {
      SbPlayerWriteEndOfStream(player_, kSbMediaTypeAudio);
      return;
    }
    DCHECK(decoding_buffers_.find(buffer->GetData()) ==
           decoding_buffers_.end());
    decoding_buffers_[buffer->GetData()] = buffer;
    SbPlayerWriteSample(
        player_, kSbMediaTypeAudio, const_cast<uint8*>(buffer->GetData()),
        buffer->GetDataSize(), TimeDeltaToSbMediaTime(buffer->GetTimestamp()),
        NULL, NULL);
    return;
  }

  if (status == DemuxerStream::kConfigChanged) {
    NOTIMPLEMENTED() << "Update video decoder config";
    stream->video_decoder_config();
    stream->Read(
        base::Bind(&SbPlayerPipeline::OnDemuxerStreamRead, this, type, ticket));
    return;
  }
  video_read_in_progress_ = false;
  if (buffer->IsEndOfStream()) {
    SbPlayerWriteEndOfStream(player_, kSbMediaTypeAudio);
    return;
  }
  SbMediaVideoSampleInfo sample_info;
  NOTIMPLEMENTED() << "Fill sample info";
  sample_info.is_key_frame = false;
  sample_info.frame_width = 1;
  sample_info.frame_height = 1;
  DCHECK(decoding_buffers_.find(buffer->GetData()) == decoding_buffers_.end());
  decoding_buffers_[buffer->GetData()] = buffer;
  SbPlayerWriteSample(
      player_, kSbMediaTypeVideo, const_cast<uint8*>(buffer->GetData()),
      buffer->GetDataSize(), TimeDeltaToSbMediaTime(buffer->GetTimestamp()),
      &sample_info, NULL);
}

void SbPlayerPipeline::OnDecoderStatus(SbMediaType type,
                                       SbPlayerDecoderState state,
                                       int ticket) {
  DCHECK(message_loop_->BelongsToCurrentThread());
  // In case if Stop() has been called.
  if (!SbPlayerIsValid(player_)) {
    return;
  }
  if (ticket != ticket_) {
    return;
  }

  switch (state) {
    case kSbPlayerDecoderStateNeedsData:
      break;
    case kSbPlayerDecoderStateBufferFull:
      return;
    case kSbPlayerDecoderStateDestroyed:
      return;
  }

  DCHECK_EQ(state, kSbPlayerDecoderStateNeedsData);
  if (type == kSbMediaTypeAudio) {
    if (audio_read_in_progress_) {
      return;
    }
    audio_read_in_progress_ = true;
  } else {
    DCHECK_EQ(type, kSbMediaTypeVideo);
    if (video_read_in_progress_) {
      return;
    }
    video_read_in_progress_ = true;
  }
  DemuxerStream::Type stream_type =
      (type == kSbMediaTypeAudio) ? DemuxerStream::AUDIO : DemuxerStream::VIDEO;
  scoped_refptr<DemuxerStream> stream = demuxer_->GetStream(stream_type);
  stream->Read(base::Bind(&SbPlayerPipeline::OnDemuxerStreamRead, this,
                          stream_type, ticket));
}

void SbPlayerPipeline::OnPlayerStatus(SbPlayerState state, int ticket) {
  DCHECK(message_loop_->BelongsToCurrentThread());

  // In case if Stop() has been called.
  if (!SbPlayerIsValid(player_)) {
    return;
  }
  if (ticket != ticket_) {
    return;
  }
  switch (state) {
    case kSbPlayerStateInitialized:
      ++ticket_;
      SbPlayerSeek(player_, 0, ticket_);
      break;
    case kSbPlayerStatePrerolling:
      break;
    case kSbPlayerStatePresenting:
      buffering_state_cb_.Run(kPrerollCompleted);
      if (!seek_cb_.is_null()) {
        base::ResetAndReturn(&seek_cb_).Run(PIPELINE_OK);
      }
      break;
    case kSbPlayerStateEndOfStream:
      ended_cb_.Run(PIPELINE_OK);
      break;
    case kSbPlayerStateDestroyed:
      break;
    case kSbPlayerStateError:
      // TODO(***REMOVED***): Handle error
      break;
  }
}

void SbPlayerPipeline::OnDeallocateSample(void* sample_buffer) {
  DCHECK(message_loop_->BelongsToCurrentThread());
  DCHECK(decoding_buffers_.find(sample_buffer) != decoding_buffers_.end());
  decoding_buffers_.erase(decoding_buffers_.find(sample_buffer));
}

// static
void SbPlayerPipeline::DecoderStatusCB(SbPlayer player,
                                       void* context,
                                       SbMediaType type,
                                       SbPlayerDecoderState state,
                                       int ticket) {
  SbPlayerPipeline* pipeline = reinterpret_cast<SbPlayerPipeline*>(context);
  DCHECK_EQ(pipeline->player_, player);
  pipeline->message_loop_->PostTask(
      FROM_HERE, base::Bind(&SbPlayerPipeline::OnDecoderStatus, pipeline, type,
                            state, ticket));
}

// static
void SbPlayerPipeline::PlayerStatusCB(SbPlayer player,
                                      void* context,
                                      SbPlayerState state,
                                      int ticket) {
  SbPlayerPipeline* pipeline = reinterpret_cast<SbPlayerPipeline*>(context);
  DCHECK_EQ(pipeline->player_, player);
  pipeline->message_loop_->PostTask(
      FROM_HERE,
      base::Bind(&SbPlayerPipeline::OnPlayerStatus, pipeline, state, ticket));
}

// static
void SbPlayerPipeline::DeallocateSampleCB(SbPlayer player,
                                          void* context,
                                          void* sample_buffer) {
  SbPlayerPipeline* pipeline = reinterpret_cast<SbPlayerPipeline*>(context);
  DCHECK_EQ(pipeline->player_, player);
  pipeline->message_loop_->PostTask(
      FROM_HERE, base::Bind(&SbPlayerPipeline::OnDeallocateSample, pipeline,
                            sample_buffer));
}

}  // namespace

#endif  // SB_HAS(PLAYER)

scoped_refptr<Pipeline> Pipeline::Create(
    const scoped_refptr<base::MessageLoopProxy>& message_loop,
    MediaLog* media_log) {
#if SB_HAS(PLAYER)
  return new SbPlayerPipeline(message_loop, media_log);
#else
  return NULL;
#endif
}

}  // namespace media
