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

#ifndef CSSOM_PROPERTY_VALUE_VISITOR_H_
#define CSSOM_PROPERTY_VALUE_VISITOR_H_

#include "base/compiler_specific.h"
#include "base/logging.h"

namespace cobalt {
namespace cssom {

class FontWeightValue;
class KeywordValue;
class LengthValue;
class NumberValue;
class PropertyNameListValue;
class RGBAColorValue;
class StringValue;
class TimeListValue;
class TransformListValue;

// Type-safe branching on a class hierarchy of CSS property values,
// implemented after a classical GoF pattern (see
// http://en.wikipedia.org/wiki/Visitor_pattern#Java_example).
class PropertyValueVisitor {
 public:
  virtual void VisitFontWeight(FontWeightValue* font_weight_value) = 0;
  virtual void VisitKeyword(KeywordValue* keyword_value) = 0;
  virtual void VisitLength(LengthValue* length_value) = 0;
  virtual void VisitNumber(NumberValue* number_value) = 0;
  virtual void VisitPropertyNameList(
      PropertyNameListValue* property_name_list_value) = 0;
  virtual void VisitRGBAColor(RGBAColorValue* color_value) = 0;
  virtual void VisitString(StringValue* string_value) = 0;
  virtual void VisitTimeList(TimeListValue* time_list_value) = 0;
  virtual void VisitTransformList(TransformListValue* transform_list_value) = 0;

 protected:
  ~PropertyValueVisitor() {}
};

// A convenience class that implements PropertyValueVisitor with NOTREACHED()
// for each method, thus one can derive from this class, implement only the
// value types that they care about, and then every other value type will
// result in an error.
class NotReachedPropertyValueVisitor : public PropertyValueVisitor {
 public:
  void VisitFontWeight(FontWeightValue* /*font_weight_value*/) OVERRIDE {
    NOTREACHED();
  }
  void VisitKeyword(KeywordValue* /*keyword_value*/) OVERRIDE {
    NOTREACHED();
  }
  void VisitLength(LengthValue* /*length_value*/) OVERRIDE {
    NOTREACHED();
  }
  void VisitNumber(NumberValue* /*number_value*/) OVERRIDE {
    NOTREACHED();
  }
  void VisitPropertyNameList(
      PropertyNameListValue* /*property_name_list_value*/) OVERRIDE {
    NOTREACHED();
  }
  void VisitRGBAColor(RGBAColorValue* /*color_value*/) OVERRIDE {
    NOTREACHED();
  }
  void VisitString(StringValue* /*string_value*/) OVERRIDE {
    NOTREACHED();
  }
  void VisitTimeList(TimeListValue* /*time_list_value*/) OVERRIDE {
    NOTREACHED();
  }
  void VisitTransformList(
      TransformListValue* /*transform_list_value*/) OVERRIDE {
    NOTREACHED();
  }
};

}  // namespace cssom
}  // namespace cobalt

#endif  // CSSOM_PROPERTY_VALUE_VISITOR_H_
