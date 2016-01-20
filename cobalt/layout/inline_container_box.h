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

#ifndef LAYOUT_INLINE_CONTAINER_BOX_H_
#define LAYOUT_INLINE_CONTAINER_BOX_H_

#include "cobalt/dom/font_list.h"
#include "cobalt/layout/container_box.h"

namespace cobalt {
namespace layout {

// The CSS 2.1 specification defines an inline box as an inline-level box whose
// contents participate in its containing inline formatting context. In fact,
// this definition matches two different types of boxes:
//   - a text box;
//   - an inline-level container box that contains other inline-level boxes
//     (including text boxes).
// This class implements the latter.
//
// Note that "inline box" and "inline-level box" are two different concepts.
// Inline-level boxes that are not inline boxes (such as inline-block elements)
// are called atomic inline-level boxes because they participate in their inline
// formatting context as a single opaque box.
//   https://www.w3.org/TR/CSS21/visuren.html#inline-boxes
class InlineContainerBox : public ContainerBox {
 public:
  InlineContainerBox(
      const scoped_refptr<cssom::ComputedStyleState>& computed_style_state,
      UsedStyleProvider* used_style_provider);
  ~InlineContainerBox() OVERRIDE;

  // From |Box|.
  Level GetLevel() const OVERRIDE;

  void UpdateContentSizeAndMargins(const LayoutParams& layout_params) OVERRIDE;
  scoped_refptr<Box> TrySplitAt(float available_width,
                                bool allow_overflow) OVERRIDE;

  bool DoesFulfillEllipsisPlacementRequirement() const OVERRIDE;
  void ResetEllipses() OVERRIDE;

  scoped_refptr<Box> TrySplitAtSecondBidiLevelRun() OVERRIDE;
  base::optional<int> GetBidiLevel() const OVERRIDE;

  void SetShouldCollapseLeadingWhiteSpace(
      bool should_collapse_leading_white_space) OVERRIDE;
  void SetShouldCollapseTrailingWhiteSpace(
      bool should_collapse_trailing_white_space) OVERRIDE;
  bool HasLeadingWhiteSpace() const OVERRIDE;
  bool HasTrailingWhiteSpace() const OVERRIDE;
  bool IsCollapsed() const OVERRIDE;

  bool JustifiesLineExistence() const OVERRIDE;
  bool HasTrailingLineBreak() const OVERRIDE;
  bool AffectsBaselineInBlockFormattingContext() const OVERRIDE;
  float GetBaselineOffsetFromTopMarginEdge() const OVERRIDE;

  // From |ContainerBox|.
  bool TryAddChild(const scoped_refptr<Box>& child_box) OVERRIDE;
  scoped_refptr<ContainerBox> TrySplitAtEnd() OVERRIDE;

 protected:
  // From |Box|.
  bool IsTransformable() const OVERRIDE;

#ifdef COBALT_BOX_DUMP_ENABLED
  void DumpClassName(std::ostream* stream) const OVERRIDE;
  void DumpProperties(std::ostream* stream) const OVERRIDE;
#endif  // COBALT_BOX_DUMP_ENABLED

 private:
  // From |Box|.
  void DoPlaceEllipsisOrProcessPlacedEllipsis(
      float desired_offset, bool* is_placement_requirement_met, bool* is_placed,
      float* placed_offset) OVERRIDE;

  scoped_refptr<Box> SplitAtIterator(
      Boxes::const_iterator child_split_iterator);

  bool should_collapse_leading_white_space_;
  bool should_collapse_trailing_white_space_;
  bool has_leading_white_space_;
  bool has_trailing_white_space_;
  bool is_collapsed_;

  bool justifies_line_existence_;
  float baseline_offset_from_margin_box_top_;
  // A font used for text width and line height calculations.
  const scoped_refptr<dom::FontList> used_font_;

  DISALLOW_COPY_AND_ASSIGN(InlineContainerBox);
};

}  // namespace layout
}  // namespace cobalt

#endif  // LAYOUT_INLINE_CONTAINER_BOX_H_
