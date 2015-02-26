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

#include "cobalt/renderer/renderer_module.h"

#include "base/debug/trace_event.h"
#include "cobalt/renderer/backend/default_graphics_system.h"
#include "cobalt/renderer/rasterizer_skia/hardware_rasterizer.h"
#include "cobalt/renderer/rasterizer_skia/software_rasterizer.h"

namespace cobalt {
namespace renderer {

RendererModule::Options::Options() {
  // Call into platform-specific code for setting up render module options.
  SetPerPlatformDefaultOptions();
}

RendererModule::RendererModule(const Options& options) {
  TRACE_EVENT0("cobalt::renderer", "RendererModule::RendererModule()");

  // Load up the platform's default graphics system.
  {
    TRACE_EVENT0("cobalt::renderer", "backend::CreateDefaultGraphicsSystem()");
    graphics_system_ = backend::CreateDefaultGraphicsSystem();
  }

  // Create/initialize the default display
  {
    TRACE_EVENT0("cobalt::renderer", "GraphicsSystem::CreateDefaultDisplay()");
    display_ = graphics_system_->CreateDefaultDisplay();
  }

  // Create a graphics context associated with the default display's render
  // target so that we have a channel to write to the display.
  {
    TRACE_EVENT0("cobalt::renderer", "GraphicsSystem::CreateGraphicsContext()");
    graphics_context_ = graphics_system_->CreateGraphicsContext();
  }

  // Create a rasterizer to rasterize our render trees.
  scoped_ptr<renderer::Rasterizer> rasterizer;
  {
    TRACE_EVENT0("cobalt::renderer", "Create Rasterizer");
    rasterizer =
        options.create_rasterizer_function.Run(graphics_context_.get());
  }

  // Setup the threaded rendering pipeline and fit our newly created rasterizer
  // into it, and direct it to render directly to the display.
  {
    TRACE_EVENT0("cobalt::renderer", "new renderer::Pipeline()");
    pipeline_ = make_scoped_ptr(
        new renderer::Pipeline(rasterizer.Pass(), display_->GetRenderTarget()));
  }
}

RendererModule::~RendererModule() {
}

}  // namespace renderer
}  // namespace cobalt
