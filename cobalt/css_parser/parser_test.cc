/*
 * Copyright 2014 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cobalt/css_parser/parser.h"

#include <cmath>
#include <string>

#include "base/bind.h"
#include "cobalt/cssom/active_pseudo_class.h"
#include "cobalt/cssom/after_pseudo_element.h"
#include "cobalt/cssom/before_pseudo_element.h"
#include "cobalt/cssom/child_combinator.h"
#include "cobalt/cssom/class_selector.h"
#include "cobalt/cssom/complex_selector.h"
#include "cobalt/cssom/compound_selector.h"
#include "cobalt/cssom/css_font_face_declaration_data.h"
#include "cobalt/cssom/css_font_face_rule.h"
#include "cobalt/cssom/css_keyframe_rule.h"
#include "cobalt/cssom/css_keyframes_rule.h"
#include "cobalt/cssom/css_rule_list.h"
#include "cobalt/cssom/css_style_declaration.h"
#include "cobalt/cssom/css_style_declaration_data.h"
#include "cobalt/cssom/css_style_rule.h"
#include "cobalt/cssom/css_style_sheet.h"
#include "cobalt/cssom/descendant_combinator.h"
#include "cobalt/cssom/empty_pseudo_class.h"
#include "cobalt/cssom/focus_pseudo_class.h"
#include "cobalt/cssom/following_sibling_combinator.h"
#include "cobalt/cssom/font_style_value.h"
#include "cobalt/cssom/font_weight_value.h"
#include "cobalt/cssom/hover_pseudo_class.h"
#include "cobalt/cssom/id_selector.h"
#include "cobalt/cssom/integer_value.h"
#include "cobalt/cssom/keyword_value.h"
#include "cobalt/cssom/length_value.h"
#include "cobalt/cssom/linear_gradient_value.h"
#include "cobalt/cssom/local_src_value.h"
#include "cobalt/cssom/matrix_function.h"
#include "cobalt/cssom/media_list.h"
#include "cobalt/cssom/media_query.h"
#include "cobalt/cssom/next_sibling_combinator.h"
#include "cobalt/cssom/not_pseudo_class.h"
#include "cobalt/cssom/number_value.h"
#include "cobalt/cssom/percentage_value.h"
#include "cobalt/cssom/property_definitions.h"
#include "cobalt/cssom/property_key_list_value.h"
#include "cobalt/cssom/property_list_value.h"
#include "cobalt/cssom/property_value_visitor.h"
#include "cobalt/cssom/rgba_color_value.h"
#include "cobalt/cssom/rotate_function.h"
#include "cobalt/cssom/scale_function.h"
#include "cobalt/cssom/simple_selector.h"
#include "cobalt/cssom/string_value.h"
#include "cobalt/cssom/time_list_value.h"
#include "cobalt/cssom/timing_function.h"
#include "cobalt/cssom/timing_function_list_value.h"
#include "cobalt/cssom/transform_function_list_value.h"
#include "cobalt/cssom/translate_function.h"
#include "cobalt/cssom/type_selector.h"
#include "cobalt/cssom/unicode_range_value.h"
#include "cobalt/cssom/url_src_value.h"
#include "cobalt/cssom/url_value.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace cobalt {
namespace css_parser {

using ::testing::_;
using ::testing::AtLeast;

class MockParserObserver {
 public:
  MOCK_METHOD1(OnWarning, void(const std::string& message));
  MOCK_METHOD1(OnError, void(const std::string& message));
};

class ParserTest : public ::testing::Test {
 public:
  ParserTest();
  ~ParserTest() OVERRIDE {}

 protected:
  ::testing::StrictMock<MockParserObserver> parser_observer_;
  Parser parser_;
  const base::SourceLocation source_location_;
};

ParserTest::ParserTest()
    : parser_(base::Bind(&MockParserObserver::OnWarning,
                         base::Unretained(&parser_observer_)),
              base::Bind(&MockParserObserver::OnError,
                         base::Unretained(&parser_observer_)),
              Parser::kShort),
      source_location_("[object ParserTest]", 1, 1) {}

// TODO(***REMOVED***): Test every reduction that has semantic action.

TEST_F(ParserTest, ParsesEmptyInput) {
  scoped_refptr<cssom::CSSStyleSheet> style_sheet =
      parser_.ParseStyleSheet("", source_location_);
  ASSERT_TRUE(style_sheet);
  EXPECT_EQ(0, style_sheet->css_rules()->length());
}

TEST_F(ParserTest, HandlesUnrecoverableSyntaxError) {
  EXPECT_CALL(
      parser_observer_,
      OnError("[object ParserTest]:1:1: error: unrecoverable syntax error"));

  scoped_refptr<cssom::CSSStyleSheet> style_sheet =
      parser_.ParseStyleSheet("@casino-royale", source_location_);
  ASSERT_TRUE(style_sheet);
  EXPECT_EQ(0, style_sheet->css_rules()->length());
}

TEST_F(ParserTest, ComputesErrorLocationOnFirstLine) {
  EXPECT_CALL(
      parser_observer_,
      OnError("[object ParserTest]:10:18: error: unrecoverable syntax error"));

  scoped_refptr<cssom::CSSStyleSheet> style_sheet = parser_.ParseStyleSheet(
      "cucumber", base::SourceLocation("[object ParserTest]", 10, 10));
  ASSERT_TRUE(style_sheet);
  EXPECT_EQ(0, style_sheet->css_rules()->length());
}

TEST_F(ParserTest, ComputesErrorLocationOnSecondLine) {
  EXPECT_CALL(
      parser_observer_,
      OnError("[object ParserTest]:11:9: error: unrecoverable syntax error"));

  scoped_refptr<cssom::CSSStyleSheet> style_sheet = parser_.ParseStyleSheet(
      "\n"
      "cucumber",
      base::SourceLocation("[object ParserTest]", 10, 10));
  ASSERT_TRUE(style_sheet);
  EXPECT_EQ(0, style_sheet->css_rules()->length());
}

TEST_F(ParserTest, IgnoresSgmlCommentDelimiters) {
  scoped_refptr<cssom::CSSStyleSheet> style_sheet =
      parser_.ParseStyleSheet("<!-- body {} -->", source_location_);
  ASSERT_TRUE(style_sheet);
  EXPECT_EQ(1, style_sheet->css_rules()->length());
}

TEST_F(ParserTest, RecoversFromInvalidAtToken) {
  EXPECT_CALL(parser_observer_,
              OnWarning("[object ParserTest]:1:9: warning: invalid rule"));

  scoped_refptr<cssom::CSSStyleSheet> style_sheet = parser_.ParseStyleSheet(
      "body {} @cobalt-magic; div {}", source_location_);
  ASSERT_TRUE(style_sheet);
  EXPECT_EQ(2, style_sheet->css_rules()->length());
}

TEST_F(ParserTest, RecoversFromInvalidRuleWhichEndsWithSemicolon) {
  EXPECT_CALL(parser_observer_,
              OnWarning("[object ParserTest]:1:9: warning: invalid rule"));

  scoped_refptr<cssom::CSSStyleSheet> style_sheet = parser_.ParseStyleSheet(
      "body {} @charset 'utf-8'; div {}", source_location_);
  ASSERT_TRUE(style_sheet);
  EXPECT_EQ(2, style_sheet->css_rules()->length());
}

TEST_F(ParserTest, RecoversFromInvalidRuleWhichEndsWithBlock) {
  EXPECT_CALL(parser_observer_,
              OnWarning("[object ParserTest]:1:9: warning: invalid rule"));

  scoped_refptr<cssom::CSSStyleSheet> style_sheet =
      parser_.ParseStyleSheet("body {} !important {} div {}", source_location_);
  ASSERT_TRUE(style_sheet);
  ASSERT_TRUE(style_sheet);
}

TEST_F(ParserTest, ParsesClassSelector) {
  scoped_refptr<cssom::CSSStyleSheet> style_sheet =
      parser_.ParseStyleSheet(".my-class {}", source_location_);

  ASSERT_EQ(1, style_sheet->css_rules()->length());
  ASSERT_EQ(cssom::CSSRule::kStyleRule,
            style_sheet->css_rules()->Item(0)->type());
  cssom::CSSStyleRule* style_rule = static_cast<cssom::CSSStyleRule*>(
      style_sheet->css_rules()->Item(0).get());
  ASSERT_EQ(1, style_rule->selectors().size());
  cssom::ComplexSelector* complex_selector =
      dynamic_cast<cssom::ComplexSelector*>(
          const_cast<cssom::Selector*>(style_rule->selectors()[0]));
  ASSERT_TRUE(complex_selector);
  cssom::CompoundSelector* compound_selector =
      complex_selector->first_selector();
  ASSERT_TRUE(compound_selector);
  ASSERT_EQ(1, compound_selector->simple_selectors().size());
  cssom::ClassSelector* class_selector =
      dynamic_cast<cssom::ClassSelector*>(const_cast<cssom::SimpleSelector*>(
          compound_selector->simple_selectors()[0]));
  ASSERT_TRUE(class_selector);
  EXPECT_EQ("my-class", class_selector->class_name());
}

TEST_F(ParserTest, ParsesAfterPseudoElementCSS2) {
  scoped_refptr<cssom::CSSStyleSheet> style_sheet =
      parser_.ParseStyleSheet(":after {}", source_location_);

  ASSERT_EQ(1, style_sheet->css_rules()->length());
  ASSERT_EQ(cssom::CSSRule::kStyleRule,
            style_sheet->css_rules()->Item(0)->type());
  cssom::CSSStyleRule* style_rule = static_cast<cssom::CSSStyleRule*>(
      style_sheet->css_rules()->Item(0).get());
  ASSERT_EQ(1, style_rule->selectors().size());
  cssom::ComplexSelector* complex_selector =
      dynamic_cast<cssom::ComplexSelector*>(
          const_cast<cssom::Selector*>(style_rule->selectors()[0]));
  ASSERT_TRUE(complex_selector);
  cssom::CompoundSelector* compound_selector =
      complex_selector->first_selector();
  ASSERT_TRUE(compound_selector);
  ASSERT_EQ(1, compound_selector->simple_selectors().size());
  cssom::AfterPseudoElement* after_pseudo_element =
      dynamic_cast<cssom::AfterPseudoElement*>(
          const_cast<cssom::SimpleSelector*>(
              compound_selector->simple_selectors()[0]));
  ASSERT_TRUE(after_pseudo_element);
}

TEST_F(ParserTest, ParsesAfterPseudoElementCSS3) {
  scoped_refptr<cssom::CSSStyleSheet> style_sheet =
      parser_.ParseStyleSheet("::after {}", source_location_);

  ASSERT_EQ(1, style_sheet->css_rules()->length());
  ASSERT_EQ(cssom::CSSRule::kStyleRule,
            style_sheet->css_rules()->Item(0)->type());
  cssom::CSSStyleRule* style_rule = static_cast<cssom::CSSStyleRule*>(
      style_sheet->css_rules()->Item(0).get());
  ASSERT_EQ(1, style_rule->selectors().size());
  cssom::ComplexSelector* complex_selector =
      dynamic_cast<cssom::ComplexSelector*>(
          const_cast<cssom::Selector*>(style_rule->selectors()[0]));
  ASSERT_TRUE(complex_selector);
  cssom::CompoundSelector* compound_selector =
      complex_selector->first_selector();
  ASSERT_TRUE(compound_selector);
  ASSERT_EQ(1, compound_selector->simple_selectors().size());
  cssom::AfterPseudoElement* after_pseudo_element =
      dynamic_cast<cssom::AfterPseudoElement*>(
          const_cast<cssom::SimpleSelector*>(
              compound_selector->simple_selectors()[0]));
  ASSERT_TRUE(after_pseudo_element);
}

TEST_F(ParserTest, ParsesBeforePseudoElementCSS2) {
  scoped_refptr<cssom::CSSStyleSheet> style_sheet =
      parser_.ParseStyleSheet(":before {}", source_location_);

  ASSERT_EQ(1, style_sheet->css_rules()->length());
  ASSERT_EQ(cssom::CSSRule::kStyleRule,
            style_sheet->css_rules()->Item(0)->type());
  cssom::CSSStyleRule* style_rule = static_cast<cssom::CSSStyleRule*>(
      style_sheet->css_rules()->Item(0).get());
  ASSERT_EQ(1, style_rule->selectors().size());
  cssom::ComplexSelector* complex_selector =
      dynamic_cast<cssom::ComplexSelector*>(
          const_cast<cssom::Selector*>(style_rule->selectors()[0]));
  ASSERT_TRUE(complex_selector);
  cssom::CompoundSelector* compound_selector =
      complex_selector->first_selector();
  ASSERT_TRUE(compound_selector);
  ASSERT_EQ(1, compound_selector->simple_selectors().size());
  cssom::BeforePseudoElement* before_pseudo_element =
      dynamic_cast<cssom::BeforePseudoElement*>(
          const_cast<cssom::SimpleSelector*>(
              compound_selector->simple_selectors()[0]));
  ASSERT_TRUE(before_pseudo_element);
}

TEST_F(ParserTest, ParsesBeforePseudoElementCSS3) {
  scoped_refptr<cssom::CSSStyleSheet> style_sheet =
      parser_.ParseStyleSheet("::before {}", source_location_);

  ASSERT_EQ(1, style_sheet->css_rules()->length());
  ASSERT_EQ(cssom::CSSRule::kStyleRule,
            style_sheet->css_rules()->Item(0)->type());
  cssom::CSSStyleRule* style_rule = static_cast<cssom::CSSStyleRule*>(
      style_sheet->css_rules()->Item(0).get());
  ASSERT_EQ(1, style_rule->selectors().size());
  cssom::ComplexSelector* complex_selector =
      dynamic_cast<cssom::ComplexSelector*>(
          const_cast<cssom::Selector*>(style_rule->selectors()[0]));
  ASSERT_TRUE(complex_selector);
  cssom::CompoundSelector* compound_selector =
      complex_selector->first_selector();
  ASSERT_TRUE(compound_selector);
  ASSERT_EQ(1, compound_selector->simple_selectors().size());
  cssom::BeforePseudoElement* before_pseudo_element =
      dynamic_cast<cssom::BeforePseudoElement*>(
          const_cast<cssom::SimpleSelector*>(
              compound_selector->simple_selectors()[0]));
  ASSERT_TRUE(before_pseudo_element);
}

TEST_F(ParserTest, ParsesActivePseudoClass) {
  scoped_refptr<cssom::CSSStyleSheet> style_sheet =
      parser_.ParseStyleSheet(":active {}", source_location_);

  ASSERT_EQ(1, style_sheet->css_rules()->length());
  ASSERT_EQ(cssom::CSSRule::kStyleRule,
            style_sheet->css_rules()->Item(0)->type());
  cssom::CSSStyleRule* style_rule = static_cast<cssom::CSSStyleRule*>(
      style_sheet->css_rules()->Item(0).get());
  ASSERT_EQ(1, style_rule->selectors().size());
  cssom::ComplexSelector* complex_selector =
      dynamic_cast<cssom::ComplexSelector*>(
          const_cast<cssom::Selector*>(style_rule->selectors()[0]));
  ASSERT_TRUE(complex_selector);
  cssom::CompoundSelector* compound_selector =
      complex_selector->first_selector();
  ASSERT_TRUE(compound_selector);
  ASSERT_EQ(1, compound_selector->simple_selectors().size());
  cssom::ActivePseudoClass* active_pseudo_class =
      dynamic_cast<cssom::ActivePseudoClass*>(
          const_cast<cssom::SimpleSelector*>(
              compound_selector->simple_selectors()[0]));
  ASSERT_TRUE(active_pseudo_class);
}

TEST_F(ParserTest, ParsesEmptyPseudoClass) {
  scoped_refptr<cssom::CSSStyleSheet> style_sheet =
      parser_.ParseStyleSheet(":empty {}", source_location_);

  ASSERT_EQ(1, style_sheet->css_rules()->length());
  ASSERT_EQ(cssom::CSSRule::kStyleRule,
            style_sheet->css_rules()->Item(0)->type());
  cssom::CSSStyleRule* style_rule = static_cast<cssom::CSSStyleRule*>(
      style_sheet->css_rules()->Item(0).get());
  ASSERT_EQ(1, style_rule->selectors().size());
  cssom::ComplexSelector* complex_selector =
      dynamic_cast<cssom::ComplexSelector*>(
          const_cast<cssom::Selector*>(style_rule->selectors()[0]));
  ASSERT_TRUE(complex_selector);
  cssom::CompoundSelector* compound_selector =
      complex_selector->first_selector();
  ASSERT_TRUE(compound_selector);
  ASSERT_EQ(1, compound_selector->simple_selectors().size());
  cssom::EmptyPseudoClass* empty_pseudo_class =
      dynamic_cast<cssom::EmptyPseudoClass*>(const_cast<cssom::SimpleSelector*>(
          compound_selector->simple_selectors()[0]));
  ASSERT_TRUE(empty_pseudo_class);
}

TEST_F(ParserTest, ParsesFocusPseudoClass) {
  scoped_refptr<cssom::CSSStyleSheet> style_sheet =
      parser_.ParseStyleSheet(":focus {}", source_location_);

  ASSERT_EQ(1, style_sheet->css_rules()->length());
  ASSERT_EQ(cssom::CSSRule::kStyleRule,
            style_sheet->css_rules()->Item(0)->type());
  cssom::CSSStyleRule* style_rule = static_cast<cssom::CSSStyleRule*>(
      style_sheet->css_rules()->Item(0).get());
  ASSERT_EQ(1, style_rule->selectors().size());
  cssom::ComplexSelector* complex_selector =
      dynamic_cast<cssom::ComplexSelector*>(
          const_cast<cssom::Selector*>(style_rule->selectors()[0]));
  ASSERT_TRUE(complex_selector);
  cssom::CompoundSelector* compound_selector =
      complex_selector->first_selector();
  ASSERT_TRUE(compound_selector);
  ASSERT_EQ(1, compound_selector->simple_selectors().size());
  cssom::FocusPseudoClass* focus_pseudo_class =
      dynamic_cast<cssom::FocusPseudoClass*>(const_cast<cssom::SimpleSelector*>(
          compound_selector->simple_selectors()[0]));
  ASSERT_TRUE(focus_pseudo_class);
}

TEST_F(ParserTest, ParsesHoverPseudoClass) {
  scoped_refptr<cssom::CSSStyleSheet> style_sheet =
      parser_.ParseStyleSheet(":hover {}", source_location_);

  ASSERT_EQ(1, style_sheet->css_rules()->length());
  ASSERT_EQ(cssom::CSSRule::kStyleRule,
            style_sheet->css_rules()->Item(0)->type());
  cssom::CSSStyleRule* style_rule = static_cast<cssom::CSSStyleRule*>(
      style_sheet->css_rules()->Item(0).get());
  ASSERT_EQ(1, style_rule->selectors().size());
  cssom::ComplexSelector* complex_selector =
      dynamic_cast<cssom::ComplexSelector*>(
          const_cast<cssom::Selector*>(style_rule->selectors()[0]));
  ASSERT_TRUE(complex_selector);
  cssom::CompoundSelector* compound_selector =
      complex_selector->first_selector();
  ASSERT_TRUE(compound_selector);
  ASSERT_EQ(1, compound_selector->simple_selectors().size());
  cssom::HoverPseudoClass* hover_pseudo_class =
      dynamic_cast<cssom::HoverPseudoClass*>(const_cast<cssom::SimpleSelector*>(
          compound_selector->simple_selectors()[0]));
  ASSERT_TRUE(hover_pseudo_class);
}

TEST_F(ParserTest, ParsesNotPseudoClass) {
  scoped_refptr<cssom::CSSStyleSheet> style_sheet =
      parser_.ParseStyleSheet(":not(div.my-class) {}", source_location_);

  ASSERT_EQ(1, style_sheet->css_rules()->length());
  ASSERT_EQ(cssom::CSSRule::kStyleRule,
            style_sheet->css_rules()->Item(0)->type());
  cssom::CSSStyleRule* style_rule = static_cast<cssom::CSSStyleRule*>(
      style_sheet->css_rules()->Item(0).get());
  ASSERT_EQ(1, style_rule->selectors().size());
  cssom::ComplexSelector* complex_selector =
      dynamic_cast<cssom::ComplexSelector*>(
          const_cast<cssom::Selector*>(style_rule->selectors()[0]));
  ASSERT_TRUE(complex_selector);
  cssom::CompoundSelector* compound_selector =
      complex_selector->first_selector();
  ASSERT_TRUE(compound_selector);
  ASSERT_EQ(1, compound_selector->simple_selectors().size());
  cssom::NotPseudoClass* not_pseudo_class =
      dynamic_cast<cssom::NotPseudoClass*>(const_cast<cssom::SimpleSelector*>(
          compound_selector->simple_selectors()[0]));
  ASSERT_TRUE(not_pseudo_class);

  // Check the selector within :not pseudo class.
  cssom::CompoundSelector* not_compound_selector = not_pseudo_class->selector();
  ASSERT_TRUE(not_compound_selector);
  ASSERT_EQ(2, not_compound_selector->simple_selectors().size());
  cssom::TypeSelector* type_selector =
      dynamic_cast<cssom::TypeSelector*>(const_cast<cssom::SimpleSelector*>(
          not_compound_selector->simple_selectors()[0]));
  ASSERT_TRUE(type_selector);
  EXPECT_EQ("div", type_selector->element_name());
  cssom::ClassSelector* class_selector =
      dynamic_cast<cssom::ClassSelector*>(const_cast<cssom::SimpleSelector*>(
          not_compound_selector->simple_selectors()[1]));
  ASSERT_TRUE(class_selector);
  EXPECT_EQ("my-class", class_selector->class_name());
}

TEST_F(ParserTest, ParsesNotPseudoClassShouldNotTakeEmptyParameter) {
  EXPECT_CALL(parser_observer_,
              OnWarning("[object ParserTest]:1:1: warning: unsupported "
                        "selector within :not()"));

  scoped_refptr<cssom::CSSStyleSheet> style_sheet =
      parser_.ParseStyleSheet(":not() {}", source_location_);

  EXPECT_EQ(0, style_sheet->css_rules()->length());
}

TEST_F(ParserTest, ParsesNotPseudoClassShouldNotTakeComplexSelector) {
  EXPECT_CALL(parser_observer_,
              OnWarning("[object ParserTest]:1:1: warning: unsupported "
                        "selector within :not()"));

  scoped_refptr<cssom::CSSStyleSheet> style_sheet =
      parser_.ParseStyleSheet(":not(div span) {}", source_location_);

  EXPECT_EQ(0, style_sheet->css_rules()->length());
}

TEST_F(ParserTest, ParsesIdSelector) {
  scoped_refptr<cssom::CSSStyleSheet> style_sheet =
      parser_.ParseStyleSheet("#my-id {}", source_location_);

  ASSERT_EQ(1, style_sheet->css_rules()->length());
  ASSERT_EQ(cssom::CSSRule::kStyleRule,
            style_sheet->css_rules()->Item(0)->type());
  cssom::CSSStyleRule* style_rule = static_cast<cssom::CSSStyleRule*>(
      style_sheet->css_rules()->Item(0).get());
  ASSERT_EQ(1, style_rule->selectors().size());
  cssom::ComplexSelector* complex_selector =
      dynamic_cast<cssom::ComplexSelector*>(
          const_cast<cssom::Selector*>(style_rule->selectors()[0]));
  ASSERT_TRUE(complex_selector);
  cssom::CompoundSelector* compound_selector =
      complex_selector->first_selector();
  ASSERT_TRUE(compound_selector);
  ASSERT_EQ(1, compound_selector->simple_selectors().size());
  cssom::IdSelector* id_selector =
      dynamic_cast<cssom::IdSelector*>(const_cast<cssom::SimpleSelector*>(
          compound_selector->simple_selectors()[0]));
  ASSERT_TRUE(id_selector);
  EXPECT_EQ("my-id", id_selector->id());
}

TEST_F(ParserTest, ParsesTypeSelector) {
  scoped_refptr<cssom::CSSStyleSheet> style_sheet =
      parser_.ParseStyleSheet("div {}", source_location_);

  ASSERT_EQ(1, style_sheet->css_rules()->length());
  ASSERT_EQ(cssom::CSSRule::kStyleRule,
            style_sheet->css_rules()->Item(0)->type());
  cssom::CSSStyleRule* style_rule = static_cast<cssom::CSSStyleRule*>(
      style_sheet->css_rules()->Item(0).get());
  ASSERT_EQ(1, style_rule->selectors().size());
  cssom::ComplexSelector* complex_selector =
      dynamic_cast<cssom::ComplexSelector*>(
          const_cast<cssom::Selector*>(style_rule->selectors()[0]));
  ASSERT_TRUE(complex_selector);
  cssom::CompoundSelector* compound_selector =
      complex_selector->first_selector();
  ASSERT_TRUE(compound_selector);
  ASSERT_EQ(1, compound_selector->simple_selectors().size());
  cssom::TypeSelector* type_selector =
      dynamic_cast<cssom::TypeSelector*>(const_cast<cssom::SimpleSelector*>(
          compound_selector->simple_selectors()[0]));
  ASSERT_TRUE(type_selector);
  EXPECT_EQ("div", type_selector->element_name());
}

TEST_F(ParserTest, ParsesCompoundSelector) {
  scoped_refptr<cssom::CSSStyleSheet> style_sheet =
      parser_.ParseStyleSheet("div.my-class {}", source_location_);

  ASSERT_EQ(1, style_sheet->css_rules()->length());
  ASSERT_EQ(cssom::CSSRule::kStyleRule,
            style_sheet->css_rules()->Item(0)->type());
  cssom::CSSStyleRule* style_rule = static_cast<cssom::CSSStyleRule*>(
      style_sheet->css_rules()->Item(0).get());
  ASSERT_EQ(1, style_rule->selectors().size());
  cssom::ComplexSelector* complex_selector =
      dynamic_cast<cssom::ComplexSelector*>(
          const_cast<cssom::Selector*>(style_rule->selectors()[0]));
  ASSERT_TRUE(complex_selector);
  cssom::CompoundSelector* compound_selector =
      complex_selector->first_selector();
  ASSERT_TRUE(compound_selector);
  ASSERT_EQ(2, compound_selector->simple_selectors().size());
  cssom::TypeSelector* type_selector =
      dynamic_cast<cssom::TypeSelector*>(const_cast<cssom::SimpleSelector*>(
          compound_selector->simple_selectors()[0]));
  ASSERT_TRUE(type_selector);
  EXPECT_EQ("div", type_selector->element_name());
  cssom::ClassSelector* class_selector =
      dynamic_cast<cssom::ClassSelector*>(const_cast<cssom::SimpleSelector*>(
          compound_selector->simple_selectors()[1]));
  ASSERT_TRUE(class_selector);
  EXPECT_EQ("my-class", class_selector->class_name());
}

TEST_F(ParserTest, ParsesComplexSelectorDescendantCombinator) {
  scoped_refptr<cssom::CSSStyleSheet> style_sheet =
      parser_.ParseStyleSheet("div div {}", source_location_);

  ASSERT_EQ(1, style_sheet->css_rules()->length());
  ASSERT_EQ(cssom::CSSRule::kStyleRule,
            style_sheet->css_rules()->Item(0)->type());
  cssom::CSSStyleRule* style_rule = static_cast<cssom::CSSStyleRule*>(
      style_sheet->css_rules()->Item(0).get());
  ASSERT_EQ(1, style_rule->selectors().size());
  cssom::ComplexSelector* complex_selector =
      dynamic_cast<cssom::ComplexSelector*>(
          const_cast<cssom::Selector*>(style_rule->selectors()[0]));
  ASSERT_TRUE(complex_selector);
  ASSERT_EQ(1, complex_selector->combinators().size());
  cssom::DescendantCombinator* descendant_combinator =
      dynamic_cast<cssom::DescendantCombinator*>(
          const_cast<cssom::Combinator*>(complex_selector->combinators()[0]));
  ASSERT_TRUE(descendant_combinator);
}

TEST_F(ParserTest, ParsesComplexSelectorFollowingSiblingCombinator) {
  scoped_refptr<cssom::CSSStyleSheet> style_sheet =
      parser_.ParseStyleSheet("div ~ div {}", source_location_);

  ASSERT_EQ(1, style_sheet->css_rules()->length());
  ASSERT_EQ(cssom::CSSRule::kStyleRule,
            style_sheet->css_rules()->Item(0)->type());
  cssom::CSSStyleRule* style_rule = static_cast<cssom::CSSStyleRule*>(
      style_sheet->css_rules()->Item(0).get());
  ASSERT_EQ(1, style_rule->selectors().size());
  cssom::ComplexSelector* complex_selector =
      dynamic_cast<cssom::ComplexSelector*>(
          const_cast<cssom::Selector*>(style_rule->selectors()[0]));
  ASSERT_TRUE(complex_selector);
  ASSERT_EQ(1, complex_selector->combinators().size());
  cssom::FollowingSiblingCombinator* following_sibling_combinator =
      dynamic_cast<cssom::FollowingSiblingCombinator*>(
          const_cast<cssom::Combinator*>(complex_selector->combinators()[0]));
  ASSERT_TRUE(following_sibling_combinator);
}

TEST_F(ParserTest, ParsesComplexSelectorChildCombinator) {
  scoped_refptr<cssom::CSSStyleSheet> style_sheet =
      parser_.ParseStyleSheet("div > div {}", source_location_);

  ASSERT_EQ(1, style_sheet->css_rules()->length());
  ASSERT_EQ(cssom::CSSRule::kStyleRule,
            style_sheet->css_rules()->Item(0)->type());
  cssom::CSSStyleRule* style_rule = static_cast<cssom::CSSStyleRule*>(
      style_sheet->css_rules()->Item(0).get());
  ASSERT_EQ(1, style_rule->selectors().size());
  cssom::ComplexSelector* complex_selector =
      dynamic_cast<cssom::ComplexSelector*>(
          const_cast<cssom::Selector*>(style_rule->selectors()[0]));
  ASSERT_TRUE(complex_selector);
  ASSERT_EQ(1, complex_selector->combinators().size());
  cssom::ChildCombinator* child_combinator =
      dynamic_cast<cssom::ChildCombinator*>(
          const_cast<cssom::Combinator*>(complex_selector->combinators()[0]));
  ASSERT_TRUE(child_combinator);
}

TEST_F(ParserTest, ParsesComplexSelectorNextSiblingCombinator) {
  scoped_refptr<cssom::CSSStyleSheet> style_sheet =
      parser_.ParseStyleSheet("div + div {}", source_location_);

  ASSERT_EQ(1, style_sheet->css_rules()->length());
  ASSERT_EQ(cssom::CSSRule::kStyleRule,
            style_sheet->css_rules()->Item(0)->type());
  cssom::CSSStyleRule* style_rule = static_cast<cssom::CSSStyleRule*>(
      style_sheet->css_rules()->Item(0).get());
  ASSERT_EQ(1, style_rule->selectors().size());
  cssom::ComplexSelector* complex_selector =
      dynamic_cast<cssom::ComplexSelector*>(
          const_cast<cssom::Selector*>(style_rule->selectors()[0]));
  ASSERT_TRUE(complex_selector);
  ASSERT_EQ(1, complex_selector->combinators().size());
  cssom::NextSiblingCombinator* next_sibling_combinator =
      dynamic_cast<cssom::NextSiblingCombinator*>(
          const_cast<cssom::Combinator*>(complex_selector->combinators()[0]));
  ASSERT_TRUE(next_sibling_combinator);
}

TEST_F(ParserTest, ParsesDeclarationListWithTrailingSemicolon) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "color: #0047ab;  /* Cobalt blue */\n"
          "font-size: 100px;\n",
          source_location_);

  EXPECT_TRUE(style->color());
  EXPECT_TRUE(style->font_size());
}

