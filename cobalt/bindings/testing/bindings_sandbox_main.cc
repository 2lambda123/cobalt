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

#include <iostream>
#include <string>

#include "cobalt/base/wrap_main.h"
#include "cobalt/bindings/testing/window.h"
#include "cobalt/script/environment_settings.h"
#include "cobalt/script/global_object_proxy.h"
#include "cobalt/script/javascript_engine.h"
#include "cobalt/script/source_code.h"

using cobalt::bindings::testing::Window;
using cobalt::script::EnvironmentSettings;
using cobalt::script::GlobalObjectProxy;
using cobalt::script::JavaScriptEngine;
using cobalt::script::SourceCode;

namespace {

int SandboxMain(int argc, char** argv) {
  // Environment Settings object
  scoped_ptr<EnvironmentSettings> environment_settings =
      make_scoped_ptr(new EnvironmentSettings());

  // Initialize the JavaScript engine.
  scoped_ptr<JavaScriptEngine> engine = JavaScriptEngine::CreateEngine();

  // Create a new global object
  scoped_refptr<GlobalObjectProxy> global_object_proxy =
      engine->CreateGlobalObjectProxy();

  scoped_refptr<Window> test_window = new Window();
  global_object_proxy->CreateGlobalObject(test_window,
                                          environment_settings.get());

  while (true) {
    // Interactive prompt.
    std::cout << "> ";

    // Read user input from stdin.
    std::string line;
    std::getline(std::cin, line);
    if (line.empty())
      continue;

    // Convert the utf8 string into a form that can be consumed by the
    // JavaScript engine.
    scoped_refptr<SourceCode> source = SourceCode::CreateSourceCode(
        line, base::SourceLocation("[stdin]", 1, 1));

    // Execute the script and get the results of execution.
    std::string result;
    bool success = global_object_proxy->EvaluateScript(source, &result);

    // Echo the results to stdout.
    if (!success) {
      std::cout << "Exception: ";
    }
    std::cout << result << std::endl;
  }
}

}  // namespace

COBALT_WRAP_SIMPLE_MAIN(SandboxMain);
