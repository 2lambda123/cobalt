// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/proxy/polling_proxy_config_service.h"

#include "base/lock.h"
#include "base/message_loop_proxy.h"
#include "base/observer_list.h"
#include "base/scoped_ptr.h"
#include "base/worker_pool.h"
#include "net/proxy/proxy_config.h"

namespace net {

// Reference-counted wrapper that does all the work (needs to be
// reference-counted since we post tasks between threads; may outlive
// the parent PollingProxyConfigService).
class PollingProxyConfigService::Core
    : public base::RefCountedThreadSafe<PollingProxyConfigService::Core> {
 public:
  Core(base::TimeDelta poll_interval,
       GetConfigFunction get_config_func)
      : get_config_func_(get_config_func),
        has_config_(false),
        poll_task_outstanding_(false),
        poll_interval_(poll_interval),
        have_initialized_origin_loop_(false) {
  }

  // Called when the parent PollingProxyConfigService is destroyed
  // (observers should not be called past this point).
  void Orphan() {
    AutoLock l(lock_);
    origin_loop_proxy_ = NULL;
  }

  bool GetLatestProxyConfig(ProxyConfig* config) {
    LazyInitializeOriginLoop();
    DCHECK(origin_loop_proxy_->BelongsToCurrentThread());

    OnLazyPoll();

    // If we have already retrieved the proxy settings (on worker thread)
    // then return what we last saw.
    if (has_config_) {
      *config = last_config_;
      return true;
    }
    return false;
  }

  void AddObserver(Observer* observer) {
    LazyInitializeOriginLoop();
    DCHECK(origin_loop_proxy_->BelongsToCurrentThread());
    observers_.AddObserver(observer);
  }

  void RemoveObserver(Observer* observer) {
    DCHECK(origin_loop_proxy_->BelongsToCurrentThread());
    observers_.RemoveObserver(observer);
  }

  // Check for a new configuration if enough time has elapsed.
  void OnLazyPoll() {
    LazyInitializeOriginLoop();
    DCHECK(origin_loop_proxy_->BelongsToCurrentThread());

    if (poll_task_outstanding_)
      return;  // Still waiting for earlier test to finish.

    base::TimeTicks now = base::TimeTicks::Now();

    if (last_poll_time_.is_null() ||
        (now - last_poll_time_) > poll_interval_) {
      last_poll_time_ = now;
      poll_task_outstanding_ = true;
      WorkerPool::PostTask(
          FROM_HERE,
          NewRunnableMethod(
              this, &Core::PollOnWorkerThread, get_config_func_), true);
    }
  }

 private:
  void PollOnWorkerThread(GetConfigFunction func) {
    ProxyConfig config;
    func(&config);

    AutoLock l(lock_);
    if (origin_loop_proxy_) {
      origin_loop_proxy_->PostTask(
          FROM_HERE,
          NewRunnableMethod(this, &Core::GetConfigCompleted, config));
    }
  }

  // Called after the worker thread has finished retrieving a configuration.
  void GetConfigCompleted(const ProxyConfig& config) {
    DCHECK(poll_task_outstanding_);
    poll_task_outstanding_ = false;

    if (!origin_loop_proxy_)
      return;  // Was orphaned (parent has already been destroyed).

    DCHECK(origin_loop_proxy_->BelongsToCurrentThread());

    if (!has_config_ || !last_config_.Equals(config)) {
      // If the configuration has changed, notify the observers.
      has_config_ = true;
      last_config_ = config;
      FOR_EACH_OBSERVER(Observer, observers_, OnProxyConfigChanged(config));
    }
  }

  void LazyInitializeOriginLoop() {
    // TODO(eroman): Really this should be done in the constructor, but right
    //               now chrome is constructing the ProxyConfigService on the
    //               UI thread so we can't cache the IO thread for the purpose
    //               of DCHECKs until the first call is made.
    if (!have_initialized_origin_loop_) {
      origin_loop_proxy_ = base::MessageLoopProxy::CreateForCurrentThread();
      have_initialized_origin_loop_ = true;
    }
  }

  GetConfigFunction get_config_func_;
  ObserverList<Observer> observers_;
  bool has_config_;
  bool poll_task_outstanding_;
  ProxyConfig last_config_;
  base::TimeTicks last_poll_time_;
  base::TimeDelta poll_interval_;
  bool have_initialized_origin_loop_;

  Lock lock_;
  scoped_refptr<base::MessageLoopProxy> origin_loop_proxy_;
};

PollingProxyConfigService::PollingProxyConfigService(
    base::TimeDelta poll_interval,
    GetConfigFunction get_config_func)
    : core_(new Core(poll_interval, get_config_func)) {
}

PollingProxyConfigService::~PollingProxyConfigService() {
  core_->Orphan();
}

void PollingProxyConfigService::AddObserver(Observer* observer) {
  core_->AddObserver(observer);
}

void PollingProxyConfigService::RemoveObserver(Observer* observer) {
  core_->RemoveObserver(observer);
}

bool PollingProxyConfigService::GetLatestProxyConfig(ProxyConfig* config) {
  return core_->GetLatestProxyConfig(config);
}

void PollingProxyConfigService::OnLazyPoll() {
  core_->OnLazyPoll();
}

}  // namespace net
