// Copyright 2015 The Cobalt Authors. All Rights Reserved.
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

#ifndef COBALT_DEBUG_DEBUG_HUB_H_
#define COBALT_DEBUG_DEBUG_HUB_H_

#if defined(ENABLE_DEBUG_CONSOLE)

#include <string>

#include "base/callback.h"
#include "base/memory/weak_ptr.h"
#include "base/message_loop.h"
#include "cobalt/base/log_message_handler.h"
#include "cobalt/debug/debugger.h"
#include "cobalt/script/callback_function.h"
#include "cobalt/script/script_value.h"
#include "cobalt/script/wrappable.h"

namespace cobalt {
namespace debug {

// This class implements an interface to JavaScript for debugging.
// The public methods of this class are expected to be exposed in DebugHub.idl.
// The main (and typically only) JavaScript client is the debug console.
// A stub implementation is used if ENABLE_DEBUG_CONSOLE is not defined.
//
// Log messages generated by the app via LOG are sent to the client
// via a JavaScript callback registered with SetLogMessageCallback.
// This typically includes console output from the main web module.
//
// The JavaScript client can examine the available list of registered console
// values.
//
// There is a debugger client available via the |debugger_| member that can
// connect to the debug server attached to the main web module. The debug
// console uses this to send debugging commands and receive debugging events.
// The debug server is accessed using the |get_debug_server_callback|
// parameter specified in the constructor.
//
// The JavaScript client can send arbitrary messages to an object of this class
// on a specific named channel using the SendCommand method. These messages are
// forwarded to any component of the app that registers a message handler for
// that channel by creating an instance of a class derived from CommandHandler.

class DebugHub : public script::Wrappable {
 public:
  // Type for log message callback on JS side.
  typedef script::CallbackFunction<
      void(int severity, const std::string& file, int line,
           size_t message_start, const std::string& msg)> LogMessageCallback;
  typedef script::ScriptValue<LogMessageCallback> LogMessageCallbackArg;

  // Function signature to call when we need to query for the Hud visibility
  // mode.
  typedef base::Callback<int()> GetHudModeCallback;

  // Debug console visibility modes.
  static const int kDebugConsoleOff = 0;
  static const int kDebugConsoleHud = 1;
  static const int kDebugConsoleOn = 2;
  static const int kDebugConsoleNumModes = kDebugConsoleOn + 1;

  // Declare logging levels here for export to JS
  static const int kLogInfo = logging::LOG_INFO;
  static const int kLogWarning = logging::LOG_WARNING;
  static const int kLogError = logging::LOG_ERROR;
  static const int kLogErrorReport = logging::LOG_ERROR_REPORT;
  static const int kLogFatal = logging::LOG_FATAL;

  DebugHub(const GetHudModeCallback& get_hud_mode_callback,
           const Debugger::GetDebugServerCallback& get_debug_server_callback);
  ~DebugHub();

  // Gets the JavaScript debugger client interface.
  const scoped_refptr<Debugger>& debugger() { return debugger_; }

  // Called from JS to set the log message callback.
  // Must be called from |message_loop_|.
  void SetLogMessageCallback(const LogMessageCallbackArg& callback);

  // Gets the collection of available CVal names as an alphabetically ordered,
  // space-separated list.
  std::string GetConsoleValueNames() const;

  // Gets the value of a named CVal as a pretty string.
  std::string GetConsoleValue(const std::string& name) const;

  int GetDebugConsoleMode() const;

  DEFINE_WRAPPABLE_TYPE(DebugHub);
  void TraceMembers(script::Tracer* tracer) override;

 private:
  // Called by LogMessageHandler for each log message.
  // May be called from any thread.
  bool OnLogMessage(int severity, const char* file, int line,
                    size_t message_start, const std::string& str);

  // Forwards a log message to |log_message_callback|.
  // Must be called on |message_loop_|.
  void OnLogMessageInternal(int severity, const char* file, int line,
                            size_t message_start, const std::string& str);

  // The message loop this object was constructed on, which should be the
  // same as the WebModule this object is attached to.
  MessageLoop* message_loop_;

  // A function that we can call to query the Hud visibility mode.
  const GetHudModeCallback get_hud_mode_callback_;

  // The JavaScript callback to forward log messages to.
  base::LogMessageHandler::CallbackId log_message_handler_callback_id_;

  // Interface to the JavaScript debugger client.
  scoped_refptr<Debugger> debugger_;

  // Use a weak pointer when posting |OnLogMessageInternal| tasks.
  base::WeakPtrFactory<DebugHub> weak_ptr_factory_;
  base::WeakPtr<DebugHub> weak_ptr_;

  // Used to ensure we don't run the log message callback recursively.
  bool is_logging_;

  scoped_ptr<LogMessageCallbackArg::Reference> log_message_callback_;
};

}  // namespace debug
}  // namespace cobalt

#endif  // ENABLE_DEBUG_CONSOLE
#endif  // COBALT_DEBUG_DEBUG_HUB_H_
