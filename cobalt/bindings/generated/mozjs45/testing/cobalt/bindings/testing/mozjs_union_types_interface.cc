// Copyright 2017 Google Inc. All Rights Reserved.
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

#include "cobalt/bindings/testing/mozjs_union_types_interface.h"

#include "base/debug/trace_event.h"
#include "cobalt/base/polymorphic_downcast.h"
#include "cobalt/script/global_environment.h"
#include "cobalt/script/opaque_handle.h"
#include "cobalt/script/script_value.h"
#include "cobalt/bindings/testing/arbitrary_interface.h"
#include "cobalt/bindings/testing/base_interface.h"
#include "cobalt/bindings/testing/mozjs_arbitrary_interface.h"
#include "cobalt/bindings/testing/mozjs_base_interface.h"

#include "mozjs_gen_type_conversion.h"

#include "base/lazy_instance.h"
#include "cobalt/script/exception_state.h"
#include "cobalt/script/mozjs-45/callback_function_conversion.h"
#include "cobalt/script/mozjs-45/conversion_helpers.h"
#include "cobalt/script/mozjs-45/mozjs_callback_function.h"
#include "cobalt/script/mozjs-45/mozjs_exception_state.h"
#include "cobalt/script/mozjs-45/mozjs_global_environment.h"
#include "cobalt/script/mozjs-45/mozjs_object_handle.h"
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
using cobalt::bindings::testing::UnionTypesInterface;
using cobalt::bindings::testing::MozjsUnionTypesInterface;
using cobalt::bindings::testing::ArbitraryInterface;
using cobalt::bindings::testing::BaseInterface;
using cobalt::bindings::testing::MozjsArbitraryInterface;
using cobalt::bindings::testing::MozjsBaseInterface;
using cobalt::script::CallbackInterfaceTraits;
using cobalt::script::GlobalEnvironment;
using cobalt::script::OpaqueHandle;
using cobalt::script::OpaqueHandleHolder;
using cobalt::script::ScriptValue;
using cobalt::script::ValueHandle;
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
using cobalt::script::mozjs::kNoConversionFlags;
}  // namespace

namespace cobalt {
namespace bindings {
namespace testing {

namespace {

class MozjsUnionTypesInterfaceHandler : public ProxyHandler {
 public:
  MozjsUnionTypesInterfaceHandler()
      : ProxyHandler(indexed_property_hooks, named_property_hooks) {}

 private:
  static NamedPropertyHooks named_property_hooks;
  static IndexedPropertyHooks indexed_property_hooks;
};

ProxyHandler::NamedPropertyHooks
MozjsUnionTypesInterfaceHandler::named_property_hooks = {
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
};
ProxyHandler::IndexedPropertyHooks
MozjsUnionTypesInterfaceHandler::indexed_property_hooks = {
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
};

static base::LazyInstance<MozjsUnionTypesInterfaceHandler>
    proxy_handler;

bool HasInstance(JSContext *context, JS::HandleObject type,
                   JS::MutableHandleValue vp, bool *success) {
  JS::RootedObject global_object(
      context, JS_GetGlobalForObject(context, type));
  DCHECK(global_object);

  JS::RootedObject prototype(
      context, MozjsUnionTypesInterface::GetPrototype(context, global_object));

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
    "UnionTypesInterface",
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
    "UnionTypesInterfacePrototype",
};

const JSClass interface_object_class_definition = {
    "UnionTypesInterfaceConstructor",
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

bool get_unionProperty(
    JSContext* context, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.thisv().isObject()) {
    MozjsExceptionState exception(context);
    exception.SetSimpleException(script::kTypeError, "Invalid this.");
    return false;
  }
  JS::RootedObject object(context, &args.thisv().toObject());
  const JSClass* proto_class =
      MozjsUnionTypesInterface::PrototypeClass(context);
  if (proto_class == JS_GetClass(object)) {
    // Simply returns true if the object is this class's prototype object.
    // There is no need to return any value due to the object is not a platform
    // object. The execution reaches here when Object.getOwnPropertyDescriptor
    // gets called on native object prototypes.
    return true;
  }

  MozjsGlobalEnvironment* global_environment =
      static_cast<MozjsGlobalEnvironment*>(JS_GetContextPrivate(context));
  WrapperFactory* wrapper_factory = global_environment->wrapper_factory();
  if (!wrapper_factory->DoesObjectImplementInterface(
        object, base::GetTypeId<UnionTypesInterface>())) {
    MozjsExceptionState exception(context);
    exception.SetSimpleException(script::kDoesNotImplementInterface);
    return false;
  }
  MozjsExceptionState exception_state(context);
  JS::RootedValue result_value(context);

  WrapperPrivate* wrapper_private =
      WrapperPrivate::GetFromObject(context, object);
  UnionTypesInterface* impl =
      wrapper_private->wrappable<UnionTypesInterface>().get();

  if (!exception_state.is_exception_set()) {
    ToJSValue(context,
              impl->union_property(),
              &result_value);
  }
  if (!exception_state.is_exception_set()) {
    args.rval().set(result_value);
  }
  return !exception_state.is_exception_set();
}

bool set_unionProperty(
    JSContext* context, unsigned argc, JS::Value* vp) {

  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.thisv().isObject()) {
    MozjsExceptionState exception(context);
    exception.SetSimpleException(script::kTypeError, "Invalid this.");
    return false;
  }
  JS::RootedObject object(context, &args.thisv().toObject());

