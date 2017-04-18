/*
 * Copyright 2017 Google Inc. All Rights Reserved.
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

#include "cobalt/browser/memory_settings/memory_settings.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include "base/memory/scoped_ptr.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace cobalt {
namespace browser {
namespace memory_settings {

TEST(IntSetting, ParseFromString) {
  scoped_ptr<IntSetting> int_setting(new IntSetting("dummy"));
  EXPECT_EQ(std::string("dummy"), int_setting->name());
  ASSERT_TRUE(int_setting->TryParseValue(MemorySetting::kCmdLine, "123"));
  EXPECT_EQ(123, int_setting->value());
  EXPECT_EQ(MemorySetting::kCmdLine, int_setting->source_type());
  EXPECT_EQ(std::string("123"), int_setting->ValueToString());
}

TEST(DimensionSetting, ParseFromString) {
  scoped_ptr<DimensionSetting> rect_setting(new DimensionSetting("dummy"));
  ASSERT_TRUE(
      rect_setting->TryParseValue(MemorySetting::kCmdLine, "1234x5678"));
  EXPECT_EQ(math::Size(1234, 5678), rect_setting->value());
  EXPECT_EQ(MemorySetting::kCmdLine, rect_setting->source_type());
  EXPECT_EQ(std::string("1234x5678"), rect_setting->ValueToString());
}

}  // namespace memory_settings
}  // namespace browser
}  // namespace cobalt
