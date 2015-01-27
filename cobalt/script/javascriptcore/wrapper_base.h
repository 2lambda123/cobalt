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
#ifndef SCRIPT_JAVASCRIPTCORE_WRAPPER_BASE_H_
#define SCRIPT_JAVASCRIPTCORE_WRAPPER_BASE_H_

#include "base/memory/ref_counted.h"

#include "cobalt/script/wrappable.h"
#include "third_party/WebKit/Source/JavaScriptCore/runtime/JSDestructibleObject.h"

namespace cobalt {
namespace script {
namespace javascriptcore {

// All JavaScriptCore wrapper classes will inherit from this. This provides a
// common base class for all wrapper objects. It holds a reference-counted
// handle to a Wrappable object, which is the base class of all objects that
// can be referenced from JavaScript.
class WrapperBase : public JSC::JSDestructibleObject {
 protected:
  WrapperBase(JSC::JSGlobalObject* global_object, JSC::Structure* structure,
              const scoped_refptr<Wrappable>& impl)
      : JSC::JSDestructibleObject(global_object->globalData(), structure),
        wrappable_(impl) {}

  // static override. This will be called when this object is garbage collected.
  static void destroy(JSC::JSCell* cell) {
    // Note that the WrapperBase destructor is non-virtual.
    // JSC's garbage-collected heap seems to not support classes with virtual
    // functions, as suggested by the lack of a virtual destructor in JSCell,
    // the base class for all GC objects, and the lack of any virtual functions
    // in WebKit's bindings implementation.
    // Garbage-collected classes with a non-trivial destructor and/or
    // member variables that need to be destructed must override destroy() to
    // ensure that the class is destructed properly.
    static_cast<WrapperBase*>(cell)->~WrapperBase();
  }

  scoped_refptr<Wrappable>& wrappable() { return wrappable_; }

 private:
  scoped_refptr<Wrappable> wrappable_;
};

}  // namespace javascriptcore
}  // namespace script
}  // namespace cobalt
#endif  // SCRIPT_JAVASCRIPTCORE_WRAPPER_BASE_H_
