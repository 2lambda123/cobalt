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

#include "cobalt/layout/block_level_replaced_box.h"

#include "cobalt/layout/used_style.h"

namespace cobalt {
namespace layout {

BlockLevelReplacedBox::BlockLevelReplacedBox(
    const scoped_refptr<cssom::ComputedStyleState>& computed_style_state,
    const UsedStyleProvider* used_style_provider,
    const ReplaceImageCB& replace_image_cb,
    const scoped_refptr<Paragraph>& paragraph, int32 text_position,
    const base::optional<float>& maybe_intrinsic_width,
    const base::optional<float>& maybe_intrinsic_height,
    const base::optional<float>& maybe_intrinsic_ratio)
    : ReplacedBox(computed_style_state, used_style_provider, replace_image_cb,
                  paragraph, text_position, maybe_intrinsic_width,
                  maybe_intrinsic_height, maybe_intrinsic_ratio) {}

Box::Level BlockLevelReplacedBox::GetLevel() const { return kBlockLevel; }

void BlockLevelReplacedBox::UpdateHorizontalMargins(
    float containing_block_width, float border_box_width,
    const base::optional<float>& maybe_margin_left,
    const base::optional<float>& maybe_margin_right) {
  // Calculate the horizonal margins for block-level, replaced elements in
  // normal flow.
  //   http://www.w3.org/TR/CSS21/visudet.html#block-replaced-width
  UpdateHorizontalMarginsAssumingBlockLevelInFlowBox(
      containing_block_width, border_box_width, maybe_margin_left,
      maybe_margin_right);
}

#ifdef COBALT_BOX_DUMP_ENABLED

void BlockLevelReplacedBox::DumpClassName(std::ostream* stream) const {
  *stream << "BlockLevelReplacedBox ";
}

#endif  // COBALT_BOX_DUMP_ENABLED

}  // namespace layout
}  // namespace cobalt
