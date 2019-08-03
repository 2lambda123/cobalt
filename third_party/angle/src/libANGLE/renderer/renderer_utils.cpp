//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// renderer_utils:
//   Helper methods pertaining to most or all back-ends.
//

#include "libANGLE/renderer/renderer_utils.h"

#include "image_util/copyimage.h"
#include "image_util/imageformats.h"

#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/Format.h"

#include <string.h>

namespace rx
{

namespace
{
typedef std::pair<gl::FormatType, ColorWriteFunction> FormatWriteFunctionPair;
typedef std::map<gl::FormatType, ColorWriteFunction> FormatWriteFunctionMap;

static inline void InsertFormatWriteFunctionMapping(FormatWriteFunctionMap *map,
                                                    GLenum format,
                                                    GLenum type,
                                                    ColorWriteFunction writeFunc)
{
    map->insert(FormatWriteFunctionPair(gl::FormatType(format, type), writeFunc));
}

static FormatWriteFunctionMap BuildFormatWriteFunctionMap()
{
    using namespace angle;  //  For image writing functions

    FormatWriteFunctionMap map;

    // clang-format off
    //                                    | Format               | Type                             | Color write function             |
    InsertFormatWriteFunctionMapping(&map, GL_RGBA,               GL_UNSIGNED_BYTE,                  WriteColor<R8G8B8A8, GLfloat>     );
    InsertFormatWriteFunctionMapping(&map, GL_RGBA,               GL_BYTE,                           WriteColor<R8G8B8A8S, GLfloat>    );
    InsertFormatWriteFunctionMapping(&map, GL_RGBA,               GL_UNSIGNED_SHORT_4_4_4_4,         WriteColor<R4G4B4A4, GLfloat>     );
    InsertFormatWriteFunctionMapping(&map, GL_RGBA,               GL_UNSIGNED_SHORT_5_5_5_1,         WriteColor<R5G5B5A1, GLfloat>     );
    InsertFormatWriteFunctionMapping(&map, GL_RGBA,               GL_UNSIGNED_INT_2_10_10_10_REV,    WriteColor<R10G10B10A2, GLfloat>  );
    InsertFormatWriteFunctionMapping(&map, GL_RGBA,               GL_FLOAT,                          WriteColor<R32G32B32A32F, GLfloat>);
    InsertFormatWriteFunctionMapping(&map, GL_RGBA,               GL_HALF_FLOAT,                     WriteColor<R16G16B16A16F, GLfloat>);
    InsertFormatWriteFunctionMapping(&map, GL_RGBA,               GL_HALF_FLOAT_OES,                 WriteColor<R16G16B16A16F, GLfloat>);
    InsertFormatWriteFunctionMapping(&map, GL_RGBA, GL_UNSIGNED_SHORT,
                                     WriteColor<R16G16B16A16, GLfloat>);
    InsertFormatWriteFunctionMapping(&map, GL_RGBA, GL_SHORT, WriteColor<R16G16B16A16S, GLfloat>);

    InsertFormatWriteFunctionMapping(&map, GL_RGBA_INTEGER,       GL_UNSIGNED_BYTE,                  WriteColor<R8G8B8A8, GLuint>      );
    InsertFormatWriteFunctionMapping(&map, GL_RGBA_INTEGER,       GL_BYTE,                           WriteColor<R8G8B8A8S, GLint>      );
    InsertFormatWriteFunctionMapping(&map, GL_RGBA_INTEGER,       GL_UNSIGNED_SHORT,                 WriteColor<R16G16B16A16, GLuint>  );
    InsertFormatWriteFunctionMapping(&map, GL_RGBA_INTEGER,       GL_SHORT,                          WriteColor<R16G16B16A16S, GLint>  );
    InsertFormatWriteFunctionMapping(&map, GL_RGBA_INTEGER,       GL_UNSIGNED_INT,                   WriteColor<R32G32B32A32, GLuint>  );
    InsertFormatWriteFunctionMapping(&map, GL_RGBA_INTEGER,       GL_INT,                            WriteColor<R32G32B32A32S, GLint>  );
    InsertFormatWriteFunctionMapping(&map, GL_RGBA_INTEGER,       GL_UNSIGNED_INT_2_10_10_10_REV,    WriteColor<R10G10B10A2, GLuint>   );

    InsertFormatWriteFunctionMapping(&map, GL_RGB,                GL_UNSIGNED_BYTE,                  WriteColor<R8G8B8, GLfloat>       );
    InsertFormatWriteFunctionMapping(&map, GL_RGB,                GL_BYTE,                           WriteColor<R8G8B8S, GLfloat>      );
    InsertFormatWriteFunctionMapping(&map, GL_RGB,                GL_UNSIGNED_SHORT_5_6_5,           WriteColor<R5G6B5, GLfloat>       );
    InsertFormatWriteFunctionMapping(&map, GL_RGB,                GL_UNSIGNED_INT_10F_11F_11F_REV,   WriteColor<R11G11B10F, GLfloat>   );
    InsertFormatWriteFunctionMapping(&map, GL_RGB,                GL_UNSIGNED_INT_5_9_9_9_REV,       WriteColor<R9G9B9E5, GLfloat>     );
    InsertFormatWriteFunctionMapping(&map, GL_RGB,                GL_FLOAT,                          WriteColor<R32G32B32F, GLfloat>   );
    InsertFormatWriteFunctionMapping(&map, GL_RGB,                GL_HALF_FLOAT,                     WriteColor<R16G16B16F, GLfloat>   );
    InsertFormatWriteFunctionMapping(&map, GL_RGB,                GL_HALF_FLOAT_OES,                 WriteColor<R16G16B16F, GLfloat>   );
    InsertFormatWriteFunctionMapping(&map, GL_RGB, GL_UNSIGNED_SHORT,
                                     WriteColor<R16G16B16, GLfloat>);
    InsertFormatWriteFunctionMapping(&map, GL_RGB, GL_SHORT, WriteColor<R16G16B16S, GLfloat>);

    InsertFormatWriteFunctionMapping(&map, GL_RGB_INTEGER,        GL_UNSIGNED_BYTE,                  WriteColor<R8G8B8, GLuint>        );
    InsertFormatWriteFunctionMapping(&map, GL_RGB_INTEGER,        GL_BYTE,                           WriteColor<R8G8B8S, GLint>        );
    InsertFormatWriteFunctionMapping(&map, GL_RGB_INTEGER,        GL_UNSIGNED_SHORT,                 WriteColor<R16G16B16, GLuint>     );
    InsertFormatWriteFunctionMapping(&map, GL_RGB_INTEGER,        GL_SHORT,                          WriteColor<R16G16B16S, GLint>     );
    InsertFormatWriteFunctionMapping(&map, GL_RGB_INTEGER,        GL_UNSIGNED_INT,                   WriteColor<R32G32B32, GLuint>     );
    InsertFormatWriteFunctionMapping(&map, GL_RGB_INTEGER,        GL_INT,                            WriteColor<R32G32B32S, GLint>     );

    InsertFormatWriteFunctionMapping(&map, GL_RG,                 GL_UNSIGNED_BYTE,                  WriteColor<R8G8, GLfloat>         );
    InsertFormatWriteFunctionMapping(&map, GL_RG,                 GL_BYTE,                           WriteColor<R8G8S, GLfloat>        );
    InsertFormatWriteFunctionMapping(&map, GL_RG,                 GL_FLOAT,                          WriteColor<R32G32F, GLfloat>      );
    InsertFormatWriteFunctionMapping(&map, GL_RG,                 GL_HALF_FLOAT,                     WriteColor<R16G16F, GLfloat>      );
    InsertFormatWriteFunctionMapping(&map, GL_RG,                 GL_HALF_FLOAT_OES,                 WriteColor<R16G16F, GLfloat>      );
    InsertFormatWriteFunctionMapping(&map, GL_RG, GL_UNSIGNED_SHORT, WriteColor<R16G16, GLfloat>);
    InsertFormatWriteFunctionMapping(&map, GL_RG, GL_SHORT, WriteColor<R16G16S, GLfloat>);

    InsertFormatWriteFunctionMapping(&map, GL_RG_INTEGER,         GL_UNSIGNED_BYTE,                  WriteColor<R8G8, GLuint>          );
    InsertFormatWriteFunctionMapping(&map, GL_RG_INTEGER,         GL_BYTE,                           WriteColor<R8G8S, GLint>          );
    InsertFormatWriteFunctionMapping(&map, GL_RG_INTEGER,         GL_UNSIGNED_SHORT,                 WriteColor<R16G16, GLuint>        );
    InsertFormatWriteFunctionMapping(&map, GL_RG_INTEGER,         GL_SHORT,                          WriteColor<R16G16S, GLint>        );
    InsertFormatWriteFunctionMapping(&map, GL_RG_INTEGER,         GL_UNSIGNED_INT,                   WriteColor<R32G32, GLuint>        );
    InsertFormatWriteFunctionMapping(&map, GL_RG_INTEGER,         GL_INT,                            WriteColor<R32G32S, GLint>        );

    InsertFormatWriteFunctionMapping(&map, GL_RED,                GL_UNSIGNED_BYTE,                  WriteColor<R8, GLfloat>           );
    InsertFormatWriteFunctionMapping(&map, GL_RED,                GL_BYTE,                           WriteColor<R8S, GLfloat>          );
    InsertFormatWriteFunctionMapping(&map, GL_RED,                GL_FLOAT,                          WriteColor<R32F, GLfloat>         );
    InsertFormatWriteFunctionMapping(&map, GL_RED,                GL_HALF_FLOAT,                     WriteColor<R16F, GLfloat>         );
    InsertFormatWriteFunctionMapping(&map, GL_RED,                GL_HALF_FLOAT_OES,                 WriteColor<R16F, GLfloat>         );
    InsertFormatWriteFunctionMapping(&map, GL_RED, GL_UNSIGNED_SHORT, WriteColor<R16, GLfloat>);
    InsertFormatWriteFunctionMapping(&map, GL_RED, GL_SHORT, WriteColor<R16S, GLfloat>);

    InsertFormatWriteFunctionMapping(&map, GL_RED_INTEGER,        GL_UNSIGNED_BYTE,                  WriteColor<R8, GLuint>            );
    InsertFormatWriteFunctionMapping(&map, GL_RED_INTEGER,        GL_BYTE,                           WriteColor<R8S, GLint>            );
    InsertFormatWriteFunctionMapping(&map, GL_RED_INTEGER,        GL_UNSIGNED_SHORT,                 WriteColor<R16, GLuint>           );
    InsertFormatWriteFunctionMapping(&map, GL_RED_INTEGER,        GL_SHORT,                          WriteColor<R16S, GLint>           );
    InsertFormatWriteFunctionMapping(&map, GL_RED_INTEGER,        GL_UNSIGNED_INT,                   WriteColor<R32, GLuint>           );
    InsertFormatWriteFunctionMapping(&map, GL_RED_INTEGER,        GL_INT,                            WriteColor<R32S, GLint>           );

    InsertFormatWriteFunctionMapping(&map, GL_LUMINANCE_ALPHA,    GL_UNSIGNED_BYTE,                  WriteColor<L8A8, GLfloat>         );
    InsertFormatWriteFunctionMapping(&map, GL_LUMINANCE,          GL_UNSIGNED_BYTE,                  WriteColor<L8, GLfloat>           );
    InsertFormatWriteFunctionMapping(&map, GL_ALPHA,              GL_UNSIGNED_BYTE,                  WriteColor<A8, GLfloat>           );
    InsertFormatWriteFunctionMapping(&map, GL_LUMINANCE_ALPHA,    GL_FLOAT,                          WriteColor<L32A32F, GLfloat>      );
    InsertFormatWriteFunctionMapping(&map, GL_LUMINANCE,          GL_FLOAT,                          WriteColor<L32F, GLfloat>         );
    InsertFormatWriteFunctionMapping(&map, GL_ALPHA,              GL_FLOAT,                          WriteColor<A32F, GLfloat>         );
    InsertFormatWriteFunctionMapping(&map, GL_LUMINANCE_ALPHA,    GL_HALF_FLOAT,                     WriteColor<L16A16F, GLfloat>      );
    InsertFormatWriteFunctionMapping(&map, GL_LUMINANCE_ALPHA,    GL_HALF_FLOAT_OES,                 WriteColor<L16A16F, GLfloat>      );
    InsertFormatWriteFunctionMapping(&map, GL_LUMINANCE,          GL_HALF_FLOAT,                     WriteColor<L16F, GLfloat>         );
    InsertFormatWriteFunctionMapping(&map, GL_LUMINANCE,          GL_HALF_FLOAT_OES,                 WriteColor<L16F, GLfloat>         );
    InsertFormatWriteFunctionMapping(&map, GL_ALPHA,              GL_HALF_FLOAT,                     WriteColor<A16F, GLfloat>         );
    InsertFormatWriteFunctionMapping(&map, GL_ALPHA,              GL_HALF_FLOAT_OES,                 WriteColor<A16F, GLfloat>         );

    InsertFormatWriteFunctionMapping(&map, GL_BGRA_EXT,           GL_UNSIGNED_BYTE,                  WriteColor<B8G8R8A8, GLfloat>     );
    InsertFormatWriteFunctionMapping(&map, GL_BGRA_EXT,           GL_UNSIGNED_SHORT_4_4_4_4_REV_EXT, WriteColor<A4R4G4B4, GLfloat>     );
    InsertFormatWriteFunctionMapping(&map, GL_BGRA_EXT,           GL_UNSIGNED_SHORT_1_5_5_5_REV_EXT, WriteColor<A1R5G5B5, GLfloat>     );

    InsertFormatWriteFunctionMapping(&map, GL_SRGB_EXT,           GL_UNSIGNED_BYTE,                  WriteColor<R8G8B8, GLfloat>       );
    InsertFormatWriteFunctionMapping(&map, GL_SRGB_ALPHA_EXT,     GL_UNSIGNED_BYTE,                  WriteColor<R8G8B8A8, GLfloat>     );

    InsertFormatWriteFunctionMapping(&map, GL_COMPRESSED_RGB_S3TC_DXT1_EXT,    GL_UNSIGNED_BYTE,     nullptr                              );
    InsertFormatWriteFunctionMapping(&map, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,   GL_UNSIGNED_BYTE,     nullptr                              );
    InsertFormatWriteFunctionMapping(&map, GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE, GL_UNSIGNED_BYTE,     nullptr                              );
    InsertFormatWriteFunctionMapping(&map, GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE, GL_UNSIGNED_BYTE,     nullptr                              );

    InsertFormatWriteFunctionMapping(&map, GL_DEPTH_COMPONENT,    GL_UNSIGNED_SHORT,                 nullptr                              );
    InsertFormatWriteFunctionMapping(&map, GL_DEPTH_COMPONENT,    GL_UNSIGNED_INT,                   nullptr                              );
    InsertFormatWriteFunctionMapping(&map, GL_DEPTH_COMPONENT,    GL_FLOAT,                          nullptr                              );

    InsertFormatWriteFunctionMapping(&map, GL_STENCIL,            GL_UNSIGNED_BYTE,                  nullptr                              );

    InsertFormatWriteFunctionMapping(&map, GL_DEPTH_STENCIL,      GL_UNSIGNED_INT_24_8,              nullptr                              );
    InsertFormatWriteFunctionMapping(&map, GL_DEPTH_STENCIL,      GL_FLOAT_32_UNSIGNED_INT_24_8_REV, nullptr                              );
    // clang-format on

    return map;
}
}  // anonymous namespace

PackPixelsParams::PackPixelsParams()
    : format(GL_NONE), type(GL_NONE), outputPitch(0), packBuffer(nullptr), offset(0)
{
}

PackPixelsParams::PackPixelsParams(const gl::Rectangle &areaIn,
                                   GLenum formatIn,
                                   GLenum typeIn,
                                   GLuint outputPitchIn,
                                   const gl::PixelPackState &packIn,
                                   ptrdiff_t offsetIn)
    : area(areaIn),
      format(formatIn),
      type(typeIn),
      outputPitch(outputPitchIn),
      packBuffer(packIn.pixelBuffer.get()),
      pack(packIn.alignment, packIn.reverseRowOrder),
      offset(offsetIn)
{
}

void PackPixels(const PackPixelsParams &params,
                const angle::Format &sourceFormat,
                int inputPitchIn,
                const uint8_t *sourceIn,
                uint8_t *destWithoutOffset)
{
    uint8_t *destWithOffset = destWithoutOffset + params.offset;

    const uint8_t *source = sourceIn;
    int inputPitch        = inputPitchIn;

    if (params.pack.reverseRowOrder)
    {
        source += inputPitch * (params.area.height - 1);
        inputPitch = -inputPitch;
    }

    const auto &sourceGLInfo = gl::GetSizedInternalFormatInfo(sourceFormat.glInternalFormat);

    if (sourceGLInfo.format == params.format && sourceGLInfo.type == params.type)
    {
        // Direct copy possible
        for (int y = 0; y < params.area.height; ++y)
        {
            memcpy(destWithOffset + y * params.outputPitch, source + y * inputPitch,
                   params.area.width * sourceGLInfo.pixelBytes);
        }
        return;
    }

    ASSERT(sourceGLInfo.sized);

    gl::FormatType formatType(params.format, params.type);
    ColorCopyFunction fastCopyFunc =
        GetFastCopyFunction(sourceFormat.fastCopyFunctions, formatType);
    const auto &destFormatInfo = gl::GetInternalFormatInfo(formatType.format, formatType.type);

    if (fastCopyFunc)
    {
        // Fast copy is possible through some special function
        for (int y = 0; y < params.area.height; ++y)
        {
            for (int x = 0; x < params.area.width; ++x)
            {
                uint8_t *dest =
                    destWithOffset + y * params.outputPitch + x * destFormatInfo.pixelBytes;
                const uint8_t *src = source + y * inputPitch + x * sourceGLInfo.pixelBytes;

                fastCopyFunc(src, dest);
            }
        }
        return;
    }

    ColorWriteFunction colorWriteFunction = GetColorWriteFunction(formatType);

    // Maximum size of any Color<T> type used.
    uint8_t temp[16];
    static_assert(sizeof(temp) >= sizeof(gl::ColorF) && sizeof(temp) >= sizeof(gl::ColorUI) &&
                      sizeof(temp) >= sizeof(gl::ColorI),
                  "Unexpected size of gl::Color struct.");

    const auto &colorReadFunction = sourceFormat.colorReadFunction;

    for (int y = 0; y < params.area.height; ++y)
    {
        for (int x = 0; x < params.area.width; ++x)
        {
            uint8_t *dest      = destWithOffset + y * params.outputPitch + x * destFormatInfo.pixelBytes;
            const uint8_t *src = source + y * inputPitch + x * sourceGLInfo.pixelBytes;

            // readFunc and writeFunc will be using the same type of color, CopyTexImage
            // will not allow the copy otherwise.
            colorReadFunction(src, temp);
            colorWriteFunction(temp, dest);
        }
    }
}

ColorWriteFunction GetColorWriteFunction(const gl::FormatType &formatType)
{
    static const FormatWriteFunctionMap formatTypeMap = BuildFormatWriteFunctionMap();
    auto iter = formatTypeMap.find(formatType);
    ASSERT(iter != formatTypeMap.end());
    if (iter != formatTypeMap.end())
    {
        return iter->second;
    }
    else
    {
        return nullptr;
    }
}

ColorCopyFunction GetFastCopyFunction(const FastCopyFunctionMap &fastCopyFunctions,
                                      const gl::FormatType &formatType)
{
    return fastCopyFunctions.get(formatType);
}

bool FastCopyFunctionMap::has(const gl::FormatType &formatType) const
{
    return (get(formatType) != nullptr);
}

ColorCopyFunction FastCopyFunctionMap::get(const gl::FormatType &formatType) const
{
    for (size_t index = 0; index < mSize; ++index)
    {
        if (mData[index].format == formatType.format && mData[index].type == formatType.type)
        {
            return mData[index].func;
        }
    }

    return nullptr;
}

}  // namespace rx
