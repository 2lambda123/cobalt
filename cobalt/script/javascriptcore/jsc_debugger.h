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
#ifndef COBALT_SCRIPT_JAVASCRIPTCORE_JSC_DEBUGGER_H_
#define COBALT_SCRIPT_JAVASCRIPTCORE_JSC_DEBUGGER_H_

#include <string>

#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/threading/thread_checker.h"
#include "cobalt/script/javascript_debugger_interface.h"
#include "cobalt/script/javascriptcore/jsc_global_object_proxy.h"

#include "third_party/WebKit/Source/JavaScriptCore/config.h"
#include "third_party/WebKit/Source/JavaScriptCore/debugger/Debugger.h"
#include "third_party/WebKit/Source/JavaScriptCore/debugger/DebuggerCallFrame.h"
#include "third_party/WebKit/Source/WTF/wtf/HashSet.h"
#include "third_party/WebKit/Source/WTF/wtf/text/WTFString.h"

namespace JSC {
class ExecState;
class JSGlobalData;
class JSGlobalObject;
class JSScope;
class JSValue;
class SourceProvider;
}

namespace cobalt {
namespace script {
namespace javascriptcore {

// JavaScriptCore-specific implementation of a JavaScript debugger.
// Uses multiple inheritance in accordance with the C++ style guide to extend
// JSC::Debugger and implement JavaScriptDebuggerInterface.
// https://engdoc.***REMOVED***/eng/doc/devguide/cpp/styleguide.shtml?cl=head#Multiple_Inheritance
// Only the JavaScriptDebuggerInterface is publicly exposed.
// This class is not designed to be thread-safe - it is assumed that all
// public methods will be run on the same message loop as the JavaScript
// global object to which this debugger connects.
class JSCDebugger : protected JSC::Debugger,
                    public JavaScriptDebuggerInterface {
 public:
  JSCDebugger(GlobalObjectProxy* global_object_proxy, Delegate* delegate);

  ~JSCDebugger() OVERRIDE;

  // Implementation of JavaScriptDebuggerInterface.
  scoped_ptr<base::DictionaryValue> GetScriptSource(
      const scoped_ptr<base::DictionaryValue>& params) OVERRIDE;

  void Pause() OVERRIDE;
  void Resume() OVERRIDE;
  void StepInto() OVERRIDE;
  void StepOut() OVERRIDE;
  void StepOver() OVERRIDE;

 protected:
  // Hides a non-virtual JSC::Debugger method with the same name.
  void attach(JSC::JSGlobalObject* global_object);

  // The following methods are overrides of pure virtual methods in
  // JSC::Debugger, hence the non-standard names.
  void detach(JSC::JSGlobalObject* global_object) OVERRIDE;

  void sourceParsed(JSC::ExecState* exec_state,
                    JSC::SourceProvider* source_provider, int error_line,
                    const WTF::String& error_message) OVERRIDE;

  void exception(const JSC::DebuggerCallFrame& call_frame, intptr_t source_id,
                 int line_number, int column_number, bool has_handler) OVERRIDE;

  void atStatement(const JSC::DebuggerCallFrame& call_frame, intptr_t source_id,
                   int line_number, int column_number) OVERRIDE;

  void callEvent(const JSC::DebuggerCallFrame& call_frame, intptr_t source_id,
                 int line_number, int column_number) OVERRIDE;

  void returnEvent(const JSC::DebuggerCallFrame& call_frame, intptr_t source_id,
                   int line_number, int column_number) OVERRIDE;

  void willExecuteProgram(const JSC::DebuggerCallFrame& call_frame,
                          intptr_t source_id, int line_number,
                          int column_number) OVERRIDE;

  void didExecuteProgram(const JSC::DebuggerCallFrame& call_frame,
                         intptr_t source_id, int line_number,
                         int column_number) OVERRIDE;

  void didReachBreakpoint(const JSC::DebuggerCallFrame& call_frame,
                          intptr_t source_id, int line_number,
                          int column_number) OVERRIDE;

 private:
  // Type used to store a set of source providers.
  typedef WTF::HashSet<JSC::SourceProvider*> SourceProviderSet;

  // Functor to iterate over the JS cells and gather source providers.
  class GathererFunctor : public JSC::MarkedBlock::VoidFunctor {
   public:
    GathererFunctor(JSC::JSGlobalObject* global_object,
                    SourceProviderSet* source_providers)
        : global_object_(global_object), source_providers_(source_providers) {}

    void operator()(JSC::JSCell* cell);

   private:
    SourceProviderSet* source_providers_;
    JSC::JSGlobalObject* global_object_;
  };

  // Sets the |is_paused_| member of the debugger while in scope, unsets it
  // on destruction.
  class ScopedPausedState {
   public:
    explicit ScopedPausedState(JSCDebugger* debugger) : debugger(debugger) {
      debugger->is_paused_ = true;
    }
    ~ScopedPausedState() { debugger->is_paused_ = false; }

   private:
    JSCDebugger* debugger;
  };

  // Functions to create data objects for devtools.
  scoped_ptr<base::DictionaryValue> CreateCallFrameData(
      const JSC::DebuggerCallFrame& call_frame,
      scoped_ptr<base::DictionaryValue> location,
      scoped_ptr<base::ListValue> scope_chain);
  scoped_ptr<base::ListValue> CreateCallStackData(JSC::CallFrame* call_frame);
  scoped_ptr<base::DictionaryValue> CreateLocationData(intptr_t source_id,
                                                       int line_number,
                                                       int column_number);
  scoped_ptr<base::DictionaryValue> CreateRemoteObjectData(
      JSC::JSObject* object);
  scoped_ptr<base::ListValue> CreateScopeChainData(
      const JSC::CallFrame* call_frame);
  scoped_ptr<base::DictionaryValue> CreateScopeData(JSC::JSScope* scope);

  // Convenience function to get the global object pointer from the proxy.
  JSCGlobalObject* GetGlobalObject() const;

  // Populates the set of source providers.
  void GatherSourceProviders(JSC::JSGlobalObject* global_object);

  // Update functions called by the overridden methods from JSC:Debugger above
  // (e.g. |atStatement|) as script is executed.
  void UpdateAndPauseIfNeeded(const JSC::DebuggerCallFrame& call_frame,
                              intptr_t source_id, int line_number,
                              int column_number);
  void UpdateSourceLocation(intptr_t source_id, int line_number,
                            int column_number);
  void UpdateCallFrame(const JSC::DebuggerCallFrame& call_frame);
  void PauseIfNeeded(const JSC::DebuggerCallFrame& call_frame);

  // Sends event notifications via |delegate_|.
  void SendPausedEvent(const JSC::DebuggerCallFrame& call_frame);
  void SendResumedEvent();

  base::ThreadChecker thread_checker_;
  GlobalObjectProxy* global_object_proxy_;

  // Lifetime managed by the caller of this object's constructor.
  Delegate* delegate_;

  // Code execution control flags. Script execution can pause on the next
  // statement, or on a specific call frame.
  bool pause_on_next_statement_;
  JSC::CallFrame* pause_on_call_frame_;

  // Current call frame.
  JSC::DebuggerCallFrame current_call_frame_;

  // Current source location.
  intptr_t current_source_id_;
  int current_line_number_;
  int current_column_number_;

  // Whether script execution is currently paused.
  bool is_paused_;

  // Set of source providers (scripts).
  SourceProviderSet source_providers_;
};

}  // namespace javascriptcore
}  // namespace script
}  // namespace cobalt

#endif  // COBALT_SCRIPT_JAVASCRIPTCORE_JSC_DEBUGGER_H_
