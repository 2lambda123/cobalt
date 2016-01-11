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

#ifndef COBALT_DOM_HTML_ANCHOR_ELEMENT_H_
#define COBALT_DOM_HTML_ANCHOR_ELEMENT_H_

#include <string>

#include "cobalt/dom/html_element.h"

namespace cobalt {
namespace dom {

class Document;

// The HTML Anchor Element (<a>) defines a hyperlink to a location on the same
// page or any other page on the Web.
//   http://www.w3.org/TR/html5/text-level-semantics.html#htmlanchorelement
class HTMLAnchorElement : public HTMLElement {
 public:
  static const char kTagName[];

  explicit HTMLAnchorElement(Document* document)
      : HTMLElement(document, base::Token(kTagName)) {}

  // Custom, not in any spec.
  scoped_refptr<HTMLAnchorElement> AsHTMLAnchorElement() OVERRIDE {
    return this;
  }

  DEFINE_WRAPPABLE_TYPE(HTMLAnchorElement);

 private:
  ~HTMLAnchorElement() OVERRIDE {}
};

}  // namespace dom
}  // namespace cobalt

#endif  // COBALT_DOM_HTML_ANCHOR_ELEMENT_H_
