// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <vector>

#include "base/bind.h"
#include "base/callback_helpers.h"
#include "base/message_loop.h"
#include "media/base/decoder_buffer.h"
#include "media/base/decrypt_config.h"
#include "media/base/mock_callback.h"
#include "media/base/mock_filters.h"
#include "media/base/video_frame.h"
#include "media/filters/ffmpeg_decoder_unittest.h"
#include "media/filters/decrypting_video_decoder.h"
#include "testing/gmock/include/gmock/gmock.h"

using ::testing::_;
using ::testing::Invoke;
using ::testing::IsNull;
using ::testing::ReturnRef;
using ::testing::SaveArg;
using ::testing::StrictMock;

namespace media {

static const VideoFrame::Format kVideoFormat = VideoFrame::YV12;
static const gfx::Size kCodedSize(320, 240);
static const gfx::Rect kVisibleRect(320, 240);
static const gfx::Size kNaturalSize(320, 240);
static const uint8 kFakeKeyId[] = { 0x4b, 0x65, 0x79, 0x20, 0x49, 0x44 };
static const uint8 kFakeIv[DecryptConfig::kDecryptionKeySize] = { 0 };

// Create a fake non-empty encrypted buffer.
static scoped_refptr<DecoderBuffer> CreateFakeEncryptedBuffer() {
  const int buffer_size = 16;  // Need a non-empty buffer;
  scoped_refptr<DecoderBuffer> buffer(new DecoderBuffer(buffer_size));
  buffer->SetDecryptConfig(scoped_ptr<DecryptConfig>(new DecryptConfig(
      std::string(reinterpret_cast<const char*>(kFakeKeyId),
                  arraysize(kFakeKeyId)),
      std::string(reinterpret_cast<const char*>(kFakeIv), arraysize(kFakeIv)),
      0,
      std::vector<SubsampleEntry>())));
  return buffer;
}

ACTION_P(ReturnBuffer, buffer) {
  arg0.Run(buffer ? DemuxerStream::kOk : DemuxerStream::kAborted, buffer);
}

ACTION(ReturnConfigChanged) {
  arg0.Run(DemuxerStream::kConfigChanged, scoped_refptr<DecoderBuffer>(NULL));
}

ACTION_P(RunCallback1, param) {
  arg1.Run(param);
}

ACTION_P2(RunCallback2, param1, param2) {
  arg1.Run(param1, param2);
}

class DecryptingVideoDecoderTest : public testing::Test {
 public:
  DecryptingVideoDecoderTest()
      : decryptor_(new StrictMock<MockDecryptor>()),
        decoder_(new StrictMock<DecryptingVideoDecoder>(
            base::Bind(&Identity<scoped_refptr<base::MessageLoopProxy> >,
                       message_loop_.message_loop_proxy()),
                       decryptor_.get())),
        demuxer_(new StrictMock<MockDemuxerStream>()),
        encrypted_buffer_(CreateFakeEncryptedBuffer()),
        decoded_video_frame_(VideoFrame::CreateBlackFrame(kCodedSize)),
        null_video_frame_(scoped_refptr<VideoFrame>()),
        end_of_stream_video_frame_(VideoFrame::CreateEmptyFrame()) {
  }

  virtual ~DecryptingVideoDecoderTest() {
    Stop();
  }

  void InitializeAndExpectStatus(const VideoDecoderConfig& config,
                                 PipelineStatus status) {
    EXPECT_CALL(*demuxer_, video_decoder_config())
        .WillRepeatedly(ReturnRef(config));

    decoder_->Initialize(demuxer_, NewExpectedStatusCB(status),
                         base::Bind(&MockStatisticsCB::OnStatistics,
                                    base::Unretained(&statistics_cb_)));
    message_loop_.RunAllPending();
  }

  void Initialize() {
    EXPECT_CALL(*decryptor_, InitializeVideoDecoder(_, _))
        .WillOnce(RunCallback1(true));

    config_.Initialize(kCodecVP8, VIDEO_CODEC_PROFILE_UNKNOWN, kVideoFormat,
                       kCodedSize, kVisibleRect, kNaturalSize,
                       NULL, 0, true, true);

    InitializeAndExpectStatus(config_, PIPELINE_OK);
  }

