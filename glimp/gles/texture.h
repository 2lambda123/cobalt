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

#ifndef GLIMP_GLES_TEXTURE_H_
#define GLIMP_GLES_TEXTURE_H_

#include <GLES3/gl3.h>

#include "glimp/gles/pixel_format.h"
#include "glimp/gles/texture_impl.h"
#include "glimp/nb/ref_counted.h"
#include "glimp/nb/scoped_ptr.h"

namespace glimp {
namespace gles {

class Texture : public nb::RefCountedThreadSafe<Texture> {
 public:
  explicit Texture(nb::scoped_ptr<TextureImpl> impl);

  // Called when glBindTexture() is called.
  void SetTarget(GLenum target);

  // Implements support for glTexImage2D().
  void SetData(GLint level,
               PixelFormat pixel_format,
               GLsizei width,
               GLsizei height,
               int pitch_in_bytes,
               const GLvoid* pixels);

  // Implements support for glTexSubImage2D().
  void UpdateData(GLint level,
                  GLint xoffset,
                  GLint yoffset,
                  GLsizei width,
                  GLsizei height,
                  int pitch_in_bytes,
                  const GLvoid* pixels);

  // Returns true if this texture can be used as a framebuffer component.
  // Essentially, this function is asking whether we can render to the texture
  // or not.
  bool CanBeAttachedToFramebuffer() const;

  // Returns true if the target has been set (e.g. via glBindTexture()).
  bool target_valid() const { return target_valid_; }

  // Returns the target (set via glBindTexture()).  Must be called only if
  // target_valid() is true.
  GLenum target() const {
    SB_DCHECK(target_valid_);
    return target_;
  }

  TextureImpl* impl() const { return impl_.get(); }

  // Returns whether the data has been set yet or not.
  bool texture_allocated() const { return texture_allocated_; }

  int width() const {
    SB_DCHECK(texture_allocated_);
    return width_;
  }

  int height() const {
    SB_DCHECK(texture_allocated_);
    return height_;
  }

  PixelFormat pixel_format() const {
    SB_DCHECK(texture_allocated_);
    return pixel_format_;
  }

 private:
  friend class nb::RefCountedThreadSafe<Texture>;
  ~Texture() {}

  nb::scoped_ptr<TextureImpl> impl_;

  // The target type this texture was last bound as, set through a call to
  // glBindTexture().
  GLenum target_;

  // Represents whether or not target_ as been initialized yet.
  bool target_valid_;

  // True if underlying texture data has been allocated yet or not (e.g.
  // will be true after glTexImage2D() is called.)
  bool texture_allocated_;

  // The width and height of the texture, in pixels.
  int width_;
  int height_;

  // The pixel format of the set data.
  PixelFormat pixel_format_;
};

}  // namespace gles
}  // namespace glimp

#endif  // GLIMP_GLES_TEXTURE_H_
