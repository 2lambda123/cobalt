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

#include "cobalt/dom/testing/stub_css_parser.h"

#include "cobalt/cssom/css_style_declaration_data.h"
#include "cobalt/cssom/css_style_sheet.h"
#include "cobalt/cssom/property_value.h"

namespace cobalt {
namespace dom {
namespace testing {

scoped_refptr<cssom::CSSStyleSheet> StubCSSParser::ParseStyleSheet(
    const std::string& input, const base::SourceLocation& input_location) {
  return new cssom::CSSStyleSheet();
}

scoped_refptr<cssom::CSSStyleDeclarationData>
StubCSSParser::ParseDeclarationList(
    const std::string& input, const base::SourceLocation& input_location) {
  return new cssom::CSSStyleDeclarationData();
}

scoped_refptr<cssom::PropertyValue> StubCSSParser::ParsePropertyValue(
    const std::string& property_name, const std::string& property_value,
    const base::SourceLocation& property_location) {
  return NULL;
}

}  // namespace testing
}  // namespace dom
}  // namespace cobalt
