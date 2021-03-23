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
#ifndef COBALT_SCRIPT_MOZJS_MOZJS_USER_OBJECT_HOLDER_H_
#define COBALT_SCRIPT_MOZJS_MOZJS_USER_OBJECT_HOLDER_H_

#include "base/hash_tables.h"
#include "base/memory/weak_ptr.h"
#include "cobalt/base/polymorphic_downcast.h"
#include "cobalt/script/mozjs/mozjs_global_environment.h"
#include "cobalt/script/mozjs/referenced_object_map.h"
#include "cobalt/script/mozjs/util/algorithm_helpers.h"
#include "cobalt/script/mozjs/wrapper_factory.h"
#include "cobalt/script/mozjs/wrapper_private.h"
#include "cobalt/script/script_value.h"
#include "nb/memory_scope.h"
#include "third_party/mozjs/js/src/jsapi.h"

namespace cobalt {
namespace script {
namespace mozjs {

// Template class that implements the ScriptValue template class for lifetime
// management of User Objects passed from the bindings layer into Cobalt. See
// the definition of ScriptValue for further detail.
// This class does not root the underlying JSObject that MozjsUserObjectType
// holds a reference to. The object will be traced when any owning objects are
// traced.
template <typename MozjsUserObjectType>
class MozjsUserObjectHolder
    : public ScriptValue<typename MozjsUserObjectType::BaseType> {
 public:
  typedef ScriptValue<typename MozjsUserObjectType::BaseType> BaseClass;

  MozjsUserObjectHolder()
      : context_(NULL),
        wrapper_factory_(NULL),
        prevent_garbage_collection_count_(0) {}

  MozjsUserObjectHolder(JS::HandleObject object, JSContext* context,
                        WrapperFactory* wrapper_factory)
      : handle_(MozjsUserObjectType(context, object)),
        context_(context),
        wrapper_factory_(wrapper_factory),
        prevent_garbage_collection_count_(0) {}

  MozjsUserObjectHolder(JS::HandleValue value, JSContext* context,
                        WrapperFactory* wrapper_factory)
      : handle_(MozjsUserObjectType(context, value)),
        context_(context),
        wrapper_factory_(wrapper_factory),
        prevent_garbage_collection_count_(0) {}

  ~MozjsUserObjectHolder() {
    DCHECK_EQ(prevent_garbage_collection_count_, 0);
    DCHECK(!persistent_root_);
  }

  void RegisterOwner(Wrappable* owner) OVERRIDE {
    JSAutoRequest auto_request(context_);
    JS::RootedValue owned_value(context_, js_value());
    DLOG_IF(WARNING, handle_->WasCollected())
        << "Owned value has been garbage collected.";
    // We have to check for null, because null is apparently a GC Thing.
    if (!owned_value.isNullOrUndefined() && owned_value.isGCThing()) {
      MozjsGlobalEnvironment* global_environment =
          MozjsGlobalEnvironment::GetFromContext(context_);
      intptr_t key = ReferencedObjectMap::GetKeyForWrappable(owner);
      global_environment->referenced_objects()->AddReferencedObject(
          key, owned_value);
    }
  }

  void DeregisterOwner(Wrappable* owner) OVERRIDE {
    // |owner| may be in the process of being destructed, so don't use it.
    JSAutoRequest auto_request(context_);
    JS::RootedValue owned_value(context_, js_value());
    // We have to check for null, because null is apparently a GC Thing.
    if (!owned_value.isNullOrUndefined() && owned_value.isGCThing()) {
      MozjsGlobalEnvironment* global_environment =
          MozjsGlobalEnvironment::GetFromContext(context_);
      intptr_t key = ReferencedObjectMap::GetKeyForWrappable(owner);
      global_environment->referenced_objects()->RemoveReferencedObject(
          key, owned_value);
    }
  }

  void PreventGarbageCollection() OVERRIDE {
    if (prevent_garbage_collection_count_++ == 0 && handle_) {
      JSAutoRequest auto_request(context_);
      persistent_root_ = handle_->value();
      JSBool result = JS_AddNamedValueRoot(
          context_, &persistent_root_.value(),
          "MozjsUserObjectHolder::PreventGarbageCollection");
      DCHECK(result);
    }
  }

  void AllowGarbageCollection() OVERRIDE {
    if (--prevent_garbage_collection_count_ == 0 && handle_) {
      JSAutoRequest auto_request(context_);
      JS_RemoveValueRoot(context_, &persistent_root_.value());
      persistent_root_ = base::nullopt;
    }
  }

  const typename MozjsUserObjectType::BaseType* GetScriptValue()
      const OVERRIDE {
    return handle_ ? &handle_.value() : NULL;
  }

  scoped_ptr<BaseClass> MakeCopy() const OVERRIDE {
    TRACK_MEMORY_SCOPE("Javascript");
    DCHECK(handle_);
    JSAutoRequest auto_request(context_);
    JS::RootedValue rooted_value(context_, js_value());
    return make_scoped_ptr<BaseClass>(
        new MozjsUserObjectHolder(rooted_value, context_, wrapper_factory_));
  }

  bool EqualTo(const BaseClass& other) const OVERRIDE {
    const MozjsUserObjectHolder* mozjs_other =
        base::polymorphic_downcast<const MozjsUserObjectHolder*>(&other);
    if (!handle_) {
      return !mozjs_other->handle_;
    } else if (!mozjs_other->handle_) {
      return false;
    }

    DCHECK(handle_);
    DCHECK(mozjs_other->handle_);

    JS::RootedValue value1(context_, js_value());
    JS::RootedValue value2(context_, mozjs_other->js_value());
    return util::IsSameGcThing(context_, value1, value2);
  }

  const JS::Value& js_value() const {
    DCHECK(handle_);
    return handle_->value();
  }

  JSObject* js_object() const {
    DCHECK(handle_);
    return handle_->handle();
  }

 private:
  typedef base::hash_map<const Wrappable*, base::WeakPtr<WrapperPrivate> >
      WrappableAndPrivateHashMap;

  base::optional<MozjsUserObjectType> handle_;
  JSContext* context_;
  WrapperFactory* wrapper_factory_;
  int prevent_garbage_collection_count_;
  base::optional<JS::Value> persistent_root_;
};

}  // namespace mozjs
}  // namespace script
}  // namespace cobalt

#endif  // COBALT_SCRIPT_MOZJS_MOZJS_USER_OBJECT_HOLDER_H_