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
  'variables': {
    'cobalt_code': 1,
  },
  'targets': [
    {
      'target_name': 'cobalt',
      'type': '<(final_executable_type)',
      'sources': [
        'main.cc',
      ],
      'dependencies': [
        '<(DEPTH)/cobalt/trace_event/trace_event.gyp:trace_event',
        '<(DEPTH)/cobalt/xhr/xhr.gyp:xhr_copy_test_data',
        'browser',
      ],
      'conditions': [
        ['cobalt_copy_test_data == 1', {
          'dependencies': ['browser_copy_test_data'],
        }],
      ],
    },

    {
      'target_name': 'cobalt_deploy',
      'type': 'none',
      'dependencies': [
        'cobalt',
      ],
      'variables': {
        'executable_name': 'cobalt',
      },
      'includes': [ '../build/deploy.gypi' ],
    },

    {
      'target_name': 'browser',
      'type': 'static_library',
      'sources': [
        'application.cc',
        'application.h',
        'browser_module.cc',
        'browser_module.h',
        'switches.cc',
        'switches.h',
        'web_module.cc',
        'web_module.h',
      ],
      'dependencies': [
        '<(DEPTH)/cobalt/base/base.gyp:base',
        '<(DEPTH)/cobalt/bindings/browser.gyp:bindings',
        '<(DEPTH)/cobalt/browser/<(actual_target_arch)/platform_browser.gyp:platform_browser',
        '<(DEPTH)/cobalt/css_parser/css_parser.gyp:css_parser',
        '<(DEPTH)/cobalt/dom/dom.gyp:dom',
        '<(DEPTH)/cobalt/dom_parser/dom_parser.gyp:dom_parser',
        '<(DEPTH)/cobalt/input/input.gyp:input',
        '<(DEPTH)/cobalt/layout/layout.gyp:layout',
        '<(DEPTH)/cobalt/math/math.gyp:math',
        '<(DEPTH)/cobalt/media/media.gyp:media',
        '<(DEPTH)/cobalt/network/network.gyp:network',
        '<(DEPTH)/cobalt/renderer/renderer.gyp:renderer',
        '<(DEPTH)/cobalt/script/javascriptcore.gyp:engine',
        '<(DEPTH)/cobalt/xhr/xhr.gyp:xhr',
        '<(DEPTH)/googleurl/googleurl.gyp:googleurl',
      ],
    },

    {
      'target_name': 'browser_copy_test_data',
      'type': 'none',
      'actions': [
        {
          'action_name': 'browser_copy_test_data',
          'variables': {
            'input_files': [
              '<(DEPTH)/cobalt/browser/testdata/',
            ],
            'output_dir': 'cobalt/browser/testdata/',
          },
          'includes': [ '../build/copy_data.gypi' ],
        },
      ],
    },
  ],
}
