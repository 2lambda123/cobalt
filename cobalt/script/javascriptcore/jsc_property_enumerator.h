/*
 * Copyright 2015 Google Inc. All Rights Reserved.
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
#ifndef SCRIPT_JAVASCRIPTCORE_JSC_PROPERTY_ENUMERATOR_H_
#define SCRIPT_JAVASCRIPTCORE_JSC_PROPERTY_ENUMERATOR_H_

#include "cobalt/script/javascriptcore/conversion_helpers.h"
#include "cobalt/script/property_enumerator.h"

namespace cobalt {
namespace script {
namespace javascriptcore {

// Implements the PropertyEnumerator interface. Enumerated properties will
// be added to the JSC::PropertyNameArray that is passed in at construction.
class JSCPropertyEnumerator : public PropertyEnumerator {
 public:
  JSCPropertyEnumerator(JSC::ExecState* exec_state,
                        JSC::PropertyNameArray* property_names)
      : exec_state_(exec_state), property_names_(property_names) {}
  void AddProperty(const std::string& property_name) OVERRIDE {
    WTF::String wtf_string = ToWTFString(property_name);
    JSC::Identifier identifier(exec_state_, wtf_string.impl());
    property_names_->add(identifier);
  }

 private:
  JSC::ExecState* exec_state_;
  JSC::PropertyNameArray* property_names_;
};

}  // namespace javascriptcore
}  // namespace script
}  // namespace cobalt

#endif  // SCRIPT_JAVASCRIPTCORE_JSC_PROPERTY_ENUMERATOR_H_
