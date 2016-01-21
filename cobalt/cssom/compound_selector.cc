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

#include "cobalt/cssom/compound_selector.h"

#include <algorithm>

#include "cobalt/cssom/pseudo_element.h"
#include "cobalt/cssom/selector_visitor.h"

namespace cobalt {
namespace cssom {
namespace {

bool SimpleSelectorsLessThan(const SimpleSelector* lhs,
                             const SimpleSelector* rhs) {
  if (lhs->type() < rhs->type()) {
    return true;
  }
  if (lhs->type() > rhs->type()) {
    return false;
  }

  return (lhs->prefix() < rhs->prefix()) ||
         (lhs->prefix() == rhs->prefix() && lhs->text() < rhs->text());
}

}  // namespace

CompoundSelector::CompoundSelector()
    : should_normalize_(false), pseudo_element_(NULL), left_combinator_(NULL) {}

CompoundSelector::~CompoundSelector() {}

void CompoundSelector::Accept(SelectorVisitor* visitor) {
  visitor->VisitCompoundSelector(this);
}

void CompoundSelector::AppendSelector(
    scoped_ptr<SimpleSelector> simple_selector) {
  specificity_.AddFrom(simple_selector->GetSpecificity());
  if (simple_selector->AsPseudoElement()) {
    DCHECK(!pseudo_element_);
    pseudo_element_ = simple_selector->AsPseudoElement();
  }
  simple_selectors_.push_back(simple_selector.release());
  should_normalize_ = true;
}

const std::string& CompoundSelector::GetNormalizedSelectorText() {
  if (should_normalize_) {
    should_normalize_ = false;
    std::sort(simple_selectors_.begin(), simple_selectors_.end(),
              SimpleSelectorsLessThan);
    normalized_selector_text_ = "";
    for (SimpleSelectors::iterator it = simple_selectors_.begin();
         it != simple_selectors_.end(); ++it) {
      normalized_selector_text_ += (*it)->prefix().c_str();
      normalized_selector_text_ += (*it)->text().c_str();
    }
  }
  return normalized_selector_text_;
}

void CompoundSelector::set_right_combinator(scoped_ptr<Combinator> combinator) {
  right_combinator_ = combinator.Pass();
}

}  // namespace cssom
}  // namespace cobalt
