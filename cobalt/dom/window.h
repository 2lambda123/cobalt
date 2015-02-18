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

#ifndef DOM_WINDOW_H_
#define DOM_WINDOW_H_

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "cobalt/dom/html_element_factory.h"
#include "cobalt/script/wrappable.h"
#include "googleurl/src/gurl.h"

namespace cobalt {
namespace browser {
class ResourceLoaderFactory;
}  // namespace browser

namespace cssom {
class CSSParser;
}  // namespace cssom

namespace script {
class ScriptRunner;
}  // namespace script

namespace dom {

class Document;
class DocumentBuilder;

// The window object represents a window containing a DOM document.
//   http://www.w3.org/TR/html5/browsers.html#the-window-object
//
// TODO(***REMOVED***): Properly handle viewport resolution change event.
// TODO(***REMOVED***): Inherit from EventTarget.
class Window : public script::Wrappable {
 public:
  Window(int width, int height, cssom::CSSParser* css_parser,
         browser::ResourceLoaderFactory* resource_loader_factory,
         script::ScriptRunner* script_runner, const GURL& url);

  int inner_width() const { return width_; }
  int inner_height() const { return height_; }

  const scoped_refptr<Document>& document() { return document_; }

 private:
  ~Window() OVERRIDE {}

  int width_;
  int height_;

  HTMLElementFactory html_element_factory_;
  scoped_ptr<DocumentBuilder> document_builder_;
  scoped_refptr<Document> document_;

  friend class scoped_ptr<Window>;
  DISALLOW_COPY_AND_ASSIGN(Window);
};

}  // namespace dom
}  // namespace cobalt

#endif  // DOM_WINDOW_H_