  void ReadAndExpectFrameReadyWith(
      VideoDecoder::Status status,
      const scoped_refptr<VideoFrame>& video_frame) {
    EXPECT_CALL(*this, FrameReady(status, video_frame));

    decoder_->Read(base::Bind(&DecryptingVideoDecoderTest::FrameReady,
                              base::Unretained(this)));
    message_loop_.RunAllPending();
  }

  // Sets up expectations and actions to put DecryptingVideoDecoder in an
  // active normal decoding state.
  void EnterNormalDecodingState() {
    EXPECT_CALL(*demuxer_, Read(_))
        .WillOnce(ReturnBuffer(encrypted_buffer_))
        .WillRepeatedly(ReturnBuffer(DecoderBuffer::CreateEOSBuffer()));
    EXPECT_CALL(*decryptor_, DecryptAndDecodeVideo(_, _))
        .WillOnce(RunCallback2(Decryptor::kSuccess, decoded_video_frame_))
        .WillRepeatedly(RunCallback2(Decryptor::kSuccess,
                                     end_of_stream_video_frame_));
    EXPECT_CALL(statistics_cb_, OnStatistics(_));

    ReadAndExpectFrameReadyWith(VideoDecoder::kOk, decoded_video_frame_);
  }

  // Sets up expectations and actions to put DecryptingVideoDecoder in an end
  // of stream state. This function must be called after
  // EnterNormalDecodingState() to work.
  void EnterEndOfStreamState() {
    ReadAndExpectFrameReadyWith(VideoDecoder::kOk, end_of_stream_video_frame_);
  }

  // Make the read callback pending by saving and not firing it.
  void EnterPendingReadState() {
    EXPECT_TRUE(pending_demuxer_read_cb_.is_null());
    EXPECT_CALL(*demuxer_, Read(_))
        .WillOnce(SaveArg<0>(&pending_demuxer_read_cb_));
    decoder_->Read(base::Bind(&DecryptingVideoDecoderTest::FrameReady,
                              base::Unretained(this)));
    message_loop_.RunAllPending();
    // Make sure the Read() on the decoder triggers a Read() on the demuxer.
    EXPECT_FALSE(pending_demuxer_read_cb_.is_null());
  }

  // Make the video decode callback pending by saving and not firing it.
  void EnterPendingDecryptAndDecodeState() {
    EXPECT_TRUE(pending_video_decode_cb_.is_null());
    EXPECT_CALL(*demuxer_, Read(_))
        .WillRepeatedly(ReturnBuffer(encrypted_buffer_));
    EXPECT_CALL(*decryptor_, DecryptAndDecodeVideo(encrypted_buffer_, _))
        .WillOnce(SaveArg<1>(&pending_video_decode_cb_));

    decoder_->Read(base::Bind(&DecryptingVideoDecoderTest::FrameReady,
                              base::Unretained(this)));
    message_loop_.RunAllPending();
    // Make sure the Read() on the decoder triggers a DecryptAndDecode() on the
    // decryptor.
    EXPECT_FALSE(pending_video_decode_cb_.is_null());
  }

  void AbortPendingVideoDecodeCB() {
    if (!pending_video_decode_cb_.is_null()) {
      base::ResetAndReturn(&pending_video_decode_cb_).Run(
          Decryptor::kSuccess, scoped_refptr<VideoFrame>(NULL));
    }
  }

  void AbortAllPendingCBs() {
    if (!pending_init_cb_.is_null()) {
      ASSERT_TRUE(pending_video_decode_cb_.is_null());
      base::ResetAndReturn(&pending_init_cb_).Run(false);
      return;
    }

    AbortPendingVideoDecodeCB();
  }

  void Reset() {
    EXPECT_CALL(*decryptor_, CancelDecryptAndDecodeVideo())
        .WillOnce(Invoke(
            this, &DecryptingVideoDecoderTest::AbortPendingVideoDecodeCB));

    decoder_->Reset(NewExpectedClosure());
    message_loop_.RunAllPending();
  }

