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

#include "cobalt/browser/browser_module.h"

#include "base/bind.h"
#include "base/command_line.h"
#include "base/debug/trace_event.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/stl_util.h"
#include "cobalt/base/cobalt_paths.h"
#include "cobalt/base/tokens.h"
#include "cobalt/browser/resource_provider_array_buffer_allocator.h"
#include "cobalt/browser/screen_shot_writer.h"
#include "cobalt/browser/switches.h"
#include "cobalt/dom/keycode.h"
#include "cobalt/h5vcc/h5vcc.h"
#include "cobalt/input/input_device_manager_fuzzer.h"

namespace cobalt {
namespace browser {
namespace {

// TODO(***REMOVED***): Subscribe to viewport size changes.

#if defined(ENABLE_DEBUG_CONSOLE)

// Command to navigate to a URL.
const char kNavigateCommand[] = "navigate";

// Help strings for the navigate command.
const char kNavigateCommandShortHelp[] = "Navigates to URL.";
const char kNavigateCommandLongHelp[] =
    "Destroys the current document and window and creates a new one in its "
    "place that displays the specified URL.";

// Command to reload the current URL.
const char kReloadCommand[] = "reload";

// Help strings for the navigate command.
const char kReloadCommandShortHelp[] = "Reloads the current URL.";
const char kReloadCommandLongHelp[] =
    "Destroys the current document and window and creates a new one in its "
    "place that displays the current URL.";

const char kFuzzerToggleCommand[] = "fuzzer_toggle";
const char kFuzzerToggleCommandShortHelp[] = "Toggles the input fuzzer on/off.";
const char kFuzzerToggleCommandLongHelp[] =
    "Each time this is called, it will toggle whether the input fuzzer is "
    "activated or not.  While activated, input will constantly and randomly be "
    "generated and passed directly into the main web module.";

#if defined(ENABLE_SCREENSHOT)
// Command to reload the current URL.
const char kScreenshotCommand[] = "screenshot";

// Help strings for the navigate command.
const char kScreenshotCommandShortHelp[] = "Takes a screenshot.";
const char kScreenshotCommandLongHelp[] =
    "Creates a screenshot of the most recent layout tree and writes it "
    "to disk. Logs the filename of the screenshot to the console when done.";
#endif  // defined(ENABLE_SCREENSHOT)

void OnNavigateMessage(BrowserModule* browser_module,
                       const std::string& message) {
  GURL url(message);
  DLOG_IF(WARNING, !url.is_valid()) << "Invalid URL: " << message;
  if (url.is_valid()) {
    browser_module->Navigate(url);
  }
}

void OnReloadMessage(BrowserModule* browser_module,
                     const std::string& message) {
  UNREFERENCED_PARAMETER(message);
  browser_module->Navigate(GURL());
}

#if defined(ENABLE_SCREENSHOT)
void ScreenshotCompleteCallback(const FilePath& output_path) {
  DLOG(INFO) << "Screenshot written to " << output_path.value();
}

void OnScreenshotMessage(BrowserModule* browser_module,
                         const std::string& message) {
  UNREFERENCED_PARAMETER(message);
  FilePath dir;
  if (!PathService::Get(cobalt::paths::DIR_COBALT_DEBUG_OUT, &dir)) {
    NOTREACHED() << "Failed to get debug out directory.";
  }

  base::Time::Exploded exploded;
  base::Time::Now().LocalExplode(&exploded);
  DCHECK(exploded.HasValidValues());
  std::string screenshot_file_name =
      StringPrintf("screenshot-%04d-%02d-%02d_%02d-%02d-%02d.png",
                   exploded.year, exploded.month, exploded.day_of_month,
                   exploded.hour, exploded.minute, exploded.second);

  FilePath output_path = dir.Append(screenshot_file_name);
  browser_module->RequestScreenshotToFile(
      output_path, base::Bind(&ScreenshotCompleteCallback, output_path));
}
#endif  // defined(ENABLE_SCREENSHOT)

#endif  // defined(ENABLE_DEBUG_CONSOLE)

scoped_refptr<script::Wrappable> CreateH5VCC(
    const h5vcc::H5vcc::Settings& settings) {
  return scoped_refptr<script::Wrappable>(new h5vcc::H5vcc(settings));
}

}  // namespace

BrowserModule::BrowserModule(const GURL& url,
                             system_window::SystemWindow* system_window,
                             account::AccountManager* account_manager,
                             const Options& options)
    : storage_manager_(options.storage_manager_options),
      renderer_module_(system_window, options.renderer_module_options),
#if defined(ENABLE_GPU_ARRAY_BUFFER_ALLOCATOR)
      array_buffer_allocator_(new ResourceProviderArrayBufferAllocator(
          renderer_module_.pipeline()->GetResourceProvider())),
#endif  // defined(ENABLE_GPU_ARRAY_BUFFER_ALLOCATOR)
      media_module_(media::MediaModule::Create(
          renderer_module_.pipeline()->GetResourceProvider(),
          options.media_module_options)),
      network_module_(&storage_manager_, system_window->event_dispatcher(),
                      options.network_module_options),
      render_tree_combiner_(renderer_module_.pipeline()),
      self_message_loop_(MessageLoop::current()),
#if defined(ENABLE_DEBUG_CONSOLE)
      ALLOW_THIS_IN_INITIALIZER_LIST(navigate_command_handler_(
          kNavigateCommand,
          base::Bind(&OnNavigateMessage, base::Unretained(this)),
          kNavigateCommandShortHelp, kNavigateCommandLongHelp)),
      ALLOW_THIS_IN_INITIALIZER_LIST(reload_command_handler_(
          kReloadCommand, base::Bind(&OnReloadMessage, base::Unretained(this)),
          kReloadCommandShortHelp, kReloadCommandLongHelp)),
      ALLOW_THIS_IN_INITIALIZER_LIST(fuzzer_toggle_command_handler_(
          kFuzzerToggleCommand,
          base::Bind(&BrowserModule::OnFuzzerToggle, base::Unretained(this)),
          kFuzzerToggleCommandShortHelp, kFuzzerToggleCommandLongHelp)),
#if defined(ENABLE_SCREENSHOT)
      ALLOW_THIS_IN_INITIALIZER_LIST(screenshot_command_handler_(
          kScreenshotCommand,
          base::Bind(&OnScreenshotMessage, base::Unretained(this)),
          kScreenshotCommandShortHelp, kScreenshotCommandLongHelp)),
      screen_shot_writer_(new ScreenShotWriter(renderer_module_.pipeline())),
#endif  // defined(ENABLE_SCREENSHOT)
#endif  // defined(ENABLE_DEBUG_CONSOLE)
      ALLOW_THIS_IN_INITIALIZER_LIST(
          h5vcc_url_handler_(this, system_window, account_manager)),
      initial_url_(url),
      web_module_options_(options.web_module_options),
      has_resumed_(true, false),
      will_quit_(false) {
  // Setup our main web module to have the H5VCC API injected into it.
  DCHECK(!ContainsKey(web_module_options_.injected_window_attributes, "h5vcc"));
  h5vcc::H5vcc::Settings h5vcc_settings;
  h5vcc_settings.network_module = &network_module_;
  h5vcc_settings.account_manager = account_manager;
  web_module_options_.injected_window_attributes["h5vcc"] =
      base::Bind(&CreateH5VCC, h5vcc_settings);

#if defined(ENABLE_COMMAND_LINE_SWITCHES)
  CommandLine* command_line = CommandLine::ForCurrentProcess();

#if defined(ENABLE_DEBUG_CONSOLE)
  if (command_line->HasSwitch(switches::kInputFuzzer)) {
    OnFuzzerToggle(std::string());
  }
#endif
#endif  // ENABLE_COMMAND_LINE_SWITCHES

  input_device_manager_ = input::InputDeviceManager::CreateFromWindow(
      base::Bind(&BrowserModule::OnKeyEventProduced, base::Unretained(this)),
      system_window);

#if defined(ENABLE_DEBUG_CONSOLE)
  const math::Size& viewport_size =
      renderer_module_.render_target()->GetSurfaceInfo().size;

  debug_console_.reset(new DebugConsole(
      base::Bind(&BrowserModule::OnDebugConsoleRenderTreeProduced,
                 base::Unretained(this)),
      base::Bind(&BrowserModule::OnError, base::Unretained(this)),
      media_module_.get(), &network_module_, viewport_size,
      renderer_module_.pipeline()->GetResourceProvider(),
      renderer_module_.pipeline()->refresh_rate(),
      base::Bind(&BrowserModule::ExecuteJavascript, base::Unretained(this)),
      base::Bind(&BrowserModule::CreateDebugServer, base::Unretained(this))));
#endif  // defined(ENABLE_DEBUG_CONSOLE)

  // Always render the debug console. It will draw nothing if disabled.
  // This setting is ignored if ENABLE_DEBUG_CONSOLE is not defined.
  // TODO(***REMOVED***) Render tree combiner should probably be refactored.
  render_tree_combiner_.set_render_debug_console(true);

  NavigateWithCallbackInternal(url, base::Closure());
}

BrowserModule::~BrowserModule() {}

// Algorithm for Navigate:
//   https://www.w3.org/TR/html5/browsers.html#navigate
void BrowserModule::Navigate(const GURL& url) {
  DCHECK_EQ(MessageLoop::current(), self_message_loop_);
  NavigateWithCallback(url, base::Closure());
}

void BrowserModule::NavigateWithCallback(const GURL& url,
                                         const base::Closure& loaded_callback) {
  // Always post this as a task in case this is being called from the WebModule.
  self_message_loop_->PostTask(
      FROM_HERE, base::Bind(&BrowserModule::NavigateWithCallbackInternal,
                            base::Unretained(this), url, loaded_callback));
}

void BrowserModule::NavigateWithCallbackInternal(
    const GURL& url, const base::Closure& loaded_callback) {
  GURL new_url;

  if (url.is_empty()) {
    DCHECK(web_module_);
    new_url = web_module_->url();
    DLOG(INFO) << "Reloading " << new_url;
  } else {
    new_url = url;
    if (web_module_) {
      GURL old_url = web_module_->url();
      DLOG(INFO) << "Navigating to " << new_url << " from " << old_url;
      // 7. Fragment identifiers: Apply the URL parser algorithm to the absolute
      // URL of the new resource and the address of the active document of the
      // browsing context being navigated. If all the components of the
      // resulting parsed URLs, ignoring any fragment components, are identical,
      // and the new resource is to be fetched using HTTP GET or equivalent, and
      // the parsed URL of the new resource has a fragment component that is not
      // null (even if it is empty), then navigate to that fragment identifier
      // and abort these steps.
      // NOTE(***REMOVED***): This means, if the new url doesn't have hash, we should
      // always navigate to the new url, even if it is the same as the current
      // one.
      GURL::Replacements replacements;
      replacements.ClearRef();
      if (new_url.has_ref() &&
          new_url.ReplaceComponents(replacements) ==
              old_url.ReplaceComponents(replacements)) {
        web_module_->set_url(new_url);
        web_module_->InjectEvent(new dom::Event(base::Tokens::hashchange()));
        return;
      }
    }
  }

  // First try the registered handlers (e.g. for h5vcc://). If one of these
  // handles the URL, we don't use the web module.
  if (TryURLHandlers(new_url)) {
    return;
  }

  // Reset it explicitly first, so we don't get a memory high-watermark after
  // the second WebModule's construtor runs, but before scoped_ptr::reset() is
  // run.
  web_module_.reset(NULL);

  WebModule::Options options(web_module_options_);

  options.navigation_callback =
      base::Bind(&BrowserModule::Navigate, base::Unretained(this));
  if (!loaded_callback.is_null()) {
    options.loaded_callbacks.push_back(loaded_callback);
  }

  const math::Size& viewport_size =
      renderer_module_.render_target()->GetSurfaceInfo().size;

  // Show a splash screen while we're waiting for the web page to load.
  DestroySplashScreen();
  splash_screen_.reset(new SplashScreen(
      base::Bind(&BrowserModule::OnRenderTreeProduced, base::Unretained(this)),
      base::Bind(&BrowserModule::OnError, base::Unretained(this)),
      &network_module_, viewport_size,
      renderer_module_.pipeline()->GetResourceProvider(),
      renderer_module_.pipeline()->refresh_rate()));
  options.loaded_callbacks.push_back(
      base::Bind(&BrowserModule::DestroySplashScreen, base::Unretained(this)));
  options.array_buffer_allocator = array_buffer_allocator_.get();

  web_module_.reset(new WebModule(
      new_url,
      base::Bind(&BrowserModule::OnRenderTreeProduced, base::Unretained(this)),
      base::Bind(&BrowserModule::OnError, base::Unretained(this)),
      media_module_.get(), &network_module_, viewport_size,
      renderer_module_.pipeline()->GetResourceProvider(),
      renderer_module_.pipeline()->refresh_rate(), options));
}

void BrowserModule::SetPaused(bool paused) {
  // This method should not be called on the browser's own thread, as
  // we will be unable to signal the |has_resumed_| event when the
  // |Pause| method blocks the thread.
  DCHECK_NE(MessageLoop::current(), self_message_loop_);

  if (paused) {
    has_resumed_.Reset();
    self_message_loop_->PostTask(
        FROM_HERE, base::Bind(&BrowserModule::Pause, base::Unretained(this)));
  } else {
    has_resumed_.Signal();
  }
}

void BrowserModule::SetWillQuit() {
  base::AutoLock lock(quit_lock_);
  will_quit_ = true;
}

bool BrowserModule::WillQuit() {
  base::AutoLock lock(quit_lock_);
  return will_quit_;
}

void BrowserModule::Pause() {
  // This method must be called on the browser's own thread.
  DCHECK_EQ(MessageLoop::current(), self_message_loop_);

  media_module_->PauseAllPlayers();

  // Block the thread until the browser has been resumed.
  DLOG(INFO) << "Pausing browser loop with " << self_message_loop_->Size()
             << " items in queue.";
  has_resumed_.Wait();
  DLOG(INFO) << "Resuming browser loop with " << self_message_loop_->Size()
             << " items in queue.";

  if (!WillQuit()) {
    media_module_->ResumeAllPlayers();
  }
}

#if defined(ENABLE_SCREENSHOT)
void BrowserModule::RequestScreenshotToFile(const FilePath& path,
                                            const base::Closure& done_cb) {
  screen_shot_writer_->RequestScreenshot(path, done_cb);
}

void BrowserModule::RequestScreenshotToBuffer(
    const ScreenShotWriter::PNGEncodeCompleteCallback&
        encode_complete_callback) {
  screen_shot_writer_->RequestScreenshotToMemory(encode_complete_callback);
}
#endif

void BrowserModule::OnRenderTreeProduced(
    const browser::WebModule::LayoutResults& layout_results) {
  TRACE_EVENT0("cobalt::browser", "BrowserModule::OnRenderTreeProduced()");
  render_tree_combiner_.UpdateMainRenderTree(renderer::Submission(
      layout_results.render_tree, layout_results.animations,
      layout_results.layout_time));

#if defined(ENABLE_SCREENSHOT)
  screen_shot_writer_->SetLastPipelineSubmission(renderer::Submission(
      layout_results.render_tree, layout_results.animations,
      layout_results.layout_time));
#endif
}

#if defined(ENABLE_DEBUG_CONSOLE)
void BrowserModule::OnFuzzerToggle(const std::string& message) {
  if (MessageLoop::current() != self_message_loop_) {
    self_message_loop_->PostTask(FROM_HERE,
                                 base::Bind(&BrowserModule::OnFuzzerToggle,
                                            base::Unretained(this), message));
    return;
  }

  if (!input_device_manager_fuzzer_) {
    // Wire up the input fuzzer key generator to the keyboard event callback.
    input_device_manager_fuzzer_ = scoped_ptr<input::InputDeviceManager>(
        new input::InputDeviceManagerFuzzer(
            base::Bind(&BrowserModule::InjectKeyEventToMainWebModule,
                       base::Unretained(this))));
  } else {
    input_device_manager_fuzzer_.reset();
  }
}

void BrowserModule::OnDebugConsoleRenderTreeProduced(
    const browser::WebModule::LayoutResults& layout_results) {
  TRACE_EVENT0("cobalt::browser",
               "BrowserModule::OnDebugConsoleRenderTreeProduced()");
  render_tree_combiner_.UpdateDebugConsoleRenderTree(renderer::Submission(
      layout_results.render_tree, layout_results.animations,
      layout_results.layout_time));
}

#endif  // defined(ENABLE_DEBUG_CONSOLE)

void BrowserModule::OnKeyEventProduced(
    const scoped_refptr<dom::KeyboardEvent>& event) {
  if (MessageLoop::current() != self_message_loop_) {
    self_message_loop_->PostTask(FROM_HERE,
                                 base::Bind(&BrowserModule::OnKeyEventProduced,
                                            base::Unretained(this), event));
    return;
  }

  TRACE_EVENT0("cobalt::browser", "BrowserModule::OnKeyEventProduced()");

  // Filter the key event.
  if (!FilterKeyEvent(event)) {
    return;
  }

#if defined(ENABLE_DEBUG_CONSOLE)
  trace_manager.OnKeyEventProduced();
#endif  // defined(ENABLE_DEBUG_CONSOLE)

  InjectKeyEventToMainWebModule(event);
}

void BrowserModule::InjectKeyEventToMainWebModule(
    const scoped_refptr<dom::KeyboardEvent>& event) {
  web_module_->InjectEvent(event);
}

void BrowserModule::OnError(const std::string& error) {
  LOG(ERROR) << error;
  std::string url_string = "h5vcc://network-failure";

  // Retry the current URL. If there is no web module (this can happen in
  // certain cases), use the default URL.
  if (web_module_) {
    url_string += "?retry-url=" + web_module_->url().spec();
  } else {
    url_string += "?retry-url=" + initial_url_.spec();
  }

  NavigateWithCallbackInternal(GURL(url_string), base::Closure());
}

bool BrowserModule::FilterKeyEvent(
    const scoped_refptr<dom::KeyboardEvent>& event) {
  // Check for hotkeys first. If it is a hotkey, no more processing is needed.
  if (!FilterKeyEventForHotkeys(event)) {
    return false;
  }

#if defined(ENABLE_DEBUG_CONSOLE)
  // If the debug console is fully visible, it gets the next chance to handle
  // key events.
  if (debug_console_->GetMode() >= debug::DebugHub::kDebugConsoleOn) {
    if (!debug_console_->FilterKeyEvent(event)) {
      return false;
    }
  }
#endif  // defined(ENABLE_DEBUG_CONSOLE)

  return true;
}

bool BrowserModule::FilterKeyEventForHotkeys(
    const scoped_refptr<dom::KeyboardEvent>& event) {
#if !defined(ENABLE_DEBUG_CONSOLE)
  UNREFERENCED_PARAMETER(event);
#else
  if (event->key_code() == dom::keycode::kF1 ||
      (event->ctrl_key() && event->key_code() == dom::keycode::kO)) {
    if (event->type() == base::Tokens::keydown()) {
      // Ctrl+O toggles the debug console display.
      debug_console_->CycleMode();
    }
    return false;
  }
#endif  // defined(ENABLE_DEBUG_CONSOLE)

  return true;
}

void BrowserModule::AddURLHandler(
    const URLHandler::URLHandlerCallback& callback) {
  url_handlers_.push_back(callback);
}

void BrowserModule::RemoveURLHandler(
    const URLHandler::URLHandlerCallback& callback) {
  for (URLHandlerCollection::iterator iter = url_handlers_.begin();
       iter != url_handlers_.end(); ++iter) {
    if (iter->Equals(callback)) {
      url_handlers_.erase(iter);
      return;
    }
  }
}

bool BrowserModule::TryURLHandlers(const GURL& url) {
  for (URLHandlerCollection::const_iterator iter = url_handlers_.begin();
       iter != url_handlers_.end(); ++iter) {
    if (iter->Run(url)) {
      return true;
    }
  }

  // No registered handler handled the URL, let the caller handle it.
  return false;
}

void BrowserModule::DestroySplashScreen() { splash_screen_.reset(NULL); }

#if defined(ENABLE_WEBDRIVER)
scoped_ptr<webdriver::SessionDriver> BrowserModule::CreateSessionDriver(
    const webdriver::protocol::SessionId& session_id) {
  return make_scoped_ptr(new webdriver::SessionDriver(
      session_id,
      base::Bind(&BrowserModule::NavigateWithCallback, base::Unretained(this)),
      base::Bind(&BrowserModule::CreateWindowDriver, base::Unretained(this))));
}
#endif

void BrowserModule::SetProxy(const std::string& proxy_rules) {
  // NetworkModule will ensure this happens on the correct thread.
  network_module_.SetProxy(proxy_rules);
}

}  // namespace browser
}  // namespace cobalt
