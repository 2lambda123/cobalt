// Copyright 2015 Google Inc. All Rights Reserved.
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

#include "starboard/socket.h"

#include <errno.h>
#include <sys/socket.h>

#include "starboard/log.h"
#include "starboard/shared/posix/handle_eintr.h"
#include "starboard/shared/posix/socket_internal.h"

namespace sbposix = starboard::shared::posix;

SbSocketError SbSocketBind(SbSocket socket,
                           const SbSocketAddress* local_address) {
  if (!SbSocketIsValid(socket)) {
    errno = EBADF;
    return kSbSocketErrorFailed;
  }

  sbposix::SockAddr sock_addr;
  if (!sock_addr.FromSbSocketAddress(local_address)) {
    SB_DLOG(ERROR) << __FUNCTION__ << ": Invalid address";
    return (socket->error = sbposix::TranslateSocketErrno(EINVAL));
  }

  SB_DCHECK(socket->socket_fd >= 0);
  if (local_address->type != socket->address_type) {
    SB_DLOG(ERROR) << __FUNCTION__ << ": Incompatible addresses: "
                   << "socket type = " << socket->address_type
                   << ", argument type = " << local_address->type;
    return (socket->error = sbposix::TranslateSocketErrno(EAFNOSUPPORT));
  }

  int result = HANDLE_EINTR(
      bind(socket->socket_fd, sock_addr.sockaddr(), sock_addr.length));
  if (result != 0) {
    return (socket->error = sbposix::TranslateSocketErrno(errno));
  }

  return (socket->error = kSbSocketOk);
}
