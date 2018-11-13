

// Copyright 2018 The Cobalt Authors. All Rights Reserved.
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

#include "cobalt/bindings/testing/v8c_garbage_collection_test_interface.h"

#include "base/debug/trace_event.h"
#include "cobalt/base/polymorphic_downcast.h"
#include "cobalt/script/global_environment.h"
#include "cobalt/script/script_value.h"
#include "cobalt/script/value_handle.h"
#include "cobalt/bindings/testing/garbage_collection_test_interface.h"
#include "cobalt/bindings/testing/v8c_garbage_collection_test_interface.h"

#include "v8c_gen_type_conversion.h"

#include "cobalt/script/callback_interface_traits.h"
#include "cobalt/script/v8c/v8c_array_buffer.h"
#include "cobalt/script/v8c/v8c_array_buffer_view.h"
#include "cobalt/script/v8c/callback_function_conversion.h"
#include "cobalt/script/v8c/conversion_helpers.h"
#include "cobalt/script/v8c/entry_scope.h"
#include "cobalt/script/v8c/helpers.h"
#include "cobalt/script/v8c/isolate_fellowship.h"
#include "cobalt/script/v8c/native_promise.h"
#include "cobalt/script/v8c/type_traits.h"
#include "cobalt/script/v8c/v8c_typed_arrays.h"
#include "cobalt/script/v8c/v8c_data_view.h"
#include "cobalt/script/v8c/v8c_callback_function.h"
#include "cobalt/script/v8c/v8c_callback_interface_holder.h"
#include "cobalt/script/v8c/v8c_engine.h"
#include "cobalt/script/v8c/v8c_exception_state.h"
#include "cobalt/script/v8c/v8c_global_environment.h"
#include "cobalt/script/v8c/v8c_property_enumerator.h"
#include "cobalt/script/v8c/v8c_value_handle.h"
#include "cobalt/script/v8c/wrapper_private.h"
#include "v8/include/v8.h"


namespace {
using cobalt::bindings::testing::GarbageCollectionTestInterface;
using cobalt::bindings::testing::V8cGarbageCollectionTestInterface;
using cobalt::bindings::testing::GarbageCollectionTestInterface;
using cobalt::bindings::testing::V8cGarbageCollectionTestInterface;
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
using cobalt::script::v8c::kConversionFlagClamped;
using cobalt::script::v8c::kConversionFlagNullable;
using cobalt::script::v8c::kConversionFlagObjectOnly;
using cobalt::script::v8c::kConversionFlagRestricted;
using cobalt::script::v8c::kConversionFlagTreatNullAsEmptyString;
using cobalt::script::v8c::kConversionFlagTreatUndefinedAsEmptyString;
using cobalt::script::v8c::kNoConversionFlags;
using cobalt::script::v8c::NewInternalString;
using cobalt::script::v8c::ToJSValue;
using cobalt::script::v8c::TypeTraits;
using cobalt::script::v8c::V8cExceptionState;
using cobalt::script::v8c::V8cGlobalEnvironment;
using cobalt::script::v8c::V8cPropertyEnumerator;
using cobalt::script::v8c::WrapperFactory;
using cobalt::script::v8c::WrapperPrivate;
}  // namespace

