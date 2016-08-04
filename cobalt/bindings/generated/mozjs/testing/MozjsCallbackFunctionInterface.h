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
// Auto-generated from template: bindings/mozjs/templates/interface.h.template

// clang-format off

#ifndef MozjsCallbackFunctionInterface_h
#define MozjsCallbackFunctionInterface_h

#include "base/hash_tables.h"
#include "base/lazy_instance.h"
#include "base/memory/ref_counted.h"
#include "base/threading/thread_checker.h"
#include "cobalt/base/polymorphic_downcast.h"
#include "cobalt/script/wrappable.h"
#include "cobalt/bindings/testing/callback_function_interface.h"

#include "third_party/mozjs/js/src/jsapi.h"

namespace cobalt {
namespace bindings {
namespace testing {

class MozjsCallbackFunctionInterface {
 public:
  static JSObject* CreateProxy(JSContext* context,
      const scoped_refptr<script::Wrappable>& wrappable);
  static const JSClass* PrototypeClass(JSContext* context);
  static JSObject* GetPrototype(JSContext* context);
  static JSObject* GetInterfaceObject(JSContext* context);
};

}  // namespace bindings
}  // namespace testing
}  // namespace cobalt

#endif  // MozjsCallbackFunctionInterface_h