TEST_F(ParserTest, ParsesDeclarationListWithoutTrailingSemicolon) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "color: #0047ab;\n"
          "font-size: 100px\n",
          source_location_);

  EXPECT_TRUE(style->color());
  EXPECT_TRUE(style->font_size());
}

TEST_F(ParserTest, ParsesDeclarationListWithEmptyDeclaration) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "color: #0047ab;\n"
          ";\n"
          "font-size: 100px;\n",
          source_location_);

  EXPECT_TRUE(style->color());
  EXPECT_TRUE(style->font_size());
}

TEST_F(ParserTest, RecoversFromInvalidPropertyDeclaration) {
  EXPECT_CALL(
      parser_observer_,
      OnWarning("[object ParserTest]:2:1: warning: invalid declaration"));

  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "color: #0047ab;\n"
          "1px;\n"
          "font-size: 100px;\n",
          source_location_);

  EXPECT_TRUE(style->color());
  EXPECT_TRUE(style->font_size());
}

TEST_F(ParserTest, SilentlyIgnoresNonWebKitProperties) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "-moz-transform: scale(2);\n"
          "-ms-transform: scale(2);\n"
          "-o-transform: scale(2);\n"
          "transform: scale(2);\n",
          source_location_);

  EXPECT_TRUE(style->transform());
}

TEST_F(ParserTest, WarnsAboutInvalidStandardAndWebKitProperties) {
  EXPECT_CALL(
      parser_observer_,
      OnWarning("[object ParserTest]:1:1: warning: unsupported property pony"));

  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "pony: rainbowdash;\n"
          "color: #9edbf9;\n",
          source_location_);

  EXPECT_TRUE(style->color());
}

TEST_F(ParserTest, WarnsAboutInvalidPropertyValues) {
  EXPECT_CALL(
      parser_observer_,
      OnWarning("[object ParserTest]:1:19: warning: unsupported value"));

  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "background-color: morning haze over Paris;\n"
          "color: #fff;\n",
          source_location_);

  EXPECT_EQ(cssom::GetPropertyInitialValue(cssom::kBackgroundColorProperty),
            style->background_color());
  EXPECT_TRUE(style->color());
}

TEST_F(ParserTest, ParsesInherit) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-color: inherit;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetInherit(), style->background_color());
}

TEST_F(ParserTest, ParsesInitial) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-color: initial;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetInitial(), style->background_color());
}

TEST_F(ParserTest, ParsesBackgroundWithOnlyColor) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background: rgba(0, 0, 0, .8);",
                                        source_location_);

  scoped_refptr<cssom::RGBAColorValue> background_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->background_color().get());
  ASSERT_TRUE(background_color);
  EXPECT_EQ(0x000000cc, background_color->value());
}

TEST_F(ParserTest, ParsesBackgroundWithOnlyRepeat) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background: no-repeat;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> background_repeat_list =
      dynamic_cast<cssom::PropertyListValue*>(style->background_repeat().get());
  ASSERT_TRUE(background_repeat_list);
  EXPECT_EQ(2, background_repeat_list->value().size());

  EXPECT_EQ(cssom::KeywordValue::GetNoRepeat(),
            background_repeat_list->value()[0]);

  EXPECT_EQ(cssom::KeywordValue::GetNoRepeat(),
            background_repeat_list->value()[1]);
}

TEST_F(ParserTest, ParsesBackgroundWithColorAndURL) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "background: url(foo.png) rgba(0, 0, 0, .8);", source_location_);

  scoped_refptr<cssom::RGBAColorValue> background_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->background_color().get());
  ASSERT_TRUE(background_color);
  EXPECT_EQ(0x000000cc, background_color->value());

  scoped_refptr<cssom::PropertyListValue> background_image_list =
      dynamic_cast<cssom::PropertyListValue*>(style->background_image().get());
  ASSERT_TRUE(background_image_list);
  EXPECT_EQ(1, background_image_list->value().size());

  scoped_refptr<cssom::URLValue> url_value =
      dynamic_cast<cssom::URLValue*>(background_image_list->value()[0].get());
  EXPECT_EQ("foo.png", url_value->value());
}

TEST_F(ParserTest, ParsesBackgroundWithColorAndURLInDifferentOrder) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "background: rgba(0, 0, 0, .8) url(foo.png);", source_location_);

  scoped_refptr<cssom::RGBAColorValue> background_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->background_color().get());
  ASSERT_TRUE(background_color);
  EXPECT_EQ(0x000000cc, background_color->value());

  scoped_refptr<cssom::PropertyListValue> background_image_list =
      dynamic_cast<cssom::PropertyListValue*>(style->background_image().get());
  ASSERT_TRUE(background_image_list);
  EXPECT_EQ(1, background_image_list->value().size());

  scoped_refptr<cssom::URLValue> url_value =
      dynamic_cast<cssom::URLValue*>(background_image_list->value()[0].get());
  EXPECT_EQ("foo.png", url_value->value());
}

TEST_F(ParserTest, ParsesBackgroundWithColorAndPosition) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "background: rgba(0, 0, 0, .8) 70px 45%;", source_location_);

  scoped_refptr<cssom::RGBAColorValue> background_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->background_color().get());
  ASSERT_TRUE(background_color);
  EXPECT_EQ(0x000000cc, background_color->value());

  scoped_refptr<cssom::PropertyListValue> background_position_list =
      dynamic_cast<cssom::PropertyListValue*>(
          style->background_position().get());
  ASSERT_TRUE(background_position_list);

  scoped_refptr<cssom::LengthValue> length_value_left =
      dynamic_cast<cssom::LengthValue*>(
          background_position_list->value()[0].get());
  EXPECT_FLOAT_EQ(70, length_value_left->value());
  EXPECT_EQ(cssom::kPixelsUnit, length_value_left->unit());

  scoped_refptr<cssom::PercentageValue> percentage_value_right =
      dynamic_cast<cssom::PercentageValue*>(
          background_position_list->value()[1].get());
  EXPECT_FLOAT_EQ(0.45f, percentage_value_right->value());
}

TEST_F(ParserTest, ParsesBackgroundWithColorAndPositionInDifferentOrder) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "background: 70px 45% rgba(0, 0, 0, .8);", source_location_);

  scoped_refptr<cssom::PropertyListValue> background_position_list =
      dynamic_cast<cssom::PropertyListValue*>(
          style->background_position().get());
  ASSERT_TRUE(background_position_list);

  scoped_refptr<cssom::LengthValue> length_value_left =
      dynamic_cast<cssom::LengthValue*>(
          background_position_list->value()[0].get());
  EXPECT_FLOAT_EQ(70, length_value_left->value());
  EXPECT_EQ(cssom::kPixelsUnit, length_value_left->unit());

  scoped_refptr<cssom::PercentageValue> percentage_value_right =
      dynamic_cast<cssom::PercentageValue*>(
          background_position_list->value()[1].get());
  EXPECT_FLOAT_EQ(0.45f, percentage_value_right->value());

  scoped_refptr<cssom::RGBAColorValue> background_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->background_color().get());
  ASSERT_TRUE(background_color);
  EXPECT_EQ(0x000000cc, background_color->value());
}

TEST_F(ParserTest, ParsesBackgroundWithURLPositionAndSize) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "background: url(foo.png) center / 50px 80px;", source_location_);

  scoped_refptr<cssom::PropertyListValue> background_image_list =
      dynamic_cast<cssom::PropertyListValue*>(style->background_image().get());
  ASSERT_TRUE(background_image_list);
  EXPECT_EQ(1, background_image_list->value().size());

  scoped_refptr<cssom::URLValue> url_value =
      dynamic_cast<cssom::URLValue*>(background_image_list->value()[0].get());
  EXPECT_EQ("foo.png", url_value->value());

  scoped_refptr<cssom::PropertyListValue> background_position_list =
      dynamic_cast<cssom::PropertyListValue*>(
          style->background_position().get());
  ASSERT_TRUE(background_position_list);

  EXPECT_EQ(2, background_position_list->value().size());
  EXPECT_EQ(cssom::KeywordValue::GetCenter(),
            background_position_list->value()[0]);
  EXPECT_EQ(cssom::KeywordValue::GetCenter(),
            background_position_list->value()[1]);

  scoped_refptr<cssom::PropertyListValue> background_size_list =
      dynamic_cast<cssom::PropertyListValue*>(style->background_size().get());
  ASSERT_TRUE(background_size_list);

  scoped_refptr<cssom::LengthValue> size_value_left =
      dynamic_cast<cssom::LengthValue*>(background_size_list->value()[0].get());
  EXPECT_FLOAT_EQ(50, size_value_left->value());
  EXPECT_EQ(cssom::kPixelsUnit, size_value_left->unit());

  scoped_refptr<cssom::LengthValue> size_value_right =
      dynamic_cast<cssom::LengthValue*>(background_size_list->value()[1].get());
  EXPECT_FLOAT_EQ(80, size_value_right->value());
  EXPECT_EQ(cssom::kPixelsUnit, size_value_right->unit());
}

TEST_F(ParserTest, ParsesBackgroundWithURLPositionAndSizeInDifferentOrder) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "background: center/50px 80px url(foo.png);", source_location_);

  scoped_refptr<cssom::PropertyListValue> background_image_list =
      dynamic_cast<cssom::PropertyListValue*>(style->background_image().get());
  ASSERT_TRUE(background_image_list);
  EXPECT_EQ(1, background_image_list->value().size());

  scoped_refptr<cssom::URLValue> url_value =
      dynamic_cast<cssom::URLValue*>(background_image_list->value()[0].get());
  EXPECT_EQ("foo.png", url_value->value());

  scoped_refptr<cssom::PropertyListValue> background_position_list =
      dynamic_cast<cssom::PropertyListValue*>(
          style->background_position().get());
  ASSERT_TRUE(background_position_list);

  EXPECT_EQ(2, background_position_list->value().size());
  EXPECT_EQ(cssom::KeywordValue::GetCenter(),
            background_position_list->value()[0]);
  EXPECT_EQ(cssom::KeywordValue::GetCenter(),
            background_position_list->value()[1]);

  scoped_refptr<cssom::PropertyListValue> background_size_list =
      dynamic_cast<cssom::PropertyListValue*>(style->background_size().get());
  ASSERT_TRUE(background_size_list);

  scoped_refptr<cssom::LengthValue> size_value_left =
      dynamic_cast<cssom::LengthValue*>(background_size_list->value()[0].get());
  EXPECT_FLOAT_EQ(50, size_value_left->value());
  EXPECT_EQ(cssom::kPixelsUnit, size_value_left->unit());

  scoped_refptr<cssom::LengthValue> size_value_right =
      dynamic_cast<cssom::LengthValue*>(background_size_list->value()[1].get());
  EXPECT_FLOAT_EQ(80, size_value_right->value());
  EXPECT_EQ(cssom::kPixelsUnit, size_value_right->unit());
}

TEST_F(ParserTest, ParsesBackgroundWithURLNoRepeat) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background: url(foo.png) no-repeat",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> background_image_list =
      dynamic_cast<cssom::PropertyListValue*>(style->background_image().get());
  ASSERT_TRUE(background_image_list);
  EXPECT_EQ(1, background_image_list->value().size());

  scoped_refptr<cssom::URLValue> url_value =
      dynamic_cast<cssom::URLValue*>(background_image_list->value()[0].get());
  EXPECT_EQ("foo.png", url_value->value());

  scoped_refptr<cssom::PropertyListValue> background_repeat =
      dynamic_cast<cssom::PropertyListValue*>(style->background_repeat().get());
  ASSERT_TRUE(background_repeat);

  EXPECT_EQ(cssom::KeywordValue::GetNoRepeat(), background_repeat->value()[0]);
  EXPECT_EQ(cssom::KeywordValue::GetNoRepeat(), background_repeat->value()[1]);
}

TEST_F(ParserTest, ParsesBackgroundWithNoRepeatAndColor) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "background: no-repeat rgba(0, 0, 0, .8);", source_location_);

  scoped_refptr<cssom::PropertyListValue> background_repeat =
      dynamic_cast<cssom::PropertyListValue*>(style->background_repeat().get());
  ASSERT_TRUE(background_repeat);

  EXPECT_EQ(cssom::KeywordValue::GetNoRepeat(), background_repeat->value()[0]);
  EXPECT_EQ(cssom::KeywordValue::GetNoRepeat(), background_repeat->value()[1]);

  scoped_refptr<cssom::RGBAColorValue> background_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->background_color().get());
  ASSERT_TRUE(background_color);
  EXPECT_EQ(0x000000cc, background_color->value());
}

TEST_F(ParserTest, ParsesBackgroundWithBackgroundRepeatProperty) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "background: url(foo.png); background-repeat: no-repeat;",
          source_location_);

  scoped_refptr<cssom::PropertyListValue> background_image_list =
      dynamic_cast<cssom::PropertyListValue*>(style->background_image().get());
  ASSERT_TRUE(background_image_list);
  EXPECT_EQ(1, background_image_list->value().size());

  scoped_refptr<cssom::URLValue> url_value =
      dynamic_cast<cssom::URLValue*>(background_image_list->value()[0].get());
  EXPECT_EQ("foo.png", url_value->value());

  scoped_refptr<cssom::PropertyListValue> background_repeat =
      dynamic_cast<cssom::PropertyListValue*>(style->background_repeat().get());
  ASSERT_TRUE(background_repeat);
}

TEST_F(ParserTest, ParsesBackgroundWithURLNoRepeatAndPosition) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "background: url(foo.png) center no-repeat", source_location_);

  scoped_refptr<cssom::PropertyListValue> background_image_list =
      dynamic_cast<cssom::PropertyListValue*>(style->background_image().get());
  ASSERT_TRUE(background_image_list);
  EXPECT_EQ(1, background_image_list->value().size());

  scoped_refptr<cssom::URLValue> url_value =
      dynamic_cast<cssom::URLValue*>(background_image_list->value()[0].get());
  EXPECT_EQ("foo.png", url_value->value());

  scoped_refptr<cssom::PropertyListValue> background_position_list =
      dynamic_cast<cssom::PropertyListValue*>(
          style->background_position().get());
  ASSERT_TRUE(background_position_list);

  EXPECT_EQ(cssom::KeywordValue::GetCenter(),
            background_position_list->value()[0]);
  EXPECT_EQ(cssom::KeywordValue::GetCenter(),
            background_position_list->value()[1]);

  scoped_refptr<cssom::PropertyListValue> background_repeat =
      dynamic_cast<cssom::PropertyListValue*>(style->background_repeat().get());
  ASSERT_TRUE(background_repeat);

  EXPECT_EQ(cssom::KeywordValue::GetNoRepeat(), background_repeat->value()[0]);
  EXPECT_EQ(cssom::KeywordValue::GetNoRepeat(), background_repeat->value()[1]);
}

TEST_F(ParserTest, ParsesBackgroundWithNoRepeatAndCenter) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background: no-repeat center",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> background_position_list =
      dynamic_cast<cssom::PropertyListValue*>(
          style->background_position().get());
  ASSERT_TRUE(background_position_list);

  EXPECT_EQ(cssom::KeywordValue::GetCenter(),
            background_position_list->value()[0]);
  EXPECT_EQ(cssom::KeywordValue::GetCenter(),
            background_position_list->value()[1]);

  scoped_refptr<cssom::PropertyListValue> background_repeat =
      dynamic_cast<cssom::PropertyListValue*>(style->background_repeat().get());
  ASSERT_TRUE(background_repeat);

  EXPECT_EQ(cssom::KeywordValue::GetNoRepeat(), background_repeat->value()[0]);
  EXPECT_EQ(cssom::KeywordValue::GetNoRepeat(), background_repeat->value()[1]);
}

TEST_F(ParserTest, ParsesBackgroundWithNone) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background: none;", source_location_);

  scoped_refptr<cssom::PropertyListValue> background_image_list =
      dynamic_cast<cssom::PropertyListValue*>(style->background_image().get());
  ASSERT_TRUE(background_image_list);
  EXPECT_EQ(1, background_image_list->value().size());

  EXPECT_EQ(cssom::KeywordValue::GetNone(), background_image_list->value()[0]);
}

TEST_F(ParserTest, ParsesBackgroundWithTransparent) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background: transparent;",
                                        source_location_);

  scoped_refptr<cssom::RGBAColorValue> background_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->background_color().get());
  ASSERT_TRUE(background_color);
  EXPECT_EQ(0x00000000, background_color->value());
}

TEST_F(ParserTest, ParsesBackgroundWithLinearGradient) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "background: linear-gradient(rgba(0,0,0,0.56), rgba(0,0,0,0.9));",
          source_location_);

  scoped_refptr<cssom::PropertyListValue> background_image_list =
      dynamic_cast<cssom::PropertyListValue*>(style->background_image().get());
  ASSERT_TRUE(background_image_list);
  EXPECT_EQ(1, background_image_list->value().size());

  scoped_refptr<cssom::LinearGradientValue> linear_gradient_value =
      dynamic_cast<cssom::LinearGradientValue*>(
          background_image_list->value()[0].get());
  EXPECT_FALSE(linear_gradient_value->angle_in_radians());
  EXPECT_EQ(cssom::LinearGradientValue::kBottom,
            *linear_gradient_value->side_or_corner());

  const cssom::LinearGradientValue::ColorStopList* color_stop_list_value =
      linear_gradient_value->color_stop_list();
  EXPECT_EQ(2, color_stop_list_value->size());

  float color_list[2] = {0x0000008E, 0x000000E5};

  for (size_t i = 0; i < color_stop_list_value->size(); ++i) {
    const cssom::ColorStop* color_stop_value = (*color_stop_list_value)[i];
    scoped_refptr<cssom::RGBAColorValue> color = color_stop_value->rgba();
    ASSERT_TRUE(color);
    EXPECT_EQ(color_list[i], color->value());
    EXPECT_FALSE(color_stop_value->position());
  }
}

TEST_F(ParserTest, ParsesBackgroundWithLinearGradientContainsTransparent) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "background: linear-gradient(to bottom, rgba(0,0,0,0.56), "
          "rgba(0,0,0,0.9), transparent);",
          source_location_);

  scoped_refptr<cssom::PropertyListValue> background_image_list =
      dynamic_cast<cssom::PropertyListValue*>(style->background_image().get());
  ASSERT_TRUE(background_image_list);
  EXPECT_EQ(1, background_image_list->value().size());

  scoped_refptr<cssom::LinearGradientValue> linear_gradient_value =
      dynamic_cast<cssom::LinearGradientValue*>(
          background_image_list->value()[0].get());
  EXPECT_FALSE(linear_gradient_value->angle_in_radians());
  EXPECT_EQ(cssom::LinearGradientValue::kBottom,
            *linear_gradient_value->side_or_corner());

  const cssom::LinearGradientValue::ColorStopList* color_stop_list_value =
      linear_gradient_value->color_stop_list();
  EXPECT_EQ(3, color_stop_list_value->size());

  float color_list[3] = {0x0000008E, 0x000000E5, 0x00000000};

  for (size_t i = 0; i < color_stop_list_value->size(); ++i) {
    const cssom::ColorStop* color_stop_value = (*color_stop_list_value)[i];
    scoped_refptr<cssom::RGBAColorValue> color = color_stop_value->rgba();
    ASSERT_TRUE(color);
    EXPECT_EQ(color_list[i], color->value());
    EXPECT_FALSE(color_stop_value->position());
  }
}

TEST_F(ParserTest, ParsesBackgroundColor) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-color: #fff;",
                                        source_location_);

  scoped_refptr<cssom::RGBAColorValue> background_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->background_color().get());
  ASSERT_TRUE(background_color);
  EXPECT_EQ(0xffffffff, background_color->value());
}

TEST_F(ParserTest, ParsesBackgroundColorWithKeywordAqua) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-color: aqua;",
                                        source_location_);

  scoped_refptr<cssom::RGBAColorValue> background_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->background_color().get());
  ASSERT_TRUE(background_color);
  EXPECT_EQ(0x00FFFFFF, background_color->value());
}

TEST_F(ParserTest, ParsesBackgroundColorWithKeywordBlack) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-color: black;",
                                        source_location_);

  scoped_refptr<cssom::RGBAColorValue> background_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->background_color().get());
  ASSERT_TRUE(background_color);
  EXPECT_EQ(0x000000FF, background_color->value());
}

TEST_F(ParserTest, ParsesBackgroundColorWithKeywordBlue) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-color: blue;",
                                        source_location_);

  scoped_refptr<cssom::RGBAColorValue> background_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->background_color().get());
  ASSERT_TRUE(background_color);
  EXPECT_EQ(0x0000FFFF, background_color->value());
}

TEST_F(ParserTest, ParsesBackgroundColorWithKeywordFuchsia) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-color: fuchsia;",
                                        source_location_);

  scoped_refptr<cssom::RGBAColorValue> background_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->background_color().get());
  ASSERT_TRUE(background_color);
  EXPECT_EQ(0xFF00FFFF, background_color->value());
}

TEST_F(ParserTest, ParsesBackgroundColorWithKeywordGray) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-color: gray;",
                                        source_location_);

  scoped_refptr<cssom::RGBAColorValue> background_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->background_color().get());
  ASSERT_TRUE(background_color);
  EXPECT_EQ(0x808080FF, background_color->value());
}

TEST_F(ParserTest, ParsesBackgroundColorWithKeywordGreen) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-color: green;",
                                        source_location_);

  scoped_refptr<cssom::RGBAColorValue> background_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->background_color().get());
  ASSERT_TRUE(background_color);
  EXPECT_EQ(0x008000FF, background_color->value());
}

TEST_F(ParserTest, ParsesBackgroundColorWithKeywordLime) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-color: lime;",
                                        source_location_);

  scoped_refptr<cssom::RGBAColorValue> background_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->background_color().get());
  ASSERT_TRUE(background_color);
  EXPECT_EQ(0x00FF00FF, background_color->value());
}

TEST_F(ParserTest, ParsesBackgroundColorWithKeywordMaroon) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-color: maroon;",
                                        source_location_);

  scoped_refptr<cssom::RGBAColorValue> background_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->background_color().get());
  ASSERT_TRUE(background_color);
  EXPECT_EQ(0x800000FF, background_color->value());
}

TEST_F(ParserTest, ParsesBackgroundColorWithKeywordNavy) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-color: navy;",
                                        source_location_);

  scoped_refptr<cssom::RGBAColorValue> background_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->background_color().get());
  ASSERT_TRUE(background_color);
  EXPECT_EQ(0x000080FF, background_color->value());
}

TEST_F(ParserTest, ParsesBackgroundColorWithKeywordOlive) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-color: olive;",
                                        source_location_);

  scoped_refptr<cssom::RGBAColorValue> background_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->background_color().get());
  ASSERT_TRUE(background_color);
  EXPECT_EQ(0x808000FF, background_color->value());
}

TEST_F(ParserTest, ParsesBackgroundColorWithKeywordPurple) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-color: purple;",
                                        source_location_);

  scoped_refptr<cssom::RGBAColorValue> background_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->background_color().get());
  ASSERT_TRUE(background_color);
  EXPECT_EQ(0x800080FF, background_color->value());
}

TEST_F(ParserTest, ParsesBackgroundColorWithKeywordRed) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-color: red;",
                                        source_location_);

  scoped_refptr<cssom::RGBAColorValue> background_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->background_color().get());
  ASSERT_TRUE(background_color);
  EXPECT_EQ(0xFF0000FF, background_color->value());
}

TEST_F(ParserTest, ParsesBackgroundColorWithKeywordSilver) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-color: silver;",
                                        source_location_);

  scoped_refptr<cssom::RGBAColorValue> background_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->background_color().get());
  ASSERT_TRUE(background_color);
  EXPECT_EQ(0xC0C0C0FF, background_color->value());
}

TEST_F(ParserTest, ParsesBackgroundColorWithKeywordTeal) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-color: teal;",
                                        source_location_);

  scoped_refptr<cssom::RGBAColorValue> background_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->background_color().get());
  ASSERT_TRUE(background_color);
  EXPECT_EQ(0x008080FF, background_color->value());
}

TEST_F(ParserTest, ParsesBackgroundColorWithKeywordTransparent) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-color: transparent;",
                                        source_location_);

  scoped_refptr<cssom::RGBAColorValue> background_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->background_color().get());
  ASSERT_TRUE(background_color);
  EXPECT_EQ(0x00000000, background_color->value());
}

TEST_F(ParserTest, ParsesBackgroundColorWithKeywordWhite) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-color: white;",
                                        source_location_);

  scoped_refptr<cssom::RGBAColorValue> background_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->background_color().get());
  ASSERT_TRUE(background_color);
  EXPECT_EQ(0xFFFFFFFF, background_color->value());
}

TEST_F(ParserTest, ParsesBackgroundColorWithKeywordYellow) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-color: yellow;",
                                        source_location_);

  scoped_refptr<cssom::RGBAColorValue> background_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->background_color().get());
  ASSERT_TRUE(background_color);
  EXPECT_EQ(0xFFFF00FF, background_color->value());
}

TEST_F(ParserTest, ParsesBackgroundColorWithKeywordInitial) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-color: initial;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetInitial(), style->background_color());
}

TEST_F(ParserTest, ParsesBackgroundColorWithKeywordInherit) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-color: inherit;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetInherit(), style->background_color());
}

TEST_F(ParserTest, ParsesBackgroundImageNone) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-image: none;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> background_image_list =
      dynamic_cast<cssom::PropertyListValue*>(style->background_image().get());
  ASSERT_TRUE(background_image_list);
  EXPECT_EQ(1, background_image_list->value().size());

  EXPECT_EQ(cssom::KeywordValue::GetNone(), background_image_list->value()[0]);
}

TEST_F(ParserTest, ParsesBackgroundImageInitial) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-image: initial;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetInitial(), style->background_image());
}

TEST_F(ParserTest, ParsesBackgroundImageInherit) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-image: inherit;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetInherit(), style->background_image());
}

TEST_F(ParserTest, ParsesSingleBackgroundImageURL) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-image: url(foo.png);",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> background_image_list =
      dynamic_cast<cssom::PropertyListValue*>(style->background_image().get());
  ASSERT_TRUE(background_image_list);
  EXPECT_EQ(1, background_image_list->value().size());

  scoped_refptr<cssom::URLValue> url_value =
      dynamic_cast<cssom::URLValue*>(background_image_list->value()[0].get());
  EXPECT_EQ("foo.png", url_value->value());
}

TEST_F(ParserTest, ParsesMultipleBackgroundImageURLs) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "background-image: url(foo.png), url(bar.jpg), url(baz.png);",
          source_location_);

  scoped_refptr<cssom::PropertyListValue> background_image_list =
      dynamic_cast<cssom::PropertyListValue*>(style->background_image().get());
  ASSERT_TRUE(background_image_list);
  EXPECT_EQ(3, background_image_list->value().size());

  scoped_refptr<cssom::URLValue> url_value_1 =
      dynamic_cast<cssom::URLValue*>(background_image_list->value()[0].get());
  EXPECT_EQ("foo.png", url_value_1->value());

  scoped_refptr<cssom::URLValue> url_value_2 =
      dynamic_cast<cssom::URLValue*>(background_image_list->value()[1].get());
  EXPECT_EQ("bar.jpg", url_value_2->value());

  scoped_refptr<cssom::URLValue> url_value_3 =
      dynamic_cast<cssom::URLValue*>(background_image_list->value()[2].get());
  EXPECT_EQ("baz.png", url_value_3->value());
}

TEST_F(ParserTest, ParsesMultipleBackgroundImagesWithNone) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "background-image: url(foo.png), none, url(baz.png);",
          source_location_);

  scoped_refptr<cssom::PropertyListValue> background_image_list =
      dynamic_cast<cssom::PropertyListValue*>(style->background_image().get());
  ASSERT_TRUE(background_image_list);
  EXPECT_EQ(3, background_image_list->value().size());

  scoped_refptr<cssom::URLValue> url_value_1 =
      dynamic_cast<cssom::URLValue*>(background_image_list->value()[0].get());
  EXPECT_EQ("foo.png", url_value_1->value());

  EXPECT_EQ(cssom::KeywordValue::GetNone(), background_image_list->value()[1]);

  scoped_refptr<cssom::URLValue> url_value_3 =
      dynamic_cast<cssom::URLValue*>(background_image_list->value()[2].get());
  EXPECT_EQ("baz.png", url_value_3->value());
}

TEST_F(ParserTest, ParsesBackgroundImageLinearGradientWithDirection) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "background-image: linear-gradient(180deg, rgba(34, 34, 34, 0) 50px,"
          "rgba(34, 34, 34, 0) 100px);",
          source_location_);

  scoped_refptr<cssom::PropertyListValue> background_image_list =
      dynamic_cast<cssom::PropertyListValue*>(style->background_image().get());
  ASSERT_TRUE(background_image_list);
  EXPECT_EQ(1, background_image_list->value().size());

  scoped_refptr<cssom::LinearGradientValue> linear_gradient_value =
      dynamic_cast<cssom::LinearGradientValue*>(
          background_image_list->value()[0].get());
  EXPECT_FLOAT_EQ(static_cast<float>(M_PI),
                  *linear_gradient_value->angle_in_radians());
  EXPECT_FALSE(linear_gradient_value->side_or_corner());

  const cssom::LinearGradientValue::ColorStopList* color_stop_list_value =
      linear_gradient_value->color_stop_list();
  EXPECT_EQ(2, color_stop_list_value->size());

  float percentage_list[2] = {50.0f, 100.0f};

  for (size_t i = 0; i < color_stop_list_value->size(); ++i) {
    const cssom::ColorStop* color_stop_value = (*color_stop_list_value)[i];
    scoped_refptr<cssom::RGBAColorValue> color = color_stop_value->rgba();
    ASSERT_TRUE(color);
    EXPECT_EQ(0x22222200, color->value());

    scoped_refptr<cssom::LengthValue> position =
        dynamic_cast<cssom::LengthValue*>(color_stop_value->position().get());
    EXPECT_FLOAT_EQ(percentage_list[i], position->value());
    EXPECT_EQ(cssom::kPixelsUnit, position->unit());
  }
}

