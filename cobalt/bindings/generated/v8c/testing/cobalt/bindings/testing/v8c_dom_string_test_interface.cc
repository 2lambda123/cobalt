

// Copyright 2018 Google Inc. All Rights Reserved.
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

// clang-format off

// This file has been auto-generated by bindings/code_generator_cobalt.py. DO NOT MODIFY!
// Auto-generated from template: bindings/v8c/templates/interface.cc.template

#include "cobalt/bindings/testing/v8c_dom_string_test_interface.h"

#include "base/debug/trace_event.h"
#include "cobalt/base/polymorphic_downcast.h"
#include "cobalt/script/global_environment.h"
#include "cobalt/script/script_value.h"
#include "cobalt/script/value_handle.h"

#include "v8c_gen_type_conversion.h"

#include "cobalt/script/callback_interface_traits.h"
#include "cobalt/script/v8c/callback_function_conversion.h"
#include "cobalt/script/v8c/conversion_helpers.h"
#include "cobalt/script/v8c/entry_scope.h"
#include "cobalt/script/v8c/native_promise.h"
#include "cobalt/script/v8c/type_traits.h"
#include "cobalt/script/v8c/v8c_callback_function.h"
#include "cobalt/script/v8c/v8c_callback_interface_holder.h"
#include "cobalt/script/v8c/v8c_exception_state.h"
#include "cobalt/script/v8c/v8c_global_environment.h"
#include "cobalt/script/v8c/v8c_value_handle.h"
#include "cobalt/script/v8c/wrapper_private.h"
#include "v8/include/v8.h"


namespace {
using cobalt::bindings::testing::DOMStringTestInterface;
using cobalt::bindings::testing::V8cDOMStringTestInterface;
using cobalt::script::CallbackInterfaceTraits;
using cobalt::script::GlobalEnvironment;
using cobalt::script::ScriptValue;
using cobalt::script::ValueHandle;
using cobalt::script::ValueHandle;
using cobalt::script::ValueHandleHolder;
using cobalt::script::Wrappable;

using cobalt::script::v8c::EntryScope;
using cobalt::script::v8c::EscapableEntryScope;
using cobalt::script::v8c::FromJSValue;
using cobalt::script::v8c::InterfaceData;
using cobalt::script::v8c::kConversionFlagClamped;
using cobalt::script::v8c::kConversionFlagNullable;
using cobalt::script::v8c::kConversionFlagObjectOnly;
using cobalt::script::v8c::kConversionFlagRestricted;
using cobalt::script::v8c::kConversionFlagTreatNullAsEmptyString;
using cobalt::script::v8c::kConversionFlagTreatUndefinedAsEmptyString;
using cobalt::script::v8c::kNoConversionFlags;
using cobalt::script::v8c::ToJSValue;
using cobalt::script::v8c::TypeTraits;
using cobalt::script::v8c::V8cExceptionState;
using cobalt::script::v8c::V8cGlobalEnvironment;
using cobalt::script::v8c::WrapperFactory;
using cobalt::script::v8c::WrapperPrivate;

v8::Local<v8::Object> DummyFunctor(v8::Isolate*, const scoped_refptr<Wrappable>&) {
  NOTIMPLEMENTED();
  return {};
}

}  // namespace

