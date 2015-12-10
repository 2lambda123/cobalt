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

#include "cobalt/layout/anonymous_block_box.h"
#include "cobalt/layout/inline_formatting_context.h"
#include "cobalt/layout/used_style.h"

namespace cobalt {
namespace layout {

AnonymousBlockBox::AnonymousBlockBox(
    const scoped_refptr<cssom::ComputedStyleState>& computed_style_state,
    UsedStyleProvider* used_style_provider)
    : BlockContainerBox(computed_style_state, used_style_provider),
      used_font_(used_style_provider->GetUsedFontList(
          computed_style_state->style()->font_family(),
          computed_style_state->style()->font_size(),
          computed_style_state->style()->font_style(),
          computed_style_state->style()->font_weight())) {}

Box::Level AnonymousBlockBox::GetLevel() const { return kBlockLevel; }

AnonymousBlockBox* AnonymousBlockBox::AsAnonymousBlockBox() { return this; }

void AnonymousBlockBox::SplitBidiLevelRuns() {
  ContainerBox::SplitBidiLevelRuns();

  for (Boxes::const_iterator child_box_iterator = child_boxes().begin();
       child_box_iterator != child_boxes().end();) {
    Box* child_box = *child_box_iterator;
    ++child_box_iterator;

    scoped_refptr<Box> child_box_after_split =
        child_box->TrySplitAtSecondBidiLevelRun();
    if (child_box_after_split) {
      child_box_iterator =
          InsertDirectChild(child_box_iterator, child_box_after_split);
    }
  }
}

bool AnonymousBlockBox::HasTrailingLineBreak() const {
  return !child_boxes().empty() && child_boxes().back()->HasTrailingLineBreak();
}

bool AnonymousBlockBox::TryAddChild(const scoped_refptr<Box>& /*child_box*/) {
  NOTREACHED();
  return false;
}

void AnonymousBlockBox::AddInlineLevelChild(
    const scoped_refptr<Box>& child_box) {
  DCHECK(child_box->GetLevel() == kInlineLevel ||
         child_box->IsAbsolutelyPositioned());
  PushBackDirectChild(child_box);
}

#ifdef COBALT_BOX_DUMP_ENABLED

void AnonymousBlockBox::DumpClassName(std::ostream* stream) const {
  *stream << "AnonymousBlockBox ";
}

#endif  // COBALT_BOX_DUMP_ENABLED

scoped_ptr<FormattingContext> AnonymousBlockBox::UpdateRectOfInFlowChildBoxes(
    const LayoutParams& child_layout_params) {
  // Lay out child boxes in the normal flow.
  //   http://www.w3.org/TR/CSS21/visuren.html#normal-flow
  scoped_ptr<InlineFormattingContext> inline_formatting_context(
      new InlineFormattingContext(
          computed_style()->line_height(), used_font_->GetFontMetrics(),
          child_layout_params, computed_style()->text_align(),
          computed_style()->white_space(),
          GetUsedLength(computed_style()->text_indent())));

  for (Boxes::const_iterator child_box_iterator = child_boxes().begin();
       child_box_iterator != child_boxes().end();) {
    Box* child_box = *child_box_iterator;
    ++child_box_iterator;

    if (child_box->IsAbsolutelyPositioned()) {
      inline_formatting_context->BeginEstimateStaticPosition(child_box);
    } else {
      scoped_refptr<Box> child_box_after_split =
          inline_formatting_context->BeginUpdateRectAndMaybeSplit(child_box);
      if (child_box_after_split) {
        // Re-insert the rest of the child box and attempt to lay it out in
        // the next iteration of the loop.
        // TODO(***REMOVED***): If every child box is split, this becomes an O(N^2)
        //               operation where N is the number of child boxes.
        //               Consider using a new vector instead and swap its
        //               contents after the layout is done.
        child_box_iterator =
            InsertDirectChild(child_box_iterator, child_box_after_split);
      }
    }
  }
  inline_formatting_context->EndUpdates();
  return inline_formatting_context.PassAs<FormattingContext>();
}

}  // namespace layout
}  // namespace cobalt