TEST_F(ParserTest, ParsesBackgroundImageLinearGradientWithoutDirection) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "background-image: linear-gradient(rgba(34, 34, 34, 0),"
          "rgba(34, 34, 34, 0) 60%);",
          source_location_);

  scoped_refptr<cssom::PropertyListValue> background_image_list =
      dynamic_cast<cssom::PropertyListValue*>(style->background_image().get());
  ASSERT_TRUE(background_image_list);
  EXPECT_EQ(1, background_image_list->value().size());

  scoped_refptr<cssom::LinearGradientValue> linear_gradient_value =
      dynamic_cast<cssom::LinearGradientValue*>(
          background_image_list->value()[0].get());
  EXPECT_FALSE(linear_gradient_value->angle_in_radians());
  EXPECT_EQ(cssom::LinearGradientValue::kBottom,
            *linear_gradient_value->side_or_corner());

  const cssom::LinearGradientValue::ColorStopList* color_stop_list_value =
      linear_gradient_value->color_stop_list();
  EXPECT_EQ(2, color_stop_list_value->size());

  for (size_t i = 0; i < color_stop_list_value->size(); ++i) {
    const cssom::ColorStop* color_stop_value = (*color_stop_list_value)[i];
    scoped_refptr<cssom::RGBAColorValue> color = color_stop_value->rgba();
    ASSERT_TRUE(color);
    EXPECT_EQ(0x22222200, color->value());

    if (i == 0) {
      EXPECT_FALSE(color_stop_value->position());
    } else {
      scoped_refptr<cssom::PercentageValue> position =
          dynamic_cast<cssom::PercentageValue*>(
              color_stop_value->position().get());
      EXPECT_FLOAT_EQ(0.6f, position->value());
    }
  }
}

TEST_F(ParserTest, ParsesBackgroundImageLinearGradientAndURL) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "background-image: linear-gradient(to right top, "
          "rgba(34, 34, 34, 0) 0%, rgba(34, 34, 34, 0) 80px), url(foo.png), "
          "url(bar.jpg);",
          source_location_);

  scoped_refptr<cssom::PropertyListValue> background_image_list =
      dynamic_cast<cssom::PropertyListValue*>(style->background_image().get());
  ASSERT_TRUE(background_image_list);
  EXPECT_EQ(3, background_image_list->value().size());

  scoped_refptr<cssom::LinearGradientValue> linear_gradient_value =
      dynamic_cast<cssom::LinearGradientValue*>(
          background_image_list->value()[0].get());
  EXPECT_FALSE(linear_gradient_value->angle_in_radians());
  EXPECT_EQ(cssom::LinearGradientValue::kTopRight,
            *linear_gradient_value->side_or_corner());

  const cssom::LinearGradientValue::ColorStopList* color_stop_list_value =
      linear_gradient_value->color_stop_list();
  EXPECT_EQ(2, color_stop_list_value->size());

  for (size_t i = 0; i < color_stop_list_value->size(); ++i) {
    const cssom::ColorStop* color_stop_value = (*color_stop_list_value)[i];
    scoped_refptr<cssom::RGBAColorValue> color = color_stop_value->rgba();
    ASSERT_TRUE(color);
    EXPECT_EQ(0x22222200, color->value());

    if (i == 0) {
      scoped_refptr<cssom::PercentageValue> position =
          dynamic_cast<cssom::PercentageValue*>(
              color_stop_value->position().get());
      EXPECT_FLOAT_EQ(0.0f, position->value());
    } else {
      scoped_refptr<cssom::LengthValue> position =
          dynamic_cast<cssom::LengthValue*>(color_stop_value->position().get());
      EXPECT_FLOAT_EQ(80.0f, position->value());
      EXPECT_EQ(cssom::kPixelsUnit, position->unit());
    }
  }

  scoped_refptr<cssom::URLValue> background_image_1 =
      dynamic_cast<cssom::URLValue*>(background_image_list->value()[1].get());
  ASSERT_TRUE(background_image_1);
  EXPECT_EQ("foo.png", background_image_1->value());

  scoped_refptr<cssom::URLValue> background_image_2 =
      dynamic_cast<cssom::URLValue*>(background_image_list->value()[2].get());
  ASSERT_TRUE(background_image_2);
  EXPECT_EQ("bar.jpg", background_image_2->value());
}

TEST_F(ParserTest, ParsesBackgroundPositionCenter) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-position: center;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> background_position_list =
      dynamic_cast<cssom::PropertyListValue*>(
          style->background_position().get());
  ASSERT_TRUE(background_position_list);
  EXPECT_EQ(2, background_position_list->value().size());

  EXPECT_EQ(cssom::KeywordValue::GetCenter(),
            background_position_list->value()[0]);
  EXPECT_EQ(cssom::KeywordValue::GetCenter(),
            background_position_list->value()[1]);
}

TEST_F(ParserTest, ParsesBackgroundPositionOneValue) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-position: 20%;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> background_position_list =
      dynamic_cast<cssom::PropertyListValue*>(
          style->background_position().get());
  ASSERT_TRUE(background_position_list);
  EXPECT_EQ(2, background_position_list->value().size());

  scoped_refptr<cssom::PercentageValue> percentage =
      dynamic_cast<cssom::PercentageValue*>(
          background_position_list->value()[0].get());
  ASSERT_TRUE(percentage);
  EXPECT_FLOAT_EQ(0.2f, percentage->value());

  EXPECT_EQ(cssom::KeywordValue::GetCenter(),
            background_position_list->value()[1]);
}

TEST_F(ParserTest, ParsesBackgroundPositionCenterLeft) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-position: center left;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> background_position_list =
      dynamic_cast<cssom::PropertyListValue*>(
          style->background_position().get());
  ASSERT_TRUE(background_position_list);
  EXPECT_EQ(2, background_position_list->value().size());

  EXPECT_EQ(cssom::KeywordValue::GetCenter(),
            background_position_list->value()[0]);
  EXPECT_EQ(cssom::KeywordValue::GetLeft(),
            background_position_list->value()[1]);
}

TEST_F(ParserTest, ParsesBackgroundPositionRightCenter) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-position: right center;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> background_position_list =
      dynamic_cast<cssom::PropertyListValue*>(
          style->background_position().get());
  ASSERT_TRUE(background_position_list);
  EXPECT_EQ(2, background_position_list->value().size());

  EXPECT_EQ(cssom::KeywordValue::GetRight(),
            background_position_list->value()[0]);
  EXPECT_EQ(cssom::KeywordValue::GetCenter(),
            background_position_list->value()[1]);
}

TEST_F(ParserTest, ParsesBackgroundPositionCenterCenter) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-position: center center;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> background_position_list =
      dynamic_cast<cssom::PropertyListValue*>(
          style->background_position().get());
  ASSERT_TRUE(background_position_list);
  EXPECT_EQ(2, background_position_list->value().size());

  EXPECT_EQ(cssom::KeywordValue::GetCenter(),
            background_position_list->value()[0]);
  EXPECT_EQ(cssom::KeywordValue::GetCenter(),
            background_position_list->value()[1]);
}

TEST_F(ParserTest, ParsesBackgroundPositionValueCenter) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-position: 20px center;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> background_position_list =
      dynamic_cast<cssom::PropertyListValue*>(
          style->background_position().get());
  ASSERT_TRUE(background_position_list);
  EXPECT_EQ(2, background_position_list->value().size());

  scoped_refptr<cssom::LengthValue> length = dynamic_cast<cssom::LengthValue*>(
      background_position_list->value()[0].get());
  ASSERT_TRUE(length);
  EXPECT_FLOAT_EQ(20, length->value());
  EXPECT_EQ(cssom::kPixelsUnit, length->unit());

  EXPECT_EQ(cssom::KeywordValue::GetCenter(),
            background_position_list->value()[1]);
}

TEST_F(ParserTest, ParsesBackgroundPositionBottomRight) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-position: bottom right;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> background_position_list =
      dynamic_cast<cssom::PropertyListValue*>(
          style->background_position().get());
  ASSERT_TRUE(background_position_list);
  EXPECT_EQ(2, background_position_list->value().size());

  EXPECT_EQ(cssom::KeywordValue::GetBottom(),
            background_position_list->value()[0]);
  EXPECT_EQ(cssom::KeywordValue::GetRight(),
            background_position_list->value()[1]);
}

TEST_F(ParserTest, ParsesBackgroundPositionWarnsAboutInvalidTwoValues_1) {
  EXPECT_CALL(parser_observer_, OnWarning("[object ParserTest]:1:22: warning: "
                                          "invalid background position value"));

  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-position: top 20%;",
                                        source_location_);
}

TEST_F(ParserTest, ParsesBackgroundPositionWarnsAboutInvalidTwoValues_2) {
  EXPECT_CALL(parser_observer_, OnWarning("[object ParserTest]:1:26: warning: "
                                          "invalid background position value"));

  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-position: 15% left;",
                                        source_location_);
}

TEST_F(ParserTest, ParsesBackgroundPositionWithThreeValues) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-position: left 10px top;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> background_position_list =
      dynamic_cast<cssom::PropertyListValue*>(
          style->background_position().get());
  ASSERT_TRUE(background_position_list);
  EXPECT_EQ(3, background_position_list->value().size());

  EXPECT_EQ(cssom::KeywordValue::GetLeft(),
            background_position_list->value()[0]);

  scoped_refptr<cssom::LengthValue> length = dynamic_cast<cssom::LengthValue*>(
      background_position_list->value()[1].get());
  ASSERT_TRUE(length);
  EXPECT_FLOAT_EQ(10, length->value());
  EXPECT_EQ(cssom::kPixelsUnit, length->unit());

  EXPECT_EQ(cssom::KeywordValue::GetTop(),
            background_position_list->value()[2]);
}

TEST_F(ParserTest, ParsesBackgroundPositionWarnsAboutInvalidThreeValues_1) {
  EXPECT_CALL(parser_observer_, OnWarning("[object ParserTest]:1:22: warning: "
                                          "invalid background position value"));

  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-position: left 30px 15%;",
                                        source_location_);
}

TEST_F(ParserTest, ParsesBackgroundPositionWarnsAboutInvalidThreeValues_2) {
  EXPECT_CALL(parser_observer_, OnWarning("[object ParserTest]:1:29: warning: "
                                          "invalid background position value"));

  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-position: bottom top 20%;",
                                        source_location_);
}

TEST_F(ParserTest, ParsesBackgroundPositionWarnsAboutInvalidThreeValues_3) {
  EXPECT_CALL(parser_observer_, OnWarning("[object ParserTest]:1:33: warning: "
                                          "invalid background position value"));

  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "background-position: 20% bottom right;", source_location_);
}

TEST_F(ParserTest, ParsesBackgroundPositionWithFourValues) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "background-position: bottom 20% right 10%;", source_location_);

  scoped_refptr<cssom::PropertyListValue> background_position_list =
      dynamic_cast<cssom::PropertyListValue*>(
          style->background_position().get());
  ASSERT_TRUE(background_position_list);
  EXPECT_EQ(4, background_position_list->value().size());

  EXPECT_EQ(cssom::KeywordValue::GetBottom(),
            background_position_list->value()[0]);

  scoped_refptr<cssom::PercentageValue> percentage_first =
      dynamic_cast<cssom::PercentageValue*>(
          background_position_list->value()[1].get());
  EXPECT_FLOAT_EQ(0.2f, percentage_first->value());

  EXPECT_EQ(cssom::KeywordValue::GetRight(),
            background_position_list->value()[2]);

  scoped_refptr<cssom::PercentageValue> percentage_second =
      dynamic_cast<cssom::PercentageValue*>(
          background_position_list->value()[3].get());
  EXPECT_FLOAT_EQ(0.1f, percentage_second->value());
}

TEST_F(ParserTest, ParsesBackgroundPositionWarnsAboutInvalidFourValues_1) {
  EXPECT_CALL(parser_observer_, OnWarning("[object ParserTest]:1:32: warning: "
                                          "invalid background position value"));

  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "background-position: left 30px right 20%;", source_location_);
}

TEST_F(ParserTest, ParsesBackgroundPositionWarnsAboutInvalidFourValues_2) {
  EXPECT_CALL(parser_observer_, OnWarning("[object ParserTest]:1:32: warning: "
                                          "invalid background position value"));

  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "background-position: left 30px left 20%;", source_location_);
}

TEST_F(ParserTest, ParsesBackgroundPositionDoublePercentage) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-position: 60% 90%;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> background_position_list =
      dynamic_cast<cssom::PropertyListValue*>(
          style->background_position().get());
  ASSERT_TRUE(background_position_list);
  EXPECT_EQ(2, background_position_list->value().size());

  scoped_refptr<cssom::PercentageValue> percentage_value_left =
      dynamic_cast<cssom::PercentageValue*>(
          background_position_list->value()[0].get());
  EXPECT_FLOAT_EQ(0.6f, percentage_value_left->value());

  scoped_refptr<cssom::PercentageValue> percentage_value_right =
      dynamic_cast<cssom::PercentageValue*>(
          background_position_list->value()[1].get());
  EXPECT_FLOAT_EQ(0.9f, percentage_value_right->value());
}

TEST_F(ParserTest, ParsesBackgroundPositionDoubleLength) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-position: 60px 90px;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> background_position_list =
      dynamic_cast<cssom::PropertyListValue*>(
          style->background_position().get());
  ASSERT_TRUE(background_position_list);
  EXPECT_EQ(2, background_position_list->value().size());

  scoped_refptr<cssom::LengthValue> length_value_left =
      dynamic_cast<cssom::LengthValue*>(
          background_position_list->value()[0].get());
  EXPECT_FLOAT_EQ(60, length_value_left->value());
  EXPECT_EQ(cssom::kPixelsUnit, length_value_left->unit());

  scoped_refptr<cssom::LengthValue> length_value_right =
      dynamic_cast<cssom::LengthValue*>(
          background_position_list->value()[1].get());
  EXPECT_FLOAT_EQ(90, length_value_right->value());
  EXPECT_EQ(cssom::kPixelsUnit, length_value_right->unit());
}

TEST_F(ParserTest, ParsesBackgroundPositionLengthAndPercentage) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-position: 60px 70%;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> background_position_list =
      dynamic_cast<cssom::PropertyListValue*>(
          style->background_position().get());
  ASSERT_TRUE(background_position_list);
  EXPECT_EQ(2, background_position_list->value().size());

  scoped_refptr<cssom::LengthValue> length_value_left =
      dynamic_cast<cssom::LengthValue*>(
          background_position_list->value()[0].get());
  EXPECT_FLOAT_EQ(60, length_value_left->value());
  EXPECT_EQ(cssom::kPixelsUnit, length_value_left->unit());

  scoped_refptr<cssom::PercentageValue> percentage_value_right =
      dynamic_cast<cssom::PercentageValue*>(
          background_position_list->value()[1].get());
  EXPECT_FLOAT_EQ(0.7f, percentage_value_right->value());
}

TEST_F(ParserTest, ParsesBackgroundPositionWithKeywordInitial) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-position: initial;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetInitial(), style->background_position());
}

TEST_F(ParserTest, ParsesBackgroundPositionWithKeywordInherit) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-position: inherit;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetInherit(), style->background_position());
}

TEST_F(ParserTest, ParsesBackgroundRepeatNoRepeat) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-repeat: no-repeat;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> background_repeat_list =
      dynamic_cast<cssom::PropertyListValue*>(style->background_repeat().get());
  ASSERT_TRUE(background_repeat_list);
  EXPECT_EQ(2, background_repeat_list->value().size());

  EXPECT_EQ(cssom::KeywordValue::GetNoRepeat(),
            background_repeat_list->value()[0]);

  EXPECT_EQ(cssom::KeywordValue::GetNoRepeat(),
            background_repeat_list->value()[1]);
}

TEST_F(ParserTest, ParsesBackgroundRepeatRepeat) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-repeat: repeat;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> background_repeat_list =
      dynamic_cast<cssom::PropertyListValue*>(style->background_repeat().get());
  ASSERT_TRUE(background_repeat_list);
  EXPECT_EQ(2, background_repeat_list->value().size());

  EXPECT_EQ(cssom::KeywordValue::GetRepeat(),
            background_repeat_list->value()[0]);

  EXPECT_EQ(cssom::KeywordValue::GetRepeat(),
            background_repeat_list->value()[1]);
}

TEST_F(ParserTest, ParsesBackgroundRepeatNoRepeatSpecifiedTwice) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "background-repeat: no-repeat no-repeat;", source_location_);

  scoped_refptr<cssom::PropertyListValue> background_repeat_list =
      dynamic_cast<cssom::PropertyListValue*>(style->background_repeat().get());
  ASSERT_TRUE(background_repeat_list);
  EXPECT_EQ(2, background_repeat_list->value().size());

  EXPECT_EQ(cssom::KeywordValue::GetNoRepeat(),
            background_repeat_list->value()[0]);

  EXPECT_EQ(cssom::KeywordValue::GetNoRepeat(),
            background_repeat_list->value()[1]);
}

TEST_F(ParserTest, ParsesBackgroundRepeatRepeatX) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-repeat: repeat-x;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> background_repeat_list =
      dynamic_cast<cssom::PropertyListValue*>(style->background_repeat().get());
  ASSERT_TRUE(background_repeat_list);
  EXPECT_EQ(2, background_repeat_list->value().size());

  EXPECT_EQ(cssom::KeywordValue::GetRepeat(),
            background_repeat_list->value()[0]);

  EXPECT_EQ(cssom::KeywordValue::GetNoRepeat(),
            background_repeat_list->value()[1]);
}

TEST_F(ParserTest, ParsesBackgroundRepeatRepeatY) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-repeat: repeat-y;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> background_repeat_list =
      dynamic_cast<cssom::PropertyListValue*>(style->background_repeat().get());
  ASSERT_TRUE(background_repeat_list);
  EXPECT_EQ(2, background_repeat_list->value().size());

  EXPECT_EQ(cssom::KeywordValue::GetNoRepeat(),
            background_repeat_list->value()[0]);

  EXPECT_EQ(cssom::KeywordValue::GetRepeat(),
            background_repeat_list->value()[1]);
}

TEST_F(ParserTest, ParsesBackgroundRepeatWithKeywordInitial) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-repeat: initial;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetInitial(), style->background_repeat());
}

TEST_F(ParserTest, ParsesBackgroundRepeatWithKeywordInherit) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-repeat: inherit;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetInherit(), style->background_repeat());
}

TEST_F(ParserTest, ParsesBackgroundSizeContain) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-size: contain;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetContain(), style->background_size());
}

TEST_F(ParserTest, ParsesBackgroundSizeCover) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-size: cover;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetCover(), style->background_size());
}

TEST_F(ParserTest, ParsesBackgroundSizeSingleAuto) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-size: auto;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> background_size_list =
      dynamic_cast<cssom::PropertyListValue*>(style->background_size().get());
  ASSERT_TRUE(background_size_list);
  EXPECT_EQ(2, background_size_list->value().size());
  EXPECT_EQ(cssom::KeywordValue::GetAuto(), background_size_list->value()[0]);
  EXPECT_EQ(cssom::KeywordValue::GetAuto(), background_size_list->value()[1]);
}

TEST_F(ParserTest, ParsesBackgroundSizeDoubleAuto) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-size: auto auto;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> background_size_list =
      dynamic_cast<cssom::PropertyListValue*>(style->background_size().get());
  ASSERT_TRUE(background_size_list);
  EXPECT_EQ(2, background_size_list->value().size());
  EXPECT_EQ(cssom::KeywordValue::GetAuto(), background_size_list->value()[0]);
  EXPECT_EQ(cssom::KeywordValue::GetAuto(), background_size_list->value()[1]);
}

TEST_F(ParserTest, ParsesBackgroundSizeSinglePercentage) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-size: 20%;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> background_size_list =
      dynamic_cast<cssom::PropertyListValue*>(style->background_size().get());
  ASSERT_TRUE(background_size_list);
  EXPECT_EQ(2, background_size_list->value().size());

  scoped_refptr<cssom::PercentageValue> percentage_value =
      dynamic_cast<cssom::PercentageValue*>(
          background_size_list->value()[0].get());
  EXPECT_FLOAT_EQ(0.2f, percentage_value->value());
  EXPECT_EQ(cssom::KeywordValue::GetAuto(), background_size_list->value()[1]);
}

TEST_F(ParserTest, ParsesBackgroundSizeDoublePercentage) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-size: 40% 60%;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> background_size_list =
      dynamic_cast<cssom::PropertyListValue*>(style->background_size().get());
  ASSERT_TRUE(background_size_list);
  EXPECT_EQ(2, background_size_list->value().size());

  scoped_refptr<cssom::PercentageValue> percentage_value_left =
      dynamic_cast<cssom::PercentageValue*>(
          background_size_list->value()[0].get());
  scoped_refptr<cssom::PercentageValue> percentage_value_right =
      dynamic_cast<cssom::PercentageValue*>(
          background_size_list->value()[1].get());

  EXPECT_FLOAT_EQ(0.4f, percentage_value_left->value());
  EXPECT_FLOAT_EQ(0.6f, percentage_value_right->value());
}

TEST_F(ParserTest, ParsesBackgroundSizeOneAutoOnePercentage) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-size: auto 20%;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> background_size_list =
      dynamic_cast<cssom::PropertyListValue*>(style->background_size().get());
  ASSERT_TRUE(background_size_list);
  EXPECT_EQ(2, background_size_list->value().size());

  EXPECT_EQ(cssom::KeywordValue::GetAuto(),
            background_size_list->value()[0].get());

  scoped_refptr<cssom::PercentageValue> percentage_value =
      dynamic_cast<cssom::PercentageValue*>(
          background_size_list->value()[1].get());
  EXPECT_FLOAT_EQ(0.2f, percentage_value->value());
}

TEST_F(ParserTest, ParsesBackgroundSizeOnePercentageOneAuto) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-size: 20% auto;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> background_size_list =
      dynamic_cast<cssom::PropertyListValue*>(style->background_size().get());
  ASSERT_TRUE(background_size_list);
  EXPECT_EQ(2, background_size_list->value().size());

  scoped_refptr<cssom::PercentageValue> percentage_value =
      dynamic_cast<cssom::PercentageValue*>(
          background_size_list->value()[0].get());
  EXPECT_FLOAT_EQ(0.2f, percentage_value->value());

  EXPECT_EQ(cssom::KeywordValue::GetAuto(),
            background_size_list->value()[1].get());
}

TEST_F(ParserTest, ParsesBackgroundSizeWithKeywordInitial) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-size: initial;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetInitial(), style->background_size());
}

TEST_F(ParserTest, ParsesBackgroundSizeWithKeywordInherit) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("background-size: inherit;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetInherit(), style->background_size());
}

TEST_F(ParserTest, ParsesBorderWithWidthColorAndStyle) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("border: .5em #fff solid;",
                                        source_location_);

  scoped_refptr<cssom::LengthValue> border_left_width =
      dynamic_cast<cssom::LengthValue*>(style->border_left_width().get());
  ASSERT_TRUE(border_left_width);
  EXPECT_FLOAT_EQ(0.5f, border_left_width->value());
  EXPECT_EQ(cssom::kFontSizesAkaEmUnit, border_left_width->unit());

  scoped_refptr<cssom::LengthValue> border_right_width =
      dynamic_cast<cssom::LengthValue*>(style->border_right_width().get());
  ASSERT_TRUE(border_right_width);
  EXPECT_FLOAT_EQ(0.5f, border_right_width->value());
  EXPECT_EQ(cssom::kFontSizesAkaEmUnit, border_right_width->unit());

  scoped_refptr<cssom::LengthValue> border_top_width =
      dynamic_cast<cssom::LengthValue*>(style->border_top_width().get());
  ASSERT_TRUE(border_top_width);
  EXPECT_FLOAT_EQ(0.5f, border_top_width->value());
  EXPECT_EQ(cssom::kFontSizesAkaEmUnit, border_top_width->unit());

  scoped_refptr<cssom::LengthValue> border_bottom_width =
      dynamic_cast<cssom::LengthValue*>(style->border_bottom_width().get());
  ASSERT_TRUE(border_bottom_width);
  EXPECT_FLOAT_EQ(0.5f, border_bottom_width->value());
  EXPECT_EQ(cssom::kFontSizesAkaEmUnit, border_bottom_width->unit());

  scoped_refptr<cssom::RGBAColorValue> border_left_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->border_left_color().get());
  ASSERT_TRUE(border_left_color);
  EXPECT_EQ(0xffffffff, border_left_color->value());

  scoped_refptr<cssom::RGBAColorValue> border_right_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->border_right_color().get());
  ASSERT_TRUE(border_right_color);
  EXPECT_EQ(0xffffffff, border_right_color->value());

  scoped_refptr<cssom::RGBAColorValue> border_top_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->border_top_color().get());
  ASSERT_TRUE(border_top_color);
  EXPECT_EQ(0xffffffff, border_top_color->value());

  scoped_refptr<cssom::RGBAColorValue> border_bottom_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->border_bottom_color().get());
  ASSERT_TRUE(border_bottom_color);
  EXPECT_EQ(0xffffffff, border_bottom_color->value());

  EXPECT_EQ(cssom::KeywordValue::GetSolid(), style->border_left_style());
  EXPECT_EQ(cssom::KeywordValue::GetSolid(), style->border_right_style());
  EXPECT_EQ(cssom::KeywordValue::GetSolid(), style->border_top_style());
  EXPECT_EQ(cssom::KeywordValue::GetSolid(), style->border_bottom_style());
}

TEST_F(ParserTest, ParsesBorderColorWidth) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("border: gray 20px;", source_location_);

  scoped_refptr<cssom::LengthValue> border_top_width =
      dynamic_cast<cssom::LengthValue*>(style->border_top_width().get());
  ASSERT_TRUE(border_top_width);
  EXPECT_FLOAT_EQ(20.0f, border_top_width->value());
  EXPECT_EQ(cssom::kPixelsUnit, border_top_width->unit());

  scoped_refptr<cssom::LengthValue> border_right_width =
      dynamic_cast<cssom::LengthValue*>(style->border_right_width().get());
  ASSERT_TRUE(border_right_width);
  EXPECT_FLOAT_EQ(20.0f, border_right_width->value());
  EXPECT_EQ(cssom::kPixelsUnit, border_right_width->unit());

  scoped_refptr<cssom::LengthValue> border_bottom_width =
      dynamic_cast<cssom::LengthValue*>(style->border_bottom_width().get());
  ASSERT_TRUE(border_bottom_width);
  EXPECT_FLOAT_EQ(20.0f, border_bottom_width->value());
  EXPECT_EQ(cssom::kPixelsUnit, border_bottom_width->unit());

  scoped_refptr<cssom::LengthValue> border_left_width =
      dynamic_cast<cssom::LengthValue*>(style->border_left_width().get());
  ASSERT_TRUE(border_left_width);
  EXPECT_FLOAT_EQ(20.0f, border_left_width->value());
  EXPECT_EQ(cssom::kPixelsUnit, border_left_width->unit());

  scoped_refptr<cssom::RGBAColorValue> border_top_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->border_top_color().get());
  ASSERT_TRUE(border_top_color);
  EXPECT_EQ(0x808080FF, border_top_color->value());

  scoped_refptr<cssom::RGBAColorValue> border_right_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->border_right_color().get());
  ASSERT_TRUE(border_right_color);
  EXPECT_EQ(0x808080FF, border_right_color->value());

  scoped_refptr<cssom::RGBAColorValue> border_bottom_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->border_bottom_color().get());
  ASSERT_TRUE(border_bottom_color);
  EXPECT_EQ(0x808080FF, border_bottom_color->value());

  scoped_refptr<cssom::RGBAColorValue> border_left_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->border_left_color().get());
  ASSERT_TRUE(border_left_color);
  EXPECT_EQ(0x808080FF, border_left_color->value());
}

TEST_F(ParserTest, ParsesSingleBorderColor) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("border-color: rgba(0, 0, 0, .8);",
                                        source_location_);

  scoped_refptr<cssom::RGBAColorValue> border_left_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->border_left_color().get());
  ASSERT_TRUE(border_left_color);
  EXPECT_EQ(0x000000cc, border_left_color->value());

  scoped_refptr<cssom::RGBAColorValue> border_right_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->border_right_color().get());
  ASSERT_TRUE(border_right_color);
  EXPECT_EQ(0x000000cc, border_right_color->value());

  scoped_refptr<cssom::RGBAColorValue> border_top_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->border_top_color().get());
  ASSERT_TRUE(border_top_color);
  EXPECT_EQ(0x000000cc, border_top_color->value());

  scoped_refptr<cssom::RGBAColorValue> border_bottom_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->border_bottom_color().get());
  ASSERT_TRUE(border_bottom_color);
  EXPECT_EQ(0x000000cc, border_bottom_color->value());
}

TEST_F(ParserTest, ParsesBorderColorWithTwoValues) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("border-color: #fff rgba(0, 0, 0, .8);",
                                        source_location_);

  scoped_refptr<cssom::RGBAColorValue> border_top_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->border_top_color().get());
  ASSERT_TRUE(border_top_color);
  EXPECT_EQ(0xFFFFFFFF, border_top_color->value());

  scoped_refptr<cssom::RGBAColorValue> border_right_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->border_right_color().get());
  ASSERT_TRUE(border_right_color);
  EXPECT_EQ(0x000000cc, border_right_color->value());

  scoped_refptr<cssom::RGBAColorValue> border_bottom_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->border_bottom_color().get());
  ASSERT_TRUE(border_bottom_color);
  EXPECT_EQ(0xFFFFFFFF, border_bottom_color->value());

  scoped_refptr<cssom::RGBAColorValue> border_left_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->border_left_color().get());
  ASSERT_TRUE(border_left_color);
  EXPECT_EQ(0x000000cc, border_left_color->value());
}

TEST_F(ParserTest, ParsesBorderColorWithThreeValues) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "border-color: #fff rgba(0, 0, 0, .8) rgba(0,0,0,0.9);",
          source_location_);

  scoped_refptr<cssom::RGBAColorValue> border_top_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->border_top_color().get());
  ASSERT_TRUE(border_top_color);
  EXPECT_EQ(0xFFFFFFFF, border_top_color->value());

  scoped_refptr<cssom::RGBAColorValue> border_right_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->border_right_color().get());
  ASSERT_TRUE(border_right_color);
  EXPECT_EQ(0x000000cc, border_right_color->value());

  scoped_refptr<cssom::RGBAColorValue> border_bottom_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->border_bottom_color().get());
  ASSERT_TRUE(border_bottom_color);
  EXPECT_EQ(0x000000E5, border_bottom_color->value());

  scoped_refptr<cssom::RGBAColorValue> border_left_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->border_left_color().get());
  ASSERT_TRUE(border_left_color);
  EXPECT_EQ(0x000000cc, border_left_color->value());
}

