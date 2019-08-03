//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Framebuffer11.h: Defines the Framebuffer11 class.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_FRAMBUFFER11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_FRAMBUFFER11_H_

#include "libANGLE/renderer/d3d/FramebufferD3D.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"
#include "libANGLE/signal_utils.h"

namespace rx
{
class Renderer11;

class Framebuffer11 : public FramebufferD3D, public OnRenderTargetDirtyReceiver
{
  public:
    Framebuffer11(const gl::FramebufferState &data, Renderer11 *renderer);
    virtual ~Framebuffer11();

    gl::Error discard(size_t count, const GLenum *attachments) override;
    gl::Error invalidate(size_t count, const GLenum *attachments) override;
    gl::Error invalidateSub(size_t count, const GLenum *attachments, const gl::Rectangle &area) override;

    // Invalidate the cached swizzles of all bound texture attachments.
    gl::Error markAttachmentsDirty() const;

    void syncState(ContextImpl *contextImpl, const gl::Framebuffer::DirtyBits &dirtyBits) override;

    const RenderTargetArray &getCachedColorRenderTargets() const
    {
        return mCachedColorRenderTargets;
    }
    const RenderTarget11 *getCachedDepthStencilRenderTarget() const
    {
        return mCachedDepthStencilRenderTarget;
    }

    bool hasAnyInternalDirtyBit() const;
    void syncInternalState(ContextImpl *contextImpl);

    void signal(size_t channelID) override;

    gl::Error getSamplePosition(size_t index, GLfloat *xy) const override;

  private:
    gl::Error clearImpl(ContextImpl *context, const ClearParameters &clearParams) override;

    gl::Error readPixelsImpl(const gl::Rectangle &area,
                             GLenum format,
                             GLenum type,
                             size_t outputPitch,
                             const gl::PixelPackState &pack,
                             uint8_t *pixels) const override;

    gl::Error blitImpl(const gl::Rectangle &sourceArea,
                       const gl::Rectangle &destArea,
                       const gl::Rectangle *scissor,
                       bool blitRenderTarget,
                       bool blitDepth,
                       bool blitStencil,
                       GLenum filter,
                       const gl::Framebuffer *sourceFramebuffer) override;

    gl::Error invalidateBase(size_t count, const GLenum *attachments, bool useEXTBehavior) const;
    gl::Error invalidateAttachment(const gl::FramebufferAttachment *attachment) const;

    GLenum getRenderTargetImplementationFormat(RenderTargetD3D *renderTarget) const override;

    void updateColorRenderTarget(size_t colorIndex);
    void updateDepthStencilRenderTarget();

    Renderer11 *const mRenderer;
    RenderTargetArray mCachedColorRenderTargets;
    RenderTarget11 *mCachedDepthStencilRenderTarget;

    std::vector<OnRenderTargetDirtyBinding> mColorRenderTargetsDirty;
    OnRenderTargetDirtyBinding mDepthStencilRenderTargetDirty;

    gl::Framebuffer::DirtyBits mInternalDirtyBits;
};

}

#endif // LIBANGLE_RENDERER_D3D_D3D11_FRAMBUFFER11_H_
