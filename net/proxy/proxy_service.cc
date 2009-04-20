// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/proxy/proxy_service.h"

#include <algorithm>

#include "base/compiler_specific.h"
#include "base/logging.h"
#include "base/string_util.h"
#include "googleurl/src/gurl.h"
#include "net/base/net_errors.h"
#include "net/proxy/proxy_config_service_fixed.h"
#include "net/proxy/proxy_script_fetcher.h"
#if defined(OS_WIN)
#include "net/proxy/proxy_config_service_win.h"
#include "net/proxy/proxy_resolver_winhttp.h"
#elif defined(OS_MACOSX)
#include "net/proxy/proxy_resolver_mac.h"
#elif defined(OS_LINUX)
#include "net/proxy/proxy_config_service_linux.h"
#endif
#include "net/proxy/proxy_resolver.h"
#include "net/proxy/proxy_resolver_v8.h"

using base::TimeDelta;
using base::TimeTicks;

namespace net {

// Config getter that fails every time.
class ProxyConfigServiceNull : public ProxyConfigService {
 public:
  virtual int GetProxyConfig(ProxyConfig* config) {
    return ERR_NOT_IMPLEMENTED;
  }
};

// Strip away any reference fragments and the username/password, as they
// are not relevant to proxy resolution.
static GURL SanitizeURLForProxyResolver(const GURL& url) {
  // TODO(eroman): The following duplicates logic from
  // HttpUtil::SpecForRequest. Should probably live in net_util.h
  GURL::Replacements replacements;
  replacements.ClearUsername();
  replacements.ClearPassword();
  replacements.ClearRef();
  return url.ReplaceComponents(replacements);
}

// Runs on the PAC thread to notify the proxy resolver of the fetched PAC
// script contents. This task shouldn't outlive ProxyService, since
// |resolver| is owned by ProxyService.
class NotifyFetchCompletionTask : public Task {
 public:
  NotifyFetchCompletionTask(ProxyResolver* resolver, const std::string& bytes)
      : resolver_(resolver), bytes_(bytes) {}

  virtual void Run() {
    resolver_->SetPacScript(bytes_);
  }