TEST_F(ParserTest, ParsesBorderColorWithFourValues) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "border-color: #fff rgba(0, 0, 0, .8) rgba(0,0,0,0.9) "
          "rgba(0,0,0,0.56);",
          source_location_);

  scoped_refptr<cssom::RGBAColorValue> border_top_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->border_top_color().get());
  ASSERT_TRUE(border_top_color);
  EXPECT_EQ(0xFFFFFFFF, border_top_color->value());

  scoped_refptr<cssom::RGBAColorValue> border_right_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->border_right_color().get());
  ASSERT_TRUE(border_right_color);
  EXPECT_EQ(0x000000CC, border_right_color->value());

  scoped_refptr<cssom::RGBAColorValue> border_bottom_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->border_bottom_color().get());
  ASSERT_TRUE(border_bottom_color);
  EXPECT_EQ(0x000000E5, border_bottom_color->value());

  scoped_refptr<cssom::RGBAColorValue> border_left_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->border_left_color().get());
  ASSERT_TRUE(border_left_color);
  EXPECT_EQ(0x0000008E, border_left_color->value());
}

TEST_F(ParserTest, ParsesBorderTopColor) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("border-top-color: rgba(0, 0, 0, .8);",
                                        source_location_);

  scoped_refptr<cssom::RGBAColorValue> border_top_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->border_top_color().get());
  ASSERT_TRUE(border_top_color);
  EXPECT_EQ(0x000000cc, border_top_color->value());
}

TEST_F(ParserTest, ParsesBorderRightColor) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "border-right-color: rgba(0, 0, 0, .8);", source_location_);

  scoped_refptr<cssom::RGBAColorValue> border_right_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->border_right_color().get());
  ASSERT_TRUE(border_right_color);
  EXPECT_EQ(0x000000cc, border_right_color->value());
}

TEST_F(ParserTest, ParsesBorderBottomColor) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "border-bottom-color: rgba(0, 0, 0, .8);", source_location_);

  scoped_refptr<cssom::RGBAColorValue> border_bottom_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->border_bottom_color().get());
  ASSERT_TRUE(border_bottom_color);
  EXPECT_EQ(0x000000cc, border_bottom_color->value());
}

TEST_F(ParserTest, ParsesBorderLeftColor) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("border-left-color: rgba(0, 0, 0, .8);",
                                        source_location_);

  scoped_refptr<cssom::RGBAColorValue> border_left_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->border_left_color().get());
  ASSERT_TRUE(border_left_color);
  EXPECT_EQ(0x000000cc, border_left_color->value());
}

TEST_F(ParserTest, ParsesSingleBorderWidth) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("border-width: .8em;",
                                        source_location_);

  scoped_refptr<cssom::LengthValue> border_top_width =
      dynamic_cast<cssom::LengthValue*>(style->border_top_width().get());
  ASSERT_TRUE(border_top_width);
  EXPECT_FLOAT_EQ(0.8f, border_top_width->value());
  EXPECT_EQ(cssom::kFontSizesAkaEmUnit, border_top_width->unit());

  scoped_refptr<cssom::LengthValue> border_right_width =
      dynamic_cast<cssom::LengthValue*>(style->border_right_width().get());
  ASSERT_TRUE(border_right_width);
  EXPECT_FLOAT_EQ(0.8f, border_right_width->value());
  EXPECT_EQ(cssom::kFontSizesAkaEmUnit, border_right_width->unit());

  scoped_refptr<cssom::LengthValue> border_bottom_width =
      dynamic_cast<cssom::LengthValue*>(style->border_bottom_width().get());
  ASSERT_TRUE(border_bottom_width);
  EXPECT_FLOAT_EQ(0.8f, border_bottom_width->value());
  EXPECT_EQ(cssom::kFontSizesAkaEmUnit, border_bottom_width->unit());

  scoped_refptr<cssom::LengthValue> border_left_width =
      dynamic_cast<cssom::LengthValue*>(style->border_left_width().get());
  ASSERT_TRUE(border_left_width);
  EXPECT_FLOAT_EQ(0.8f, border_left_width->value());
  EXPECT_EQ(cssom::kFontSizesAkaEmUnit, border_left_width->unit());
}

TEST_F(ParserTest, ParsesBorderWidthWithTwoValues) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("border-width: .8em 20px;",
                                        source_location_);

  scoped_refptr<cssom::LengthValue> border_top_width =
      dynamic_cast<cssom::LengthValue*>(style->border_top_width().get());
  ASSERT_TRUE(border_top_width);
  EXPECT_FLOAT_EQ(0.8f, border_top_width->value());
  EXPECT_EQ(cssom::kFontSizesAkaEmUnit, border_top_width->unit());

  scoped_refptr<cssom::LengthValue> border_right_width =
      dynamic_cast<cssom::LengthValue*>(style->border_right_width().get());
  ASSERT_TRUE(border_right_width);
  EXPECT_FLOAT_EQ(20.0f, border_right_width->value());
  EXPECT_EQ(cssom::kPixelsUnit, border_right_width->unit());

  scoped_refptr<cssom::LengthValue> border_bottom_width =
      dynamic_cast<cssom::LengthValue*>(style->border_bottom_width().get());
  ASSERT_TRUE(border_bottom_width);
  EXPECT_FLOAT_EQ(0.8f, border_bottom_width->value());
  EXPECT_EQ(cssom::kFontSizesAkaEmUnit, border_bottom_width->unit());

  scoped_refptr<cssom::LengthValue> border_left_width =
      dynamic_cast<cssom::LengthValue*>(style->border_left_width().get());
  ASSERT_TRUE(border_left_width);
  EXPECT_FLOAT_EQ(20.0f, border_left_width->value());
  EXPECT_EQ(cssom::kPixelsUnit, border_left_width->unit());
}

TEST_F(ParserTest, ParsesBorderWidthWithThreeValues) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("border-width: .8em 20px 30px;",
                                        source_location_);

  scoped_refptr<cssom::LengthValue> border_top_width =
      dynamic_cast<cssom::LengthValue*>(style->border_top_width().get());
  ASSERT_TRUE(border_top_width);
  EXPECT_FLOAT_EQ(0.8f, border_top_width->value());
  EXPECT_EQ(cssom::kFontSizesAkaEmUnit, border_top_width->unit());

  scoped_refptr<cssom::LengthValue> border_right_width =
      dynamic_cast<cssom::LengthValue*>(style->border_right_width().get());
  ASSERT_TRUE(border_right_width);
  EXPECT_FLOAT_EQ(20.0f, border_right_width->value());
  EXPECT_EQ(cssom::kPixelsUnit, border_right_width->unit());

  scoped_refptr<cssom::LengthValue> border_bottom_width =
      dynamic_cast<cssom::LengthValue*>(style->border_bottom_width().get());
  ASSERT_TRUE(border_bottom_width);
  EXPECT_FLOAT_EQ(30.0f, border_bottom_width->value());
  EXPECT_EQ(cssom::kPixelsUnit, border_bottom_width->unit());

  scoped_refptr<cssom::LengthValue> border_left_width =
      dynamic_cast<cssom::LengthValue*>(style->border_left_width().get());
  ASSERT_TRUE(border_left_width);
  EXPECT_FLOAT_EQ(20.0f, border_left_width->value());
  EXPECT_EQ(cssom::kPixelsUnit, border_left_width->unit());
}

TEST_F(ParserTest, ParsesBorderWidthWithFourValues) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("border-width: .8em 20px 30px .5em;",
                                        source_location_);

  scoped_refptr<cssom::LengthValue> border_top_width =
      dynamic_cast<cssom::LengthValue*>(style->border_top_width().get());
  ASSERT_TRUE(border_top_width);
  EXPECT_FLOAT_EQ(0.8f, border_top_width->value());
  EXPECT_EQ(cssom::kFontSizesAkaEmUnit, border_top_width->unit());

  scoped_refptr<cssom::LengthValue> border_right_width =
      dynamic_cast<cssom::LengthValue*>(style->border_right_width().get());
  ASSERT_TRUE(border_right_width);
  EXPECT_FLOAT_EQ(20.0f, border_right_width->value());
  EXPECT_EQ(cssom::kPixelsUnit, border_right_width->unit());

  scoped_refptr<cssom::LengthValue> border_bottom_width =
      dynamic_cast<cssom::LengthValue*>(style->border_bottom_width().get());
  ASSERT_TRUE(border_bottom_width);
  EXPECT_FLOAT_EQ(30.0f, border_bottom_width->value());
  EXPECT_EQ(cssom::kPixelsUnit, border_bottom_width->unit());

  scoped_refptr<cssom::LengthValue> border_left_width =
      dynamic_cast<cssom::LengthValue*>(style->border_left_width().get());
  ASSERT_TRUE(border_left_width);
  EXPECT_FLOAT_EQ(0.5f, border_left_width->value());
  EXPECT_EQ(cssom::kFontSizesAkaEmUnit, border_left_width->unit());
}

TEST_F(ParserTest, ParsesBorderTopWidth) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("border-top-width: .8em;",
                                        source_location_);

  scoped_refptr<cssom::LengthValue> border_top_width =
      dynamic_cast<cssom::LengthValue*>(style->border_top_width().get());
  ASSERT_TRUE(border_top_width);
  EXPECT_FLOAT_EQ(0.8f, border_top_width->value());
  EXPECT_EQ(cssom::kFontSizesAkaEmUnit, border_top_width->unit());
}

TEST_F(ParserTest, ParsesBorderBottomWidth) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("border-bottom-width: 35px;",
                                        source_location_);

  scoped_refptr<cssom::LengthValue> border_bottom_width =
      dynamic_cast<cssom::LengthValue*>(style->border_bottom_width().get());
  ASSERT_TRUE(border_bottom_width);
  EXPECT_FLOAT_EQ(35.0f, border_bottom_width->value());
  EXPECT_EQ(cssom::kPixelsUnit, border_bottom_width->unit());
}

TEST_F(ParserTest, ParsesBorderLeftWidth) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("border-left-width: 20px;",
                                        source_location_);

  scoped_refptr<cssom::LengthValue> border_left_width =
      dynamic_cast<cssom::LengthValue*>(style->border_left_width().get());
  ASSERT_TRUE(border_left_width);
  EXPECT_FLOAT_EQ(20.0f, border_left_width->value());
  EXPECT_EQ(cssom::kPixelsUnit, border_left_width->unit());
}

TEST_F(ParserTest, ParsesBorderRightWidth) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("border-right-width: .4em;",
                                        source_location_);

  scoped_refptr<cssom::LengthValue> border_right_width =
      dynamic_cast<cssom::LengthValue*>(style->border_right_width().get());
  ASSERT_TRUE(border_right_width);
  EXPECT_FLOAT_EQ(0.4f, border_right_width->value());
  EXPECT_EQ(cssom::kFontSizesAkaEmUnit, border_right_width->unit());
}

TEST_F(ParserTest, ParsesSingleBorderStyleSolid) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("border-style: solid;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetSolid(), style->border_left_style());
  EXPECT_EQ(cssom::KeywordValue::GetSolid(), style->border_right_style());
  EXPECT_EQ(cssom::KeywordValue::GetSolid(), style->border_top_style());
  EXPECT_EQ(cssom::KeywordValue::GetSolid(), style->border_bottom_style());
}

TEST_F(ParserTest, ParsesSingleBorderStyleHidden) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("border-style: hidden;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetHidden(), style->border_left_style());
  EXPECT_EQ(cssom::KeywordValue::GetHidden(), style->border_right_style());
  EXPECT_EQ(cssom::KeywordValue::GetHidden(), style->border_top_style());
  EXPECT_EQ(cssom::KeywordValue::GetHidden(), style->border_bottom_style());
}

TEST_F(ParserTest, ParsesBorderTop) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("border-top: 1px solid #777;",
                                        source_location_);

  scoped_refptr<cssom::LengthValue> border_top_width =
      dynamic_cast<cssom::LengthValue*>(style->border_top_width().get());
  ASSERT_TRUE(border_top_width);
  EXPECT_FLOAT_EQ(1.0f, border_top_width->value());
  EXPECT_EQ(cssom::kPixelsUnit, border_top_width->unit());

  EXPECT_EQ(cssom::KeywordValue::GetSolid(), style->border_top_style());

  scoped_refptr<cssom::RGBAColorValue> border_top_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->border_top_color().get());
  ASSERT_TRUE(border_top_color);
  EXPECT_EQ(0x777777FF, border_top_color->value());
}

TEST_F(ParserTest, ParsesBorderBottom) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "border-bottom: 1em solid "
          "rgba(255, 255, 255, .2);",
          source_location_);

  scoped_refptr<cssom::LengthValue> border_bottom_width =
      dynamic_cast<cssom::LengthValue*>(style->border_bottom_width().get());
  ASSERT_TRUE(border_bottom_width);
  EXPECT_FLOAT_EQ(1.0f, border_bottom_width->value());
  EXPECT_EQ(cssom::kFontSizesAkaEmUnit, border_bottom_width->unit());

  EXPECT_EQ(cssom::KeywordValue::GetSolid(), style->border_bottom_style());

  scoped_refptr<cssom::RGBAColorValue> border_bottom_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->border_bottom_color().get());
  ASSERT_TRUE(border_bottom_color);
  EXPECT_EQ(0xFFFFFF33, border_bottom_color->value());
}

TEST_F(ParserTest, ParsesBorderLeft) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("border-left: .7em solid transparent;",
                                        source_location_);

  scoped_refptr<cssom::LengthValue> border_left_width =
      dynamic_cast<cssom::LengthValue*>(style->border_left_width().get());
  ASSERT_TRUE(border_left_width);
  EXPECT_FLOAT_EQ(0.7f, border_left_width->value());
  EXPECT_EQ(cssom::kFontSizesAkaEmUnit, border_left_width->unit());

  EXPECT_EQ(cssom::KeywordValue::GetSolid(), style->border_left_style());

  scoped_refptr<cssom::RGBAColorValue> border_left_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->border_left_color().get());
  ASSERT_TRUE(border_left_color);
  EXPECT_EQ(0x00000000, border_left_color->value());
}

TEST_F(ParserTest, ParsesBorderRight) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "border-right: .1em solid "
          "rgba(255, 255, 255, .2);",
          source_location_);

  scoped_refptr<cssom::LengthValue> border_right_width =
      dynamic_cast<cssom::LengthValue*>(style->border_right_width().get());
  ASSERT_TRUE(border_right_width);
  EXPECT_FLOAT_EQ(0.1f, border_right_width->value());
  EXPECT_EQ(cssom::kFontSizesAkaEmUnit, border_right_width->unit());

  EXPECT_EQ(cssom::KeywordValue::GetSolid(), style->border_right_style());

  scoped_refptr<cssom::RGBAColorValue> border_right_color =
      dynamic_cast<cssom::RGBAColorValue*>(style->border_right_color().get());
  ASSERT_TRUE(border_right_color);
  EXPECT_EQ(0xFFFFFF33, border_right_color->value());
}

TEST_F(ParserTest, ParsesBorderRadiusLength) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("border-radius: 0.2em;",
                                        source_location_);

  scoped_refptr<cssom::LengthValue> border_radius =
      dynamic_cast<cssom::LengthValue*>(style->border_radius().get());
  ASSERT_TRUE(border_radius);
  EXPECT_FLOAT_EQ(0.2f, border_radius->value());
  EXPECT_EQ(cssom::kFontSizesAkaEmUnit, border_radius->unit());
}

TEST_F(ParserTest, ParsesBorderRadiusPercentage) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("border-radius: 50%;",
                                        source_location_);

  scoped_refptr<cssom::PercentageValue> border_radius =
      dynamic_cast<cssom::PercentageValue*>(style->border_radius().get());
  ASSERT_TRUE(border_radius);
  EXPECT_FLOAT_EQ(0.5f, border_radius->value());
}

TEST_F(ParserTest, Parses3DigitColor) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("color: #123;", source_location_);

  scoped_refptr<cssom::RGBAColorValue> color =
      dynamic_cast<cssom::RGBAColorValue*>(style->color().get());
  ASSERT_TRUE(color);
  EXPECT_EQ(0x112233ff, color->value());
}

TEST_F(ParserTest, Parses6DigitColor) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("color: #0047ab;  /* Cobalt blue */\n",
                                        source_location_);

  scoped_refptr<cssom::RGBAColorValue> color =
      dynamic_cast<cssom::RGBAColorValue*>(style->color().get());
  ASSERT_TRUE(color);
  EXPECT_EQ(0x0047abff, color->value());
}

TEST_F(ParserTest, ParsesNoneContent) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("content: none;", source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetNone(), style->content());
}

TEST_F(ParserTest, ParsesNormalContent) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("content: normal;", source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetNormal(), style->content());
}

TEST_F(ParserTest, ParsesURLContent) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("content: url(foo.png);",
                                        source_location_);

  scoped_refptr<cssom::URLValue> content_image =
      dynamic_cast<cssom::URLValue*>(style->content().get());
  ASSERT_TRUE(content_image);
  EXPECT_EQ("foo.png", content_image->value());
}

TEST_F(ParserTest, ParsesStringContent) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("content: \"cobalt FTW!\";",
                                        source_location_);

  scoped_refptr<cssom::StringValue> content =
      dynamic_cast<cssom::StringValue*>(style->content().get());
  ASSERT_TRUE(content);
  EXPECT_EQ("cobalt FTW!", content->value());
}

TEST_F(ParserTest, ParsesRGBColorWithOutOfRangeIntegers) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("color: rgb(300, 0, -300);",
                                        source_location_);

  scoped_refptr<cssom::RGBAColorValue> color =
      dynamic_cast<cssom::RGBAColorValue*>(style->color().get());
  ASSERT_TRUE(color);
  EXPECT_EQ(0xff0000ff, color->value());
}

TEST_F(ParserTest, ParsesRGBAColorWithIntegers) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("color: rgba(255, 128, 1, 0.5);",
                                        source_location_);

  scoped_refptr<cssom::RGBAColorValue> color =
      dynamic_cast<cssom::RGBAColorValue*>(style->color().get());
  ASSERT_TRUE(color);
  EXPECT_EQ(0xff80017f, color->value());
}

TEST_F(ParserTest, ParsesBlockDisplay) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("display: block;", source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetBlock(), style->display());
}

TEST_F(ParserTest, ParsesInlineDisplay) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("display: inline;", source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetInline(), style->display());
}

TEST_F(ParserTest, ParsesInlineBlockDisplay) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("display: inline-block;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetInlineBlock(), style->display());
}

TEST_F(ParserTest, ParsesNoneDisplay) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("display: none;", source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetNone(), style->display());
}

TEST_F(ParserTest, ParsesFontShorthandInherit) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font: inherit;", source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetInherit(), style->font_style());
  EXPECT_EQ(cssom::KeywordValue::GetInherit(), style->font_weight());
  EXPECT_EQ(cssom::KeywordValue::GetInherit(), style->font_size());
  EXPECT_EQ(cssom::KeywordValue::GetInherit(), style->font_family());
}

TEST_F(ParserTest, ParsesFontShorthandInitial) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font: initial;", source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetInitial(), style->font_style());
  EXPECT_EQ(cssom::KeywordValue::GetInitial(), style->font_weight());
  EXPECT_EQ(cssom::KeywordValue::GetInitial(), style->font_size());
  EXPECT_EQ(cssom::KeywordValue::GetInitial(), style->font_family());
}

TEST_F(ParserTest, ParsesFontShorthandSizeFamily) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "font: 50px 'Roboto', Noto, sans-serif;", source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetInitial(), style->font_style());
  EXPECT_EQ(cssom::KeywordValue::GetInitial(), style->font_weight());

  scoped_refptr<cssom::LengthValue> font_size =
      dynamic_cast<cssom::LengthValue*>(style->font_size().get());
  ASSERT_TRUE(font_size);
  EXPECT_FLOAT_EQ(50, font_size->value());
  EXPECT_EQ(cssom::kPixelsUnit, font_size->unit());

  scoped_refptr<cssom::PropertyListValue> font_family_list =
      dynamic_cast<cssom::PropertyListValue*>(style->font_family().get());
  ASSERT_TRUE(font_family_list);
  EXPECT_EQ(font_family_list->value().size(), 3);

  scoped_refptr<cssom::StringValue> font_family_name_1 =
      dynamic_cast<cssom::StringValue*>(
          font_family_list->get_item_modulo_size(0).get());
  ASSERT_TRUE(font_family_name_1);
  EXPECT_EQ("Roboto", font_family_name_1->value());

  scoped_refptr<cssom::StringValue> font_family_name_2 =
      dynamic_cast<cssom::StringValue*>(
          font_family_list->get_item_modulo_size(1).get());
  ASSERT_TRUE(font_family_name_2);
  EXPECT_EQ("Noto", font_family_name_2->value());

  EXPECT_EQ(cssom::KeywordValue::GetSansSerif(),
            font_family_list->get_item_modulo_size(2));
}

TEST_F(ParserTest, ParsesFontShorthandStyleSizeFamily) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font: italic 50em Noto;",
                                        source_location_);

  EXPECT_EQ(cssom::FontStyleValue::GetItalic(), style->font_style());
  EXPECT_EQ(cssom::KeywordValue::GetInitial(), style->font_weight());

  scoped_refptr<cssom::LengthValue> font_size =
      dynamic_cast<cssom::LengthValue*>(style->font_size().get());
  ASSERT_TRUE(font_size);
  EXPECT_FLOAT_EQ(50, font_size->value());
  EXPECT_EQ(cssom::kFontSizesAkaEmUnit, font_size->unit());

  scoped_refptr<cssom::PropertyListValue> font_family_list =
      dynamic_cast<cssom::PropertyListValue*>(style->font_family().get());
  ASSERT_TRUE(font_family_list);
  EXPECT_EQ(font_family_list->value().size(), 1);

  scoped_refptr<cssom::StringValue> font_family_name =
      dynamic_cast<cssom::StringValue*>(
          font_family_list->get_item_modulo_size(0).get());
  ASSERT_TRUE(font_family_name);
  EXPECT_EQ("Noto", font_family_name->value());
}

TEST_F(ParserTest, ParsesFontShorthandWeightSizeFamily) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font: bold 10em sans-serif;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetInitial(), style->font_style());
  EXPECT_EQ(cssom::FontWeightValue::GetBoldAka700(), style->font_weight());

  scoped_refptr<cssom::LengthValue> font_size =
      dynamic_cast<cssom::LengthValue*>(style->font_size().get());
  ASSERT_TRUE(font_size);
  EXPECT_FLOAT_EQ(10, font_size->value());
  EXPECT_EQ(cssom::kFontSizesAkaEmUnit, font_size->unit());

  scoped_refptr<cssom::PropertyListValue> font_family_list =
      dynamic_cast<cssom::PropertyListValue*>(style->font_family().get());
  ASSERT_TRUE(font_family_list);
  EXPECT_EQ(font_family_list->value().size(), 1);

  EXPECT_EQ(cssom::KeywordValue::GetSansSerif(),
            font_family_list->get_item_modulo_size(0));
}

TEST_F(ParserTest, ParsesFontShorthandStyleWeightSizeFamily) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font: italic 300 50px 'Roboto';",
                                        source_location_);

  EXPECT_EQ(cssom::FontStyleValue::GetItalic(), style->font_style());
  EXPECT_EQ(cssom::FontWeightValue::GetLightAka300(), style->font_weight());

  scoped_refptr<cssom::LengthValue> font_size =
      dynamic_cast<cssom::LengthValue*>(style->font_size().get());
  ASSERT_TRUE(font_size);
  EXPECT_FLOAT_EQ(50, font_size->value());
  EXPECT_EQ(cssom::kPixelsUnit, font_size->unit());

  scoped_refptr<cssom::PropertyListValue> font_family_list =
      dynamic_cast<cssom::PropertyListValue*>(style->font_family().get());
  ASSERT_TRUE(font_family_list);
  EXPECT_EQ(font_family_list->value().size(), 1);

  scoped_refptr<cssom::StringValue> font_family_name =
      dynamic_cast<cssom::StringValue*>(
          font_family_list->get_item_modulo_size(0).get());
  ASSERT_TRUE(font_family_name);
  EXPECT_EQ("Roboto", font_family_name->value());
}

TEST_F(ParserTest, ParsesFontShorthandStyleNormal) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font: normal 600 50px Roboto;",
                                        source_location_);

  EXPECT_EQ(cssom::FontStyleValue::GetNormal(), style->font_style());
  EXPECT_EQ(cssom::FontWeightValue::GetSemiBoldAka600(), style->font_weight());

  scoped_refptr<cssom::LengthValue> font_size =
      dynamic_cast<cssom::LengthValue*>(style->font_size().get());
  ASSERT_TRUE(font_size);
  EXPECT_FLOAT_EQ(50, font_size->value());
  EXPECT_EQ(cssom::kPixelsUnit, font_size->unit());

  scoped_refptr<cssom::PropertyListValue> font_family_list =
      dynamic_cast<cssom::PropertyListValue*>(style->font_family().get());
  ASSERT_TRUE(font_family_list);
  EXPECT_EQ(font_family_list->value().size(), 1);

  scoped_refptr<cssom::StringValue> font_family_name =
      dynamic_cast<cssom::StringValue*>(
          font_family_list->get_item_modulo_size(0).get());
  ASSERT_TRUE(font_family_name);
  EXPECT_EQ("Roboto", font_family_name->value());
}

TEST_F(ParserTest, ParsesFontShorthandWeightNormal) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font: italic normal 50px Roboto;",
                                        source_location_);

  EXPECT_EQ(cssom::FontStyleValue::GetItalic(), style->font_style());
  EXPECT_EQ(cssom::FontWeightValue::GetNormalAka400(), style->font_weight());

  scoped_refptr<cssom::LengthValue> font_size =
      dynamic_cast<cssom::LengthValue*>(style->font_size().get());
  ASSERT_TRUE(font_size);
  EXPECT_FLOAT_EQ(50, font_size->value());
  EXPECT_EQ(cssom::kPixelsUnit, font_size->unit());

  scoped_refptr<cssom::PropertyListValue> font_family_list =
      dynamic_cast<cssom::PropertyListValue*>(style->font_family().get());
  ASSERT_TRUE(font_family_list);
  EXPECT_EQ(font_family_list->value().size(), 1);

  scoped_refptr<cssom::StringValue> font_family_name =
      dynamic_cast<cssom::StringValue*>(
          font_family_list->get_item_modulo_size(0).get());
  ASSERT_TRUE(font_family_name);
  EXPECT_EQ("Roboto", font_family_name->value());
}

TEST_F(ParserTest, ParsesFontShorthandStyleWeightNormal) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font: normal normal 50px Roboto;",
                                        source_location_);

  EXPECT_EQ(cssom::FontStyleValue::GetNormal(), style->font_style());
  EXPECT_EQ(cssom::FontWeightValue::GetNormalAka400(), style->font_weight());

  scoped_refptr<cssom::LengthValue> font_size =
      dynamic_cast<cssom::LengthValue*>(style->font_size().get());
  ASSERT_TRUE(font_size);
  EXPECT_FLOAT_EQ(50, font_size->value());
  EXPECT_EQ(cssom::kPixelsUnit, font_size->unit());

  scoped_refptr<cssom::PropertyListValue> font_family_list =
      dynamic_cast<cssom::PropertyListValue*>(style->font_family().get());
  ASSERT_TRUE(font_family_list);
  EXPECT_EQ(font_family_list->value().size(), 1);

  scoped_refptr<cssom::StringValue> font_family_name =
      dynamic_cast<cssom::StringValue*>(
          font_family_list->get_item_modulo_size(0).get());
  ASSERT_TRUE(font_family_name);
  EXPECT_EQ("Roboto", font_family_name->value());
}

TEST_F(ParserTest, ParsesFontFamilyInherit) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font-family: inherit;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetInherit(), style->font_family());
}

TEST_F(ParserTest, ParsesFontFamilyInitial) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font-family: initial;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetInitial(), style->font_family());
}

TEST_F(ParserTest, ParsesFontFamilyGenericName) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font-family: serif;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> font_family_list =
      dynamic_cast<cssom::PropertyListValue*>(style->font_family().get());
  ASSERT_TRUE(font_family_list);
  EXPECT_EQ(font_family_list->value().size(), 1);

  EXPECT_EQ(cssom::KeywordValue::GetSerif(),
            font_family_list->get_item_modulo_size(0));
}

TEST_F(ParserTest, ParsesFontFamilySingleIdentifierName) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font-family: Noto;", source_location_);

  scoped_refptr<cssom::PropertyListValue> font_family_list =
      dynamic_cast<cssom::PropertyListValue*>(style->font_family().get());
  ASSERT_TRUE(font_family_list);
  EXPECT_EQ(font_family_list->value().size(), 1);

  scoped_refptr<cssom::StringValue> font_family_string =
      dynamic_cast<cssom::StringValue*>(
          font_family_list->get_item_modulo_size(0).get());
  ASSERT_TRUE(font_family_string);
  EXPECT_EQ("Noto", font_family_string->value());
}

TEST_F(ParserTest, ParsesFontFamilySingleIdentifierPropertyName) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font-family: content;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> font_family_list =
      dynamic_cast<cssom::PropertyListValue*>(style->font_family().get());
  ASSERT_TRUE(font_family_list);
  EXPECT_EQ(font_family_list->value().size(), 1);

  scoped_refptr<cssom::StringValue> font_family_string =
      dynamic_cast<cssom::StringValue*>(
          font_family_list->get_item_modulo_size(0).get());
  ASSERT_TRUE(font_family_string);
  EXPECT_EQ("content", font_family_string->value());
}

TEST_F(ParserTest, ParsesFontFamilyMultipleIdentifierName) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font-family: Roboto;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> font_family_list =
      dynamic_cast<cssom::PropertyListValue*>(style->font_family().get());
  ASSERT_TRUE(font_family_list);
  EXPECT_EQ(font_family_list->value().size(), 1);

  scoped_refptr<cssom::StringValue> font_family_string =
      dynamic_cast<cssom::StringValue*>(
          font_family_list->get_item_modulo_size(0).get());
  ASSERT_TRUE(font_family_string);
  EXPECT_EQ("Roboto", font_family_string->value());
}

TEST_F(ParserTest, ParsesFontFamilyIdentifierNameContainingInheritKeyword) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font-family: Droid inherit droid;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> font_family_list =
      dynamic_cast<cssom::PropertyListValue*>(style->font_family().get());
  ASSERT_TRUE(font_family_list);
  EXPECT_EQ(font_family_list->value().size(), 1);

  scoped_refptr<cssom::StringValue> font_family_string =
      dynamic_cast<cssom::StringValue*>(
          font_family_list->get_item_modulo_size(0).get());
  ASSERT_TRUE(font_family_string);
  EXPECT_EQ("Droid inherit droid", font_family_string->value());
}

