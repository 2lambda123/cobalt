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
    'graphics_system_stub_sources': [
      'display_stub.h',
      'graphics_context_stub.h',
      'graphics_system_stub.h',
      'render_target_stub.h',
      'texture_stub.h',
    ],
  },

  'targets': [
    {
      'target_name': 'renderer_backend',
      'type': 'static_library',
      'sources': [
        'default_graphics_system.h',
        'display.h',
        'graphics_context.h',
        'graphics_system.h',
        'render_target.h',
        'surface_info.h',
        'texture.h',
        '<@(graphics_system_stub_sources)'
      ],

      'dependencies': [
        '<(DEPTH)/cobalt/base/base.gyp:base',
        '<(DEPTH)/cobalt/math/math.gyp:math',
        '<(DEPTH)/cobalt/renderer/backend/<(actual_target_arch)/platform_backend.gyp:renderer_platform_backend',
      ],
    },
  ],
}
