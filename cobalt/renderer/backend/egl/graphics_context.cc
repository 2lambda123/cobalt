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

#include "cobalt/renderer/backend/egl/graphics_context.h"

#include <GLES2/gl2.h>

#include "cobalt/base/polymorphic_downcast.h"
#include "cobalt/renderer/backend/egl/texture.h"
#include "cobalt/renderer/backend/egl/utils.h"

namespace cobalt {
namespace renderer {
namespace backend {

namespace {

bool HasExtension(const char* extension) {
  const char* raw_extension_string =
      reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
  DCHECK(raw_extension_string);

  return strstr(raw_extension_string, extension) != NULL;
}

}  // namespace

GraphicsContextEGL::GraphicsContextEGL(EGLDisplay display, EGLConfig config)
    : display_(display),
      config_(config) {
  EGLint context_attrib_list[] = {
      EGL_CONTEXT_CLIENT_VERSION, 2,
      EGL_NONE,
  };

  // Create the OpenGL ES 2.0 context.
  context_ =
      eglCreateContext(display, config, EGL_NO_CONTEXT, context_attrib_list);
  CHECK_EQ(EGL_SUCCESS, eglGetError());

  // Create a dummy EGLSurface object to be assigned as the target surface
  // when we need to make OpenGL calls that do not depend on a surface (e.g.
  // creating a texture).
  EGLint null_surface_attrib_list[] = {
      EGL_WIDTH, 1,
      EGL_HEIGHT, 1,
      EGL_NONE,
  };
  null_surface_ =
      eglCreatePbufferSurface(display, config, null_surface_attrib_list);
  CHECK_EQ(EGL_SUCCESS, eglGetError());

  MakeCurrent();

  bgra_format_supported_ = HasExtension("GL_EXT_texture_format_BGRA8888");

  SetupBlitToRenderTargetObjects();

  ReleaseCurrentContext();
}

void GraphicsContextEGL::SetupBlitToRenderTargetObjects() {
  // Setup shaders used when blitting the current texture.
  blit_program_ = glCreateProgram();

  blit_vertex_shader_ = glCreateShader(GL_VERTEX_SHADER);
  const char* blit_vertex_shader_source = "\
      attribute vec2 a_position;\
      attribute vec2 a_tex_coord;\
      varying vec2 v_tex_coord;\
      \
      void main() {\
        gl_Position = vec4(a_position.x, a_position.y, 0, 1);\
        v_tex_coord = a_tex_coord;\
      }\
      ";
  int blit_vertex_shader_source_length = strlen(blit_vertex_shader_source);
  GL_CALL(glShaderSource(blit_vertex_shader_, 1, &blit_vertex_shader_source,
                         &blit_vertex_shader_source_length));
  GL_CALL(glCompileShader(blit_vertex_shader_));
  GL_CALL(glAttachShader(blit_program_, blit_vertex_shader_));

  blit_fragment_shader_ = glCreateShader(GL_FRAGMENT_SHADER);
  const char* blit_fragment_shader_source = "\
      precision mediump float;\
      varying vec2 v_tex_coord;\
      uniform sampler2D texture;\
      \
      void main() {\
      gl_FragColor = texture2D(texture, v_tex_coord);\
      }\
      ";
  int blit_fragment_shader_source_length = strlen(blit_fragment_shader_source);
  GL_CALL(glShaderSource(blit_fragment_shader_, 1, &blit_fragment_shader_source,
                         &blit_fragment_shader_source_length));
  GL_CALL(glCompileShader(blit_fragment_shader_));
  GL_CALL(glAttachShader(blit_program_, blit_fragment_shader_));

  GL_CALL(glBindAttribLocation(
      blit_program_, kBlitPositionAttribute, "a_position"));
  GL_CALL(glBindAttribLocation(
      blit_program_, kBlitTexcoordAttribute, "a_tex_coord"));

  GL_CALL(glLinkProgram(blit_program_));

  // Setup a vertex buffer that can blit a quad with a full texture, to be used
  // by Frame::BlitToRenderTarget().
  struct QuadVertex {
    float position_x;
    float position_y;
    float tex_coord_u;
    float tex_coord_v;
  };
  const QuadVertex kQuadVerts[4] = {
      {-1.0f, -1.0f, 0.0f, 1.0f},
      {-1.0f, 1.0f, 0.0f, 0.0f},
      {1.0f, -1.0f, 1.0f, 1.0f},
      {1.0f, 1.0, 1.0f, 0.0f},
  };
  GL_CALL(glGenBuffers(1, &blit_vertex_buffer_));
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, blit_vertex_buffer_));
  GL_CALL(glBufferData(
      GL_ARRAY_BUFFER, sizeof(kQuadVerts), kQuadVerts, GL_STATIC_DRAW));
}

GraphicsContextEGL::~GraphicsContextEGL() {
  MakeCurrent();

  GL_CALL(glDeleteBuffers(1, &blit_vertex_buffer_));
  GL_CALL(glDeleteProgram(blit_program_));
  GL_CALL(glDeleteShader(blit_fragment_shader_));
  GL_CALL(glDeleteShader(blit_vertex_shader_));

  ReleaseCurrentContext();

  EGL_CALL(eglDestroySurface(display_, null_surface_));
  EGL_CALL(eglDestroyContext(display_, context_));
}

void GraphicsContextEGL::MakeCurrentWithSurface(EGLSurface surface) {
  DCHECK_NE(EGL_NO_SURFACE, surface) <<
      "Use ReleaseCurrentContext().";

  EGL_CALL(eglMakeCurrent(display_, surface, surface, context_));
}

void GraphicsContextEGL::MakeCurrent() {
  MakeCurrentWithSurface(null_surface_);
}

void GraphicsContextEGL::ReleaseCurrentContext() {
  EGL_CALL(eglMakeCurrent(
      display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
}

scoped_ptr<TextureData> GraphicsContextEGL::AllocateTextureData(
    const SurfaceInfo& surface_info) {
  return scoped_ptr<TextureData>(new TextureDataEGL(surface_info));
}

scoped_ptr<Texture> GraphicsContextEGL::CreateTexture(
    scoped_ptr<TextureData> texture_data) {
  scoped_ptr<TextureDataEGL> texture_data_egl(
      base::polymorphic_downcast<TextureDataEGL*>(texture_data.release()));

  scoped_ptr<Texture> texture(
      new TextureEGL(this, texture_data_egl.Pass(), bgra_format_supported_));

  return texture.Pass();
}

scoped_ptr<RawTextureMemory> GraphicsContextEGL::AllocateRawTextureMemory(
    size_t size_in_bytes, size_t alignment) {
  // TODO(***REMOVED***): Needs an implementation.  We probably want to use PBuffers
  //               for this, but it's not obvious right to me how to do that
  //               in such a way that can be done from any thread (and thus
  //               doesn't require a context to be current).
  NOTREACHED();
  return scoped_ptr<RawTextureMemory>();
}

scoped_ptr<Texture> GraphicsContextEGL::CreateTextureFromRawMemory(
    const scoped_refptr<ConstRawTextureMemory>& raw_texture_memory,
    intptr_t offset, const SurfaceInfo& surface_info, int pitch_in_bytes) {
  NOTREACHED();
  return scoped_ptr<Texture>();
}

scoped_refptr<RenderTarget> GraphicsContextEGL::CreateOffscreenRenderTarget(
    const math::Size& dimensions) {
  scoped_refptr<RenderTarget> render_target(new PBufferRenderTargetEGL(
      display_, config_, dimensions));

  return render_target;
}

scoped_ptr<Texture> GraphicsContextEGL::CreateTextureFromOffscreenRenderTarget(
    const scoped_refptr<RenderTarget>& render_target) {
  PBufferRenderTargetEGL* pbuffer_render_target =
      base::polymorphic_downcast<PBufferRenderTargetEGL*>(render_target.get());

  scoped_ptr<Texture> texture(new TextureEGL(this, pbuffer_render_target));

  return texture.Pass();
}

namespace {
void VerticallyFlipPixels(uint8_t* pixels, int pitch_in_bytes, int height) {
  int half_height = height / 2;
  for (int row = 0; row < half_height; ++row) {
    uint8_t* top_row = pixels + row * pitch_in_bytes;
    uint8_t* bottom_row =
        pixels + (height - 1 - row) * pitch_in_bytes;
    for (int i = 0; i < pitch_in_bytes; ++i) {
      std::swap(top_row[i], bottom_row[i]);
    }
  }
}
}  // namespace

scoped_array<uint8_t> GraphicsContextEGL::GetCopyOfTexturePixelDataAsRGBA(
    const Texture& texture) {
  const TextureEGL* texture_egl =
      base::polymorphic_downcast<const TextureEGL*>(&texture);
  MakeCurrent();

  GLuint texture_framebuffer;
  GL_CALL(glGenFramebuffers(1, &texture_framebuffer));
  GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, texture_framebuffer));

  GL_CALL(glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
      texture_egl->gl_handle(), 0));
  DCHECK_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

