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

#ifndef LAYOUT_LAYOUT_MANAGER_H_
#define LAYOUT_LAYOUT_MANAGER_H_

#include "base/callback.h"
#include "base/memory/ref_counted.h"
#include "cobalt/browser/dom/document.h"
#include "cobalt/render_tree/node.h"
#include "cobalt/render_tree/resource_provider.h"

namespace cobalt {
namespace layout {

// Produces the render tree each time when the document needs layout update.
class LayoutManager : public browser::DocumentObserver {
 public:
  typedef base::Callback<void(const scoped_refptr<render_tree::Node>&)>
      OnRenderTreeProducedCallback;

  LayoutManager(const scoped_refptr<browser::Document>& document,
                const math::SizeF& viewport_size,
                render_tree::ResourceProvider* resource_provider,
                const OnRenderTreeProducedCallback& on_render_tree_produced);
  ~LayoutManager();

  void OnLoad() OVERRIDE;
  void OnMutation() OVERRIDE;

 private:
  const scoped_refptr<browser::Document> document_;
  const math::SizeF viewport_size_;
  render_tree::ResourceProvider* const resource_provider_;
  const OnRenderTreeProducedCallback on_render_tree_produced_callback_;
};

}  // namespace layout
}  // namespace cobalt

#endif  // LAYOUT_LAYOUT_MANAGER_H_
