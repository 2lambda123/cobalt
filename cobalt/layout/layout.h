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

#ifndef LAYOUT_LAYOUT_H_
#define LAYOUT_LAYOUT_H_

#include "cobalt/browser/dom/html_element.h"
#include "cobalt/math/size_f.h"
#include "cobalt/render_tree/node.h"
#include "cobalt/render_tree/resource_provider.h"

namespace cobalt {
namespace layout {

// Layout engine supports a subset of CSS as defined by ***REMOVED***cobalt-css.
//
// Most of the code conforms to CSS Level 3 specifications, although the basic
// box model is intentionally implemented after CSS 2.1
// (http://www.w3.org/TR/CSS2/visuren.html) as recommended by a newer draft
// (http://dev.w3.org/csswg/css-box/) which is undergoing active changes.

// Main entry point to the layout engine.
// Produces the render tree which is a result of recursive layout of
// the given HTML element.
scoped_refptr<render_tree::Node> Layout(
    const scoped_refptr<browser::HTMLElement>& root_element,
    const math::SizeF& viewport_size,
    render_tree::ResourceProvider* resource_provider);

}  // namespace layout
}  // namespace cobalt

#endif  // LAYOUT_LAYOUT_H_
