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

#ifndef DOM_DOM_IMPLEMENTATION_H_
#define DOM_DOM_IMPLEMENTATION_H_

#include <string>

#include "base/memory/ref_counted.h"
#include "base/optional.h"
#include "cobalt/dom/document_type.h"
#include "cobalt/dom/xml_document.h"
#include "cobalt/script/wrappable.h"

namespace cobalt {
namespace dom {

// The DOMImplementation interface represent an object providing methods which
// are not dependent on any particular document. Such an object is returned by
// the Document.implementation property.
//   http://www.w3.org/TR/2015/WD-dom-20150618/#interface-domimplementation
class DOMImplementation : public script::Wrappable {
 public:
  DOMImplementation() {}
  ~DOMImplementation() OVERRIDE {}

  // Web API: DOMImplementation
  scoped_refptr<XMLDocument> CreateDocument(
      base::optional<std::string> namespace_name,
      const std::string& qualified_name);
  scoped_refptr<XMLDocument> CreateDocument(
      base::optional<std::string> namespace_name,
      const std::string& qualified_name, scoped_refptr<DocumentType> doctype);

  DEFINE_WRAPPABLE_TYPE(DOMImplementation);

 private:
  DISALLOW_COPY_AND_ASSIGN(DOMImplementation);
};

}  // namespace dom
}  // namespace cobalt

#endif  // DOM_DOM_IMPLEMENTATION_H_
