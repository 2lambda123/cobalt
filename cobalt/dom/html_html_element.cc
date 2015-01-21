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

#include "cobalt/dom/html_html_element.h"

#include "cobalt/dom/document.h"

namespace cobalt {
namespace dom {

// static
const char* HTMLHtmlElement::kTagName = "html";

// static
scoped_refptr<HTMLHtmlElement> HTMLHtmlElement::Create() {
  return make_scoped_refptr(new HTMLHtmlElement());
}

HTMLHtmlElement::HTMLHtmlElement() {}

HTMLHtmlElement::~HTMLHtmlElement() {}

void HTMLHtmlElement::AttachToDocument(Document* document) {
  Node::AttachToDocument(document);
  this->owner_document()->SetHtmlInternal(this);
}

void HTMLHtmlElement::DetachFromDocument() {
  this->owner_document()->SetHtmlInternal(NULL);
  Node::DetachFromDocument();
}

}  // namespace dom
}  // namespace cobalt
