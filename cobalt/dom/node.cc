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

#include "cobalt/dom/node.h"

#include "cobalt/dom/comment.h"
#include "cobalt/dom/document.h"
#include "cobalt/dom/dom_stats.h"
#include "cobalt/dom/element.h"
#include "cobalt/dom/html_collection.h"
#include "cobalt/dom/text.h"

namespace cobalt {
namespace dom {

scoped_refptr<Node> Node::AppendChild(const scoped_refptr<Node>& new_child) {
  return InsertBefore(new_child, NULL);
}

bool Node::Contains(const scoped_refptr<Node>& other_node) const {
  const Node* child = first_child_;
  while (child) {
    if (child == other_node || child->Contains(other_node)) {
      return true;
    }
    child = child->next_sibling_;
  }
  return false;
}

bool Node::HasChildNodes() const { return first_child_ != NULL; }

scoped_refptr<Node> Node::InsertBefore(
    const scoped_refptr<Node>& new_child,
    const scoped_refptr<Node>& reference_child) {
  if (!new_child) {
    // TODO(***REMOVED***): Throw JS ReferenceError.
    return NULL;
  }
  // Check if this node can accept new_child as a child.
  if (!CheckAcceptAsChild(new_child)) {
    // TODO(***REMOVED***): Throw JS HierarchyRequestError.
    return NULL;
  }
  if (reference_child && reference_child->parent_ != this) {
    // TODO(***REMOVED***): Throw JS NotFoundError.
    return NULL;
  }
  // Inserting before itself doesn't change anything.
  if (reference_child == new_child) {
    return new_child;
  }

  if (new_child->parent_) {
    new_child->parent_->RemoveChild(new_child);
  }
  new_child->parent_ = AsWeakPtr();

  scoped_refptr<Node> next_sibling = reference_child;
  base::WeakPtr<Node> previous_sibling;

  if (next_sibling) {
    previous_sibling = next_sibling->previous_sibling_;
  } else {
    previous_sibling = last_child_;
  }

  if (previous_sibling) {
    previous_sibling->next_sibling_ = new_child;
  } else {
    first_child_ = new_child;
  }
  new_child->previous_sibling_ = previous_sibling;

  if (next_sibling) {
    next_sibling->previous_sibling_ = new_child->AsWeakPtr();
  } else {
    last_child_ = new_child->AsWeakPtr();
  }
  new_child->next_sibling_ = next_sibling;

  new_child->UpdateNodeGeneration();
  if (owner_document_) {
    new_child->AttachToDocument(owner_document_.get());
  }

  return new_child;
}

// Algorithm for RemoveChild:
//   http://www.w3.org/TR/2014/WD-dom-20140710/#concept-node-remove
scoped_refptr<Node> Node::RemoveChild(const scoped_refptr<Node>& node) {
  // Custom, not in any spec.
  if (!node) {
    // TODO(***REMOVED***): Throw JS ReferenceError.
    return NULL;
  }

  // Pre-remove 1. If child's parent is not parent, throw a "NotFoundError"
  //               exception.
  if (node->parent_ != this) {
    // TODO(***REMOVED***): Throw JS NotFoundError.
    return NULL;
  }

  // Custom, not in any spec.
  if (node->owner_document_) {
    node->DetachFromDocument();
  }
  node->UpdateNodeGeneration();

  // 2. ~ 7. Not needed by Cobalt.

  // 8. Remove node from its parent.
  if (node->previous_sibling_) {
    node->previous_sibling_->next_sibling_ = node->next_sibling_;
  } else {
    first_child_ = node->next_sibling_;
  }
  if (node->next_sibling_) {
    node->next_sibling_->previous_sibling_ = node->previous_sibling_;
  } else {
    last_child_ = node->previous_sibling_;
  }
  node->parent_.reset();
  node->previous_sibling_.reset();
  node->next_sibling_ = NULL;

  return node;
}

// Algorithm for ReplaceChild:
//   http://www.w3.org/TR/2014/WD-dom-20140710/#concept-node-replace
scoped_refptr<Node> Node::ReplaceChild(const scoped_refptr<Node>& node,
                                       const scoped_refptr<Node>& child) {
  // Custom, not in any spec.
  if (!node || !child) {
    // TODO(***REMOVED***): Throw JS ReferenceError.
    return NULL;
  }
  if (child == node) {
    return node;
  }

  // 1. If parent is not a Document, DocumentFragment, or Element node,
  //    throw a "HierarchyRequestError".
  if (!child->parent_->IsDocument() && !child->parent_->IsElement()) {
    // TODO(***REMOVED***): Throw JS HierarchyRequestError.
    return NULL;
  }

  // 3. If child's parent is not parent, throw a "NotFoundError" exception.
  if (child->parent_ != this) {
    // TODO(***REMOVED***): Throw JS NotFoundError.
    return NULL;
  }

  // 4. If node is not a DocumentFragment, DocumentType, Element, Text,
  //    ProcessingInstruction, or Comment node, throw a "HierarchyRequestError".
  if (!node->IsElement() && !node->IsText() && !node->IsComment()) {
    // TODO(***REMOVED***): Throw JS HierarchyRequestError.
    return NULL;
  }

  // 5. If either node is a Text node and parent is a document, or node is a
  //    doctype and parent is not a document, throw a "HierarchyRequestError".
  if (node->IsText() && node->parent_->IsDocument()) {
    // TODO(***REMOVED***): Throw JS HierarchyRequestError.
    return NULL;
  }

  // 6. Not needed by Cobalt.

  // 7. Let reference child be child's next sibling.
  scoped_refptr<Node> reference_child = child->next_sibling_;

  // 8. If reference child is node, set it to node's next sibling.
  if (reference_child == node) reference_child = node->next_sibling_;

  // 9. Not needed by Cobalt.

  // 10. Remove child from its parent with the suppress observers flag set.
  RemoveChild(child);

  // 11. Insert node into parent before reference child with the suppress
  //     observers flag set.
  InsertBefore(node, reference_child);

  return child;
}

scoped_refptr<Document> Node::owner_document() { return owner_document_.get(); }

std::string Node::text_content() const {
  std::string content;

  const Node* child = first_child_;
  while (child) {
    if (!child->IsComment()) {
      content.append(child->text_content());
    }
    child = child->next_sibling_;
  }

  return content;
}

void Node::set_text_content(const std::string& value) {
  // Remove all children and replace them with a single Text node.
  while (HasChildNodes()) {
    RemoveChild(first_child());
  }
  AppendChild(Text::Create(value));
}

scoped_refptr<HTMLCollection> Node::Children() {
  return HTMLCollection::CreateWithChildElements(this);
}

scoped_refptr<Element> Node::FirstElementChild() {
  Node* child = first_child();
  while (child) {
    scoped_refptr<Element> element = child->AsElement();
    if (element) {
      return element;
    }
    child = child->next_sibling();
  }
  return NULL;
}

scoped_refptr<Element> Node::LastElementChild() {
  Node* child = last_child();
  while (child) {
    scoped_refptr<Element> element = child->AsElement();
    if (element) {
      return element;
    }
    child = child->previous_sibling();
  }
  return NULL;
}

unsigned int Node::ChildElementCount() {
  unsigned int num_elements = 0;
  const Node* child = first_child();
  while (child) {
    if (child->IsElement()) {
      ++num_elements;
    }
    child = child->next_sibling();
  }
  return num_elements;
}

scoped_refptr<Comment> Node::AsComment() { return NULL; }

scoped_refptr<Document> Node::AsDocument() { return NULL; }

scoped_refptr<Element> Node::AsElement() { return NULL; }

scoped_refptr<Text> Node::AsText() { return NULL; }

Node::Node() : node_generation_(kInitialNodeGeneration) {
  DOMStats::GetInstance()->Add(this);
}

Node::~Node() { DOMStats::GetInstance()->Remove(this); }

void Node::AttachToDocument(Document* document) {
  DCHECK_EQ(static_cast<Document*>(NULL), owner_document_.get());
  owner_document_ = base::AsWeakPtr(document);

  Node* child = first_child_;
  while (child) {
    child->AttachToDocument(document);
    child = child->next_sibling_;
  }

  owner_document()->RecordMutation();
}

void Node::DetachFromDocument() {
  DCHECK_NE(static_cast<Document*>(NULL), owner_document_.get());
  scoped_refptr<Document> previous_owner_document = owner_document();
  owner_document_.reset();

  Node* child = first_child_;
  while (child) {
    child->DetachFromDocument();
    child = child->next_sibling_;
  }

  previous_owner_document->RecordMutation();
}

bool Node::CheckAcceptAsChild(const scoped_refptr<Node>& child) const {
  return true;
}

void Node::UpdateNodeGeneration() {
  if (++node_generation_ == kInvalidNodeGeneration) {
    node_generation_ = kInitialNodeGeneration;
  }
  if (parent_) {
    parent_->UpdateNodeGeneration();
  }
}

}  // namespace dom
}  // namespace cobalt
