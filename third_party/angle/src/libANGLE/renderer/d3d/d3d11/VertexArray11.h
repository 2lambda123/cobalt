//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VertexArray11.h: Defines the rx::VertexArray11 class which implements rx::VertexArrayImpl.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_VERTEXARRAY11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_VERTEXARRAY11_H_

#include "libANGLE/Framebuffer.h"
#include "libANGLE/renderer/VertexArrayImpl.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "libANGLE/signal_utils.h"

namespace rx
{
class Renderer11;

class VertexArray11 : public VertexArrayImpl, public OnBufferDataDirtyReceiver
{
  public:
    VertexArray11(const gl::VertexArrayState &data);
    ~VertexArray11() override;

    void syncState(ContextImpl *contextImpl, const gl::VertexArray::DirtyBits &dirtyBits) override;
    // This will flush any pending attrib updates and then check the dynamic attribs mask.
    bool hasDynamicAttrib(const gl::State &state);
    gl::Error updateDirtyAndDynamicAttribs(VertexDataManager *vertexDataManager,
                                           const gl::State &state,
                                           GLint start,
                                           GLsizei count,
                                           GLsizei instances);
    void clearDirtyAndPromoteDynamicAttribs(const gl::State &state, GLsizei count);

    const std::vector<TranslatedAttribute> &getTranslatedAttribs() const;

    // SignalReceiver implementation
    void signal(size_t channelID) override;

  private:
    void updateVertexAttribStorage(size_t attribIndex);
    void flushAttribUpdates(const gl::State &state);

    std::vector<VertexStorageType> mAttributeStorageTypes;
    std::vector<TranslatedAttribute> mTranslatedAttribs;

    // The mask of attributes marked as dynamic.
    gl::AttributesMask mDynamicAttribsMask;

    // A mask of attributes that need to be re-evaluated.
    gl::AttributesMask mAttribsToUpdate;

    // A set of attributes we know are dirty, and need to be re-translated.
    gl::AttributesMask mAttribsToTranslate;

    // We need to keep a safe pointer to the Buffer so we can attach the correct dirty callbacks.
    std::vector<BindingPointer<gl::Buffer>> mCurrentBuffers;

    std::vector<OnBufferDataDirtyBinding> mOnBufferDataDirty;
};

}  // namespace rx

#endif // LIBANGLE_RENDERER_D3D_D3D11_VERTEXARRAY11_H_
