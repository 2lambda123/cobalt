# Copyright 2016 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""Starboard Linux X64 X11 gcc 4.4 platform configuration for gyp_cobalt."""

import logging
import os
import sys

# Import the shared Linux platform configuration.
sys.path.append(
    os.path.realpath(
        os.path.join(
            os.path.dirname(__file__), os.pardir, os.pardir, os.pardir,
            'shared')))
# pylint: disable=import-self,g-import-not-at-top
import gyp_configuration as shared_configuration


class PlatformConfig(shared_configuration.PlatformConfig):
  """Starboard Linux platform configuration."""

  def __init__(self, platform, asan_enabled_by_default=False):
    super(PlatformConfig, self).__init__(
        platform, asan_enabled_by_default, goma_supports_compiler=False)

  def GetVariables(self, configuration):
    variables = super(PlatformConfig, self).GetVariables(configuration)
    variables.update({'clang': 0,})
    return variables

  def GetEnvironmentVariables(self):
    env_variables = super(PlatformConfig, self).GetEnvironmentVariables()
    env_variables.update({
        'CC': 'gcc-4.4',
        'CXX': 'g++-4.4',
    })
    return env_variables


def CreatePlatformConfig():
  try:
    return PlatformConfig('linux-x64x11-gcc-4-4')
  except RuntimeError as e:
    logging.critical(e)
    return None
