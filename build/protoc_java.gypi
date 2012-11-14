# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This file is meant to be included into a target to provide a rule
# to invoke protoc in a consistent manner. This is only to be included
# for Java targets. When including this file, a .jar-file will be generated.
# For other targets, see protoc.gypi.
#
# To use this, create a gyp target with the following form:
# {
#   'target_name': 'my_proto_lib',
#   'sources': [
#     'foo.proto',
#     'bar.proto',
#   ],
#   'variables': {
#     'proto_in_dir': '.'
#   },
#   'includes': ['path/to/this/gypi/file'],
# }
#
# The 'proto_in_dir' variable must be the relative path to the
# directory containing the .proto files.  If left out, it defaults to '.'.
#
# The 'output_java_files' variable specifies a list of output files that will
# be generated. It is based on the package and java_outer_classname fields in
# the proto. All the values must be prefixed with >(java_out_dir), since that
# is the root directory of all the output.
#
# Implementation notes:
# A target_name of foo and proto-specified 'package' java.package.path produces:
#   <(PRODUCT_DIR)/java_proto/foo/{java/package/path/}{Foo,Bar}.java
# where Foo and Bar are taken from 'java_outer_classname' of the protos.
#
# How the .jar-file is created is different than how protoc is used for other
# targets, and as such, this lives in its own file.

{
  'variables': {
    'protoc': '<(PRODUCT_DIR)/<(EXECUTABLE_PREFIX)protoc<(EXECUTABLE_SUFFIX)',
    'java_out_dir': '<(PRODUCT_DIR)/java_proto/<(_target_name)/src',
    'proto_in_dir%': '.',
    'stamp_file': '<(java_out_dir).stamp',
    # Variables needed by java.gypi below.
    'package_name': '<(_target_name)',
    'java_in_dir': '<(DEPTH)/build/android/empty',
    'script': '<(DEPTH)/build/protoc_java.py',
    'generated_src_dirs': ['<(java_out_dir)'],
  },
  'actions': [
    {
      'action_name': 'genproto_java',
      'inputs': [
        '<(script)',
        '<(protoc)',
        '<@(_sources)',
      ],
      # We do not know the names of the generated files, so we use a stamp.
      'outputs': [
        '<(stamp_file)',
      ],
      'action': [
        '<(script)',
        '<(protoc)',
        '<(proto_in_dir)',
        '<(java_out_dir)',
        '<(stamp_file)',
        '<@(_sources)',
      ],
      'message': 'Generating Java code from <(proto_in_dir)',
    },
  ],
  'dependencies': [
    '<(DEPTH)/third_party/protobuf/protobuf.gyp:protoc#host',
    '<(DEPTH)/third_party/protobuf/protobuf.gyp:protobuf_lite_javalib',
  ],
  'includes': [ 'java.gypi' ],
}
