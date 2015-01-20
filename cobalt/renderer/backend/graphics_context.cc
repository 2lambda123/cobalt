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

#include "cobalt/renderer/backend/graphics_context.h"

namespace cobalt {
namespace renderer {
namespace backend {

scoped_ptr<Texture> GraphicsContext::CreateTextureFromCopy(
    const SurfaceInfo& surface_info, int pitch_in_bytes,
    const uint8_t* pixel_data) {
  DCHECK(pixel_data);

  // Default implementation is to forward to CreateImage().
  scoped_ptr<TextureData> texture_source_data =
      AllocateTextureData(surface_info);

  uint8_t* image_source_memory = texture_source_data->GetMemory();
  int image_source_pitch_in_bytes = texture_source_data->GetPitchInBytes();
  int bytes_per_pixel = SurfaceInfo::BytesPerPixel(surface_info.format);

  DCHECK_LE(bytes_per_pixel * surface_info.width, image_source_pitch_in_bytes);
  DCHECK_LE(bytes_per_pixel * surface_info.width, pitch_in_bytes);

  // Copy the data specified by the user into the image source data memory.
  if (pitch_in_bytes == image_source_pitch_in_bytes) {
    // If the pitches are equal, we can do the entire copy in one memcpy().
    memcpy(image_source_memory, pixel_data,
           surface_info.height * pitch_in_bytes);
  } else {
    // If the pitches are not equal, we must memcpy each row seperately.
    for (int i = 0; i < surface_info.height; ++i) {
      memcpy(image_source_memory + i * image_source_pitch_in_bytes,
             pixel_data + i * pitch_in_bytes,
             surface_info.width * bytes_per_pixel);
    }
  }

  // Finally wrap our filled out TextureData object in a Image and
  // return the result.
  return CreateTexture(texture_source_data.Pass());
}

}  // namespace backend
}  // namespace renderer
}  // namespace cobalt
