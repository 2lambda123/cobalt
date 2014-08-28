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

# This file should be included in all .gyp files used by Cobalt. Normally,
# this should be done automatically by gyp_cobalt.
{
  'variables': {
    # Cobalt variables.
    'cobalt': 1,

    # Contains the current build configuration.
    'cobalt_config%': 'Gold',
    'cobalt_fastbuild%': 0,
    # Contains the sha1 of the cobalt git repo head.
    'cobalt_sha1%': 0,
    # Contains the name of the hosting OS. The value is defined by gyp_cobalt.
    'host_os%': 'win',

    # The variables allow changing the target type on platforms where the
    # native code may require an additional packaging step (ex. Android).
    'gtest_target_type%': 'executable',
    'final_executable_type%': 'executable',
    'posix_emulation_target_type%': 'static_library',

    # Needed for backwards compatibility with lbshell code.
    'lbshell_root%': '<(DEPTH)/lbshell',
    'lb_shell_sha1%': '<(cobalt_sha1)',

    # Compiler configuration.

    # The following variables are used to specify compiler and linker
    # flags by build configuration. Platform specific .gypi includes and
    # .gyp targets can use them to add additional flags.

    'compiler_flags%': [],
    'linker_flags%': [],

    'compiler_flags_debug%': [],
    'linker_flags_debug%': [],

    'compiler_flags_devel%': [],
    'linker_flags_devel%': [],

    'compiler_flags_qa%': [],
    'linker_flags_qa%': [],

    'compiler_flags_gold%': [],
    'linker_flags_gold%': [],

    'compiler_flags_host%': [],
    'linker_flags_host%': [],

    'platform_libraries%': [],

    # Customize variables used by Chromium's build/common.gypi.

    # Disable a check that looks for an official google api key.
    'use_official_google_api_keys': 0,
    # Prevents common.gypi from running a bash script which is not required
    # to compile Cobalt.
    'clang_use_chrome_plugins': 0,
    # Disables treat warnings as errors.
    'werror': '',
    # Cobalt doesn't currently support tcmalloc.
    'linux_use_tcmalloc': 0,
  },

  'target_defaults': {
    'defines': [
      '__LB_SHELL__',
    ],
    'cflags': [ '<@(compiler_flags)' ],
    'ldflags': [ '<@(linker_flags)' ],
    'cflags_host': [
      '<@(compiler_flags_host)',
      '-D__LB_HOST__'
    ],
    'ldflags_host': [ '<@(linker_flags_host)' ],

    # Location of Cygwin which is used by the build system when running on a
    # Windows platform.
    'msvs_cygwin_dirs': ['<(DEPTH)/third_party/cygwin'],

    # Allows any source file to include files relative to the source tree.
    'include_dirs': [ '<(DEPTH)' ],
    'libraries': [ '<@(platform_libraries)' ],

    # TODO(***REMOVED***): This is needed to support the option to include
    # posix_emulation.h to all compiled source files. This dependency should
    # be refactored and removed.
    'include_dirs_target': [
      '<(DEPTH)/lbshell/src',
      '<(DEPTH)/lbshell/src/platform/<(target_arch)',
      '<(DEPTH)/lbshell/src/platform/<(target_arch)/posix_emulation/lb_shell',
      # headers that we don't need, but should exist somewhere in the path:
      '<(DEPTH)/lbshell/src/platform/<(target_arch)/posix_emulation/place_holders',
    ],
    'conditions': [
      ['posix_emulation_target_type == "shared_library"', {
        'defines': [
          '__LB_BASE_SHARED__=1',
        ],
      }],
    ],

    # TODO(***REMOVED***): Revisit and remove unused configurations.
    'configurations': {
      'debug_base': {
        'abstract': 1,
        # no optimization, include symbols:
        'cflags': [ '<@(compiler_flags_debug)' ],
        'ldflags': [ '<@(linker_flags_debug)' ],
        'defines': [
          '_DEBUG',
          '__LB_SHELL__FORCE_LOGGING__',
          '__LB_SHELL__ENABLE_CONSOLE__',
          '__LB_SHELL__ENABLE_SCREENSHOT__',
        ],
      }, # end of debug_base
      'devel_base': {
        'abstract': 1,
        # optimize, include symbols:
        'cflags': [ '<@(compiler_flags_devel)' ],
        'ldflags': [ '<@(linker_flags_devel)' ],
        'defines': [
          'NDEBUG',
          '__LB_SHELL__FORCE_LOGGING__',
          '__LB_SHELL__ENABLE_CONSOLE__',
          '__LB_SHELL__ENABLE_SCREENSHOT__',
        ],
      }, # end of devel_base
      'qa_base': {
        'abstract': 1,
        # optimize:
        'cflags': [ '<@(compiler_flags_qa)' ],
        'ldflags': [ '<@(linker_flags_qa)' ],
        'defines': [
          'NDEBUG',
          '__LB_SHELL__FOR_QA__',
          '__LB_SHELL__ENABLE_CONSOLE__',
          '__LB_SHELL__ENABLE_SCREENSHOT__',
        ],
      }, # end of devel_base
      'gold_base': {
        'abstract': 1,
        # optimize:
        'cflags': [ '<@(compiler_flags_gold)' ],
        'ldflags': [ '<@(linker_flags_gold)' ],
        'defines': [
          'NDEBUG',
          '__LB_SHELL__FOR_RELEASE__',
        ],
      }, # end of gold_base
    }, # end of configurations
  }, # end of target_defaults
}
