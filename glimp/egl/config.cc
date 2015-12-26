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

#include "glimp/egl/config.h"

#include <algorithm>

#include "starboard/log.h"

namespace glimp {
namespace egl {

namespace {
bool AttributeKeyAndValueAreValid(int key, int value) {
  switch (key) {
    // First deal with the trivial keys where all values are valid.
    case EGL_RED_SIZE:
    case EGL_GREEN_SIZE:
    case EGL_BLUE_SIZE:
    case EGL_ALPHA_SIZE:
    case EGL_SURFACE_TYPE: {
      return true;
    }

    // Deal with boolean values.
    case EGL_BIND_TO_TEXTURE_RGBA: {
      switch (value) {
        case EGL_DONT_CARE:
        case EGL_TRUE:
        case EGL_FALSE: {
          return true;
        }
      }
      return false;
    }
  }

  // If the switch statement didn't catch the key, this is an unknown
  // key values.
  // TODO(***REMOVED***): glimp doesn't support all values yet, and will return
  //               false for keys that it doesn't support.
  return false;
}
}  // namespace

bool ValidateConfigAttribList(const EGLint* raw_attribs,
                              ValidatedConfigAttribs* out_validated_attribs) {
  const int* current_attrib = raw_attribs;
  while (*current_attrib != EGL_NONE) {
    int key = *current_attrib++;
    int value = *current_attrib++;

    if (!AttributeKeyAndValueAreValid(key, value)) {
      return false;
    }

    // Now that the key is validated, add the key/value pair to the map.
    (*out_validated_attribs)[key] = value;
  }

  return true;
}

namespace {
// Returns whether or not a single attribute (the parameters |key| and |value|)
// matches a config or not.
bool ConfigMatchesAttribute(const Config& config, int key, int value) {
  SB_DCHECK(AttributeKeyAndValueAreValid(key, value));
  SB_DCHECK(value != EGL_DONT_CARE);

  switch (key) {
    case EGL_RED_SIZE:
    case EGL_GREEN_SIZE:
    case EGL_BLUE_SIZE:
    case EGL_ALPHA_SIZE: {
      // We match if our config's bit depth is greater than or equal to the
      // requested value.
      return config.find(key)->second >= value;
    }

    case EGL_SURFACE_TYPE: {
      // We match if our config's surface type bit mask includes the requested
      // bit mask.
      return (config.find(key)->second & value) == value;
    }

    case EGL_BIND_TO_TEXTURE_RGBA: {
      // Our config matches booleans if the requested boolean is not true, or
      // else if the config's corresponding boolean is also true.
      return value != EGL_TRUE || config.find(key)->second == EGL_TRUE;
    }
  }

  // The attributes should have been validated when this function is called,
  // so if we reach this point, then there is an inconsistency between
  // this function and AttributeKeyAndValueAreValid().
  SB_NOTREACHED();
  return false;
}

bool ConfigMatchesAttributes(const Config& config,
                             const ValidatedConfigAttribs& attrib_list) {
  for (ValidatedConfigAttribs::const_iterator iter = attrib_list.begin();
       iter != attrib_list.end(); ++iter) {
    if (iter->second != EGL_DONT_CARE) {
      if (!ConfigMatchesAttribute(config, iter->first, iter->second)) {
        return false;
      }
    }
  }
  return true;
}
}  // namespace

std::vector<Config*> FilterConfigs(const std::set<Config*>& configs,
                                   const ValidatedConfigAttribs& attrib_list) {
  std::vector<Config*> ret;

  for (std::set<Config*>::const_iterator iter = configs.begin();
       iter != configs.end(); ++iter) {
    if (ConfigMatchesAttributes(**iter, attrib_list)) {
      ret.push_back(*iter);
    }
  }

  return ret;
}

namespace {

class ConfigSorter {
 public:
  explicit ConfigSorter(const ValidatedConfigAttribs& attrib_list)
      : attrib_list_(attrib_list) {}

  // We define this such that it sorts in decreasing order of preference.
  bool operator()(const Config* lhs, const Config* rhs) const {
    // Bit depth must be sorted in ascending order as a total over all
    // channels that are specified by the config.
    if (GetTotalBitDepth(*lhs) > GetTotalBitDepth(*rhs)) {
      return true;
    }
    return false;
  }

 private:
  // Returns the bit depth for a given channel, or 0 if we don't care about
  // it's value (e.g. it is not in the specified attribute list).
  int GetTotalBitDepthForChannel(const Config& config, int key) const {
    ValidatedConfigAttribs::const_iterator found = attrib_list_.find(key);
    if (found == attrib_list_.end() || found->second == EGL_DONT_CARE) {
      return 0;
    } else {
      return found->second;
    }
  }

  // Gets the total depth for all color channels, to be used to decide the
  // sort order.
  int GetTotalBitDepth(const Config& config) const {
    int total_bit_depth = 0;
    total_bit_depth += GetTotalBitDepthForChannel(config, EGL_RED_SIZE);
    total_bit_depth += GetTotalBitDepthForChannel(config, EGL_GREEN_SIZE);
    total_bit_depth += GetTotalBitDepthForChannel(config, EGL_BLUE_SIZE);
    total_bit_depth += GetTotalBitDepthForChannel(config, EGL_ALPHA_SIZE);
    return total_bit_depth;
  }

  const ValidatedConfigAttribs& attrib_list_;
};
}  // namespace

void SortConfigs(const ValidatedConfigAttribs& attrib_list,
                 std::vector<Config*>* in_out_configs) {
  ConfigSorter config_sorter(attrib_list);
  std::sort(in_out_configs->begin(), in_out_configs->end(), config_sorter);
}

EGLConfig ToEGLConfig(Config* config) {
  return reinterpret_cast<EGLConfig>(config);
}

}  // namespace egl
}  // namespace glimp