namespace cobalt {
namespace bindings {
namespace testing {


namespace {

const int kInterfaceUniqueId = 24;









void Constructor(const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  V8cExceptionState exception_state(isolate);
  if (!info.IsConstructCall()) {
    exception_state.SetSimpleException(script::kTypeError, "Illegal constructor");
    return;
  }

  scoped_refptr<GarbageCollectionTestInterface> new_object =
      new GarbageCollectionTestInterface();
  v8::Local<v8::Value> result_value;
  ToJSValue(isolate, new_object, &result_value);
  DCHECK(result_value->IsObject());
  info.GetReturnValue().Set(result_value);
}



void previousAttributeGetter(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::Local<v8::Object> object = info.Holder();


  V8cGlobalEnvironment* global_environment = V8cGlobalEnvironment::GetFromIsolate(isolate);
  WrapperFactory* wrapper_factory = global_environment->wrapper_factory();
  if (!WrapperPrivate::HasWrapperPrivate(object) ||
      !V8cGarbageCollectionTestInterface::GetTemplate(isolate)->HasInstance(object)) {
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
  GarbageCollectionTestInterface* impl =
      wrapper_private->wrappable<GarbageCollectionTestInterface>().get();


  if (!exception_state.is_exception_set()) {
    ToJSValue(isolate,
              impl->previous(),
              &result_value);
  }
  if (exception_state.is_exception_set()) {
    return;
  }
  info.GetReturnValue().Set(result_value);
}

void previousAttributeSetter(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::Local<v8::Object> object = info.Holder();
  v8::Local<v8::Value> v8_value = info[0];

  V8cGlobalEnvironment* global_environment = V8cGlobalEnvironment::GetFromIsolate(isolate);
  WrapperFactory* wrapper_factory = global_environment->wrapper_factory();
  if (!WrapperPrivate::HasWrapperPrivate(object) ||
      !V8cGarbageCollectionTestInterface::GetTemplate(isolate)->HasInstance(object)) {
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
  GarbageCollectionTestInterface* impl =
      wrapper_private->wrappable<GarbageCollectionTestInterface>().get();
  TypeTraits<scoped_refptr<GarbageCollectionTestInterface> >::ConversionType value;
  FromJSValue(isolate, v8_value, (kConversionFlagNullable), &exception_state,
              &value);
  if (exception_state.is_exception_set()) {
    return;
  }

  impl->set_previous(value);
  result_value = v8::Undefined(isolate);
  return;
}


void nextAttributeGetter(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::Local<v8::Object> object = info.Holder();


  V8cGlobalEnvironment* global_environment = V8cGlobalEnvironment::GetFromIsolate(isolate);
  WrapperFactory* wrapper_factory = global_environment->wrapper_factory();
  if (!WrapperPrivate::HasWrapperPrivate(object) ||
      !V8cGarbageCollectionTestInterface::GetTemplate(isolate)->HasInstance(object)) {
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
  GarbageCollectionTestInterface* impl =
      wrapper_private->wrappable<GarbageCollectionTestInterface>().get();


  if (!exception_state.is_exception_set()) {
    ToJSValue(isolate,
              impl->next(),
              &result_value);
  }
  if (exception_state.is_exception_set()) {
    return;
  }
  info.GetReturnValue().Set(result_value);
}

void nextAttributeSetter(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::Local<v8::Object> object = info.Holder();
  v8::Local<v8::Value> v8_value = info[0];

  V8cGlobalEnvironment* global_environment = V8cGlobalEnvironment::GetFromIsolate(isolate);
  WrapperFactory* wrapper_factory = global_environment->wrapper_factory();
  if (!WrapperPrivate::HasWrapperPrivate(object) ||
      !V8cGarbageCollectionTestInterface::GetTemplate(isolate)->HasInstance(object)) {
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
  GarbageCollectionTestInterface* impl =
      wrapper_private->wrappable<GarbageCollectionTestInterface>().get();
  TypeTraits<scoped_refptr<GarbageCollectionTestInterface> >::ConversionType value;
  FromJSValue(isolate, v8_value, (kConversionFlagNullable), &exception_state,
              &value);
  if (exception_state.is_exception_set()) {
    return;
  }

  impl->set_next(value);
  result_value = v8::Undefined(isolate);
  return;
}



void InitializeTemplate(v8::Isolate* isolate) {
  // https://heycam.github.io/webidl/#interface-object
  // 3.6.1. Interface object
  //
  // The interface object for a given interface is a built-in function object.
  // It has properties that correspond to the constants and static operations
  // defined on that interface, as described in sections 3.6.6 Constants and
  // 3.6.8 Operations.
  //
  // If the interface is declared with a [Constructor] extended attribute,
  // then the interface object can be called as a constructor to create an
  // object that implements that interface. Calling that interface as a
  // function will throw an exception.
  //
  // Interface objects whose interfaces are not declared with a [Constructor]
  // extended attribute will throw when called, both as a function and as a
  // constructor.
  //
  // An interface object for a non-callback interface has an associated object
  // called the interface prototype object. This object has properties that
  // correspond to the regular attributes and regular operations defined on
  // the interface, and is described in more detail in 3.6.3 Interface
  // prototype object.
  v8::Local<v8::FunctionTemplate> function_template =
      v8::FunctionTemplate::New(
          isolate,
          Constructor,
          v8::Local<v8::Value>(),
          v8::Local<v8::Signature>(),
          0);
  function_template->SetLength(0);
  function_template->SetClassName(NewInternalString(isolate, "GarbageCollectionTestInterface"));
  function_template->ReadOnlyPrototype();

  v8::Local<v8::ObjectTemplate> prototype_template = function_template->PrototypeTemplate();
  v8::Local<v8::ObjectTemplate> instance_template = function_template->InstanceTemplate();
  instance_template->SetInternalFieldCount(WrapperPrivate::kInternalFieldCount);

  V8cGlobalEnvironment* global_environment = V8cGlobalEnvironment::GetFromIsolate(isolate);
  global_environment->AddInterfaceData(kInterfaceUniqueId, function_template);


  // https://heycam.github.io/webidl/#es-constants
  // 3.6.6. Constants
  //
  // For each exposed constant defined on an interface A, there must be a
  // corresponding property. The property has the following characteristics:

  // https://heycam.github.io/webidl/#es-attributes
  // 3.6.7. Attributes
  //
  // For each exposed attribute of the interface there must exist a
  // corresponding property. The characteristics of this property are as
  // follows:
  {
    // The name of the property is the identifier of the attribute.
    v8::Local<v8::String> name = NewInternalString(
        isolate,
        "previous");

    // The property has attributes { [[Get]]: G, [[Set]]: S, [[Enumerable]]:
    // true, [[Configurable]]: configurable }, where: configurable is false if
    // the attribute was declared with the [Unforgeable] extended attribute and
    // true otherwise;
    bool configurable = true;
    v8::PropertyAttribute attributes = static_cast<v8::PropertyAttribute>(
        configurable ? v8::None : v8::DontDelete);

    // G is the attribute getter created given the attribute, the interface, and
    // the relevant Realm of the object that is the location of the property;
    // and
    //
    // S is the attribute setter created given the attribute, the interface, and
    // the relevant Realm of the object that is the location of the property.
    v8::Local<v8::FunctionTemplate> getter =
        v8::FunctionTemplate::New(isolate, previousAttributeGetter);
    v8::Local<v8::FunctionTemplate> setter =
        v8::FunctionTemplate::New(isolate, previousAttributeSetter);

    // The location of the property is determined as follows:
    // Otherwise, the property exists solely on the interface's interface
    // prototype object.
    prototype_template->
        SetAccessorProperty(
            name,
            getter,
            setter,
            attributes);

  }
  {
    // The name of the property is the identifier of the attribute.
    v8::Local<v8::String> name = NewInternalString(
        isolate,
        "next");

    // The property has attributes { [[Get]]: G, [[Set]]: S, [[Enumerable]]:
    // true, [[Configurable]]: configurable }, where: configurable is false if
    // the attribute was declared with the [Unforgeable] extended attribute and
    // true otherwise;
    bool configurable = true;
    v8::PropertyAttribute attributes = static_cast<v8::PropertyAttribute>(
        configurable ? v8::None : v8::DontDelete);

    // G is the attribute getter created given the attribute, the interface, and
    // the relevant Realm of the object that is the location of the property;
    // and
    //
    // S is the attribute setter created given the attribute, the interface, and
    // the relevant Realm of the object that is the location of the property.
    v8::Local<v8::FunctionTemplate> getter =
        v8::FunctionTemplate::New(isolate, nextAttributeGetter);
    v8::Local<v8::FunctionTemplate> setter =
        v8::FunctionTemplate::New(isolate, nextAttributeSetter);

    // The location of the property is determined as follows:
    // Otherwise, the property exists solely on the interface's interface
    // prototype object.
    prototype_template->
        SetAccessorProperty(
            name,
            getter,
            setter,
            attributes);

  }

  // https://heycam.github.io/webidl/#es-operations
  // 3.6.8. Operations
  //
  // For each unique identifier of an exposed operation defined on the
  // interface, there must exist a corresponding property, unless the effective
  // overload set for that identifier and operation and with an argument count
  // of 0 has no entries.
  //
  // The characteristics of this property are as follows:

  // https://heycam.github.io/webidl/#es-stringifier
  // 3.6.8.2. Stringifiers
  prototype_template->Set(
      v8::Symbol::GetToStringTag(isolate),
      NewInternalString(isolate, "GarbageCollectionTestInterface"),
      static_cast<v8::PropertyAttribute>(v8::ReadOnly | v8::DontEnum));





}

}  // namespace


v8::Local<v8::Object> V8cGarbageCollectionTestInterface::CreateWrapper(
    v8::Isolate* isolate, const scoped_refptr<Wrappable>& wrappable) {
  EscapableEntryScope entry_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  V8cGlobalEnvironment* global_environment = V8cGlobalEnvironment::GetFromIsolate(isolate);
  if (!global_environment->HasInterfaceData(kInterfaceUniqueId)) {
    InitializeTemplate(isolate);
  }
  v8::Local<v8::FunctionTemplate> function_template = global_environment->GetInterfaceData(kInterfaceUniqueId);

  DCHECK(function_template->InstanceTemplate()->InternalFieldCount() == WrapperPrivate::kInternalFieldCount);
  v8::Local<v8::Object> object = function_template->InstanceTemplate()->NewInstance(context).ToLocalChecked();
  DCHECK(object->InternalFieldCount() == WrapperPrivate::kInternalFieldCount);

  // This |WrapperPrivate|'s lifetime will be managed by V8.
  new WrapperPrivate(isolate, wrappable, object);
  return entry_scope.Escape(object);
}


v8::Local<v8::FunctionTemplate> V8cGarbageCollectionTestInterface::GetTemplate(v8::Isolate* isolate) {
  V8cGlobalEnvironment* global_environment = V8cGlobalEnvironment::GetFromIsolate(isolate);
  if (!global_environment->HasInterfaceData(kInterfaceUniqueId)) {
    InitializeTemplate(isolate);
  }
  return global_environment->GetInterfaceData(kInterfaceUniqueId);
}


}  // namespace testing
}  // namespace bindings
}  // namespace cobalt