TEST_F(ParserTest, ParsesFontFamilyIdentifierNameContainingGenericFont) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font-family: Droid serif droid;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> font_family_list =
      dynamic_cast<cssom::PropertyListValue*>(style->font_family().get());
  ASSERT_TRUE(font_family_list);
  EXPECT_EQ(font_family_list->value().size(), 1);

  scoped_refptr<cssom::StringValue> font_family_string =
      dynamic_cast<cssom::StringValue*>(
          font_family_list->get_item_modulo_size(0).get());
  ASSERT_TRUE(font_family_string);
  EXPECT_EQ("Droid serif droid", font_family_string->value());
}

TEST_F(ParserTest, ParsesFontFamilyStringName) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font-family: \"Roboto\";",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> font_family_list =
      dynamic_cast<cssom::PropertyListValue*>(style->font_family().get());
  ASSERT_TRUE(font_family_list);
  EXPECT_EQ(font_family_list->value().size(), 1);

  scoped_refptr<cssom::StringValue> font_family_string =
      dynamic_cast<cssom::StringValue*>(
          font_family_list->get_item_modulo_size(0).get());
  ASSERT_TRUE(font_family_string);
  EXPECT_EQ("Roboto", font_family_string->value());
}

TEST_F(ParserTest, ParsesFontFamilyList) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "font-family: 'Roboto', Noto, sans-serif;", source_location_);

  scoped_refptr<cssom::PropertyListValue> font_family_list =
      dynamic_cast<cssom::PropertyListValue*>(style->font_family().get());
  ASSERT_TRUE(font_family_list);
  EXPECT_EQ(font_family_list->value().size(), 3);

  scoped_refptr<cssom::StringValue> font_family_name_1 =
      dynamic_cast<cssom::StringValue*>(
          font_family_list->get_item_modulo_size(0).get());
  ASSERT_TRUE(font_family_name_1);
  EXPECT_EQ("Roboto", font_family_name_1->value());

  scoped_refptr<cssom::StringValue> font_family_name_2 =
      dynamic_cast<cssom::StringValue*>(
          font_family_list->get_item_modulo_size(1).get());
  ASSERT_TRUE(font_family_name_2);
  EXPECT_EQ("Noto", font_family_name_2->value());

  EXPECT_EQ(cssom::KeywordValue::GetSansSerif(),
            font_family_list->get_item_modulo_size(2));
}

// TODO(***REMOVED***): Test other length units, including negative ones.

TEST_F(ParserTest, ParsesFontSize) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font-size: 100px;", source_location_);

  scoped_refptr<cssom::LengthValue> font_size =
      dynamic_cast<cssom::LengthValue*>(style->font_size().get());
  ASSERT_TRUE(font_size);
  EXPECT_FLOAT_EQ(100, font_size->value());
  EXPECT_EQ(cssom::kPixelsUnit, font_size->unit());
}

TEST_F(ParserTest, ParsesNormalFontStyle) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font-style: normal;",
                                        source_location_);

  EXPECT_EQ(cssom::FontStyleValue::GetNormal(), style->font_style());
}

TEST_F(ParserTest, ParsesItalicFontStyle) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font-style: italic;",
                                        source_location_);

  EXPECT_EQ(cssom::FontStyleValue::GetItalic(), style->font_style());
}

TEST_F(ParserTest, ParsesObliqueFontStyle) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font-style: oblique;",
                                        source_location_);

  EXPECT_EQ(cssom::FontStyleValue::GetOblique(), style->font_style());
}

TEST_F(ParserTest, ParsesInheritFontStyle) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font-style: inherit;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetInherit(), style->font_style());
}

TEST_F(ParserTest, ParsesInitialFontStyle) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font-style: initial;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetInitial(), style->font_style());
}

TEST_F(ParserTest, ParsesNormalFontWeight) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font-weight: normal;",
                                        source_location_);

  EXPECT_EQ(cssom::FontWeightValue::GetNormalAka400(), style->font_weight());
}

TEST_F(ParserTest, ParsesBoldFontWeight) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font-weight: bold;", source_location_);

  EXPECT_EQ(cssom::FontWeightValue::GetBoldAka700(), style->font_weight());
}

TEST_F(ParserTest, Parses100FontWeight) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font-weight: 100;", source_location_);

  EXPECT_EQ(cssom::FontWeightValue::GetThinAka100(), style->font_weight());
}

TEST_F(ParserTest, Parses200FontWeight) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font-weight: 200;", source_location_);

  EXPECT_EQ(cssom::FontWeightValue::GetExtraLightAka200(),
            style->font_weight());
}

TEST_F(ParserTest, Parses300FontWeight) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font-weight: 300;", source_location_);

  EXPECT_EQ(cssom::FontWeightValue::GetLightAka300(), style->font_weight());
}

TEST_F(ParserTest, Parses400FontWeight) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font-weight: 400;", source_location_);

  EXPECT_EQ(cssom::FontWeightValue::GetNormalAka400(), style->font_weight());
}

TEST_F(ParserTest, Parses500FontWeight) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font-weight: 500;", source_location_);

  EXPECT_EQ(cssom::FontWeightValue::GetMediumAka500(), style->font_weight());
}

TEST_F(ParserTest, Parses600FontWeight) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font-weight: 600;", source_location_);

  EXPECT_EQ(cssom::FontWeightValue::GetSemiBoldAka600(), style->font_weight());
}

TEST_F(ParserTest, Parses700FontWeight) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font-weight: 700;", source_location_);

  EXPECT_EQ(cssom::FontWeightValue::GetBoldAka700(), style->font_weight());
}

TEST_F(ParserTest, Parses800FontWeight) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font-weight: 800;", source_location_);

  EXPECT_EQ(cssom::FontWeightValue::GetExtraBoldAka800(), style->font_weight());
}

TEST_F(ParserTest, Parses900FontWeight) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("font-weight: 900;", source_location_);

  EXPECT_EQ(cssom::FontWeightValue::GetBlackAka900(), style->font_weight());
}

TEST_F(ParserTest, ParsesHeight) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("height: 100px;", source_location_);

  scoped_refptr<cssom::LengthValue> height =
      dynamic_cast<cssom::LengthValue*>(style->height().get());
  ASSERT_TRUE(height);
  EXPECT_FLOAT_EQ(100, height->value());
  EXPECT_EQ(cssom::kPixelsUnit, height->unit());
}

TEST_F(ParserTest, ParsesNormalLineHeight) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("line-height: normal;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetNormal(), style->line_height());
}

TEST_F(ParserTest, ParsesLineHeightInEm) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("line-height: 1.2em;",
                                        source_location_);

  scoped_refptr<cssom::LengthValue> line_height =
      dynamic_cast<cssom::LengthValue*>(style->line_height().get());
  ASSERT_TRUE(line_height);
  EXPECT_FLOAT_EQ(1.2f, line_height->value());
  EXPECT_EQ(cssom::kFontSizesAkaEmUnit, line_height->unit());
}

TEST_F(ParserTest, ParsesLineHeightPercentage) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("line-height: 220%;", source_location_);

  scoped_refptr<cssom::PercentageValue> line_height =
      dynamic_cast<cssom::PercentageValue*>(style->line_height().get());
  ASSERT_TRUE(line_height);
  EXPECT_FLOAT_EQ(2.2f, line_height->value());
}

TEST_F(ParserTest, ParsesMarginWith1Value) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("margin: auto;", source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetAuto(), style->margin_top());
  EXPECT_EQ(cssom::KeywordValue::GetAuto(), style->margin_right());
  EXPECT_EQ(cssom::KeywordValue::GetAuto(), style->margin_bottom());
  EXPECT_EQ(cssom::KeywordValue::GetAuto(), style->margin_left());
}

TEST_F(ParserTest, ParsesMarginWith2Values) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("margin: auto 0;", source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetAuto(), style->margin_top());

  scoped_refptr<cssom::LengthValue> margin_right =
      dynamic_cast<cssom::LengthValue*>(style->margin_right().get());
  ASSERT_TRUE(margin_right);
  EXPECT_FLOAT_EQ(0, margin_right->value());
  EXPECT_EQ(cssom::kPixelsUnit, margin_right->unit());

  EXPECT_EQ(cssom::KeywordValue::GetAuto(), style->margin_bottom());

  scoped_refptr<cssom::LengthValue> margin_left =
      dynamic_cast<cssom::LengthValue*>(style->margin_left().get());
  ASSERT_TRUE(margin_left);
  EXPECT_FLOAT_EQ(0, margin_left->value());
  EXPECT_EQ(cssom::kPixelsUnit, margin_left->unit());
}

TEST_F(ParserTest, ParsesMarginWith3Values) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("margin: 0 auto 10%;",
                                        source_location_);

  scoped_refptr<cssom::LengthValue> margin_top =
      dynamic_cast<cssom::LengthValue*>(style->margin_top().get());
  ASSERT_TRUE(margin_top);
  EXPECT_FLOAT_EQ(0, margin_top->value());
  EXPECT_EQ(cssom::kPixelsUnit, margin_top->unit());

  EXPECT_EQ(cssom::KeywordValue::GetAuto(), style->margin_right());

  scoped_refptr<cssom::PercentageValue> margin_bottom =
      dynamic_cast<cssom::PercentageValue*>(style->margin_bottom().get());
  ASSERT_TRUE(margin_bottom);
  EXPECT_FLOAT_EQ(0.1f, margin_bottom->value());

  EXPECT_EQ(cssom::KeywordValue::GetAuto(), style->margin_left());
}

TEST_F(ParserTest, ParsesMarginWith4Values) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("margin: 10px 20px 30px 40px;",
                                        source_location_);

  scoped_refptr<cssom::LengthValue> margin_top =
      dynamic_cast<cssom::LengthValue*>(style->margin_top().get());
  ASSERT_TRUE(margin_top);
  EXPECT_FLOAT_EQ(10, margin_top->value());
  EXPECT_EQ(cssom::kPixelsUnit, margin_top->unit());

  scoped_refptr<cssom::LengthValue> margin_right =
      dynamic_cast<cssom::LengthValue*>(style->margin_right().get());
  ASSERT_TRUE(margin_right);
  EXPECT_FLOAT_EQ(20, margin_right->value());
  EXPECT_EQ(cssom::kPixelsUnit, margin_right->unit());

  scoped_refptr<cssom::LengthValue> margin_bottom =
      dynamic_cast<cssom::LengthValue*>(style->margin_bottom().get());
  ASSERT_TRUE(margin_bottom);
  EXPECT_FLOAT_EQ(30, margin_bottom->value());
  EXPECT_EQ(cssom::kPixelsUnit, margin_bottom->unit());

  scoped_refptr<cssom::LengthValue> margin_left =
      dynamic_cast<cssom::LengthValue*>(style->margin_left().get());
  ASSERT_TRUE(margin_left);
  EXPECT_FLOAT_EQ(40, margin_left->value());
  EXPECT_EQ(cssom::kPixelsUnit, margin_left->unit());
}

TEST_F(ParserTest, ParsesMarginBottom) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("margin-bottom: 1em;",
                                        source_location_);

  scoped_refptr<cssom::LengthValue> margin_bottom =
      dynamic_cast<cssom::LengthValue*>(style->margin_bottom().get());
  ASSERT_TRUE(margin_bottom);
  EXPECT_FLOAT_EQ(1, margin_bottom->value());
  EXPECT_EQ(cssom::kFontSizesAkaEmUnit, margin_bottom->unit());
}

TEST_F(ParserTest, ParsesMarginLeft) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("margin-left: 5.5%;", source_location_);

  scoped_refptr<cssom::PercentageValue> margin_left =
      dynamic_cast<cssom::PercentageValue*>(style->margin_left().get());
  ASSERT_TRUE(margin_left);
  EXPECT_FLOAT_EQ(0.055f, margin_left->value());
}

TEST_F(ParserTest, ParsesMarginRight) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("margin-right: auto;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetAuto(), style->margin_right());
}

TEST_F(ParserTest, ParsesMarginTop) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("margin-top: initial;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetInitial(), style->margin_top());
}

TEST_F(ParserTest, ParsesMaxHeight) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("max-height: 100px;", source_location_);

  scoped_refptr<cssom::LengthValue> max_height =
      dynamic_cast<cssom::LengthValue*>(style->max_height().get());
  ASSERT_TRUE(max_height);
  EXPECT_FLOAT_EQ(100, max_height->value());
  EXPECT_EQ(cssom::kPixelsUnit, max_height->unit());
}

TEST_F(ParserTest, ParsesMaxHeightNone) {
  // 'none' is also the initial value for max-height. It is set to a length
  // value first, to ensure that the property does not have the initial value
  // for the test.
  //   http://www.w3.org/TR/CSS2/visudet.html#propdef-max-height
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("max-height: 100px; max-height: none;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetNone(), style->max_height());
}

TEST_F(ParserTest, ParsesMaxWidth) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("max-width: 100px;", source_location_);

  scoped_refptr<cssom::LengthValue> max_width =
      dynamic_cast<cssom::LengthValue*>(style->max_width().get());
  ASSERT_TRUE(max_width);
  EXPECT_FLOAT_EQ(100, max_width->value());
  EXPECT_EQ(cssom::kPixelsUnit, max_width->unit());
}

TEST_F(ParserTest, ParsesMaxWidthNone) {
  // 'none' is also the initial value for max-width. It is set to a length value
  // first, to ensure that the property does not have the initial value for the
  // test.
  //   http://www.w3.org/TR/CSS2/visudet.html#propdef-max-width
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("max-width: 100px; max-width: none;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetNone(), style->max_width());
}

TEST_F(ParserTest, ParsesMinHeight) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("min-height: 100px;", source_location_);

  scoped_refptr<cssom::LengthValue> min_height =
      dynamic_cast<cssom::LengthValue*>(style->min_height().get());
  ASSERT_TRUE(min_height);
  EXPECT_FLOAT_EQ(100, min_height->value());
  EXPECT_EQ(cssom::kPixelsUnit, min_height->unit());
}

TEST_F(ParserTest, ParsesMinWidth) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("min-width: 100px;", source_location_);

  scoped_refptr<cssom::LengthValue> min_width =
      dynamic_cast<cssom::LengthValue*>(style->min_width().get());
  ASSERT_TRUE(min_width);
  EXPECT_FLOAT_EQ(100, min_width->value());
  EXPECT_EQ(cssom::kPixelsUnit, min_width->unit());
}

TEST_F(ParserTest, ParsesOpacity) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("opacity: 0.5;", source_location_);

  scoped_refptr<cssom::NumberValue> translucent =
      dynamic_cast<cssom::NumberValue*>(style->opacity().get());
  ASSERT_TRUE(translucent);
  EXPECT_FLOAT_EQ(0.5f, translucent->value());
}

TEST_F(ParserTest, ClampsOpacityToZero) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("opacity: -3.14;", source_location_);

  scoped_refptr<cssom::NumberValue> transparent =
      dynamic_cast<cssom::NumberValue*>(style->opacity().get());
  ASSERT_TRUE(transparent);
  EXPECT_FLOAT_EQ(0, transparent->value());
}

TEST_F(ParserTest, ClampsOpacityToOne) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("opacity: 2.72;", source_location_);

  scoped_refptr<cssom::NumberValue> opaque =
      dynamic_cast<cssom::NumberValue*>(style->opacity().get());
  ASSERT_TRUE(opaque);
  EXPECT_FLOAT_EQ(1, opaque->value());
}

TEST_F(ParserTest, ParsesBreakWordOverflowWrap) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("overflow-wrap: break-word;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetBreakWord(), style->overflow_wrap());
}

TEST_F(ParserTest, ParsesNormalOverflowWrap) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("overflow-wrap: normal;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetNormal(), style->overflow_wrap());
}

TEST_F(ParserTest, ParsesHiddenOverflow) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("overflow: hidden;", source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetHidden(), style->overflow());
}

TEST_F(ParserTest, ParsesVisibleOverflow) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("overflow: visible;", source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetVisible(), style->overflow());
}

TEST_F(ParserTest, ParsesPaddingWith1Value) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("padding: inherit;", source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetInherit(), style->padding_top());
  EXPECT_EQ(cssom::KeywordValue::GetInherit(), style->padding_right());
  EXPECT_EQ(cssom::KeywordValue::GetInherit(), style->padding_bottom());
  EXPECT_EQ(cssom::KeywordValue::GetInherit(), style->padding_left());
}

TEST_F(ParserTest, ParsesPaddingWith2Values) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("padding: 10% 1.2em;",
                                        source_location_);

  scoped_refptr<cssom::PercentageValue> padding_top =
      dynamic_cast<cssom::PercentageValue*>(style->padding_top().get());
  ASSERT_TRUE(padding_top);
  EXPECT_FLOAT_EQ(0.1f, padding_top->value());

  scoped_refptr<cssom::LengthValue> padding_right =
      dynamic_cast<cssom::LengthValue*>(style->padding_right().get());
  ASSERT_TRUE(padding_right);
  EXPECT_FLOAT_EQ(1.2f, padding_right->value());
  EXPECT_EQ(cssom::kFontSizesAkaEmUnit, padding_right->unit());

  scoped_refptr<cssom::PercentageValue> padding_bottom =
      dynamic_cast<cssom::PercentageValue*>(style->padding_bottom().get());
  ASSERT_TRUE(padding_bottom);
  EXPECT_FLOAT_EQ(0.1f, padding_bottom->value());

  scoped_refptr<cssom::LengthValue> padding_left =
      dynamic_cast<cssom::LengthValue*>(style->padding_left().get());
  ASSERT_TRUE(padding_left);
  EXPECT_FLOAT_EQ(1.2f, padding_left->value());
  EXPECT_EQ(cssom::kFontSizesAkaEmUnit, padding_left->unit());
}

TEST_F(ParserTest, ParsesPaddingWith3Values) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("padding: 10% 1.2em 20%;",
                                        source_location_);

  scoped_refptr<cssom::PercentageValue> padding_top =
      dynamic_cast<cssom::PercentageValue*>(style->padding_top().get());
  ASSERT_TRUE(padding_top);
  EXPECT_FLOAT_EQ(0.1f, padding_top->value());

  scoped_refptr<cssom::LengthValue> padding_right =
      dynamic_cast<cssom::LengthValue*>(style->padding_right().get());
  ASSERT_TRUE(padding_right);
  EXPECT_FLOAT_EQ(1.2f, padding_right->value());
  EXPECT_EQ(cssom::kFontSizesAkaEmUnit, padding_right->unit());

  scoped_refptr<cssom::PercentageValue> padding_bottom =
      dynamic_cast<cssom::PercentageValue*>(style->padding_bottom().get());
  ASSERT_TRUE(padding_bottom);
  EXPECT_FLOAT_EQ(0.2f, padding_bottom->value());

  scoped_refptr<cssom::LengthValue> padding_left =
      dynamic_cast<cssom::LengthValue*>(style->padding_left().get());
  ASSERT_TRUE(padding_left);
  EXPECT_FLOAT_EQ(1.2f, padding_left->value());
  EXPECT_EQ(cssom::kFontSizesAkaEmUnit, padding_left->unit());
}

TEST_F(ParserTest, ParsesPaddingWith4Values) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("padding: 10px 20px 30px 40px;",
                                        source_location_);

  scoped_refptr<cssom::LengthValue> padding_top =
      dynamic_cast<cssom::LengthValue*>(style->padding_top().get());
  ASSERT_TRUE(padding_top);
  EXPECT_FLOAT_EQ(10, padding_top->value());
  EXPECT_EQ(cssom::kPixelsUnit, padding_top->unit());

  scoped_refptr<cssom::LengthValue> padding_right =
      dynamic_cast<cssom::LengthValue*>(style->padding_right().get());
  ASSERT_TRUE(padding_right);
  EXPECT_FLOAT_EQ(20, padding_right->value());
  EXPECT_EQ(cssom::kPixelsUnit, padding_right->unit());

  scoped_refptr<cssom::LengthValue> padding_bottom =
      dynamic_cast<cssom::LengthValue*>(style->padding_bottom().get());
  ASSERT_TRUE(padding_bottom);
  EXPECT_FLOAT_EQ(30, padding_bottom->value());
  EXPECT_EQ(cssom::kPixelsUnit, padding_bottom->unit());

  scoped_refptr<cssom::LengthValue> padding_left =
      dynamic_cast<cssom::LengthValue*>(style->padding_left().get());
  ASSERT_TRUE(padding_left);
  EXPECT_FLOAT_EQ(40, padding_left->value());
  EXPECT_EQ(cssom::kPixelsUnit, padding_left->unit());
}

TEST_F(ParserTest, ParsesPaddingBottom) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("padding-bottom: 1em;",
                                        source_location_);

  scoped_refptr<cssom::LengthValue> padding_bottom =
      dynamic_cast<cssom::LengthValue*>(style->padding_bottom().get());
  ASSERT_TRUE(padding_bottom);
  EXPECT_FLOAT_EQ(1, padding_bottom->value());
  EXPECT_EQ(cssom::kFontSizesAkaEmUnit, padding_bottom->unit());
}

TEST_F(ParserTest, ParsesPaddingLeft) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("padding-left: 5.5%;",
                                        source_location_);

  scoped_refptr<cssom::PercentageValue> padding_left =
      dynamic_cast<cssom::PercentageValue*>(style->padding_left().get());
  ASSERT_TRUE(padding_left);
  EXPECT_FLOAT_EQ(0.055f, padding_left->value());
}

TEST_F(ParserTest, ParsesPaddingRight) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("padding-right: inherit;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetInherit(), style->padding_right());
}

TEST_F(ParserTest, ParsesPaddingTop) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("padding-top: initial;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetInitial(), style->padding_top());
}

TEST_F(ParserTest, ParsesStaticPosition) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("position: static;", source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetStatic(), style->position());
}

TEST_F(ParserTest, ParsesRelativePosition) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("position: relative;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetRelative(), style->position());
}

TEST_F(ParserTest, ParsesAbsolutePosition) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("position: absolute;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetAbsolute(), style->position());
}

TEST_F(ParserTest, ParsesNoneTransform) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("transform: none;", source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetNone(), style->transform());
}

TEST_F(ParserTest, ParsesRotateTransformInDegrees) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("transform: rotate(180deg);",
                                        source_location_);

  scoped_refptr<cssom::TransformFunctionListValue> transform_list =
      dynamic_cast<cssom::TransformFunctionListValue*>(
          style->transform().get());
  ASSERT_TRUE(transform_list);
  ASSERT_EQ(1, transform_list->value().size());

  const cssom::RotateFunction* rotate_function =
      dynamic_cast<const cssom::RotateFunction*>(transform_list->value()[0]);
  ASSERT_TRUE(rotate_function);
  EXPECT_FLOAT_EQ(static_cast<float>(M_PI),
                  rotate_function->clockwise_angle_in_radians());
}

TEST_F(ParserTest, ParsesRotateTransformInGradians) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("transform: rotate(200grad);",
                                        source_location_);

  scoped_refptr<cssom::TransformFunctionListValue> transform_list =
      dynamic_cast<cssom::TransformFunctionListValue*>(
          style->transform().get());
  ASSERT_TRUE(transform_list);
  ASSERT_EQ(1, transform_list->value().size());

  const cssom::RotateFunction* rotate_function =
      dynamic_cast<const cssom::RotateFunction*>(transform_list->value()[0]);
  ASSERT_TRUE(rotate_function);
  EXPECT_FLOAT_EQ(static_cast<float>(M_PI),
                  rotate_function->clockwise_angle_in_radians());
}

TEST_F(ParserTest, ParsesRotateTransformInRadians) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "transform: rotate(3.141592653589793rad);", source_location_);

  scoped_refptr<cssom::TransformFunctionListValue> transform_list =
      dynamic_cast<cssom::TransformFunctionListValue*>(
          style->transform().get());
  ASSERT_TRUE(transform_list);
  ASSERT_EQ(1, transform_list->value().size());

  const cssom::RotateFunction* rotate_function =
      dynamic_cast<const cssom::RotateFunction*>(transform_list->value()[0]);
  ASSERT_TRUE(rotate_function);
  EXPECT_FLOAT_EQ(static_cast<float>(M_PI),
                  rotate_function->clockwise_angle_in_radians());
}

TEST_F(ParserTest, ParsesRotateTransformInTurns) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("transform: rotate(0.5turn);",
                                        source_location_);

  scoped_refptr<cssom::TransformFunctionListValue> transform_list =
      dynamic_cast<cssom::TransformFunctionListValue*>(
          style->transform().get());
  ASSERT_TRUE(transform_list);
  ASSERT_EQ(1, transform_list->value().size());

  const cssom::RotateFunction* rotate_function =
      dynamic_cast<const cssom::RotateFunction*>(transform_list->value()[0]);
  ASSERT_TRUE(rotate_function);
  EXPECT_FLOAT_EQ(static_cast<float>(M_PI),
                  rotate_function->clockwise_angle_in_radians());
}

TEST_F(ParserTest, ParsesIsotropicScaleTransform) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("transform: scale(1.5);",
                                        source_location_);

  scoped_refptr<cssom::TransformFunctionListValue> transform_list =
      dynamic_cast<cssom::TransformFunctionListValue*>(
          style->transform().get());
  ASSERT_TRUE(transform_list);
  ASSERT_EQ(1, transform_list->value().size());

  const cssom::ScaleFunction* scale_function =
      dynamic_cast<const cssom::ScaleFunction*>(transform_list->value()[0]);
  ASSERT_TRUE(scale_function);
  EXPECT_FLOAT_EQ(1.5, scale_function->x_factor());
  EXPECT_FLOAT_EQ(1.5, scale_function->y_factor());
}

TEST_F(ParserTest, ParsesIsotropicScaleXTransform) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("transform: scaleX(0.5);",
                                        source_location_);

  scoped_refptr<cssom::TransformFunctionListValue> transform_list =
      dynamic_cast<cssom::TransformFunctionListValue*>(
          style->transform().get());
  ASSERT_TRUE(transform_list);
  ASSERT_EQ(1, transform_list->value().size());

  const cssom::ScaleFunction* scale_function =
      dynamic_cast<const cssom::ScaleFunction*>(transform_list->value()[0]);
  ASSERT_TRUE(scale_function);
  EXPECT_FLOAT_EQ(0.5, scale_function->x_factor());
  EXPECT_FLOAT_EQ(1.0, scale_function->y_factor());
}

TEST_F(ParserTest, ParsesIsotropicScaleYTransform) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("transform: scaleY(20);",
                                        source_location_);

  scoped_refptr<cssom::TransformFunctionListValue> transform_list =
      dynamic_cast<cssom::TransformFunctionListValue*>(
          style->transform().get());
  ASSERT_TRUE(transform_list);
  ASSERT_EQ(1, transform_list->value().size());

  const cssom::ScaleFunction* scale_function =
      dynamic_cast<const cssom::ScaleFunction*>(transform_list->value()[0]);
  ASSERT_TRUE(scale_function);
  EXPECT_FLOAT_EQ(1.0, scale_function->x_factor());
  EXPECT_FLOAT_EQ(20.0, scale_function->y_factor());
}

// TODO(***REMOVED***): Test integers, including negative ones.
// TODO(***REMOVED***): Test reals, including negative ones.
// TODO(***REMOVED***): Test non-zero lengths without units.

TEST_F(ParserTest, ParsesAnisotropicScaleTransform) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("transform: scale(16, 9);",
                                        source_location_);

  scoped_refptr<cssom::TransformFunctionListValue> transform_list =
      dynamic_cast<cssom::TransformFunctionListValue*>(
          style->transform().get());
  ASSERT_TRUE(transform_list);
  ASSERT_EQ(1, transform_list->value().size());

  const cssom::ScaleFunction* scale_function =
      dynamic_cast<const cssom::ScaleFunction*>(transform_list->value()[0]);
  ASSERT_TRUE(scale_function);
  EXPECT_FLOAT_EQ(16, scale_function->x_factor());
  EXPECT_FLOAT_EQ(9, scale_function->y_factor());
}

TEST_F(ParserTest, Parses2DTranslateTransform) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("transform: translate(-50%, 30px);",
                                        source_location_);

  scoped_refptr<cssom::TransformFunctionListValue> transform_list =
      dynamic_cast<cssom::TransformFunctionListValue*>(
          style->transform().get());
  ASSERT_TRUE(transform_list);
  ASSERT_EQ(2, transform_list->value().size());

  const cssom::TranslateFunction* translate_function_1 =
      dynamic_cast<const cssom::TranslateFunction*>(transform_list->value()[0]);
  ASSERT_TRUE(translate_function_1);

  ASSERT_EQ(cssom::TranslateFunction::kPercentage,
            translate_function_1->offset_type());
  scoped_refptr<cssom::PercentageValue> offset_1 =
      translate_function_1->offset_as_percentage();
  EXPECT_FLOAT_EQ(-0.5f, offset_1->value());
  EXPECT_EQ(cssom::TranslateFunction::kXAxis, translate_function_1->axis());

  const cssom::TranslateFunction* translate_function_2 =
      dynamic_cast<const cssom::TranslateFunction*>(transform_list->value()[1]);
  ASSERT_TRUE(translate_function_2);

  ASSERT_EQ(cssom::TranslateFunction::kLength,
            translate_function_2->offset_type());
  scoped_refptr<cssom::LengthValue> offset_2 =
      translate_function_2->offset_as_length();
  EXPECT_FLOAT_EQ(30, offset_2->value());
  EXPECT_EQ(cssom::kPixelsUnit, offset_2->unit());
  EXPECT_EQ(cssom::TranslateFunction::kYAxis, translate_function_2->axis());
}

TEST_F(ParserTest, Parses2DTranslateTransformYOmitted) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("transform: translate(20%);",
                                        source_location_);

  scoped_refptr<cssom::TransformFunctionListValue> transform_list =
      dynamic_cast<cssom::TransformFunctionListValue*>(
          style->transform().get());
  ASSERT_TRUE(transform_list);
  ASSERT_EQ(2, transform_list->value().size());

  const cssom::TranslateFunction* translate_function_1 =
      dynamic_cast<const cssom::TranslateFunction*>(transform_list->value()[0]);
  ASSERT_TRUE(translate_function_1);

  ASSERT_EQ(cssom::TranslateFunction::kPercentage,
            translate_function_1->offset_type());
  scoped_refptr<cssom::PercentageValue> offset_1 =
      translate_function_1->offset_as_percentage();
  EXPECT_FLOAT_EQ(0.2f, offset_1->value());
  EXPECT_EQ(cssom::TranslateFunction::kXAxis, translate_function_1->axis());

  const cssom::TranslateFunction* translate_function_2 =
      dynamic_cast<const cssom::TranslateFunction*>(transform_list->value()[1]);
  ASSERT_TRUE(translate_function_2);

  ASSERT_EQ(cssom::TranslateFunction::kLength,
            translate_function_2->offset_type());
  scoped_refptr<cssom::LengthValue> offset_2 =
      translate_function_2->offset_as_length();
  EXPECT_FLOAT_EQ(0, offset_2->value());
  EXPECT_EQ(cssom::kPixelsUnit, offset_2->unit());
  EXPECT_EQ(cssom::TranslateFunction::kYAxis, translate_function_2->axis());
}

