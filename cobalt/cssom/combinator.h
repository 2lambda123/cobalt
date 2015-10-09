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

#ifndef CSSOM_COMBINATOR_H_
#define CSSOM_COMBINATOR_H_

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/scoped_vector.h"

namespace cobalt {
namespace cssom {

class Selector;
class ChildCombinator;
class CombinatorVisitor;
class DescendantCombinator;
class FollowingSiblingCombinator;
class NextSiblingCombinator;

enum CombinatorType {
  kChildCombinator,
  kDescendantCombinator,
  kNextSiblingCombinator,
  kFollowingSiblingCombinator,
  kCombinatorCount,
};

// A combinator is punctuation that represents a particular kind of relationship
// between the selectors on either side.
//   http://www.w3.org/TR/selectors4/#combinator
class Combinator {
 public:
  Combinator();
  virtual ~Combinator();

  virtual void Accept(CombinatorVisitor* visitor) = 0;

  virtual CombinatorType GetCombinatorType() = 0;

  // The selector to the right of the combinator.
  Selector* selector();
  void set_selector(scoped_ptr<Selector> selector);

 private:
  scoped_ptr<Selector> selector_;

  DISALLOW_COPY_AND_ASSIGN(Combinator);
};

typedef ScopedVector<Combinator> Combinators;

}  // namespace cssom
}  // namespace cobalt

#endif  // CSSOM_COMBINATOR_H_
