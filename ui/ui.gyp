# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'chromium_code': 1,
  },
  'includes': [
    'ui_resources.gypi',
  ],
  'targets': [
    {
      'target_name': 'ui',
      'type': '<(component)',
      'variables': { 'enable_wexit_time_destructors': 1, },
      'dependencies': [
        '../base/base.gyp:base',
        '../base/base.gyp:base_i18n',
        '../base/base.gyp:base_static',
        '../base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '../build/temp_gyp/googleurl.gyp:googleurl',
        '../net/net.gyp:net',
        '../skia/skia.gyp:skia',
        '../third_party/icu/icu.gyp:icui18n',
        '../third_party/icu/icu.gyp:icuuc',
        '../third_party/libpng/libpng.gyp:libpng',
        '../third_party/zlib/zlib.gyp:zlib',
        'base/strings/ui_strings.gyp:ui_strings',
        'ui_resources',
      ],
      'defines': [
        'UI_IMPLEMENTATION',
      ],
      # Export these dependencies since text_elider.h includes ICU headers.
      'export_dependent_settings': [
        '../net/net.gyp:net',
        '../third_party/icu/icu.gyp:icui18n',
        '../third_party/icu/icu.gyp:icuuc',
      ],
      'sources' : [
        'android/ui_jni_registrar.cc',
        'android/ui_jni_registrar.h',
        'base/accelerators/accelerator.cc',
        'base/accelerators/accelerator.h',
        'base/accelerators/platform_accelerator.h',
        'base/accelerators/platform_accelerator_cocoa.h',
        'base/accelerators/platform_accelerator_cocoa.mm',
        'base/accelerators/platform_accelerator_gtk.cc',
        'base/accelerators/platform_accelerator_gtk.h',
        'base/accelerators/accelerator_manager.cc',
        'base/accelerators/accelerator_manager.h',
        'base/accessibility/accessibility_types.h',
        'base/accessibility/accessible_text_utils.cc',
        'base/accessibility/accessible_text_utils.h',
        'base/accessibility/accessible_view_state.cc',
        'base/accessibility/accessible_view_state.h',
        'base/animation/animation.cc',
        'base/animation/animation.h',
        'base/animation/animation_container.cc',
        'base/animation/animation_container.h',
        'base/animation/animation_container_element.h',
        'base/animation/animation_container_observer.h',
        'base/animation/animation_delegate.h',
        'base/animation/linear_animation.cc',
        'base/animation/linear_animation.h',
        'base/animation/multi_animation.cc',
        'base/animation/multi_animation.h',
        'base/animation/slide_animation.cc',
        'base/animation/slide_animation.h',
        'base/animation/throb_animation.cc',
        'base/animation/throb_animation.h',
        'base/animation/tween.cc',
        'base/animation/tween.h',
        'base/clipboard/clipboard.cc',
        'base/clipboard/clipboard.h',
        'base/clipboard/clipboard_android.cc',
        'base/clipboard/clipboard_aurax11.cc',
        'base/clipboard/clipboard_chromeos.cc',
        'base/clipboard/clipboard_gtk.cc',
        'base/clipboard/clipboard_mac.mm',
        'base/clipboard/clipboard_util_win.cc',
        'base/clipboard/clipboard_util_win.h',
        'base/clipboard/clipboard_win.cc',
        'base/clipboard/custom_data_helper.cc',
        'base/clipboard/custom_data_helper.h',
        'base/clipboard/custom_data_helper_mac.mm',
        'base/clipboard/custom_data_helper_x.cc',
        'base/clipboard/scoped_clipboard_writer.cc',
        'base/clipboard/scoped_clipboard_writer.h',
        'base/cocoa/base_view.h',
        'base/cocoa/base_view.mm',
        'base/cocoa/events_mac.mm',
        'base/cocoa/find_pasteboard.h',
        'base/cocoa/find_pasteboard.mm',
        'base/cocoa/focus_tracker.h',
        'base/cocoa/focus_tracker.mm',
        'base/cocoa/fullscreen_window_manager.h',
        'base/cocoa/fullscreen_window_manager.mm',
        'base/cocoa/nib_loading.h',
        'base/cocoa/nib_loading.mm',
        'base/cocoa/underlay_opengl_hosting_window.h',
        'base/cocoa/underlay_opengl_hosting_window.mm',
        'base/cocoa/window_size_constants.h',
        'base/cocoa/window_size_constants.mm',
        'base/cursor/cursor.cc',
        'base/cursor/cursor.h',
        'base/cursor/cursor_loader.h',
        'base/cursor/cursor_loader_win.cc',
        'base/cursor/cursor_loader_win.h',
        'base/cursor/cursor_loader_x11.cc',
        'base/cursor/cursor_loader_x11.h',
        'base/cursor/cursor_win.cc',
        'base/cursor/cursor_x11.cc',
        'base/dialogs/base_shell_dialog.cc',
        'base/dialogs/base_shell_dialog.h',
        'base/dialogs/base_shell_dialog_win.cc',
        'base/dialogs/base_shell_dialog_win.h',
        'base/dialogs/gtk/select_file_dialog_impl.cc',
        'base/dialogs/gtk/select_file_dialog_impl.h',
        'base/dialogs/gtk/select_file_dialog_impl_gtk.cc',
        'base/dialogs/gtk/select_file_dialog_impl_kde.cc',
        'base/dialogs/select_file_dialog.cc',
        'base/dialogs/select_file_dialog.h',
        'base/dialogs/select_file_dialog_android.cc',
        'base/dialogs/select_file_dialog_android.h',
        'base/dialogs/select_file_dialog_factory.cc',
        'base/dialogs/select_file_dialog_factory.h',
        'base/dialogs/select_file_dialog_mac.h',
        'base/dialogs/select_file_dialog_mac.mm',
        'base/dialogs/select_file_dialog_win.cc',
        'base/dialogs/select_file_dialog_win.h',
        'base/dialogs/select_file_policy.cc',
        'base/dialogs/select_file_policy.h',
        'base/dialogs/selected_file_info.cc',
        'base/dialogs/selected_file_info.h',
        'base/dragdrop/cocoa_dnd_util.h',
        'base/dragdrop/cocoa_dnd_util.mm',
        'base/dragdrop/drag_drop_types.h',
        'base/dragdrop/drag_drop_types_gtk.cc',
        'base/dragdrop/drag_drop_types_win.cc',
        'base/dragdrop/drag_source.cc',
        'base/dragdrop/drag_source.h',
        'base/dragdrop/drag_utils.cc',
        'base/dragdrop/drag_utils.h',
        'base/dragdrop/drag_utils_aura.cc',
        'base/dragdrop/drag_utils_gtk.cc',
        'base/dragdrop/drag_utils_win.cc',
        'base/dragdrop/drop_target.cc',
        'base/dragdrop/drop_target.h',
        'base/dragdrop/gtk_dnd_util.cc',
        'base/dragdrop/gtk_dnd_util.h',
        'base/dragdrop/os_exchange_data.cc',
        'base/dragdrop/os_exchange_data.h',
        'base/dragdrop/os_exchange_data_provider_aura.cc',
        'base/dragdrop/os_exchange_data_provider_aura.h',
        'base/dragdrop/os_exchange_data_provider_gtk.cc',
        'base/dragdrop/os_exchange_data_provider_gtk.h',
        'base/dragdrop/os_exchange_data_provider_win.cc',
        'base/dragdrop/os_exchange_data_provider_win.h',
        'base/events/event.cc',
        'base/events/event.h',
        'base/events/event_constants.h',
        'base/events/event_conversion_gtk.cc',
        'base/events/event_conversion_gtk.h',
        'base/events/event_dispatcher.cc',
        'base/events/event_dispatcher.h',
        'base/events/event_handler.cc',
        'base/events/event_handler.h',
        'base/events/event_target.cc',
        'base/events/event_target.h',
        'base/events/event_utils.cc',
        'base/events/event_utils.h',
        'base/gestures/gesture_configuration.cc',
        'base/gestures/gesture_configuration.h',
        'base/gestures/gesture_point.cc',
        'base/gestures/gesture_point.h',
        'base/gestures/gesture_recognizer.h',
        'base/gestures/gesture_recognizer_impl.cc',
        'base/gestures/gesture_recognizer_impl.h',
        'base/gestures/gesture_sequence.cc',
        'base/gestures/gesture_sequence.h',
        'base/gestures/gesture_types.cc',
        'base/gestures/gesture_types.h',
        'base/gestures/gesture_util.cc',
        'base/gestures/gesture_util.h',
        'base/gestures/velocity_calculator.cc',
        'base/gestures/velocity_calculator.h',
        'base/gtk/event_synthesis_gtk.cc',
        'base/gtk/event_synthesis_gtk.h',
        'base/gtk/focus_store_gtk.cc',
        'base/gtk/focus_store_gtk.h',
        'base/gtk/g_object_destructor_filo.cc',
        'base/gtk/g_object_destructor_filo.h',
        'base/gtk/gtk_expanded_container.cc',
        'base/gtk/gtk_expanded_container.h',
        'base/gtk/gtk_floating_container.cc',
        'base/gtk/gtk_floating_container.h',
        'base/gtk/gtk_hig_constants.h',
        'base/gtk/gtk_im_context_util.cc',
        'base/gtk/gtk_im_context_util.h',
        'base/gtk/gtk_screen_util.cc',
        'base/gtk/gtk_screen_util.h',
        'base/gtk/gtk_signal.h',
        'base/gtk/gtk_signal_registrar.cc',
        'base/gtk/gtk_signal_registrar.h',
        'base/gtk/gtk_windowing.cc',
        'base/gtk/gtk_windowing.h',
        'base/gtk/menu_label_accelerator_util.cc',
        'base/gtk/menu_label_accelerator_util.h',
        'base/gtk/owned_widget_gtk.cc',
        'base/gtk/owned_widget_gtk.h',
        'base/gtk/scoped_gobject.h',
        'base/gtk/scoped_region.cc',
        'base/gtk/scoped_region.h',
        'base/hit_test.h',
        'base/keycodes/keyboard_code_conversion.cc',
        'base/keycodes/keyboard_code_conversion.h',
        'base/keycodes/keyboard_code_conversion_gtk.cc',
        'base/keycodes/keyboard_code_conversion_gtk.h',
        'base/keycodes/keyboard_code_conversion_mac.h',
        'base/keycodes/keyboard_code_conversion_mac.mm',
        'base/keycodes/keyboard_code_conversion_win.cc',
        'base/keycodes/keyboard_code_conversion_win.h',
        'base/keycodes/keyboard_code_conversion_x.cc',
        'base/keycodes/keyboard_code_conversion_x.h',
        'base/keycodes/keyboard_codes.h',
        'base/keycodes/usb_keycode_map.h',
        'base/l10n/l10n_font_util.cc',
        'base/l10n/l10n_font_util.h',
        'base/l10n/l10n_util.cc',
        'base/l10n/l10n_util.h',
        'base/l10n/l10n_util_collator.h',
        'base/l10n/l10n_util_mac.h',
        'base/l10n/l10n_util_mac.mm',
        'base/l10n/l10n_util_posix.cc',
        'base/l10n/l10n_util_win.cc',
        'base/l10n/l10n_util_win.h',
        'base/layout.cc',
        'base/layout.h',
        'base/layout_mac.mm',
        'base/models/button_menu_item_model.cc',
        'base/models/button_menu_item_model.h',
        'base/models/combobox_model.h',
        'base/models/list_model.h',
        'base/models/list_model_observer.h',
        'base/models/menu_model.cc',
        'base/models/menu_model.h',
        'base/models/menu_model_delegate.h',
        'base/models/menu_separator_types.h',
        'base/models/simple_menu_model.cc',
        'base/models/simple_menu_model.h',
        'base/models/table_model.cc',
        'base/models/table_model.h',
        'base/models/table_model_observer.h',
        'base/models/tree_model.cc',
        'base/models/tree_model.h',
        'base/models/tree_node_iterator.h',
        'base/models/tree_node_model.h',
        'base/range/range.cc',
        'base/range/range.h',
        'base/range/range_mac.mm',
        'base/range/range_win.cc',
        'base/resource/data_pack.cc',
        'base/resource/data_pack.h',
        'base/resource/resource_bundle.cc',
        'base/resource/resource_bundle.h',
        'base/resource/resource_bundle_android.cc',
        'base/resource/resource_bundle_aurax11.cc',
        'base/resource/resource_bundle_gtk.cc',
        'base/resource/resource_bundle_ios.mm',
        'base/resource/resource_bundle_mac.mm',
        'base/resource/resource_bundle_win.cc',
        'base/resource/resource_bundle_win.h',
        'base/resource/resource_data_dll_win.cc',
        'base/resource/resource_data_dll_win.h',
        'base/resource/resource_handle.h',
        'base/text/bytes_formatting.cc',
        'base/text/bytes_formatting.h',
        'base/text/text_elider.cc',
        'base/text/text_elider.h',
        'base/text/utf16_indexing.cc',
        'base/text/utf16_indexing.h',
        'base/theme_provider.cc',
        'base/theme_provider.h',
        'base/touch/touch_factory.cc',
        'base/touch/touch_factory.h',
        'base/ui_base_exports.cc',
        'base/ui_base_paths.cc',
        'base/ui_base_paths.h',
        'base/ui_base_switches.cc',
        'base/ui_base_switches.h',
        'base/ui_base_types.h',
        'base/ui_export.h',
        'base/view_prop.cc',
        'base/view_prop.h',
        'base/win/accessibility_misc_utils.cc',
        'base/win/accessibility_misc_utils.h',
        'base/win/atl_module.h',
        'base/win/dpi.cc',
        'base/win/dpi.h',
        'base/win/events_win.cc',
        'base/win/extra_sdk_defines.h',
        'base/win/foreground_helper.cc',
        'base/win/foreground_helper.h',
        'base/win/hidden_window.cc',
        'base/win/hidden_window.h',
        'base/win/hwnd_subclass.cc',
        'base/win/hwnd_subclass.h',
        'base/win/hwnd_util.cc',
        'base/win/hwnd_util.h',
        'base/win/ime_input.cc',
        'base/win/ime_input.h',
        'base/win/message_box_win.cc',
        'base/win/message_box_win.h',
        'base/win/mouse_wheel_util.cc',
        'base/win/mouse_wheel_util.h',
        'base/win/scoped_ole_initializer.cc',
        'base/win/scoped_ole_initializer.h',
        'base/win/scoped_set_map_mode.h',
        'base/win/shell.cc',
        'base/win/shell.h',
        'base/win/singleton_hwnd.cc',
        'base/win/singleton_hwnd.h',
        'base/win/window_impl.cc',
        'base/win/window_impl.h',
        'base/work_area_watcher_observer.h',
        'base/x/active_window_watcher_x.cc',
        'base/x/active_window_watcher_x.h',
        'base/x/active_window_watcher_x_observer.h',
        'base/x/events_x.cc',
        'base/x/root_window_property_watcher_x.cc',
        'base/x/root_window_property_watcher_x.h',
        'base/x/valuators.cc',
        'base/x/valuators.h',
        'base/x/work_area_watcher_x.cc',
        'base/x/work_area_watcher_x.h',
        'base/x/x11_atom_cache.cc',
        'base/x/x11_atom_cache.h',
        'base/x/x11_util.cc',
        'base/x/x11_util.h',
        'base/x/x11_util_internal.h',
        'gfx/android/gfx_jni_registrar.cc',
        'gfx/android/gfx_jni_registrar.h',
        'gfx/android/java_bitmap.cc',
        'gfx/android/java_bitmap.h',
        'gfx/android/window_android.cc',
        'gfx/android/window_android.h',
        'gfx/blit.cc',
        'gfx/blit.h',
        'gfx/canvas.cc',
        'gfx/canvas.h',
        'gfx/canvas_android.cc',
        'gfx/canvas_mac.mm',
        'gfx/canvas_paint.h',
        'gfx/canvas_paint_gtk.h',
        'gfx/canvas_paint_mac.h',
        'gfx/canvas_paint_win.h',
        'gfx/canvas_paint_gtk.cc',
        'gfx/canvas_paint_mac.mm',
        'gfx/canvas_paint_win.cc',
        'gfx/canvas_skia.cc',
        'gfx/canvas_skia_paint.h',
        'gfx/codec/jpeg_codec.cc',
        'gfx/codec/jpeg_codec.h',
        'gfx/codec/png_codec.cc',
        'gfx/codec/png_codec.h',
        'gfx/color_analysis.cc',
        'gfx/color_analysis.h',
        'gfx/color_profile.cc',
        'gfx/color_profile.h',
        'gfx/color_profile_mac.cc',
        'gfx/color_profile_win.cc',
        'gfx/color_utils.cc',
        'gfx/color_utils.h',
        'gfx/display.cc',
        'gfx/display.h',
        'gfx/display_observer.cc',
        'gfx/display_observer.h',
        'gfx/favicon_size.cc',
        'gfx/favicon_size.h',
        'gfx/font.cc',
        'gfx/font.h',
        'gfx/font_fallback_win.cc',
        'gfx/font_fallback_win.h',
        'gfx/font_list.cc',
        'gfx/font_list.h',
        'gfx/font_render_params_android.cc',
        'gfx/font_render_params_linux.cc',
        'gfx/font_render_params_linux.h',
        'gfx/font_smoothing_win.cc',
        'gfx/font_smoothing_win.h',
        'gfx/gfx_paths.cc',
        'gfx/gfx_paths.h',
        'gfx/image/canvas_image_source.cc',
        'gfx/image/canvas_image_source.h',
        'gfx/image/image.cc',
        'gfx/image/image.h',
        'gfx/image/image_ios.mm',
        'gfx/image/image_mac.mm',
        'gfx/image/image_skia.cc',
        'gfx/image/image_skia.h',
        'gfx/image/image_skia_operations.cc',
        'gfx/image/image_skia_operations.h',
        'gfx/image/image_skia_rep.cc',
        'gfx/image/image_skia_rep.h',
        'gfx/image/image_skia_source.h',
        'gfx/image/image_skia_util_ios.h',
        'gfx/image/image_skia_util_ios.mm',
        'gfx/image/image_skia_util_mac.h',
        'gfx/image/image_skia_util_mac.mm',
        'gfx/image/image_util.cc',
        'gfx/image/image_util.h',
        'gfx/image/image_util_ios.mm',
        'gfx/insets.cc',
        'gfx/insets.h',
        'gfx/insets_base.h',
        'gfx/insets_f.cc',
        'gfx/insets_f.h',
        'gfx/interpolated_transform.cc',
        'gfx/interpolated_transform.h',
        'gfx/mac/nsimage_cache.h',
        'gfx/mac/nsimage_cache.mm',
        'gfx/mac/scoped_ns_disable_screen_updates.h',
        'gfx/native_widget_types.h',
        'gfx/pango_util.cc',
        'gfx/pango_util.h',
        'gfx/path.cc',
        'gfx/path.h',
        'gfx/path_aura.cc',
        'gfx/path_gtk.cc',
        'gfx/path_win.cc',
        'gfx/path_win.h',
        'gfx/platform_font.h',
        'gfx/platform_font_android.cc',
        'gfx/platform_font_ios.h',
        'gfx/platform_font_ios.mm',
        'gfx/platform_font_mac.h',
        'gfx/platform_font_mac.mm',
        'gfx/platform_font_pango.cc',
        'gfx/platform_font_pango.h',
        'gfx/platform_font_win.cc',
        'gfx/platform_font_win.h',
        'gfx/point.cc',
        'gfx/point.h',
        'gfx/point3_f.cc',
        'gfx/point3_f.h',
        'gfx/point_base.h',
        'gfx/point_conversions.cc',
        'gfx/point_conversions.h',
        'gfx/point_f.cc',
        'gfx/point_f.h',
        'gfx/quad_f.cc',
        'gfx/quad_f.h',
        'gfx/rect.cc',
        'gfx/rect.h',
        'gfx/rect_base.h',
        'gfx/rect_base_impl.h',
        'gfx/rect_conversions.cc',
        'gfx/rect_conversions.h',
        'gfx/rect_f.cc',
        'gfx/rect_f.h',
        'gfx/render_text.cc',
        'gfx/render_text.h',
        'gfx/render_text_linux.cc',
        'gfx/render_text_linux.h',
        'gfx/render_text_mac.cc',
        'gfx/render_text_mac.h',
        'gfx/render_text_win.cc',
        'gfx/render_text_win.h',
        'gfx/safe_integer_conversions.cc',
        'gfx/safe_integer_conversions.h',
        'gfx/scoped_cg_context_save_gstate_mac.h',
        'gfx/scoped_ns_graphics_context_save_gstate_mac.h',
        'gfx/scoped_ns_graphics_context_save_gstate_mac.mm',
        'gfx/scoped_ui_graphics_push_context_ios.h',
        'gfx/scoped_ui_graphics_push_context_ios.mm',
        'gfx/screen.cc',
        'gfx/screen.h',
        'gfx/screen_android.cc',
        'gfx/screen_aura.cc',
        'gfx/screen_gtk.cc',
        'gfx/screen_ios.mm',
        'gfx/screen_mac.mm',
        'gfx/screen_win.cc',
        'gfx/screen_win.h',
        'gfx/scrollbar_size.cc',
        'gfx/scrollbar_size.h',
        'gfx/selection_model.cc',
        'gfx/selection_model.h',
        'gfx/shadow_value.cc',
        'gfx/shadow_value.h',
        'gfx/size.cc',
        'gfx/size.h',
        'gfx/size_base.h',
        'gfx/size_conversions.cc',
        'gfx/size_conversions.h',
        'gfx/size_f.cc',
        'gfx/size_f.h',
        'gfx/skbitmap_operations.cc',
        'gfx/skbitmap_operations.h',
        'gfx/skia_util.cc',
        'gfx/skia_util.h',
        'gfx/skia_utils_gtk.cc',
        'gfx/skia_utils_gtk.h',
        'gfx/switches.cc',
        'gfx/switches.h',
        'gfx/sys_color_change_listener.cc',
        'gfx/sys_color_change_listener.h',
        'gfx/text_constants.h',
        'gfx/text_utils.cc',
        'gfx/text_utils.h',
        'gfx/transform.cc',
        'gfx/transform.h',
        'gfx/transform_util.cc',
        'gfx/transform_util.h',
        'gfx/vector2d.cc',
        'gfx/vector2d.h',
        'gfx/vector2d_conversions.cc',
        'gfx/vector2d_conversions.h',
        'gfx/vector2d_f.cc',
        'gfx/vector2d_f.h',
        'gfx/vector3d_f.cc',
        'gfx/vector3d_f.h',
        'gfx/video_decode_acceleration_support_mac.h',
        'gfx/video_decode_acceleration_support_mac.mm',
        'notifications/notification_types.h',
        'notifications/notification_types.cc',
        'ui_controls/ui_controls.h',
        'ui_controls/ui_controls_aura.cc',
        'ui_controls/ui_controls_gtk.cc',
        'ui_controls/ui_controls_internal_win.cc',
        'ui_controls/ui_controls_internal_win.h',
        'ui_controls/ui_controls_mac.mm',
        'ui_controls/ui_controls_win.cc',
      ],
      'target_conditions': [
        ['OS == "ios"', {
          'sources/': [
            ['include', '^base/l10n/l10n_util_mac\\.mm$'],
          ],
        }],
      ],
      'conditions': [
        ['OS!="ios"', {
          'includes': [
            'base/ime/ime.gypi',
          ],
          'dependencies': [
            '<(libjpeg_gyp_path):libjpeg',
          ],
        }, {  # OS=="ios"
          # iOS only uses a subset of UI.
          'sources/': [
            ['exclude', '\\.(cc|mm)$'],
            ['include', '_ios\\.(cc|mm)$'],
            ['include', '^gfx/'],
            ['exclude', '^gfx/codec/jpeg_codec\\.cc$'],
            ['exclude', '^gfx/pango_util\\.'],
            ['exclude', '^gfx/platform_font_pango\\.'],
            ['include', '^base/animation/'],
            ['include', '^base/l10n/'],
            ['include', '^base/layout'],
            ['include', '^base/range/'],
            ['include', '^base/resource/'],
            ['include', '^base/text/text_elider\\.'],
            ['include', '^base/ui_base_'],
          ],
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/CoreGraphics.framework',
            ],
          },
        }],
        # TODO(asvitkine): Switch all platforms to use canvas_skia.cc.
        #                  http://crbug.com/105550
        ['use_canvas_skia==1', {
          'sources!': [
            'gfx/canvas_android.cc',
            'gfx/canvas_mac.mm',
          ],
        }, {  # use_canvas_skia!=1
          'sources!': [
            'gfx/canvas_skia.cc',
          ],
        }],
        ['use_aura==1', {
          'sources/': [
            ['exclude', 'gfx/gtk_'],
            ['exclude', 'gfx/gtk_util.cc'],
            ['exclude', 'gfx/gtk_util.h'],
            ['exclude', 'gfx/screen_gtk.cc'],
            ['exclude', 'base/dialogs/select_file_dialog_mac.mm'],
            ['exclude', 'base/dialogs/select_file_dialog_win.cc'],
            ['exclude', 'base/dragdrop/drag_utils_win.cc'],
            ['exclude', 'base/work_area_watcher_observer.h'],
            ['exclude', 'base/x/active_window_watcher_x.cc'],
            ['exclude', 'base/x/active_window_watcher_x.h'],
            ['exclude', 'base/x/active_window_watcher_x_observer.h'],
            ['exclude', 'base/x/root_window_property_watcher_x.cc'],
            ['exclude', 'base/x/root_window_property_watcher_x.h'],
            ['exclude', 'base/x/work_area_watcher_x.cc'],
            ['exclude', 'base/x/work_area_watcher_x.h'],
            ['exclude', 'ui_controls_win.cc'],
           ],
        }, {  # use_aura!=1
          'sources!': [
            'base/cursor/cursor.cc',
            'base/cursor/cursor.h',
            'base/cursor/cursor_loader_x11.cc',
            'base/cursor/cursor_loader_x11.h',
            'base/cursor/cursor_win.cc',
            'base/cursor/cursor_x11.cc',
          ]
        }],
        ['use_aura==1 and OS=="win"', {
          'sources/': [
            ['exclude', 'base/dragdrop/os_exchange_data_provider_win.cc'],
            ['exclude', 'base/dragdrop/os_exchange_data_provider_win.h'],
          ],
        }],
        ['use_aura==0 and toolkit_views==0', {
          'sources/': [
            ['exclude', '^base/gestures/*'],
          ]
        }],
        ['use_aura==1 and use_ash==0 and OS=="linux"', {
          'sources': [
            'base/linux_ui.cc',
            'base/linux_ui.h',
          ]
        }],
        ['use_ibus==1', {
          'dependencies': [
            '../build/linux/system.gyp:ibus',
          ],
        }],
        ['use_glib == 1', {
          'dependencies': [
            # font_gtk.cc uses fontconfig.
            '../build/linux/system.gyp:fontconfig',
            '../build/linux/system.gyp:glib',
            '../build/linux/system.gyp:pangocairo',
            '../build/linux/system.gyp:x11',
            '../build/linux/system.gyp:xext',
            '../build/linux/system.gyp:xfixes',
          ],
          'link_settings': {
            'libraries': [
              '-lXcursor',  # For XCursor* function calls in x11_util.cc.
              '-lXrender',  # For XRender* function calls in x11_util.cc.
              '-lXrandr',   # For XRR* function calls in x11_util.cc.
            ],
          },
          'conditions': [
            ['toolkit_views==0', {
              # Note: because of gyp predence rules this has to be defined as
              # 'sources/' rather than 'sources!'.
              'sources/': [
                ['exclude', '^base/dragdrop/drag_drop_types_gtk.cc'],
                ['exclude', '^base/dragdrop/drag_utils_gtk.cc'],
                ['exclude', '^base/dragdrop/drag_utils.cc'],
                ['exclude', '^base/dragdrop/drag_utils.h'],
                ['exclude', '^base/dragdrop/os_exchange_data.cc'],
                ['exclude', '^base/dragdrop/os_exchange_data.h'],
                ['exclude', '^base/dragdrop/os_exchange_data_provider_gtk.cc'],
                ['exclude', '^base/dragdrop/os_exchange_data_provider_gtk.h'],
              ],
            }, {
              # Note: because of gyp predence rules this has to be defined as
              # 'sources/' rather than 'sources!'.
              'sources/': [
                ['include', '^base/dragdrop/os_exchange_data.cc'],
              ],
            }],
          ],
        }],
        ['toolkit_uses_gtk == 1', {
          'dependencies': [
            '../build/linux/system.gyp:gtk',
          ],
          'sources': [
            'gfx/gtk_native_view_id_manager.cc',
            'gfx/gtk_native_view_id_manager.h',
            'gfx/gtk_preserve_window.cc',
            'gfx/gtk_preserve_window.h',
            'gfx/gtk_util.cc',
            'gfx/gtk_util.h',
            'gfx/image/cairo_cached_surface.cc',
            'gfx/image/cairo_cached_surface.h',
          ],
        }],
        ['chromeos==1', {
          'sources!': [
            'base/clipboard/clipboard_aurax11.cc',
          ],
        }],
        ['OS=="win"', {
          'sources': [
            'gfx/gdi_util.cc',
            'gfx/gdi_util.h',
            'gfx/icon_util.cc',
            'gfx/icon_util.h',
            'base/touch/touch_device_win.cc',
            'base/touch/touch_device_win.h',
          ],
          'sources!': [
            'base/touch/touch_factory.cc',
            'base/touch/touch_factory.h',
            'gfx/pango_util.h',
            'gfx/pango_util.cc',
            'gfx/platform_font_pango.cc',
            'gfx/platform_font_pango.h',
          ],
          'include_dirs': [
            '../',
            '../third_party/wtl/include',
          ],
          'msvs_settings': {
            'VCLinkerTool': {
              'DelayLoadDLLs': [
                'd2d1.dll',
                'd3d10_1.dll',
                'dwmapi.dll',
              ],
              'AdditionalDependencies': [
                'd2d1.lib',
                'd3d10_1.lib',
                'dwmapi.lib',
              ],
            },
          },
          'link_settings': {
            'libraries': [
              '-limm32.lib',
              '-ld2d1.lib',
              '-ldwmapi.lib',
              '-loleacc.lib',
            ],
          },
        },{  # OS!="win"
          'conditions': [
            ['use_aura==0', {
              'sources!': [
                'base/view_prop.cc',
                'base/view_prop.h',
              ],
            }],
          ],
          'sources!': [
            'base/dragdrop/drag_source.cc',
            'base/dragdrop/drag_source.h',
            'base/dragdrop/drag_drop_types.h',
            'base/dragdrop/drop_target.cc',
            'base/dragdrop/drop_target.h',
            'base/dragdrop/os_exchange_data.cc',
          ],
          'sources/': [
            ['exclude', '^base/win/*'],
          ],
        }],
        ['OS=="mac"', {
          'sources!': [
            'base/dragdrop/drag_utils.cc',
            'base/dragdrop/drag_utils.h',
            'base/touch/touch_factory.cc',
            'base/touch/touch_factory.h',
            'gfx/pango_util.h',
            'gfx/pango_util.cc',
            'gfx/platform_font_pango.h',
            'gfx/platform_font_pango.cc',
          ],
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/Accelerate.framework',
              '$(SDKROOT)/System/Library/Frameworks/AudioUnit.framework',
              '$(SDKROOT)/System/Library/Frameworks/CoreVideo.framework',
            ],
          },
        }],
        ['use_x11==1', {
          'all_dependent_settings': {
            'ldflags': [
              '-L<(PRODUCT_DIR)',
            ],
            'link_settings': {
              'libraries': [
                '-lX11',
                '-lXcursor',
                '-lXrandr',  # For XRR* function calls in x11_util.cc.
              ],
            },
          },
        }, {  # use_x11==0
          'sources/': [
            ['exclude', 'base/keycodes/keyboard_code_conversion_x.*'],
            ['exclude', 'base/x/*'],
          ],
        }],
        ['toolkit_views==0', {
          'sources!': [
            'base/events/event.cc',
            'base/events/event.h',
            'base/events/event_dispatcher.cc',
            'base/events/event_dispatcher.h',
            'base/events/event_handler.cc',
            'base/events/event_handler.h',
            'base/events/event_target.cc',
            'base/events/event_target.h',
            'base/x/events_x.cc',
          ],
        }],
        ['OS=="android"', {
          'sources!': [
            'base/dragdrop/drag_utils.cc',
            'base/dragdrop/drag_utils.h',
            'base/touch/touch_factory.cc',
            'base/touch/touch_factory.h',
            'gfx/pango_util.cc',
            'gfx/pango_util.h',
            'gfx/platform_font_pango.cc',
            'gfx/platform_font_pango.h',
          ],
          'dependencies': [
            'ui_jni_headers',
          ],
          'include_dirs': [
            '<(SHARED_INTERMEDIATE_DIR)/ui',
          ],
          'link_settings': {
            'libraries': [
              '-ljnigraphics',
            ],
          },
        }],
        ['OS=="android" and android_build_type==0', {
          'dependencies': [
            'ui_java',
          ],
        }],
        ['OS=="android" or OS=="ios"', {
          'sources!': [
            'gfx/render_text.cc',
            'gfx/render_text.h',
          ],
        }],
        ['OS=="linux"', {
          'libraries': [
            '-ldl',
          ],
        }],
        ['inside_chromium_build==0', {
          'dependencies': [
            '<(DEPTH)/webkit/support/setup_third_party.gyp:third_party_headers',
          ],
        }],
      ],
    },
  ],
  'conditions': [
    ['inside_chromium_build == 1', {
      'includes': [
        'ui_unittests.gypi',
      ]},
    ],
    ['OS=="android"' , {
       'targets': [
         {
           'target_name': 'ui_jni_headers',
           'type': 'none',
           'sources': [
             'android/java/src/org/chromium/ui/gfx/BitmapHelper.java',
             'android/java/src/org/chromium/ui/gfx/NativeWindow.java',
             'android/java/src/org/chromium/ui/SelectFileDialog.java',
           ],
           'variables': {
             'jni_gen_dir': 'ui',
           },
           'includes': [ '../build/jni_generator.gypi' ],
         },
         {
           'target_name': 'ui_java',
           'type': 'none',
           'variables': {
             'package_name': 'ui',
             'java_in_dir': '../ui/android/java',
           },
           'dependencies': [
             '../base/base.gyp:base_java',
           ],
           'includes': [ '../build/java.gypi' ],
         },
       ],
    }],
  ],
}
