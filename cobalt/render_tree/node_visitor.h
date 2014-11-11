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

#ifndef RENDER_TREE_NODE_VISITOR_H_
#define RENDER_TREE_NODE_VISITOR_H_

namespace cobalt {
namespace render_tree {

class ContainerNode;
class ImageNode;
class RectNode;
class TextNode;

// Type-safe branching on a class hierarchy of render tree nodes,
// implemented after a classical GoF pattern (see
// http://en.wikipedia.org/wiki/Visitor_pattern#Java_example).
class NodeVisitor {
 public:
  virtual void Visit(ContainerNode* container) = 0;
  virtual void Visit(ImageNode* image) = 0;
  virtual void Visit(RectNode* rect) = 0;
  virtual void Visit(TextNode* text) = 0;

 protected:
  ~NodeVisitor() {}
};

}  // namespace render_tree
}  // namespace cobalt

#endif  // RENDER_TREE_NODE_VISITOR_H_
