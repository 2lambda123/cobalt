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

#ifndef RENDERER_RENDERER_MODULE_H_
#define RENDERER_RENDERER_MODULE_H_

#include "cobalt/render_tree/node.h"
#include "cobalt/renderer/backend/graphics_system.h"
#include "cobalt/renderer/backend/display.h"
#include "cobalt/renderer/backend/graphics_context.h"
#include "cobalt/renderer/rasterizer.h"
#include "cobalt/renderer/pipeline.h"

namespace cobalt {
namespace renderer {

class RendererModule {
 public:
  struct Options {
    Options();

    bool use_hardware_skia_rasterizer;
  };

  explicit RendererModule(const Options& options);
  ~RendererModule();

  renderer::Pipeline* pipeline() { return pipeline_.get(); }

 private:
  scoped_ptr<renderer::backend::GraphicsSystem> graphics_system_;
  scoped_ptr<renderer::backend::Display> display_;
  scoped_ptr<renderer::backend::GraphicsContext> primary_graphics_context_;
  scoped_ptr<renderer::Rasterizer> rasterizer_;
  scoped_ptr<renderer::Pipeline> pipeline_;
};

}  // namespace renderer
}  // namespace cobalt

#endif  // RENDERER_RENDERER_MODULE_H_
