// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_URL_REQUEST_URL_REQUEST_HTTP_JOB_H_
#define NET_URL_REQUEST_URL_REQUEST_HTTP_JOB_H_
#pragma once

#include <string>
#include <vector>

#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/string16.h"
#include "base/task.h"
#include "base/time.h"
#include "net/base/auth.h"
#include "net/base/completion_callback.h"
#include "net/base/cookie_store.h"
#include "net/http/http_request_info.h"
#include "net/url_request/url_request_job.h"
#include "net/url_request/url_request_throttler_entry_interface.h"

namespace net {

class HttpResponseInfo;
class HttpTransaction;
class URLRequestContext;

// A URLRequestJob subclass that is built on top of HttpTransaction.  It
// provides an implementation for both HTTP and HTTPS.
class URLRequestHttpJob : public URLRequestJob {
 public:
  static URLRequestJob* Factory(URLRequest* request,
                                const std::string& scheme);

 protected:
  explicit URLRequestHttpJob(URLRequest* request);

  // Shadows URLRequestJob's version of this method so we can grab cookies.
  void NotifyHeadersComplete();

  // Shadows URLRequestJob's method so we can record histograms.
  void NotifyDone(const URLRequestStatus& status);

  void DestroyTransaction();

  void AddExtraHeaders();
  void AddCookieHeaderAndStart();
  void SaveCookiesAndNotifyHeadersComplete();
  void SaveNextCookie();
  void FetchResponseCookies(const HttpResponseInfo* response_info,
                            std::vector<std::string>* cookies);

  // Process the Strict-Transport-Security header, if one exists.
  void ProcessStrictTransportSecurityHeader();

  void OnStartCompleted(int result);
  void OnReadCompleted(int result);
  void NotifyBeforeSendHeadersCallback(int result);

  bool ShouldTreatAsCertificateError(int result);

  void RestartTransactionWithAuth(const string16& username,
                                  const string16& password);

  // Overridden from URLRequestJob:
  virtual void SetUpload(UploadData* upload);
  virtual void SetExtraRequestHeaders(const HttpRequestHeaders& headers);
  virtual void Start();
  virtual void Kill();
  virtual LoadState GetLoadState() const;
  virtual uint64 GetUploadProgress() const;
  virtual bool GetMimeType(std::string* mime_type) const;
  virtual bool GetCharset(std::string* charset);
  virtual void GetResponseInfo(HttpResponseInfo* info);
  virtual bool GetResponseCookies(std::vector<std::string>* cookies);
  virtual int GetResponseCode() const;
  virtual Filter* SetupFilter() const;
  virtual bool IsSafeRedirect(const GURL& location);
  virtual bool NeedsAuth();
  virtual void GetAuthChallengeInfo(scoped_refptr<AuthChallengeInfo>*);
  virtual void SetAuth(const string16& username,
                       const string16& password);
  virtual void CancelAuth();
  virtual void ContinueWithCertificate(X509Certificate* client_cert);
  virtual void ContinueDespiteLastError();
  virtual bool ReadRawData(IOBuffer* buf, int buf_size, int *bytes_read);
  virtual void StopCaching();
  virtual HostPortPair GetSocketAddress() const;

  // Keep a reference to the url request context to be sure it's not deleted
  // before us.
  scoped_refptr<const URLRequestContext> context_;

  HttpRequestInfo request_info_;
  const HttpResponseInfo* response_info_;

  std::vector<std::string> response_cookies_;
  size_t response_cookies_save_index_;

  // Auth states for proxy and origin server.
  AuthState proxy_auth_state_;
  AuthState server_auth_state_;

  string16 username_;
  string16 password_;

  CompletionCallbackImpl<URLRequestHttpJob> start_callback_;
  CompletionCallbackImpl<URLRequestHttpJob> read_callback_;
  CompletionCallbackImpl<URLRequestHttpJob>
      notify_before_headers_sent_callback_;

