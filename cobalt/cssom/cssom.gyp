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
      'target_name': 'cssom',
      'type': 'static_library',
      'sources': [
        'absolute_url_value.cc',
        'absolute_url_value.h',
        'adjacent_selector.cc',
        'adjacent_selector.h',
        'after_pseudo_element.cc',
        'after_pseudo_element.h',
        'before_pseudo_element.cc',
        'before_pseudo_element.h',
        'cascade_priority.h',
        'cascaded_style.cc',
        'cascaded_style.h',
        'character_classification.h',
        'child_combinator.cc',
        'child_combinator.h',
        'class_selector.cc',
        'class_selector.h',
        'combinator.h',
        'combinator_visitor.h',
        'complex_selector.cc',
        'complex_selector.h',
        'compound_selector.cc',
        'compound_selector.h',
        'computed_style.cc',
        'computed_style.h',
        'const_string_list_value.h',
        'css_grouping_rule.cc',
        'css_grouping_rule.h',
        'css_media_rule.cc',
        'css_media_rule.h',
        'css_parser.h',
        'css_rule.h',
        'css_rule_list.cc',
        'css_rule_list.h',
        'css_style_declaration.cc',
        'css_style_declaration.h',
        'css_style_declaration_data.cc',
        'css_style_declaration_data.h',
        'css_style_rule.cc',
        'css_style_rule.h',
        'css_style_sheet.cc',
        'css_style_sheet.h',
        'css_transition.cc',
        'css_transition.h',
        'css_transition_set.cc',
        'css_transition_set.h',
        'descendant_combinator.cc',
        'descendant_combinator.h',
        'empty_pseudo_class.cc',
        'empty_pseudo_class.h',
        'following_sibling_combinator.cc',
        'following_sibling_combinator.h',
        'font_style_value.cc',
        'font_style_value.h',
        'font_weight_value.cc',
        'font_weight_value.h',
        'id_selector.cc',
        'id_selector.h',
        'initial_style.cc',
        'initial_style.h',
        'integer_value.cc',
        'integer_value.h',
        'keyword_names.cc',
        'keyword_names.h',
        'keyword_value.cc',
        'keyword_value.h',
        'length_value.cc',
        'length_value.h',
        'linear_gradient_value.cc',
        'linear_gradient_value.h',
        'list_value.h',
        'matrix_function.cc',
        'matrix_function.h',
        'media_feature.h',
        'media_feature_names.cc',
        'media_feature_names.h',
        'media_feature_keyword_value.cc',
        'media_feature_keyword_value.h',
        'media_feature_keyword_value_names.cc',
        'media_feature_keyword_value_names.h',
        'media_list.cc',
        'media_list.h',
        'media_type_names.cc',
        'media_type_names.h',
        'media_query.h',
        'mutation_observer.h',
        'next_sibling_combinator.cc',
        'next_sibling_combinator.h',
        'number_value.cc',
        'number_value.h',
        'percentage_value.cc',
        'percentage_value.h',
        'property_list_value.h',
        'property_names.cc',
        'property_names.h',
        'property_value.h',
        'property_value_visitor.cc',
        'property_value_visitor.h',
        'pseudo_class_names.cc',
        'pseudo_class_names.h',
        'pseudo_element_names.cc',
        'pseudo_element_names.h',
        'rgba_color_value.cc',
        'rgba_color_value.h',
        'ratio_value.cc',
        'ratio_value.h',
        'resolution_value.cc',
        'resolution_value.h',
        'rotate_function.cc',
        'rotate_function.h',
        'scale_function.cc',
        'scale_function.h',
        'scoped_list_value.h',
        'scoped_ref_list_value.h',
        'selector.h',
        'selector_visitor.h',
        'specificity.cc',
        'specificity.h',
        'specified_style.cc',
        'specified_style.h',
        'string_value.cc',
        'string_value.h',
        'style_sheet.h',
        'style_sheet_list.cc',
        'style_sheet_list.h',
        'time_list_value.h',
        'timing_function.cc',
        'timing_function.h',
        'timing_function_list_value.h',
        'transform_function.cc',
        'transform_function.h',
        'transform_function_list_value.cc',
        'transform_function_list_value.h',
        'transform_function_visitor.h',
        'translate_function.cc',
        'translate_function.h',
        'type_selector.cc',
        'type_selector.h',
        'url_value.cc',
        'url_value.h',
      ],
      'dependencies': [
        '<(DEPTH)/cobalt/base/base.gyp:base',
        '<(DEPTH)/cobalt/math/math.gyp:math',
        '<(DEPTH)/googleurl/googleurl.gyp:googleurl',
      ],
    },

    {
      'target_name': 'cssom_test',
      'type': '<(gtest_target_type)',
      'sources': [
        'cascade_priority_test.cc',
        'cascaded_style_test.cc',
        'computed_style_test.cc',
        'css_grouping_rule_test.cc',
        'css_rule_list_test.cc',
        'css_style_declaration_data_test.cc',
        'css_style_declaration_test.cc',
        'css_style_sheet_test.cc',
        'css_transition_set_test.cc',
        'css_transition_test.cc',
        'keyword_value_test.cc',
        'media_list_test.cc',
        'property_value_is_equal_test.cc',
        'property_value_to_string_test.cc',
        'property_value_visitor_test.cc',
        'selector_test.cc',
        'selector_visitor_test.cc',
        'specificity_test.cc',
        'style_sheet_list_test.cc',
        'timing_function_test.cc',
        'transform_function_visitor_test.cc',
      ],
      'dependencies': [
        '<(DEPTH)/base/base.gyp:run_all_unittests',
        '<(DEPTH)/testing/gmock.gyp:gmock',
        '<(DEPTH)/testing/gtest.gyp:gtest',
        'cssom',
      ],
    },

    {
      'target_name': 'cssom_test_deploy',
      'type': 'none',
      'dependencies': [
        'cssom_test',
      ],
      'variables': {
        'executable_name': 'cssom_test',
      },
      'includes': [ '../build/deploy.gypi' ],
    },
  ],
}