TEST_F(ParserTest, ParsesTranslateXTransformLength) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("transform: translateX(0);",
                                        source_location_);

  scoped_refptr<cssom::TransformFunctionListValue> transform_list =
      dynamic_cast<cssom::TransformFunctionListValue*>(
          style->transform().get());
  ASSERT_TRUE(transform_list);
  ASSERT_EQ(1, transform_list->value().size());

  const cssom::TranslateFunction* translate_function =
      dynamic_cast<const cssom::TranslateFunction*>(transform_list->value()[0]);
  ASSERT_TRUE(translate_function);

  ASSERT_EQ(cssom::TranslateFunction::kLength,
            translate_function->offset_type());
  scoped_refptr<cssom::LengthValue> offset =
      translate_function->offset_as_length();
  EXPECT_FLOAT_EQ(0, offset->value());
  EXPECT_EQ(cssom::kPixelsUnit, offset->unit());
  EXPECT_EQ(cssom::TranslateFunction::kXAxis, translate_function->axis());
}

TEST_F(ParserTest, ParsesTranslateXTransformPercentage) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("transform: translateX(20%);",
                                        source_location_);

  scoped_refptr<cssom::TransformFunctionListValue> transform_list =
      dynamic_cast<cssom::TransformFunctionListValue*>(
          style->transform().get());
  ASSERT_TRUE(transform_list);
  ASSERT_EQ(1, transform_list->value().size());

  const cssom::TranslateFunction* translate_function =
      dynamic_cast<const cssom::TranslateFunction*>(transform_list->value()[0]);
  ASSERT_TRUE(translate_function);

  ASSERT_EQ(cssom::TranslateFunction::kPercentage,
            translate_function->offset_type());
  scoped_refptr<cssom::PercentageValue> offset =
      translate_function->offset_as_percentage();
  EXPECT_FLOAT_EQ(0.2f, offset->value());
  EXPECT_EQ(cssom::TranslateFunction::kXAxis, translate_function->axis());
}

TEST_F(ParserTest, ParsesTranslateYTransformLength) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("transform: translateY(30em);",
                                        source_location_);

  scoped_refptr<cssom::TransformFunctionListValue> transform_list =
      dynamic_cast<cssom::TransformFunctionListValue*>(
          style->transform().get());
  ASSERT_TRUE(transform_list);
  ASSERT_EQ(1, transform_list->value().size());

  const cssom::TranslateFunction* translate_function =
      dynamic_cast<const cssom::TranslateFunction*>(transform_list->value()[0]);
  ASSERT_TRUE(translate_function);

  ASSERT_EQ(cssom::TranslateFunction::kLength,
            translate_function->offset_type());
  scoped_refptr<cssom::LengthValue> offset =
      translate_function->offset_as_length();
  EXPECT_FLOAT_EQ(30, offset->value());
  EXPECT_EQ(cssom::kFontSizesAkaEmUnit, offset->unit());
  EXPECT_EQ(cssom::TranslateFunction::kYAxis, translate_function->axis());
}

TEST_F(ParserTest, ParsesTranslateYTransformPercentage) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("transform: translateY(30%);",
                                        source_location_);

  scoped_refptr<cssom::TransformFunctionListValue> transform_list =
      dynamic_cast<cssom::TransformFunctionListValue*>(
          style->transform().get());
  ASSERT_TRUE(transform_list);
  ASSERT_EQ(1, transform_list->value().size());

  const cssom::TranslateFunction* translate_function =
      dynamic_cast<const cssom::TranslateFunction*>(transform_list->value()[0]);
  ASSERT_TRUE(translate_function);

  ASSERT_EQ(cssom::TranslateFunction::kPercentage,
            translate_function->offset_type());
  scoped_refptr<cssom::PercentageValue> offset =
      translate_function->offset_as_percentage();
  EXPECT_FLOAT_EQ(0.3f, offset->value());
  EXPECT_EQ(cssom::TranslateFunction::kYAxis, translate_function->axis());
}

TEST_F(ParserTest, ParsesTranslateZTransformLength) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("transform: translateZ(-22px);",
                                        source_location_);

  scoped_refptr<cssom::TransformFunctionListValue> transform_list =
      dynamic_cast<cssom::TransformFunctionListValue*>(
          style->transform().get());
  ASSERT_TRUE(transform_list);
  ASSERT_EQ(1, transform_list->value().size());

  const cssom::TranslateFunction* translate_function =
      dynamic_cast<const cssom::TranslateFunction*>(transform_list->value()[0]);
  ASSERT_TRUE(translate_function);

  ASSERT_EQ(cssom::TranslateFunction::kLength,
            translate_function->offset_type());
  scoped_refptr<cssom::LengthValue> offset =
      translate_function->offset_as_length();
  EXPECT_FLOAT_EQ(-22, offset->value());
  EXPECT_EQ(cssom::kPixelsUnit, offset->unit());
  EXPECT_EQ(cssom::TranslateFunction::kZAxis, translate_function->axis());
}

TEST_F(ParserTest, ParsesMatrixTransform) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("transform: matrix(1, 2, 3, 4, 5, 6);",
                                        source_location_);

  scoped_refptr<cssom::TransformFunctionListValue> transform_list =
      dynamic_cast<cssom::TransformFunctionListValue*>(
          style->transform().get());
  ASSERT_TRUE(transform_list);
  ASSERT_EQ(1, transform_list->value().size());

  const cssom::MatrixFunction* matrix_function =
      dynamic_cast<const cssom::MatrixFunction*>(transform_list->value()[0]);
  ASSERT_TRUE(matrix_function);

  EXPECT_FLOAT_EQ(1.0f, matrix_function->value().Get(0, 0));
  EXPECT_FLOAT_EQ(3.0f, matrix_function->value().Get(0, 1));
  EXPECT_FLOAT_EQ(5.0f, matrix_function->value().Get(0, 2));
  EXPECT_FLOAT_EQ(2.0f, matrix_function->value().Get(1, 0));
  EXPECT_FLOAT_EQ(4.0f, matrix_function->value().Get(1, 1));
  EXPECT_FLOAT_EQ(6.0f, matrix_function->value().Get(1, 2));
  EXPECT_FLOAT_EQ(0.0f, matrix_function->value().Get(2, 0));
  EXPECT_FLOAT_EQ(0.0f, matrix_function->value().Get(2, 1));
  EXPECT_FLOAT_EQ(1.0f, matrix_function->value().Get(2, 2));
}

TEST_F(ParserTest, ParsesMultipleTransforms) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("transform: scale(2) translateZ(10px);",
                                        source_location_);

  scoped_refptr<cssom::TransformFunctionListValue> transform_list =
      dynamic_cast<cssom::TransformFunctionListValue*>(
          style->transform().get());
  ASSERT_TRUE(transform_list);
  ASSERT_EQ(2, transform_list->value().size());
  EXPECT_TRUE(transform_list->value()[0]);
  EXPECT_EQ(base::GetTypeId<cssom::ScaleFunction>(),
            transform_list->value()[0]->GetTypeId());
  EXPECT_TRUE(transform_list->value()[1]);
  EXPECT_EQ(base::GetTypeId<cssom::TranslateFunction>(),
            transform_list->value()[1]->GetTypeId());
}

TEST_F(ParserTest, RecoversFromInvalidTransformList) {
  EXPECT_CALL(
      parser_observer_,
      OnWarning(
          "[object ParserTest]:1:25: warning: invalid transform function"));

  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "transform: scale(0.001) warp(spacetime);\n"
          "color: #000;\n",
          source_location_);

  EXPECT_TRUE(style->color());
}

TEST_F(ParserTest, ParsesIntegerTabSize) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("tab-size: 4;", source_location_);

  scoped_refptr<cssom::IntegerValue> tab_size =
      dynamic_cast<cssom::IntegerValue*>(style->tab_size().get());
  ASSERT_TRUE(tab_size);
  EXPECT_EQ(4, tab_size->value());
}

TEST_F(ParserTest, ParsesLeftTextAlign) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("text-align: left;", source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetLeft(), style->text_align());
}

TEST_F(ParserTest, ParsesCenterTextAlign) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("text-align: center;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetCenter(), style->text_align());
}

TEST_F(ParserTest, ParsesRightTextAlign) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("text-align: right;", source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetRight(), style->text_align());
}

TEST_F(ParserTest, ParseZeroLengthTextIndent) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("text-indent: 0;", source_location_);

  scoped_refptr<cssom::LengthValue> text_indent =
      dynamic_cast<cssom::LengthValue*>(style->text_indent().get());
  ASSERT_TRUE(text_indent);
  EXPECT_FLOAT_EQ(0, text_indent->value());
  EXPECT_EQ(cssom::kPixelsUnit, text_indent->unit());
}

TEST_F(ParserTest, ParseRelativeLengthTextIndent) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("text-indent: 4em", source_location_);

  scoped_refptr<cssom::LengthValue> text_indent =
      dynamic_cast<cssom::LengthValue*>(style->text_indent().get());
  ASSERT_TRUE(text_indent);
  EXPECT_FLOAT_EQ(4, text_indent->value());
  EXPECT_EQ(cssom::kFontSizesAkaEmUnit, text_indent->unit());
}

TEST_F(ParserTest, ParseAbsoluteLengthTextIndent) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("text-indent: 100px;",
                                        source_location_);

  scoped_refptr<cssom::LengthValue> text_indent =
      dynamic_cast<cssom::LengthValue*>(style->text_indent().get());
  ASSERT_TRUE(text_indent);
  EXPECT_FLOAT_EQ(100, text_indent->value());
  EXPECT_EQ(cssom::kPixelsUnit, text_indent->unit());
}

TEST_F(ParserTest, ParsesClipTextOverflow) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("text-overflow: clip;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetClip(), style->text_overflow());
}

TEST_F(ParserTest, ParsesEllipsisTextOverflow) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("text-overflow: ellipsis;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetEllipsis(), style->text_overflow());
}

TEST_F(ParserTest, ParsesNoneTextTransform) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("text-transform: none;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetNone(), style->text_transform());
}

TEST_F(ParserTest, ParsesUppercaseTextTransform) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("text-transform: uppercase;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetUppercase(), style->text_transform());
}

TEST_F(ParserTest, ParsesBaselineVerticalAlign) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("vertical-align: baseline;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetBaseline(), style->vertical_align());
}

TEST_F(ParserTest, ParsesBottomVerticalAlign) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("vertical-align: bottom;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetBottom(), style->vertical_align());
}

TEST_F(ParserTest, ParsesMiddleVerticalAlign) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("vertical-align: middle;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetMiddle(), style->vertical_align());
}

TEST_F(ParserTest, ParsesTopVerticalAlign) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("vertical-align: top;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetTop(), style->vertical_align());
}

TEST_F(ParserTest, ParsesHiddenVisibility) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("visibility: hidden;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetHidden(), style->visibility());
}

TEST_F(ParserTest, ParsesVisibleVisibility) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("visibility: visible;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetVisible(), style->visibility());
}

TEST_F(ParserTest, ParsesNormalWhiteSpace) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("white-space: normal;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetNormal(), style->white_space());
}

TEST_F(ParserTest, ParsesNoWrapWhiteSpace) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("white-space: nowrap;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetNoWrap(), style->white_space());
}

TEST_F(ParserTest, ParsesPreWhiteSpace) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("white-space: pre;", source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetPre(), style->white_space());
}

TEST_F(ParserTest, ParsesInheritWhiteSpace) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("white-space: inherit;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetInherit(), style->white_space());
}

TEST_F(ParserTest, ParsesInitialWhiteSpace) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("white-space: initial;",
                                        source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetInitial(), style->white_space());
}

TEST_F(ParserTest, ParsesWidth) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("width: 100px;", source_location_);

  scoped_refptr<cssom::LengthValue> width =
      dynamic_cast<cssom::LengthValue*>(style->width().get());
  ASSERT_TRUE(width);
  EXPECT_FLOAT_EQ(100, width->value());
  EXPECT_EQ(cssom::kPixelsUnit, width->unit());
}

TEST_F(ParserTest, ParsesBreakWordWordWrap) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("word-wrap: break-word;",
                                        source_location_);

  // word-wrap is treated as an alias for overflow-wrap
  //   http://www.w3.org/TR/css-text-3/#overflow-wrap
  EXPECT_EQ(cssom::KeywordValue::GetBreakWord(), style->overflow_wrap());
}

TEST_F(ParserTest, ParsesNormalWordWrap) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("word-wrap: normal;", source_location_);

  // word-wrap is treated as an alias for overflow-wrap
  //   http://www.w3.org/TR/css-text-3/#overflow-wrap
  EXPECT_EQ(cssom::KeywordValue::GetNormal(), style->overflow_wrap());
}

TEST_F(ParserTest, ParsesValidPropertyValue) {
  scoped_refptr<cssom::RGBAColorValue> color =
      dynamic_cast<cssom::RGBAColorValue*>(
          parser_.ParsePropertyValue("color", "#c0ffee", source_location_)
              .get());
  EXPECT_TRUE(color);
}

TEST_F(ParserTest, WarnsAboutImportantInPropertyValue) {
  EXPECT_CALL(
      parser_observer_,
      OnWarning("[object ParserTest]:1:1: warning: !important is not allowed "
                "when setting single property values."));

  scoped_refptr<cssom::PropertyValue> null_value = parser_.ParsePropertyValue(
      "color", "#f0000d !important", source_location_);
  EXPECT_FALSE(null_value);
}

TEST_F(ParserTest, FailsIfPropertyValueHasTrailingSemicolon) {
  EXPECT_CALL(
      parser_observer_,
      OnError("[object ParserTest]:1:8: error: unrecoverable syntax error"));

  scoped_refptr<cssom::PropertyValue> null_value =
      parser_.ParsePropertyValue("color", "#baaaad;", source_location_);
  EXPECT_FALSE(null_value);
}

TEST_F(ParserTest, WarnsAboutInvalidPropertyValue) {
  EXPECT_CALL(parser_observer_,
              OnWarning("[object ParserTest]:1:1: warning: unsupported value"));

  scoped_refptr<cssom::PropertyValue> null_value =
      parser_.ParsePropertyValue("color", "spring grass", source_location_);
  EXPECT_FALSE(null_value);
}

TEST_F(ParserTest, ParsesKeyframesRule) {
  scoped_refptr<cssom::CSSRule> rule = parser_.ParseRule(
      "@keyframes foo {"
      "  from {"
      "    opacity: 0.25;"
      "  }"
      "  25%, 75% {"
      "    opacity: 0.5;"
      "  }"
      "  to {"
      "    opacity: 0.75;"
      "  }"
      "}",
      source_location_);

  cssom::CSSKeyframesRule* keyframes_rule =
      dynamic_cast<cssom::CSSKeyframesRule*>(rule.get());
  ASSERT_TRUE(keyframes_rule);
  EXPECT_EQ("foo", keyframes_rule->name());

  ASSERT_EQ(3, keyframes_rule->css_rules()->length());

  cssom::CSSKeyframeRule* from_keyframe_rule =
      dynamic_cast<cssom::CSSKeyframeRule*>(
          keyframes_rule->css_rules()->Item(0).get());
  ASSERT_TRUE(from_keyframe_rule);
  ASSERT_EQ(1, from_keyframe_rule->offsets().size());
  EXPECT_EQ(0.0f, from_keyframe_rule->offsets()[0]);
  cssom::NumberValue* from_opacity_value = dynamic_cast<cssom::NumberValue*>(
      from_keyframe_rule->style()->data()->opacity().get());
  ASSERT_TRUE(from_opacity_value);
  EXPECT_EQ(0.25f, from_opacity_value->value());

  cssom::CSSKeyframeRule* mid_keyframe_rule =
      dynamic_cast<cssom::CSSKeyframeRule*>(
          keyframes_rule->css_rules()->Item(1).get());
  ASSERT_TRUE(mid_keyframe_rule);
  ASSERT_EQ(2, mid_keyframe_rule->offsets().size());
  EXPECT_FLOAT_EQ(0.25f, mid_keyframe_rule->offsets()[0]);
  EXPECT_FLOAT_EQ(0.75f, mid_keyframe_rule->offsets()[1]);
  cssom::NumberValue* mid_opacity_value = dynamic_cast<cssom::NumberValue*>(
      mid_keyframe_rule->style()->data()->opacity().get());
  ASSERT_TRUE(mid_opacity_value);
  EXPECT_EQ(0.5f, mid_opacity_value->value());

  cssom::CSSKeyframeRule* to_keyframe_rule =
      dynamic_cast<cssom::CSSKeyframeRule*>(
          keyframes_rule->css_rules()->Item(2).get());
  ASSERT_TRUE(to_keyframe_rule);
  ASSERT_EQ(1, to_keyframe_rule->offsets().size());
  EXPECT_EQ(1.0f, to_keyframe_rule->offsets()[0]);
  cssom::NumberValue* to_opacity_value = dynamic_cast<cssom::NumberValue*>(
      to_keyframe_rule->style()->data()->opacity().get());
  ASSERT_TRUE(to_opacity_value);
  EXPECT_EQ(0.75f, to_opacity_value->value());
}

TEST_F(ParserTest, IgnoresOtherBrowserKeyframesRules) {
  EXPECT_CALL(parser_observer_,
              OnWarning("[object ParserTest]:1:1: warning: invalid rule"));
  EXPECT_CALL(parser_observer_,
              OnWarning("[object ParserTest]:2:1: warning: invalid rule"));

  scoped_refptr<cssom::CSSStyleSheet> style_sheet = parser_.ParseStyleSheet(
      "@-webkit-keyframes foo1 {"
      "  from {"
      "    opacity: 0.6;"
      "  }"
      "  to {"
      "    opacity: 0.8;"
      "  }"
      "}\n"
      "@-o-keyframes foo2 {"
      "  from {"
      "    opacity: 0.3;"
      "  }"
      "  to {"
      "    opacity: 0.4;"
      "  }"
      "}\n"
      "@keyframes foo3 {"
      "  from {"
      "    opacity: 0.25;"
      "  }"
      "  25%, 75% {"
      "    opacity: 0.5;"
      "  }"
      "  to {"
      "    opacity: 0.75;"
      "  }"
      "}",
      source_location_);

  ASSERT_TRUE(style_sheet);
  EXPECT_EQ(1, style_sheet->css_rules()->length());

  cssom::CSSKeyframesRule* keyframes_rule =
      dynamic_cast<cssom::CSSKeyframesRule*>(
          style_sheet->css_rules()->Item(0).get());
  ASSERT_TRUE(keyframes_rule);
  EXPECT_EQ("foo3", keyframes_rule->name());

  ASSERT_EQ(3, keyframes_rule->css_rules()->length());
}

TEST_F(ParserTest, ParsesAnimationDurationWithSingleValue) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("animation-duration: 1s;",
                                        source_location_);

  scoped_refptr<cssom::ListValue<base::TimeDelta> > animation_duration =
      dynamic_cast<cssom::ListValue<base::TimeDelta>*>(
          style->animation_duration().get());
  EXPECT_TRUE(animation_duration.get());
  ASSERT_EQ(1, animation_duration->value().size());
  EXPECT_DOUBLE_EQ(1, animation_duration->value()[0].InSecondsF());
}

TEST_F(ParserTest, ParsesAnimationDelayWithSingleValue) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("animation-delay: 1s;",
                                        source_location_);

  scoped_refptr<cssom::ListValue<base::TimeDelta> > animation_delay =
      dynamic_cast<cssom::ListValue<base::TimeDelta>*>(
          style->animation_delay().get());
  EXPECT_TRUE(animation_delay);
  ASSERT_EQ(1, animation_delay->value().size());
  EXPECT_DOUBLE_EQ(1, animation_delay->value()[0].InSecondsF());
}

TEST_F(ParserTest, ParsesAnimationDirectionWithNormalValue) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("animation-direction: normal;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> animation_direction =
      dynamic_cast<cssom::PropertyListValue*>(
          style->animation_direction().get());
  EXPECT_TRUE(animation_direction);
  ASSERT_EQ(1, animation_direction->value().size());
  EXPECT_EQ(cssom::KeywordValue::GetNormal(), animation_direction->value()[0]);
}

TEST_F(ParserTest, ParsesAnimationDirectionWithReverseValue) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("animation-direction: reverse;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> animation_direction =
      dynamic_cast<cssom::PropertyListValue*>(
          style->animation_direction().get());
  EXPECT_TRUE(animation_direction);
  ASSERT_EQ(1, animation_direction->value().size());
  EXPECT_EQ(cssom::KeywordValue::GetReverse(), animation_direction->value()[0]);
}

TEST_F(ParserTest, ParsesAnimationDirectionWithAlternateValue) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("animation-direction: alternate;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> animation_direction =
      dynamic_cast<cssom::PropertyListValue*>(
          style->animation_direction().get());
  EXPECT_TRUE(animation_direction);
  ASSERT_EQ(1, animation_direction->value().size());
  EXPECT_EQ(cssom::KeywordValue::GetAlternate(),
            animation_direction->value()[0]);
}

TEST_F(ParserTest, ParsesAnimationDirectionWithAlternateReverseValue) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "animation-direction: alternate-reverse;", source_location_);

  scoped_refptr<cssom::PropertyListValue> animation_direction =
      dynamic_cast<cssom::PropertyListValue*>(
          style->animation_direction().get());
  EXPECT_TRUE(animation_direction);
  ASSERT_EQ(1, animation_direction->value().size());
  EXPECT_EQ(cssom::KeywordValue::GetAlternateReverse(),
            animation_direction->value()[0]);
}

TEST_F(ParserTest, ParsesAnimationFillModeWithForwardsValue) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("animation-fill-mode: forwards;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> animation_fill_mode =
      dynamic_cast<cssom::PropertyListValue*>(
          style->animation_fill_mode().get());
  EXPECT_TRUE(animation_fill_mode);
  ASSERT_EQ(1, animation_fill_mode->value().size());
  EXPECT_EQ(cssom::KeywordValue::GetForwards(),
            animation_fill_mode->value()[0]);
}

TEST_F(ParserTest, ParsesAnimationFillModeWithBackwardsValue) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("animation-fill-mode: backwards;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> animation_fill_mode =
      dynamic_cast<cssom::PropertyListValue*>(
          style->animation_fill_mode().get());
  EXPECT_TRUE(animation_fill_mode);
  ASSERT_EQ(1, animation_fill_mode->value().size());
  EXPECT_EQ(cssom::KeywordValue::GetBackwards(),
            animation_fill_mode->value()[0]);
}

TEST_F(ParserTest, ParsesAnimationFillModeWithBothValue) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("animation-fill-mode: both;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> animation_fill_mode =
      dynamic_cast<cssom::PropertyListValue*>(
          style->animation_fill_mode().get());
  EXPECT_TRUE(animation_fill_mode);
  ASSERT_EQ(1, animation_fill_mode->value().size());
  EXPECT_EQ(cssom::KeywordValue::GetBoth(), animation_fill_mode->value()[0]);
}

TEST_F(ParserTest, ParsesAnimationFillModeWithListOfValues) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "animation-fill-mode: forwards, backwards, both, none;",
          source_location_);

  scoped_refptr<cssom::PropertyListValue> animation_fill_mode =
      dynamic_cast<cssom::PropertyListValue*>(
          style->animation_fill_mode().get());
  EXPECT_TRUE(animation_fill_mode);
  ASSERT_EQ(4, animation_fill_mode->value().size());
  EXPECT_EQ(cssom::KeywordValue::GetForwards(),
            animation_fill_mode->value()[0]);
  EXPECT_EQ(cssom::KeywordValue::GetBackwards(),
            animation_fill_mode->value()[1]);
  EXPECT_EQ(cssom::KeywordValue::GetBoth(), animation_fill_mode->value()[2]);
  EXPECT_EQ(cssom::KeywordValue::GetNone(), animation_fill_mode->value()[3]);
}

TEST_F(ParserTest, ParsesAnimationIterationCountWithIntegerValue) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("animation-iteration-count: 2;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> animation_iteration_count =
      dynamic_cast<cssom::PropertyListValue*>(
          style->animation_iteration_count().get());
  EXPECT_TRUE(animation_iteration_count);
  ASSERT_EQ(1, animation_iteration_count->value().size());
  cssom::NumberValue* number_value =
      base::polymorphic_downcast<cssom::NumberValue*>(
          animation_iteration_count->value()[0].get());
  EXPECT_EQ(2.0f, number_value->value());
}

TEST_F(ParserTest, ParsesAnimationIterationCountWithInfiniteValue) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("animation-iteration-count: infinite;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> animation_iteration_count =
      dynamic_cast<cssom::PropertyListValue*>(
          style->animation_iteration_count().get());
  EXPECT_TRUE(animation_iteration_count);
  ASSERT_EQ(1, animation_iteration_count->value().size());
  EXPECT_EQ(cssom::KeywordValue::GetInfinite(),
            animation_iteration_count->value()[0]);
}

TEST_F(ParserTest, ParsesAnimationIterationCountWithMultipleValues) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "animation-iteration-count: 1, infinite, 0.5;", source_location_);

  scoped_refptr<cssom::PropertyListValue> animation_iteration_count =
      dynamic_cast<cssom::PropertyListValue*>(
          style->animation_iteration_count().get());
  EXPECT_TRUE(animation_iteration_count);
  ASSERT_EQ(3, animation_iteration_count->value().size());
  cssom::NumberValue* first_value =
      base::polymorphic_downcast<cssom::NumberValue*>(
          animation_iteration_count->value()[0].get());
  EXPECT_EQ(1.0f, first_value->value());
  EXPECT_EQ(cssom::KeywordValue::GetInfinite(),
            animation_iteration_count->value()[1]);
  cssom::NumberValue* third_value =
      base::polymorphic_downcast<cssom::NumberValue*>(
          animation_iteration_count->value()[2].get());
  EXPECT_EQ(0.5f, third_value->value());
}

TEST_F(ParserTest, ParsesAnimationNameWithArbitraryName) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("animation-name: foo;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> animation_name =
      dynamic_cast<cssom::PropertyListValue*>(style->animation_name().get());
  EXPECT_TRUE(animation_name);
  ASSERT_EQ(1, animation_name->value().size());
  cssom::StringValue* name_value =
      base::polymorphic_downcast<cssom::StringValue*>(
          animation_name->value()[0].get());
  EXPECT_EQ("foo", name_value->value());
}

TEST_F(ParserTest, ParsesAnimationNameWithNone) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("animation-name: none;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> animation_name =
      dynamic_cast<cssom::PropertyListValue*>(style->animation_name().get());
  EXPECT_TRUE(animation_name);
  ASSERT_EQ(1, animation_name->value().size());
  EXPECT_EQ(cssom::KeywordValue::GetNone(), animation_name->value()[0]);
}

TEST_F(ParserTest, ParsesAnimationNameWithMultipleItems) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("animation-name: foo, none, bar;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> animation_name =
      dynamic_cast<cssom::PropertyListValue*>(style->animation_name().get());
  EXPECT_TRUE(animation_name);
  ASSERT_EQ(3, animation_name->value().size());
  cssom::StringValue* first_value =
      base::polymorphic_downcast<cssom::StringValue*>(
          animation_name->value()[0].get());
  EXPECT_EQ("foo", first_value->value());
  EXPECT_EQ(cssom::KeywordValue::GetNone(), animation_name->value()[1]);
  cssom::StringValue* third_value =
      base::polymorphic_downcast<cssom::StringValue*>(
          animation_name->value()[2].get());
  EXPECT_EQ("bar", third_value->value());
}

namespace {
scoped_refptr<cssom::TimingFunctionListValue> CreateSingleTimingFunctionValue(
    const scoped_refptr<cssom::TimingFunction>& timing_function) {
  scoped_ptr<cssom::TimingFunctionListValue::Builder>
      expected_result_list_builder(
          new cssom::TimingFunctionListValue::Builder());
  expected_result_list_builder->push_back(timing_function);
  return new cssom::TimingFunctionListValue(
      expected_result_list_builder.Pass());
}
}  // namespace

TEST_F(ParserTest, ParsesAnimationTimingFunction) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("animation-timing-function: linear;",
                                        source_location_);

  scoped_refptr<cssom::TimingFunctionListValue> animation_timing_function =
      dynamic_cast<cssom::TimingFunctionListValue*>(
          style->animation_timing_function().get());
  CHECK_NE(static_cast<cssom::TimingFunctionListValue*>(NULL),
           animation_timing_function);

  scoped_refptr<cssom::TimingFunctionListValue> expected_result_list(
      CreateSingleTimingFunctionValue(
          new cssom::CubicBezierTimingFunction(0.0f, 0.0f, 1.0f, 1.0f)));

  EXPECT_TRUE(expected_result_list->Equals(*animation_timing_function));
}

TEST_F(ParserTest, ParsesAnimationShorthandForName) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("animation: foo;", source_location_);

  scoped_refptr<cssom::PropertyListValue> animation_name =
      dynamic_cast<cssom::PropertyListValue*>(style->animation_name().get());
  EXPECT_TRUE(animation_name);
  ASSERT_EQ(1, animation_name->value().size());
  cssom::StringValue* name_value =
      base::polymorphic_downcast<cssom::StringValue*>(
          animation_name->value()[0].get());
  EXPECT_EQ("foo", name_value->value());
}

TEST_F(ParserTest, ParsesAnimationShorthandForDuration) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("animation: 1s;", source_location_);

  scoped_refptr<cssom::ListValue<base::TimeDelta> > animation_duration =
      dynamic_cast<cssom::ListValue<base::TimeDelta>*>(
          style->animation_duration().get());
  EXPECT_TRUE(animation_duration);
  ASSERT_EQ(1, animation_duration->value().size());
  EXPECT_DOUBLE_EQ(1.0f, animation_duration->value()[0].InSecondsF());
}

TEST_F(ParserTest, ParsesAnimationShorthandForDirection) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("animation: reverse;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> animation_direction =
      dynamic_cast<cssom::PropertyListValue*>(
          style->animation_direction().get());
  EXPECT_TRUE(animation_direction);
  ASSERT_EQ(1, animation_direction->value().size());
  EXPECT_EQ(cssom::KeywordValue::GetReverse(), animation_direction->value()[0]);
}

TEST_F(ParserTest, ParsesAnimationShorthandForDurationAndDelay) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("animation: 1s 2s;", source_location_);

  scoped_refptr<cssom::ListValue<base::TimeDelta> > animation_duration =
      dynamic_cast<cssom::ListValue<base::TimeDelta>*>(
          style->animation_duration().get());
  EXPECT_TRUE(animation_duration);
  ASSERT_EQ(1, animation_duration->value().size());
  EXPECT_DOUBLE_EQ(1.0f, animation_duration->value()[0].InSecondsF());

  scoped_refptr<cssom::ListValue<base::TimeDelta> > animation_delay =
      dynamic_cast<cssom::ListValue<base::TimeDelta>*>(
          style->animation_delay().get());
  EXPECT_TRUE(animation_delay);
  ASSERT_EQ(1, animation_delay->value().size());
  EXPECT_DOUBLE_EQ(2.0f, animation_delay->value()[0].InSecondsF());
}

