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
// Auto-generated from template: bindings/javascriptcore/templates/interface.h.template

#ifndef JSCWindow_h
#define JSCWindow_h

#include "base/hash_tables.h"
#include "base/lazy_instance.h"
#include "base/memory/ref_counted.h"
#include "base/threading/thread_checker.h"
#include "cobalt/base/polymorphic_downcast.h"
#include "cobalt/script/wrappable.h"
#include "JSCGlobalInterfaceParent.h"
#include "cobalt/bindings/testing/window.h"

#include "base/threading/thread_local_storage.h"
#include "cobalt/script/javascriptcore/jsc_global_object.h"
#include "cobalt/script/javascriptcore/script_object_registry.h"
#include "cobalt/script/javascriptcore/thread_local_hash_table.h"
#include "cobalt/script/javascriptcore/wrapper_base.h"
#include "cobalt/script/javascriptcore/wrapper_factory.h"
#include "third_party/WebKit/Source/JavaScriptCore/config.h"
#include "third_party/WebKit/Source/JavaScriptCore/runtime/ClassInfo.h"
#include "third_party/WebKit/Source/JavaScriptCore/runtime/JSObject.h"
#include "third_party/WebKit/Source/JavaScriptCore/runtime/Lookup.h"

namespace cobalt {
namespace bindings {
namespace testing {

class JSCWindow
    : public script::javascriptcore::JSCGlobalObject {
  typedef script::javascriptcore::JSCGlobalObject BaseClass;
 public:

  // Get the prototype object for this wrapper class.
  static JSC::JSObject* GetPrototype(JSC::JSGlobalObject* global_object);

  // Get the interface object for this wrapper class.
  static JSC::JSObject* GetConstructor(JSC::ExecState* exec_state);

  // JavaScriptCore functions and members

  DECLARE_CLASSINFO();

  // Needed when JSC::OverridesGetOwnPropertySlot StructureFlag is set
  // Must be public so that it can be accessible from getStaticValueSlot<>.
  static bool getOwnPropertySlot(JSC::JSCell*, JSC::ExecState*,
                                 JSC::PropertyName,
                                 JSC::PropertySlot&);

  // static override. Needed to support setting a property.
  static void put(JSC::JSCell*, JSC::ExecState*, JSC::PropertyName,
                  JSC::JSValue, JSC::PutPropertySlot&);

  // static override. This function will be called after a new object has
  // been created.
  void finishCreation(JSC::JSGlobalData& global_data);

  static script::javascriptcore::JSCGlobalObject* Create(
      const scoped_refptr<Window>& global_interface,
      script::EnvironmentSettings* environment_settings,
      JSC::JSGlobalData* global_data,
      script::javascriptcore::ScriptObjectRegistry* script_object_registry);

 protected:

  static const unsigned StructureFlags =
      JSC::OverridesGetOwnPropertySlot |
      BaseClass::StructureFlags;

  JSCWindow(
      JSC::JSGlobalData* global_data,
      JSC::Structure* structure,
      script::javascriptcore::ScriptObjectRegistry* script_object_registry,
      scoped_ptr<script::javascriptcore::WrapperFactory> wrapper_factory,
      script::EnvironmentSettings* environment_settings,
      const scoped_refptr<Window>& impl);
  ~JSCWindow();

 private:
  class InterfaceObject;
  class Prototype;

  static const JSC::HashTableValue property_table_values[];
  static const JSC::HashTable property_table_prototype;
  static base::LazyInstance<
      cobalt::script::javascriptcore::ThreadLocalHashTable>
          thread_local_property_table;

  static const JSC::HashTable* GetPropertyTable(JSC::ExecState* exec_state);

  static bool HasOwnPropertyOrPrototypeProperty(JSC::JSCell* cell,
      JSC::ExecState* exec_state, JSC::PropertyName property_name);

#ifdef __LB_SHELL__FORCE_LOGGING__
  struct NonTrivialStaticFields {
    // TODO(***REMOVED***): Only log attempts of usage of unsupported Web APIs
    //               (b/24548567).
    base::hash_set<std::string> properties_warned_about;
    base::Lock lock_;
  };
  static base::LazyInstance<NonTrivialStaticFields> non_trivial_static_fields;
#endif  // __LB_SHELL__FORCE_LOGGING__
};

}  // namespace bindings
}  // namespace testing
}  // namespace cobalt

#endif  // JSCWindow_h
