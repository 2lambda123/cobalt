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

#include "starboard/system.h"

#include <android/native_activity.h>

#include "starboard/android/shared/application_android.h"

using starboard::android::shared::ApplicationAndroid;

void SbSystemRequestStop(int error_level) {
  // Note this implementation is intended to suffice for
  // reporting and stopping an automated test run.
  SB_LOG(ERROR) << "***Stopping Application*** " << error_level;

  ApplicationAndroid::Get()->SetExitOnActivityDestroy();

  ANativeActivity_finish(ApplicationAndroid::Get()->GetActivity());
}
