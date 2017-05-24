# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    # Compiles and tests math libraries
    {
      'target_name': 'math',
      'type': 'static_library',
      'sources': [
        'box_f.cc',
        'box_f.h',
        'clamp.h',
        'cubic_bezier.cc',
        'cubic_bezier.h',
        'insets.cc',
        'insets.h',
        'insets_base.h',
        'insets_f.cc',
        'insets_f.h',
        'linear_interpolator.h',
        'matrix3_f.cc',
        'matrix3_f.h',
        'matrix_interpolation.cc',
        'matrix_interpolation.h',
        'point.cc',
        'point.h',
        'point3_f.cc',
        'point3_f.h',
        'point_base.h',
        'point_conversions.cc',
        'point_conversions.h',
        'point_f.cc',
        'point_f.h',
        'quad_f.cc',
        'quad_f.h',
        'r_tree.h',
        'r_tree_base.cc',
        'r_tree_base.h',
        'rational.h',
        'rect.cc',
        'rect.h',
        'rect_base.h',
        'rect_base_impl.h',
        'rect_conversions.cc',
        'rect_conversions.h',
        'rect_f.cc',
        'rect_f.h',
        'safe_integer_conversions.h',
        'size.cc',
        'size.h',
        'size_base.h',
        'size_conversions.cc',
        'size_conversions.h',
        'size_f.cc',
        'size_f.h',
        'transform_2d.cc',
        'transform_2d.h',
        'vector2d.cc',
        'vector2d.h',
        'vector2d_conversions.cc',
        'vector2d_conversions.h',
        'vector2d_f.cc',
        'vector2d_f.h',
        'vector3d_f.cc',
        'vector3d_f.h',
      ],
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
      ],
    },
    {
      'target_name': 'math_test',
      'type': '<(gtest_target_type)',
      'sources': [
        'box_unittest.cc',
        'cubic_bezier_unittest.cc',
        'insets_unittest.cc',
        'linear_interpolator_unittest.cc',
        'matrix3_unittest.cc',
        'matrix_interpolation_unittest.cc',
        'point3_unittest.cc',
        'point_unittest.cc',
        'quad_unittest.cc',
        'r_tree_unittest.cc',
        'rect_unittest.cc',
        'safe_integer_conversions_unittest.cc',
        'size_unittest.cc',
        'transform_2d_test.cc',
        'vector2d_unittest.cc',
        'vector3d_unittest.cc',
      ],
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/cobalt/base/base.gyp:base',
        '<(DEPTH)/cobalt/test/test.gyp:run_all_unittests',
        '<(DEPTH)/testing/gtest.gyp:gtest',
        'math',
      ],
    },
    {
      'target_name': 'math_test_deploy',
      'type': 'none',
      'dependencies': [
        'math_test',
      ],
      'variables': {
        'executable_name': 'math_test',
      },
      'includes': [ '../../starboard/build/deploy.gypi' ],
    },
  ],
}