  const JSClass* proto_class =
      MozjsUnionTypesInterface::PrototypeClass(context);
  if (proto_class == JS_GetClass(object)) {
    // Simply returns true if the object is this class's prototype object.
    // There is no need to return any value due to the object is not a platform
    // object. The execution reaches here when Object.getOwnPropertyDescriptor
    // gets called on native object prototypes.
    return true;
  }

  MozjsGlobalEnvironment* global_environment =
      static_cast<MozjsGlobalEnvironment*>(JS_GetContextPrivate(context));
  WrapperFactory* wrapper_factory = global_environment->wrapper_factory();
  if (!wrapper_factory->DoesObjectImplementInterface(
        object, base::GetTypeId<UnionTypesInterface>())) {
    MozjsExceptionState exception(context);
    exception.SetSimpleException(script::kDoesNotImplementInterface);
    return false;
  }
  MozjsExceptionState exception_state(context);
  JS::RootedValue result_value(context);

  WrapperPrivate* wrapper_private =
      WrapperPrivate::GetFromObject(context, object);
  UnionTypesInterface* impl =
      wrapper_private->wrappable<UnionTypesInterface>().get();
  TypeTraits<script::UnionType4<std::string, bool, scoped_refptr<ArbitraryInterface>, int32_t > >::ConversionType value;
  if (args.length() != 1) {
    NOTREACHED();
    return false;
  }
  FromJSValue(context, args[0], kNoConversionFlags, &exception_state,
              &value);
  if (exception_state.is_exception_set()) {
    return false;
  }

  impl->set_union_property(value);
  result_value.set(JS::UndefinedHandleValue);
  return !exception_state.is_exception_set();
}

bool get_unionWithNullableMemberProperty(
    JSContext* context, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.thisv().isObject()) {
    MozjsExceptionState exception(context);
    exception.SetSimpleException(script::kTypeError, "Invalid this.");
    return false;
  }
  JS::RootedObject object(context, &args.thisv().toObject());
  const JSClass* proto_class =
      MozjsUnionTypesInterface::PrototypeClass(context);
  if (proto_class == JS_GetClass(object)) {
    // Simply returns true if the object is this class's prototype object.
    // There is no need to return any value due to the object is not a platform
    // object. The execution reaches here when Object.getOwnPropertyDescriptor
    // gets called on native object prototypes.
    return true;
  }

  MozjsGlobalEnvironment* global_environment =
      static_cast<MozjsGlobalEnvironment*>(JS_GetContextPrivate(context));
  WrapperFactory* wrapper_factory = global_environment->wrapper_factory();
  if (!wrapper_factory->DoesObjectImplementInterface(
        object, base::GetTypeId<UnionTypesInterface>())) {
    MozjsExceptionState exception(context);
    exception.SetSimpleException(script::kDoesNotImplementInterface);
    return false;
  }
  MozjsExceptionState exception_state(context);
  JS::RootedValue result_value(context);

  WrapperPrivate* wrapper_private =
      WrapperPrivate::GetFromObject(context, object);
  UnionTypesInterface* impl =
      wrapper_private->wrappable<UnionTypesInterface>().get();