  void Stop() {
    EXPECT_CALL(*decryptor_, StopVideoDecoder())
        .WillOnce(Invoke(
            this, &DecryptingVideoDecoderTest::AbortAllPendingCBs));

    decoder_->Stop(NewExpectedClosure());
    message_loop_.RunAllPending();
  }

  MOCK_METHOD2(FrameReady, void(VideoDecoder::Status,
                                const scoped_refptr<VideoFrame>&));

  MessageLoop message_loop_;
  scoped_ptr<StrictMock<MockDecryptor> > decryptor_;
  scoped_refptr<StrictMock<DecryptingVideoDecoder> > decoder_;
  scoped_refptr<StrictMock<MockDemuxerStream> > demuxer_;
  MockStatisticsCB statistics_cb_;
  VideoDecoderConfig config_;

  DemuxerStream::ReadCB pending_demuxer_read_cb_;
  Decryptor::DecoderInitCB pending_init_cb_;
  Decryptor::VideoDecodeCB pending_video_decode_cb_;

  // Constant buffer/frames to be returned by the |demuxer_| and |decryptor_|.
  scoped_refptr<DecoderBuffer> encrypted_buffer_;
  scoped_refptr<VideoFrame> decoded_video_frame_;
  scoped_refptr<VideoFrame> null_video_frame_;
  scoped_refptr<VideoFrame> end_of_stream_video_frame_;

