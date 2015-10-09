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
      'target_name': 'dom',
      'type': 'static_library',
      'sources': [
        'animation_frame_request_callback_list.cc',
        'animation_frame_request_callback_list.h',
        'animation_timeline.h',
        'array_buffer.cc',
        'array_buffer.h',
        'array_buffer_view.cc',
        'array_buffer_view.h',
        'attr.cc',
        'attr.h',
        'benchmark_stat_names.cc',
        'benchmark_stat_names.h',
        'character_data.cc',
        'character_data.h',
        'comment.cc',
        'comment.h',
        'console.cc',
        'console.h',
        'crypto.cc',
        'crypto.h',
        'data_view.cc',
        'data_view.h',
        'document.cc',
        'document.h',
        'document_timeline.cc',
        'document_timeline.h',
        'document_type.cc',
        'document_type.h',
        'dom_exception.cc',
        'dom_exception.h',
        'dom_implementation.cc',
        'dom_implementation.h',
        'dom_parser.cc',
        'dom_parser.h',
        'dom_settings.cc',
        'dom_settings.h',
        'dom_string_map.cc',
        'dom_string_map.h',
        'dom_token_list.cc',
        'dom_token_list.h',
        'element.cc',
        'element.h',
        'event.cc',
        'event.h',
        'event_listener.h',
        'event_names.cc',
        'event_names.h',
        'event_queue.cc',
        'event_queue.h',
        'event_target.cc',
        'event_target.h',
        'float32_array.h',
        'float64_array.h',
        'focus_event.cc',
        'focus_event.h',
        'history.cc',
        'history.h',
        'html_anchor_element.cc',
        'html_anchor_element.h',
        'html_body_element.cc',
        'html_body_element.h',
        'html_br_element.cc',
        'html_br_element.h',
        'html_collection.cc',
        'html_collection.h',
        'html_div_element.cc',
        'html_div_element.h',
        'html_element.cc',
        'html_element.h',
        'html_element_context.cc',
        'html_element_context.h',
        'html_element_factory.cc',
        'html_element_factory.h',
        'html_head_element.cc',
        'html_head_element.h',
        'html_heading_element.cc',
        'html_heading_element.h',
        'html_html_element.cc',
        'html_html_element.h',
        'html_image_element.cc',
        'html_image_element.h',
        'html_link_element.cc',
        'html_link_element.h',
        'html_media_element.cc',
        'html_media_element.h',
        'html_meta_element.cc',
        'html_meta_element.h',
        'html_paragraph_element.cc',
        'html_paragraph_element.h',
        'html_script_element.cc',
        'html_script_element.h',
        'html_span_element.cc',
        'html_span_element.h',
        'html_style_element.cc',
        'html_style_element.h',
        'html_unknown_element.h',
        'html_video_element.cc',
        'html_video_element.h',
        'keyboard_event.cc',
        'keyboard_event.h',
        'keycode.h',
        'local_storage_database.cc',
        'local_storage_database.h',
        'location.cc',
        'location.h',
        'media_error.h',
        'media_key_complete_event.cc',
        'media_key_complete_event.h',
        'media_key_error.h',
        'media_key_error_event.cc',
        'media_key_error_event.h',
        'media_key_message_event.cc',
        'media_key_message_event.h',
        'media_key_needed_event.cc',
        'media_key_needed_event.h',
        'media_source.cc',
        'media_source.h',
        'mime_type_array.cc',
        'mime_type_array.h',
        'named_node_map.cc',
        'named_node_map.h',
        'navigator.cc',
        'navigator.h',
        'node.cc',
        'node.h',
        'node_children_iterator.h',
        'node_collection.h',
        'node_descendants_iterator.h',
        'node_list.cc',
        'node_list.h',
        'node_list_live.cc',
        'node_list_live.h',
        'performance.cc',
        'performance.h',
        'performance_timing.cc',
        'performance_timing.h',
        'plugin_array.cc',
        'plugin_array.h',
        'progress_event.cc',
        'progress_event.h',
        'pseudo_element.h',
        'rule_matching.cc',
        'rule_matching.h',
        'screen.h',
        'script_event_listener.cc',
        'script_event_listener.h',
        'serializer.cc',
        'serializer.h',
        'source_buffer.cc',
        'source_buffer.h',
        'source_buffer_list.cc',
        'source_buffer_list.h',
        'stats.cc',
        'stats.h',
        'storage.cc',
        'storage.h',
        'storage_area.cc',
        'storage_area.h',
        'storage_event.cc',
        'storage_event.h',
        'test_runner.cc',
        'test_runner.h',
        'text.cc',
        'text.h',
        'time_ranges.cc',
        'time_ranges.h',
        'typed_array.h',
        'ui_event.cc',
        'ui_event.h',
        'ui_event_with_key_state.cc',
        'ui_event_with_key_state.h',
        'uint8_array.h',
        'url.cc',
        'url.h',
        'window.cc',
        'window.h',
        'window_timers.cc',
        'window_timers.h',
        'xml_document.h',
        'xml_serializer.cc',
        'xml_serializer.h',
      ],
      'dependencies': [
        '<(DEPTH)/cobalt/base/base.gyp:base',
        '<(DEPTH)/cobalt/cssom/cssom.gyp:cssom',
        '<(DEPTH)/cobalt/debug/debug.gyp:debug',
        '<(DEPTH)/cobalt/loader/loader.gyp:loader',
        '<(DEPTH)/cobalt/media/media.gyp:media',
        '<(DEPTH)/cobalt/storage/storage.gyp:storage',
        '<(DEPTH)/googleurl/googleurl.gyp:googleurl',
      ],
    },

    {
      'target_name': 'dom_testing',
      'type': 'static_library',
      'sources': [
        'testing/fake_node.h',
        'testing/gtest_workarounds.h',
        'testing/html_collection_testing.h',
        'testing/mock_event_listener.h',
        'testing/stub_css_parser.cc',
        'testing/stub_css_parser.h',
        'testing/stub_script_runner.cc',
        'testing/stub_script_runner.h',
      ],
    },
  ],
}
