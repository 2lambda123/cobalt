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

#include "cobalt/render_tree/rect_node.h"

#include "cobalt/render_tree/brush_visitor.h"
#include "cobalt/render_tree/node_visitor.h"

namespace cobalt {
namespace render_tree {

RectNode::Builder::Builder(const math::SizeF& size, scoped_ptr<Border> border)
    : size(size), border(border.Pass()) {}

RectNode::Builder::Builder(const math::SizeF& size,
                           scoped_ptr<Brush> background_brush)
    : size(size), background_brush(background_brush.Pass()) {}

RectNode::Builder::Builder(const math::SizeF& size,
                           scoped_ptr<Brush> background_brush,
                           scoped_ptr<Border> border)
    : size(size),
      background_brush(background_brush.Pass()),
      border(border.Pass()) {}

RectNode::Builder::Builder(const Builder& other) {
  size = other.size;

  if (other.background_brush) {
    background_brush = CloneBrush(other.background_brush.get());
  }

  if (other.border) {
    border.reset(new Border(other.border->left, other.border->right,
                            other.border->top, other.border->bottom));
  }
}

RectNode::Builder::Builder(Moved moved)
    : size(moved->size),
      background_brush(moved->background_brush.Pass()),
      border(moved->border.Pass()) {}

void RectNode::Accept(NodeVisitor* visitor) { visitor->Visit(this); }

math::RectF RectNode::GetBounds() const { return math::RectF(data_.size); }

}  // namespace render_tree
}  // namespace cobalt
