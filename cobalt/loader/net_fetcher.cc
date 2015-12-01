/*
 * Copyright 2015 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cobalt/loader/net_fetcher.h"

#include <string>

#include "base/stringprintf.h"
#include "cobalt/network/network_module.h"
#include "net/url_request/url_fetcher.h"

namespace cobalt {
namespace loader {

namespace {
bool IsResponseCodeSuccess(int response_code) {
  // NetFetcher only considers success to be if the network request
  // was successful *and* we get a 2xx response back.
  // We also accept a response code of -1 for URLs where response code is not
  // meaningful, like "data:"
  // TODO(***REMOVED***): 304s are unexpected since we don't enable the HTTP cache,
  // meaning we don't add the If-Modified-Since header to our request.
  // However, it's unclear what would happen if we did, so DCHECK.
  DCHECK_NE(response_code, 304) << "Unsupported status code";
  return response_code == -1 || response_code / 100 == 2;
}
}  // namespace

NetFetcher::NetFetcher(const GURL& url, Handler* handler,
                       const network::NetworkModule* network_module,
                       const Options& options)
    : Fetcher(handler) {
  url_fetcher_.reset(
      net::URLFetcher::Create(url, options.request_method, this));
  url_fetcher_->SetRequestContext(network_module->url_request_context_getter());
  url_fetcher_->DiscardResponse();

  // Let the caller configure the url fetcher as desired.
  if (!options.setup_callback.is_null()) {
    options.setup_callback.Run(url_fetcher_.get());
  }
  url_fetcher_->Start();
}

void NetFetcher::OnURLFetchResponseStarted(const net::URLFetcher* source) {
  DCHECK(thread_checker_.CalledOnValidThread());
  if (IsResponseCodeSuccess(source->GetResponseCode())) {
    handler()->OnResponseStarted(this, source->GetResponseHeaders());
  } else {
    handler()->OnError(
        this, base::StringPrintf("NetFetcher error on %s: response code %d",
                                 source->GetURL().spec().c_str(),
                                 source->GetResponseCode()));
    // Cancel/destroy the fetcher on any error.
    url_fetcher_.reset();
  }
}

void NetFetcher::OnURLFetchComplete(const net::URLFetcher* source) {
  DCHECK(thread_checker_.CalledOnValidThread());
  const net::URLRequestStatus& status = source->GetStatus();
  const int response_code = source->GetResponseCode();
  if (status.is_success() && IsResponseCodeSuccess(response_code)) {
    handler()->OnDone(this);
  } else {
    handler()->OnError(
        this,
        base::StringPrintf("NetFetcher error on %s: %s, response code %d",
                           source->GetURL().spec().c_str(),
                           net::ErrorToString(status.error()), response_code));
  }
}

bool NetFetcher::ShouldSendDownloadData() { return true; }

void NetFetcher::OnURLFetchDownloadData(const net::URLFetcher* source,
                                        scoped_ptr<std::string> download_data) {
  UNREFERENCED_PARAMETER(source);
  handler()->OnReceived(this, download_data->data(), download_data->length());
}

NetFetcher::~NetFetcher() { DCHECK(thread_checker_.CalledOnValidThread()); }

}  // namespace loader
}  // namespace cobalt
