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

#ifndef DOM_ELEMENT_H_
#define DOM_ELEMENT_H_

#include <string>

#include "base/containers/small_map.h"
#include "base/hash_tables.h"
#include "base/optional.h"
#include "base/string_piece.h"
#include "cobalt/base/source_location.h"
#include "cobalt/dom/event_listener.h"
#include "cobalt/dom/node.h"

namespace cobalt {
namespace dom {

class DOMTokenList;
class HTMLCollection;
class HTMLElement;
class HTMLElementContext;
class NamedNodeMap;
class NodeList;

// The Element interface represents an object of a Document. This interface
// describes methods and properties common to all kinds of elements.
//   http://www.w3.org/TR/2014/WD-dom-20140710/#interface-element
class Element : public Node {
 public:
  // NOTE: The array size of SmallMap and the decision to use base::hash_map as
  // the underlying container type are based on extensive performance testing
  // with ***REMOVED***. Do not change these unless additional profiling data justifies
  // it.
  typedef base::SmallMap<base::hash_map<std::string, std::string>, 2>
      AttributeMap;

  explicit Element(Document* document);
  Element(Document* document, const std::string& tag_name);

  // Web API: Node
  //
  std::string node_name() const OVERRIDE { return tag_name(); }
  NodeType node_type() const OVERRIDE { return Node::kElementNode; }

  base::optional<std::string> text_content() const OVERRIDE;
  void set_text_content(
      const base::optional<std::string>& text_content) OVERRIDE;

  bool HasAttributes() const OVERRIDE;

  // Web API: Element
  //

  virtual std::string tag_name() const { return tag_name_; }

  const std::string& id() const { return id_attribute_; }
  void set_id(const std::string& value) { SetAttribute("id", value); }

  const std::string& class_name() const { return class_attribute_; }
  void set_class_name(const std::string& value) {
    SetAttribute("class", value);
  }

  scoped_refptr<DOMTokenList> class_list();

  scoped_refptr<NamedNodeMap> attributes();

  base::optional<std::string> GetAttribute(const std::string& name) const;
  void SetAttribute(const std::string& name, const std::string& value);
  void RemoveAttribute(const std::string& name);
  bool HasAttribute(const std::string& name) const;

  scoped_refptr<HTMLCollection> GetElementsByTagName(
      const std::string& tag_name) const;
  scoped_refptr<HTMLCollection> GetElementsByClassName(
      const std::string& class_name) const;

  // TODO(***REMOVED***, b/24212587): Support clientWidth and clientHeight properly.
  float client_width() const { return 1920; }
  float client_height() const { return 1080; }

  // Web API: DOM Parsing and Serialization (partial interface)
  //   http://www.w3.org/TR/DOM-Parsing/#extensions-to-the-element-interface
  //
  std::string inner_html() const;
  void set_inner_html(const std::string& inner_html);
  std::string outer_html() const;
  void set_outer_html(const std::string& outer_html);

  // Web API: Selectors API (partial interface)
  //   http://www.w3.org/TR/selectors-api2/#interface-definitions
  //
  scoped_refptr<Element> QuerySelector(const std::string& selectors);
  scoped_refptr<NodeList> QuerySelectorAll(const std::string& selectors);

  // Custom, not in any spec: Node.
  //
  bool IsElement() const OVERRIDE { return true; }

  scoped_refptr<Element> AsElement() OVERRIDE { return this; }

  void Accept(NodeVisitor* visitor) OVERRIDE;
  void Accept(ConstNodeVisitor* visitor) const OVERRIDE;

  scoped_refptr<Node> Duplicate() const OVERRIDE;

  // Custom, not in any spec.
  //
  // Returns whether the element has no children at all except comments or
  // processing instructions.
  //   http://www.w3.org/TR/selectors4/#empty-pseudo
  bool IsEmpty();

  // Returns whether the element has focus.
  //   http://www.w3.org/TR/selectors4/#focus-pseudo
  bool HasFocus();

  // Returns a map that holds the actual attributes of the element.
  const AttributeMap& attribute_map() const { return attribute_map_; }

  // These are called when the element is generated by the parser, rather than
  // by javascript.
  // opening_tag_location points to ">" of opening tag.
  virtual void OnParserStartTag(
      const base::SourceLocation& opening_tag_location) {
    UNREFERENCED_PARAMETER(opening_tag_location);
  }
  virtual void OnParserEndTag() {}

  virtual scoped_refptr<HTMLElement> AsHTMLElement();

  DEFINE_WRAPPABLE_TYPE(Element);

 protected:
  ~Element() OVERRIDE;

  // Getting and setting boolean attribute.
  //   http://www.w3.org/TR/html5/infrastructure.html#boolean-attribute
  bool GetBooleanAttribute(const std::string& name) const;
  void SetBooleanAttribute(const std::string& name, bool value);

  void CopyAttributes(const Element& other);

  HTMLElementContext* html_element_context();

 private:
  virtual void OnSetAttribute(const std::string& /* name */,
                              const std::string& /* value */) {}
  virtual void OnRemoveAttribute(const std::string& /* name */) {}

  // Callback for error when parsing inner / outer HTML.
  void HTMLParseError(const std::string& error);

  // Tag name of the element.
  std::string tag_name_;
  // A map that holds the actual element attributes.
  AttributeMap attribute_map_;
  // The "id" attribute for this element. Stored here in addition to being
  // stored in |attribute_map_| as an optimization for id().
  std::string id_attribute_;
  // The "class" attribute for this element. Stored here in addition to being
  // stored in |attribute_map_| as an optimization for class().
  std::string class_attribute_;
  // A weak pointer to a NamedNodeMap that proxies the actual attributes.
  // This heavy weight object is kept in memory only when needed by the user.
  base::WeakPtr<NamedNodeMap> named_node_map_;
  // Lazily created list of CSS classes.
  // After creation this is kept in memory because of the significant negative
  // performance impact of repeatedly recreating it.
  scoped_refptr<DOMTokenList> class_list_;
};

}  // namespace dom
}  // namespace cobalt

#endif  // DOM_ELEMENT_H_
