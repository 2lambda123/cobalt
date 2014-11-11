# Copyright 2014 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

{
  'targets': [
    # Compiles code related to the definition of the render tree.  The render
    # tree is the output of the layout stage and the input to the rendering
    # stage.
    {
      'target_name': 'render_tree',
      'type': 'static_library',
      'sources': [
        'border.h',
        'border_radiuses.h',
        'brush.cc',
        'brush.h',
        'brush_visitor.h',
        'clip_region.h',
        'container_node.cc',
        'container_node.h',
        'image_node.cc',
        'image_node.h',
        'node.h',
        'node_visitor.h',
        'rect_node.cc',
        'rect_node.h',
        'text_node.cc',
        'text_node.h',
      ],
      'dependencies': [
        '<(DEPTH)/cobalt/base/base.gyp:base',
        '<(DEPTH)/cobalt/math/math.gyp:math',
      ],
    },

    # Tests the render tree utility functionality.
    {
      'target_name': 'render_tree_test',
      'type': '<(gtest_target_type)',
      'sources': [
        'brush_visitor_test.cc',
        'node_visitor_test.cc',
      ],
      'dependencies': [
        '<(DEPTH)/base/base.gyp:run_all_unittests',
        '<(DEPTH)/cobalt/browser/testing/testing.gyp:browser_testing',
        'render_tree',
      ],
    },

    # Deploys the render tree library test on a console.
    {
      'target_name': 'render_tree_test_deploy',
      'type': 'none',
      'dependencies': [
        'render_tree_test',
      ],
      'variables': {
        'executable_name': 'render_tree_test',
      },
      'includes': [ '../build/deploy.gypi' ],
    },
  ],
}
