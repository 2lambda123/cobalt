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
  'targets': [
    {
      'target_name': 'loader',
      'type': 'static_library',
      'sources': [
        'decoder.h',
        'fetcher.cc',
        'fetcher.h',
        'file_fetcher.cc',
        'file_fetcher.h',
        'loader.cc',
        'loader.h',
        'text_decoder.h',
      ],
      'dependencies': [
        '<(DEPTH)/cobalt/base/base.gyp:base',
        '<(DEPTH)/googleurl/googleurl.gyp:googleurl',
      ],
    },

    {
      'target_name': 'loader_test',
      'type': '<(gtest_target_type)',
      'sources': [
        'file_fetcher_test.cc',
        'loader_test.cc',
        'text_decoder_test.cc',
      ],
      'dependencies': [
        '<(DEPTH)/base/base.gyp:run_all_unittests',
        '<(DEPTH)/cobalt/base/base.gyp:base',
        '<(DEPTH)/testing/gmock.gyp:gmock',
        '<(DEPTH)/testing/gtest.gyp:gtest',
        'loader',
        'loader_copy_test_data',
      ],
    },

    {
      'target_name': 'loader_copy_test_data',
      'type': 'none',
      'actions': [
        {
          'action_name': 'loader_copy_test_data',
          'variables': {
            'input_files': [
              '<(DEPTH)/cobalt/loader/testdata/',
            ],
            'output_dir': 'cobalt/loader/testdata/',
          },
          'includes': [ '../build/copy_data.gypi' ],
        },
      ],
    },
  ],
}
