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

#include "cobalt/script/script_runner.h"

#include "base/logging.h"
#include "cobalt/script/source_code.h"

namespace cobalt {
namespace script {

namespace {

class ScriptRunnerImpl : public ScriptRunner {
 public:
  explicit ScriptRunnerImpl(
      const scoped_refptr<GlobalObjectProxy> global_object_proxy)
      : global_object_proxy_(global_object_proxy) {}

  void Execute(const std::string& script_utf8) OVERRIDE;

 private:
  scoped_refptr<GlobalObjectProxy> global_object_proxy_;
};

void ScriptRunnerImpl::Execute(const std::string& script_utf8) {
  scoped_refptr<SourceCode> source_code =
      SourceCode::CreateSourceCode(script_utf8);
  if (source_code == NULL) {
    NOTREACHED() << "Failed to pre-process javascript source.";
    return;
  }
  std::string result;
  if (!global_object_proxy_->EvaluateScript(source_code, &result)) {
    DLOG(WARNING) << "Failed to execute javascript: " << result;
  }
}

}  // namespace

scoped_ptr<ScriptRunner> ScriptRunner::CreateScriptRunner(
    const scoped_refptr<GlobalObjectProxy> global_object_proxy) {
  return make_scoped_ptr<ScriptRunner>(
             new ScriptRunnerImpl(global_object_proxy)).Pass();
}

}  // namespace script
}  // namespace cobalt