  if (!exception_state.is_exception_set()) {
    ToJSValue(context,
              impl->union_with_nullable_member_property(),
              &result_value);
  }
  if (!exception_state.is_exception_set()) {
    args.rval().set(result_value);
  }
  return !exception_state.is_exception_set();
}

bool set_unionWithNullableMemberProperty(
    JSContext* context, unsigned argc, JS::Value* vp) {

  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.thisv().isObject()) {
    MozjsExceptionState exception(context);
    exception.SetSimpleException(script::kTypeError, "Invalid this.");
    return false;
  }
  JS::RootedObject object(context, &args.thisv().toObject());

  const JSClass* proto_class =
      MozjsUnionTypesInterface::PrototypeClass(context);
  if (proto_class == JS_GetClass(object)) {
    // Simply returns true if the object is this class's prototype object.
    // There is no need to return any value due to the object is not a platform
    // object. The execution reaches here when Object.getOwnPropertyDescriptor
    // gets called on native object prototypes.
    return true;
  }

  MozjsGlobalEnvironment* global_environment =
      static_cast<MozjsGlobalEnvironment*>(JS_GetContextPrivate(context));
  WrapperFactory* wrapper_factory = global_environment->wrapper_factory();
  if (!wrapper_factory->DoesObjectImplementInterface(
        object, base::GetTypeId<UnionTypesInterface>())) {
    MozjsExceptionState exception(context);
    exception.SetSimpleException(script::kDoesNotImplementInterface);
    return false;
  }
  MozjsExceptionState exception_state(context);
  JS::RootedValue result_value(context);

  WrapperPrivate* wrapper_private =
      WrapperPrivate::GetFromObject(context, object);
  UnionTypesInterface* impl =
      wrapper_private->wrappable<UnionTypesInterface>().get();
  TypeTraits<base::optional<script::UnionType2<double, std::string > > >::ConversionType value;
  if (args.length() != 1) {
    NOTREACHED();
    return false;
  }
  FromJSValue(context, args[0], kNoConversionFlags, &exception_state,
              &value);
  if (exception_state.is_exception_set()) {
    return false;
  }

  impl->set_union_with_nullable_member_property(value);
  result_value.set(JS::UndefinedHandleValue);
  return !exception_state.is_exception_set();
}

bool get_nullableUnionProperty(
    JSContext* context, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.thisv().isObject()) {
    MozjsExceptionState exception(context);
    exception.SetSimpleException(script::kTypeError, "Invalid this.");
    return false;
  }
  JS::RootedObject object(context, &args.thisv().toObject());
  const JSClass* proto_class =
      MozjsUnionTypesInterface::PrototypeClass(context);
  if (proto_class == JS_GetClass(object)) {
    // Simply returns true if the object is this class's prototype object.
    // There is no need to return any value due to the object is not a platform
    // object. The execution reaches here when Object.getOwnPropertyDescriptor
    // gets called on native object prototypes.
    return true;
  }

  MozjsGlobalEnvironment* global_environment =
      static_cast<MozjsGlobalEnvironment*>(JS_GetContextPrivate(context));
  WrapperFactory* wrapper_factory = global_environment->wrapper_factory();
  if (!wrapper_factory->DoesObjectImplementInterface(
        object, base::GetTypeId<UnionTypesInterface>())) {
    MozjsExceptionState exception(context);
    exception.SetSimpleException(script::kDoesNotImplementInterface);
    return false;
  }
  MozjsExceptionState exception_state(context);
  JS::RootedValue result_value(context);

  WrapperPrivate* wrapper_private =
      WrapperPrivate::GetFromObject(context, object);
  UnionTypesInterface* impl =
      wrapper_private->wrappable<UnionTypesInterface>().get();

  if (!exception_state.is_exception_set()) {
    ToJSValue(context,
              impl->nullable_union_property(),
              &result_value);
  }
  if (!exception_state.is_exception_set()) {
    args.rval().set(result_value);
  }
  return !exception_state.is_exception_set();
}

