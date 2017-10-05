# Copyright 2017 Google Inc. All Rights Reserved.
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
"""Starboard mock platform configuration for gyp_cobalt."""

import logging

import config.starboard
import gyp_utils


def CreatePlatformConfig():
  try:
    return PlatformConfig('linux-x64x11-mock')
  except RuntimeError as e:
    logging.critical(e)
    return None


class PlatformConfig(config.starboard.PlatformConfigStarboard):
  """Starboard mock platform configuration."""

  def __init__(self, platform):
    super(PlatformConfig, self).__init__(platform)
    goma_supports_compiler = True
    self.host_compiler_environment = gyp_utils.GetHostCompilerEnvironment(
        goma_supports_compiler)

  def GetBuildFormat(self):
    return 'ninja,qtcreator_ninja'

  def GetVariables(self, configuration):
    return super(PlatformConfig, self).GetVariables(configuration, use_clang=1)

  def GetGeneratorVariables(self, configuration):
    _ = configuration
    generator_variables = {
        'qtcreator_session_name_prefix': 'cobalt',
    }
    return generator_variables

  def GetEnvironmentVariables(self):
    env_variables = self.host_compiler_environment
    env_variables.update({
        'CC': self.host_compiler_environment['CC_host'],
        'CXX': self.host_compiler_environment['CXX_host'],
    })
    return env_variables
