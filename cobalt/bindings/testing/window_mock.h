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

#ifndef COBALT_BINDINGS_TESTING_WINDOW_MOCK_H_
#define COBALT_BINDINGS_TESTING_WINDOW_MOCK_H_

#include "cobalt/bindings/testing/window.h"
#include "testing/gmock/include/gmock/gmock.h"

namespace cobalt {
namespace bindings {
namespace testing {

class WindowMock : public Window {
 public:
  MOCK_METHOD0(WindowOperation, void());
  MOCK_METHOD0(window_property, std::string());
  MOCK_METHOD1(set_window_property, void(const std::string&));

  MOCK_METHOD0(ParentOperation, void());
};

}  // namespace testing
}  // namespace bindings
}  // namespace cobalt

#endif  // COBALT_BINDINGS_TESTING_WINDOW_MOCK_H_
