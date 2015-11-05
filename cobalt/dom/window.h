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

#ifndef DOM_WINDOW_H_
#define DOM_WINDOW_H_

#include <string>

#include "base/callback.h"
#include "base/hash_tables.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/timer.h"
#include "cobalt/cssom/css_parser.h"
#include "cobalt/debug/debug_hub.h"
#include "cobalt/dom/animation_frame_request_callback_list.h"
#include "cobalt/dom/crypto.h"
#include "cobalt/dom/event_target.h"
#include "cobalt/dom/h5vcc_stub.h"
#include "cobalt/dom/media_source.h"
#include "cobalt/dom/parser.h"
#if defined(ENABLE_TEST_RUNNER)
#include "cobalt/dom/test_runner.h"
#endif  // ENABLE_TEST_RUNNER
#include "cobalt/dom/window_timers.h"
#include "cobalt/loader/decoder.h"
#include "cobalt/loader/fetcher_factory.h"
#include "cobalt/loader/font/remote_font_cache.h"
#include "cobalt/loader/image/image_cache.h"
#include "cobalt/loader/loader.h"
#include "cobalt/media/web_media_player_factory.h"
#include "cobalt/script/callback_function.h"
#include "cobalt/script/execution_state.h"
#include "cobalt/script/script_runner.h"
#include "googleurl/src/gurl.h"

namespace cobalt {
namespace dom {

class Console;
class Document;
class Event;
class History;
class HTMLElementContext;
class LocalStorageDatabase;
class Location;
class Navigator;
class Performance;
class Screen;
class Storage;
class WindowTimers;

// The window object represents a window containing a DOM document.
//   http://www.w3.org/TR/html5/browsers.html#the-window-object
//
// TODO(***REMOVED***): Properly handle viewport resolution change event.
class Window : public EventTarget {
 public:
  typedef AnimationFrameRequestCallbackList::FrameRequestCallback
      FrameRequestCallback;
  typedef WindowTimers::TimerCallback TimerCallback;
  typedef base::Callback<scoped_ptr<loader::Decoder>(
      HTMLElementContext*, const scoped_refptr<Document>&,
      const base::SourceLocation&, const base::Closure&,
      const base::Callback<void(const std::string&)>&)>
      HTMLDecoderCreatorCallback;

  Window(int width, int height, cssom::CSSParser* css_parser,
         Parser* dom_parser, loader::FetcherFactory* fetcher_factory,
         loader::image::ImageCache* image_cache,
         loader::font::RemoteFontCache* remote_font_cache,
         LocalStorageDatabase* local_storage_database,
         media::WebMediaPlayerFactory* web_media_player_factory,
         script::ExecutionState* execution_state,
         script::ScriptRunner* script_runner,
         MediaSource::Registry* media_source_registry, const GURL& url,
         const std::string& user_agent, const std::string& language,
         const base::Callback<void(const std::string&)>& error_callback);

  // Web API: Window
  //
  scoped_refptr<Window> window() { return this; }
  const scoped_refptr<Document>& document() const;
  const scoped_refptr<History>& history() const;
  scoped_refptr<Location> location() const;
  scoped_refptr<Window> top() { return this; }
  const scoped_refptr<Navigator>& navigator() const;

  // Web API: Timing control for script-based animations (partial interface)
  //   http://www.w3.org/TR/animation-timing/#Window-interface-extensions
  int32 RequestAnimationFrame(
      const AnimationFrameRequestCallbackList::FrameRequestCallbackArg&);
  void CancelAnimationFrame(int32 handle);

  // Web API: CSSOM View Module: Extensions to the window interface
  //

  // As its name suggests, the Screen interface represents information about the
  // screen of the output device.
  //   http://www.w3.org/TR/cssom-view/#the-screen-interface
  const scoped_refptr<Screen>& screen();

  // The innerWidth attribute must return the viewport width including the size
  // of a rendered scroll bar (if any), or zero if there is no viewport.
  //   http://www.w3.org/TR/cssom-view/#dom-window-innerwidth
  float inner_width() const { return static_cast<float>(width_); }
  // The innerHeight attribute must return the viewport height including the
  // size of a rendered scroll bar (if any), or zero if there is no viewport.
  //   http://www.w3.org/TR/cssom-view/#dom-window-innerheight
  float inner_height() const { return static_cast<float>(height_); }

