// Copyright 2015 Google Inc. All Rights Reserved.
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

#include "starboard/shared/starboard/queue_application.h"

#include "starboard/atomic.h"
#include "starboard/condition_variable.h"
#include "starboard/event.h"
#include "starboard/log.h"
#include "starboard/time.h"

namespace starboard {
namespace shared {
namespace starboard {

Application::Event* QueueApplication::PollNextEvent() {
  Event* event = event_queue_.Poll();
  if (event != NULL) {
    return event;
  }

  TimedEvent* timed_event = GetNextDueTimedEvent();
  if (timed_event != NULL) {
    return new Event(timed_event);
  }

  return NULL;
}

void QueueApplication::Wake() {
  if (IsCurrentThread()) {
    return;
  }

  event_queue_.Wake();
}

Application::Event* QueueApplication::GetNextEvent() {
  for (;;) {
    Event* event = event_queue_.GetTimed(GetNextTimedEventTargetTime());
    if (event != NULL) {
      return event;
    }

    TimedEvent* timed_event = GetNextDueTimedEvent();
    if (timed_event != NULL) {
      return new Event(timed_event);
    }
  }
}

void QueueApplication::Inject(Event* event) {
  event_queue_.Put(event);
}

void QueueApplication::InjectTimedEvent(TimedEvent* timed_event) {
  if (timed_event_queue_.Inject(timed_event)) {
    // The time to wake up has moved earlier, so wake up the event queue to
    // recalculate the wait.
    Wake();
  }
}

void QueueApplication::CancelTimedEvent(SbEventId event_id) {
  timed_event_queue_.Cancel(event_id);

  // The wait duration will only get longer after canceling an event, so the
  // waiter will wake up as previously scheduled, see there is nothing to do,
  // and go back to sleep.
}

Application::TimedEvent* QueueApplication::GetNextDueTimedEvent() {
  return timed_event_queue_.Get();
}

SbTimeMonotonic QueueApplication::GetNextTimedEventTargetTime() {
  return timed_event_queue_.GetTime();
}

QueueApplication::TimedEventQueue::TimedEventQueue() : set_(&IsLess) {}

bool QueueApplication::TimedEventQueue::Inject(TimedEvent* timed_event) {
  ScopedLock lock(mutex_);
  SbTimeMonotonic oldTime = GetTimeLocked();
  map_[timed_event->id] = timed_event;
  set_.insert(timed_event);
  return (timed_event->target_time < oldTime);
}

void QueueApplication::TimedEventQueue::Cancel(SbEventId event_id) {
  ScopedLock lock(mutex_);
  TimedEventMap::iterator i = map_.find(event_id);
  if (i == map_.end()) {
    return;
  }

  TimedEvent* timed_event = i->second;
  map_.erase(i);
  set_.erase(timed_event);
  delete timed_event;
}

Application::TimedEvent* QueueApplication::TimedEventQueue::Get() {
  ScopedLock lock(mutex_);
  if (set_.empty()) {
    return NULL;
  }

  TimedEvent* timed_event = *(set_.begin());
  if (timed_event->target_time > SbTimeGetMonotonicNow()) {
    return NULL;
  }

  map_.erase(timed_event->id);
  set_.erase(timed_event);
  return timed_event;
}

SbTimeMonotonic QueueApplication::TimedEventQueue::GetTime() {
  ScopedLock lock(mutex_);
  return GetTimeLocked();
}

SbTimeMonotonic QueueApplication::TimedEventQueue::GetTimeLocked() {
  if (set_.empty()) {
    return kSbTimeMax;
  }

  TimedEvent* timed_event = *(set_.begin());
  SbTimeMonotonic time = timed_event->target_time;
  SbTimeMonotonic now = SbTimeGetMonotonicNow();
  if (time < now) {
    return now;
  }

  return time;
}

// static
bool QueueApplication::TimedEventQueue::IsLess(const TimedEvent* lhs,
                                               const TimedEvent* rhs) {
  SbTimeMonotonic time_difference = lhs->target_time - rhs->target_time;
  if (time_difference != 0) {
    return time_difference < 0;
  }

  // If the time differences are the same, ensure there is a strict and stable
  // ordering.
  return reinterpret_cast<uintptr_t>(lhs) < reinterpret_cast<uintptr_t>(rhs);
}

}  // namespace starboard
}  // namespace shared
}  // namespace starboard
