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

#ifndef DOM_UI_EVENT_H_
#define DOM_UI_EVENT_H_

#include "base/string_piece.h"
#include "cobalt/dom/document.h"
#include "cobalt/dom/document_builder.h"
#include "cobalt/dom/event.h"
#include "cobalt/dom/window.h"
#include "cobalt/script/wrappable.h"

namespace cobalt {
namespace dom {

// The UIEvent provides specific contextual information associated with User
// Interface events.
//   http://www.w3.org/TR/DOM-Level-3-Events/#events-uievents
class UIEvent : public Event {
 public:
  Window* view() const { return view_.get(); }

  // Custom, not in any spec.
  enum Type {
    kNoType = 0,
    kKeyDown,
    kKeyPress,
    kKeyUp,
  };

 protected:
  UIEvent(Type type, const scoped_refptr<Window>& view);
  ~UIEvent() OVERRIDE {}

  const scoped_refptr<Window> view_;

  // TODO(***REMOVED***): remove type_enum_ when we have something to deal with string
  // comparison.
  Type type_enum_;
};

}  // namespace dom
}  // namespace cobalt
#endif  // DOM_UI_EVENT_H_
