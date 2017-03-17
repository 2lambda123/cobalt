// Copyright 2017 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef STARBOARD_ANDROID_SHARED_DRM_SYSTEM_H_
#define STARBOARD_ANDROID_SHARED_DRM_SYSTEM_H_

#include "starboard/shared/starboard/drm/drm_system_internal.h"

#include <jni.h>

#include "starboard/log.h"

namespace starboard {
namespace android {
namespace shared {

class DrmSystem : public ::SbDrmSystemPrivate {
 public:
  DrmSystem(void* context,
            SbDrmSessionUpdateRequestFunc update_request_callback,
            SbDrmSessionUpdatedFunc session_updated_callback);

  ~DrmSystem() SB_OVERRIDE;
  void GenerateSessionUpdateRequest(const char* type,
                                    const void* initialization_data,
                                    int initialization_data_size) SB_OVERRIDE;
  void UpdateSession(const void* key,
                     int key_size,
                     const void* session_id,
                     int session_id_size);
  void CloseSession(const void* session_id, int session_id_size) SB_OVERRIDE;
  DecryptStatus Decrypt(InputBuffer* buffer) SB_OVERRIDE;

  bool is_valid() const { return false; }

  jobject GetMediaCrypto() const {
    SB_NOTIMPLEMENTED();
    return NULL;
  }

 private:
  void* context_;
  SbDrmSessionUpdateRequestFunc update_request_callback_;
  SbDrmSessionUpdatedFunc session_updated_callback_;
};

}  // namespace shared
}  // namespace android
}  // namespace starboard

#endif  // STARBOARD_ANDROID_SHARED_DRM_SYSTEM_H_
