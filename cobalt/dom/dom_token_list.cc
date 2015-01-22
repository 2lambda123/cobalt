/*
 * Copyright 2014 Google Inc. All Rights Reserved.
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

#include "cobalt/dom/dom_token_list.h"

#include <algorithm>

#include "base/string_split.h"
#include "base/string_util.h"
#include "cobalt/dom/element.h"
#include "cobalt/dom/stats.h"

namespace cobalt {
namespace dom {

// static
scoped_refptr<DOMTokenList> DOMTokenList::Create(
    const scoped_refptr<Element>& element, const std::string& attr_name) {
  return make_scoped_refptr(new DOMTokenList(element, attr_name));
}

// Algorithm for length:
//   http://www.w3.org/TR/2014/WD-dom-20140710/#dom-domtokenlist-length
unsigned int DOMTokenList::length() const {
  // Custom, not in any spec.
  MaybeRefresh();

  return tokens_.size();
}

// Algorithm for Item:
//   http://www.w3.org/TR/2014/WD-dom-20140710/#dom-domtokenlist-item
base::optional<std::string> DOMTokenList::Item(unsigned int index) const {
  // Custom, not in any spec.
  MaybeRefresh();

  // 1. If index is equal to or greater than the number of tokens in tokens,
  //    return null.
  if (index >= tokens_.size()) return base::nullopt;

  // 2. Return the indexth token in tokens.
  return tokens_[index];
}

// Algorithm for Contains:
//   http://www.w3.org/TR/2014/WD-dom-20140710/#dom-domtokenlist-contains
bool DOMTokenList::Contains(const std::string& token) const {
  // Custom, not in any spec.
  MaybeRefresh();

  // 1. If token is the empty string, then throw a "SyntaxError" exception.
  // 2. If token contains any ASCII whitespace, then throw an
  //    "InvalidCharacterError" exception.
  if (!IsTokenValid(token)) {
    return false;
  }

  // 3. Return true if token is in tokens, and false otherwise.
  if (std::find(tokens_.begin(), tokens_.end(), token) != tokens_.end()) {
    return true;
  }
  return false;
}

// Algorithm for Add:
//   http://www.w3.org/TR/2014/WD-dom-20140710/#dom-domtokenlist-add
void DOMTokenList::Add(const std::string& token) {
  // Custom, not in any spec.
  MaybeRefresh();

  // 1. If token is the empty string, then throw a "SyntaxError" exception.
  // 2. If token contains any ASCII whitespace, then throw an
  //    "InvalidCharacterError" exception.
  if (!IsTokenValid(token)) {
    return;
  }

  // 3. For each token in tokens, in given order, that is not in tokens, append
  //    token to tokens.
  if (std::find(tokens_.begin(), tokens_.end(), token) != tokens_.end()) {
    return;
  }
  tokens_.push_back(token);

  // 4. Run the update steps.
  UpdateSteps();
}

// Algorithm for Remove:
//   http://www.w3.org/TR/2014/WD-dom-20140710/#dom-domtokenlist-remove
void DOMTokenList::Remove(const std::string& token) {
  // Custom, not in any spec.
  MaybeRefresh();

  // 1. If token is the empty string, then throw a "SyntaxError" exception.
  // 2. If token contains any ASCII whitespace, then throw an
  //    "InvalidCharacterError" exception.
  if (!IsTokenValid(token)) {
    return;
  }

  // 3. For each token in tokens, remove token from tokens.
  tokens_.erase(std::remove(tokens_.begin(), tokens_.end(), token),
                tokens_.end());

  // 4. Run the update steps.
  UpdateSteps();
}

DOMTokenList::DOMTokenList(const scoped_refptr<Element>& element,
                           const std::string& attr_name)
    : element_(element),
      attr_name_(attr_name),
      element_node_generation_(Node::kInvalidNodeGeneration) {
  // The current implementation relies on nodes calling UpdateNodeGeneration()
  // each time the class is changed. This results in DOMTokenList only working
  // for class attribute. DOMTokenList is only used by Element::class_list(),
  // and it is not likely to be used anywhere else. Therefore DCHECK is used to
  // guarantee attr_name is always "class".
  DCHECK_EQ(attr_name, "class");
  Stats::GetInstance()->Add(this);
}

DOMTokenList::~DOMTokenList() { Stats::GetInstance()->Remove(this); }

// Algorithm for UpdateSteps:
//   http://www.w3.org/TR/2014/WD-dom-20140710/#concept-dtl-update
void DOMTokenList::UpdateSteps() const {
  element_->SetAttribute(attr_name_, JoinString(tokens_, ' '));
}

void DOMTokenList::MaybeRefresh() const {
  if (element_node_generation_ == element_->node_generation()) return;
  element_node_generation_ = element_->node_generation();
  std::string attribute = element_->GetAttribute(attr_name_).value_or("");
  base::SplitStringAlongWhitespace(attribute, &tokens_);
}

bool DOMTokenList::IsTokenValid(const std::string& token) const {
  if (token.empty()) {
    // TODO(***REMOVED***): Throw JS SyntaxError.
    return false;
  }
  if (token.find_first_of(" \n\t\r\f") != std::string::npos) {
    // TODO(***REMOVED***): Throw JS InvalidCharacterError.
    return false;
  }
  return true;
}

}  // namespace dom
}  // namespace cobalt
