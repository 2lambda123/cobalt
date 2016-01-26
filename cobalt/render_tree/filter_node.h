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

#ifndef COBALT_RENDER_TREE_FILTER_NODE_H_
#define COBALT_RENDER_TREE_FILTER_NODE_H_

#include "base/compiler_specific.h"
#include "base/optional.h"
#include "cobalt/base/type_id.h"
#include "cobalt/render_tree/node.h"
#include "cobalt/render_tree/opacity_filter.h"
#include "cobalt/render_tree/shadow.h"
#include "cobalt/render_tree/viewport_filter.h"

namespace cobalt {
namespace render_tree {

// Wrapping a render tree inside of a FilterMode expresses the desire to modify
// the image produced by the source render tree by passing it through a set
// of filters, such as an opacity filter which makes the resulting image
// transparent.
class FilterNode : public Node {
 public:
  struct Builder {
    explicit Builder(const scoped_refptr<render_tree::Node>& source);

    Builder(const OpacityFilter& opacity_filter,
            const scoped_refptr<render_tree::Node>& source);

    Builder(const ViewportFilter& viewport_filter,
            const scoped_refptr<render_tree::Node>& source);

    Builder(const Shadow& shadow_filter,
            const scoped_refptr<render_tree::Node>& source);

    // The source tree, which will be used as the input to the filters specified
    // in this FilterNode.
    scoped_refptr<render_tree::Node> source;

    // If set, this filter will make the source subtree appear transparent,
    // with the level of transparency dictated by the OpacityFilter's value.
    base::optional<OpacityFilter> opacity_filter;

    // If set, this filter will specify the viewport of source content. Only
    // the source content within the viewport rectangle will be rendered.
    // Rounded corners may be specified on this filter.
    base::optional<ViewportFilter> viewport_filter;

    // If this is set, then a drop shadow will be applied to the filter source
    // based on the alpha of the source.
    base::optional<Shadow> shadow_filter;
  };

  explicit FilterNode(const Builder& builder) : data_(builder) {}

  FilterNode(const OpacityFilter& opacity_filter,
             const scoped_refptr<render_tree::Node>& source);

  FilterNode(const ViewportFilter& viewport_filter,
             const scoped_refptr<render_tree::Node>& source);

  FilterNode(const Shadow& shadow_filter,
             const scoped_refptr<render_tree::Node>& source);

  void Accept(NodeVisitor* visitor) OVERRIDE;
  math::RectF GetBounds() const OVERRIDE;

  base::TypeId GetTypeId() const OVERRIDE {
    return base::GetTypeId<FilterNode>();
  }

  const Builder& data() const { return data_; }

 private:
  const Builder data_;
};

}  // namespace render_tree
}  // namespace cobalt

#endif  // COBALT_RENDER_TREE_FILTER_NODE_H_
