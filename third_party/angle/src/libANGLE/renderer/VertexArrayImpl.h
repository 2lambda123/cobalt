//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VertexAttribImpl.h: Defines the abstract rx::VertexAttribImpl class.

#ifndef LIBANGLE_RENDERER_VERTEXARRAYIMPL_H_
#define LIBANGLE_RENDERER_VERTEXARRAYIMPL_H_

#include "common/angleutils.h"
#include "libANGLE/Buffer.h"
#include "libANGLE/VertexArray.h"

namespace rx
{
class ContextImpl;

class VertexArrayImpl : angle::NonCopyable
{
  public:
    VertexArrayImpl(const gl::VertexArrayState &data) : mData(data) {}
    virtual ~VertexArrayImpl() { }
    virtual void syncState(ContextImpl *contextImpl, const gl::VertexArray::DirtyBits &dirtyBits) {}
  protected:
    const gl::VertexArrayState &mData;
};

}

#endif // LIBANGLE_RENDERER_VERTEXARRAYIMPL_H_