TEST_F(ParserTest, ParsesAnimationShorthandForIterationCount) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("animation: 1.5;", source_location_);

  scoped_refptr<cssom::PropertyListValue> animation_iteration_count =
      dynamic_cast<cssom::PropertyListValue*>(
          style->animation_iteration_count().get());
  EXPECT_TRUE(animation_iteration_count);
  ASSERT_EQ(1, animation_iteration_count->value().size());
  cssom::NumberValue* number_value =
      base::polymorphic_downcast<cssom::NumberValue*>(
          animation_iteration_count->value()[0].get());
  EXPECT_EQ(1.5f, number_value->value());
}

TEST_F(ParserTest, ParsesAnimationShorthandForTimingFunction) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("animation: linear;", source_location_);

  scoped_refptr<cssom::TimingFunctionListValue> animation_timing_function =
      dynamic_cast<cssom::TimingFunctionListValue*>(
          style->animation_timing_function().get());
  CHECK_NE(static_cast<cssom::TimingFunctionListValue*>(NULL),
           animation_timing_function);

  scoped_refptr<cssom::TimingFunctionListValue> expected_result_list(
      CreateSingleTimingFunctionValue(
          new cssom::CubicBezierTimingFunction(0.0f, 0.0f, 1.0f, 1.0f)));

  EXPECT_TRUE(expected_result_list->Equals(*animation_timing_function));
}

TEST_F(ParserTest, ParsesAnimationShorthandForFillMode) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("animation: forwards;",
                                        source_location_);

  scoped_refptr<cssom::PropertyListValue> animation_fill_mode =
      dynamic_cast<cssom::PropertyListValue*>(
          style->animation_fill_mode().get());
  EXPECT_TRUE(animation_fill_mode);
  ASSERT_EQ(1, animation_fill_mode->value().size());
  EXPECT_EQ(cssom::KeywordValue::GetForwards(),
            animation_fill_mode->value()[0]);
}

TEST_F(ParserTest, ParsesAnimationShorthandForAllProperties) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "animation: linear foo 1s 2s 1.5 forwards alternate;",
          source_location_);

  scoped_refptr<cssom::PropertyListValue> animation_fill_mode =
      dynamic_cast<cssom::PropertyListValue*>(
          style->animation_fill_mode().get());
  EXPECT_TRUE(animation_fill_mode);
  ASSERT_EQ(1, animation_fill_mode->value().size());
  EXPECT_EQ(cssom::KeywordValue::GetForwards(),
            animation_fill_mode->value()[0]);

  scoped_refptr<cssom::TimingFunctionListValue> animation_timing_function =
      dynamic_cast<cssom::TimingFunctionListValue*>(
          style->animation_timing_function().get());
  CHECK_NE(static_cast<cssom::TimingFunctionListValue*>(NULL),
           animation_timing_function);

  scoped_refptr<cssom::TimingFunctionListValue> expected_result_list(
      CreateSingleTimingFunctionValue(
          new cssom::CubicBezierTimingFunction(0.0f, 0.0f, 1.0f, 1.0f)));

  EXPECT_TRUE(expected_result_list->Equals(*animation_timing_function));

  scoped_refptr<cssom::PropertyListValue> animation_iteration_count =
      dynamic_cast<cssom::PropertyListValue*>(
          style->animation_iteration_count().get());
  EXPECT_TRUE(animation_iteration_count);
  ASSERT_EQ(1, animation_iteration_count->value().size());
  cssom::NumberValue* number_value =
      base::polymorphic_downcast<cssom::NumberValue*>(
          animation_iteration_count->value()[0].get());
  EXPECT_EQ(1.5f, number_value->value());

  scoped_refptr<cssom::ListValue<base::TimeDelta> > animation_duration =
      dynamic_cast<cssom::ListValue<base::TimeDelta>*>(
          style->animation_duration().get());
  EXPECT_TRUE(animation_duration);
  ASSERT_EQ(1, animation_duration->value().size());
  EXPECT_DOUBLE_EQ(1.0f, animation_duration->value()[0].InSecondsF());

  scoped_refptr<cssom::ListValue<base::TimeDelta> > animation_delay =
      dynamic_cast<cssom::ListValue<base::TimeDelta>*>(
          style->animation_delay().get());
  EXPECT_TRUE(animation_delay);
  ASSERT_EQ(1, animation_delay->value().size());
  EXPECT_DOUBLE_EQ(2.0f, animation_delay->value()[0].InSecondsF());

  scoped_refptr<cssom::PropertyListValue> animation_name =
      dynamic_cast<cssom::PropertyListValue*>(style->animation_name().get());
  EXPECT_TRUE(animation_name);
  ASSERT_EQ(1, animation_name->value().size());
  cssom::StringValue* name_value =
      base::polymorphic_downcast<cssom::StringValue*>(
          animation_name->value()[0].get());
  EXPECT_EQ("foo", name_value->value());

  scoped_refptr<cssom::PropertyListValue> animation_direction =
      dynamic_cast<cssom::PropertyListValue*>(
          style->animation_direction().get());
  EXPECT_TRUE(animation_direction);
  ASSERT_EQ(1, animation_direction->value().size());
  EXPECT_EQ(cssom::KeywordValue::GetAlternate(),
            animation_direction->value()[0]);
}

TEST_F(ParserTest, ParsesAnimationShorthandForMultipleAnimations) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("animation: foo 1s 2s, bar 3s 4;",
                                        source_location_);

  scoped_refptr<cssom::ListValue<base::TimeDelta> > animation_duration =
      dynamic_cast<cssom::ListValue<base::TimeDelta>*>(
          style->animation_duration().get());
  EXPECT_TRUE(animation_duration);
  ASSERT_EQ(2, animation_duration->value().size());
  EXPECT_DOUBLE_EQ(1.0f, animation_duration->value()[0].InSecondsF());
  EXPECT_DOUBLE_EQ(3.0f, animation_duration->value()[1].InSecondsF());

  scoped_refptr<cssom::ListValue<base::TimeDelta> > animation_delay =
      dynamic_cast<cssom::ListValue<base::TimeDelta>*>(
          style->animation_delay().get());
  EXPECT_TRUE(animation_delay);
  ASSERT_EQ(2, animation_delay->value().size());
  EXPECT_DOUBLE_EQ(2.0f, animation_delay->value()[0].InSecondsF());
  EXPECT_DOUBLE_EQ(0.0f, animation_delay->value()[1].InSecondsF());

  scoped_refptr<cssom::PropertyListValue> animation_name =
      dynamic_cast<cssom::PropertyListValue*>(style->animation_name().get());
  EXPECT_TRUE(animation_name);
  ASSERT_EQ(2, animation_name->value().size());
  cssom::StringValue* first_name_value =
      base::polymorphic_downcast<cssom::StringValue*>(
          animation_name->value()[0].get());
  EXPECT_EQ("foo", first_name_value->value());
  cssom::StringValue* second_name_value =
      base::polymorphic_downcast<cssom::StringValue*>(
          animation_name->value()[1].get());
  EXPECT_EQ("bar", second_name_value->value());

  scoped_refptr<cssom::PropertyListValue> animation_iteration_count =
      dynamic_cast<cssom::PropertyListValue*>(
          style->animation_iteration_count().get());
  EXPECT_TRUE(animation_iteration_count);
  ASSERT_EQ(2, animation_iteration_count->value().size());
  cssom::NumberValue* first_number_value =
      base::polymorphic_downcast<cssom::NumberValue*>(
          animation_iteration_count->value()[0].get());
  EXPECT_EQ(1.0f, first_number_value->value());
  cssom::NumberValue* second_number_value =
      base::polymorphic_downcast<cssom::NumberValue*>(
          animation_iteration_count->value()[1].get());
  EXPECT_EQ(4.0f, second_number_value->value());

  scoped_refptr<cssom::PropertyListValue> animation_fill_mode =
      dynamic_cast<cssom::PropertyListValue*>(
          style->animation_fill_mode().get());
  EXPECT_TRUE(animation_fill_mode);
  ASSERT_EQ(2, animation_fill_mode->value().size());
  EXPECT_EQ(cssom::KeywordValue::GetNone(), animation_fill_mode->value()[0]);
  EXPECT_EQ(cssom::KeywordValue::GetNone(), animation_fill_mode->value()[1]);
}

TEST_F(ParserTest, ParsesAnimatablePropertyNameListWithSingleValue) {
  scoped_refptr<cssom::PropertyKeyListValue> property_name_list =
      dynamic_cast<cssom::PropertyKeyListValue*>(
          parser_.ParsePropertyValue("transition-property", "color",
                                     source_location_)
              .get());
  ASSERT_TRUE(property_name_list);

  ASSERT_EQ(1, property_name_list->value().size());
  EXPECT_EQ(cssom::kColorProperty, property_name_list->value()[0]);
}

TEST_F(ParserTest, ParsesAnimatablePropertyNameListWithMultipleValues) {
  scoped_refptr<cssom::PropertyKeyListValue> property_name_list =
      dynamic_cast<cssom::PropertyKeyListValue*>(
          parser_.ParsePropertyValue("transition-property",
                                     "color, transform , background-color",
                                     source_location_)
              .get());
  ASSERT_TRUE(property_name_list);

  ASSERT_EQ(3, property_name_list->value().size());
  EXPECT_EQ(cssom::kColorProperty, property_name_list->value()[0]);
  EXPECT_EQ(cssom::kTransformProperty, property_name_list->value()[1]);
  EXPECT_EQ(cssom::kBackgroundColorProperty, property_name_list->value()[2]);
}

TEST_F(ParserTest, ParsesTransitionPropertyWithAllValue) {
  scoped_refptr<cssom::PropertyKeyListValue> property_name_list =
      dynamic_cast<cssom::PropertyKeyListValue*>(
          parser_.ParsePropertyValue("transition-property", "all",
                                     source_location_)
              .get());
  ASSERT_TRUE(property_name_list.get());

  ASSERT_EQ(1, property_name_list->value().size());
  EXPECT_EQ(cssom::kAllProperty, property_name_list->value()[0]);
}

TEST_F(ParserTest, ParsesTransitionPropertyWithNoneValue) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("transition-property: none;",
                                        source_location_);

  scoped_refptr<cssom::KeywordValue> transition_property =
      dynamic_cast<cssom::KeywordValue*>(style->transition_property().get());
  EXPECT_EQ(cssom::KeywordValue::GetNone().get(), transition_property.get());
}

TEST_F(ParserTest, ParsesTimeListWithSingleValue) {
  scoped_refptr<cssom::TimeListValue> time_list_value =
      dynamic_cast<cssom::TimeListValue*>(
          parser_.ParsePropertyValue("transition-duration", "1s",
                                     source_location_).get());
  ASSERT_TRUE(time_list_value.get());

  ASSERT_EQ(1, time_list_value->value().size());
  EXPECT_DOUBLE_EQ(1.0, time_list_value->value()[0].InSecondsF());
}

TEST_F(ParserTest, ParsesTimeListWithMultipleValues) {
  scoped_refptr<cssom::TimeListValue> time_list_value =
      dynamic_cast<cssom::TimeListValue*>(
          parser_.ParsePropertyValue("transition-duration",
                                     "2s, 1ms, 0, 2ms, 3s, 3ms",
                                     source_location_).get());
  ASSERT_TRUE(time_list_value.get());

  ASSERT_EQ(6, time_list_value->value().size());
  EXPECT_DOUBLE_EQ(2, time_list_value->value()[0].InSecondsF());
  EXPECT_DOUBLE_EQ(0.001, time_list_value->value()[1].InSecondsF());
  EXPECT_DOUBLE_EQ(0, time_list_value->value()[2].InSecondsF());
  EXPECT_DOUBLE_EQ(0.002, time_list_value->value()[3].InSecondsF());
  EXPECT_DOUBLE_EQ(3, time_list_value->value()[4].InSecondsF());
  EXPECT_DOUBLE_EQ(0.003, time_list_value->value()[5].InSecondsF());
}

TEST_F(ParserTest, ParsesNegativeTimeList) {
  scoped_refptr<cssom::TimeListValue> time_list_value =
      dynamic_cast<cssom::TimeListValue*>(
          parser_.ParsePropertyValue("transition-duration", "-4s",
                                     source_location_).get());
  ASSERT_TRUE(time_list_value.get());

  ASSERT_EQ(1, time_list_value->value().size());
  EXPECT_DOUBLE_EQ(-4, time_list_value->value()[0].InSecondsF());
}

TEST_F(ParserTest, ParsesTransitionDelayWithSingleValue) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("transition-delay: 1s;",
                                        source_location_);

  scoped_refptr<cssom::ListValue<base::TimeDelta> > transition_delay =
      dynamic_cast<cssom::ListValue<base::TimeDelta>*>(
          style->transition_delay().get());
  EXPECT_TRUE(transition_delay.get());
  ASSERT_EQ(1, transition_delay->value().size());
  EXPECT_DOUBLE_EQ(1, transition_delay->value()[0].InSecondsF());
}

TEST_F(ParserTest, ParsesTransitionDurationWithSingleValue) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("transition-duration: 1s;",
                                        source_location_);

  scoped_refptr<cssom::TimeListValue> transition_duration =
      dynamic_cast<cssom::TimeListValue*>(style->transition_duration().get());
  EXPECT_TRUE(transition_duration.get());
  ASSERT_EQ(1, transition_duration->value().size());
  EXPECT_DOUBLE_EQ(1, transition_duration->value()[0].InSecondsF());
}

namespace {

// Returns true if the timing function obtained from parsing the passed in
// property declaration string is equal to the passed in expected timing
// function.
bool TestTransitionTimingFunctionKeyword(
    Parser* parser, const base::SourceLocation& source_location,
    const std::string& property_declaration_string,
    const scoped_refptr<cssom::TimingFunction>& expected_result) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser->ParseStyleDeclarationList(
          "transition-timing-function: " + property_declaration_string + ";",
          source_location);

  scoped_refptr<cssom::TimingFunctionListValue> transition_timing_function =
      dynamic_cast<cssom::TimingFunctionListValue*>(
          style->transition_timing_function().get());
  CHECK_NE(static_cast<cssom::TimingFunctionListValue*>(NULL),
           transition_timing_function.get());

  scoped_refptr<cssom::TimingFunctionListValue> expected_result_list(
      CreateSingleTimingFunctionValue(expected_result));

  return expected_result_list->Equals(*transition_timing_function);
}

}  // namespace

TEST_F(ParserTest, ParsesTransitionTimingFunctionEaseKeyword) {
  EXPECT_TRUE(TestTransitionTimingFunctionKeyword(
      &parser_, source_location_, "ease",
      new cssom::CubicBezierTimingFunction(0.25f, 0.1f, 0.25f, 1.0f)));
}
TEST_F(ParserTest, ParsesTransitionTimingFunctionEaseInKeyword) {
  EXPECT_TRUE(TestTransitionTimingFunctionKeyword(
      &parser_, source_location_, "ease-in",
      new cssom::CubicBezierTimingFunction(0.42f, 0.0f, 1.0f, 1.0f)));
}
TEST_F(ParserTest, ParsesTransitionTimingFunctionEaseInOutKeyword) {
  EXPECT_TRUE(TestTransitionTimingFunctionKeyword(
      &parser_, source_location_, "ease-in-out",
      new cssom::CubicBezierTimingFunction(0.42f, 0.0f, 0.58f, 1.0f)));
}
TEST_F(ParserTest, ParsesTransitionTimingFunctionEaseOutKeyword) {
  EXPECT_TRUE(TestTransitionTimingFunctionKeyword(
      &parser_, source_location_, "ease-out",
      new cssom::CubicBezierTimingFunction(0.0f, 0.0f, 0.58f, 1.0f)));
}
TEST_F(ParserTest, ParsesTransitionTimingFunctionLinearKeyword) {
  EXPECT_TRUE(TestTransitionTimingFunctionKeyword(
      &parser_, source_location_, "linear",
      new cssom::CubicBezierTimingFunction(0.0f, 0.0f, 1.0f, 1.0f)));
}
TEST_F(ParserTest, ParsesTransitionTimingFunctionStepEndKeyword) {
  EXPECT_TRUE(TestTransitionTimingFunctionKeyword(
      &parser_, source_location_, "step-end",
      new cssom::SteppingTimingFunction(1,
                                        cssom::SteppingTimingFunction::kEnd)));
}
TEST_F(ParserTest, ParsesTransitionTimingFunctionStepStartKeyword) {
  EXPECT_TRUE(TestTransitionTimingFunctionKeyword(
      &parser_, source_location_, "step-start",
      new cssom::SteppingTimingFunction(
          1, cssom::SteppingTimingFunction::kStart)));
}
TEST_F(ParserTest, ParsesTransitionTimingFunctionArbitraryCubicBezierKeyword) {
  EXPECT_TRUE(TestTransitionTimingFunctionKeyword(
      &parser_, source_location_, "cubic-bezier(0.1, -2.0, 0.3, 2.0)",
      new cssom::CubicBezierTimingFunction(0.1f, -2.0f, 0.3f, 2.0f)));
}
TEST_F(ParserTest,
       ParsesTransitionTimingFunctionArbitrarySteppingStartKeyword) {
  EXPECT_TRUE(TestTransitionTimingFunctionKeyword(
      &parser_, source_location_, "steps(2, start)",
      new cssom::SteppingTimingFunction(
          2, cssom::SteppingTimingFunction::kStart)));
}
TEST_F(ParserTest, ParsesTransitionTimingFunctionArbitrarySteppingEndKeyword) {
  EXPECT_TRUE(TestTransitionTimingFunctionKeyword(
      &parser_, source_location_, "steps(2, end)",
      new cssom::SteppingTimingFunction(2,
                                        cssom::SteppingTimingFunction::kEnd)));
}
TEST_F(ParserTest,
       ParsesTransitionTimingFunctionArbitrarySteppingNoStartOrEndKeyword) {
  EXPECT_TRUE(TestTransitionTimingFunctionKeyword(
      &parser_, source_location_, "steps(2)",
      new cssom::SteppingTimingFunction(2,
                                        cssom::SteppingTimingFunction::kEnd)));
}

TEST_F(ParserTest, ParsesMultipleTransitionTimingFunctions) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "transition-timing-function: ease, ease-in, step-start;",
          source_location_);

  scoped_refptr<cssom::TimingFunctionListValue> transition_timing_function =
      dynamic_cast<cssom::TimingFunctionListValue*>(
          style->transition_timing_function().get());
  ASSERT_TRUE(transition_timing_function.get());

  scoped_ptr<cssom::TimingFunctionListValue::Builder>
      expected_result_list_builder(
          new cssom::TimingFunctionListValue::Builder());
  expected_result_list_builder->push_back(
      cssom::TimingFunction::GetEase().get());
  expected_result_list_builder->push_back(
      cssom::TimingFunction::GetEaseIn().get());
  expected_result_list_builder->push_back(
      cssom::TimingFunction::GetStepStart().get());
  scoped_refptr<cssom::TimingFunctionListValue> expected_result_list(
      new cssom::TimingFunctionListValue(expected_result_list_builder.Pass()));

  EXPECT_TRUE(expected_result_list->Equals(*transition_timing_function));
}

TEST_F(ParserTest, AboveRangeCubicBezierP1XParameterProduceError) {
  EXPECT_CALL(parser_observer_,
              OnError(
                  "[object ParserTest]:1:1: error: cubic-bezier control point "
                  "x values must be in the range [0, 1]."));
  scoped_refptr<cssom::PropertyValue> error_value = parser_.ParsePropertyValue(
      "transition-timing-function", "cubic-bezier(2, 0, 0.5, 0)",
      source_location_);
  // Test that the ease function was returned in place of the error function.
  EXPECT_TRUE(error_value->Equals(*CreateSingleTimingFunctionValue(
                                      cssom::TimingFunction::GetEase().get())));
}
TEST_F(ParserTest, BelowRangeCubicBezierP1XParameterProduceError) {
  EXPECT_CALL(parser_observer_,
              OnError(
                  "[object ParserTest]:1:1: error: cubic-bezier control point "
                  "x values must be in the range [0, 1]."));
  scoped_refptr<cssom::PropertyValue> error_value = parser_.ParsePropertyValue(
      "transition-timing-function", "cubic-bezier(-1, 0, 0.5, 0)",
      source_location_);
  // Test that the ease function was returned in place of the error function.
  EXPECT_TRUE(error_value->Equals(*CreateSingleTimingFunctionValue(
                                      cssom::TimingFunction::GetEase().get())));
}
TEST_F(ParserTest, AboveRangeCubicBezierP2XParameterProduceError) {
  EXPECT_CALL(parser_observer_,
              OnError(
                  "[object ParserTest]:1:1: error: cubic-bezier control point "
                  "x values must be in the range [0, 1]."));
  scoped_refptr<cssom::PropertyValue> error_value = parser_.ParsePropertyValue(
      "transition-timing-function", "cubic-bezier(0.5, 0, 2, 0)",
      source_location_);
  // Test that the ease function was returned in place of the error function.
  EXPECT_TRUE(error_value->Equals(*CreateSingleTimingFunctionValue(
                                      cssom::TimingFunction::GetEase().get())));
}
TEST_F(ParserTest, BelowRangeCubicBezierP2XParameterProduceError) {
  EXPECT_CALL(parser_observer_,
              OnError(
                  "[object ParserTest]:1:1: error: cubic-bezier control point "
                  "x values must be in the range [0, 1]."));
  scoped_refptr<cssom::PropertyValue> error_value = parser_.ParsePropertyValue(
      "transition-timing-function", "cubic-bezier(0.5, 0, -1, 0)",
      source_location_);
  // Test that the ease function was returned in place of the error function.
  EXPECT_TRUE(error_value->Equals(*CreateSingleTimingFunctionValue(
                                      cssom::TimingFunction::GetEase().get())));
}

TEST_F(ParserTest, ParsesTransitionShorthandOfMultipleItemsWithNoDefaults) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "transition: background-color 1s ease-in 0.5s, "
          "transform 2s ease-out 0.5s;",
          source_location_);

  // Test transition-property was set properly.
  scoped_refptr<cssom::PropertyKeyListValue> property_name_list =
      dynamic_cast<cssom::PropertyKeyListValue*>(
          style->transition_property().get());
  ASSERT_TRUE(property_name_list.get());
  ASSERT_EQ(2, property_name_list->value().size());
  EXPECT_EQ(cssom::kBackgroundColorProperty, property_name_list->value()[0]);
  EXPECT_EQ(cssom::kTransformProperty, property_name_list->value()[1]);

  // Test transition-duration was set properly.
  scoped_refptr<cssom::TimeListValue> duration_list =
      dynamic_cast<cssom::TimeListValue*>(style->transition_duration().get());
  ASSERT_TRUE(duration_list.get());
  ASSERT_EQ(2, duration_list->value().size());
  EXPECT_DOUBLE_EQ(1, duration_list->value()[0].InSecondsF());
  EXPECT_DOUBLE_EQ(2, duration_list->value()[1].InSecondsF());

  // Test transition-timing-function was set properly.
  scoped_refptr<cssom::TimingFunctionListValue> timing_function_list =
      dynamic_cast<cssom::TimingFunctionListValue*>(
          style->transition_timing_function().get());
  ASSERT_TRUE(timing_function_list.get());
  ASSERT_EQ(2, timing_function_list->value().size());
  EXPECT_TRUE(timing_function_list->value()[0]->Equals(
      *cssom::TimingFunction::GetEaseIn()));
  EXPECT_TRUE(timing_function_list->value()[1]->Equals(
      *cssom::TimingFunction::GetEaseOut()));

  // Test transition-delay was set properly.
  scoped_refptr<cssom::TimeListValue> delay_list =
      dynamic_cast<cssom::TimeListValue*>(style->transition_delay().get());
  ASSERT_TRUE(delay_list.get());
  ASSERT_EQ(2, delay_list->value().size());
  EXPECT_DOUBLE_EQ(0.5, delay_list->value()[0].InSecondsF());
  EXPECT_DOUBLE_EQ(0.5, delay_list->value()[1].InSecondsF());
}

TEST_F(ParserTest,
       ParsesTransitionShorthandOfSingleItemWith1PropertySpecified) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("transition: background-color;",
                                        source_location_);

  // Test transition-property was set properly.
  scoped_refptr<cssom::PropertyKeyListValue> property_name_list =
      dynamic_cast<cssom::PropertyKeyListValue*>(
          style->transition_property().get());
  ASSERT_TRUE(property_name_list.get());
  ASSERT_EQ(1, property_name_list->value().size());
  EXPECT_EQ(cssom::kBackgroundColorProperty, property_name_list->value()[0]);

  // Test transition-duration was set properly.
  scoped_refptr<cssom::TimeListValue> duration_list =
      dynamic_cast<cssom::TimeListValue*>(style->transition_duration().get());
  ASSERT_TRUE(duration_list.get());
  ASSERT_EQ(1, duration_list->value().size());
  EXPECT_EQ(
      base::polymorphic_downcast<const cssom::TimeListValue*>(
          cssom::GetPropertyInitialValue(cssom::kTransitionDurationProperty)
              .get())
          ->value()[0],
      duration_list->value()[0]);

  // Test transition-timing-function was set properly.
  scoped_refptr<cssom::TimingFunctionListValue> timing_function_list =
      dynamic_cast<cssom::TimingFunctionListValue*>(
          style->transition_timing_function().get());
  ASSERT_TRUE(timing_function_list.get());
  ASSERT_EQ(1, timing_function_list->value().size());
  EXPECT_TRUE(base::polymorphic_downcast<const cssom::TimingFunctionListValue*>(
                  cssom::GetPropertyInitialValue(
                      cssom::kTransitionTimingFunctionProperty)
                      .get())
                  ->value()[0]
                  ->Equals(*timing_function_list->value()[0]));

  // Test transition-delay was set properly.
  scoped_refptr<cssom::TimeListValue> delay_list =
      dynamic_cast<cssom::TimeListValue*>(style->transition_delay().get());
  ASSERT_TRUE(delay_list.get());
  ASSERT_EQ(1, delay_list->value().size());
  EXPECT_EQ(
      base::polymorphic_downcast<const cssom::TimeListValue*>(
          cssom::GetPropertyInitialValue(cssom::kTransitionDelayProperty).get())
          ->value()[0],
      delay_list->value()[0]);
}

TEST_F(ParserTest,
       ParsesTransitionShorthandOfSingleItemWith2PropertiesSpecified) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("transition: background-color 1s;",
                                        source_location_);

  // Test transition-property was set properly.
  scoped_refptr<cssom::PropertyKeyListValue> property_name_list =
      dynamic_cast<cssom::PropertyKeyListValue*>(
          style->transition_property().get());
  ASSERT_TRUE(property_name_list.get());
  ASSERT_EQ(1, property_name_list->value().size());
  EXPECT_EQ(cssom::kBackgroundColorProperty, property_name_list->value()[0]);

  // Test transition-duration was set properly.
  scoped_refptr<cssom::TimeListValue> duration_list =
      dynamic_cast<cssom::TimeListValue*>(style->transition_duration().get());
  ASSERT_TRUE(duration_list.get());
  ASSERT_EQ(1, duration_list->value().size());
  EXPECT_DOUBLE_EQ(1, duration_list->value()[0].InSecondsF());

  // Test transition-timing-function was set properly.
  scoped_refptr<cssom::TimingFunctionListValue> timing_function_list =
      dynamic_cast<cssom::TimingFunctionListValue*>(
          style->transition_timing_function().get());
  ASSERT_TRUE(timing_function_list.get());
  ASSERT_EQ(1, timing_function_list->value().size());
  EXPECT_TRUE(base::polymorphic_downcast<const cssom::TimingFunctionListValue*>(
                  cssom::GetPropertyInitialValue(
                      cssom::kTransitionTimingFunctionProperty)
                      .get())
                  ->value()[0]
                  ->Equals(*timing_function_list->value()[0]));

  // Test transition-delay was set properly.
  scoped_refptr<cssom::TimeListValue> delay_list =
      dynamic_cast<cssom::TimeListValue*>(style->transition_delay().get());
  ASSERT_TRUE(delay_list.get());
  ASSERT_EQ(1, delay_list->value().size());
  EXPECT_EQ(
      base::polymorphic_downcast<const cssom::TimeListValue*>(
          cssom::GetPropertyInitialValue(cssom::kTransitionDelayProperty).get())
          ->value()[0],
      delay_list->value()[0]);
}

TEST_F(ParserTest,
       ParsesTransitionShorthandOfSingleItemWith3PropertiesSpecified) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "transition: background-color 1s ease-in;", source_location_);

  // Test transition-property was set properly.
  scoped_refptr<cssom::PropertyKeyListValue> property_name_list =
      dynamic_cast<cssom::PropertyKeyListValue*>(
          style->transition_property().get());
  ASSERT_TRUE(property_name_list.get());
  ASSERT_EQ(1, property_name_list->value().size());
  EXPECT_EQ(cssom::kBackgroundColorProperty, property_name_list->value()[0]);

  // Test transition-duration was set properly.
  scoped_refptr<cssom::TimeListValue> duration_list =
      dynamic_cast<cssom::TimeListValue*>(style->transition_duration().get());
  ASSERT_TRUE(duration_list.get());
  ASSERT_EQ(1, duration_list->value().size());
  EXPECT_DOUBLE_EQ(1, duration_list->value()[0].InSecondsF());

  // Test transition-timing-function was set properly.
  scoped_refptr<cssom::TimingFunctionListValue> timing_function_list =
      dynamic_cast<cssom::TimingFunctionListValue*>(
          style->transition_timing_function().get());
  ASSERT_TRUE(timing_function_list.get());
  ASSERT_EQ(1, timing_function_list->value().size());
  EXPECT_TRUE(timing_function_list->value()[0]->Equals(
      *cssom::TimingFunction::GetEaseIn()));

  // Test transition-delay was set properly.
  scoped_refptr<cssom::TimeListValue> delay_list =
      dynamic_cast<cssom::TimeListValue*>(style->transition_delay().get());
  ASSERT_TRUE(delay_list.get());
  ASSERT_EQ(1, delay_list->value().size());
  EXPECT_EQ(
      base::polymorphic_downcast<const cssom::TimeListValue*>(
          cssom::GetPropertyInitialValue(cssom::kTransitionDelayProperty).get())
          ->value()[0],
      delay_list->value()[0]);
}

