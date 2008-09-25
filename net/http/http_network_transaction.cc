// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/http/http_network_transaction.h"

#include "base/compiler_specific.h"
#include "base/string_util.h"
#include "base/trace_event.h"
#include "build/build_config.h"
#include "net/base/client_socket_factory.h"
#include "net/base/host_resolver.h"
#include "net/base/load_flags.h"
#if defined(OS_WIN)
#include "net/base/ssl_client_socket.h"
#endif
#include "net/base/upload_data_stream.h"
#include "net/http/http_chunked_decoder.h"
#include "net/http/http_network_session.h"
#include "net/http/http_request_info.h"
#include "net/http/http_util.h"

// TODO(darin):
//  - authentication
//  - proxies (need to call ReconsiderProxyAfterError and handle SSL tunnel)
//  - ssl

namespace net {

//-----------------------------------------------------------------------------

HttpNetworkTransaction::HttpNetworkTransaction(HttpNetworkSession* session,
                                               ClientSocketFactory* csf)
    : ALLOW_THIS_IN_INITIALIZER_LIST(
          io_callback_(this, &HttpNetworkTransaction::OnIOComplete)),
      user_callback_(NULL),
      session_(session),
      request_(NULL),
      pac_request_(NULL),
      socket_factory_(csf),
      connection_(session->connection_pool()),
      reused_socket_(false),
      using_ssl_(false),
      using_proxy_(false),
      using_tunnel_(false),
      establishing_tunnel_(false),
      request_headers_bytes_sent_(0),
      header_buf_capacity_(0),
      header_buf_len_(0),
      header_buf_body_offset_(-1),
      header_buf_http_offset_(-1),
      content_length_(-1),  // -1 means unspecified.
      content_read_(0),
      read_buf_(NULL),
      read_buf_len_(0),
      next_state_(STATE_NONE) {
}

void HttpNetworkTransaction::Destroy() {
  delete this;
}

int HttpNetworkTransaction::Start(const HttpRequestInfo* request_info,
                                  CompletionCallback* callback) {
  request_ = request_info;

  next_state_ = STATE_RESOLVE_PROXY;
  int rv = DoLoop(OK);
  if (rv == ERR_IO_PENDING)
    user_callback_ = callback;
  return rv;
}

int HttpNetworkTransaction::RestartIgnoringLastError(
    CompletionCallback* callback) {
  // TODO(wtc): If the connection is no longer alive, call
  // connection_.socket()->ReconnectIgnoringLastError().
  next_state_ = STATE_WRITE_HEADERS;
  int rv = DoLoop(OK);
  if (rv == ERR_IO_PENDING)
    user_callback_ = callback;
  return rv; 
}

int HttpNetworkTransaction::RestartWithAuth(
    const std::wstring& username,
    const std::wstring& password,
    CompletionCallback* callback) {
  return ERR_FAILED;  // TODO(darin): implement me!
}

int HttpNetworkTransaction::Read(char* buf, int buf_len,
                                 CompletionCallback* callback) {
  DCHECK(response_.headers);
  DCHECK(buf);
  DCHECK(buf_len > 0);

  if (!connection_.is_initialized())
    return 0;  // connection_ has been reset.  Treat like EOF.

  read_buf_ = buf;
  read_buf_len_ = buf_len;

  next_state_ = STATE_READ_BODY;
  int rv = DoLoop(OK);
  if (rv == ERR_IO_PENDING)
    user_callback_ = callback;
  return rv;
}

const HttpResponseInfo* HttpNetworkTransaction::GetResponseInfo() const {
  return (response_.headers || response_.ssl_info.cert) ? &response_ : NULL;
}

LoadState HttpNetworkTransaction::GetLoadState() const {
  // TODO(wtc): Define a new LoadState value for the
  // STATE_INIT_CONNECTION_COMPLETE state, which delays the HTTP request.
  switch (next_state_) {
    case STATE_RESOLVE_PROXY_COMPLETE:
      return LOAD_STATE_RESOLVING_PROXY_FOR_URL;
    case STATE_RESOLVE_HOST_COMPLETE:
      return LOAD_STATE_RESOLVING_HOST;
    case STATE_CONNECT_COMPLETE:
      return LOAD_STATE_CONNECTING;
    case STATE_WRITE_HEADERS_COMPLETE:
    case STATE_WRITE_BODY_COMPLETE:
      return LOAD_STATE_SENDING_REQUEST;
    case STATE_READ_HEADERS_COMPLETE:
      return LOAD_STATE_WAITING_FOR_RESPONSE;
    case STATE_READ_BODY_COMPLETE:
      return LOAD_STATE_READING_RESPONSE;
    default:
      return LOAD_STATE_IDLE;
  }
}

uint64 HttpNetworkTransaction::GetUploadProgress() const {
  if (!request_body_stream_.get())
    return 0;

  return request_body_stream_->position();
}

HttpNetworkTransaction::~HttpNetworkTransaction() {
  // If we still have an open socket, then make sure to close it so we don't
  // try to reuse it later on.
  if (connection_.is_initialized())
    connection_.set_socket(NULL);

  if (pac_request_)
    session_->proxy_service()->CancelPacRequest(pac_request_);
}

void HttpNetworkTransaction::BuildRequestHeaders() {
  // For proxy use the full url. Otherwise just the absolute path.
  // This strips out any reference/username/password.
  std::string path = using_proxy_ ?
      HttpUtil::SpecForRequest(request_->url) :
      HttpUtil::PathForRequest(request_->url);

  request_headers_ = request_->method + " " + path +
      " HTTP/1.1\r\nHost: " + request_->url.host();
  if (request_->url.IntPort() != -1)
    request_headers_ += ":" + request_->url.port();
  request_headers_ += "\r\n";

  // For compat with HTTP/1.0 servers and proxies:
  if (using_proxy_)
    request_headers_ += "Proxy-";
  request_headers_ += "Connection: keep-alive\r\n";

  if (!request_->user_agent.empty())
    request_headers_ += "User-Agent: " + request_->user_agent + "\r\n";

  // Our consumer should have made sure that this is a safe referrer.  See for
  // instance WebCore::FrameLoader::HideReferrer.
  if (request_->referrer.is_valid())
    request_headers_ += "Referer: " + request_->referrer.spec() + "\r\n";

  // Add a content length header?
  if (request_->upload_data) {
    request_body_stream_.reset(new UploadDataStream(request_->upload_data));
    request_headers_ +=
        "Content-Length: " + Uint64ToString(request_body_stream_->size()) +
        "\r\n";
  } else if (request_->method == "POST" || request_->method == "PUT" ||
             request_->method == "HEAD") {
    // An empty POST/PUT request still needs a content length.  As for HEAD,
    // IE and Safari also add a content length header.  Presumably it is to
    // support sending a HEAD request to an URL that only expects to be sent a
    // POST or some other method that normally would have a message body.
    request_headers_ += "Content-Length: 0\r\n";
  }

  // Honor load flags that impact proxy caches.
  if (request_->load_flags & LOAD_BYPASS_CACHE) {
    request_headers_ += "Pragma: no-cache\r\nCache-Control: no-cache\r\n";
  } else if (request_->load_flags & LOAD_VALIDATE_CACHE) {
    request_headers_ += "Cache-Control: max-age=0\r\n";
  }

  // TODO(darin): Need to prune out duplicate headers.

  request_headers_ += request_->extra_headers;
  request_headers_ += "\r\n";
}

// The HTTP CONNECT method for establishing a tunnel connection is documented
// in draft-luotonen-web-proxy-tunneling-01.txt and RFC 2817, Sections 5.2 and
// 5.3.
void HttpNetworkTransaction::BuildTunnelRequest() {
  std::string port;
  if (request_->url.has_port()) {
    port = request_->url.port();
  } else {
    port = "443";
  }

  // RFC 2616 Section 9 says the Host request-header field MUST accompany all
  // HTTP/1.1 requests.
  request_headers_ = "CONNECT " + request_->url.host() + ":" + port +
      " HTTP/1.1\r\nHost: " + request_->url.host();
  if (request_->url.IntPort() != -1)
    request_headers_ += ":" + request_->url.port();
  request_headers_ += "\r\n";

  if (!request_->user_agent.empty())
    request_headers_ += "User-Agent: " + request_->user_agent + "\r\n";

  // TODO(wtc): Add the Proxy-Authorization header, if necessary, to handle
  // proxy authentication.

  request_headers_ += "\r\n";
}

void HttpNetworkTransaction::DoCallback(int rv) {
  DCHECK(rv != ERR_IO_PENDING);
  DCHECK(user_callback_);

  // Since Run may result in Read being called, clear user_callback_ up front.
  CompletionCallback* c = user_callback_;
  user_callback_ = NULL;
  c->Run(rv);
}

void HttpNetworkTransaction::OnIOComplete(int result) {
  int rv = DoLoop(result);
  if (rv != ERR_IO_PENDING)
    DoCallback(rv);
}

int HttpNetworkTransaction::DoLoop(int result) {
  DCHECK(next_state_ != STATE_NONE);

  int rv = result;
  do {
    State state = next_state_;
    next_state_ = STATE_NONE;
    switch (state) {
      case STATE_RESOLVE_PROXY:
        DCHECK(rv == OK);
        TRACE_EVENT_BEGIN("http.resolve_proxy", request_, request_->url.spec());
        rv = DoResolveProxy();
        break;
      case STATE_RESOLVE_PROXY_COMPLETE:
        rv = DoResolveProxyComplete(rv);
        TRACE_EVENT_END("http.resolve_proxy", request_, request_->url.spec());
        break;
      case STATE_INIT_CONNECTION:
        DCHECK(rv == OK);
        TRACE_EVENT_BEGIN("http.init_conn", request_, request_->url.spec());
        rv = DoInitConnection();
        break;
      case STATE_INIT_CONNECTION_COMPLETE:
        rv = DoInitConnectionComplete(rv);
        TRACE_EVENT_END("http.init_conn", request_, request_->url.spec());
        break;
      case STATE_RESOLVE_HOST:
        DCHECK(rv == OK);
        TRACE_EVENT_BEGIN("http.resolve_host", request_, request_->url.spec());
        rv = DoResolveHost();
        break;
      case STATE_RESOLVE_HOST_COMPLETE:
        rv = DoResolveHostComplete(rv);
        TRACE_EVENT_END("http.resolve_host", request_, request_->url.spec());
        break;
      case STATE_CONNECT:
        DCHECK(rv == OK);
        TRACE_EVENT_BEGIN("http.connect", request_, request_->url.spec());
        rv = DoConnect();
        break;
      case STATE_CONNECT_COMPLETE:
        rv = DoConnectComplete(rv);
        TRACE_EVENT_END("http.connect", request_, request_->url.spec());
        break;
      case STATE_SSL_CONNECT_OVER_TUNNEL:
        DCHECK(rv == OK);
        TRACE_EVENT_BEGIN("http.ssl_tunnel", request_, request_->url.spec());
        rv = DoSSLConnectOverTunnel();
        break;
      case STATE_SSL_CONNECT_OVER_TUNNEL_COMPLETE:
        rv = DoSSLConnectOverTunnelComplete(rv);
        TRACE_EVENT_END("http.ssl_tunnel", request_, request_->url.spec());
        break;
      case STATE_WRITE_HEADERS:
        DCHECK(rv == OK);
        TRACE_EVENT_BEGIN("http.write_headers", request_, request_->url.spec());
        rv = DoWriteHeaders();
        break;
      case STATE_WRITE_HEADERS_COMPLETE:
        rv = DoWriteHeadersComplete(rv);
        TRACE_EVENT_END("http.write_headers", request_, request_->url.spec());
        break;
      case STATE_WRITE_BODY:
        DCHECK(rv == OK);
        TRACE_EVENT_BEGIN("http.write_body", request_, request_->url.spec());
        rv = DoWriteBody();
        break;
      case STATE_WRITE_BODY_COMPLETE:
        rv = DoWriteBodyComplete(rv);
        TRACE_EVENT_END("http.write_body", request_, request_->url.spec());
        break;
      case STATE_READ_HEADERS:
        DCHECK(rv == OK);
        TRACE_EVENT_BEGIN("http.read_headers", request_, request_->url.spec());
        rv = DoReadHeaders();
        break;
      case STATE_READ_HEADERS_COMPLETE:
        rv = DoReadHeadersComplete(rv);
        TRACE_EVENT_END("http.read_headers", request_, request_->url.spec());
        break;
      case STATE_READ_BODY:
        DCHECK(rv == OK);
        TRACE_EVENT_BEGIN("http.read_body", request_, request_->url.spec());
        rv = DoReadBody();
        break;
      case STATE_READ_BODY_COMPLETE:
        rv = DoReadBodyComplete(rv);
        TRACE_EVENT_END("http.read_body", request_, request_->url.spec());
        break;
      default:
        NOTREACHED() << "bad state";
        rv = ERR_FAILED;
        break;
    }
  } while (rv != ERR_IO_PENDING && next_state_ != STATE_NONE);

  return rv;
}

int HttpNetworkTransaction::DoResolveProxy() {
  DCHECK(!pac_request_);

  next_state_ = STATE_RESOLVE_PROXY_COMPLETE;

  return session_->proxy_service()->ResolveProxy(
      request_->url, &proxy_info_, &io_callback_, &pac_request_);
}

int HttpNetworkTransaction::DoResolveProxyComplete(int result) {
  next_state_ = STATE_INIT_CONNECTION;

  pac_request_ = NULL;

  if (result != OK) {
    DLOG(ERROR) << "Failed to resolve proxy: " << result;
    proxy_info_.UseDirect();
  }
  return OK;
}

int HttpNetworkTransaction::DoInitConnection() {
  DCHECK(!connection_.is_initialized());

  next_state_ = STATE_INIT_CONNECTION_COMPLETE;

  using_ssl_ = request_->url.SchemeIs("https");
  using_proxy_ = !proxy_info_.is_direct() && !using_ssl_;
  using_tunnel_ = !proxy_info_.is_direct() && using_ssl_;

  // Build the string used to uniquely identify connections of this type.
  std::string connection_group;
  if (using_proxy_ || using_tunnel_)
    connection_group = "proxy/" + proxy_info_.proxy_server() + "/";
  if (!using_proxy_)
    connection_group.append(request_->url.GetOrigin().spec());

  DCHECK(!connection_group.empty());
  return connection_.Init(connection_group, &io_callback_);
}

int HttpNetworkTransaction::DoInitConnectionComplete(int result) {
  if (result < 0)
    return result;

  DCHECK(connection_.is_initialized());

  // Set the reused_socket_ flag to indicate that we are using a keep-alive
  // connection.  This flag is used to handle errors that occur while we are
  // trying to reuse a keep-alive connection.
  if (reused_socket_ = (connection_.socket() != NULL)) {
    next_state_ = STATE_WRITE_HEADERS;
  } else {
    next_state_ = STATE_RESOLVE_HOST;
  }
  return OK;
}

int HttpNetworkTransaction::DoResolveHost() {
  next_state_ = STATE_RESOLVE_HOST_COMPLETE;

  std::string host;
  int port;

  // Determine the host and port to connect to.
  if (using_proxy_ || using_tunnel_) {
    const std::string& proxy = proxy_info_.proxy_server();
    StringTokenizer t(proxy, ":");
    // TODO(darin): Handle errors here.  Perhaps HttpProxyInfo should do this
    // before claiming a proxy server configuration.
    t.GetNext();
    host = t.token();
    t.GetNext();
    port = static_cast<int>(StringToInt64(t.token()));
  } else {
    // Direct connection
    host = request_->url.host();
    port = request_->url.IntPort();
    if (port == url_parse::PORT_UNSPECIFIED) {
      if (using_ssl_) {
        port = 443;  // Default HTTPS port
      } else {
        port = 80;   // Default HTTP port
      }
    }
  }

  return resolver_.Resolve(host, port, &addresses_, &io_callback_);
}

int HttpNetworkTransaction::DoResolveHostComplete(int result) {
  if (result == OK)
    next_state_ = STATE_CONNECT;
  return result;
}

int HttpNetworkTransaction::DoConnect() {
  next_state_ = STATE_CONNECT_COMPLETE;

  DCHECK(!connection_.socket());

  ClientSocket* s = socket_factory_->CreateTCPClientSocket(addresses_);

  // If we are using a direct SSL connection, then go ahead and create the SSL
  // wrapper socket now.  Otherwise, we need to first issue a CONNECT request.
  if (using_ssl_ && !using_tunnel_)
    s = socket_factory_->CreateSSLClientSocket(s, request_->url.host());

  connection_.set_socket(s);
  return connection_.socket()->Connect(&io_callback_);
}

int HttpNetworkTransaction::DoConnectComplete(int result) {
  if (result == OK) {
    next_state_ = STATE_WRITE_HEADERS;
    if (using_tunnel_)
      establishing_tunnel_ = true;
  } else if (IsCertificateError(result)) {
    result = HandleCertificateError(result);
  }
  return result;
}

int HttpNetworkTransaction::DoSSLConnectOverTunnel() {
  next_state_ = STATE_SSL_CONNECT_OVER_TUNNEL_COMPLETE;

  ClientSocket* s = connection_.release_socket();
  s = socket_factory_->CreateSSLClientSocket(s, request_->url.host());
  connection_.set_socket(s);
  return connection_.socket()->Connect(&io_callback_);
}

int HttpNetworkTransaction::DoSSLConnectOverTunnelComplete(int result) {
  if (result == OK) {
    next_state_ = STATE_WRITE_HEADERS;
  } else if (IsCertificateError(result)) {
    result = HandleCertificateError(result);
  }
  return result;
}

int HttpNetworkTransaction::DoWriteHeaders() {
  next_state_ = STATE_WRITE_HEADERS_COMPLETE;

  // This is constructed lazily (instead of within our Start method), so that
  // we have proxy info available.
  if (request_headers_.empty()) {
    if (establishing_tunnel_) {
      BuildTunnelRequest();
    } else {
      BuildRequestHeaders();
    }
  }

  // Record our best estimate of the 'request time' as the time when we send
  // out the first bytes of the request headers.
  if (request_headers_bytes_sent_ == 0)
    response_.request_time = Time::Now();

  const char* buf = request_headers_.data() + request_headers_bytes_sent_;
  int buf_len = static_cast<int>(request_headers_.size() -
                                 request_headers_bytes_sent_);
  DCHECK(buf_len > 0);

  return connection_.socket()->Write(buf, buf_len, &io_callback_);
}

int HttpNetworkTransaction::DoWriteHeadersComplete(int result) {
  if (result < 0)
    return HandleIOError(result);

  request_headers_bytes_sent_ += result;
  if (request_headers_bytes_sent_ < request_headers_.size()) {
    next_state_ = STATE_WRITE_HEADERS;
  } else if (!establishing_tunnel_ && request_->upload_data) {
    next_state_ = STATE_WRITE_BODY;
  } else {
    next_state_ = STATE_READ_HEADERS;
  }
  return OK;
}

int HttpNetworkTransaction::DoWriteBody() {
  next_state_ = STATE_WRITE_BODY_COMPLETE;

  DCHECK(request_->upload_data);
  DCHECK(request_body_stream_.get());

  const char* buf = request_body_stream_->buf();
  int buf_len = static_cast<int>(request_body_stream_->buf_len());

  return connection_.socket()->Write(buf, buf_len, &io_callback_);
}

int HttpNetworkTransaction::DoWriteBodyComplete(int result) {
  if (result < 0)
    return HandleIOError(result);

  request_body_stream_->DidConsume(result);

  if (request_body_stream_->position() < request_body_stream_->size()) {
    next_state_ = STATE_WRITE_BODY;
  } else {
    next_state_ = STATE_READ_HEADERS;
  }
  return OK;
}

int HttpNetworkTransaction::DoReadHeaders() {
  next_state_ = STATE_READ_HEADERS_COMPLETE;

  // Grow the read buffer if necessary.
  if (header_buf_len_ == header_buf_capacity_) {
    header_buf_capacity_ += kHeaderBufInitialSize;
    header_buf_.reset(static_cast<char*>(
        realloc(header_buf_.release(), header_buf_capacity_)));
  }

  char* buf = header_buf_.get() + header_buf_len_;
  int buf_len = header_buf_capacity_ - header_buf_len_;

  return connection_.socket()->Read(buf, buf_len, &io_callback_);
}

int HttpNetworkTransaction::DoReadHeadersComplete(int result) {
  if (result < 0)
    return HandleIOError(result);

  // Record our best estimate of the 'response time' as the time when we read
  // the first bytes of the response headers.
  if (header_buf_len_ == 0)
    response_.response_time = Time::Now();

  // The socket was closed before we found end-of-headers.
  if (result == 0) {
    if (establishing_tunnel_) {
      // The socket was closed before the tunnel could be established.
      return ERR_TUNNEL_CONNECTION_FAILED;
    }
    if (has_found_status_line_start()) {
      // Assume EOF is end-of-headers.
      header_buf_body_offset_ = header_buf_len_;
    } else {
      // No status line was matched yet, assume HTTP/0.9
      // (this will also match a HTTP/1.x that got closed early).
      header_buf_body_offset_ = 0;
    }
  } else {
    header_buf_len_ += result;
    DCHECK(header_buf_len_ <= header_buf_capacity_);

    // Look for the start of the status line, if it hasn't been found yet.
    if (!has_found_status_line_start()) {
      header_buf_http_offset_ = HttpUtil::LocateStartOfStatusLine(
          header_buf_.get(), header_buf_len_);
    }

    if (has_found_status_line_start()) {
      int eoh = HttpUtil::LocateEndOfHeaders(
          header_buf_.get(), header_buf_len_, header_buf_http_offset_);
      if (eoh == -1) {
        // Haven't found the end of headers yet, keep reading.
        next_state_ = STATE_READ_HEADERS;
        return OK;
      }
      header_buf_body_offset_ = eoh;
    } else if (header_buf_len_ < 8) {
      // Not enough data to decide whether this is HTTP/0.9 yet.
      // 8 bytes = (4 bytes of junk) + "http".length()
      next_state_ = STATE_READ_HEADERS;
      return OK;
    } else {
      // Enough data was read -- there is no status line.
      header_buf_body_offset_ = 0;
    }
  }

  // And, we are done with the Start or the SSL tunnel CONNECT sequence.
  return DidReadResponseHeaders();
}

int HttpNetworkTransaction::DoReadBody() {
  DCHECK(read_buf_);
  DCHECK(read_buf_len_ > 0);
  DCHECK(connection_.is_initialized());

  next_state_ = STATE_READ_BODY_COMPLETE;

  // We may have already consumed the indicated content length.
  if (content_length_ != -1 && content_read_ >= content_length_)
    return 0;

  // We may have some data remaining in the header buffer.
  if (header_buf_.get() && header_buf_body_offset_ < header_buf_len_) {
    int n = std::min(read_buf_len_, header_buf_len_ - header_buf_body_offset_);
    memcpy(read_buf_, header_buf_.get() + header_buf_body_offset_, n);
    header_buf_body_offset_ += n;
    if (header_buf_body_offset_ == header_buf_len_) {
      header_buf_.reset();
      header_buf_capacity_ = 0;
      header_buf_len_ = 0;
      header_buf_body_offset_ = -1;
    }
    return n;
  }

  return connection_.socket()->Read(read_buf_, read_buf_len_, &io_callback_);
}

int HttpNetworkTransaction::DoReadBodyComplete(int result) {
  // We are done with the Read call.

  bool unfiltered_eof = (result == 0);

  // Filter incoming data if appropriate.  FilterBuf may return an error.
  if (result > 0 && chunked_decoder_.get()) {
    result = chunked_decoder_->FilterBuf(read_buf_, result);
    if (result == 0 && !chunked_decoder_->reached_eof()) {
      // Don't signal completion of the Read call yet or else it'll look like
      // we received end-of-file.  Wait for more data.
      next_state_ = STATE_READ_BODY;
      return OK;
    }
  }

  bool done = false, keep_alive = false;
  if (result < 0) {
    // Error while reading the socket.
    done = true;
  } else {
    content_read_ += result;
    if (unfiltered_eof ||
        (content_length_ != -1 && content_read_ >= content_length_) ||
        (chunked_decoder_.get() && chunked_decoder_->reached_eof())) {
      done = true;
      keep_alive = response_.headers->IsKeepAlive();
      // We can't reuse the connection if we read more than the advertised
      // content length.
      if (unfiltered_eof ||
          (content_length_ != -1 && content_read_ > content_length_))
        keep_alive = false;
    }
  }

  // Clean up the HttpConnection if we are done.
  if (done) {
    if (!keep_alive)
      connection_.set_socket(NULL);
    connection_.Reset();
    // The next Read call will return 0 (EOF).
  }

  // Clear these to avoid leaving around old state.
  read_buf_ = NULL;
  read_buf_len_ = 0;

  return result;
}

int HttpNetworkTransaction::DidReadResponseHeaders() {
  scoped_refptr<HttpResponseHeaders> headers;
  if (has_found_status_line_start()) {
    headers = new HttpResponseHeaders(
        HttpUtil::AssembleRawHeaders(
            header_buf_.get(), header_buf_body_offset_));
  } else {
    // Fabricate a status line to to preserve the HTTP/0.9 version.
    // (otherwise HttpResponseHeaders will default it to HTTP/1.0).
    headers = new HttpResponseHeaders(std::string("HTTP/0.9 200 OK"));
  }

  if (headers->GetParsedHttpVersion() < HttpVersion(1, 0)) {
    // Require the "HTTP/1.x" status line for SSL CONNECT.
    if (establishing_tunnel_)
      return ERR_TUNNEL_CONNECTION_FAILED;

    // HTTP/0.9 doesn't support the PUT method, so lack of response headers
    // indicates a buggy server.  See:
    // https://bugzilla.mozilla.org/show_bug.cgi?id=193921
    if (request_->method == "PUT")
      return ERR_METHOD_NOT_SUPPORTED;
  }

  // Check for an intermediate 100 Continue response.  An origin server is
  // allowed to send this response even if we didn't ask for it, so we just
  // need to skip over it.
  if (headers->response_code() == 100) {
    header_buf_len_ -= header_buf_body_offset_;
    // If we've already received some bytes after the 100 Continue response,
    // move them to the beginning of header_buf_.
    if (header_buf_len_) {
      memmove(header_buf_.get(), header_buf_.get() + header_buf_body_offset_,
              header_buf_len_);
    }
    header_buf_body_offset_ = -1;
    next_state_ = STATE_READ_HEADERS;
    return OK;
  }

  if (establishing_tunnel_ && headers->response_code() == 200) {
    if (header_buf_body_offset_ != header_buf_len_) {
      // The proxy sent extraneous data after the headers.
      return ERR_TUNNEL_CONNECTION_FAILED;
    }
    next_state_ = STATE_SSL_CONNECT_OVER_TUNNEL;
    // Reset for the real request and response headers.
    request_headers_.clear();
    request_headers_bytes_sent_ = 0;
    header_buf_len_ = 0;
    header_buf_body_offset_ = 0;
    establishing_tunnel_ = false;
    return OK;
  }

  // TODO(wtc): Handle 401 server or 407 proxy authentication challenge.
  response_.headers = headers;
  response_.vary_data.Init(*request_, *response_.headers);

  // Figure how to determine EOF:

  // For certain responses, we know the content length is always 0.
  switch (response_.headers->response_code()) {
    case 204:  // No Content
    case 205:  // Reset Content
    case 304:  // Not Modified
      content_length_ = 0;
      break;
  }

  if (content_length_ == -1) {
    // Ignore spurious chunked responses from HTTP/1.0 servers and proxies.
    // Otherwise "Transfer-Encoding: chunked" trumps "Content-Length: N"
    if (response_.headers->GetHttpVersion() >= HttpVersion(1, 1) &&
        response_.headers->HasHeaderValue("Transfer-Encoding", "chunked")) {
      chunked_decoder_.reset(new HttpChunkedDecoder());
    } else {
      content_length_ = response_.headers->GetContentLength();
      // If content_length_ is still -1, then we have to wait for the server to
      // close the connection.
    }
  }

#if defined(OS_WIN)
  if (using_ssl_ && !establishing_tunnel_) {
    SSLClientSocket* ssl_socket =
        reinterpret_cast<SSLClientSocket*>(connection_.socket());
    ssl_socket->GetSSLInfo(&response_.ssl_info);
  }
#endif

  return OK;
}

int HttpNetworkTransaction::HandleCertificateError(int error) {
  DCHECK(using_ssl_);

  const int kCertFlags = LOAD_IGNORE_CERT_COMMON_NAME_INVALID |
                         LOAD_IGNORE_CERT_DATE_INVALID |
                         LOAD_IGNORE_CERT_AUTHORITY_INVALID |
                         LOAD_IGNORE_CERT_WRONG_USAGE;
  if (request_->load_flags & kCertFlags) {
    switch (error) {
      case ERR_CERT_COMMON_NAME_INVALID:
        if (request_->load_flags & LOAD_IGNORE_CERT_COMMON_NAME_INVALID)
          error = OK;
        break;
      case ERR_CERT_DATE_INVALID:
        if (request_->load_flags & LOAD_IGNORE_CERT_DATE_INVALID)
          error = OK;
        break;
      case ERR_CERT_AUTHORITY_INVALID:
        if (request_->load_flags & LOAD_IGNORE_CERT_AUTHORITY_INVALID)
          error = OK;
        break;
    }
  }

#if defined(OS_WIN)
  if (error != OK) {
    SSLClientSocket* ssl_socket =
        reinterpret_cast<SSLClientSocket*>(connection_.socket());
    ssl_socket->GetSSLInfo(&response_.ssl_info);
  }
#endif
  return error;
}

// This method determines whether it is safe to resend the request after an
// IO error.  It can only be called in response to request header or body
// write errors or response header read errors.  It should not be used in
// other cases, such as a Connect error.
int HttpNetworkTransaction::HandleIOError(int error) {
  switch (error) {
    // If we try to reuse a connection that the server is in the process of
    // closing, we may end up successfully writing out our request (or a
    // portion of our request) only to find a connection error when we try to
    // read from (or finish writing to) the socket.
    case ERR_CONNECTION_RESET:
    case ERR_CONNECTION_CLOSED:
    case ERR_CONNECTION_ABORTED:
      if (!establishing_tunnel_ &&
          reused_socket_ &&    // We reused a keep-alive connection.
          !header_buf_len_) {  // We haven't received any response header yet.
        connection_.set_socket(NULL);
        connection_.Reset();
        request_headers_bytes_sent_ = 0;
        if (request_body_stream_.get())
          request_body_stream_->Reset();
        next_state_ = STATE_INIT_CONNECTION;  // Resend the request.
        error = OK;
      }
      break;
  }
  return error;
}

}  // namespace net

