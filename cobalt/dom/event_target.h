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

#ifndef DOM_EVENT_TARGET_H_
#define DOM_EVENT_TARGET_H_

#include <vector>

#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/string_piece.h"
#include "cobalt/dom/event.h"
#include "cobalt/dom/event_listener.h"
#include "cobalt/script/script_object_handle_visitor.h"
#include "cobalt/script/wrappable.h"

namespace cobalt {
namespace dom {

// The EventTarget interface represents an object that holds event listeners
// and possibly generates events.
// This interface describes methods and properties common to all kinds of
// EventTarget.
//   http://www.w3.org/TR/2014/WD-dom-20140710/#eventtarget
class EventTarget : public script::Wrappable,
                    public base::SupportsWeakPtr<EventTarget> {
 public:
  // Web API: EventTarget
  //
  void AddEventListener(const std::string& type,
                        const scoped_refptr<EventListener>& listener,
                        bool use_capture);
  void RemoveEventListener(const std::string& type,
                           const scoped_refptr<EventListener>& listener,
                           bool use_capture);
  bool DispatchEvent(const scoped_refptr<Event>& event);

  // Custom, not in any spec.
  //
  // MarkJSObjectAsNotCollectable gives contained event listeners a chance to
  // mark contained JS objects as in use so they won't be collected by GC.
  void MarkJSObjectAsNotCollectable(script::ScriptObjectHandleVisitor* visitor);

 private:
  struct EventListenerInfo {
    std::string type;
    scoped_refptr<EventListener> listener;
    bool use_capture;
  };
  typedef std::vector<EventListenerInfo> EventListenerInfos;

  EventListenerInfos event_listener_infos_;
};

}  // namespace dom
}  // namespace cobalt

#endif  // DOM_EVENT_TARGET_H_