TEST_F(
    ParserTest,
    ParsesTransitionShorthandOfSingleItemWith3PropertiesSpecifiedNotInOrder) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "transition: ease-in background-color 1s;", source_location_);

  // Test transition-property was set properly.
  scoped_refptr<cssom::PropertyKeyListValue> property_name_list =
      dynamic_cast<cssom::PropertyKeyListValue*>(
          style->transition_property().get());
  ASSERT_TRUE(property_name_list.get());
  ASSERT_EQ(1, property_name_list->value().size());
  EXPECT_EQ(cssom::kBackgroundColorProperty, property_name_list->value()[0]);

  // Test transition-duration was set properly.
  scoped_refptr<cssom::TimeListValue> duration_list =
      dynamic_cast<cssom::TimeListValue*>(style->transition_duration().get());
  ASSERT_TRUE(duration_list.get());
  ASSERT_EQ(1, duration_list->value().size());
  EXPECT_DOUBLE_EQ(1, duration_list->value()[0].InSecondsF());

  // Test transition-timing-function was set properly.
  scoped_refptr<cssom::TimingFunctionListValue> timing_function_list =
      dynamic_cast<cssom::TimingFunctionListValue*>(
          style->transition_timing_function().get());
  ASSERT_TRUE(timing_function_list.get());
  ASSERT_EQ(1, timing_function_list->value().size());
  EXPECT_TRUE(timing_function_list->value()[0]->Equals(
      *cssom::TimingFunction::GetEaseIn()));

  // Test transition-delay was set properly.
  scoped_refptr<cssom::TimeListValue> delay_list =
      dynamic_cast<cssom::TimeListValue*>(style->transition_delay().get());
  ASSERT_TRUE(delay_list.get());
  ASSERT_EQ(1, delay_list->value().size());
  EXPECT_EQ(
      base::polymorphic_downcast<const cssom::TimeListValue*>(
          cssom::GetPropertyInitialValue(cssom::kTransitionDelayProperty).get())
          ->value()[0],
      delay_list->value()[0]);
}

TEST_F(ParserTest,
       ParsesTransitionShorthandOfMultipleItemsWith2PropertiesSpecified) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "transition: background-color 1s, transform 2s;", source_location_);

  // Test transition-property was set properly.
  scoped_refptr<cssom::PropertyKeyListValue> property_name_list =
      dynamic_cast<cssom::PropertyKeyListValue*>(
          style->transition_property().get());
  ASSERT_TRUE(property_name_list.get());
  ASSERT_EQ(2, property_name_list->value().size());
  EXPECT_EQ(cssom::kBackgroundColorProperty, property_name_list->value()[0]);
  EXPECT_EQ(cssom::kTransformProperty, property_name_list->value()[1]);

  // Test transition-duration was set properly.
  scoped_refptr<cssom::TimeListValue> duration_list =
      dynamic_cast<cssom::TimeListValue*>(style->transition_duration().get());
  ASSERT_TRUE(duration_list.get());
  ASSERT_EQ(2, duration_list->value().size());
  EXPECT_DOUBLE_EQ(1, duration_list->value()[0].InSecondsF());
  EXPECT_DOUBLE_EQ(2, duration_list->value()[1].InSecondsF());

  // Test transition-timing-function was set properly.
  scoped_refptr<cssom::TimingFunctionListValue> timing_function_list =
      dynamic_cast<cssom::TimingFunctionListValue*>(
          style->transition_timing_function().get());
  ASSERT_TRUE(timing_function_list.get());
  ASSERT_EQ(2, timing_function_list->value().size());
  EXPECT_TRUE(base::polymorphic_downcast<const cssom::TimingFunctionListValue*>(
                  cssom::GetPropertyInitialValue(
                      cssom::kTransitionTimingFunctionProperty)
                      .get())
                  ->value()[0]
                  ->Equals(*timing_function_list->value()[0]));
  EXPECT_TRUE(base::polymorphic_downcast<const cssom::TimingFunctionListValue*>(
                  cssom::GetPropertyInitialValue(
                      cssom::kTransitionTimingFunctionProperty)
                      .get())
                  ->value()[0]
                  ->Equals(*timing_function_list->value()[1]));

  // Test transition-delay was set properly.
  scoped_refptr<cssom::TimeListValue> delay_list =
      dynamic_cast<cssom::TimeListValue*>(style->transition_delay().get());
  ASSERT_TRUE(delay_list.get());
  ASSERT_EQ(2, delay_list->value().size());
  EXPECT_EQ(
      base::polymorphic_downcast<const cssom::TimeListValue*>(
          cssom::GetPropertyInitialValue(cssom::kTransitionDelayProperty).get())
          ->value()[0],
      delay_list->value()[0]);
  EXPECT_EQ(
      base::polymorphic_downcast<const cssom::TimeListValue*>(
          cssom::GetPropertyInitialValue(cssom::kTransitionDelayProperty).get())
          ->value()[0],
      delay_list->value()[1]);
}

TEST_F(ParserTest, ParsesTransitionShorthandWithErrorBeforeSemicolon) {
  EXPECT_CALL(parser_observer_, OnError(
                                    "[object ParserTest]:1:16: error: "
                                    "unsupported property value for animation"))
      .Times(AtLeast(1));

  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "transition: 1s voodoo-magic; display: inline;", source_location_);

  // Test transition-property was set properly.
  scoped_refptr<cssom::PropertyKeyListValue> property_name_list =
      dynamic_cast<cssom::PropertyKeyListValue*>(
          style->transition_property().get());
  ASSERT_TRUE(property_name_list.get());
  ASSERT_EQ(0, property_name_list->value().size());

  // Test transition-duration was set properly.
  scoped_refptr<cssom::TimeListValue> duration_list =
      dynamic_cast<cssom::TimeListValue*>(style->transition_duration().get());
  ASSERT_TRUE(duration_list.get());
  ASSERT_EQ(0, duration_list->value().size());

  // Test transition-timing-function was set properly.
  scoped_refptr<cssom::TimingFunctionListValue> timing_function_list =
      dynamic_cast<cssom::TimingFunctionListValue*>(
          style->transition_timing_function().get());
  ASSERT_TRUE(timing_function_list.get());
  ASSERT_EQ(0, timing_function_list->value().size());

  // Test transition-delay was set properly.
  scoped_refptr<cssom::TimeListValue> delay_list =
      dynamic_cast<cssom::TimeListValue*>(style->transition_delay().get());
  ASSERT_TRUE(delay_list.get());
  ASSERT_EQ(0, delay_list->value().size());

  // Test that despite the error, display inline was still set correctly.
  EXPECT_EQ(cssom::KeywordValue::GetInline(), style->display());
}

TEST_F(ParserTest, ParsesTransitionShorthandWithErrorBeforeSpace) {
  EXPECT_CALL(parser_observer_, OnError(
                                    "[object ParserTest]:1:16: error: "
                                    "unsupported property value for animation"))
      .Times(AtLeast(1));

  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("transition: 1s voodoo-magic 2s;",
                                        source_location_);

  // Test transition-property was set properly.
  scoped_refptr<cssom::PropertyKeyListValue> property_name_list =
      dynamic_cast<cssom::PropertyKeyListValue*>(
          style->transition_property().get());
  ASSERT_TRUE(property_name_list.get());
  ASSERT_EQ(0, property_name_list->value().size());

  // Test transition-duration was set properly.
  scoped_refptr<cssom::TimeListValue> duration_list =
      dynamic_cast<cssom::TimeListValue*>(style->transition_duration().get());
  ASSERT_TRUE(duration_list.get());
  ASSERT_EQ(0, duration_list->value().size());

  // Test transition-timing-function was set properly.
  scoped_refptr<cssom::TimingFunctionListValue> timing_function_list =
      dynamic_cast<cssom::TimingFunctionListValue*>(
          style->transition_timing_function().get());
  ASSERT_TRUE(timing_function_list.get());
  ASSERT_EQ(0, timing_function_list->value().size());

  // Test transition-delay was set properly.
  scoped_refptr<cssom::TimeListValue> delay_list =
      dynamic_cast<cssom::TimeListValue*>(style->transition_delay().get());
  ASSERT_TRUE(delay_list.get());
  ASSERT_EQ(0, delay_list->value().size());
}

TEST_F(ParserTest,
       ParsesTransitionShorthandIgnoringErrorButProceedingWithNonError) {
  EXPECT_CALL(parser_observer_, OnError(
                                    "[object ParserTest]:1:16: error: "
                                    "unsupported property value for animation"))
      .Times(AtLeast(1));

  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "transition: 1s voodoo-magic, transform 2s;", source_location_);

  // Test transition-property was set properly.
  scoped_refptr<cssom::PropertyKeyListValue> property_name_list =
      dynamic_cast<cssom::PropertyKeyListValue*>(
          style->transition_property().get());
  ASSERT_TRUE(property_name_list.get());
  ASSERT_EQ(1, property_name_list->value().size());
  EXPECT_EQ(cssom::kTransformProperty, property_name_list->value()[0]);

  // Test transition-duration was set properly.
  scoped_refptr<cssom::TimeListValue> duration_list =
      dynamic_cast<cssom::TimeListValue*>(style->transition_duration().get());
  ASSERT_TRUE(duration_list.get());
  ASSERT_EQ(1, duration_list->value().size());
  EXPECT_DOUBLE_EQ(2, duration_list->value()[0].InSecondsF());

  // Test transition-timing-function was set properly.
  scoped_refptr<cssom::TimingFunctionListValue> timing_function_list =
      dynamic_cast<cssom::TimingFunctionListValue*>(
          style->transition_timing_function().get());
  ASSERT_TRUE(timing_function_list.get());
  ASSERT_EQ(1, timing_function_list->value().size());
  EXPECT_TRUE(base::polymorphic_downcast<const cssom::TimingFunctionListValue*>(
                  cssom::GetPropertyInitialValue(
                      cssom::kTransitionTimingFunctionProperty)
                      .get())
                  ->value()[0]
                  ->Equals(*timing_function_list->value()[0]));

  // Test transition-delay was set properly.
  scoped_refptr<cssom::TimeListValue> delay_list =
      dynamic_cast<cssom::TimeListValue*>(style->transition_delay().get());
  ASSERT_TRUE(delay_list.get());
  ASSERT_EQ(1, delay_list->value().size());
  EXPECT_EQ(
      base::polymorphic_downcast<const cssom::TimeListValue*>(
          cssom::GetPropertyInitialValue(cssom::kTransitionDelayProperty).get())
          ->value()[0],
      delay_list->value()[0]);
}

TEST_F(ParserTest, ParsesTransitionShorthandWithNoneIsValid) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("transition: none 0s;",
                                        source_location_);

  // Test transition-property was set properly.
  EXPECT_EQ(cssom::KeywordValue::GetNone(), style->transition_property());

  // Test transition-duration was set properly.
  scoped_refptr<cssom::TimeListValue> duration_list =
      dynamic_cast<cssom::TimeListValue*>(style->transition_duration().get());
  ASSERT_TRUE(duration_list.get());
  ASSERT_EQ(1, duration_list->value().size());
  EXPECT_DOUBLE_EQ(0, duration_list->value()[0].InSecondsF());
}

TEST_F(ParserTest, ParsesLeftWithLength) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("left: 10px;", source_location_);

  scoped_refptr<cssom::LengthValue> length =
      dynamic_cast<cssom::LengthValue*>(style->left().get());
  ASSERT_TRUE(length);
  EXPECT_EQ(10, length->value());
  EXPECT_EQ(cssom::kPixelsUnit, length->unit());
}

TEST_F(ParserTest, ParsesLeftWithPercentage) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("left: 10%;", source_location_);

  scoped_refptr<cssom::PercentageValue> percentage =
      dynamic_cast<cssom::PercentageValue*>(style->left().get());
  ASSERT_TRUE(percentage);
  EXPECT_FLOAT_EQ(0.1f, percentage->value());
}

TEST_F(ParserTest, ParsesLeftWithNegativePercentage) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("left: -10%;", source_location_);

  scoped_refptr<cssom::PercentageValue> percentage =
      dynamic_cast<cssom::PercentageValue*>(style->left().get());
  ASSERT_TRUE(percentage);
  EXPECT_FLOAT_EQ(-0.1f, percentage->value());
}

TEST_F(ParserTest, ParsesLeftWithAuto) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("left: auto;", source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetAuto(), style->left());
}

TEST_F(ParserTest, ParsesTopWithLength) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("top: 10px;", source_location_);

  scoped_refptr<cssom::LengthValue> length =
      dynamic_cast<cssom::LengthValue*>(style->top().get());
  ASSERT_TRUE(length);
  EXPECT_EQ(10, length->value());
  EXPECT_EQ(cssom::kPixelsUnit, length->unit());
}

TEST_F(ParserTest, ParsesTopWithPercentage) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("top: 10%;", source_location_);

  scoped_refptr<cssom::PercentageValue> percentage =
      dynamic_cast<cssom::PercentageValue*>(style->top().get());
  ASSERT_TRUE(percentage);
  EXPECT_FLOAT_EQ(0.1f, percentage->value());
}

TEST_F(ParserTest, ParsesTopWithNegativePercentage) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("top: -10%;", source_location_);

  scoped_refptr<cssom::PercentageValue> percentage =
      dynamic_cast<cssom::PercentageValue*>(style->top().get());
  ASSERT_TRUE(percentage);
  EXPECT_FLOAT_EQ(-0.1f, percentage->value());
}

TEST_F(ParserTest, ParsesTopWithAuto) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("top: auto;", source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetAuto(), style->top());
}

TEST_F(ParserTest, ParsesRightWithLength) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("right: 10px;", source_location_);

  scoped_refptr<cssom::LengthValue> length =
      dynamic_cast<cssom::LengthValue*>(style->right().get());
  ASSERT_TRUE(length);
  EXPECT_EQ(10, length->value());
  EXPECT_EQ(cssom::kPixelsUnit, length->unit());
}

TEST_F(ParserTest, ParsesRightWithPercentage) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("right: 10%;", source_location_);

  scoped_refptr<cssom::PercentageValue> percentage =
      dynamic_cast<cssom::PercentageValue*>(style->right().get());
  ASSERT_TRUE(percentage);
  EXPECT_FLOAT_EQ(0.1f, percentage->value());
}

TEST_F(ParserTest, ParsesRightWithNegativePercentage) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("right: -10%;", source_location_);

  scoped_refptr<cssom::PercentageValue> percentage =
      dynamic_cast<cssom::PercentageValue*>(style->right().get());
  ASSERT_TRUE(percentage);
  EXPECT_FLOAT_EQ(-0.1f, percentage->value());
}

TEST_F(ParserTest, ParsesRightWithAuto) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("right: auto;", source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetAuto(), style->right());
}

TEST_F(ParserTest, ParsesBottomWithLength) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("bottom: 10px;", source_location_);

  scoped_refptr<cssom::LengthValue> length =
      dynamic_cast<cssom::LengthValue*>(style->bottom().get());
  ASSERT_TRUE(length);
  EXPECT_EQ(10, length->value());
  EXPECT_EQ(cssom::kPixelsUnit, length->unit());
}

TEST_F(ParserTest, ParsesBottomWithPercentage) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("bottom: 10%;", source_location_);

  scoped_refptr<cssom::PercentageValue> percentage =
      dynamic_cast<cssom::PercentageValue*>(style->bottom().get());
  ASSERT_TRUE(percentage);
  EXPECT_FLOAT_EQ(0.1f, percentage->value());
}

TEST_F(ParserTest, ParsesBottomWithNegativePercentage) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("bottom: -10%;", source_location_);

  scoped_refptr<cssom::PercentageValue> percentage =
      dynamic_cast<cssom::PercentageValue*>(style->bottom().get());
  ASSERT_TRUE(percentage);
  EXPECT_FLOAT_EQ(-0.1f, percentage->value());
}

TEST_F(ParserTest, ParsesBottomWithAuto) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("bottom: auto;", source_location_);

  EXPECT_EQ(cssom::KeywordValue::GetAuto(), style->bottom());
}

TEST_F(ParserTest, ParsesZIndex) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("z-index: 3;", source_location_);

  scoped_refptr<cssom::IntegerValue> z_index =
      dynamic_cast<cssom::IntegerValue*>(style->z_index().get());
  ASSERT_TRUE(z_index);
  EXPECT_EQ(3, z_index->value());
}

TEST_F(ParserTest, ParsesNegativeZIndex) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList("z-index: -2;", source_location_);

  scoped_refptr<cssom::IntegerValue> z_index =
      dynamic_cast<cssom::IntegerValue*>(style->z_index().get());
  ASSERT_TRUE(z_index);
  EXPECT_EQ(-2, z_index->value());
}

// Test that style declarations in a list are parsed in specified order.
//   http://www.w3.org/TR/cssom/#dom-cssstyledeclaration-csstext
TEST_F(ParserTest, ParsesDeclarationsInSpecifiedOrder) {
  scoped_refptr<cssom::CSSStyleDeclarationData> style =
      parser_.ParseStyleDeclarationList(
          "height:20px; width:150em; height:20%; width:160px;",
          source_location_);

  scoped_refptr<cssom::PercentageValue> height =
      dynamic_cast<cssom::PercentageValue*>(style->height().get());
  ASSERT_TRUE(height);
  EXPECT_EQ(0.2f, height->value());

  scoped_refptr<cssom::LengthValue> width =
      dynamic_cast<cssom::LengthValue*>(style->width().get());
  ASSERT_TRUE(width);
  EXPECT_EQ(160, width->value());
  EXPECT_EQ(cssom::kPixelsUnit, width->unit());
}

TEST_F(ParserTest, ParsesFontFaceFamily) {
  scoped_refptr<cssom::CSSFontFaceDeclarationData> font_face =
      parser_.ParseFontFaceDeclarationList("font-family: 'Roboto';",
                                           source_location_);

  scoped_refptr<cssom::PropertyListValue> font_family_list =
      dynamic_cast<cssom::PropertyListValue*>(font_face->family().get());
  ASSERT_TRUE(font_family_list);
  EXPECT_EQ(font_family_list->value().size(), 1);

  scoped_refptr<cssom::StringValue> font_family_string =
      dynamic_cast<cssom::StringValue*>(
          font_family_list->get_item_modulo_size(0).get());
  ASSERT_TRUE(font_family_string);
  EXPECT_EQ("Roboto", font_family_string->value());
}

TEST_F(ParserTest, ParsesFontFaceSrcLocalString) {
  scoped_refptr<cssom::CSSFontFaceDeclarationData> font_face =
      parser_.ParseFontFaceDeclarationList("src: local('Roboto');",
                                           source_location_);

  scoped_refptr<cssom::PropertyListValue> src_list =
      dynamic_cast<cssom::PropertyListValue*>(font_face->src().get());
  ASSERT_TRUE(src_list);
  EXPECT_EQ(src_list->value().size(), 1);

  scoped_refptr<cssom::LocalSrcValue> local_src =
      dynamic_cast<cssom::LocalSrcValue*>(
          src_list->get_item_modulo_size(0).get());
  ASSERT_TRUE(local_src);
  EXPECT_EQ("Roboto", local_src->value());
}

TEST_F(ParserTest, ParsesFontFaceSrcLocalIdentifier) {
  scoped_refptr<cssom::CSSFontFaceDeclarationData> font_face =
      parser_.ParseFontFaceDeclarationList("src: local(Roboto);",
                                           source_location_);

  scoped_refptr<cssom::PropertyListValue> src_list =
      dynamic_cast<cssom::PropertyListValue*>(font_face->src().get());
  ASSERT_TRUE(src_list);
  EXPECT_EQ(src_list->value().size(), 1);

  scoped_refptr<cssom::LocalSrcValue> local_src =
      dynamic_cast<cssom::LocalSrcValue*>(
          src_list->get_item_modulo_size(0).get());
  ASSERT_TRUE(local_src);
  EXPECT_EQ("Roboto", local_src->value());
}

TEST_F(ParserTest, ParsesFontFaceSrcUrlWithoutFormat) {
  scoped_refptr<cssom::CSSFontFaceDeclarationData> font_face =
      parser_.ParseFontFaceDeclarationList("src: url('../assets/icons.ttf');",
                                           source_location_);

  scoped_refptr<cssom::PropertyListValue> src_list =
      dynamic_cast<cssom::PropertyListValue*>(font_face->src().get());
  ASSERT_TRUE(src_list);
  EXPECT_EQ(src_list->value().size(), 1);

  scoped_refptr<cssom::UrlSrcValue> url_src = dynamic_cast<cssom::UrlSrcValue*>(
      src_list->get_item_modulo_size(0).get());
  ASSERT_TRUE(url_src);
  EXPECT_EQ("", url_src->format());

  scoped_refptr<cssom::URLValue> url_value =
      dynamic_cast<cssom::URLValue*>(url_src->url().get());
  EXPECT_EQ("../assets/icons.ttf", url_value->value());
}

TEST_F(ParserTest, ParsesFontFaceSrcUrlWithFormat) {
  scoped_refptr<cssom::CSSFontFaceDeclarationData> font_face =
      parser_.ParseFontFaceDeclarationList(
          "src: url(../assets/icons.ttf) format('truetype');",
          source_location_);

  scoped_refptr<cssom::PropertyListValue> src_list =
      dynamic_cast<cssom::PropertyListValue*>(font_face->src().get());
  ASSERT_TRUE(src_list);
  EXPECT_EQ(src_list->value().size(), 1);

  scoped_refptr<cssom::UrlSrcValue> url_src = dynamic_cast<cssom::UrlSrcValue*>(
      src_list->get_item_modulo_size(0).get());
  ASSERT_TRUE(url_src);
  EXPECT_EQ("truetype", url_src->format());

  scoped_refptr<cssom::URLValue> url_value =
      dynamic_cast<cssom::URLValue*>(url_src->url().get());

  EXPECT_EQ("../assets/icons.ttf", url_value->value());
}

TEST_F(ParserTest, ParsesFontFaceSrcList) {
  scoped_refptr<cssom::CSSFontFaceDeclarationData> font_face =
      parser_.ParseFontFaceDeclarationList(
          "src: local(Roboto), "
          "url(../assets/icons.ttf) format('truetype');",
          source_location_);

  scoped_refptr<cssom::PropertyListValue> src_list =
      dynamic_cast<cssom::PropertyListValue*>(font_face->src().get());
  ASSERT_TRUE(src_list);
  EXPECT_EQ(src_list->value().size(), 2);

  scoped_refptr<cssom::LocalSrcValue> local_src =
      dynamic_cast<cssom::LocalSrcValue*>(
          src_list->get_item_modulo_size(0).get());
  ASSERT_TRUE(local_src);
  EXPECT_EQ("Roboto", local_src->value());

  scoped_refptr<cssom::UrlSrcValue> url_src = dynamic_cast<cssom::UrlSrcValue*>(
      src_list->get_item_modulo_size(1).get());
  ASSERT_TRUE(url_src);
  EXPECT_EQ("truetype", url_src->format());

  scoped_refptr<cssom::URLValue> url_value =
      dynamic_cast<cssom::URLValue*>(url_src->url().get());
  EXPECT_EQ("../assets/icons.ttf", url_value->value());
}

TEST_F(ParserTest, ParsesFontFaceStyle) {
  scoped_refptr<cssom::CSSFontFaceDeclarationData> font_face =
      parser_.ParseFontFaceDeclarationList("font-style: italic;",
                                           source_location_);
  EXPECT_EQ(cssom::FontStyleValue::GetItalic(), font_face->style());
}

TEST_F(ParserTest, ParsesFontFaceWeight) {
  scoped_refptr<cssom::CSSFontFaceDeclarationData> font_face =
      parser_.ParseFontFaceDeclarationList("font-weight: bold;",
                                           source_location_);
  EXPECT_EQ(cssom::FontWeightValue::GetBoldAka700(), font_face->weight());
}

TEST_F(ParserTest, ParsesFontFaceUnicodeRangeSingleValue) {
  scoped_refptr<cssom::CSSFontFaceDeclarationData> font_face =
      parser_.ParseFontFaceDeclarationList("unicode-range: U+100;",
                                           source_location_);

  scoped_refptr<cssom::PropertyListValue> unicode_range_list =
      dynamic_cast<cssom::PropertyListValue*>(font_face->unicode_range().get());
  ASSERT_TRUE(unicode_range_list);
  EXPECT_EQ(unicode_range_list->value().size(), 1);

  scoped_refptr<cssom::UnicodeRangeValue> unicode_range =
      dynamic_cast<cssom::UnicodeRangeValue*>(
          unicode_range_list->get_item_modulo_size(0).get());
  ASSERT_TRUE(unicode_range);
  ASSERT_TRUE(unicode_range->IsValid());
  EXPECT_EQ(0x100, unicode_range->start());
  EXPECT_EQ(0x100, unicode_range->end());
}

TEST_F(ParserTest, ParsesFontFaceUnicodeRangeSingleRange) {
  scoped_refptr<cssom::CSSFontFaceDeclarationData> font_face =
      parser_.ParseFontFaceDeclarationList("unicode-range: U+100000-10FFFF;",
                                           source_location_);

  scoped_refptr<cssom::PropertyListValue> unicode_range_list =
      dynamic_cast<cssom::PropertyListValue*>(font_face->unicode_range().get());
  ASSERT_TRUE(unicode_range_list);
  EXPECT_EQ(unicode_range_list->value().size(), 1);

  scoped_refptr<cssom::UnicodeRangeValue> unicode_range =
      dynamic_cast<cssom::UnicodeRangeValue*>(
          unicode_range_list->get_item_modulo_size(0).get());
  ASSERT_TRUE(unicode_range);
  ASSERT_TRUE(unicode_range->IsValid());
  EXPECT_EQ(0x100000, unicode_range->start());
  EXPECT_EQ(0x10FFFF, unicode_range->end());
}

TEST_F(ParserTest, ParsesFontFaceUnicodeRangeSingleWildcardRange) {
  scoped_refptr<cssom::CSSFontFaceDeclarationData> font_face =
      parser_.ParseFontFaceDeclarationList("unicode-range: U+???;",
                                           source_location_);

  scoped_refptr<cssom::PropertyListValue> unicode_range_list =
      dynamic_cast<cssom::PropertyListValue*>(font_face->unicode_range().get());
  ASSERT_TRUE(unicode_range_list);
  EXPECT_EQ(unicode_range_list->value().size(), 1);

  scoped_refptr<cssom::UnicodeRangeValue> unicode_range =
      dynamic_cast<cssom::UnicodeRangeValue*>(
          unicode_range_list->get_item_modulo_size(0).get());
  ASSERT_TRUE(unicode_range);
  ASSERT_TRUE(unicode_range->IsValid());
  EXPECT_EQ(0x000, unicode_range->start());
  EXPECT_EQ(0xfff, unicode_range->end());
}

TEST_F(ParserTest, ParsesFontFaceUnicodeRangeEndGreaterThanBegin) {
  scoped_refptr<cssom::CSSFontFaceDeclarationData> font_face =
      parser_.ParseFontFaceDeclarationList("unicode-range: U+100-0;",
                                           source_location_);

  scoped_refptr<cssom::PropertyListValue> unicode_range_list =
      dynamic_cast<cssom::PropertyListValue*>(font_face->unicode_range().get());
  ASSERT_TRUE(unicode_range_list);
  EXPECT_EQ(unicode_range_list->value().size(), 1);

  scoped_refptr<cssom::UnicodeRangeValue> unicode_range =
      dynamic_cast<cssom::UnicodeRangeValue*>(
          unicode_range_list->get_item_modulo_size(0).get());
  ASSERT_TRUE(unicode_range);
  ASSERT_FALSE(unicode_range->IsValid());
  EXPECT_EQ(0x100, unicode_range->start());
  EXPECT_EQ(0x0, unicode_range->end());
}

TEST_F(ParserTest, ParsesFontFaceUnicodeRangeList) {
  scoped_refptr<cssom::CSSFontFaceDeclarationData> font_face =
      parser_.ParseFontFaceDeclarationList(
          "unicode-range: U+10, U+???, U+100000-10ffff;", source_location_);

  scoped_refptr<cssom::PropertyListValue> unicode_range_list =
      dynamic_cast<cssom::PropertyListValue*>(font_face->unicode_range().get());
  ASSERT_TRUE(unicode_range_list);
  EXPECT_EQ(unicode_range_list->value().size(), 3);

  scoped_refptr<cssom::UnicodeRangeValue> unicode_range1 =
      dynamic_cast<cssom::UnicodeRangeValue*>(
          unicode_range_list->get_item_modulo_size(0).get());
  ASSERT_TRUE(unicode_range1);
  ASSERT_TRUE(unicode_range1->IsValid());
  EXPECT_EQ(0x10, unicode_range1->start());
  EXPECT_EQ(0x10, unicode_range1->end());

  scoped_refptr<cssom::UnicodeRangeValue> unicode_range2 =
      dynamic_cast<cssom::UnicodeRangeValue*>(
          unicode_range_list->get_item_modulo_size(1).get());
  ASSERT_TRUE(unicode_range2);
  ASSERT_TRUE(unicode_range2->IsValid());
  EXPECT_EQ(0x000, unicode_range2->start());
  EXPECT_EQ(0xfff, unicode_range2->end());

  scoped_refptr<cssom::UnicodeRangeValue> unicode_range3 =
      dynamic_cast<cssom::UnicodeRangeValue*>(
          unicode_range_list->get_item_modulo_size(2).get());
  ASSERT_TRUE(unicode_range3);
  ASSERT_TRUE(unicode_range3->IsValid());
  EXPECT_EQ(0x100000, unicode_range3->start());
  EXPECT_EQ(0x10ffff, unicode_range3->end());
}

TEST_F(ParserTest, ParsesEmptyMediaQuery) {
  scoped_refptr<cssom::MediaQuery> media_query =
      parser_.ParseMediaQuery("", source_location_).get();
  ASSERT_TRUE(media_query.get());
  ASSERT_EQ(media_query->media_query(), "");
}

TEST_F(ParserTest, ParsesValidMediaQuery) {
  scoped_refptr<cssom::MediaQuery> media_query =
      parser_.ParseMediaQuery("(max-width: 1024px) and (max-height: 512px)",
                              source_location_)
          .get();
  ASSERT_TRUE(media_query.get());
  // TODO(***REMOVED***): Update when media query serialization is implemented.
  ASSERT_EQ(media_query->media_query(), "");
}

TEST_F(ParserTest, ParsesEmptyMediaList) {
  scoped_refptr<cssom::MediaList> media_list =
      parser_.ParseMediaList("", source_location_).get();
  ASSERT_TRUE(media_list.get());
  ASSERT_EQ(media_list->media_text(), "");
}

TEST_F(ParserTest, ParsesValidMediaList) {
  scoped_refptr<cssom::MediaList> media_list =
      parser_.ParseMediaList("(max-width: 1024px), (max-height: 512px)",
                             source_location_)
          .get();
  ASSERT_TRUE(media_list.get());
  ASSERT_EQ(media_list->length(), 2);

  // TODO(***REMOVED***): Update when media query serialization is implemented.
  ASSERT_EQ(media_list->media_text(), ", ");
}

}  // namespace css_parser
}  // namespace cobalt
