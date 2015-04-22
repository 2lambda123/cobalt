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

#ifndef CSSOM_EMPTY_PSEUDO_CLASS_H_
#define CSSOM_EMPTY_PSEUDO_CLASS_H_

#include <string>

#include "base/compiler_specific.h"
#include "cobalt/cssom/selector.h"

namespace cobalt {
namespace cssom {

// The :empty pseudo-class represents an element that has no children. In terms
// of the document tree, only element nodes and content nodes (such as DOM
// text nodes, CDATA nodes, and entity references) whose data has a non-zero
// length must be considered as affecting emptiness; comments, processing
// instructions, and other nodes must not affect whether an element is
// considered empty or not.
//   http://www.w3.org/TR/selectors4/#empty-pseudo
class EmptyPseudoClass : public Selector {
 public:
  EmptyPseudoClass() {}
  ~EmptyPseudoClass() OVERRIDE {}

  Specificity GetSpecificity() const OVERRIDE { return Specificity(0, 1, 0); }

  void Accept(SelectorVisitor* visitor) OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(EmptyPseudoClass);
};

}  // namespace cssom
}  // namespace cobalt

#endif  // CSSOM_EMPTY_PSEUDO_CLASS_H_
