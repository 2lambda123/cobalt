// Copyright 2015 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "starboard/string.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace starboard {
namespace nplb {
namespace {

TEST(SbStringCompareNoCaseTest, SunnyDaySelf) {
  const char kString[] = "0123456789";
  EXPECT_EQ(0, SbStringCompareNoCase(kString, kString));
  EXPECT_EQ(0, SbStringCompareNoCase("", ""));
}

TEST(SbStringCompareNoCaseTest, SunnyDayEmptyLessThanNotEmpty) {
  const char kString[] = "0123456789";
  EXPECT_GT(0, SbStringCompareNoCase("", kString));
}

TEST(SbStringCompareNoCaseTest, SunnyDayCase) {
  const char kString1[] = "aBcDeFgHiJkLmNoPqRsTuVwXyZ";
  const char kString2[] = "AbCdEfGhIjKlMnOpQrStUvWxYz";
  EXPECT_EQ(0, SbStringCompareNoCase(kString1, kString2));
  EXPECT_EQ(0, SbStringCompareNoCase(kString2, kString1));
}

}  // namespace
}  // namespace nplb
}  // namespace starboard