bool set_nullableUnionProperty(
    JSContext* context, unsigned argc, JS::Value* vp) {

  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.thisv().isObject()) {
    MozjsExceptionState exception(context);
    exception.SetSimpleException(script::kTypeError, "Invalid this.");
    return false;
  }
  JS::RootedObject object(context, &args.thisv().toObject());

  const JSClass* proto_class =
      MozjsUnionTypesInterface::PrototypeClass(context);
  if (proto_class == JS_GetClass(object)) {
    // Simply returns true if the object is this class's prototype object.
    // There is no need to return any value due to the object is not a platform
    // object. The execution reaches here when Object.getOwnPropertyDescriptor
    // gets called on native object prototypes.
    return true;
  }

  MozjsGlobalEnvironment* global_environment =
      static_cast<MozjsGlobalEnvironment*>(JS_GetContextPrivate(context));
  WrapperFactory* wrapper_factory = global_environment->wrapper_factory();
  if (!wrapper_factory->DoesObjectImplementInterface(
        object, base::GetTypeId<UnionTypesInterface>())) {
    MozjsExceptionState exception(context);
    exception.SetSimpleException(script::kDoesNotImplementInterface);
    return false;
  }
  MozjsExceptionState exception_state(context);
  JS::RootedValue result_value(context);

  WrapperPrivate* wrapper_private =
      WrapperPrivate::GetFromObject(context, object);
  UnionTypesInterface* impl =
      wrapper_private->wrappable<UnionTypesInterface>().get();
  TypeTraits<base::optional<script::UnionType2<double, std::string > > >::ConversionType value;
  if (args.length() != 1) {
    NOTREACHED();
    return false;
  }
  FromJSValue(context, args[0], (kConversionFlagNullable), &exception_state,
              &value);
  if (exception_state.is_exception_set()) {
    return false;
  }

  impl->set_nullable_union_property(value);
  result_value.set(JS::UndefinedHandleValue);
  return !exception_state.is_exception_set();
}

bool get_unionBaseProperty(
    JSContext* context, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.thisv().isObject()) {
    MozjsExceptionState exception(context);
    exception.SetSimpleException(script::kTypeError, "Invalid this.");
    return false;
  }
  JS::RootedObject object(context, &args.thisv().toObject());
  const JSClass* proto_class =
      MozjsUnionTypesInterface::PrototypeClass(context);
  if (proto_class == JS_GetClass(object)) {
    // Simply returns true if the object is this class's prototype object.
    // There is no need to return any value due to the object is not a platform
    // object. The execution reaches here when Object.getOwnPropertyDescriptor
    // gets called on native object prototypes.
    return true;
  }

  MozjsGlobalEnvironment* global_environment =
      static_cast<MozjsGlobalEnvironment*>(JS_GetContextPrivate(context));
  WrapperFactory* wrapper_factory = global_environment->wrapper_factory();
  if (!wrapper_factory->DoesObjectImplementInterface(
        object, base::GetTypeId<UnionTypesInterface>())) {
    MozjsExceptionState exception(context);
    exception.SetSimpleException(script::kDoesNotImplementInterface);
    return false;
  }
  MozjsExceptionState exception_state(context);
  JS::RootedValue result_value(context);

  WrapperPrivate* wrapper_private =
      WrapperPrivate::GetFromObject(context, object);
  UnionTypesInterface* impl =
      wrapper_private->wrappable<UnionTypesInterface>().get();

  if (!exception_state.is_exception_set()) {
    ToJSValue(context,
              impl->union_base_property(),
              &result_value);
  }
  if (!exception_state.is_exception_set()) {
    args.rval().set(result_value);
  }
  return !exception_state.is_exception_set();
}

