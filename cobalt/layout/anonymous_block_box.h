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

#ifndef LAYOUT_ANONYMOUS_BLOCK_BOX_H_
#define LAYOUT_ANONYMOUS_BLOCK_BOX_H_

#include "cobalt/layout/block_container_box.h"

namespace cobalt {
namespace layout {

// A block-level block container box that establish an inline formatting
// context. Anonymous block boxes are created to enclose inline-level
// children in a block formatting context.
//   http://www.w3.org/TR/CSS21/visuren.html#anonymous-block-level
class AnonymousBlockBox : public BlockContainerBox {
 public:
  AnonymousBlockBox(
      const scoped_refptr<const cssom::CSSStyleDeclarationData>& computed_style,
      const cssom::TransitionSet* transitions,
      const UsedStyleProvider* used_style_provider)
      : BlockContainerBox(computed_style, transitions, used_style_provider) {}

  // From |Box|.
  Level GetLevel() const OVERRIDE;
  AnonymousBlockBox* AsAnonymousBlockBox() OVERRIDE;

  // From |ContainerBox|.

  // This method should never be called, instead all children have to be added
  // through |AddInlineLevelChild|.
  bool TryAddChild(scoped_ptr<Box>* child_box) OVERRIDE;

  // Rest of the public methods.

  // An anonymous block box may only contain inline-level children.
  void AddInlineLevelChild(scoped_ptr<Box> child_box);

 protected:
  // From |Box|.
  void DumpClassName(std::ostream* stream) const OVERRIDE;

  // From |BlockContainerBox|.
  float GetUsedWidthBasedOnContainingBlock(
      float containing_block_width, bool* width_depends_on_containing_block,
      bool* width_depends_on_child_boxes) const OVERRIDE;
  float GetUsedHeightBasedOnContainingBlock(
      float containing_block_height,
      bool* height_depends_on_child_boxes) const OVERRIDE;
  scoped_ptr<FormattingContext> UpdateUsedRectOfChildren(
      const LayoutParams& child_layout_params) OVERRIDE;
  float GetUsedWidthBasedOnChildBoxes(
      const FormattingContext& formatting_context) const OVERRIDE;
  float GetUsedHeightBasedOnChildBoxes(
      const FormattingContext& formatting_context) const OVERRIDE;
};

}  // namespace layout
}  // namespace cobalt

#endif  // LAYOUT_ANONYMOUS_BLOCK_BOX_H_
