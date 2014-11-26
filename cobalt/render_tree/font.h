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

#ifndef RENDER_TREE_FONT_H_
#define RENDER_TREE_FONT_H_

#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "cobalt/math/size_f.h"

namespace cobalt {
namespace render_tree {

// The Font class is an abstract base class representing all information
// required by the rasterizer to determine the font metrics for a given
// string of text.  Typically this implies that a font typeface, size, and
// style must at least be described when instantiating a concrete Font derived
// class.  Since Font objects may be created in the frontend, but must be
// accessed by the rasterizer, it is expected that they will be downcast again
// to a rasterizer-specific type through base::Downcast().
class Font : public base::RefCountedThreadSafe<Font> {
 public:
  // A position and size of the given text. Size is guaranteeed to be
  // consistent with the given font and include every glyph.
  // The return value is given in units of pixels.
  virtual math::SizeF GetBounds(const std::string& text) const = 0;

 protected:
  virtual ~Font() {}

  // Allow the reference counting system access to our destructor.
  friend class base::RefCountedThreadSafe<Font>;
};

}  // namespace render_tree
}  // namespace cobalt

#endif  // RENDER_TREE_FONT_H_