bool set_unionBaseProperty(
    JSContext* context, unsigned argc, JS::Value* vp) {

  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.thisv().isObject()) {
    MozjsExceptionState exception(context);
    exception.SetSimpleException(script::kTypeError, "Invalid this.");
    return false;
  }
  JS::RootedObject object(context, &args.thisv().toObject());

  const JSClass* proto_class =
      MozjsUnionTypesInterface::PrototypeClass(context);
  if (proto_class == JS_GetClass(object)) {
    // Simply returns true if the object is this class's prototype object.
    // There is no need to return any value due to the object is not a platform
    // object. The execution reaches here when Object.getOwnPropertyDescriptor
    // gets called on native object prototypes.
    return true;
  }

  MozjsGlobalEnvironment* global_environment =
      static_cast<MozjsGlobalEnvironment*>(JS_GetContextPrivate(context));
  WrapperFactory* wrapper_factory = global_environment->wrapper_factory();
  if (!wrapper_factory->DoesObjectImplementInterface(
        object, base::GetTypeId<UnionTypesInterface>())) {
    MozjsExceptionState exception(context);
    exception.SetSimpleException(script::kDoesNotImplementInterface);
    return false;
  }
  MozjsExceptionState exception_state(context);
  JS::RootedValue result_value(context);

  WrapperPrivate* wrapper_private =
      WrapperPrivate::GetFromObject(context, object);
  UnionTypesInterface* impl =
      wrapper_private->wrappable<UnionTypesInterface>().get();
  TypeTraits<script::UnionType2<scoped_refptr<BaseInterface>, std::string > >::ConversionType value;
  if (args.length() != 1) {
    NOTREACHED();
    return false;
  }
  FromJSValue(context, args[0], kNoConversionFlags, &exception_state,
              &value);
  if (exception_state.is_exception_set()) {
    return false;
  }

  impl->set_union_base_property(value);
  result_value.set(JS::UndefinedHandleValue);
  return !exception_state.is_exception_set();
}



const JSPropertySpec prototype_properties[] = {
  {  // Read/Write property
    "unionProperty",
    JSPROP_SHARED | JSPROP_ENUMERATE,
    { { &get_unionProperty, NULL } },
    { { &set_unionProperty, NULL } },
  },
  {  // Read/Write property
    "unionWithNullableMemberProperty",
    JSPROP_SHARED | JSPROP_ENUMERATE,
    { { &get_unionWithNullableMemberProperty, NULL } },
    { { &set_unionWithNullableMemberProperty, NULL } },
  },
  {  // Read/Write property
    "nullableUnionProperty",
    JSPROP_SHARED | JSPROP_ENUMERATE,
    { { &get_nullableUnionProperty, NULL } },
    { { &set_nullableUnionProperty, NULL } },
  },
  {  // Read/Write property
    "unionBaseProperty",
    JSPROP_SHARED | JSPROP_ENUMERATE,
    { { &get_unionBaseProperty, NULL } },
    { { &set_unionBaseProperty, NULL } },
  },
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

  JS::RootedObject function_prototype(
      context, JS_GetFunctionPrototype(context, global_object));
  DCHECK(function_prototype);
  // Create the Interface object.
  interface_data->interface_object = JS_NewObjectWithGivenProto(
      context, &interface_object_class_definition,
      function_prototype);

  // Add the InterfaceObject.name property.
  JS::RootedObject rooted_interface_object(
      context, interface_data->interface_object);
  JS::RootedValue name_value(context);
  const char name[] =
      "UnionTypesInterface";
  name_value.setString(JS_NewStringCopyZ(context, name));
  success = JS_DefineProperty(
      context, rooted_interface_object, "name", name_value, JSPROP_READONLY,
      NULL, NULL);
  DCHECK(success);

  // Define interface object properties (including constants).
  success = JS_DefineProperties(context, rooted_interface_object,
                                interface_object_properties);
  DCHECK(success);
  // Define interface object functions (static).
  success = JS_DefineFunctions(context, rooted_interface_object,
                               interface_object_functions);
  DCHECK(success);

  // Set the Prototype.constructor and Constructor.prototype properties.
  DCHECK(interface_data->interface_object);
  DCHECK(interface_data->prototype);
  success = JS_LinkConstructorAndPrototype(
      context,
      rooted_interface_object,
      rooted_prototype);
  DCHECK(success);
}

inline InterfaceData* GetInterfaceData(JSContext* context) {
  const int kInterfaceUniqueId = 48;
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
JSObject* MozjsUnionTypesInterface::CreateProxy(
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
const JSClass* MozjsUnionTypesInterface::PrototypeClass(
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
JSObject* MozjsUnionTypesInterface::GetPrototype(
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

// static
JSObject* MozjsUnionTypesInterface::GetInterfaceObject(
    JSContext* context, JS::HandleObject global_object) {
  DCHECK(JS_IsGlobalObject(global_object));

  InterfaceData* interface_data = GetInterfaceData(context);
  if (!interface_data->interface_object) {
    InitializePrototypeAndInterfaceObject(
        interface_data, context, global_object);
  }
  DCHECK(interface_data->interface_object);
  return interface_data->interface_object;
}


namespace {
}  // namespace


}  // namespace testing
}  // namespace bindings
}  // namespace cobalt
