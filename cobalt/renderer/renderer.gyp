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
    {
      # This target conveniently aggregates all sub-modules required to declare
      # and implement rasterization commands as well as display/GPU resource
      # management and communication.  It will eventually also contain source
      # code to help glue all of the components together.
      'target_name': 'renderer',
      'type': 'static_library',
      'sources': [
        'pipeline.cc',
        'pipeline.h',
        'rasterizer.h',
        'render_tree_backend_conversions.cc',
        'render_tree_backend_conversions.h',
        'renderer_module.cc',
        'renderer_module.h',
        'renderer_module_default_options_<(actual_target_arch).cc',
        'resource_provider.h',
      ],

      'includes': [
        'copy_font_data.gypi',
        'rasterizer_skia/rasterizer_skia.gypi'
      ],

      'dependencies': [
        '<(DEPTH)/cobalt/base/base.gyp:base',
        '<(DEPTH)/cobalt/math/math.gyp:math',
        '<(DEPTH)/cobalt/render_tree/render_tree.gyp:render_tree',
        '<(DEPTH)/cobalt/render_tree/render_tree.gyp:animations',
        '<(DEPTH)/cobalt/renderer/backend/backend.gyp:renderer_backend',
      ],
    },

    {
      # This target provides functionality for testing that rasterization
      # results for a given render tree match up with a pre-existing
      # "expected output" image that is stored offline as a file.
      'target_name': 'render_tree_pixel_tester',
      'type': 'static_library',
      'sources': [
        'render_tree_pixel_tester.cc',
        'render_tree_pixel_tester.h',
      ],
      'dependencies': [
        '<(DEPTH)/cobalt/base/base.gyp:base',
        '<(DEPTH)/cobalt/renderer/rasterizer_skia/skia/skia.gyp:skia',
        '<(DEPTH)/cobalt/renderer/test/png_utils/png_utils.gyp:png_utils',
        'renderer',
      ],
    },

    {
      'target_name': 'renderer_test',
      'type': '<(gtest_target_type)',
      'sources': [
        'animations_test.cc',
        'pipeline_test.cc',
        'rasterizer_test.cc',
        'rasterizer_test_fixture.cc',
        'rasterizer_test_fixture.h',
        'resource_provider_test.cc',
      ],
      'dependencies': [
        '<(DEPTH)/base/base.gyp:run_all_unittests',
        '<(DEPTH)/cobalt/base/base.gyp:base',
        '<(DEPTH)/testing/gmock.gyp:gmock',
        '<(DEPTH)/testing/gtest.gyp:gtest',
        'render_tree_pixel_tester',
      ],
      'actions': [
        {
          'action_name': 'renderer_copy_test_data',
          'variables': {
            'input_files': [
              '<(DEPTH)/cobalt/renderer/rasterizer_testdata',
            ],
            'output_dir': 'cobalt/renderer',
          },
          'includes': ['../build/copy_data.gypi'],
        }
      ],
    },
    {
      'target_name': 'renderer_test_deploy',
      'type': 'none',
      'dependencies': [
        'renderer_test',
      ],
      'variables': {
        'executable_name': 'renderer_test',
      },
      'includes': [ '../build/deploy.gypi' ],
    },

    {
      'target_name': 'renderer_benchmark',
      'type': '<(final_executable_type)',
      'sources': [
        'rasterizer_benchmark.cc',
      ],
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/cobalt/renderer/test/scenes/scenes.gyp:scenes',
        '<(DEPTH)/cobalt/trace_event/trace_event.gyp:run_all_benchmarks',
        'renderer',
      ],
    },
  ],
}
