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

#include "starboard/file.h"
#include "starboard/nplb/file_helpers.h"
#include "starboard/system.h"
#include "testing/gtest/include/gtest/gtest.h"

using starboard::nplb::ScopedRandomFile;

namespace {

TEST(SbSystemGetLastErrorTest, SunnyDay) {
  // Opening a non-existant file should generate an error on all platforms.
  ScopedRandomFile random_file(ScopedRandomFile::kDontCreate);
  SbFile file = SbFileOpen(random_file.filename().c_str(),
                           kSbFileOpenOnly | kSbFileRead, NULL, NULL);

  SbSystemError error = SbSystemGetLastError();
  EXPECT_NE(0, error);
}

}  // namespace
