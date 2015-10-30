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

#include "starboard/system.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

#if SB_IS(COMPILER_MSVC)
#pragma optimize("", off)
#elif SB_IS(COMPILER_GCC)
#pragma GCC push_options
#pragma GCC optimize("O0")
#endif

int GetStackWithAnExtraFrame(void **out_stack, int stack_size) {
  return SbSystemGetStack(out_stack, stack_size);
}

void WowThatsADeepStack() {
  void *stack1[10] = {0};
  void *stack2[10] = {0};
  EXPECT_LT(0, SbSystemGetStack(stack1, SB_ARRAY_SIZE_INT(stack1)));
  EXPECT_LT(1, GetStackWithAnExtraFrame(stack2, SB_ARRAY_SIZE_INT(stack2)));
  EXPECT_EQ(stack1[3], stack2[4]);
}

void EvenDeeper() {
  WowThatsADeepStack();
}

void Deeper() {
  EvenDeeper();
}

void Deep() {
  Deeper();
}

#if SB_IS(COMPILER_MSVC)
#pragma optimize("", on)
#elif SB_IS(COMPILER_GCC)
#pragma GCC pop_options
#endif

TEST(SbSystemGetStackTest, SunnyDay) {
  // Ensure we have more entries than actual stack frames, to check that we have
  // the correct target positioning within the destination array.
  void *stack[1024] = {0};
  int count = SbSystemGetStack(&stack[2], SB_ARRAY_SIZE_INT(stack) - 2);
  EXPECT_LT(0, count);
  EXPECT_GE(SB_ARRAY_SIZE_INT(stack) - 2, count);

  // The target should be filled in from the front of the destination array
  // towards the back.
  EXPECT_NE(static_cast<void *>(NULL), stack[2]);

  // Ensure no data before the given entry is modified.
  EXPECT_EQ(static_cast<void *>(NULL), stack[0]);
  EXPECT_EQ(static_cast<void *>(NULL), stack[1]);
}

TEST(SbSystemGetStackTest, SunnyDayShortStack) {
  // Ensure we have fewer entries than actual stack frames.
  void *stack[2] = {0};
  int count = SbSystemGetStack(stack, SB_ARRAY_SIZE_INT(stack));
  EXPECT_LT(0, count);
  EXPECT_GE(SB_ARRAY_SIZE_INT(stack), count);
  EXPECT_NE(static_cast<void *>(NULL), stack[0]);
}

TEST(SbSystemGetStackTest, SunnyDayNoStack) {
  void *stack[1] = {0};
  int count = SbSystemGetStack(stack, 0);
  EXPECT_EQ(0, count);
  EXPECT_EQ(static_cast<void *>(NULL), stack[0]);
}

TEST(SbSystemGetStackTest, SunnyDayStackDirection) {
  // Make sure there are enough frames for us to look down the stack a ways,
  // without assuming anything about how the test runner calls or compiles these
  // tests.
  Deep();
}

}  // namespace
