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

#if defined(ENABLE_DEBUG_CONSOLE)

#include "cobalt/debug/debugger.h"

#include "base/json/json_writer.h"
#include "base/values.h"

namespace cobalt {
namespace debug {

Debugger::Debugger(
    const CreateDebugServerCallback& create_debug_server_callback)
    : create_debug_server_callback_(create_debug_server_callback),
      on_event_(new DebuggerEventTarget()) {}

Debugger::~Debugger() {}

void Debugger::Attach(const AttachCallbackArg& callback) {
  debug_server_.reset();
  debug_server_ = create_debug_server_callback_.Run(
      base::Bind(&Debugger::OnEvent, base::Unretained(this)),
      base::Bind(&Debugger::OnDetach, base::Unretained(this)));
  AttachCallbackArg::Reference callback_reference(this, callback);
  callback_reference.value().Run();
}

void Debugger::Detach(const AttachCallbackArg& callback) {
  debug_server_.reset();
  AttachCallbackArg::Reference callback_reference(this, callback);
  callback_reference.value().Run();
}

void Debugger::SendCommand(const std::string& method,
                           const std::string& json_params,
                           const CommandCallbackArg& callback) const {
  if (!debug_server_) {
    scoped_ptr<base::DictionaryValue> response(new base::DictionaryValue);
    response->SetString("error.message",
                        "Debugger is not connected - call attach first.");
    std::string json_response;
    base::JSONWriter::Write(response.get(), &json_response);
    CommandCallbackArg::Reference callback_ref(this, callback);
    callback_ref.value().Run(json_response);
    return;
  }

  scoped_refptr<CommandCallbackInfo> callback_info(
      new CommandCallbackInfo(this, callback));
  debug_server_->SendCommand(
      method, json_params,
      base::Bind(&Debugger::OnCommandResponse, this, callback_info));
}

void Debugger::OnCommandResponse(
    const scoped_refptr<CommandCallbackInfo>& callback_info,
    const base::optional<std::string>& response) const {
  // Run the script callback on the message loop the command was sent from.
  callback_info->message_loop_proxy->PostTask(
      FROM_HERE,
      base::Bind(&Debugger::RunCommandCallback, this, callback_info, response));
}

void Debugger::OnEvent(const std::string& method,
                       const base::optional<std::string>& response) const {
  // Pass to the onEvent handler. The handler will notify the JavaScript
  // listener on the message loop the listener was registered on.
  on_event_->DispatchEvent(method, response);
}

void Debugger::OnDetach(const std::string& reason) const {
  NOTIMPLEMENTED() << "Debugger detached: " + reason;
}

void Debugger::RunCommandCallback(
    const scoped_refptr<CommandCallbackInfo>& callback_info,
    base::optional<std::string> response) const {
  DCHECK_EQ(base::MessageLoopProxy::current(),
            callback_info->message_loop_proxy);
  callback_info->callback.value().Run(response);
}
}  // namespace debug
}  // namespace cobalt

#endif  // ENABLE_DEBUG_CONSOLE
