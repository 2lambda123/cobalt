

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
// Auto-generated from template: bindings/mozjs45/templates/interface.cc.template

#include "cobalt/bindings/testing/mozjs_no_interface_object_interface.h"

#include "base/debug/trace_event.h"
#include "cobalt/base/polymorphic_downcast.h"
#include "cobalt/script/global_environment.h"
#include "cobalt/script/script_value.h"
#include "cobalt/script/value_handle.h"

#include "mozjs_gen_type_conversion.h"

#include "base/lazy_instance.h"
#include "cobalt/script/exception_state.h"
#include "cobalt/script/mozjs-45/callback_function_conversion.h"
#include "cobalt/script/mozjs-45/conversion_helpers.h"
#include "cobalt/script/mozjs-45/mozjs_callback_function.h"
#include "cobalt/script/mozjs-45/mozjs_exception_state.h"
#include "cobalt/script/mozjs-45/mozjs_global_environment.h"
#include "cobalt/script/mozjs-45/mozjs_property_enumerator.h"
#include "cobalt/script/mozjs-45/mozjs_user_object_holder.h"
#include "cobalt/script/mozjs-45/mozjs_value_handle.h"
#include "cobalt/script/mozjs-45/native_promise.h"
#include "cobalt/script/mozjs-45/proxy_handler.h"
#include "cobalt/script/mozjs-45/type_traits.h"
#include "cobalt/script/mozjs-45/wrapper_factory.h"
#include "cobalt/script/mozjs-45/wrapper_private.h"
#include "cobalt/script/property_enumerator.h"
#include "cobalt/script/sequence.h"
#include "third_party/mozjs-45/js/src/jsapi.h"
#include "third_party/mozjs-45/js/src/jsfriendapi.h"


namespace {
using cobalt::bindings::testing::NoInterfaceObjectInterface;
using cobalt::bindings::testing::MozjsNoInterfaceObjectInterface;
using cobalt::script::CallbackInterfaceTraits;
using cobalt::script::GlobalEnvironment;
using cobalt::script::ScriptValue;
using cobalt::script::ValueHandle;
using cobalt::script::ValueHandle;
using cobalt::script::ValueHandleHolder;
using cobalt::script::Wrappable;

using cobalt::script::CallbackFunction;
using cobalt::script::CallbackInterfaceTraits;
using cobalt::script::ExceptionState;
using cobalt::script::Wrappable;
using cobalt::script::mozjs::FromJSValue;
using cobalt::script::mozjs::InterfaceData;
using cobalt::script::mozjs::MozjsCallbackFunction;
using cobalt::script::mozjs::MozjsExceptionState;
using cobalt::script::mozjs::MozjsGlobalEnvironment;
using cobalt::script::mozjs::MozjsPropertyEnumerator;
using cobalt::script::mozjs::MozjsUserObjectHolder;
using cobalt::script::mozjs::ProxyHandler;
using cobalt::script::mozjs::ToJSValue;
using cobalt::script::mozjs::TypeTraits;
using cobalt::script::mozjs::WrapperFactory;
using cobalt::script::mozjs::WrapperPrivate;
using cobalt::script::mozjs::kConversionFlagClamped;
using cobalt::script::mozjs::kConversionFlagNullable;
using cobalt::script::mozjs::kConversionFlagRestricted;
using cobalt::script::mozjs::kConversionFlagTreatNullAsEmptyString;
using cobalt::script::mozjs::kConversionFlagTreatUndefinedAsEmptyString;
using cobalt::script::mozjs::kConversionFlagObjectOnly;
using cobalt::script::mozjs::kNoConversionFlags;
}  // namespace

namespace cobalt {
namespace bindings {
namespace testing {


namespace {







class MozjsNoInterfaceObjectInterfaceHandler : public ProxyHandler {
 public:
  MozjsNoInterfaceObjectInterfaceHandler()
      : ProxyHandler(indexed_property_hooks, named_property_hooks) {}

