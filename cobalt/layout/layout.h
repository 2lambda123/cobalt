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

#ifndef COBALT_LAYOUT_LAYOUT_H_
#define COBALT_LAYOUT_LAYOUT_H_

#include "cobalt/dom/document.h"
#include "cobalt/layout/block_formatting_block_container_box.h"
#include "cobalt/loader/image/image_cache.h"
#include "cobalt/math/size_f.h"
#include "cobalt/render_tree/animations/node_animations_map.h"
#include "cobalt/render_tree/node.h"
#include "cobalt/render_tree/resource_provider.h"
#include "third_party/icu/public/common/unicode/brkiter.h"

namespace cobalt {
namespace layout {

class UsedStyleProvider;

// Layout engine supports a subset of CSS as defined by ***REMOVED***cobalt-css.
//
// Most of the code conforms to CSS Level 3 specifications, although the basic
// box model is intentionally implemented after CSS 2.1
// (https://www.w3.org/TR/CSS2/visuren.html) as recommended by a newer draft
// (http://dev.w3.org/csswg/css-box/) which is undergoing active changes.

struct RenderTreeWithAnimations {
  RenderTreeWithAnimations(
      const scoped_refptr<render_tree::Node>& render_tree,
      const scoped_refptr<render_tree::animations::NodeAnimationsMap>&
          animations)
      : render_tree(render_tree), animations(animations) {}

  scoped_refptr<render_tree::Node> render_tree;
  scoped_refptr<render_tree::animations::NodeAnimationsMap> animations;
};

// Update the computed styles, then generate and layout the box tree.
void UpdateComputedStylesAndLayoutBoxTree(
    const scoped_refptr<dom::Document>& document,
    UsedStyleProvider* used_style_provider,
    icu::BreakIterator* line_break_iterator,
    scoped_refptr<BlockLevelBlockContainerBox>* initial_containing_block);

// Main entry point to the layout engine.
// Produces the render tree (along with corresponding animations) which is a
// result of recursive layout of the given HTML element.
RenderTreeWithAnimations Layout(
    const scoped_refptr<dom::Document>& document,
    UsedStyleProvider* used_style_provider,
    icu::BreakIterator* line_break_iterator,
    scoped_refptr<BlockLevelBlockContainerBox>* initial_containing_block);

}  // namespace layout
}  // namespace cobalt

#endif  // COBALT_LAYOUT_LAYOUT_H_
