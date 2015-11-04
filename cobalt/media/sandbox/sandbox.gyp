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

# This is a sample sandbox application for experimenting with the Cobalt
# media/renderer interface.

{
  'targets': [
    # This target will build a sandbox application that allows for easy
    # experimentation with the media interface on any platform.  This can
    # also be useful for visually inspecting the output that the Cobalt
    # media stack is producing.
    {
      'target_name': 'web_media_player_sandbox',
      'type': '<(final_executable_type)',
      'sources': [
        'media_sandbox.cc',
        'media_sandbox.h',
        'web_media_player_helper.cc',
        'web_media_player_helper.h',
        'web_media_player_sandbox.cc',
      ],
      'dependencies': [
        '<(DEPTH)/cobalt/base/base.gyp:base',
        # Use test data from browser to avoid keeping two copies of video files.
        '<(DEPTH)/cobalt/browser/browser.gyp:browser_copy_test_data',
        '<(DEPTH)/cobalt/loader/loader.gyp:loader',
        '<(DEPTH)/cobalt/media/media.gyp:media',
        '<(DEPTH)/cobalt/network/network.gyp:network',
        '<(DEPTH)/cobalt/renderer/renderer.gyp:renderer',
        '<(DEPTH)/cobalt/system_window/system_window.gyp:system_window',
        '<(DEPTH)/cobalt/trace_event/trace_event.gyp:trace_event',
        '<(DEPTH)/googleurl/googleurl.gyp:googleurl',
      ],
    },

    {
      'target_name': 'web_media_player_sandbox_deploy',
      'type': 'none',
      'dependencies': [
        'web_media_player_sandbox',
      ],
      'variables': {
        'executable_name': 'web_media_player_sandbox',
      },
      'includes': [ '../../build/deploy.gypi' ],
    },
  ],
  'conditions': [
    ['target_arch == "linux" or target_arch == "ps3"', {
      # This target will build a sandbox application that allows for easy
      # experimentation with the video decoder on any platform.  It simply
      # creates a video decoder and a demuxer without bringing up a whole
      # media pipeline.
      'targets': [
        {
          'target_name': 'video_decoder_sandbox',
          'type': '<(final_executable_type)',
            'sources': [
            'demuxer_helper.cc',
            'demuxer_helper.h',
            'media_sandbox.cc',
            'media_sandbox.h',
            'video_decoder_sandbox.cc',
          ],
          'dependencies': [
            '<(DEPTH)/cobalt/base/base.gyp:base',
            # Use test data from browser to avoid keeping two copies of video files.
            '<(DEPTH)/cobalt/browser/browser.gyp:browser_copy_test_data',
            '<(DEPTH)/cobalt/loader/loader.gyp:loader',
            '<(DEPTH)/cobalt/media/media.gyp:media',
            '<(DEPTH)/cobalt/network/network.gyp:network',
            '<(DEPTH)/cobalt/renderer/renderer.gyp:renderer',
            '<(DEPTH)/cobalt/system_window/system_window.gyp:system_window',
            '<(DEPTH)/cobalt/trace_event/trace_event.gyp:trace_event',
            '<(DEPTH)/googleurl/googleurl.gyp:googleurl',
          ],
        },

        {
          'target_name': 'video_decoder_sandbox_deploy',
          'type': 'none',
          'dependencies': [
            'video_decoder_sandbox',
          ],
          'variables': {
            'executable_name': 'video_decoder_sandbox',
          },
          'includes': [ '../../build/deploy.gypi' ],
        },

        # This target will build a sandbox application that allows for easy
        # experimentation with the raw video decoder on any platform.  It simply
        # creates a raw video decoder and a demuxer without even using a
        # VideoDecoder.
        {
          'target_name': 'raw_video_decoder_sandbox',
          'type': '<(final_executable_type)',
            'sources': [
            'demuxer_helper.cc',
            'demuxer_helper.h',
            'media_sandbox.cc',
            'media_sandbox.h',
            'raw_video_decoder_sandbox.cc',
          ],
          'dependencies': [
            '<(DEPTH)/cobalt/base/base.gyp:base',
            # Use test data from browser to avoid keeping two copies of video files.
            '<(DEPTH)/cobalt/browser/browser.gyp:browser_copy_test_data',
            '<(DEPTH)/cobalt/loader/loader.gyp:loader',
            '<(DEPTH)/cobalt/media/media.gyp:media',
            '<(DEPTH)/cobalt/network/network.gyp:network',
            '<(DEPTH)/cobalt/renderer/renderer.gyp:renderer',
            '<(DEPTH)/cobalt/system_window/system_window.gyp:system_window',
            '<(DEPTH)/cobalt/trace_event/trace_event.gyp:trace_event',
            '<(DEPTH)/googleurl/googleurl.gyp:googleurl',
          ],
        },

        {
          'target_name': 'raw_video_decoder_sandbox_deploy',
          'type': 'none',
          'dependencies': [
            'raw_video_decoder_sandbox',
          ],
          'variables': {
            'executable_name': 'raw_video_decoder_sandbox',
          },
          'includes': [ '../../build/deploy.gypi' ],
        },
      ],
    }],
  ],
}
