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

// This file has been auto-generated by bindings/code_generator_cobalt.py. DO NOT MODIFY!
// Auto-generated from template: bindings/mozjs/templates/interface.cc.template

// clang-format off

#include "MozjsOperationsTestInterface.h"

#include "base/debug/trace_event.h"
#include "cobalt/base/polymorphic_downcast.h"
#include "cobalt/script/global_object_proxy.h"
#include "cobalt/script/opaque_handle.h"
#include "cobalt/script/script_object.h"
#include "MozjsArbitraryInterface.h"
#include "cobalt/bindings/testing/arbitrary_interface.h"

#include "base/lazy_instance.h"
#include "cobalt/script/mozjs/conversion_helpers.h"
#include "cobalt/script/mozjs/mozjs_exception_state.h"
#include "cobalt/script/mozjs/mozjs_global_object_proxy.h"
#include "cobalt/script/mozjs/mozjs_object_handle_holder.h"
#include "cobalt/script/mozjs/mozjs_callback_function.h"
#include "cobalt/script/mozjs/type_traits.h"
#include "cobalt/script/mozjs/wrapper_factory.h"
#include "cobalt/script/mozjs/wrapper_private.h"
#include "third_party/mozjs/js/src/jsapi.h"
#include "third_party/mozjs/js/src/jsfriendapi.h"

namespace {
using cobalt::bindings::testing::OperationsTestInterface;
using cobalt::bindings::testing::MozjsOperationsTestInterface;
using cobalt::bindings::testing::ArbitraryInterface;
using cobalt::bindings::testing::MozjsArbitraryInterface;
using cobalt::script::CallbackInterfaceTraits;
using cobalt::script::GlobalObjectProxy;
using cobalt::script::OpaqueHandle;
using cobalt::script::OpaqueHandleHolder;
using cobalt::script::ScriptObject;
using cobalt::script::Wrappable;

using cobalt::script::CallbackFunction;
using cobalt::script::CallbackInterfaceTraits;
using cobalt::script::mozjs::FromJSValue;
using cobalt::script::mozjs::InterfaceData;
using cobalt::script::mozjs::MozjsCallbackFunction;
using cobalt::script::mozjs::MozjsExceptionState;
using cobalt::script::mozjs::MozjsGlobalObjectProxy;
using cobalt::script::mozjs::MozjsObjectHandleHolder;
using cobalt::script::mozjs::ToJSValue;
using cobalt::script::mozjs::TypeTraits;
using cobalt::script::mozjs::WrapperPrivate;
using cobalt::script::mozjs::WrapperFactory;
using cobalt::script::Wrappable;
}  // namespace

