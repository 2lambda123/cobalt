// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/base/tcp_listen_socket.h"

#if defined(OS_WIN)
// winsock2.h must be included first in order to ensure it is included before
// windows.h.
#include <winsock2.h>
#elif defined(OS_POSIX)
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "net/base/net_errors.h"
#endif

#include "base/logging.h"
#include "base/sys_byteorder.h"
#include "base/threading/platform_thread.h"
#include "build/build_config.h"
#include "net/base/net_util.h"
#include "net/base/winsock_init.h"

#if defined(__LB_SHELL__)
#include "lb_network_helpers.h"
#endif

using std::string;

namespace net {

// static
scoped_refptr<TCPListenSocket> TCPListenSocket::CreateAndListen(
    const string& ip, int port, StreamListenSocket::Delegate* del) {
  SocketDescriptor s = CreateAndBind(ip, port);
  if (s == kInvalidSocket)
    return NULL;
  scoped_refptr<TCPListenSocket> sock(new TCPListenSocket(s, del));
  sock->Listen();
  return sock;
}

TCPListenSocket::TCPListenSocket(SocketDescriptor s,
                                 StreamListenSocket::Delegate* del)
    : StreamListenSocket(s, del) {
}

TCPListenSocket::~TCPListenSocket() {}

SocketDescriptor TCPListenSocket::CreateAndBind(const string& ip, int port) {
#if defined(OS_WIN)
  EnsureWinsockInit();
#endif

  SocketDescriptor s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (s != kInvalidSocket) {
#if defined(OS_POSIX)
    // Allow rapid reuse.
    static const int kOn = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &kOn, sizeof(kOn));
#endif
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    addr.sin_port = base::HostToNet16(port);
    if (bind(s, reinterpret_cast<sockaddr*>(&addr), sizeof(addr))) {
#if defined(OS_WIN)
      closesocket(s);
#elif defined(__LB_SHELL__)
      LB::Platform::close_socket(s);
#elif defined(OS_POSIX)
      close(s);
#endif
      LOG(ERROR) << "Could not bind socket to " << ip << ":" << port;
      s = kInvalidSocket;
    }
  }
  return s;
}

SocketDescriptor TCPListenSocket::CreateAndBindAnyPort(const string& ip,
                                                       int* port) {
  SocketDescriptor s = CreateAndBind(ip, 0);
  if (s == kInvalidSocket)
    return kInvalidSocket;
  sockaddr_in addr;
  socklen_t addr_size = sizeof(addr);
  bool failed = getsockname(s, reinterpret_cast<struct sockaddr*>(&addr),
                            &addr_size) != 0;
  if (addr_size != sizeof(addr))
    failed = true;
  if (failed) {
    LOG(ERROR) << "Could not determine bound port, getsockname() failed";
#if defined(OS_WIN)
    closesocket(s);
#elif defined(__LB_SHELL__)
    LB::Platform::close_socket(s);
#elif defined(OS_POSIX)
    close(s);
#endif
    return kInvalidSocket;
  }
  *port = base::NetToHost16(addr.sin_port);
  return s;
}

void TCPListenSocket::Accept() {
  SocketDescriptor conn = AcceptSocket();
  if (conn == kInvalidSocket)
    return;
  scoped_refptr<TCPListenSocket> sock(
      new TCPListenSocket(conn, socket_delegate_));
  // It's up to the delegate to AddRef if it wants to keep it around.
#if defined(OS_POSIX)
  sock->WatchSocket(WAITING_READ);
#endif
  socket_delegate_->DidAccept(this, sock);
}

TCPListenSocketFactory::TCPListenSocketFactory(const string& ip, int port)
    : ip_(ip),
      port_(port) {
}

TCPListenSocketFactory::~TCPListenSocketFactory() {}

scoped_refptr<StreamListenSocket> TCPListenSocketFactory::CreateAndListen(
    StreamListenSocket::Delegate* delegate) const {
  return TCPListenSocket::CreateAndListen(ip_, port_, delegate);
}

}  // namespace net