  const SurfaceInfo& surface_info = texture.GetSurfaceInfo();

  scoped_array<uint8_t> pixels(
      new uint8_t[surface_info.size.width() * surface_info.size.height() *
                  surface_info.BytesPerPixel()]);
  GL_CALL(glReadPixels(0, 0, surface_info.size.width(),
                       surface_info.size.height(), GL_RGBA, GL_UNSIGNED_BYTE,
                       pixels.get()));

  GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
  GL_CALL(glDeleteFramebuffers(1, &texture_framebuffer));

  GL_CALL(glFinish());
  ReleaseCurrentContext();

  // Vertically flip the resulting pixel data before returning so that the 0th
  // pixel is at the top-left.  While this is not a fast procedure, this
  // entire function is only intended to be used in debug/test code.
  int pitch_in_bytes = surface_info.size.width() * surface_info.BytesPerPixel();
#if !defined(__LB_LINUX__)
  // Flip the pixels so that (0,0) is at the top-left.  The Mesa Gallium
  // EGL implementation on Linux seems to return already flipped pixels.
  VerticallyFlipPixels(pixels.get(), pitch_in_bytes,
                       surface_info.size.height());
#endif

  return pixels.Pass();
}

GraphicsContextEGL::Frame::Frame(GraphicsContextEGL* owner, EGLSurface surface)
    : owner_(owner), scoped_make_current_(owner, surface) {}

