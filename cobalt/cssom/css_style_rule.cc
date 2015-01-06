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

#include "cobalt/cssom/css_style_rule.h"

#include "cobalt/cssom/css_style_declaration.h"
#include "cobalt/cssom/selector.h"

namespace cobalt {
namespace cssom {

const scoped_refptr<CSSStyleDeclaration>& CSSStyleRule::style() {
  return style_;
}

CSSStyleRule::CSSStyleRule(Selectors selectors,
                           const scoped_refptr<CSSStyleDeclaration>& style)
    : selectors_(selectors.Pass()), style_(style) {}

CSSStyleRule::~CSSStyleRule() {}

}  // namespace cssom
}  // namespace cobalt
