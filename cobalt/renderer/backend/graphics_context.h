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

#ifndef RENDERER_BACKEND_GRAPHICS_CONTEXT_H_
#define RENDERER_BACKEND_GRAPHICS_CONTEXT_H_

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "cobalt/math/size.h"
#include "cobalt/renderer/backend/render_target.h"
#include "cobalt/renderer/backend/surface_info.h"
#include "cobalt/renderer/backend/texture.h"

namespace cobalt {
namespace renderer {
namespace backend {

// The GraphicsContext captures the concept of a data channel to the GPU.
// The above definition implies that all graphics commands must eventually
// be issued through a graphics context. Basic rendering functionality
// is provided by the abstract interface defined below, and this is to allow
// for easy and quick rendering of images in sandbox or debug settings.  For
// more elaborate hardware acceleration, platform-specific graphics context
// objects must be acquired from a platform's concrete GraphicsContext object
// and then issued to that directly. A graphics context will always be
// associated with a render target on which all rendering output will appear.
// Commands issued through the graphics context may or may not be executed up
// until Submit() is called, but after it is called they will definitely be
// scheduled for execution.
class GraphicsContext {
 public:
  virtual ~GraphicsContext() {}

  // This method will allocate CPU-accessible memory with the given
  // SurfaceInfo specifications.  The resulting TextureData object
  // allows access to pixel memory which the caller can write to and eventually
  // pass the object in to CreateTexture() to finalize a texture.
  virtual scoped_ptr<TextureData> AllocateTextureData(
      const SurfaceInfo& surface_info) = 0;

  // Constructs a texture from the given formatted pixel data.
  virtual scoped_ptr<Texture> CreateTexture(
      scoped_ptr<TextureData> texture_data) = 0;

  // This function can be used to allocate a chunk of memory that is potentially
  // directly accessible by the GPU as a texture.  This would be used along
  // with functions like CreateTextureFromRawMemory().  In general, one should
  // use AllocateTextureData() instead, as it is not sensitive to platform
  // specific details.
  virtual scoped_ptr<RawTextureMemory> AllocateRawTextureMemory(
      size_t size_in_bytes, size_t alignment) = 0;

  // Creates a texture from a raw chunk of contiguous texture memory allocated
  // via AllocateRawTextureMemory().  The RawTextureMemory return value of
  // AllocateRawTextureMemory() must be converted to a ConstRawTextureMemory
  // object before being passed to this function, to ensure that the data does
  // not change once the GPU potentially has access to it.  offset specifies
  // the offset from the beginning of raw_texture_memory the actual texture
  // data begins at.  An example case where this method is useful is to support
  // multi-plane image data, where the image data for all planes is stored in
  // raw_texture_memory, but a separate texture is created for each different
  // plane.
  virtual scoped_ptr<Texture> CreateTextureFromRawMemory(
      const scoped_refptr<ConstRawTextureMemory>& raw_texture_memory,
      intptr_t offset, const SurfaceInfo& surface_info, int pitch_in_bytes) = 0;

  // Constructs a texture from a pointer to raw memory.  This method will likely
  // require a pixel copy to take place, and it is recommended that
  // CreateTexture() be used instead, if possible.
  virtual scoped_ptr<Texture> CreateTextureFromCopy(
      const SurfaceInfo& surface_info, int pitch_in_bytes,
      const uint8_t* pixel_data);

  // Creates an offscreen render target that can be targeted by a
  // GraphicsContext::Frame.  This might be used in unit tests where a
  // display render target is not needed and we simply wish to render to an
  // offscreen buffer and analyze the results offline.
  // This method allows platforms to choose their most compatible format that
  // can express RGBA, though not necessarily in that order.  The chosen format
  // can be queried from the render target after it has been created.
  virtual scoped_refptr<RenderTarget> CreateOffscreenRenderTarget(
      const math::Size& dimensions) = 0;

  // Creates a texture that references the same image as a render target.  This
  // is useful for acquiring a texture for subsequent render passes from a
  // render target that was recently rendered to.
  virtual scoped_ptr<Texture> CreateTextureFromOffscreenRenderTarget(
      const scoped_refptr<RenderTarget>& render_target) = 0;

  // Saves texture pixels to CPU memory that is then returned as a scoped_array.
  // This function will wait for the pipeline to flush, so it is slow and should
  // only be used in a debug context.  One use case for the returned image
  // memory is to encode it and serialize it as a PNG file for offline viewing.
  // The format of the returned memory can be found by examining the results of
  // texture.GetSurfaceInfo().  The pitch of each row is equal to the width.
  // The pixel format of the returned data is always RGBA8, in that order.
  // The output alpha format is always unpremultiplied alpha.
  virtual scoped_array<uint8_t> GetCopyOfTexturePixelDataAsRGBA(
      const Texture& texture) = 0;

  // The interface for creating and submitting frames to be rendered is
  // to call GraphicsContext::StartFrame() which instantiates a
  // GraphicsContext::Frame object which binds the context to a render target.
  // The created Frame's destruction is the signal that the frame is complete
  // and should be flushed.
  class Frame {
   public:
    virtual ~Frame() {}

    // Clear the screen with the specified color.
    virtual void Clear(float red, float green, float blue, float alpha) = 0;

    // Renders the specified texture to the entire associated render target,
    // stretching if necessary.  This method is provides a method for a software
    // rasterized image to be sent to the display.
    // TODO(***REMOVED***): Re-evaluate if there's a better home for this function that
    //               better conveys the function's specific use-case for sending
    //               software-rendered images to the display.  b/19081247
    virtual void BlitToRenderTarget(const Texture& texture) = 0;
  };

  virtual scoped_ptr<Frame> StartFrame(
      const scoped_refptr<backend::RenderTarget>& render_target) = 0;
};

}  // namespace backend
}  // namespace renderer
}  // namespace cobalt

#endif  // RENDERER_BACKEND_GRAPHICS_CONTEXT_H_