GraphicsContextEGL::Frame::~Frame() { owner_->OnFrameEnd(); }

void GraphicsContextEGL::Frame::Clear(float red, float green, float blue,
                                      float alpha) {
  const SurfaceInfo& target_surface_info =
      owner_->render_target_->GetSurfaceInfo();
  GL_CALL(glViewport(0, 0, target_surface_info.size.width(),
                     target_surface_info.size.height()));
  GL_CALL(glScissor(0, 0, target_surface_info.size.width(),
                    target_surface_info.size.height()));

  GL_CALL(glClearColor(red, green, blue, alpha));
  GL_CALL(glClear(GL_COLOR_BUFFER_BIT));
}

void GraphicsContextEGL::Frame::BlitToRenderTarget(const Texture& texture) {
  const TextureEGL* texture_egl =
      base::polymorphic_downcast<const TextureEGL*>(&texture);
  const SurfaceInfo& target_surface_info =
      owner_->render_target_->GetSurfaceInfo();

  // Render a texture as a full-screen quad to the output render target.
  GL_CALL(glViewport(0, 0, target_surface_info.size.width(),
                     target_surface_info.size.height()));
  GL_CALL(glScissor(0, 0, target_surface_info.size.width(),
                    target_surface_info.size.height()));

  GL_CALL(glUseProgram(owner_->blit_program_));

  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, owner_->blit_vertex_buffer_));
  GL_CALL(glVertexAttribPointer(kBlitPositionAttribute, 2, GL_FLOAT, GL_FALSE,
                                sizeof(float) * 4, 0));
  GL_CALL(glVertexAttribPointer(kBlitTexcoordAttribute, 2, GL_FLOAT, GL_FALSE,
                                sizeof(float) * 4,
                                reinterpret_cast<GLvoid*>(sizeof(float) * 2)));
  GL_CALL(glEnableVertexAttribArray(kBlitPositionAttribute));
  GL_CALL(glEnableVertexAttribArray(kBlitTexcoordAttribute));

  GL_CALL(glActiveTexture(GL_TEXTURE0));
  GL_CALL(glBindTexture(GL_TEXTURE_2D, texture_egl->gl_handle()));

  GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

  GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));

  GL_CALL(glDisableVertexAttribArray(kBlitTexcoordAttribute));
  GL_CALL(glDisableVertexAttribArray(kBlitPositionAttribute));
  GL_CALL(glUseProgram(0));
}

scoped_ptr<GraphicsContext::Frame> GraphicsContextEGL::StartFrame(
    const scoped_refptr<RenderTarget>& render_target) {
  render_target_ = make_scoped_refptr(
      base::polymorphic_downcast<RenderTargetEGL*>(render_target.get()));

  return scoped_ptr<GraphicsContext::Frame>(
      new Frame(this, render_target_->GetSurface()));
}

void GraphicsContextEGL::OnFrameEnd() {
  GL_CALL(glFlush());
  EGL_CALL(eglSwapBuffers(display_, render_target_->GetSurface()));

  render_target_ = NULL;
}

}  // namespace backend
}  // namespace renderer
}  // namespace cobalt
