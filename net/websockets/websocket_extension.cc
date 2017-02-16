// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/websockets/websocket_extension.h"

#include <map>
#include <string>

#include "base/logging.h"
#include "net/http/http_util.h"

namespace net {

WebSocketExtension::Parameter::Parameter(const std::string& name)
    : name_(name) {}

WebSocketExtension::Parameter::Parameter(const std::string& name,
                                         const std::string& value)
    : name_(name), value_(value) {
  DCHECK(!value.empty());
  // |extension-param| must be a token.
  DCHECK(HttpUtil::IsToken(value));
}

bool WebSocketExtension::Parameter::Equals(const Parameter& other) const {
  return name_ == other.name_ && value_ == other.value_;
}

WebSocketExtension::WebSocketExtension() {}

WebSocketExtension::WebSocketExtension(const std::string& name) : name_(name) {}

WebSocketExtension::WebSocketExtension(const WebSocketExtension& other) {
  name_ = other.name_;
  parameters_ = other.parameters_;
}

WebSocketExtension::~WebSocketExtension() {}

bool WebSocketExtension::Equals(const WebSocketExtension& other) const {
  if (name_ != other.name_)
    return false;
  if (parameters_.size() != other.parameters_.size())
    return false;

  std::multimap<std::string, std::string> this_parameters, other_parameters;
  for (std::vector<Parameter>::const_iterator iterator = parameters_.begin();
       iterator != parameters_.end(); ++iterator) {
    const Parameter& p(*iterator);
    this_parameters.insert(std::make_pair(p.name(), p.value()));
  }
  for (std::vector<Parameter>::const_iterator iterator =
           other.parameters_.begin();
       iterator != other.parameters_.end(); ++iterator) {
    const Parameter& p(*iterator);
    other_parameters.insert(std::make_pair(p.name(), p.value()));
  }
  return this_parameters == other_parameters;
}

std::string WebSocketExtension::ToString() const {
  if (name_.empty())
    return std::string();

  std::string result = name_;

  for (std::vector<Parameter>::const_iterator iterator = parameters_.begin();
       iterator != parameters_.end(); ++iterator) {
    const Parameter& param(*iterator);
    result += "; " + param.name();
    if (!param.HasValue())
      continue;

    // |extension-param| must be a token and we don't need to quote it.
    DCHECK(HttpUtil::IsToken(param.value()));
    result += "=" + param.value();
  }
  return result;
}

}  // namespace net
