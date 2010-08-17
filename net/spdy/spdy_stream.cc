// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/spdy/spdy_stream.h"

#include "base/logging.h"
#include "base/message_loop.h"
#include "base/singleton.h"
#include "net/spdy/spdy_session.h"

namespace net {

SpdyStream::SpdyStream(
    SpdySession* session, spdy::SpdyStreamId stream_id, bool pushed)
    : continue_buffering_data_(true),
      stream_id_(stream_id),
      priority_(0),
      stalled_by_flow_control_(false),
      send_window_size_(spdy::kInitialWindowSize),
      pushed_(pushed),
      metrics_(Singleton<BandwidthMetrics>::get()),
      response_received_(false),
      session_(session),
      delegate_(NULL),
      request_time_(base::Time::Now()),
      response_(new spdy::SpdyHeaderBlock),
      io_state_(STATE_NONE),
      response_status_(OK),
      cancelled_(false),
      send_bytes_(0),
      recv_bytes_(0),
      histograms_recorded_(false) {}

SpdyStream::~SpdyStream() {
  DLOG(INFO) << "Deleting SpdyStream for stream " << stream_id_;
}

void SpdyStream::SetDelegate(Delegate* delegate) {
  CHECK(delegate);
  delegate_ = delegate;

  if (pushed_) {
    CHECK(response_received());
    MessageLoop::current()->PostTask(
        FROM_HERE, NewRunnableMethod(this,
                                     &SpdyStream::PushedStreamReplayData));
  } else {
    continue_buffering_data_ = false;
  }
}

void SpdyStream::PushedStreamReplayData() {
  if (cancelled_ || delegate_ == NULL)
    return;

  delegate_->OnResponseReceived(*response_, response_time_, OK);

  continue_buffering_data_ = false;
  std::vector<scoped_refptr<IOBufferWithSize> > buffers;
  buffers.swap(pending_buffers_);
  for (size_t i = 0; i < buffers.size(); ++i) {
    if (delegate_){
      if (buffers[i])
        delegate_->OnDataReceived(buffers[i]->data(), buffers[i]->size());
      else
        delegate_->OnDataReceived(NULL, 0);
    }
  }
}

void SpdyStream::DetachDelegate() {
  delegate_ = NULL;
  if (!closed())
    Cancel();
}

const linked_ptr<spdy::SpdyHeaderBlock>& SpdyStream::spdy_headers() const {
  return request_;
}

void SpdyStream::set_spdy_headers(
    const linked_ptr<spdy::SpdyHeaderBlock>& headers) {
  request_ = headers;
}

void SpdyStream::IncreaseSendWindowSize(int delta_window_size) {
  DCHECK_GE(delta_window_size, 1);
  int new_window_size = send_window_size_ + delta_window_size;

  // We should ignore WINDOW_UPDATEs received before or after this state,
  // since before means we've not written SYN_STREAM yet (i.e. it's too
  // early) and after means we've written a DATA frame with FIN bit.
  if (io_state_ != STATE_SEND_BODY_COMPLETE)
    return;

  // it's valid for send_window_size_ to become negative (via an incoming
  // SETTINGS), in which case incoming WINDOW_UPDATEs will eventually make
  // it positive; however, if send_window_size_ is positive and incoming
  // WINDOW_UPDATE makes it negative, we have an overflow.
  if (send_window_size_ > 0 && new_window_size < 0) {
    LOG(WARNING) << "Received WINDOW_UPDATE [delta:" << delta_window_size
                 << "] for stream " << stream_id_
                 << " overflows send_window_size_ [current:"
                 << send_window_size_ << "]";
    session_->ResetStream(stream_id_, spdy::FLOW_CONTROL_ERROR);
    return;
  }

  LOG(INFO) << "Increasing stream " << stream_id_
            << " send_window_size_ [current:" << send_window_size_ << "]"
            << " by " << delta_window_size << " bytes";
  send_window_size_ = new_window_size;

  if (stalled_by_flow_control_) {
    stalled_by_flow_control_ = false;
    io_state_ = STATE_SEND_BODY;
    DoLoop(OK);
  }
}

void SpdyStream::DecreaseSendWindowSize(int delta_window_size) {
  // we only call this method when sending a frame, therefore
  // |delta_window_size| should be within the valid frame size range.
  DCHECK_GE(delta_window_size, 1);
  DCHECK_LE(delta_window_size, kMaxSpdyFrameChunkSize);

  // |send_window_size_| should have been at least |delta_window_size| for
  // this call to happen.
  DCHECK_GE(send_window_size_, delta_window_size);

  LOG(INFO) << "Decreasing stream " << stream_id_
            << " send_window_size_ [current:" << send_window_size_ << "]"
            << " by " << delta_window_size  << " bytes";
  send_window_size_ -= delta_window_size;
}

base::Time SpdyStream::GetRequestTime() const {
  return request_time_;
}

void SpdyStream::SetRequestTime(base::Time t) {
  request_time_ = t;
}

int SpdyStream::OnResponseReceived(const spdy::SpdyHeaderBlock& response) {
  int rv = OK;
  LOG(INFO) << "Received response for stream " << stream_id_;

  metrics_.StartStream();

  DCHECK(response_->empty());
  *response_ = response;  // TODO(ukai): avoid copy.

  recv_first_byte_time_ = base::TimeTicks::Now();
  response_time_ = base::Time::Now();

  // If we receive a response before we are in STATE_WAITING_FOR_RESPONSE, then
  // the server has sent the SYN_REPLY too early.
  if (!pushed_ && io_state_ != STATE_WAITING_FOR_RESPONSE)
    return ERR_SPDY_PROTOCOL_ERROR;
  if (pushed_)
    CHECK(io_state_ == STATE_NONE);
  io_state_ = STATE_OPEN;

  if (delegate_)
    rv = delegate_->OnResponseReceived(*response_, response_time_, rv);
  // If delegate_ is not yet attached, we'll call OnResponseReceived after the
  // delegate gets attached to the stream.

  return rv;
}

void SpdyStream::OnDataReceived(const char* data, int length) {
  DCHECK_GE(length, 0);
  LOG(INFO) << "SpdyStream: Data (" << length << " bytes) received for "
            << stream_id_;

  if (!delegate_ || continue_buffering_data_) {
    // It should be valid for this to happen in the server push case.
    // We'll return received data when delegate gets attached to the stream.
    if (length > 0) {
      IOBufferWithSize* buf = new IOBufferWithSize(length);
      memcpy(buf->data(), data, length);
      pending_buffers_.push_back(buf);
    }
    else
      pending_buffers_.push_back(NULL);
    return;
  }

 CHECK(!closed());

  // If we don't have a response, then the SYN_REPLY did not come through.
  // We cannot pass data up to the caller unless the reply headers have been
  // received.
  if (!response_received()) {
    session_->CloseStream(stream_id_, ERR_SYN_REPLY_NOT_RECEIVED);
    return;
  }

  // A zero-length read means that the stream is being closed.
  if (!length) {
    metrics_.StopStream();
    scoped_refptr<SpdyStream> self(this);
    session_->CloseStream(stream_id_, net::OK);
    UpdateHistograms();
    return;
  }

  // Track our bandwidth.
  metrics_.RecordBytes(length);
  recv_bytes_ += length;
  recv_last_byte_time_ = base::TimeTicks::Now();

  if (!delegate_) {
    // It should be valid for this to happen in the server push case.
    // We'll return received data when delegate gets attached to the stream.
    IOBufferWithSize* buf = new IOBufferWithSize(length);
    memcpy(buf->data(), data, length);
    pending_buffers_.push_back(buf);
    return;
  }

  delegate_->OnDataReceived(data, length);
}

// This function is only called when an entire frame is written.
void SpdyStream::OnWriteComplete(int bytes) {
  DCHECK_LE(0, bytes);
  send_bytes_ += bytes;
  if (cancelled() || closed())
    return;
  DoLoop(bytes);
}

void SpdyStream::OnClose(int status) {
  io_state_ = STATE_DONE;
  response_status_ = status;
  Delegate* delegate = delegate_;
  delegate_ = NULL;
  if (delegate)
    delegate->OnClose(status);
}

void SpdyStream::Cancel() {
  if (cancelled())
    return;

  cancelled_ = true;
  if (session_->IsStreamActive(stream_id_))
    session_->ResetStream(stream_id_, spdy::CANCEL);
}

int SpdyStream::SendRequest(bool has_upload_data) {
  // Pushed streams do not send any data, and should always be in STATE_OPEN or
  // STATE_DONE. However, we still want to return IO_PENDING to mimic non-push
  // behavior.
  has_upload_data_ = has_upload_data;
  if (pushed_) {
    send_time_ = base::TimeTicks::Now();
    DCHECK(!has_upload_data_);
    DCHECK(response_received());
    return ERR_IO_PENDING;
  }
  CHECK_EQ(STATE_NONE, io_state_);
  io_state_ = STATE_SEND_HEADERS;
  return DoLoop(OK);
}

int SpdyStream::WriteStreamData(IOBuffer* data, int length,
                                spdy::SpdyDataFlags flags) {
  return session_->WriteStreamData(stream_id_, data, length, flags);
}

bool SpdyStream::GetSSLInfo(SSLInfo* ssl_info, bool* was_npn_negotiated) {
  return session_->GetSSLInfo(ssl_info, was_npn_negotiated);
}

int SpdyStream::DoLoop(int result) {
  do {
    State state = io_state_;
    io_state_ = STATE_NONE;
    switch (state) {
      // State machine 1: Send headers and body.
      case STATE_SEND_HEADERS:
        CHECK_EQ(OK, result);
        net_log_.BeginEvent(NetLog::TYPE_SPDY_STREAM_SEND_HEADERS, NULL);
        result = DoSendHeaders();
        break;
      case STATE_SEND_HEADERS_COMPLETE:
        net_log_.EndEvent(NetLog::TYPE_SPDY_STREAM_SEND_HEADERS, NULL);
        result = DoSendHeadersComplete(result);
        break;
      case STATE_SEND_BODY:
        CHECK_EQ(OK, result);
        net_log_.BeginEvent(NetLog::TYPE_SPDY_STREAM_SEND_BODY, NULL);
        result = DoSendBody();
        break;
      case STATE_SEND_BODY_COMPLETE:
        net_log_.EndEvent(NetLog::TYPE_SPDY_STREAM_SEND_BODY, NULL);
        result = DoSendBodyComplete(result);
        break;
      // This is an intermediary waiting state. This state is reached when all
      // data has been sent, but no data has been received.
      case STATE_WAITING_FOR_RESPONSE:
        io_state_ = STATE_WAITING_FOR_RESPONSE;
        result = ERR_IO_PENDING;
        break;
      // State machine 2: connection is established.
      // In STATE_OPEN, OnResponseReceived has already been called.
      // OnDataReceived, OnClose and OnWriteCompelte can be called.
      // Only OnWriteCompletee calls DoLoop(().
      //
      // For HTTP streams, no data is sent from the client while in the OPEN
      // state, so OnWriteComplete is never called here.  The HTTP body is
      // handled in the OnDataReceived callback, which does not call into
      // DoLoop.
      //
      // For WebSocket streams, which are bi-directional, we'll send and
      // receive data once the connection is established.  Received data is
      // handled in OnDataReceived.  Sent data is handled in OnWriteComplete,
      // which calls DoOpen().
      case STATE_OPEN:
        result = DoOpen(result);
        break;

      case STATE_DONE:
        DCHECK(result != ERR_IO_PENDING);
        break;
      default:
        NOTREACHED() << io_state_;
        break;
    }
  } while (result != ERR_IO_PENDING && io_state_ != STATE_NONE &&
           io_state_ != STATE_OPEN);

  return result;
}

int SpdyStream::DoSendHeaders() {
  CHECK(!cancelled_);

  spdy::SpdyControlFlags flags = spdy::CONTROL_FLAG_NONE;
  if (!has_upload_data_)
    flags = spdy::CONTROL_FLAG_FIN;

  CHECK(request_.get());
  int result = session_->WriteSynStream(
      stream_id_, static_cast<RequestPriority>(priority_), flags,
      request_);
  if (result != ERR_IO_PENDING)
    return result;

  send_time_ = base::TimeTicks::Now();
  io_state_ = STATE_SEND_HEADERS_COMPLETE;
  return ERR_IO_PENDING;
}

int SpdyStream::DoSendHeadersComplete(int result) {
  if (result < 0)
    return result;

  CHECK_GT(result, 0);

  if (!delegate_)
    return ERR_UNEXPECTED;

  // There is no body, skip that state.
  if (delegate_->OnSendHeadersComplete(result)) {
    io_state_ = STATE_WAITING_FOR_RESPONSE;
    return OK;
  }

  io_state_ = STATE_SEND_BODY;
  return OK;
}

// DoSendBody is called to send the optional body for the request.  This call
// will also be called as each write of a chunk of the body completes.
int SpdyStream::DoSendBody() {
  // If we're already in the STATE_SENDING_BODY state, then we've already
  // sent a portion of the body.  In that case, we need to first consume
  // the bytes written in the body stream.  Note that the bytes written is
  // the number of bytes in the frame that were written, only consume the
  // data portion, of course.
  io_state_ = STATE_SEND_BODY_COMPLETE;
  if (!delegate_)
    return ERR_UNEXPECTED;
  return delegate_->OnSendBody();
}

int SpdyStream::DoSendBodyComplete(int result) {
  if (result < 0)
    return result;

  CHECK_NE(result, 0);

  if (!delegate_)
    return ERR_UNEXPECTED;

  if (!delegate_->OnSendBodyComplete(result))
    io_state_ = STATE_SEND_BODY;
  else
    io_state_ = STATE_WAITING_FOR_RESPONSE;

  return OK;
}

int SpdyStream::DoOpen(int result) {
  if (delegate_)
    delegate_->OnDataSent(result);
  io_state_ = STATE_OPEN;
  return result;
}

void SpdyStream::UpdateHistograms() {
  if (histograms_recorded_)
    return;

  histograms_recorded_ = true;

  // We need all timers to be filled in, otherwise metrics can be bogus.
  if (send_time_.is_null() || recv_first_byte_time_.is_null() ||
      recv_last_byte_time_.is_null())
    return;

  UMA_HISTOGRAM_TIMES("Net.SpdyStreamTimeToFirstByte",
      recv_first_byte_time_ - send_time_);
  UMA_HISTOGRAM_TIMES("Net.SpdyStreamDownloadTime",
      recv_last_byte_time_ - recv_first_byte_time_);
  UMA_HISTOGRAM_TIMES("Net.SpdyStreamTime",
      recv_last_byte_time_ - send_time_);

  UMA_HISTOGRAM_COUNTS("Net.SpdySendBytes", send_bytes_);
  UMA_HISTOGRAM_COUNTS("Net.SpdyRecvBytes", recv_bytes_);
}

}  // namespace net
