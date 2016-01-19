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
 * limitations under the License.`
 */

#ifndef LAYOUT_WHITE_SPACE_PROCESSING_H_
#define LAYOUT_WHITE_SPACE_PROCESSING_H_

#include <string>
#include <vector>

#include "base/basictypes.h"

namespace cobalt {
namespace layout {

// Performs white space collapsing and transformation that correspond to
// the phase I of the white space processing.
//   https://www.w3.org/TR/css3-text/#white-space-phase-1
void CollapseWhiteSpace(std::string* text);

bool FindNextNewlineSequence(const std::string& utf8_text, size_t index,
                             size_t* sequence_start, size_t* sequence_length);

}  // namespace layout
}  // namespace cobalt

#endif  // LAYOUT_WHITE_SPACE_PROCESSING_H_
