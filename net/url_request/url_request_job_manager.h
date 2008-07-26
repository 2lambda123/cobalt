// Copyright 2008, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//    * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//    * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef NET_URL_REQUEST_URL_REQUEST_JOB_MANAGER_H__
#define NET_URL_REQUEST_URL_REQUEST_JOB_MANAGER_H__

#include <map>

#include "base/lock.h"
#include "net/url_request/url_request.h"

// This class is responsible for managing the set of protocol factories and
// request interceptors that determine how an URLRequestJob gets created to
// handle an URLRequest.
//
// MULTI-THREADING NOTICE:
//   URLRequest is designed to have all consumers on a single thread, and so no
//   attempt is made to support ProtocolFactory or Interceptor instances being
//   registered/unregistered or in any way poked on multiple threads.  However,
//   we do support checking for supported schemes FROM ANY THREAD (i.e., it is
//   safe to call SupportsScheme on any thread).
//
class URLRequestJobManager {
 public:
  URLRequestJobManager();

  // Instantiate an URLRequestJob implementation based on the registered
  // interceptors and protocol factories.  This will always succeed in
  // returning a job unless we are--in the extreme case--out of memory.
  URLRequestJob* CreateJob(URLRequest* request) const;

  // Returns true if there is a protocol factory registered for the given
  // scheme.  Note: also returns true if there is a built-in handler for the
  // given scheme.
  bool SupportsScheme(const std::string& scheme) const;

  // Register a protocol factory associated with the given scheme.  The factory
  // parameter may be null to clear any existing association.  Returns the
  // previously registered protocol factory if any.
  URLRequest::ProtocolFactory* RegisterProtocolFactory(
      const std::string& scheme, URLRequest::ProtocolFactory* factory);

  // Register/unregister a request interceptor.
  void RegisterRequestInterceptor(URLRequest::Interceptor* interceptor);
  void UnregisterRequestInterceptor(URLRequest::Interceptor* interceptor);

 private:
  typedef std::map<std::string,URLRequest::ProtocolFactory*> FactoryMap;
  typedef std::vector<URLRequest::Interceptor*> InterceptorList;

  mutable Lock lock_;
  FactoryMap factories_;
  InterceptorList interceptors_;

#ifndef NDEBUG
  // We use this to assert that CreateJob and the registration functions all
  // run on the same thread.
  mutable HANDLE allowed_thread_;

  // The first guy to call this function sets the allowed thread.  This way we
  // avoid needing to define that thread externally.  Since we expect all
  // callers to be on the same thread, we don't worry about threads racing to
  // set the allowed thread.
  bool IsAllowedThread() const {
    if (!allowed_thread_)
      allowed_thread_ = GetCurrentThread();
    return allowed_thread_ == GetCurrentThread();
  }
#endif

  DISALLOW_EVIL_CONSTRUCTORS(URLRequestJobManager);
};

#endif  // NET_URL_REQUEST_URL_REQUEST_JOB_MANAGER_H__
