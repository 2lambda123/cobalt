# Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
    # Everything below  this is duplicated in the GN build, except Mac support.
    # If you change anything also change angle/BUILD.gn
    'variables':
    {
        'util_sources':
        [
            'com_utils.h',
            'keyboard.h',
            'geometry_utils.cpp',
            'geometry_utils.h',
            'mouse.h',
            'random_utils.cpp',
            'random_utils.h',
            'shader_utils.cpp',
            'shader_utils.h',
            'system_utils.h',
            'Event.h',
            'EGLWindow.cpp',
            'EGLWindow.h',
            'Matrix.cpp',
            'Matrix.h',
            'OSPixmap.h',
            'OSWindow.cpp',
            'OSWindow.h',
            'Timer.h',
        ],
        'util_win32_sources':
        [
            'windows/win32/Win32_system_utils.cpp',
            'windows/win32/Win32Pixmap.cpp',
            'windows/win32/Win32Pixmap.h',
            'windows/win32/Win32Window.cpp',
            'windows/win32/Win32Window.h',
            'windows/Windows_system_utils.cpp',
            'windows/WindowsTimer.cpp',
            'windows/WindowsTimer.h',
        ],
        'util_winrt_sources':
        [
            'windows/winrt/WinRT_system_utils.cpp',
            'windows/winrt/WinRTPixmap.cpp',
            'windows/winrt/WinRTWindow.cpp',
            'windows/winrt/WinRTWindow.h',
            'windows/Windows_system_utils.cpp',
            'windows/WindowsTimer.cpp',
            'windows/WindowsTimer.h',
        ],
        'util_linux_sources':
        [
            'linux/LinuxTimer.cpp',
            'linux/LinuxTimer.h',
            'posix/Posix_system_utils.cpp',
        ],
        'util_x11_sources':
        [
            'x11/X11Pixmap.cpp',
            'x11/X11Pixmap.h',
            'x11/X11Window.cpp',
            'x11/X11Window.h',
        ],
        'util_ozone_sources':
        [
            'ozone/OzonePixmap.cpp',
            'ozone/OzoneWindow.cpp',
            'ozone/OzoneWindow.h',
        ],
        'util_osx_sources':
        [
            'osx/OSXTimer.cpp',
            'osx/OSXTimer.h',
            'osx/OSXPixmap.mm',
            'osx/OSXPixmap.h',
            'osx/OSXWindow.mm',
            'osx/OSXWindow.h',
            'posix/Posix_system_utils.cpp',
        ],
        'util_android_sources':
        [
            'android/AndroidPixmap.cpp',
            'android/AndroidWindow.cpp',
            'android/AndroidWindow.h',
            'android/third_party/android_native_app_glue.c',
            'android/third_party/android_native_app_glue.h',
        ],
    },
    'targets':
    [
        {
            'target_name': 'angle_util_config',
            'type': 'none',
            'direct_dependent_settings':
            {
                'include_dirs':
                [
                    '<(angle_path)/include',
                    '<(angle_path)/util',
                ],
                'sources':
                [
                    '<@(util_sources)',
                ],
                'defines':
                [
                    'GL_GLEXT_PROTOTYPES',
                    'EGL_EGLEXT_PROTOTYPES',
                    'LIBANGLE_UTIL_IMPLEMENTATION',
                ],
                'conditions':
                [
                    ['OS=="win" and angle_build_winrt==0',
                    {
                        'sources':
                        [
                            '<@(util_win32_sources)',
                        ],
                    }],
                    ['OS=="win" and angle_build_winrt==1',
                    {
                        'sources':
                        [
                            '<@(util_winrt_sources)',
                        ],
                    }],
                    ['OS=="linux"',
                    {
                        'sources':
                        [
                            '<@(util_linux_sources)',
                        ],
                        'link_settings':
                        {
                            'libraries':
                            [
                                '-ldl',
                            ],
                        },
                    }],
                    ['use_x11==1',
                    {
                        'sources':
                        [
                            '<@(util_x11_sources)',
                        ],
                        'link_settings':
                        {
                            'ldflags':
                            [
                                '<!@(<(pkg-config) --libs-only-L --libs-only-other x11 xi)',
                            ],
                            'libraries':
                            [
                                '<!@(<(pkg-config) --libs-only-l x11 xi)',
                            ],
                        },
                    }],
                    ['use_ozone==1',
                    {
                        'sources':
                        [
                            '<@(util_ozone_sources)',
                        ],
                    }],
                    ['OS=="mac"',
                    {
                        'sources':
                        [
                            '<@(util_osx_sources)',
                        ],
                        'xcode_settings':
                        {
                            'DYLIB_INSTALL_NAME_BASE': '@rpath',
                        },
                        'link_settings':
                        {
                            'libraries':
                            [
                                '$(SDKROOT)/System/Library/Frameworks/AppKit.framework',
                                '$(SDKROOT)/System/Library/Frameworks/QuartzCore.framework',
                            ],
                        },
                    }],
                ],
            },
        },

        {
            'target_name': 'angle_util',
            'type': 'shared_library',
            'includes': [ '../gyp/common_defines.gypi', ],
            'dependencies':
            [
                'angle_util_config',
                '<(angle_path)/src/angle.gyp:angle_common',
                '<(angle_path)/src/angle.gyp:libEGL',
                '<(angle_path)/src/angle.gyp:libGLESv2',
            ],
            'export_dependent_settings':
            [
                '<(angle_path)/src/angle.gyp:angle_common',
            ],
            'direct_dependent_settings':
            {
                'include_dirs':
                [
                    '<(angle_path)/include',
                    '<(angle_path)/util',
                ],
                'defines':
                [
                    'GL_GLEXT_PROTOTYPES',
                    'EGL_EGLEXT_PROTOTYPES',
                ],
            },
        },

        {
            'target_name': 'angle_util_static',
            'type': 'static_library',
            'includes': [ '../gyp/common_defines.gypi', ],
            'dependencies':
            [
                'angle_util_config',
                '<(angle_path)/src/angle.gyp:angle_common',
                '<(angle_path)/src/angle.gyp:libEGL_static',
                '<(angle_path)/src/angle.gyp:libGLESv2_static',
            ],
            'export_dependent_settings':
            [
                '<(angle_path)/src/angle.gyp:angle_common',
            ],
            'direct_dependent_settings':
            {
                'include_dirs':
                [
                    '<(angle_path)/include',
                    '<(angle_path)/util',
                ],
                'defines':
                [
                    'GL_GLEXT_PROTOTYPES',
                    'EGL_EGLEXT_PROTOTYPES',
                ],
            },
        },
    ],
}