namespace cobalt {
namespace bindings {
namespace testing {


namespace {



void propertyAttributeGetter(
    v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::Local<v8::Object> object = info.This();


  V8cGlobalEnvironment* global_environment = V8cGlobalEnvironment::GetFromIsolate(isolate);
  WrapperFactory* wrapper_factory = global_environment->wrapper_factory();
  if (!wrapper_factory->DoesObjectImplementInterface(
        object, base::GetTypeId<DOMStringTestInterface>())) {
    V8cExceptionState exception(isolate);
    exception.SetSimpleException(script::kDoesNotImplementInterface);
    return;
  }
  V8cExceptionState exception_state{isolate};
  v8::Local<v8::Value> result_value;

  WrapperPrivate* wrapper_private =
      WrapperPrivate::GetFromWrapperObject(object);
  if (!wrapper_private) {
    NOTIMPLEMENTED();
    return;
  }
  DOMStringTestInterface* impl =
      wrapper_private->wrappable<DOMStringTestInterface>().get();

  if (!exception_state.is_exception_set()) {
    ToJSValue(isolate,
              impl->property(),
              &result_value);
  }
  if (!exception_state.is_exception_set()) {
    info.GetReturnValue().Set(result_value);
  }
}

void propertyAttributeSetter(
    v8::Local<v8::String> property,
    v8::Local<v8::Value> v8_value,
    const v8::PropertyCallbackInfo<void>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::Local<v8::Object> object = info.This();

  V8cGlobalEnvironment* global_environment = V8cGlobalEnvironment::GetFromIsolate(isolate);
  WrapperFactory* wrapper_factory = global_environment->wrapper_factory();
  if (!wrapper_factory->DoesObjectImplementInterface(
        object, base::GetTypeId<DOMStringTestInterface>())) {
    V8cExceptionState exception(isolate);
    exception.SetSimpleException(script::kDoesNotImplementInterface);
    return;
  }
  V8cExceptionState exception_state{isolate};
  v8::Local<v8::Value> result_value;

  WrapperPrivate* wrapper_private =
      WrapperPrivate::GetFromWrapperObject(object);
  if (!wrapper_private) {
    NOTIMPLEMENTED();
    return;
  }
  DOMStringTestInterface* impl =
      wrapper_private->wrappable<DOMStringTestInterface>().get();
  TypeTraits<std::string >::ConversionType value;
  FromJSValue(isolate, v8_value, kNoConversionFlags, &exception_state,
              &value);
  if (exception_state.is_exception_set()) {
    return;
  }

  impl->set_property(value);
  result_value = v8::Undefined(isolate);
  return;
}

void readOnlyPropertyAttributeGetter(
    v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::Local<v8::Object> object = info.This();


  V8cGlobalEnvironment* global_environment = V8cGlobalEnvironment::GetFromIsolate(isolate);
  WrapperFactory* wrapper_factory = global_environment->wrapper_factory();
  if (!wrapper_factory->DoesObjectImplementInterface(
        object, base::GetTypeId<DOMStringTestInterface>())) {
    V8cExceptionState exception(isolate);
    exception.SetSimpleException(script::kDoesNotImplementInterface);
    return;
  }
  V8cExceptionState exception_state{isolate};
  v8::Local<v8::Value> result_value;

  WrapperPrivate* wrapper_private =
      WrapperPrivate::GetFromWrapperObject(object);
  if (!wrapper_private) {
    NOTIMPLEMENTED();
    return;
  }
  DOMStringTestInterface* impl =
      wrapper_private->wrappable<DOMStringTestInterface>().get();

  if (!exception_state.is_exception_set()) {
    ToJSValue(isolate,
              impl->read_only_property(),
              &result_value);
  }
  if (!exception_state.is_exception_set()) {
    info.GetReturnValue().Set(result_value);
  }
}


void readOnlyTokenPropertyAttributeGetter(
    v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::Local<v8::Object> object = info.This();


  V8cGlobalEnvironment* global_environment = V8cGlobalEnvironment::GetFromIsolate(isolate);
  WrapperFactory* wrapper_factory = global_environment->wrapper_factory();
  if (!wrapper_factory->DoesObjectImplementInterface(
        object, base::GetTypeId<DOMStringTestInterface>())) {
    V8cExceptionState exception(isolate);
    exception.SetSimpleException(script::kDoesNotImplementInterface);
    return;
  }
  V8cExceptionState exception_state{isolate};
  v8::Local<v8::Value> result_value;

  WrapperPrivate* wrapper_private =
      WrapperPrivate::GetFromWrapperObject(object);
  if (!wrapper_private) {
    NOTIMPLEMENTED();
    return;
  }
  DOMStringTestInterface* impl =
      wrapper_private->wrappable<DOMStringTestInterface>().get();

  if (!exception_state.is_exception_set()) {
    ToJSValue(isolate,
              impl->read_only_token_property(),
              &result_value);
  }
  if (!exception_state.is_exception_set()) {
    info.GetReturnValue().Set(result_value);
  }
}


void nullIsEmptyPropertyAttributeGetter(
    v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::Local<v8::Object> object = info.This();


  V8cGlobalEnvironment* global_environment = V8cGlobalEnvironment::GetFromIsolate(isolate);
  WrapperFactory* wrapper_factory = global_environment->wrapper_factory();
  if (!wrapper_factory->DoesObjectImplementInterface(
        object, base::GetTypeId<DOMStringTestInterface>())) {
    V8cExceptionState exception(isolate);
    exception.SetSimpleException(script::kDoesNotImplementInterface);
    return;
  }
  V8cExceptionState exception_state{isolate};
  v8::Local<v8::Value> result_value;

  WrapperPrivate* wrapper_private =
      WrapperPrivate::GetFromWrapperObject(object);
  if (!wrapper_private) {
    NOTIMPLEMENTED();
    return;
  }
  DOMStringTestInterface* impl =
      wrapper_private->wrappable<DOMStringTestInterface>().get();

  if (!exception_state.is_exception_set()) {
    ToJSValue(isolate,
              impl->null_is_empty_property(),
              &result_value);
  }
  if (!exception_state.is_exception_set()) {
    info.GetReturnValue().Set(result_value);
  }
}

void nullIsEmptyPropertyAttributeSetter(
    v8::Local<v8::String> property,
    v8::Local<v8::Value> v8_value,
    const v8::PropertyCallbackInfo<void>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::Local<v8::Object> object = info.This();

  V8cGlobalEnvironment* global_environment = V8cGlobalEnvironment::GetFromIsolate(isolate);
  WrapperFactory* wrapper_factory = global_environment->wrapper_factory();
  if (!wrapper_factory->DoesObjectImplementInterface(
        object, base::GetTypeId<DOMStringTestInterface>())) {
    V8cExceptionState exception(isolate);
    exception.SetSimpleException(script::kDoesNotImplementInterface);
    return;
  }
  V8cExceptionState exception_state{isolate};
  v8::Local<v8::Value> result_value;

  WrapperPrivate* wrapper_private =
      WrapperPrivate::GetFromWrapperObject(object);
  if (!wrapper_private) {
    NOTIMPLEMENTED();
    return;
  }
  DOMStringTestInterface* impl =
      wrapper_private->wrappable<DOMStringTestInterface>().get();
  TypeTraits<std::string >::ConversionType value;
  FromJSValue(isolate, v8_value, (kConversionFlagTreatNullAsEmptyString), &exception_state,
              &value);
  if (exception_state.is_exception_set()) {
    return;
  }

  impl->set_null_is_empty_property(value);
  result_value = v8::Undefined(isolate);
  return;
}

void undefinedIsEmptyPropertyAttributeGetter(
    v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::Local<v8::Object> object = info.This();


  V8cGlobalEnvironment* global_environment = V8cGlobalEnvironment::GetFromIsolate(isolate);
  WrapperFactory* wrapper_factory = global_environment->wrapper_factory();
  if (!wrapper_factory->DoesObjectImplementInterface(
        object, base::GetTypeId<DOMStringTestInterface>())) {
    V8cExceptionState exception(isolate);
    exception.SetSimpleException(script::kDoesNotImplementInterface);
    return;
  }
  V8cExceptionState exception_state{isolate};
  v8::Local<v8::Value> result_value;

  WrapperPrivate* wrapper_private =
      WrapperPrivate::GetFromWrapperObject(object);
  if (!wrapper_private) {
    NOTIMPLEMENTED();
    return;
  }
  DOMStringTestInterface* impl =
      wrapper_private->wrappable<DOMStringTestInterface>().get();

  if (!exception_state.is_exception_set()) {
    ToJSValue(isolate,
              impl->undefined_is_empty_property(),
              &result_value);
  }
  if (!exception_state.is_exception_set()) {
    info.GetReturnValue().Set(result_value);
  }
}

void undefinedIsEmptyPropertyAttributeSetter(
    v8::Local<v8::String> property,
    v8::Local<v8::Value> v8_value,
    const v8::PropertyCallbackInfo<void>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::Local<v8::Object> object = info.This();

  V8cGlobalEnvironment* global_environment = V8cGlobalEnvironment::GetFromIsolate(isolate);
  WrapperFactory* wrapper_factory = global_environment->wrapper_factory();
  if (!wrapper_factory->DoesObjectImplementInterface(
        object, base::GetTypeId<DOMStringTestInterface>())) {
    V8cExceptionState exception(isolate);
    exception.SetSimpleException(script::kDoesNotImplementInterface);
    return;
  }
  V8cExceptionState exception_state{isolate};
  v8::Local<v8::Value> result_value;

  WrapperPrivate* wrapper_private =
      WrapperPrivate::GetFromWrapperObject(object);
  if (!wrapper_private) {
    NOTIMPLEMENTED();
    return;
  }
  DOMStringTestInterface* impl =
      wrapper_private->wrappable<DOMStringTestInterface>().get();
  TypeTraits<std::string >::ConversionType value;
  FromJSValue(isolate, v8_value, (kConversionFlagTreatUndefinedAsEmptyString), &exception_state,
              &value);
  if (exception_state.is_exception_set()) {
    return;
  }

  impl->set_undefined_is_empty_property(value);
  result_value = v8::Undefined(isolate);
  return;
}

void nullableUndefinedIsEmptyPropertyAttributeGetter(
    v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::Local<v8::Object> object = info.This();


  V8cGlobalEnvironment* global_environment = V8cGlobalEnvironment::GetFromIsolate(isolate);
  WrapperFactory* wrapper_factory = global_environment->wrapper_factory();
  if (!wrapper_factory->DoesObjectImplementInterface(
        object, base::GetTypeId<DOMStringTestInterface>())) {
    V8cExceptionState exception(isolate);
    exception.SetSimpleException(script::kDoesNotImplementInterface);
    return;
  }
  V8cExceptionState exception_state{isolate};
  v8::Local<v8::Value> result_value;

  WrapperPrivate* wrapper_private =
      WrapperPrivate::GetFromWrapperObject(object);
  if (!wrapper_private) {
    NOTIMPLEMENTED();
    return;
  }
  DOMStringTestInterface* impl =
      wrapper_private->wrappable<DOMStringTestInterface>().get();

  if (!exception_state.is_exception_set()) {
    ToJSValue(isolate,
              impl->nullable_undefined_is_empty_property(),
              &result_value);
  }
  if (!exception_state.is_exception_set()) {
    info.GetReturnValue().Set(result_value);
  }
}

void nullableUndefinedIsEmptyPropertyAttributeSetter(
    v8::Local<v8::String> property,
    v8::Local<v8::Value> v8_value,
    const v8::PropertyCallbackInfo<void>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::Local<v8::Object> object = info.This();

  V8cGlobalEnvironment* global_environment = V8cGlobalEnvironment::GetFromIsolate(isolate);
  WrapperFactory* wrapper_factory = global_environment->wrapper_factory();
  if (!wrapper_factory->DoesObjectImplementInterface(
        object, base::GetTypeId<DOMStringTestInterface>())) {
    V8cExceptionState exception(isolate);
    exception.SetSimpleException(script::kDoesNotImplementInterface);
    return;
  }
  V8cExceptionState exception_state{isolate};
  v8::Local<v8::Value> result_value;

  WrapperPrivate* wrapper_private =
      WrapperPrivate::GetFromWrapperObject(object);
  if (!wrapper_private) {
    NOTIMPLEMENTED();
    return;
  }
  DOMStringTestInterface* impl =
      wrapper_private->wrappable<DOMStringTestInterface>().get();
  TypeTraits<base::optional<std::string > >::ConversionType value;
  FromJSValue(isolate, v8_value, (kConversionFlagNullable | kConversionFlagTreatUndefinedAsEmptyString), &exception_state,
              &value);
  if (exception_state.is_exception_set()) {
    return;
  }

  impl->set_nullable_undefined_is_empty_property(value);
  result_value = v8::Undefined(isolate);
  return;
}


void InitializeTemplateAndInterfaceObject(v8::Isolate* isolate, InterfaceData* interface_data) {
  v8::Local<v8::FunctionTemplate> function_template = v8::FunctionTemplate::New(isolate);
  function_template->SetClassName(
    v8::String::NewFromUtf8(isolate, "DOMStringTestInterface",
        v8::NewStringType::kInternalized).ToLocalChecked());
  v8::Local<v8::ObjectTemplate> instance_template = function_template->InstanceTemplate();
  instance_template->SetInternalFieldCount(1);


  v8::Local<v8::ObjectTemplate> prototype_template = function_template->PrototypeTemplate();


  instance_template->SetAccessor(
    v8::String::NewFromUtf8(isolate, "property",
                              v8::NewStringType::kInternalized)
          .ToLocalChecked(),
    propertyAttributeGetter
    ,propertyAttributeSetter
  );
  instance_template->SetAccessor(
    v8::String::NewFromUtf8(isolate, "readOnlyProperty",
                              v8::NewStringType::kInternalized)
          .ToLocalChecked(),
    readOnlyPropertyAttributeGetter
  );
  instance_template->SetAccessor(
    v8::String::NewFromUtf8(isolate, "readOnlyTokenProperty",
                              v8::NewStringType::kInternalized)
          .ToLocalChecked(),
    readOnlyTokenPropertyAttributeGetter
  );
  instance_template->SetAccessor(
    v8::String::NewFromUtf8(isolate, "nullIsEmptyProperty",
                              v8::NewStringType::kInternalized)
          .ToLocalChecked(),
    nullIsEmptyPropertyAttributeGetter
    ,nullIsEmptyPropertyAttributeSetter
  );
  instance_template->SetAccessor(
    v8::String::NewFromUtf8(isolate, "undefinedIsEmptyProperty",
                              v8::NewStringType::kInternalized)
          .ToLocalChecked(),
    undefinedIsEmptyPropertyAttributeGetter
    ,undefinedIsEmptyPropertyAttributeSetter
  );
  instance_template->SetAccessor(
    v8::String::NewFromUtf8(isolate, "nullableUndefinedIsEmptyProperty",
                              v8::NewStringType::kInternalized)
          .ToLocalChecked(),
    nullableUndefinedIsEmptyPropertyAttributeGetter
    ,nullableUndefinedIsEmptyPropertyAttributeSetter
  );


  interface_data->function_template.Set(isolate, function_template);
}

inline InterfaceData* GetInterfaceData(V8cGlobalEnvironment* global_environment) {
  const int kInterfaceUniqueId = 12;
  // By convention, the |V8cGlobalEnvironment| that we are associated with
  // will hold our |InterfaceData| at index |kInterfaceUniqueId|, as we asked
  // for it to be there in the first place, and could not have conflicted with
  // any other interface.
  return global_environment->GetInterfaceData(kInterfaceUniqueId);
}

}  // namespace

v8::Local<v8::Object> V8cDOMStringTestInterface::CreateWrapper(v8::Isolate* isolate, const scoped_refptr<Wrappable>& wrappable) {
  EscapableEntryScope entry_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  V8cGlobalEnvironment* global_environment = V8cGlobalEnvironment::GetFromIsolate(isolate);
  InterfaceData* interface_data = GetInterfaceData(global_environment);
  if (interface_data->function_template.IsEmpty()) {
    InitializeTemplateAndInterfaceObject(isolate, interface_data);
  }
  DCHECK(!interface_data->function_template.IsEmpty());

  v8::Local<v8::FunctionTemplate> function_template = interface_data->function_template.Get(isolate);
  DCHECK(function_template->InstanceTemplate()->InternalFieldCount() == 1);
  v8::Local<v8::Object> object = function_template->InstanceTemplate()->NewInstance(context).ToLocalChecked();
  DCHECK(object->InternalFieldCount() == 1);

  // This |WrapperPrivate|'s lifetime will be managed by V8.
  new WrapperPrivate(isolate, wrappable, object);
  return entry_scope.Escape(object);
}

v8::Local<v8::FunctionTemplate> V8cDOMStringTestInterface::CreateTemplate(v8::Isolate* isolate) {
  V8cGlobalEnvironment* global_environment = V8cGlobalEnvironment::GetFromIsolate(isolate);
  InterfaceData* interface_data = GetInterfaceData(global_environment);
  if (interface_data->function_template.IsEmpty()) {
    InitializeTemplateAndInterfaceObject(isolate, interface_data);
  }

  return interface_data->function_template.Get(isolate);
}


}  // namespace testing
}  // namespace bindings
}  // namespace cobalt


