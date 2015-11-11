# Copyright 2015 Google Inc. All Rights Reserved.
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
      'target_name': 'system_window',
      'type': 'static_library',
      'sources': [
        'application_event.h',
        'keyboard_event.h',
        'system_window.h',
        'system_window_common.cc',
      ],
      'dependencies': [
        '<(DEPTH)/cobalt/base/base.gyp:base',
        '<(DEPTH)/cobalt/system_window/<(actual_target_arch)/platform_system_window.gyp:platform_system_window',
      ],
    },
  ],
}