 private:
  DISALLOW_COPY_AND_ASSIGN(DecryptingVideoDecoderTest);
};

TEST_F(DecryptingVideoDecoderTest, Initialize_Normal) {
  Initialize();
}

// Ensure that DecryptingVideoDecoder only accepts encrypted video.
TEST_F(DecryptingVideoDecoderTest, Initialize_UnencryptedVideoConfig) {
  VideoDecoderConfig config(kCodecVP8, VIDEO_CODEC_PROFILE_UNKNOWN,
                            kVideoFormat,
                            kCodedSize, kVisibleRect, kNaturalSize,
                            NULL, 0, false);

  InitializeAndExpectStatus(config, DECODER_ERROR_NOT_SUPPORTED);
}

// Ensure decoder handles invalid video configs without crashing.
TEST_F(DecryptingVideoDecoderTest, Initialize_InvalidVideoConfig) {
  VideoDecoderConfig config(kCodecVP8, VIDEO_CODEC_PROFILE_UNKNOWN,
                            VideoFrame::INVALID,
                            kCodedSize, kVisibleRect, kNaturalSize,
                            NULL, 0, true);

  InitializeAndExpectStatus(config, PIPELINE_ERROR_DECODE);
}

// Ensure decoder handles unsupported video configs without crashing.
TEST_F(DecryptingVideoDecoderTest, Initialize_UnsupportedVideoConfig) {
  EXPECT_CALL(*decryptor_, InitializeVideoDecoder(_, _))
      .WillOnce(RunCallback1(false));

  VideoDecoderConfig config(kCodecVP8, VIDEO_CODEC_PROFILE_UNKNOWN,
                            kVideoFormat,
                            kCodedSize, kVisibleRect, kNaturalSize,
                            NULL, 0, true);

  InitializeAndExpectStatus(config, DECODER_ERROR_NOT_SUPPORTED);
}

// Test normal decrypt and decode case.
TEST_F(DecryptingVideoDecoderTest, DecryptAndDecode_Normal) {
  Initialize();
  EnterNormalDecodingState();
}

// Test the case where the decryptor returns error when doing decrypt and
// decode.
TEST_F(DecryptingVideoDecoderTest, DecryptAndDecode_DecodeError) {
  Initialize();

  EXPECT_CALL(*demuxer_, Read(_))
      .WillRepeatedly(ReturnBuffer(encrypted_buffer_));
  EXPECT_CALL(*decryptor_, DecryptAndDecodeVideo(_, _))
      .WillRepeatedly(RunCallback2(Decryptor::kError,
                                   scoped_refptr<VideoFrame>(NULL)));

  ReadAndExpectFrameReadyWith(VideoDecoder::kDecodeError, null_video_frame_);
}

// Test the case where the decryptor does not have the decryption key to do
// decrypt and decode.
TEST_F(DecryptingVideoDecoderTest, DecryptAndDecode_NoKey) {
  Initialize();

  EXPECT_CALL(*demuxer_, Read(_))
      .WillRepeatedly(ReturnBuffer(encrypted_buffer_));
  EXPECT_CALL(*decryptor_, DecryptAndDecodeVideo(_, _))
      .WillRepeatedly(RunCallback2(Decryptor::kNoKey, null_video_frame_));

  ReadAndExpectFrameReadyWith(VideoDecoder::kDecodeError, null_video_frame_);
}

// Test the case where the decryptor returns kNeedMoreData to ask for more
// buffers before it can produce a frame.
TEST_F(DecryptingVideoDecoderTest, DecryptAndDecode_NeedMoreData) {
  Initialize();

  EXPECT_CALL(*demuxer_, Read(_))
      .Times(2)
      .WillRepeatedly(ReturnBuffer(encrypted_buffer_));
  EXPECT_CALL(*decryptor_, DecryptAndDecodeVideo(_, _))
      .WillOnce(RunCallback2(Decryptor::kNeedMoreData,
                             scoped_refptr<VideoFrame>()))
      .WillRepeatedly(RunCallback2(Decryptor::kSuccess, decoded_video_frame_));
  EXPECT_CALL(statistics_cb_, OnStatistics(_))
      .Times(2);

  ReadAndExpectFrameReadyWith(VideoDecoder::kOk, decoded_video_frame_);
}

// Test the case where the decryptor receives end-of-stream buffer.
TEST_F(DecryptingVideoDecoderTest, DecryptAndDecode_EndOfStream) {
  Initialize();
  EnterNormalDecodingState();
  EnterEndOfStreamState();
}

// Test resetting when decoder has initialized but has not decoded any frame.
TEST_F(DecryptingVideoDecoderTest, Reset_Initialized) {
  Initialize();
  Reset();
}

// Test resetting when decoder has decoded single frame.
TEST_F(DecryptingVideoDecoderTest, Reset_Decoding) {
  Initialize();
  EnterNormalDecodingState();
  Reset();
}

// Test resetting when decoder has hit end of stream.
TEST_F(DecryptingVideoDecoderTest, Reset_EndOfStream) {
  Initialize();
  EnterNormalDecodingState();
  EnterEndOfStreamState();
  Reset();
}

// Test resetting after the decoder has been reset.
TEST_F(DecryptingVideoDecoderTest, Reset_AfterReset) {
  Initialize();
  EnterNormalDecodingState();
  Reset();
  Reset();
}

// Test resetting when there is a pending read on the demuxer.
TEST_F(DecryptingVideoDecoderTest, Reset_DuringPendingDemuxerRead) {
  Initialize();
  EnterPendingReadState();

  EXPECT_CALL(*this, FrameReady(VideoDecoder::kOk, IsNull()));

  Reset();
  base::ResetAndReturn(&pending_demuxer_read_cb_).Run(DemuxerStream::kOk,
                                                      encrypted_buffer_);
  message_loop_.RunAllPending();
}

// Test resetting when there is a pending video decode callback on the
// decryptor.
TEST_F(DecryptingVideoDecoderTest, Reset_DuringPendingDecryptAndDecode) {
  Initialize();
  EnterPendingDecryptAndDecodeState();

  EXPECT_CALL(*this, FrameReady(VideoDecoder::kOk, IsNull()));

  Reset();
}

// Test stopping when decoder has initialized but has not decoded any frame.
TEST_F(DecryptingVideoDecoderTest, Stop_Initialized) {
  Initialize();
  Stop();
}

// Test stopping when decoder has decoded single frame.
TEST_F(DecryptingVideoDecoderTest, Stop_Decoding) {
  Initialize();
  EnterNormalDecodingState();
  Stop();
}

// Test stopping when decoder has hit end of stream.
TEST_F(DecryptingVideoDecoderTest, Stop_EndOfStream) {
  Initialize();
  EnterNormalDecodingState();
  EnterEndOfStreamState();
  Stop();
}

// Test stopping when there is a pending read on the demuxer.
TEST_F(DecryptingVideoDecoderTest, Stop_DuringPendingInitialize) {
  EXPECT_CALL(*decryptor_, InitializeVideoDecoder(_, _))
      .WillOnce(SaveArg<1>(&pending_init_cb_));

  config_.Initialize(kCodecVP8, VIDEO_CODEC_PROFILE_UNKNOWN, kVideoFormat,
                     kCodedSize, kVisibleRect, kNaturalSize, NULL, 0, true,
                     true);
  InitializeAndExpectStatus(config_, DECODER_ERROR_NOT_SUPPORTED);
  EXPECT_FALSE(pending_init_cb_.is_null());

  Stop();
}

// Test stopping when there is a pending read on the demuxer.
TEST_F(DecryptingVideoDecoderTest, Stop_DuringPendingDemuxerRead) {
  Initialize();
  EnterPendingReadState();

  EXPECT_CALL(*this, FrameReady(VideoDecoder::kOk, IsNull()));

  Stop();
  base::ResetAndReturn(&pending_demuxer_read_cb_).Run(DemuxerStream::kOk,
                                                      encrypted_buffer_);
  message_loop_.RunAllPending();
}

// Test stopping when there is a pending video decode callback on the
// decryptor.
TEST_F(DecryptingVideoDecoderTest, Stop_DuringPendingDecryptAndDecode) {
  Initialize();
  EnterPendingDecryptAndDecodeState();

  EXPECT_CALL(*this, FrameReady(VideoDecoder::kOk, IsNull()));

  Stop();
}

// Test stopping when there is a pending reset on the decoder.
// Reset is pending because it cannot complete when the video decode callback
// is pending.
TEST_F(DecryptingVideoDecoderTest, Stop_DuringPendingReset) {
  Initialize();
  EnterPendingDecryptAndDecodeState();

  EXPECT_CALL(*decryptor_, CancelDecryptAndDecodeVideo());
  EXPECT_CALL(*this, FrameReady(VideoDecoder::kOk, IsNull()));

  decoder_->Reset(NewExpectedClosure());
  Stop();
}

// Test stopping after the decoder has been reset.
TEST_F(DecryptingVideoDecoderTest, Stop_AfterReset) {
  Initialize();
  EnterNormalDecodingState();
  Reset();
  Stop();
}

// Test stopping after the decoder has been stopped.
TEST_F(DecryptingVideoDecoderTest, Stop_AfterStop) {
  Initialize();
  EnterNormalDecodingState();
  Stop();
  Stop();
}

// Test aborted read on the demuxer stream.
TEST_F(DecryptingVideoDecoderTest, AbortPendingDemuxerRead) {
  Initialize();

  // ReturnBuffer() with NULL triggers aborted demuxer read.
  EXPECT_CALL(*demuxer_, Read(_))
      .WillOnce(ReturnBuffer(scoped_refptr<DecoderBuffer>()));

  ReadAndExpectFrameReadyWith(VideoDecoder::kOk, null_video_frame_);
}

// Test aborted read on the demuxer stream when the decoder is being reset.
TEST_F(DecryptingVideoDecoderTest, AbortPendingDemuxerReadDuringReset) {
  Initialize();
  EnterPendingReadState();

  // Make sure we get a NULL video frame returned.
  EXPECT_CALL(*this, FrameReady(VideoDecoder::kOk, IsNull()));

  Reset();
  base::ResetAndReturn(&pending_demuxer_read_cb_).Run(DemuxerStream::kAborted,
                                                      NULL);
  message_loop_.RunAllPending();
}

// Test config change on the demuxer stream.
TEST_F(DecryptingVideoDecoderTest, ConfigChanged) {
  Initialize();

  EXPECT_CALL(*demuxer_, Read(_))
      .WillOnce(ReturnConfigChanged());

  // TODO(xhwang): Update this test when kConfigChanged is supported in
  // DecryptingVideoDecoder.
  ReadAndExpectFrameReadyWith(VideoDecoder::kDecodeError, null_video_frame_);
}

}  // namespace media
