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

#ifndef LAYOUT_BOX_H_
#define LAYOUT_BOX_H_

#include "cobalt/cssom/css_style_declaration.h"
#include "cobalt/math/rect_f.h"
#include "cobalt/render_tree/composition_node.h"

namespace cobalt {
namespace layout {

class ContainingBlock;
class UsedStyleProvider;

struct LayoutOptions {
  // Indicates whether box is first on the line.
  bool beginning_of_line;
};

// Base class for all boxes.
//
// Box is a central concept of CSS basic box model
// (see http://www.w3.org/TR/CSS2/box.html).
// Layout engine, given DOM and CSSOM, produces a box tree.
class Box {
 public:
  Box(ContainingBlock* containing_block,
      const scoped_refptr<cssom::CSSStyleDeclaration>& computed_style,
      UsedStyleProvider* used_style_provider);

  // In CSS, many box positions and sizes are calculated with respect to
  // the edges of a rectangular box called a containing block.
  //   http://www.w3.org/TR/CSS2/visuren.html#containing-block
  ContainingBlock* containing_block() const { return containing_block_; }

  // Computed style contains CSS values from the last stage of processing
  // before the layout. The computed value resolves the specified value as far
  // as possible without laying out the document or performing other expensive
  // or hard-to-parallelize operations, such as resolving network requests or
  // retrieving values other than from the element and its parent.
  //   http://www.w3.org/TR/css-cascade-3/#computed
  scoped_refptr<cssom::CSSStyleDeclaration> computed_style() const {
    return computed_style_;
  }

  // Reflects the used values of left, top, right, bottom, width, and height.
  math::RectF& used_frame() { return used_frame_; }
  const math::RectF& used_frame() const { return used_frame_; }

  // Lays out the box and all its descendants recursively.
  virtual void Layout(const LayoutOptions& options) = 0;

  // Converts layout subtree into render subtree.
  virtual void AddToRenderTree(
      render_tree::CompositionNodeMutable* composition_node);

 protected:
  UsedStyleProvider* used_style_provider() const {
    return used_style_provider_;
  }

  math::Matrix3F GetTransform();

 private:
  ContainingBlock* const containing_block_;
  const scoped_refptr<cssom::CSSStyleDeclaration> computed_style_;
  UsedStyleProvider* const used_style_provider_;

  math::RectF used_frame_;
};

}  // namespace layout
}  // namespace cobalt

#endif  // LAYOUT_BOX_H_
