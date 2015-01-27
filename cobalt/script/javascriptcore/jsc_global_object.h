/*
 * Copyright 2014 Google Inc. All Rights Reserved.
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
#ifndef SCRIPT_JAVASCRIPTCORE_JSC_GLOBAL_OBJECT_H_
#define SCRIPT_JAVASCRIPTCORE_JSC_GLOBAL_OBJECT_H_

#include "base/hash_tables.h"

#include "config.h"
#undef LOG  // Defined by WTF, also redefined by chromium. Unneeded by cobalt.
#include "third_party/WebKit/Source/JavaScriptCore/runtime/JSGlobalData.h"
#include "third_party/WebKit/Source/JavaScriptCore/runtime/JSGlobalObject.h"

namespace cobalt {
namespace script {
namespace javascriptcore {

// JSCGlobalObject is JavaScriptCore's Global Object in Cobalt. It inherits from
// JSC::GlobalObject so we can use this wherever a JSC::GlobalObject would be
// used, allowing us to downcast to JSCGlobalObject.
// JSCGlobalObject is a garbage-collected object.
class JSCGlobalObject : public JSC::JSGlobalObject {
 public:
  // Create a new garbage-collected JSCGlobalObject instance.
  static JSCGlobalObject* Create(JSC::JSGlobalData* global_data);

  // Get the cached prototype for this ClassInfo, or NULL if none exist.
  JSC::JSObject* GetCachedPrototype(const JSC::ClassInfo* class_info);

  // Cache the prototype for this ClassInfo.
  void CachePrototype(const JSC::ClassInfo* class_info,
                      JSC::JSObject* prototype);

  // JavaScriptCore stuff
  static const JSC::ClassInfo s_info;
  // Classes that inherit from JSC::GlobalObject set this flag, and set a
  // finalizer method on creation.
  static const bool needsDestruction = false;
  // Statically override this to ensure that we visit objects that this
  // object it references, to ensure that they are not garbage collected.
  static void visitChildren(JSC::JSCell* cell, JSC::SlotVisitor& visitor);  // NOLINT

 private:
  JSCGlobalObject(JSC::JSGlobalData* global_data, JSC::Structure* structure);

#if defined(__LB_LINUX__)
  struct hash_function {
    std::size_t operator()(const JSC::ClassInfo* class_info) const {
      return BASE_HASH_NAMESPACE::hash<intptr_t>()(
          reinterpret_cast<intptr_t>(class_info));
    }
  };
  typedef base::hash_map<const JSC::ClassInfo*,
                         JSC::WriteBarrier<JSC::JSObject>,
                         hash_function> PrototypeMap;
#else
  typedef base::hash_map<const JSC::ClassInfo*,
                         JSC::WriteBarrier<JSC::JSObject> > PrototypeMap;
#endif

  PrototypeMap prototype_map_;
};

}  // namespace javascriptcore
}  // namespace script
}  // namespace cobalt

#endif  // SCRIPT_JAVASCRIPTCORE_JSC_GLOBAL_OBJECT_H_
