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

#include "cobalt/cssom/css_declared_style_data.h"

#include <limits>

#include "base/string_util.h"
#include "cobalt/cssom/keyword_value.h"
#include "cobalt/cssom/length_value.h"

namespace cobalt {
namespace cssom {

CSSDeclaredStyleData::CSSDeclaredStyleData() {}

unsigned int CSSDeclaredStyleData::length() const {
  size_t declared_property_values_size = declared_property_values_.size();
  DCHECK_LE(declared_property_values_size,
            std::numeric_limits<unsigned int>::max());
  return static_cast<unsigned int>(declared_property_values_size);
}

const char* CSSDeclaredStyleData::Item(unsigned int index) const {
  if (index >= declared_property_values_.size()) return NULL;
  PropertyValues::const_iterator property_value_iterator =
      declared_property_values_.begin();
  while (index--) {
    ++property_value_iterator;
  }
  return GetPropertyName(property_value_iterator->first);
}

scoped_refptr<PropertyValue> CSSDeclaredStyleData::GetPropertyValue(
    PropertyKey key) const {
  DCHECK_GT(key, kNoneProperty);
  DCHECK_LE(key, kMaxLonghandPropertyKey);
  if (declared_properties_[key]) {
    return declared_property_values_.find(key)->second;
  }
  return NULL;
}

void CSSDeclaredStyleData::ClearPropertyValueAndImportance(PropertyKey key) {
  DCHECK_NE(kNoneProperty, key);
  if (IsShorthandProperty(key)) {
    const LonghandPropertySet& longhand_properties =
        ExpandShorthandProperty(key);
    for (LonghandPropertySet::const_iterator iter = longhand_properties.begin();
         iter != longhand_properties.end(); ++iter) {
      ClearPropertyValueAndImportanceForLonghandProperty(*iter);
    }
  } else {
    ClearPropertyValueAndImportanceForLonghandProperty(key);
  }
}

void CSSDeclaredStyleData::ClearPropertyValueAndImportanceForLonghandProperty(
    PropertyKey key) {
  SetPropertyValueAndImportance(key, NULL, false);
}

void CSSDeclaredStyleData::SetPropertyValueAndImportance(
    PropertyKey key, const scoped_refptr<PropertyValue>& property_value,
    bool important) {
  DCHECK_GT(key, kNoneProperty);
  DCHECK_LE(key, kMaxLonghandPropertyKey);
  if (property_value) {
    declared_properties_.set(key, true);
    declared_property_values_[key] = property_value;
  } else if (declared_properties_[key]) {
    declared_properties_.set(key, false);
    declared_property_values_.erase(key);
  }
  important_properties_.set(key, important);
}

void CSSDeclaredStyleData::AssignFrom(const CSSDeclaredStyleData& rhs) {
  declared_properties_ = rhs.declared_properties_;
  declared_property_values_ = rhs.declared_property_values_;
  important_properties_ = rhs.important_properties_;
}

std::string CSSDeclaredStyleData::GetPropertyValueString(
    const PropertyKey key) const {
  DCHECK_GT(key, kNoneProperty);
  DCHECK_LE(key, kMaxLonghandPropertyKey);
  if (declared_properties_[key]) {
    return declared_property_values_.find(key)->second->ToString();
  }
  return "";
}

std::string CSSDeclaredStyleData::SerializeCSSDeclarationBlock() const {
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

bool CSSDeclaredStyleData::operator==(const CSSDeclaredStyleData& that) const {
  DCHECK_EQ(declared_properties_.size(), that.declared_properties_.size());
  DCHECK_EQ(declared_properties_.size(), important_properties_.size());
  DCHECK_EQ(that.declared_properties_.size(),
            that.important_properties_.size());

  for (size_t i = 0; i < declared_properties_.size(); ++i) {
    if (declared_properties_[i] != that.declared_properties_[i]) {
      return false;
    }
    if (!declared_properties_[i]) {
      continue;
    }
    if (important_properties_[i] != that.important_properties_[i]) {
      return false;
    }
    PropertyKey key = static_cast<PropertyKey>(i);
    if (!declared_property_values_.find(key)->second->Equals(
            *that.declared_property_values_.find(key)->second)) {
      return false;
    }
  }
  return true;
}

}  // namespace cssom
}  // namespace cobalt
