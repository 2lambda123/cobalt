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
  'includes': [
    '../bindings/bindings.gypi',
  ],
  'variables': {
    # Base directory into which generated sources and intermediate files should
    # be generated.
    'bindings_output_dir': '<(SHARED_INTERMEDIATE_DIR)/bindings/browser',

    # Bindings for the interfaces in this list will be generated, and there must
    # be an implementation declared in a header that lives in the same
    # directory of each IDL.
    'source_idl_files': [
        '../audio/audio_buffer.idl',
        '../audio/audio_buffer_source_node.idl',
        '../audio/audio_context.idl',
        '../audio/audio_destination_node.idl',
        '../audio/audio_node.idl',

        '../cssom/css.idl',
        '../cssom/css_condition_rule.idl',
        '../cssom/css_grouping_rule.idl',
        '../cssom/css_font_face_rule.idl',
        '../cssom/css_media_rule.idl',
        '../cssom/css_keyframe_rule.idl',
        '../cssom/css_keyframes_rule.idl',
        '../cssom/css_rule.idl',
        '../cssom/css_rule_list.idl',
        '../cssom/css_style_declaration.idl',
        '../cssom/css_style_rule.idl',
        '../cssom/css_style_sheet.idl',
        '../cssom/media_list.idl',
        '../cssom/style_sheet.idl',
        '../cssom/style_sheet_list.idl',

        '../debug/debug_hub.idl',
        '../debug/debugger.idl',
        '../debug/debugger_event_target.idl',
        '../debug/debug_script_runner.idl',

        '../dom/animation_event.idl',
        '../dom/array_buffer.idl',
        '../dom/array_buffer_view.idl',
        '../dom/attr.idl',
        '../dom/audio_track.idl',
        '../dom/audio_track_list.idl',
        '../dom/blob.idl',
        '../dom/camera_3d.idl',
        '../dom/cdata_section.idl',
        '../dom/character_data.idl',
        '../dom/comment.idl',
        '../dom/console.idl',
        '../dom/crypto.idl',
        '../dom/data_view.idl',
        '../dom/device_orientation_event.idl',
        '../dom/document.idl',
        '../dom/document_timeline.idl',
        '../dom/document_type.idl',
        '../dom/dom_exception.idl',
        '../dom/dom_implementation.idl',
        '../dom/dom_parser.idl',
        '../dom/dom_rect.idl',
        '../dom/dom_rect_list.idl',
        '../dom/dom_rect_read_only.idl',
        '../dom/dom_string_map.idl',
        '../dom/dom_token_list.idl',
        '../dom/element.idl',
        '../dom/event.idl',
        '../dom/event_listener.idl',
        '../dom/event_target.idl',
        '../dom/float32_array.idl',
        '../dom/float64_array.idl',
        '../dom/focus_event.idl',
        '../dom/history.idl',
        '../dom/html_anchor_element.idl',
        '../dom/html_body_element.idl',
        '../dom/html_br_element.idl',
        '../dom/html_collection.idl',
        '../dom/html_div_element.idl',
        '../dom/html_element.idl',
        '../dom/html_head_element.idl',
        '../dom/html_heading_element.idl',
        '../dom/html_html_element.idl',
        '../dom/html_image_element.idl',
        '../dom/html_link_element.idl',
        '../dom/html_media_element.idl',
        '../dom/html_meta_element.idl',
        '../dom/html_paragraph_element.idl',
        '../dom/html_script_element.idl',
        '../dom/html_span_element.idl',
        '../dom/html_style_element.idl',
        '../dom/html_title_element.idl',
        '../dom/html_unknown_element.idl',
        '../dom/html_video_element.idl',
        '../dom/keyboard_event.idl',
        '../dom/location.idl',
        '../dom/media_error.idl',
        '../dom/media_query_list.idl',
        '../dom/media_source.idl',
        '../dom/memory_info.idl',
        '../dom/message_event.idl',
        '../dom/mime_type_array.idl',
        '../dom/mouse_event.idl',
        '../dom/mutation_observer.idl',
        '../dom/mutation_record.idl',
        '../dom/named_node_map.idl',
        '../dom/navigator.idl',
        '../dom/node.idl',
        '../dom/node_list.idl',
        '../dom/performance.idl',
        '../dom/performance_timing.idl',
        '../dom/plugin_array.idl',
        '../dom/pointer_event.idl',
        '../dom/progress_event.idl',
        '../dom/screen.idl',
        '../dom/security_policy_violation_event.idl',
        '../dom/source_buffer.idl',
        '../dom/source_buffer_list.idl',
        '../dom/storage.idl',
        '../dom/storage_event.idl',
        '../dom/test_runner.idl',
        '../dom/text.idl',
        '../dom/time_ranges.idl',
        '../dom/track_default.idl',
        '../dom/track_default_list.idl',
        '../dom/track_event.idl',
        '../dom/transition_event.idl',
        '../dom/ui_event.idl',
        '../dom/uint16_array.idl',
        '../dom/uint32_array.idl',
        '../dom/uint8_array.idl',
        '../dom/url.idl',
        '../dom/video_playback_quality.idl',
        '../dom/video_track.idl',
        '../dom/video_track_list.idl',
        '../dom/wheel_event.idl',
        '../dom/window.idl',
        '../dom/xml_document.idl',
        '../dom/xml_serializer.idl',

        '../fetch/fetch_internal.idl',

        '../h5vcc/dial/dial_http_request.idl',
        '../h5vcc/dial/dial_http_response.idl',
        '../h5vcc/dial/dial_server.idl',
        '../h5vcc/h5vcc.idl',
        '../h5vcc/h5vcc_accessibility.idl',
        '../h5vcc/h5vcc_account_info.idl',
        '../h5vcc/h5vcc_account_manager.idl',
        '../h5vcc/h5vcc_audio_config.idl',
        '../h5vcc/h5vcc_audio_config_array.idl',
        '../h5vcc/h5vcc_c_val.idl',
        '../h5vcc/h5vcc_c_val_key_list.idl',
        '../h5vcc/h5vcc_crash_log.idl',
        '../h5vcc/h5vcc_deep_link_event_target.idl',
        '../h5vcc/h5vcc_runtime.idl',
        '../h5vcc/h5vcc_runtime_event_target.idl',
        '../h5vcc/h5vcc_settings.idl',
        '../h5vcc/h5vcc_sso.idl',
        '../h5vcc/h5vcc_storage.idl',
        '../h5vcc/h5vcc_system.idl',
        '../h5vcc/h5vcc_trace_event.idl',

        '../media_session/media_metadata.idl',
        '../media_session/media_session.idl',

        '../speech/speech_recognition.idl',
        '../speech/speech_recognition_alternative.idl',
        '../speech/speech_recognition_error.idl',
        '../speech/speech_recognition_event.idl',
        '../speech/speech_recognition_result.idl',
        '../speech/speech_recognition_result_list.idl',

        '../speech/speech_synthesis.idl',
        '../speech/speech_synthesis_error_event.idl',
        '../speech/speech_synthesis_event.idl',
        '../speech/speech_synthesis_utterance.idl',
        '../speech/speech_synthesis_voice.idl',

        '../web_animations/animatable.idl',
        '../web_animations/animation.idl',
        '../web_animations/animation_effect_read_only.idl',
        '../web_animations/animation_effect_timing_read_only.idl',
        '../web_animations/animation_timeline.idl',
        '../web_animations/keyframe.idl',
        '../web_animations/keyframe_effect_read_only.idl',

        '../webdriver/script_executor.idl',
        '../webdriver/script_executor_params.idl',
        '../webdriver/script_executor_result.idl',

        '../websocket/close_event.idl',
        '../websocket/web_socket.idl',

        '../xhr/xml_http_request.idl',
        '../xhr/xml_http_request_event_target.idl',
        '../xhr/xml_http_request_upload.idl',
    ],

    # IDL files that will end up generating a .h that will be #included in
    # Cobalt directly. IDL files for dictionaries and enums.
    'generated_header_idl_files': [
        '../audio/audio_node_channel_count_mode.idl',
        '../audio/audio_node_channel_interpretation.idl',
        '../dom/blob_property_bag.idl',
        '../dom/device_orientation_event_init.idl',
        '../dom/document_ready_state.idl',
        '../dom/dom_parser_supported_type.idl',
        '../dom/event_init.idl',
        '../dom/event_modifier_init.idl',
        '../dom/focus_event_init.idl',
        '../dom/keyboard_event_init.idl',
        '../dom/ui_event_init.idl',
        '../dom/media_source_end_of_stream_error.idl',
        '../dom/media_source_ready_state.idl',
        '../dom/mouse_event_init.idl',
        '../dom/mutation_observer_init.idl',
        '../dom/pointer_event_init.idl',
        '../dom/source_buffer_append_mode.idl',
        '../dom/track_default_type.idl',
        '../dom/wheel_event_init.idl',
        '../media_session/media_image.idl',
        '../media_session/media_metadata_init.idl',
        '../media_session/media_session_action.idl',
        '../media_session/media_session_playback_state.idl',
        '../page_visibility/visibility_state.idl',
        '../speech/speech_recognition_error_code.idl',
        '../speech/speech_synthesis_error_code.idl',
        '../web_animations/animation_fill_mode.idl',
        '../web_animations/animation_playback_direction.idl',
        '../websocket/close_event_init.idl',
    ],

    # Partial interfaces and the right-side of "implements". Also includes
    # callback interfaces.
    # Code will not get generated for these interfaces; they are used to add
    # functionality to other interfaces.
    'dependency_idl_files': [
        '../cssom/link_style.idl',

        '../dom/buffer_source.idl',
        '../dom/document__web_animations_api.idl',
        '../dom/document_cssom.idl',
        '../dom/document_html5.idl',
        '../dom/element_css_inline_style.idl',
        '../dom/element_cssom_view.idl',
        '../dom/element_dom_parsing_and_serialization.idl',
        '../dom/global_crypto.idl',
        '../dom/global_event_handlers.idl',
        '../dom/html_element_cssom_view.idl',
        '../dom/mouse_event_cssom_view.idl',
        '../dom/navigator_id.idl',
        '../dom/navigator_language.idl',
        '../dom/navigator_plugins.idl',
        '../dom/navigator_storage_utils.idl',
        '../dom/non_document_type_child_node.idl',
        '../dom/non_element_parent_node.idl',
        '../dom/parent_node.idl',
        '../dom/performance__high_resolution_time.idl',
        '../dom/speech_synthesis_getter.idl',
        '../dom/url_utils.idl',
        '../dom/window__animation_timing.idl',
        '../dom/window__performance.idl',
        '../dom/window_cssom.idl',
        '../dom/window_cssom_view.idl',
        '../dom/window_event_handlers.idl',
        '../dom/window_local_storage.idl',
        '../dom/window_session_storage.idl',
        '../dom/window_timers.idl',
        '../media_session/navigator_media_session.idl',
        '../page_visibility/document.idl',
    ],

    'conditions': [
      ['cobalt_media_source_2016==1', {
        'source_idl_files': [
            '../dom/eme/media_encrypted_event.idl',
            '../dom/eme/media_key_message_event.idl',
            '../dom/eme/media_key_session.idl',
            '../dom/eme/media_key_system_access.idl',
            '../dom/eme/media_keys.idl',
        ],
        'generated_header_idl_files': [
            '../dom/eme/media_encrypted_event_init.idl',
            '../dom/eme/media_key_message_event_init.idl',
            '../dom/eme/media_key_message_type.idl',
            '../dom/eme/media_key_session_type.idl',
            '../dom/eme/media_key_system_configuration.idl',
            '../dom/eme/media_key_system_media_capability.idl',
            '../dom/eme/media_keys_requirement.idl',
        ],
        'dependency_idl_files': [
            '../dom/eme/html_media_element.idl',
            '../dom/eme/navigator.idl',
        ],
      }, {
        'source_idl_files': [
            '../dom/media_key_complete_event.idl',
            '../dom/media_key_error.idl',
            '../dom/media_key_error_event.idl',
            '../dom/media_key_message_event.idl',
            '../dom/media_key_needed_event.idl',
        ],
        'dependency_idl_files': [
            '../dom/html_media_element_eme_01b.idl',
        ],
      }],
    ],
  },
}
