/*
 * Copyright 2016 Google Inc. All Rights Reserved.
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

#include "cobalt/cssom/css_computed_style_data.h"

#include <limits>

#include "base/lazy_instance.h"
#include "base/string_util.h"
#include "cobalt/cssom/keyword_value.h"
#include "cobalt/cssom/length_value.h"

namespace cobalt {
namespace cssom {

CSSComputedStyleData::CSSComputedStyleData()
    : has_inherited_properties_(false) {}

unsigned int CSSComputedStyleData::length() const {
  // If the style has an ancestor set, we'll have to take inherited property
  // values from ancestors into account.
  DCHECK(!ancestor_computed_style_)
      << "length() Not implemented for non-inline styles";
  size_t declared_property_values_size = declared_property_values_.size();
  DCHECK_LE(declared_property_values_size,
            std::numeric_limits<unsigned int>::max());
  return static_cast<unsigned int>(declared_property_values_size);
}

const char* CSSComputedStyleData::Item(unsigned int index) const {
  // If the style has an ancestor set, we'll have to take inherited property
  // values from ancestors into account.
  DCHECK(!ancestor_computed_style_)
      << "Item() Not implemented for non-inline styles";
  if (index >= declared_property_values_.size()) return NULL;
  PropertyValues::const_iterator property_value_iterator =
      declared_property_values_.begin();
  while (index--) {
    ++property_value_iterator;
  }
  return GetPropertyName(property_value_iterator->first);
}

const scoped_refptr<PropertyValue>&
CSSComputedStyleData::GetPropertyValueReference(PropertyKey key) const {
  DCHECK_GT(key, kNoneProperty);
  DCHECK_LE(key, kMaxLonghandPropertyKey);
  if (declared_properties_[key]) {
    return declared_property_values_.find(key)->second;
  }
  if (ancestor_computed_style_ &&
      GetPropertyInheritance(key) == kInheritedYes) {
    return ancestor_computed_style_->GetPropertyValueReference(key);
  }
  return GetComputedInitialValue(key);
}

scoped_refptr<PropertyValue>&
CSSComputedStyleData::GetDeclaredPropertyValueReference(PropertyKey key) {
  DCHECK(declared_properties_[key]);
  return declared_property_values_.find(key)->second;
}

namespace {
struct NonTrivialStaticFields {
  NonTrivialStaticFields()
      : block_keyword_value(cssom::KeywordValue::GetBlock()),
        zero_length_value(new cssom::LengthValue(0.0f, cssom::kPixelsUnit)) {}

  const scoped_refptr<cssom::PropertyValue> block_keyword_value;
  const scoped_refptr<cssom::PropertyValue> zero_length_value;

 private:
  DISALLOW_COPY_AND_ASSIGN(NonTrivialStaticFields);
};

base::LazyInstance<NonTrivialStaticFields> non_trivial_static_fields =
    LAZY_INSTANCE_INITIALIZER;
}  // namespace

const scoped_refptr<PropertyValue>&
CSSComputedStyleData::GetComputedInitialValue(PropertyKey key) const {
  switch (key) {
    case kBorderTopColorProperty:
    case kBorderRightColorProperty:
    case kBorderBottomColorProperty:
    case kBorderLeftColorProperty:
      // Note that border color is not inherited.
      // The initial value of border color is 'currentColor' which means the
      // border color is the same as the value of the 'color' property.
      //    https://www.w3.org/TR/css3-color/#currentcolor
      return color();

    case kBorderTopWidthProperty:
    case kBorderRightWidthProperty:
    case kBorderBottomWidthProperty:
    case kBorderLeftWidthProperty:
      // If the border style is 'none' or 'hidden', border width would be 0.
      //   https://www.w3.org/TR/css3-background/#border-width
      if (IsBorderStyleNoneOrHiddenForAnEdge(key)) {
        return non_trivial_static_fields.Get().zero_length_value;
      }
      break;

    case kDisplayProperty:
      // The initial value of "display" (inline) become "block" if "position" is
      // "absolute" or "fixed".
      //    https://www.w3.org/TR/CSS21/visuren.html#dis-pos-flo
      if (position() == cssom::KeywordValue::GetAbsolute() ||
          position() == cssom::KeywordValue::GetFixed()) {
        return non_trivial_static_fields.Get().block_keyword_value;
      }
      break;

    case kAllProperty:
    case kAnimationDelayProperty:
    case kAnimationDirectionProperty:
    case kAnimationDurationProperty:
    case kAnimationFillModeProperty:
    case kAnimationIterationCountProperty:
    case kAnimationNameProperty:
    case kAnimationProperty:
    case kAnimationTimingFunctionProperty:
    case kBackgroundColorProperty:
    case kBackgroundImageProperty:
    case kBackgroundPositionProperty:
    case kBackgroundProperty:
    case kBackgroundRepeatProperty:
    case kBackgroundSizeProperty:
    case kBorderBottomStyleProperty:
    case kBorderBottomProperty:
    case kBorderColorProperty:
    case kBorderLeftProperty:
    case kBorderLeftStyleProperty:
    case kBorderProperty:
    case kBorderRightProperty:
    case kBorderRightStyleProperty:
    case kBorderStyleProperty:
    case kBorderTopProperty:
    case kBorderTopStyleProperty:
    case kBorderWidthProperty:
    case kBorderRadiusProperty:
    case kBottomProperty:
    case kBoxShadowProperty:
    case kColorProperty:
    case kContentProperty:
    case kFontFamilyProperty:
    case kFontProperty:
    case kFontStyleProperty:
    case kFontWeightProperty:
    case kFontSizeProperty:
    case kHeightProperty:
    case kLeftProperty:
    case kLineHeightProperty:
    case kMarginBottomProperty:
    case kMarginLeftProperty:
    case kMarginProperty:
    case kMarginRightProperty:
    case kMarginTopProperty:
    case kMaxHeightProperty:
    case kMaxWidthProperty:
    case kMinHeightProperty:
    case kMinWidthProperty:
    case kNoneProperty:
    case kOpacityProperty:
    case kOverflowProperty:
    case kOverflowWrapProperty:
    case kPaddingBottomProperty:
    case kPaddingLeftProperty:
    case kPaddingProperty:
    case kPaddingRightProperty:
    case kPaddingTopProperty:
    case kPositionProperty:
    case kRightProperty:
    case kSrcProperty:
    case kTextAlignProperty:
    case kTextIndentProperty:
    case kTextOverflowProperty:
    case kTextShadowProperty:
    case kTextTransformProperty:
    case kTopProperty:
    case kTransformOriginProperty:
    case kTransformProperty:
    case kTransitionDelayProperty:
    case kTransitionDurationProperty:
    case kTransitionProperty:
    case kTransitionPropertyProperty:
    case kTransitionTimingFunctionProperty:
    case kUnicodeRangeProperty:
    case kVerticalAlignProperty:
    case kVisibilityProperty:
    case kWhiteSpaceProperty:
    case kWidthProperty:
    case kWordWrapProperty:
    case kZIndexProperty:
      break;
  }

  return GetPropertyInitialValue(key);
}

bool CSSComputedStyleData::IsBorderStyleNoneOrHiddenForAnEdge(
    PropertyKey key) const {
  scoped_refptr<PropertyValue> border_style;
  if (key == kBorderTopWidthProperty) {
    border_style = border_top_style();
  } else if (key == kBorderRightWidthProperty) {
    border_style = border_right_style();
  } else if (key == kBorderBottomWidthProperty) {
    border_style = border_bottom_style();
  } else {
    DCHECK_EQ(key, kBorderLeftWidthProperty);
    border_style = border_left_style();
  }

  if (border_style == cssom::KeywordValue::GetNone() ||
      border_style == cssom::KeywordValue::GetHidden()) {
    return true;
  }

  return false;
}

void CSSComputedStyleData::SetPropertyValue(
    const PropertyKey key, const scoped_refptr<PropertyValue>& value) {
  DCHECK_GT(key, kNoneProperty);
  DCHECK_LE(key, kMaxLonghandPropertyKey);

  if (value) {
    declared_properties_.set(key, true);
    declared_property_values_[key] = value;
    has_inherited_properties_ |= GetPropertyInheritance(key) == kInheritedYes;
  } else if (declared_properties_[key]) {
    declared_properties_.set(key, false);
    declared_property_values_.erase(key);
  }
}

void CSSComputedStyleData::AssignFrom(const CSSComputedStyleData& rhs) {
  declared_properties_ = rhs.declared_properties_;
  declared_property_values_ = rhs.declared_property_values_;
  has_inherited_properties_ = rhs.has_inherited_properties_;
}

std::string CSSComputedStyleData::SerializeCSSDeclarationBlock() const {
  std::string serialized_text;
  for (PropertyValues::const_iterator property_value_iterator =
           declared_property_values_.begin();
       property_value_iterator != declared_property_values_.end();
       ++property_value_iterator) {
    if (!serialized_text.empty()) {
      serialized_text.push_back(' ');
    }
    serialized_text.append(GetPropertyName(property_value_iterator->first));
    serialized_text.append(": ");
    serialized_text.append(property_value_iterator->second->ToString());
    serialized_text.push_back(';');
  }
  return serialized_text;
}

void CSSComputedStyleData::SetParentComputedStyle(
    const scoped_refptr<const CSSComputedStyleData>& parent_computed_style) {
  if (parent_computed_style->has_inherited_properties_) {
    ancestor_computed_style_ = parent_computed_style;
  } else {
    // If the parent style does not have any inherited properties, we will never
    // need to refer to it for inherited properties, so we refer to the parent
    // style's ancestor instead. This gives the number of ancestor dereferences
    // needed to find the value of an inherited property an upper bound equal to
    // the total number of inheritable properties.
    ancestor_computed_style_ = parent_computed_style->ancestor_computed_style_;
  }
}

bool CSSComputedStyleData::operator==(const CSSComputedStyleData& that) const {
  DCHECK_EQ(declared_properties_.size(), that.declared_properties_.size());

  for (size_t i = 0; i < declared_properties_.size(); ++i) {
    if (declared_properties_[i] != that.declared_properties_[i]) {
      return false;
    }
    if (!declared_properties_[i]) {
      continue;
    }
    PropertyKey key = static_cast<PropertyKey>(i);
    if (!declared_property_values_.find(key)->second->Equals(
            *that.declared_property_values_.find(key)->second)) {
      return false;
    }
  }

  if (ancestor_computed_style_ == that.ancestor_computed_style_) {
    return true;
  }

  if (ancestor_computed_style_ == NULL ||
      that.ancestor_computed_style_ == NULL) {
    return false;
  }

  return *ancestor_computed_style_ == *that.ancestor_computed_style_;
}

}  // namespace cssom
}  // namespace cobalt
