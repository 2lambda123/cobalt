/*
 * Copyright 2017 Google Inc. All Rights Reserved.
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

#include "cobalt/browser/memory_settings/calculations.h"

#include <algorithm>

#include "cobalt/browser/memory_settings/build_settings.h"
#include "cobalt/browser/memory_settings/constants.h"
#include "cobalt/math/size.h"

namespace cobalt {
namespace browser {
namespace memory_settings {
namespace {

template <typename T>
T ClampValue(const T& input, const T& minimum, const T& maximum) {
  return std::max<T>(minimum, std::min<T>(maximum, input));
}

double DisplayScaleTo1080p(const math::Size& dimensions) {
  static const double kNumReferencePixels = 1920. * 1080.;
  const double num_pixels = static_cast<double>(dimensions.width()) *
                            static_cast<double>(dimensions.height());
  return num_pixels / kNumReferencePixels;
}

// LinearRemap is a type of linear interpolation which maps a value from
// one number line to a corresponding value on another number line.
// Example:
//  LinearRemap linear_remap(0, 1, 5, 10);
//  EXPECT_EQ(5.0f, linear_remap.Map(0));
//  EXPECT_EQ(7.5f, linear_remap.Map(.5));
//  EXPECT_EQ(10.f, linear_remap.Map(1));
class LinearRemap {
 public:
  LinearRemap(double amin, double amax, double bmin, double bmax)
      : amin_(amin), amax_(amax), bmin_(bmin), bmax_(bmax) {}

  // Maps the value from the number line [amin,amax] to [bmin,bmax]. For
  // example:
  //   Map(amin) -> bmin.
  //   Map(amax) -> bmax.
  //   Map((amax+amin)/2) -> (bmax+bmin)/2.
  double Map(double value) const {
    value -= amin_;
    value /= (amax_ - amin_);
    value *= (bmax_ - bmin_);
    value += bmin_;
    return value;
  }

 private:
  const double amin_, amax_, bmin_, bmax_;
};

math::Size ExpandTextureSizeToContain(const int64_t num_pixels) {
  // Iterate through to find size to contain number of pixels.
  math::Size texture_size(1, 1);
  bool toggle = false;

  // Expand the texture by powers of two until the specific number of pixels
  // is contained.
  while (texture_size.GetArea() < num_pixels) {
    if (!toggle) {
      texture_size.set_width(texture_size.width() * 2);
    } else {
      texture_size.set_height(texture_size.height() * 2);
    }
    toggle = !toggle;
  }
  return texture_size;
}

}  // namespace

int64_t CalculateImageCacheSize(const math::Size& dimensions) {
  const double display_scale = DisplayScaleTo1080p(dimensions);
  static const int64_t kReferenceSize1080p = 32 * 1024 * 1024;
  double output_bytes = kReferenceSize1080p * display_scale;

  return ClampValue<int64_t>(static_cast<int64_t>(output_bytes),
                             kMinImageCacheSize, kMaxImageCacheSize);
}

TextureDimensions CalculateSkiaGlyphAtlasTextureSize(
    const math::Size& ui_resolution) {
  // LinearRemap defines a mapping function which will map the number
  // of ui_resolution pixels to the number of texture atlas pixels such that:
  // 1080p (1920x1080) => maps to => 2048x2048 texture atlas pixels
  // 4k    (3840x2160) => maps to => 8192x4096 texture atlas pixels
  LinearRemap remap(1920 * 1080, 3840 * 2160, 2048 * 2048, 4096 * 8192);

  // Apply mapping.
  const int num_ui_pixels = ui_resolution.GetArea();
  const int64_t num_atlas_pixels =
      static_cast<int64_t>(remap.Map(num_ui_pixels));

  // Texture atlas sizes are generated in powers of two. This function will
  // produce such a texture.
  math::Size atlas_texture = ExpandTextureSizeToContain(num_atlas_pixels);

  // Clamp the atlas texture to be within a minimum range.
  math::Size clamped_atlas_texture(
      std::max<int>(kMinSkiaGlyphTextureAtlasWidth, atlas_texture.width()),
      std::max<int>(kMinSkiaGlyphTextureAtlasWidth, atlas_texture.height()));

  TextureDimensions texture_dimensions(clamped_atlas_texture.width(),
                                       clamped_atlas_texture.height(),
                                       kSkiaGlyphAtlasTextureBytesPerPixel);
  return texture_dimensions;
}

int64_t CalculateSoftwareSurfaceCacheSizeInBytes(
    const math::Size& ui_resolution) {
  // LinearRemap defines a mapping function which will map the number
  // of ui_resolution pixels to the number of surface texture cache such:
  // 720p (1280x720)   => maps to => 4MB &
  // 1080p (1920x1200) => maps to => 9MB
  LinearRemap remap(1280 * 720, 1920 * 1080, 4 * 1024 * 1024, 9 * 1024 * 1024);

  int64_t surface_cache_size_in_bytes =
      static_cast<int64_t>(remap.Map(ui_resolution.GetArea()));

  return surface_cache_size_in_bytes;
}

int64_t CalculateSkiaCacheSize(const math::Size& ui_resolution) {
  // This is normalized return 4MB @ 1080p and scales accordingly.
  LinearRemap remap(0, 1920 * 1080, 0, 4 * 1024 * 1024);
  int64_t output = static_cast<int64_t>(remap.Map(ui_resolution.GetArea()));
  return std::max<int64_t>(output, kMinSkiaCacheSize);
}

}  // namespace memory_settings
}  // namespace browser
}  // namespace cobalt