  bool read_in_progress_;

  // An URL for an SDCH dictionary as suggested in a Get-Dictionary HTTP header.
  GURL sdch_dictionary_url_;

  scoped_ptr<HttpTransaction> transaction_;

  // This is used to supervise traffic and enforce exponential back-off.
  scoped_refptr<URLRequestThrottlerEntryInterface> throttling_entry_;

  // Indicated if an SDCH dictionary was advertised, and hence an SDCH
  // compressed response is expected.  We use this to help detect (accidental?)
  // proxy corruption of a response, which sometimes marks SDCH content as
  // having no content encoding <oops>.
  bool sdch_dictionary_advertised_;

  // For SDCH latency experiments, when we are able to do SDCH, we may enable
  // either an SDCH latency test xor a pass through test.  The following bools
  // indicate what we decided on for this instance.
  bool sdch_test_activated_;  // Advertising a dictionary for sdch.
  bool sdch_test_control_;    // Not even accepting-content sdch.

  // For recording of stats, we need to remember if this is cached content.
  bool is_cached_content_;

 private:
  enum CompletionCause {
    ABORTED,
    FINISHED
  };

  class HttpFilterContext;

  virtual ~URLRequestHttpJob();

  void RecordTimer();
  void ResetTimer();

  virtual void UpdatePacketReadTimes();
  void RecordPacketStats(FilterContext::StatisticSelector statistic) const;

  void RecordCompressionHistograms();
  bool IsCompressibleContent() const;

  // Starts the transaction if extensions using the webrequest API do not
  // object.
  void StartTransaction();
  void StartTransactionInternal();

  void RecordPerfHistograms(CompletionCause reason);
  void DoneWithRequest(CompletionCause reason);

  // Callback functions for Cookie Monster
  void CheckCookiePolicyAndLoad(const CookieList& cookie_list);
  void OnCookiesLoaded(
      std::string* cookie_line,
      std::vector<CookieStore::CookieInfo>* cookie_infos);
  void DoStartTransaction();
  void OnCookieSaved(bool cookie_status);
  void CookieHandled();

  // Some servers send the body compressed, but specify the content length as
  // the uncompressed size. If this is the case, we return true in order
  // to request to work around this non-adherence to the HTTP standard.
  // |rv| is the standard return value of a read function indicating the number
  // of bytes read or, if negative, an error code.
  bool ShouldFixMismatchedContentLength(int rv) const;

  base::Time request_creation_time_;

  // Data used for statistics gathering. This data is only used for histograms
  // and is not required. It is only gathered if packet_timing_enabled_ == true.
  //
  // TODO(jar): improve the quality of the gathered info by gathering most times
  // at a lower point in the network stack, assuring we have actual packet
  // boundaries, rather than approximations.  Also note that input byte count
  // as gathered here is post-SSL, and post-cache-fetch, and does not reflect
  // true packet arrival times in such cases.

  // Enable recording of packet arrival times for histogramming.
  bool packet_timing_enabled_;
  bool done_;  // True when we are done doing work.

  // The number of bytes that have been accounted for in packets (where some of
  // those packets may possibly have had their time of arrival recorded).
  int64 bytes_observed_in_packets_;

  // The request time may not be available when we are being destroyed, so we
  // snapshot it early on.
  base::Time request_time_snapshot_;

  // Since we don't save all packet times in packet_times_, we save the
  // last time for use in histograms.
  base::Time final_packet_time_;

  // The start time for the job, ignoring re-starts.
  base::TimeTicks start_time_;

  scoped_ptr<HttpFilterContext> filter_context_;
  ScopedRunnableMethodFactory<URLRequestHttpJob> method_factory_;
  base::WeakPtrFactory<URLRequestHttpJob> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(URLRequestHttpJob);
};

}  // namespace net

#endif  // NET_URL_REQUEST_URL_REQUEST_HTTP_JOB_H_
