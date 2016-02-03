// Copyright 2016 Google Inc. All Rights Reserved.
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

#include <limits>

#include "starboard/double.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace starboard {
namespace nplb {
namespace {

TEST(SbDoubleExponentTest, SunnyDay) {
  EXPECT_EQ(1.0, SbDoubleExponent(1.0, 1.0));
  EXPECT_EQ(2.0, SbDoubleExponent(2.0, 1.0));
  EXPECT_EQ(1.0, SbDoubleExponent(1.0, 2.0));
  EXPECT_EQ(256.0, SbDoubleExponent(2.0, 8.0));
  EXPECT_EQ(0.5, SbDoubleExponent(2.0, -1.0));

  double nan = std::numeric_limits<double>::quiet_NaN();
  EXPECT_EQ(1.0, SbDoubleExponent(1.0, nan));
  EXPECT_TRUE(SbDoubleIsNan(SbDoubleExponent(2.0, nan)));
  EXPECT_TRUE(SbDoubleIsNan(SbDoubleExponent(nan, 1.0)));
  EXPECT_TRUE(SbDoubleIsNan(SbDoubleExponent(nan, 2.0)));

  double infinity = std::numeric_limits<double>::infinity();
  EXPECT_EQ(0.0, SbDoubleExponent(0.5, infinity));
  EXPECT_EQ(1.0, SbDoubleExponent(1.0, infinity));
  double d = SbDoubleExponent(2.0, infinity);
  EXPECT_FALSE(SbDoubleIsFinite(&d));
}

}  // namespace
}  // namespace nplb
}  // namespace starboard
