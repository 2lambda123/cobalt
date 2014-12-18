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

#include "cobalt/browser/application.h"

#include "base/command_line.h"
#include "base/logging.h"
#include "base/run_loop.h"
#include "cobalt/browser/switches.h"

namespace cobalt {
namespace browser {

namespace {

std::string GetInitialURL() {
  // Allow the user to override the default initial URL via a command line
  // parameter.
  CommandLine *command_line = CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(switches::kInitialURL)) {
    return command_line->GetSwitchValueASCII(switches::kInitialURL);
  }

  static const char* kDefaultInitialURL = "https://www.youtube.com/tv";
  return std::string(kDefaultInitialURL);
}

}  // namespace

Application::Application()
    : ui_message_loop_(MessageLoop::TYPE_UI), initial_url_(GetInitialURL()) {}

Application::~Application() { DCHECK(!ui_message_loop_.is_running()); }

void Application::Quit() {
  if (!quit_closure_.is_null()) {
    quit_closure_.Run();
  }
}

void Application::Run() {
  base::RunLoop run_loop;
  quit_closure_ = run_loop.QuitClosure();
  run_loop.Run();
}

}  // namespace browser
}  // namespace cobalt