 private:
  static NamedPropertyHooks named_property_hooks;
  static IndexedPropertyHooks indexed_property_hooks;
};

ProxyHandler::NamedPropertyHooks
MozjsNoInterfaceObjectInterfaceHandler::named_property_hooks = {
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
};

ProxyHandler::IndexedPropertyHooks
MozjsNoInterfaceObjectInterfaceHandler::indexed_property_hooks = {
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
};

static base::LazyInstance<MozjsNoInterfaceObjectInterfaceHandler>
    proxy_handler;

bool DummyConstructor(JSContext* context, unsigned int argc, JS::Value* vp) {
  MozjsExceptionState exception(context);
  exception.SetSimpleException(
      script::kTypeError, "NoInterfaceObjectInterface is not constructible.");
  return false;
}


bool HasInstance(JSContext *context, JS::HandleObject type,
                   JS::MutableHandleValue vp, bool *success) {
  JS::RootedObject global_object(
      context, JS_GetGlobalForObject(context, type));
  DCHECK(global_object);

  JS::RootedObject prototype(
      context, MozjsNoInterfaceObjectInterface::GetPrototype(context, global_object));

  // |IsDelegate| walks the prototype chain of an object returning true if
  // .prototype is found.
  bool is_delegate;
  if (!IsDelegate(context, prototype, vp, &is_delegate)) {
    *success = false;
    return false;
  }

  *success = is_delegate;
  return true;
}

const JSClass instance_class_definition = {
    "NoInterfaceObjectInterface",
    0 | JSCLASS_HAS_PRIVATE,
    NULL,  // addProperty
    NULL,  // delProperty
    NULL,  // getProperty
    NULL,  // setProperty
    NULL,  // enumerate
    NULL,  // resolve
    NULL,  // mayResolve
    &WrapperPrivate::Finalizer,  // finalize
    NULL,  // call
    NULL,  // hasInstance
    NULL,  // construct
    &WrapperPrivate::Trace,  // trace
};

const JSClass prototype_class_definition = {
    "NoInterfaceObjectInterfacePrototype",
};

const JSClass interface_object_class_definition = {
    "NoInterfaceObjectInterfaceConstructor",
    0,
    NULL,  // addProperty
    NULL,  // delProperty
    NULL,  // getProperty
    NULL,  // setProperty
    NULL,  // enumerate
    NULL,  // resolve
    NULL,  // mayResolve
    NULL,  // finalize
    NULL,  // call
    &HasInstance,
    NULL,
};



const JSPropertySpec prototype_properties[] = {

  JS_PS_END
};

const JSFunctionSpec prototype_functions[] = {
  JS_FS_END
};

const JSPropertySpec interface_object_properties[] = {

  JS_PS_END
};

const JSFunctionSpec interface_object_functions[] = {
  JS_FS_END
};

const JSPropertySpec own_properties[] = {
  JS_PS_END
};

void InitializePrototypeAndInterfaceObject(
    InterfaceData* interface_data, JSContext* context,
    JS::HandleObject global_object) {
  DCHECK(!interface_data->prototype);
  DCHECK(!interface_data->interface_object);
  DCHECK(JS_IsGlobalObject(global_object));

  JS::RootedObject parent_prototype(
      context, JS_GetObjectPrototype(context, global_object));
  DCHECK(parent_prototype);

  interface_data->prototype = JS_NewObjectWithGivenProto(
    context, &prototype_class_definition, parent_prototype
  );

  JS::RootedObject rooted_prototype(context, interface_data->prototype);
  bool success = JS_DefineProperties(
      context,
      rooted_prototype,
      prototype_properties);

  DCHECK(success);
  success = JS_DefineFunctions(
      context, rooted_prototype, prototype_functions);
  DCHECK(success);

}

inline InterfaceData* GetInterfaceData(JSContext* context) {
  const int kInterfaceUniqueId = 37;
  MozjsGlobalEnvironment* global_environment =
      static_cast<MozjsGlobalEnvironment*>(JS_GetContextPrivate(context));
  // By convention, the |MozjsGlobalEnvironment| that we are associated with
  // will hold our |InterfaceData| at index |kInterfaceUniqueId|, as we asked
  // for it to be there in the first place, and could not have conflicted with
  // any other interface.
  return global_environment->GetInterfaceData(kInterfaceUniqueId);
}

}  // namespace

// static
JSObject* MozjsNoInterfaceObjectInterface::CreateProxy(
    JSContext* context, const scoped_refptr<Wrappable>& wrappable) {
  DCHECK(MozjsGlobalEnvironment::GetFromContext(context));
  JS::RootedObject global_object(
      context,
      MozjsGlobalEnvironment::GetFromContext(context)->global_object());
  DCHECK(global_object);

  InterfaceData* interface_data = GetInterfaceData(context);
  JS::RootedObject prototype(context, GetPrototype(context, global_object));
  DCHECK(prototype);
  JS::RootedObject new_object(
      context,
      JS_NewObjectWithGivenProto(
          context, &instance_class_definition, prototype));
  DCHECK(new_object);
  JS::RootedObject proxy(context,
      ProxyHandler::NewProxy(
          context, proxy_handler.Pointer(), new_object, prototype));
  WrapperPrivate::AddPrivateData(context, proxy, wrappable);
  return proxy;
}

// static
const JSClass* MozjsNoInterfaceObjectInterface::PrototypeClass(
      JSContext* context) {
  DCHECK(MozjsGlobalEnvironment::GetFromContext(context));
  JS::RootedObject global_object(
      context,
      MozjsGlobalEnvironment::GetFromContext(context)->global_object());
  DCHECK(global_object);

  JS::RootedObject prototype(context, GetPrototype(context, global_object));
  const JSClass* proto_class = JS_GetClass(prototype);
  return proto_class;
}

// static
JSObject* MozjsNoInterfaceObjectInterface::GetPrototype(
    JSContext* context, JS::HandleObject global_object) {
  DCHECK(JS_IsGlobalObject(global_object));

  InterfaceData* interface_data = GetInterfaceData(context);
  if (!interface_data->prototype) {
    // Create new prototype that has all the props and methods
    InitializePrototypeAndInterfaceObject(
        interface_data, context, global_object);
  }
  DCHECK(interface_data->prototype);
  return interface_data->prototype;
}


namespace {


}  // namespace


}  // namespace testing
}  // namespace bindings
}  // namespace cobalt