namespace cobalt {
namespace bindings {
namespace testing {

namespace {

InterfaceData* CreateCachedInterfaceData() {
  InterfaceData* interface_data = new InterfaceData();
  memset(&interface_data->instance_class_definition, 0,
         sizeof(interface_data->instance_class_definition));
  memset(&interface_data->prototype_class_definition, 0,
         sizeof(interface_data->prototype_class_definition));
  memset(&interface_data->interface_object_class_definition, 0,
         sizeof(interface_data->interface_object_class_definition));

  JSClass* instance_class = &interface_data->instance_class_definition;
  const int kGlobalFlags = 0;
  instance_class->name = "MozjsOperationsTestInterface";
  instance_class->flags = kGlobalFlags | JSCLASS_HAS_PRIVATE;
  instance_class->addProperty = JS_PropertyStub;
  instance_class->delProperty = JS_DeletePropertyStub;
  instance_class->getProperty = JS_PropertyStub;
  instance_class->setProperty = JS_StrictPropertyStub;
  instance_class->enumerate = JS_EnumerateStub;
  instance_class->resolve = JS_ResolveStub;
  instance_class->convert = JS_ConvertStub;
  // Function to be called before on object of this class is garbage collected.
  instance_class->finalize = &WrapperPrivate::Finalizer;

  JSClass* prototype_class = &interface_data->prototype_class_definition;
  prototype_class->name = "MozjsOperationsTestInterfacePrototype";
  prototype_class->flags = 0;
  prototype_class->addProperty = JS_PropertyStub;
  prototype_class->delProperty = JS_DeletePropertyStub;
  prototype_class->getProperty = JS_PropertyStub;
  prototype_class->setProperty = JS_StrictPropertyStub;
  prototype_class->enumerate = JS_EnumerateStub;
  prototype_class->resolve = JS_ResolveStub;
  prototype_class->convert = JS_ConvertStub;

  JSClass* interface_object_class = &interface_data->interface_object_class_definition;
  interface_object_class->name = "MozjsOperationsTestInterfaceConstructor";
  interface_object_class->flags = 0;
  interface_object_class->addProperty = JS_PropertyStub;
  interface_object_class->delProperty = JS_DeletePropertyStub;
  interface_object_class->getProperty = JS_PropertyStub;
  interface_object_class->setProperty = JS_StrictPropertyStub;
  interface_object_class->enumerate = JS_EnumerateStub;
  interface_object_class->resolve = JS_ResolveStub;
  interface_object_class->convert = JS_ConvertStub;
  return interface_data;
}

JSBool fcn_longFunctionNoArgs(
    JSContext* context, uint32_t argc, JS::Value *vp) {
  MozjsExceptionState exception_state(context);
  JS::RootedValue result_value(context);

  // Compute the 'this' value.
  JS::RootedValue this_value(context, JS_ComputeThis(context, vp));
  // 'this' should be an object.
  JS::RootedObject object(context);
  if (JS_TypeOfValue(context, this_value) != JSTYPE_OBJECT) {
    NOTREACHED();
    return false;
  }
  if (!JS_ValueToObject(context, this_value, object.address())) {
    NOTREACHED();
    return false;
  }

  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  OperationsTestInterface* impl =
      WrapperPrivate::GetWrappable<OperationsTestInterface>(object);
  TypeTraits<int32_t >::ReturnType value =
      impl->LongFunctionNoArgs();
  if (!exception_state.IsExceptionSet()) {
    ToJSValue(value, &exception_state, &result_value);
  }

  if (!exception_state.IsExceptionSet()) {
    args.rval().set(result_value);
  }
  return !exception_state.IsExceptionSet();
}

JSBool fcn_objectFunctionNoArgs(
    JSContext* context, uint32_t argc, JS::Value *vp) {
  MozjsExceptionState exception_state(context);
  JS::RootedValue result_value(context);

  // Compute the 'this' value.
  JS::RootedValue this_value(context, JS_ComputeThis(context, vp));
  // 'this' should be an object.
  JS::RootedObject object(context);
  if (JS_TypeOfValue(context, this_value) != JSTYPE_OBJECT) {
    NOTREACHED();
    return false;
  }
  if (!JS_ValueToObject(context, this_value, object.address())) {
    NOTREACHED();
    return false;
  }

  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  OperationsTestInterface* impl =
      WrapperPrivate::GetWrappable<OperationsTestInterface>(object);
  TypeTraits<scoped_refptr<ArbitraryInterface> >::ReturnType value =
      impl->ObjectFunctionNoArgs();
  if (!exception_state.IsExceptionSet()) {
    ToJSValue(value, &exception_state, &result_value);
  }

  if (!exception_state.IsExceptionSet()) {
    args.rval().set(result_value);
  }
  return !exception_state.IsExceptionSet();
}

JSBool fcn_optionalArgumentWithDefault(
    JSContext* context, uint32_t argc, JS::Value *vp) {
  MozjsExceptionState exception_state(context);
  JS::RootedValue result_value(context);

  // Compute the 'this' value.
  JS::RootedValue this_value(context, JS_ComputeThis(context, vp));
  // 'this' should be an object.
  JS::RootedObject object(context);
  if (JS_TypeOfValue(context, this_value) != JSTYPE_OBJECT) {
    NOTREACHED();
    return false;
  }
  if (!JS_ValueToObject(context, this_value, object.address())) {
    NOTREACHED();
    return false;
  }

  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  const size_t kMinArguments = 1;
  if (args.length() < kMinArguments) {
    exception_state.SetSimpleException(
        script::ExceptionState::kTypeError, "Not enough arguments.");
    return false;
  }
  TypeTraits<double >::ConversionType arg1;
  DCHECK_LT(0, args.length());
  FromJSValue(context, args.handleAt(0), &exception_state, &arg1);
  if (exception_state.IsExceptionSet()) {
    return false;
  }
  OperationsTestInterface* impl =
      WrapperPrivate::GetWrappable<OperationsTestInterface>(object);
  impl->OptionalArgumentWithDefault(arg1);
  result_value.set(JS::UndefinedHandleValue);

  if (!exception_state.IsExceptionSet()) {
    args.rval().set(result_value);
  }
  return !exception_state.IsExceptionSet();
}

JSBool fcn_optionalArguments(
    JSContext* context, uint32_t argc, JS::Value *vp) {
  MozjsExceptionState exception_state(context);
  JS::RootedValue result_value(context);

  // Compute the 'this' value.
  JS::RootedValue this_value(context, JS_ComputeThis(context, vp));
  // 'this' should be an object.
  JS::RootedObject object(context);
  if (JS_TypeOfValue(context, this_value) != JSTYPE_OBJECT) {
    NOTREACHED();
    return false;
  }
  if (!JS_ValueToObject(context, this_value, object.address())) {
    NOTREACHED();
    return false;
  }

  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  const size_t kMinArguments = 3;
  if (args.length() < kMinArguments) {
    exception_state.SetSimpleException(
        script::ExceptionState::kTypeError, "Not enough arguments.");
    return false;
  }
  TypeTraits<int32_t >::ConversionType arg1;
  DCHECK_LT(0, args.length());
  FromJSValue(context, args.handleAt(0), &exception_state, &arg1);
  if (exception_state.IsExceptionSet()) {
    return false;
  }
  TypeTraits<int32_t >::ConversionType arg2;
  DCHECK_LT(1, args.length());
  FromJSValue(context, args.handleAt(1), &exception_state, &arg2);
  if (exception_state.IsExceptionSet()) {
    return false;
  }
  TypeTraits<int32_t >::ConversionType arg3;
  DCHECK_LT(2, args.length());
  FromJSValue(context, args.handleAt(2), &exception_state, &arg3);
  if (exception_state.IsExceptionSet()) {
    return false;
  }
  OperationsTestInterface* impl =
      WrapperPrivate::GetWrappable<OperationsTestInterface>(object);
  impl->OptionalArguments(arg1, arg2, arg3);
  result_value.set(JS::UndefinedHandleValue);

  if (!exception_state.IsExceptionSet()) {
    args.rval().set(result_value);
  }
  return !exception_state.IsExceptionSet();
}

JSBool fcn_optionalNullableArgumentsWithDefaults(
    JSContext* context, uint32_t argc, JS::Value *vp) {
  MozjsExceptionState exception_state(context);
  JS::RootedValue result_value(context);

  // Compute the 'this' value.
  JS::RootedValue this_value(context, JS_ComputeThis(context, vp));
  // 'this' should be an object.
  JS::RootedObject object(context);
  if (JS_TypeOfValue(context, this_value) != JSTYPE_OBJECT) {
    NOTREACHED();
    return false;
  }
  if (!JS_ValueToObject(context, this_value, object.address())) {
    NOTREACHED();
    return false;
  }

  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  const size_t kMinArguments = 2;
  if (args.length() < kMinArguments) {
    exception_state.SetSimpleException(
        script::ExceptionState::kTypeError, "Not enough arguments.");
    return false;
  }
  TypeTraits<base::optional<bool > >::ConversionType arg1;
  DCHECK_LT(0, args.length());
  FromJSValue(context, args.handleAt(0), &exception_state, &arg1);
  if (exception_state.IsExceptionSet()) {
    return false;
  }
  TypeTraits<scoped_refptr<ArbitraryInterface> >::ConversionType arg2;
  DCHECK_LT(1, args.length());
  FromJSValue(context, args.handleAt(1), &exception_state, &arg2);
  if (exception_state.IsExceptionSet()) {
    return false;
  }
  OperationsTestInterface* impl =
      WrapperPrivate::GetWrappable<OperationsTestInterface>(object);
  impl->OptionalNullableArgumentsWithDefaults(arg1, arg2);
  result_value.set(JS::UndefinedHandleValue);

  if (!exception_state.IsExceptionSet()) {
    args.rval().set(result_value);
  }
  return !exception_state.IsExceptionSet();
}

JSBool fcn_overloadedFunction(
    JSContext* context, uint32_t argc, JS::Value *vp) {
  MozjsExceptionState exception_state(context);
  JS::RootedValue result_value(context);

  // Compute the 'this' value.
  JS::RootedValue this_value(context, JS_ComputeThis(context, vp));
  // 'this' should be an object.
  JS::RootedObject object(context);
  if (JS_TypeOfValue(context, this_value) != JSTYPE_OBJECT) {
    NOTREACHED();
    return false;
  }
  if (!JS_ValueToObject(context, this_value, object.address())) {
    NOTREACHED();
    return false;
  }

  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  OperationsTestInterface* impl =
      WrapperPrivate::GetWrappable<OperationsTestInterface>(object);
  impl->OverloadedFunction();
  result_value.set(JS::UndefinedHandleValue);

  if (!exception_state.IsExceptionSet()) {
    args.rval().set(result_value);
  }
  return !exception_state.IsExceptionSet();
}

JSBool fcn_overloadedNullable(
    JSContext* context, uint32_t argc, JS::Value *vp) {
  MozjsExceptionState exception_state(context);
  JS::RootedValue result_value(context);

  // Compute the 'this' value.
  JS::RootedValue this_value(context, JS_ComputeThis(context, vp));
  // 'this' should be an object.
  JS::RootedObject object(context);
  if (JS_TypeOfValue(context, this_value) != JSTYPE_OBJECT) {
    NOTREACHED();
    return false;
  }
  if (!JS_ValueToObject(context, this_value, object.address())) {
    NOTREACHED();
    return false;
  }

  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  const size_t kMinArguments = 1;
  if (args.length() < kMinArguments) {
    exception_state.SetSimpleException(
        script::ExceptionState::kTypeError, "Not enough arguments.");
    return false;
  }
  TypeTraits<int32_t >::ConversionType arg;
  DCHECK_LT(0, args.length());
  FromJSValue(context, args.handleAt(0), &exception_state, &arg);
  if (exception_state.IsExceptionSet()) {
    return false;
  }
  OperationsTestInterface* impl =
      WrapperPrivate::GetWrappable<OperationsTestInterface>(object);
  impl->OverloadedNullable(arg);
  result_value.set(JS::UndefinedHandleValue);

  if (!exception_state.IsExceptionSet()) {
    args.rval().set(result_value);
  }
  return !exception_state.IsExceptionSet();
}

JSBool fcn_stringFunctionNoArgs(
    JSContext* context, uint32_t argc, JS::Value *vp) {
  MozjsExceptionState exception_state(context);
  JS::RootedValue result_value(context);

  // Compute the 'this' value.
  JS::RootedValue this_value(context, JS_ComputeThis(context, vp));
  // 'this' should be an object.
  JS::RootedObject object(context);
  if (JS_TypeOfValue(context, this_value) != JSTYPE_OBJECT) {
    NOTREACHED();
    return false;
  }
  if (!JS_ValueToObject(context, this_value, object.address())) {
    NOTREACHED();
    return false;
  }

  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  OperationsTestInterface* impl =
      WrapperPrivate::GetWrappable<OperationsTestInterface>(object);
  TypeTraits<std::string >::ReturnType value =
      impl->StringFunctionNoArgs();
  if (!exception_state.IsExceptionSet()) {
    ToJSValue(value, &exception_state, &result_value);
  }

  if (!exception_state.IsExceptionSet()) {
    args.rval().set(result_value);
  }
  return !exception_state.IsExceptionSet();
}

JSBool fcn_variadicPrimitiveArguments(
    JSContext* context, uint32_t argc, JS::Value *vp) {
  MozjsExceptionState exception_state(context);
  JS::RootedValue result_value(context);

  // Compute the 'this' value.
  JS::RootedValue this_value(context, JS_ComputeThis(context, vp));
  // 'this' should be an object.
  JS::RootedObject object(context);
  if (JS_TypeOfValue(context, this_value) != JSTYPE_OBJECT) {
    NOTREACHED();
    return false;
  }
  if (!JS_ValueToObject(context, this_value, object.address())) {
    NOTREACHED();
    return false;
  }

  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  const size_t kMinArguments = 1;
  if (args.length() < kMinArguments) {
    exception_state.SetSimpleException(
        script::ExceptionState::kTypeError, "Not enough arguments.");
    return false;
  }
  TypeTraits<std::vector<int32_t> >::ConversionType bools;
  DCHECK_LT(0, args.length());
  FromJSValue(context, args.handleAt(0), &exception_state, &bools);
  if (exception_state.IsExceptionSet()) {
    return false;
  }
  OperationsTestInterface* impl =
      WrapperPrivate::GetWrappable<OperationsTestInterface>(object);
  impl->VariadicPrimitiveArguments(bools);
  result_value.set(JS::UndefinedHandleValue);

  if (!exception_state.IsExceptionSet()) {
    args.rval().set(result_value);
  }
  return !exception_state.IsExceptionSet();
}

JSBool fcn_variadicStringArgumentsAfterOptionalArgument(
    JSContext* context, uint32_t argc, JS::Value *vp) {
  MozjsExceptionState exception_state(context);
  JS::RootedValue result_value(context);

  // Compute the 'this' value.
  JS::RootedValue this_value(context, JS_ComputeThis(context, vp));
  // 'this' should be an object.
  JS::RootedObject object(context);
  if (JS_TypeOfValue(context, this_value) != JSTYPE_OBJECT) {
    NOTREACHED();
    return false;
  }
  if (!JS_ValueToObject(context, this_value, object.address())) {
    NOTREACHED();
    return false;
  }

  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  const size_t kMinArguments = 2;
  if (args.length() < kMinArguments) {
    exception_state.SetSimpleException(
        script::ExceptionState::kTypeError, "Not enough arguments.");
    return false;
  }
  TypeTraits<bool >::ConversionType optional_arg;
  DCHECK_LT(0, args.length());
  FromJSValue(context, args.handleAt(0), &exception_state, &optional_arg);
  if (exception_state.IsExceptionSet()) {
    return false;
  }
  TypeTraits<std::vector<std::string> >::ConversionType strings;
  DCHECK_LT(1, args.length());
  FromJSValue(context, args.handleAt(1), &exception_state, &strings);
  if (exception_state.IsExceptionSet()) {
    return false;
  }
  OperationsTestInterface* impl =
      WrapperPrivate::GetWrappable<OperationsTestInterface>(object);
  impl->VariadicStringArgumentsAfterOptionalArgument(optional_arg, strings);
  result_value.set(JS::UndefinedHandleValue);

  if (!exception_state.IsExceptionSet()) {
    args.rval().set(result_value);
  }
  return !exception_state.IsExceptionSet();
}

JSBool fcn_voidFunctionLongArg(
    JSContext* context, uint32_t argc, JS::Value *vp) {
  MozjsExceptionState exception_state(context);
  JS::RootedValue result_value(context);

  // Compute the 'this' value.
  JS::RootedValue this_value(context, JS_ComputeThis(context, vp));
  // 'this' should be an object.
  JS::RootedObject object(context);
  if (JS_TypeOfValue(context, this_value) != JSTYPE_OBJECT) {
    NOTREACHED();
    return false;
  }
  if (!JS_ValueToObject(context, this_value, object.address())) {
    NOTREACHED();
    return false;
  }

  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  const size_t kMinArguments = 1;
  if (args.length() < kMinArguments) {
    exception_state.SetSimpleException(
        script::ExceptionState::kTypeError, "Not enough arguments.");
    return false;
  }
  TypeTraits<int32_t >::ConversionType arg;
  DCHECK_LT(0, args.length());
  FromJSValue(context, args.handleAt(0), &exception_state, &arg);
  if (exception_state.IsExceptionSet()) {
    return false;
  }
  OperationsTestInterface* impl =
      WrapperPrivate::GetWrappable<OperationsTestInterface>(object);
  impl->VoidFunctionLongArg(arg);
  result_value.set(JS::UndefinedHandleValue);

  if (!exception_state.IsExceptionSet()) {
    args.rval().set(result_value);
  }
  return !exception_state.IsExceptionSet();
}

JSBool fcn_voidFunctionNoArgs(
    JSContext* context, uint32_t argc, JS::Value *vp) {
  MozjsExceptionState exception_state(context);
  JS::RootedValue result_value(context);

  // Compute the 'this' value.
  JS::RootedValue this_value(context, JS_ComputeThis(context, vp));
  // 'this' should be an object.
  JS::RootedObject object(context);
  if (JS_TypeOfValue(context, this_value) != JSTYPE_OBJECT) {
    NOTREACHED();
    return false;
  }
  if (!JS_ValueToObject(context, this_value, object.address())) {
    NOTREACHED();
    return false;
  }

  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  OperationsTestInterface* impl =
      WrapperPrivate::GetWrappable<OperationsTestInterface>(object);
  impl->VoidFunctionNoArgs();
  result_value.set(JS::UndefinedHandleValue);

  if (!exception_state.IsExceptionSet()) {
    args.rval().set(result_value);
  }
  return !exception_state.IsExceptionSet();
}

JSBool fcn_voidFunctionObjectArg(
    JSContext* context, uint32_t argc, JS::Value *vp) {
  MozjsExceptionState exception_state(context);
  JS::RootedValue result_value(context);

  // Compute the 'this' value.
  JS::RootedValue this_value(context, JS_ComputeThis(context, vp));
  // 'this' should be an object.
  JS::RootedObject object(context);
  if (JS_TypeOfValue(context, this_value) != JSTYPE_OBJECT) {
    NOTREACHED();
    return false;
  }
  if (!JS_ValueToObject(context, this_value, object.address())) {
    NOTREACHED();
    return false;
  }

  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  const size_t kMinArguments = 1;
  if (args.length() < kMinArguments) {
    exception_state.SetSimpleException(
        script::ExceptionState::kTypeError, "Not enough arguments.");
    return false;
  }
  TypeTraits<scoped_refptr<ArbitraryInterface> >::ConversionType arg;
  DCHECK_LT(0, args.length());
  FromJSValue(context, args.handleAt(0), &exception_state, &arg);
  if (exception_state.IsExceptionSet()) {
    return false;
  }
  OperationsTestInterface* impl =
      WrapperPrivate::GetWrappable<OperationsTestInterface>(object);
  impl->VoidFunctionObjectArg(arg);
  result_value.set(JS::UndefinedHandleValue);

  if (!exception_state.IsExceptionSet()) {
    args.rval().set(result_value);
  }
  return !exception_state.IsExceptionSet();
}

JSBool fcn_voidFunctionStringArg(
    JSContext* context, uint32_t argc, JS::Value *vp) {
  MozjsExceptionState exception_state(context);
  JS::RootedValue result_value(context);

  // Compute the 'this' value.
  JS::RootedValue this_value(context, JS_ComputeThis(context, vp));
  // 'this' should be an object.
  JS::RootedObject object(context);
  if (JS_TypeOfValue(context, this_value) != JSTYPE_OBJECT) {
    NOTREACHED();
    return false;
  }
  if (!JS_ValueToObject(context, this_value, object.address())) {
    NOTREACHED();
    return false;
  }

  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  const size_t kMinArguments = 1;
  if (args.length() < kMinArguments) {
    exception_state.SetSimpleException(
        script::ExceptionState::kTypeError, "Not enough arguments.");
    return false;
  }
  TypeTraits<std::string >::ConversionType arg;
  DCHECK_LT(0, args.length());
  FromJSValue(context, args.handleAt(0), &exception_state, &arg);
  if (exception_state.IsExceptionSet()) {
    return false;
  }
  OperationsTestInterface* impl =
      WrapperPrivate::GetWrappable<OperationsTestInterface>(object);
  impl->VoidFunctionStringArg(arg);
  result_value.set(JS::UndefinedHandleValue);

  if (!exception_state.IsExceptionSet()) {
    args.rval().set(result_value);
  }
  return !exception_state.IsExceptionSet();
}


const JSPropertySpec prototype_properties[] = {
  JS_PS_END
};

const JSFunctionSpec prototype_functions[] = {
  {
      "longFunctionNoArgs",
      JSOP_WRAPPER(&fcn_longFunctionNoArgs),
      0,
      JSPROP_ENUMERATE,
      NULL,
  },
  {
      "objectFunctionNoArgs",
      JSOP_WRAPPER(&fcn_objectFunctionNoArgs),
      0,
      JSPROP_ENUMERATE,
      NULL,
  },
  {
      "optionalArgumentWithDefault",
      JSOP_WRAPPER(&fcn_optionalArgumentWithDefault),
      0,
      JSPROP_ENUMERATE,
      NULL,
  },
  {
      "optionalArguments",
      JSOP_WRAPPER(&fcn_optionalArguments),
      1,
      JSPROP_ENUMERATE,
      NULL,
  },
  {
      "optionalNullableArgumentsWithDefaults",
      JSOP_WRAPPER(&fcn_optionalNullableArgumentsWithDefaults),
      0,
      JSPROP_ENUMERATE,
      NULL,
  },
  {
      "overloadedFunction",
      JSOP_WRAPPER(&fcn_overloadedFunction),
      0,
      JSPROP_ENUMERATE,
      NULL,
  },
  {
      "overloadedNullable",
      JSOP_WRAPPER(&fcn_overloadedNullable),
      1,
      JSPROP_ENUMERATE,
      NULL,
  },
  {
      "stringFunctionNoArgs",
      JSOP_WRAPPER(&fcn_stringFunctionNoArgs),
      0,
      JSPROP_ENUMERATE,
      NULL,
  },
  {
      "variadicPrimitiveArguments",
      JSOP_WRAPPER(&fcn_variadicPrimitiveArguments),
      0,
      JSPROP_ENUMERATE,
      NULL,
  },
  {
      "variadicStringArgumentsAfterOptionalArgument",
      JSOP_WRAPPER(&fcn_variadicStringArgumentsAfterOptionalArgument),
      0,
      JSPROP_ENUMERATE,
      NULL,
  },
  {
      "voidFunctionLongArg",
      JSOP_WRAPPER(&fcn_voidFunctionLongArg),
      1,
      JSPROP_ENUMERATE,
      NULL,
  },
  {
      "voidFunctionNoArgs",
      JSOP_WRAPPER(&fcn_voidFunctionNoArgs),
      0,
      JSPROP_ENUMERATE,
      NULL,
  },
  {
      "voidFunctionObjectArg",
      JSOP_WRAPPER(&fcn_voidFunctionObjectArg),
      1,
      JSPROP_ENUMERATE,
      NULL,
  },
  {
      "voidFunctionStringArg",
      JSOP_WRAPPER(&fcn_voidFunctionStringArg),
      1,
      JSPROP_ENUMERATE,
      NULL,
  },
  JS_FS_END
};

const JSPropertySpec own_properties[] = {
  JS_PS_END
};

void InitializePrototypeAndInterfaceObject(
    InterfaceData* interface_data, JSContext* context) {
  DCHECK(!interface_data->prototype);
  DCHECK(!interface_data->interface_object);

  MozjsGlobalObjectProxy* global_object_proxy =
      static_cast<MozjsGlobalObjectProxy*>(JS_GetContextPrivate(context));
  JS::RootedObject global_object(context, global_object_proxy->global_object());
  DCHECK(global_object);
  JS::RootedObject parent_prototype(
      context, JS_GetObjectPrototype(context, global_object));
  DCHECK(parent_prototype);

  // Create the Prototype object.
  interface_data->prototype = JS_NewObjectWithGivenProto(
      context, &interface_data->prototype_class_definition, parent_prototype, NULL);
  bool success = JS_DefineProperties(
      context, interface_data->prototype, prototype_properties);
  DCHECK(success);
  success = JS_DefineFunctions(
      context, interface_data->prototype, prototype_functions);
  DCHECK(success);

  JS::RootedObject function_prototype(
      context, JS_GetFunctionPrototype(context, global_object));
  DCHECK(function_prototype);
  // Create the Interface object.
  interface_data->interface_object = JS_NewObjectWithGivenProto(
      context, &interface_data->interface_object_class_definition,
      function_prototype, NULL);

  // Add the InterfaceObject.name property.
  JS::RootedObject rooted_interface_object(
      context, interface_data->interface_object);
  JS::RootedValue name_value(context);
  const char name[] = "OperationsTestInterface";
  name_value.setString(JS_NewStringCopyZ(context, "OperationsTestInterface"));
  success =
      JS_DefineProperty(context, rooted_interface_object, "name", name_value,
                        JS_PropertyStub, JS_StrictPropertyStub,
                        JSPROP_READONLY);
  DCHECK(success);

  // Set the Prototype.constructor and Constructor.prototype properties.
  DCHECK(interface_data->interface_object);
  DCHECK(interface_data->prototype);
  JS::RootedObject rooted_prototype(context, interface_data->prototype);
  success = JS_LinkConstructorAndPrototype(
      context,
      rooted_interface_object,
      rooted_prototype);
  DCHECK(success);
}

InterfaceData* GetInterfaceData(JSContext* context) {
  MozjsGlobalObjectProxy* global_object_proxy =
      static_cast<MozjsGlobalObjectProxy*>(JS_GetContextPrivate(context));
  // Use the address of the properties definition for this interface as a
  // unique key for looking up the InterfaceData for this interface.
  intptr_t key = reinterpret_cast<intptr_t>(&own_properties);
  InterfaceData* interface_data = global_object_proxy->GetInterfaceData(key);
  if (!interface_data) {
    interface_data = CreateCachedInterfaceData();
    DCHECK(interface_data);
    global_object_proxy->CacheInterfaceData(key, interface_data);
    DCHECK_EQ(interface_data, global_object_proxy->GetInterfaceData(key));
  }
  return interface_data;
}

}  // namespace

// static
JSObject* MozjsOperationsTestInterface::CreateInstance(
    JSContext* context, const scoped_refptr<Wrappable>& wrappable) {
  InterfaceData* interface_data = GetInterfaceData(context);
  JS::RootedObject prototype(context, GetPrototype(context));
  DCHECK(prototype);
  JS::RootedObject new_object(context, JS_NewObjectWithGivenProto(
      context, &interface_data->instance_class_definition, prototype, NULL));
  DCHECK(new_object);
  WrapperPrivate::AddPrivateData(new_object, wrappable);
  return new_object;
}

// static
JSObject* MozjsOperationsTestInterface::GetPrototype(JSContext* context) {
  InterfaceData* interface_data = GetInterfaceData(context);
  if (!interface_data->prototype) {
    // Create new prototype that has all the props and methods
    InitializePrototypeAndInterfaceObject(interface_data, context);
  }
  DCHECK(interface_data->prototype);
  return interface_data->prototype;
}

// static
JSObject* MozjsOperationsTestInterface::GetInterfaceObject(JSContext* context) {
  InterfaceData* interface_data = GetInterfaceData(context);
  if (!interface_data->interface_object) {
    InitializePrototypeAndInterfaceObject(interface_data, context);
  }
  DCHECK(interface_data->interface_object);
  return interface_data->interface_object;
}


namespace {
}  // namespace


}  // namespace testing
}  // namespace bindings
}  // namespace cobalt
