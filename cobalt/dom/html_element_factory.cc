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

#include "cobalt/dom/html_element_factory.h"

#include "base/logging.h"
#include "cobalt/browser/script/script_runner.h"
#include "cobalt/dom/html_body_element.h"
#include "cobalt/dom/html_div_element.h"
#include "cobalt/dom/html_head_element.h"
#include "cobalt/dom/html_html_element.h"
#include "cobalt/dom/html_link_element.h"
#include "cobalt/dom/html_script_element.h"
#include "cobalt/dom/html_span_element.h"
#include "cobalt/dom/html_unknown_element.h"

namespace cobalt {
namespace dom {

template <typename T>
scoped_refptr<HTMLElement> HTMLElementFactory::CreateHTMLElementT() {
  return T::Create();
}

template <>
scoped_refptr<HTMLElement>
HTMLElementFactory::CreateHTMLElementT<HTMLLinkElement>() {
  return HTMLLinkElement::Create(loader_factory_, css_parser_);
}

template <>
scoped_refptr<HTMLElement>
HTMLElementFactory::CreateHTMLElementT<HTMLScriptElement>() {
  return HTMLScriptElement::Create(
      loader_factory_,
      make_scoped_ptr(new browser::script::ScriptRunner(global_object_proxy_)));
}

HTMLElementFactory::HTMLElementFactory(
    browser::ResourceLoaderFactory* loader_factory,
    cssom::CSSParser* css_parser,
    const scoped_refptr<browser::script::GlobalObjectProxy>&
        global_object_proxy)
    : loader_factory_(loader_factory),
      css_parser_(css_parser),
      global_object_proxy_(global_object_proxy) {
  tag_name_to_create_html_element_t_callback_map_[HTMLBodyElement::kTagName] =
      base::Bind(&HTMLElementFactory::CreateHTMLElementT<HTMLBodyElement>,
                 base::Unretained(this));
  tag_name_to_create_html_element_t_callback_map_[HTMLDivElement::kTagName] =
      base::Bind(&HTMLElementFactory::CreateHTMLElementT<HTMLDivElement>,
                 base::Unretained(this));
  tag_name_to_create_html_element_t_callback_map_[HTMLHeadElement::kTagName] =
      base::Bind(&HTMLElementFactory::CreateHTMLElementT<HTMLHeadElement>,
                 base::Unretained(this));
  tag_name_to_create_html_element_t_callback_map_[HTMLHtmlElement::kTagName] =
      base::Bind(&HTMLElementFactory::CreateHTMLElementT<HTMLHtmlElement>,
                 base::Unretained(this));
  tag_name_to_create_html_element_t_callback_map_[HTMLLinkElement::kTagName] =
      base::Bind(&HTMLElementFactory::CreateHTMLElementT<HTMLLinkElement>,
                 base::Unretained(this));
  tag_name_to_create_html_element_t_callback_map_[HTMLScriptElement::kTagName] =
      base::Bind(&HTMLElementFactory::CreateHTMLElementT<HTMLScriptElement>,
                 base::Unretained(this));
  tag_name_to_create_html_element_t_callback_map_[HTMLSpanElement::kTagName] =
      base::Bind(&HTMLElementFactory::CreateHTMLElementT<HTMLSpanElement>,
                 base::Unretained(this));
}

HTMLElementFactory::~HTMLElementFactory() {}

scoped_refptr<HTMLElement> HTMLElementFactory::CreateHTMLElement(
    const base::StringPiece& tag_name) {
  TagNameToCreateHTMLElementTCallbackMap::const_iterator iter =
      tag_name_to_create_html_element_t_callback_map_.find(tag_name);
  if (iter != tag_name_to_create_html_element_t_callback_map_.end()) {
    return iter->second.Run();
  } else {
    // TODO(***REMOVED***): Report unknown HTML tag.
    return HTMLUnknownElement::Create(tag_name);
  }
}

}  // namespace dom
}  // namespace cobalt
