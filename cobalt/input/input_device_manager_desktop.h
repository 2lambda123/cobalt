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

#ifndef COBALT_INPUT_INPUT_DEVICE_MANAGER_DESKTOP_H_
#define COBALT_INPUT_INPUT_DEVICE_MANAGER_DESKTOP_H_

#include "cobalt/input/input_device_manager.h"
#include "cobalt/input/keypress_generator_filter.h"
#include "cobalt/system_window/system_window.h"

namespace cobalt {
namespace input {

class InputDeviceManagerDesktop : public InputDeviceManager {
 public:
  InputDeviceManagerDesktop(const KeyboardEventCallback& callback,
                            system_window::SystemWindow* system_window);

  ~InputDeviceManagerDesktop() OVERRIDE;

 protected:
  // Called to handle a keyboard event generated by the referenced system
  // window.
  void HandleKeyboardEvent(const base::Event* event);

 private:
  // Reference to the system window that will provide keyboard events.
  system_window::SystemWindow* system_window_;

  // Store a callback wrapping the object event handler, HandleKeyboardEvent.
  // This is so we can remove it again when this object is destroyed.
  base::EventCallback keyboard_event_callback_;

  // Keyboard event filters to process the events generated.
  KeypressGeneratorFilter keypress_generator_filter_;
};

}  // namespace input
}  // namespace cobalt

#endif  // COBALT_INPUT_INPUT_DEVICE_MANAGER_DESKTOP_H_
