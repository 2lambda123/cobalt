/*
 * Copyright 2015 Google Inc. All Rights Reserved.
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

#include "cobalt/bindings/testing/bindings_test_base.h"
#include "cobalt/bindings/testing/callback_function_interface.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using ::testing::Invoke;
using ::testing::InSequence;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::_;

namespace cobalt {
namespace bindings {
namespace testing {

namespace {
typedef InterfaceBindingsTest<CallbackFunctionInterface> CallbackFunctionTest;

template <typename T>
class ScriptObjectOwner {
 public:
  explicit ScriptObjectOwner(script::Wrappable* owner) : owner_(owner) {}
  void TakeOwnership(const T& function) {
    if (!function.IsNull()) {
      reference_.emplace(owner_, function);
    } else {
      reference_ = base::nullopt;
    }
  }
  typename T::Reference& reference() { return reference_.value(); }
  bool IsSet() { return static_cast<bool>(reference_); }

 private:
  script::Wrappable* owner_;
  base::optional<typename T::Reference> reference_;
};
}  // namespace

TEST_F(CallbackFunctionTest, ScriptCallbackCanBeCalledFromC) {
  std::string result;
  EXPECT_TRUE(EvaluateScript("var was_called = false;", NULL));

  typedef ScriptObjectOwner<script::ScriptObject<
      CallbackFunctionInterface::VoidFunction> > FunctionOwner;
  FunctionOwner function_owner(&test_mock());
  EXPECT_CALL(test_mock(), TakesVoidFunction(_))
      .WillOnce(Invoke(&function_owner, &FunctionOwner::TakeOwnership));
  EXPECT_TRUE(EvaluateScript(
      "test.takesVoidFunction(function() { was_called = true });", NULL));
  ASSERT_TRUE(function_owner.IsSet());
  function_owner.reference().value().Run();
  EXPECT_TRUE(EvaluateScript("was_called;", &result));
  EXPECT_STREQ("true", result.c_str());
}

TEST_F(CallbackFunctionTest, ScriptFunctionIsNotGarbageCollected) {
  std::string result;
  EXPECT_TRUE(EvaluateScript("var num_called = 0;", NULL));

  typedef ScriptObjectOwner<script::ScriptObject<
      CallbackFunctionInterface::VoidFunction> > FunctionOwner;
  FunctionOwner function_owner(&test_mock());
  EXPECT_CALL(test_mock(), TakesVoidFunction(_))
      .WillOnce(Invoke(&function_owner, &FunctionOwner::TakeOwnership));
  EXPECT_TRUE(EvaluateScript(
      "test.takesVoidFunction(function() { num_called++ });", NULL));
  ASSERT_TRUE(function_owner.IsSet());

  function_owner.reference().value().Run();
  EXPECT_TRUE(EvaluateScript("num_called == 1;", &result));
  EXPECT_STREQ("true", result.c_str());

  engine_->CollectGarbage();

  // Call it once more to ensure the function has not been garbage collected.
  function_owner.reference().value().Run();
  EXPECT_TRUE(EvaluateScript("num_called == 2;", &result));
  EXPECT_STREQ("true", result.c_str());
}

TEST_F(CallbackFunctionTest, CallbackWithOneParameter) {
  std::string result;
  EXPECT_TRUE(EvaluateScript("var callback_value;", NULL));

  // Store a handle to the callback passed from script.
  typedef ScriptObjectOwner<script::ScriptObject<
      CallbackFunctionInterface::FunctionWithOneParameter> > FunctionOwner;
  FunctionOwner function_owner(&test_mock());
  EXPECT_CALL(test_mock(), TakesFunctionWithOneParameter(_))
      .WillOnce(Invoke(&function_owner, &FunctionOwner::TakeOwnership));
  EXPECT_TRUE(EvaluateScript(
      "test.takesFunctionWithOneParameter(function(value) { "
      "callback_value = value });",
      NULL));
  ASSERT_TRUE(function_owner.IsSet());

  // Run the callback, and check that the value was passed through to script.
  function_owner.reference().value().Run(5);
  EXPECT_TRUE(EvaluateScript("callback_value;", &result));
  EXPECT_STREQ("5", result.c_str());
}

TEST_F(CallbackFunctionTest, CallbackWithSeveralParameters) {
  std::string result;
  EXPECT_TRUE(EvaluateScript("var value1;", NULL));
  EXPECT_TRUE(EvaluateScript("var value2;", NULL));
  EXPECT_TRUE(EvaluateScript("var value3;", NULL));

  // Store a handle to the callback passed from script.
  typedef ScriptObjectOwner<script::ScriptObject<
      CallbackFunctionInterface::FunctionWithSeveralParameters> > FunctionOwner;
  FunctionOwner function_owner(&test_mock());
  EXPECT_CALL(test_mock(), TakesFunctionWithSeveralParameters(_))
      .WillOnce(Invoke(&function_owner, &FunctionOwner::TakeOwnership));
  EXPECT_TRUE(EvaluateScript(
      "test.takesFunctionWithSeveralParameters("
      "function(param1, param2, param3) { "
      "value1 = param1; value2 = param2; value3 = param3; });",
      NULL));
  ASSERT_TRUE(function_owner.IsSet());

  // Execute the callback
  scoped_refptr<ArbitraryInterface> some_object =
      new ::testing::StrictMock<ArbitraryInterface>();
  function_owner.reference().value().Run(3.14, "some string", some_object);

  // Verify that each parameter got passed to script as expected.
  EXPECT_TRUE(EvaluateScript("value1;", &result));
  EXPECT_STREQ("3.14", result.c_str());

  EXPECT_TRUE(EvaluateScript("value2;", &result));
  EXPECT_STREQ("some string", result.c_str());

  EXPECT_CALL((*some_object.get()), ArbitraryFunction());
  EXPECT_TRUE(EvaluateScript("value3.arbitraryFunction();", NULL));
}

TEST_F(CallbackFunctionTest, CallbackWithNullableParameters) {
  std::string result;
  EXPECT_TRUE(EvaluateScript("var value1;", NULL));
  EXPECT_TRUE(EvaluateScript("var value2;", NULL));
  EXPECT_TRUE(EvaluateScript("var value3;", NULL));

  // Store a handle to the callback passed from script.
  typedef ScriptObjectOwner<script::ScriptObject<
      CallbackFunctionInterface::FunctionWithNullableParameters> >
      FunctionOwner;
  FunctionOwner function_owner(&test_mock());
  EXPECT_CALL(test_mock(), TakesFunctionWithNullableParameters(_))
      .WillOnce(Invoke(&function_owner, &FunctionOwner::TakeOwnership));
  EXPECT_TRUE(EvaluateScript(
      "test.takesFunctionWithNullableParameters("
      "function(param1, param2, param3) { "
      "value1 = param1; value2 = param2; value3 = param3; });",
      NULL));
  ASSERT_TRUE(function_owner.IsSet());

  // Execute the callback
  function_owner.reference().value().Run(base::nullopt, base::nullopt, NULL);

  // Verify that each parameter got passed to script as expected.
  EXPECT_TRUE(EvaluateScript("value1;", &result));
  EXPECT_STREQ("null", result.c_str());

  EXPECT_TRUE(EvaluateScript("value2;", &result));
  EXPECT_STREQ("null", result.c_str());

  EXPECT_TRUE(EvaluateScript("value3;", &result));
  EXPECT_STREQ("null", result.c_str());
}

TEST_F(CallbackFunctionTest, CallbackAttribute) {
  InSequence in_sequence_dummy;

  std::string result;
  EXPECT_TRUE(EvaluateScript("var num_called = 0;", NULL));

  typedef ScriptObjectOwner<script::ScriptObject<
      CallbackFunctionInterface::VoidFunction> > FunctionOwner;
  FunctionOwner function_owner(&test_mock());
  EXPECT_CALL(test_mock(), set_callback_attribute(_))
      .WillOnce(Invoke(&function_owner, &FunctionOwner::TakeOwnership));
  EXPECT_TRUE(EvaluateScript(
      "var callback_function = function() { ++num_called; };", NULL));
  EXPECT_TRUE(
      EvaluateScript("test.callbackAttribute = callback_function;", NULL));
  ASSERT_TRUE(function_owner.IsSet());

  // Execute the callback
  function_owner.reference().value().Run();
  EXPECT_TRUE(EvaluateScript("num_called;", &result));
  EXPECT_STREQ("1", result.c_str());

  // Check that the getter references the same object
  EXPECT_CALL(test_mock(), callback_attribute())
      .WillOnce(Return(&function_owner.reference().referenced_object()));
  EXPECT_TRUE(
      EvaluateScript("test.callbackAttribute === callback_function;", &result));
  EXPECT_STREQ("true", result.c_str());

  // Get the callback and execute it
  EXPECT_CALL(test_mock(), callback_attribute())
      .WillOnce(Return(&function_owner.reference().referenced_object()));
  EXPECT_TRUE(
      EvaluateScript("var callback_function2 = test.callbackAttribute;", NULL));
  EXPECT_TRUE(EvaluateScript("callback_function2();", NULL));
  EXPECT_TRUE(EvaluateScript("num_called;", &result));
  EXPECT_STREQ("2", result.c_str());
}

TEST_F(CallbackFunctionTest, SetNullableCallbackAttribute) {
  InSequence in_sequence_dummy;

  std::string result;
  typedef ScriptObjectOwner<script::ScriptObject<
      CallbackFunctionInterface::VoidFunction> > FunctionOwner;
  FunctionOwner function_owner(&test_mock());
  EXPECT_CALL(test_mock(), set_nullable_callback_attribute(_))
      .WillOnce(Invoke(&function_owner, &FunctionOwner::TakeOwnership));
  EXPECT_TRUE(EvaluateScript(
      "test.nullableCallbackAttribute = function() {  };", NULL));
  EXPECT_TRUE(function_owner.IsSet());

  EXPECT_CALL(test_mock(), set_nullable_callback_attribute(_))
      .WillOnce(Invoke(&function_owner, &FunctionOwner::TakeOwnership));
  EXPECT_TRUE(EvaluateScript("test.nullableCallbackAttribute = null;", NULL));
  EXPECT_FALSE(function_owner.IsSet());
}

TEST_F(CallbackFunctionTest, GetNullableCallbackAttribute) {
  InSequence in_sequence_dummy;

  std::string result;
  EXPECT_CALL(test_mock(), nullable_callback_attribute())
      .WillOnce(Return(static_cast<script::ScriptObject<
                           CallbackFunctionInterface::VoidFunction>*>(NULL)));
  EXPECT_TRUE(
      EvaluateScript("test.nullableCallbackAttribute == null;", &result));
  EXPECT_STREQ("true", result.c_str());
}

}  // namespace testing
}  // namespace bindings
}  // namespace cobalt
