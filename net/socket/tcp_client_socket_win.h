// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_SOCKET_TCP_CLIENT_SOCKET_WIN_H_
#define NET_SOCKET_TCP_CLIENT_SOCKET_WIN_H_

#include <winsock2.h>

#include "base/object_watcher.h"
#include "net/base/address_list.h"
#include "net/base/completion_callback.h"
#include "net/base/net_log.h"
#include "net/socket/client_socket.h"

namespace net {

class BoundNetLog;

class TCPClientSocketWin : public ClientSocket {
 public:
  // The IP address(es) and port number to connect to.  The TCP socket will try
  // each IP address in the list until it succeeds in establishing a
  // connection.
  TCPClientSocketWin(const AddressList& addresses, net::NetLog* net_log);

  ~TCPClientSocketWin();

  // ClientSocket methods:
  virtual int Connect(CompletionCallback* callback);
  virtual void Disconnect();
  virtual bool IsConnected() const;
  virtual bool IsConnectedAndIdle() const;
  virtual int GetPeerAddress(AddressList* address) const;
  virtual const BoundNetLog& NetLog() const { return net_log_; }

  // Socket methods:
  // Multiple outstanding requests are not supported.
  // Full duplex mode (reading and writing at the same time) is supported
  virtual int Read(IOBuffer* buf, int buf_len, CompletionCallback* callback);
  virtual int Write(IOBuffer* buf, int buf_len, CompletionCallback* callback);

  virtual bool SetReceiveBufferSize(int32 size);
  virtual bool SetSendBufferSize(int32 size);

 private:
  class Core;

  // Performs the actual connect().  Returns a net error code.
  int DoConnect();

  int CreateSocket(const struct addrinfo* ai);
  void DoReadCallback(int rv);
  void DoWriteCallback(int rv);
  void DidCompleteConnect();
  void DidCompleteRead();
  void DidCompleteWrite();

  SOCKET socket_;

  // The list of addresses we should try in order to establish a connection.
  AddressList addresses_;

  // Where we are in above list, or NULL if all addrinfos have been tried.
  const struct addrinfo* current_ai_;

  // The various states that the socket could be in.
  bool waiting_connect_;
  bool waiting_read_;
  bool waiting_write_;

  // The core of the socket that can live longer than the socket itself. We pass
  // resources to the Windows async IO functions and we have to make sure that
  // they are not destroyed while the OS still references them.
  scoped_refptr<Core> core_;

  // External callback; called when connect or read is complete.
  CompletionCallback* read_callback_;

  // External callback; called when write is complete.
  CompletionCallback* write_callback_;

  BoundNetLog net_log_;

  DISALLOW_COPY_AND_ASSIGN(TCPClientSocketWin);
};

}  // namespace net

#endif  // NET_SOCKET_TCP_CLIENT_SOCKET_WIN_H_
