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

#include "cobalt/layout/block_formatting_block_container_box.h"

#include "cobalt/cssom/computed_style.h"
#include "cobalt/cssom/keyword_value.h"
#include "cobalt/layout/anonymous_block_box.h"
#include "cobalt/layout/block_formatting_context.h"
#include "cobalt/layout/used_style.h"

namespace cobalt {
namespace layout {

BlockFormattingBlockContainerBox::BlockFormattingBlockContainerBox(
    const scoped_refptr<cssom::ComputedStyleState>& computed_style_state,
    UsedStyleProvider* used_style_provider)
    : BlockContainerBox(computed_style_state, used_style_provider) {}

bool BlockFormattingBlockContainerBox::TryAddChild(
    const scoped_refptr<Box>& child_box) {
  AddChild(child_box);
  return true;
}

void BlockFormattingBlockContainerBox::AddChild(
    const scoped_refptr<Box>& child_box) {
  switch (child_box->GetLevel()) {
    case kBlockLevel:
      if (!child_box->IsAbsolutelyPositioned()) {
        PushBackDirectChild(child_box);
        break;
      }
    // Fall through if child is out-of-flow.

    case kInlineLevel:
      // An inline formatting context required,
      // add a child to an anonymous block box.
      GetOrAddAnonymousBlockBox()->AddInlineLevelChild(child_box);
      break;
  }
}

scoped_ptr<FormattingContext>
BlockFormattingBlockContainerBox::UpdateRectOfInFlowChildBoxes(
    const LayoutParams& child_layout_params) {
  // Lay out child boxes in the normal flow.
  //   http://www.w3.org/TR/CSS21/visuren.html#normal-flow
  scoped_ptr<BlockFormattingContext> block_formatting_context(
      new BlockFormattingContext(child_layout_params));
  for (Boxes::const_iterator child_box_iterator = child_boxes().begin();
       child_box_iterator != child_boxes().end(); ++child_box_iterator) {
    Box* child_box = *child_box_iterator;
    if (child_box->IsAbsolutelyPositioned()) {
      block_formatting_context->EstimateStaticPosition(child_box);
    } else {
      block_formatting_context->UpdateRect(child_box);
    }
  }
  return block_formatting_context.PassAs<FormattingContext>();
}

AnonymousBlockBox*
BlockFormattingBlockContainerBox::GetLastChildAsAnonymousBlockBox() {
  return child_boxes().empty() ? NULL
                               : child_boxes().back()->AsAnonymousBlockBox();
}

AnonymousBlockBox*
BlockFormattingBlockContainerBox::GetOrAddAnonymousBlockBox() {
  AnonymousBlockBox* anonymous_block_box = GetLastChildAsAnonymousBlockBox();
  // If either the last box is not an anonymous block box, or the anonymous
  // block box already has a trailing line break and can't accept any additional
  // children, then create a new anonymous block box.
  if (anonymous_block_box == NULL ||
      anonymous_block_box->HasTrailingLineBreak()) {
    // TODO(***REMOVED***): Determine which animations to propagate to the
    //               anonymous block box, instead of none at all.
    scoped_refptr<cssom::ComputedStyleState> computed_style_state =
        new cssom::ComputedStyleState();
    computed_style_state->set_style(
        GetComputedStyleOfAnonymousBox(computed_style()));
    computed_style_state->set_animations(new web_animations::AnimationSet());
    scoped_refptr<AnonymousBlockBox> new_anonymous_block_box(
        new AnonymousBlockBox(computed_style_state, used_style_provider()));
    anonymous_block_box = new_anonymous_block_box.get();
    PushBackDirectChild(new_anonymous_block_box);
  }
  return anonymous_block_box;
}

BlockLevelBlockContainerBox::BlockLevelBlockContainerBox(
    const scoped_refptr<cssom::ComputedStyleState>& computed_style_state,
    UsedStyleProvider* used_style_provider)
    : BlockFormattingBlockContainerBox(computed_style_state,
                                       used_style_provider) {}

BlockLevelBlockContainerBox::~BlockLevelBlockContainerBox() {}

Box::Level BlockLevelBlockContainerBox::GetLevel() const { return kBlockLevel; }

#ifdef COBALT_BOX_DUMP_ENABLED

void BlockLevelBlockContainerBox::DumpClassName(std::ostream* stream) const {
  *stream << "BlockLevelBlockContainerBox ";
}

#endif  // COBALT_BOX_DUMP_ENABLED

InlineLevelBlockContainerBox::InlineLevelBlockContainerBox(
    const scoped_refptr<cssom::ComputedStyleState>& computed_style_state,
    const scoped_refptr<Paragraph>& paragraph, int32 text_position,
    UsedStyleProvider* used_style_provider)
    : BlockFormattingBlockContainerBox(computed_style_state,
                                       used_style_provider),
      paragraph_(paragraph),
      text_position_(text_position) {}

InlineLevelBlockContainerBox::~InlineLevelBlockContainerBox() {}

Box::Level InlineLevelBlockContainerBox::GetLevel() const {
  return kInlineLevel;
}

base::optional<int> InlineLevelBlockContainerBox::GetBidiLevel() const {
  return paragraph_->GetBidiLevel(text_position_);
}

#ifdef COBALT_BOX_DUMP_ENABLED

void InlineLevelBlockContainerBox::DumpClassName(std::ostream* stream) const {
  *stream << "InlineLevelBlockContainerBox ";
}

void InlineLevelBlockContainerBox::DumpProperties(std::ostream* stream) const {
  BlockContainerBox::DumpProperties(stream);

  *stream << "text_position=" << text_position_ << " "
          << "bidi_level=" << paragraph_->GetBidiLevel(text_position_) << " ";
}

#endif  // COBALT_BOX_DUMP_ENABLED

}  // namespace layout
}  // namespace cobalt