 private:
  ProxyResolver* resolver_;
  std::string bytes_;
};

// ProxyService::PacRequest ---------------------------------------------------

// We rely on the fact that the origin thread (and its message loop) will not
// be destroyed until after the PAC thread is destroyed.

class ProxyService::PacRequest :
    public base::RefCountedThreadSafe<ProxyService::PacRequest> {
 public:
  // |service| -- the ProxyService that owns this request.
  // |url|     -- the url of the query.
  // |results| -- the structure to fill with proxy resolve results.
  PacRequest(ProxyService* service,
             const GURL& url,
             ProxyInfo* results,
             CompletionCallback* callback)
      : service_(service),
        callback_(callback),
        results_(results),
        url_(url),
        is_started_(false),
        origin_loop_(MessageLoop::current()) {
    DCHECK(callback);
  }

  // Start the resolve proxy request on the PAC thread.
  void Query() {
    is_started_ = true;
    AddRef();  // balanced in QueryComplete

    GURL query_url = SanitizeURLForProxyResolver(url_);
    const GURL& pac_url = service_->config_.pac_url;
    results_->config_id_ = service_->config_.id();

    service_->pac_thread()->message_loop()->PostTask(FROM_HERE,
        NewRunnableMethod(this, &ProxyService::PacRequest::DoQuery,
                          service_->resolver(), query_url, pac_url));
  }

  // Run the request's callback on the current message loop.
  void PostCallback(int result_code) {
    AddRef();  // balanced in DoCallback
    MessageLoop::current()->PostTask(FROM_HERE,
        NewRunnableMethod(this, &ProxyService::PacRequest::DoCallback,
            result_code));
  }

  void Cancel() {
    // Clear these to inform QueryComplete that it should not try to
    // access them.
    service_ = NULL;
    callback_ = NULL;
    results_ = NULL;
  }

  // Returns true if Cancel() has been called.
  bool was_cancelled() const { return callback_ == NULL; }

 private:
  friend class ProxyService;

  // Runs on the PAC thread.
  void DoQuery(ProxyResolver* resolver,
               const GURL& query_url,
               const GURL& pac_url) {
    int rv = resolver->GetProxyForURL(query_url, pac_url, &results_buf_);
    origin_loop_->PostTask(FROM_HERE,
        NewRunnableMethod(this, &PacRequest::QueryComplete, rv));
  }

  // Runs the completion callback on the origin thread.
  void QueryComplete(int result_code) {
    // The PacRequest may have been cancelled after it was started.
    if (!was_cancelled()) {
      service_->DidCompletePacRequest(results_->config_id_, result_code);

      if (result_code == OK) {
        results_->Use(results_buf_);
        results_->RemoveBadProxies(service_->proxy_retry_info_);
      }
      callback_->Run(result_code);

      // We check for cancellation once again, in case the callback deleted
      // the owning ProxyService (whose destructor will in turn cancel us).
      if (!was_cancelled())
        service_->RemoveFrontOfRequestQueue(this);
    }

    Release();  // balances the AddRef in Query.  we may get deleted after
                // we return.
  }

  // Runs the completion callback on the origin thread.
  void DoCallback(int result_code) {
    if (!was_cancelled()) {
      callback_->Run(result_code);
    }
    Release();  // balances the AddRef in PostCallback.
  }

  // Must only be used on the "origin" thread.
  ProxyService* service_;
  CompletionCallback* callback_;
  ProxyInfo* results_;
  GURL url_;
  bool is_started_;

  // Usable from within DoQuery on the PAC thread.
  ProxyInfo results_buf_;
  MessageLoop* origin_loop_;
};

// ProxyService ---------------------------------------------------------------

ProxyService::ProxyService(ProxyConfigService* config_service,
                           ProxyResolver* resolver)
    : config_service_(config_service),
      resolver_(resolver),
      config_is_bad_(false),
      config_has_been_updated_(false),
      ALLOW_THIS_IN_INITIALIZER_LIST(proxy_script_fetcher_callback_(
          this, &ProxyService::OnScriptFetchCompletion)),
      fetched_pac_config_id_(ProxyConfig::INVALID_ID),
      fetched_pac_error_(OK),
      in_progress_fetch_config_id_(ProxyConfig::INVALID_ID) {
}

// static
ProxyService* ProxyService::Create(const ProxyInfo* pi) {
  if (pi) {
    // The ProxyResolver is set to NULL, since it should never be called
    // (because the configuration will never require PAC).
    return new ProxyService(new ProxyConfigServiceFixed(*pi), NULL);
  }
#if defined(OS_WIN)
  return new ProxyService(new ProxyConfigServiceWin(),
                          new ProxyResolverWinHttp());
#elif defined(OS_MACOSX)
  return new ProxyService(new ProxyConfigServiceMac(),
                          new ProxyResolverMac());
#elif defined(OS_LINUX)
  // On Linux we use the V8Resolver, no fallback implementation.
  return CreateNull();
#else
  return CreateNull();
#endif
}

// static
ProxyService* ProxyService::CreateUsingV8Resolver(
    const ProxyInfo* pi, URLRequestContext* url_request_context) {
  if (pi) {
    // The ProxyResolver is set to NULL, since it should never be called
    // (because the configuration will never require PAC).
    return new ProxyService(new ProxyConfigServiceFixed(*pi), NULL);
  }

  // Choose the system configuration service appropriate for each platform.
  ProxyConfigService* config_service;
#if defined(OS_WIN)
  config_service = new ProxyConfigServiceWin();
#elif defined(OS_MACOSX)
  config_service = new ProxyConfigServiceMac();
#elif defined(OS_LINUX)
  config_service = new ProxyConfigServiceLinux();
#else
  return CreateNull();
#endif

  // Create a ProxyService that uses V8 to evaluate PAC scripts.
  ProxyService* proxy_service = new ProxyService(
      config_service, new ProxyResolverV8());

  // Configure PAC script downloads to be issued using |url_request_context|.
  proxy_service->SetProxyScriptFetcher(
      ProxyScriptFetcher::Create(url_request_context));

  return proxy_service;
}

// static
ProxyService* ProxyService::CreateNull() {
  // The ProxyResolver is set to NULL, since it should never be called
  // (because the configuration will never require PAC).
  return new ProxyService(new ProxyConfigServiceNull, NULL);
}

int ProxyService::ResolveProxy(const GURL& url, ProxyInfo* result,
                               CompletionCallback* callback,
                               PacRequest** pac_request) {
  DCHECK(callback);

  // Check if the request can be completed right away. This is the case when
  // using a direct connection, or when the config is bad.
  UpdateConfigIfOld();
  int rv = TryToCompleteSynchronously(url, result);
  if (rv != ERR_IO_PENDING)
    return rv;

  // Otherwise, push the request into the work queue.
  scoped_refptr<PacRequest> req = new PacRequest(this, url, result, callback);
  pending_requests_.push_back(req);
  ProcessPendingRequests(req.get());

  // Completion will be notifed through |callback|, unless the caller cancels
  // the request using |pac_request|.
  if (pac_request)
    *pac_request = req.get();
  return rv;  // ERR_IO_PENDING
}

int ProxyService::TryToCompleteSynchronously(const GURL& url,
                                             ProxyInfo* result) {
  result->config_id_ = config_.id();

  DCHECK(config_.id() != ProxyConfig::INVALID_ID);

  // Fallback to a "direct" (no proxy) connection if the current configuration
  // is known to be bad.
  if (config_is_bad_) {
    // Reset this flag to false in case the ProxyInfo object is being
    // re-used by the caller.
    result->config_was_tried_ = false;
  } else {
    // Remember that we are trying to use the current proxy configuration.
    result->config_was_tried_ = true;

    if (!config_.proxy_rules.empty()) {
      ApplyProxyRules(url, config_.proxy_rules, result);
      return OK;
    }

    if (config_.pac_url.is_valid() || config_.auto_detect) {
      // If we failed to download the PAC script, return the network error
      // from the failed download. This is only going to happen for the first
      // request after the failed download -- after that |config_is_bad_| will
      // be set to true, so we short-cuircuit sooner.
      if (fetched_pac_error_ != OK && !IsFetchingPacScript()) {
        DidCompletePacRequest(fetched_pac_config_id_, fetched_pac_error_);
        return fetched_pac_error_;
      }
      return ERR_IO_PENDING;
    }
  }

  // otherwise, we have no proxy config
  result->UseDirect();
  return OK;
}

void ProxyService::ApplyProxyRules(const GURL& url,
                                   const ProxyConfig::ProxyRules& proxy_rules,
                                   ProxyInfo* result) {
  DCHECK(!proxy_rules.empty());

  if (ShouldBypassProxyForURL(url)) {
    result->UseDirect();
    return;
  }

  switch (proxy_rules.type) {
    case ProxyConfig::ProxyRules::TYPE_SINGLE_PROXY:
      result->UseProxyServer(proxy_rules.single_proxy);
      break;
    case ProxyConfig::ProxyRules::TYPE_PROXY_PER_SCHEME: {
      const ProxyServer* entry = proxy_rules.MapSchemeToProxy(url.scheme());
      if (entry) {
        result->UseProxyServer(*entry);
      } else {
        // We failed to find a matching proxy server for the current URL
        // scheme. Default to direct.
        result->UseDirect();
      }
      break;
    }
    default:
      result->UseDirect();
      NOTREACHED();
      break;
  }
}

void ProxyService::InitPacThread() {
  if (!pac_thread_.get()) {
    pac_thread_.reset(new base::Thread("pac-thread"));
    pac_thread_->Start();
  }
}

ProxyService::~ProxyService() {
  // Cancel the inprogress request (if any), and free the rest.
  for (PendingRequestsQueue::iterator it = pending_requests_.begin();
       it != pending_requests_.end();
       ++it) {
    (*it)->Cancel();
  }
}

void ProxyService::ProcessPendingRequests(PacRequest* recent_req) {
  if (pending_requests_.empty())
    return;

  // While the PAC script is being downloaded, requests are blocked.
  if (IsFetchingPacScript())
    return;

  // Get the next request to process (FIFO).
  PacRequest* req = pending_requests_.front().get();
  if (req->is_started_)
    return;

  // The configuration may have changed since |req| was added to the
  // queue. It could be this request now completes synchronously.
  if (req != recent_req) {
    UpdateConfigIfOld();
    int rv = TryToCompleteSynchronously(req->url_, req->results_);
    if (rv != ERR_IO_PENDING) {
      req->PostCallback(rv);
      RemoveFrontOfRequestQueue(req);
      return;
    }
  }

  // Check if a new PAC script needs to be downloaded.
  DCHECK(config_.id() != ProxyConfig::INVALID_ID);
  if (!resolver_->does_fetch() && config_.id() != fetched_pac_config_id_) {
    // For auto-detect we use the well known WPAD url.
    GURL pac_url = config_.auto_detect ?
        GURL("http://wpad/wpad.dat") : config_.pac_url;

    in_progress_fetch_config_id_ = config_.id();

    LOG(INFO) << "Starting fetch of PAC script " << pac_url
              << " for config_id=" << in_progress_fetch_config_id_;

    proxy_script_fetcher_->Fetch(
        pac_url, &in_progress_fetch_bytes_, &proxy_script_fetcher_callback_);
    return;
  }

  // The only choice left now is to actually run the ProxyResolver on
  // the PAC thread.
  InitPacThread();
  req->Query();
}

void ProxyService::RemoveFrontOfRequestQueue(PacRequest* expected_req) {
  DCHECK(pending_requests_.front().get() == expected_req);
  pending_requests_.pop_front();

  // Start next work item.
  ProcessPendingRequests(NULL);
}

void ProxyService::OnScriptFetchCompletion(int result) {
  DCHECK(IsFetchingPacScript());
  DCHECK(!resolver_->does_fetch());

  LOG(INFO) << "Completed PAC script fetch for config_id="
            << in_progress_fetch_config_id_
            << " with error " << ErrorToString(result)
            << ". Fetched a total of " << in_progress_fetch_bytes_.size()
            << " bytes";

  // Notify the ProxyResolver of the new script data (will be empty string if
  // result != OK).
  InitPacThread();
  pac_thread()->message_loop()->PostTask(FROM_HERE,
      new NotifyFetchCompletionTask(
          resolver_.get(), in_progress_fetch_bytes_));

  fetched_pac_config_id_ = in_progress_fetch_config_id_;
  fetched_pac_error_ = result;
  in_progress_fetch_config_id_ = ProxyConfig::INVALID_ID;
  in_progress_fetch_bytes_.clear();

  // Start a pending request if any.
  ProcessPendingRequests(NULL);
}

int ProxyService::ReconsiderProxyAfterError(const GURL& url,
                                            ProxyInfo* result,
                                            CompletionCallback* callback,
                                            PacRequest** pac_request) {
  // Check to see if we have a new config since ResolveProxy was called.  We
  // want to re-run ResolveProxy in two cases: 1) we have a new config, or 2) a
  // direct connection failed and we never tried the current config.

  bool re_resolve = result->config_id_ != config_.id();
  if (!re_resolve) {
    UpdateConfig();
    if (result->config_id_ != config_.id()) {
      // A new configuration!
      re_resolve = true;
    } else if (!result->config_was_tried_) {
      // We never tried the proxy configuration since we thought it was bad,
      // but because we failed to establish a connection, let's try the proxy
      // configuration again to see if it will work now.
      config_is_bad_ = false;
      re_resolve = true;
    }
  }
  if (re_resolve) {
    // If we have a new config or the config was never tried, we delete the
    // list of bad proxies and we try again.
    proxy_retry_info_.clear();
    return ResolveProxy(url, result, callback, pac_request);
  }

  // We don't have new proxy settings to try, fallback to the next proxy
  // in the list.
  bool was_direct = result->is_direct();
  if (!was_direct && result->Fallback(&proxy_retry_info_))
    return OK;

  if (!config_.auto_detect && !config_.proxy_rules.empty()) {
    // If auto detect is on, then we should try a DIRECT connection
    // as the attempt to reach the proxy failed.
    return ERR_FAILED;
  }

  // If we already tried a direct connection, then just give up.
  if (was_direct)
    return ERR_FAILED;

  // Try going direct.
  result->UseDirect();
  return OK;
}

// There are four states of the request we need to handle:
// (1) Not started (just sitting in the queue).
// (2) Executing PacRequest::DoQuery in the PAC thread.
// (3) Waiting for PacRequest::QueryComplete to be run on the origin thread.
// (4) Waiting for PacRequest::DoCallback to be run on the origin thread.
void ProxyService::CancelPacRequest(PacRequest* req) {
  DCHECK(req);

  bool is_active_request = req->is_started_ && !pending_requests_.empty() &&
      pending_requests_.front().get() == req;

  req->Cancel();

  if (is_active_request) {
    RemoveFrontOfRequestQueue(req);
    return;
  }

  // Otherwise just delete the request from the queue.
  PendingRequestsQueue::iterator it = std::find(
      pending_requests_.begin(), pending_requests_.end(), req);
  if (it != pending_requests_.end()) {
    pending_requests_.erase(it);
  }
}

void ProxyService::SetProxyScriptFetcher(
    ProxyScriptFetcher* proxy_script_fetcher) {
  proxy_script_fetcher_.reset(proxy_script_fetcher);
}

void ProxyService::DidCompletePacRequest(int config_id, int result_code) {
  // If we get an error that indicates a bad PAC config, then we should
  // remember that, and not try the PAC config again for a while.

  // Our config may have already changed.
  if (result_code == OK || config_id != config_.id())
    return;

  // Remember that this configuration doesn't work.
  config_is_bad_ = true;
}

void ProxyService::UpdateConfig() {
  config_has_been_updated_ = true;

  ProxyConfig latest;
  if (config_service_->GetProxyConfig(&latest) != OK)
    return;
  config_last_update_time_ = TimeTicks::Now();

  if (latest.Equals(config_))
    return;

  LOG(INFO) << "New proxy configuration was loaded:\n" << latest;

  config_ = latest;
  config_is_bad_ = false;

  // We have a new config, we should clear the list of bad proxies.
  proxy_retry_info_.clear();
}

void ProxyService::UpdateConfigIfOld() {
  // The overhead of calling ProxyConfigService::GetProxyConfig is very low.
  const TimeDelta kProxyConfigMaxAge = TimeDelta::FromSeconds(5);

  // Periodically check for a new config.
  if (!config_has_been_updated_ ||
      (TimeTicks::Now() - config_last_update_time_) > kProxyConfigMaxAge)
    UpdateConfig();
}

bool ProxyService::ShouldBypassProxyForURL(const GURL& url) {
  std::string url_domain = url.scheme();
  if (!url_domain.empty())
    url_domain += "://";

  url_domain += url.host();
  // This isn't superfluous; GURL case canonicalization doesn't hit the embedded
  // percent-encoded characters.
  StringToLowerASCII(&url_domain);

  if (config_.proxy_bypass_local_names) {
    if (url.host().find('.') == std::string::npos)
      return true;
  }

  for(std::vector<std::string>::const_iterator i = config_.proxy_bypass.begin();
      i != config_.proxy_bypass.end(); ++i) {
    std::string bypass_url_domain = *i;

    // The proxy server bypass list can contain entities with http/https
    // If no scheme is specified then it indicates that all schemes are
    // allowed for the current entry. For matching this we just use
    // the protocol scheme of the url passed in.
    if (bypass_url_domain.find("://") == std::string::npos) {
      std::string bypass_url_domain_with_scheme = url.scheme();
      bypass_url_domain_with_scheme += "://";
      bypass_url_domain_with_scheme += bypass_url_domain;

      bypass_url_domain = bypass_url_domain_with_scheme;
    }

    StringToLowerASCII(&bypass_url_domain);

    if (MatchPattern(url_domain, bypass_url_domain))
      return true;

    // Some systems (the Mac, for example) allow CIDR-style specification of
    // proxy bypass for IP-specified hosts (e.g.  "10.0.0.0/8"; see
    // http://www.tcd.ie/iss/internet/osx_proxy.php for a real-world example).
    // That's kinda cool so we'll provide that for everyone.
    // TODO(avi): implement here
  }

  return false;
}

SyncProxyServiceHelper::SyncProxyServiceHelper(MessageLoop* io_message_loop,
                                               ProxyService* proxy_service)
    : io_message_loop_(io_message_loop),
      proxy_service_(proxy_service),
      event_(false, false),
      ALLOW_THIS_IN_INITIALIZER_LIST(callback_(
          this, &SyncProxyServiceHelper::OnCompletion)) {
  DCHECK(io_message_loop_ != MessageLoop::current());
}

int SyncProxyServiceHelper::ResolveProxy(const GURL& url,
                                         ProxyInfo* proxy_info) {
  DCHECK(io_message_loop_ != MessageLoop::current());

  io_message_loop_->PostTask(FROM_HERE, NewRunnableMethod(
      this, &SyncProxyServiceHelper::StartAsyncResolve, url));

  event_.Wait();

  if (result_ == net::OK) {
    *proxy_info = proxy_info_;
  }
  return result_;
}

int SyncProxyServiceHelper::ReconsiderProxyAfterError(const GURL& url,
                                                      ProxyInfo* proxy_info) {
  DCHECK(io_message_loop_ != MessageLoop::current());

  io_message_loop_->PostTask(FROM_HERE, NewRunnableMethod(
      this, &SyncProxyServiceHelper::StartAsyncReconsider, url));

  event_.Wait();

  if (result_ == net::OK) {
    *proxy_info = proxy_info_;
  }
  return result_;
}

void SyncProxyServiceHelper::StartAsyncResolve(const GURL& url) {
  result_ = proxy_service_->ResolveProxy(url, &proxy_info_, &callback_, NULL);
  if (result_ != net::ERR_IO_PENDING) {
    OnCompletion(result_);
  }
}

void SyncProxyServiceHelper::StartAsyncReconsider(const GURL& url) {
  result_ = proxy_service_->ReconsiderProxyAfterError(
      url, &proxy_info_, &callback_, NULL);
  if (result_ != net::ERR_IO_PENDING) {
    OnCompletion(result_);
  }
}

void SyncProxyServiceHelper::OnCompletion(int rv) {
  result_ = rv;
  event_.Signal();
}

}  // namespace net
