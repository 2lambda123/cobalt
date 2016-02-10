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

#ifndef COBALT_RENDERER_BACKEND_RENDER_TARGET_STUB_H_
#define COBALT_RENDERER_BACKEND_RENDER_TARGET_STUB_H_

#include "cobalt/renderer/backend/pixel_data_stub.h"
#include "cobalt/renderer/backend/render_target.h"
#include "cobalt/renderer/backend/surface_info.h"

namespace cobalt {
namespace renderer {
namespace backend {

// A render target for the stub graphics system.
// The render target has no functionality and returns fake but valid metadata
// if queried.
class RenderTargetStub : public RenderTarget {
 public:
  explicit RenderTargetStub(const SurfaceInfo& surface_info) :
      pixel_data_(new PixelDataStub(surface_info)) {}

  const SurfaceInfo& GetSurfaceInfo() OVERRIDE {
    return pixel_data_->surface_info();
  }

  const scoped_refptr<PixelDataStub>& pixel_data() const { return pixel_data_; }

  intptr_t GetPlatformHandle() OVERRIDE { return 0; }

 private:
  ~RenderTargetStub() {}

  scoped_refptr<PixelDataStub> pixel_data_;
};

}  // namespace backend
}  // namespace renderer
}  // namespace cobalt

#endif  // COBALT_RENDERER_BACKEND_RENDER_TARGET_STUB_H_