  // The screenX attribute must return the x-coordinate, relative to the origin
  // of the screen of the output device, of the left of the client window as
  // number of pixels, or zero if there is no such thing.
  //   http://www.w3.org/TR/cssom-view/#dom-window-screenx
  float screen_x() const { return 0.0f; }
  // The screenY attribute must return the y-coordinate, relative to the origin
  // of the screen of the output device, of the top of the client window as
  // number of pixels, or zero if there is no such thing.
  //   http://www.w3.org/TR/cssom-view/#dom-window-screeny
  float screen_y() const { return 0.0f; }
  // The outerWidth attribute must return the width of the client window.
  //   http://www.w3.org/TR/cssom-view/#dom-window-outerwidth
  float outer_width() const { return static_cast<float>(width_); }
  // The outerHeight attribute must return the height of the client window.
  //   http://www.w3.org/TR/cssom-view/#dom-window-outerheight
  float outer_height() const { return static_cast<float>(height_); }
  // The devicePixelRatio attribute returns the ratio of CSS pixels per device
  // pixel.
  //   http://www.w3.org/TR/cssom-view/#dom-window-devicepixelratio
  float device_pixel_ratio() const { return 1.0f; }

  // Web API: GlobalCrypto (implements)
  //   http://www.w3.org/TR/WebCryptoAPI/#crypto-interface
  scoped_refptr<Crypto> crypto() const;

  // Web API: WindowTimers (implements)
  //   http://www.w3.org/TR/html5/webappapis.html#timers
  //
  int SetTimeout(const WindowTimers::TimerCallbackArg& handler) {
    return SetTimeout(handler, 0);
  }

  int SetTimeout(const WindowTimers::TimerCallbackArg& handler, int timeout);

  void ClearTimeout(int handle);

  int SetInterval(const WindowTimers::TimerCallbackArg& handler) {
    return SetInterval(handler, 0);
  }

  int SetInterval(const WindowTimers::TimerCallbackArg& handler, int timeout);

  void ClearInterval(int handle);

  // Web API: Storage (implements)
  scoped_refptr<Storage> local_storage() const;
  scoped_refptr<Storage> session_storage() const;

  // Access to the Performance API (partial interface)
  //   https://dvcs.w3.org/hg/webperf/raw-file/tip/specs/NavigationTiming/Overview.html#sec-window.performance-attribute
  const scoped_refptr<Performance>& performance() const;

  // Custom, not in any spec.
  //
  const scoped_refptr<Console>& console() const;

  // Handles debug communication with the main and debug console windows.
  const scoped_refptr<debug::DebugHub>& debug_hub() const;
  void set_debug_hub(const scoped_refptr<debug::DebugHub>& debug_hub);

#if defined(ENABLE_TEST_RUNNER)
  const scoped_refptr<TestRunner>& test_runner() const;
#endif  // ENABLE_TEST_RUNNER

  const scoped_refptr<H5vccStub>& h5vcc() const { return h5vcc_; }
  void set_h5vcc(const scoped_refptr<H5vccStub>& h5vcc) { h5vcc_ = h5vcc; }

  HTMLElementContext* html_element_context() const;

  // Will fire the animation frame callbacks and reset the animation frame
  // request callback list.
  void RunAnimationFrameCallbacks();

  // Call this to inject an event into the window which will ultimately make
  // its way to the appropriate object in DOM.
  void InjectEvent(const scoped_refptr<Event>& event);

  scoped_refptr<Window> opener() const { return NULL; }
  DEFINE_WRAPPABLE_TYPE(Window);

 private:
  class RelayLoadEvent;

  ~Window() OVERRIDE;

  int width_;
  int height_;

  const scoped_ptr<HTMLElementContext> html_element_context_;
  scoped_refptr<Performance> performance_;
  scoped_refptr<Document> document_;
  scoped_ptr<loader::Loader> document_loader_;
  scoped_refptr<History> history_;
  scoped_refptr<Navigator> navigator_;
  scoped_ptr<RelayLoadEvent> relay_on_load_event_;
  scoped_refptr<Console> console_;
  scoped_ptr<WindowTimers> window_timers_;
  scoped_ptr<AnimationFrameRequestCallbackList>
      animation_frame_request_callback_list_;

  scoped_refptr<Crypto> crypto_;

  scoped_refptr<Storage> local_storage_;
  scoped_refptr<Storage> session_storage_;

  scoped_refptr<Screen> screen_;

  scoped_refptr<debug::DebugHub> debug_hub_;

#if defined(ENABLE_TEST_RUNNER)
  scoped_refptr<TestRunner> test_runner_;
#endif  // ENABLE_TEST_RUNNER

  scoped_refptr<H5vccStub> h5vcc_;

  DISALLOW_COPY_AND_ASSIGN(Window);
};

}  // namespace dom
}  // namespace cobalt

#endif  // DOM_WINDOW_H_
