# Copyright 2016 The Cobalt Authors. All Rights Reserved.
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
"""Starboard win32 shared platform configuration for gyp_cobalt."""

import logging
import os
import re
import subprocess
import sys

import config.base
import starboard.shared.win32.sdk_configuration as win_sdk_configuration
from starboard.tools.paths import STARBOARD_ROOT
from starboard.tools.testing import test_filter


def GetWindowsVersion():
  out = subprocess.check_output('ver', universal_newlines=True, shell=True)
  lines = [l for l in out.split('\n') if l]
  for l in lines:
    m = re.search(r'Version\s([0-9\.]+)', out)
    if m and m.group(1):
      major, minor, build = m.group(1).split('.')[0:3]
      return (int(major), int(minor), int(build))
  raise IOError('Could not retrieve windows version')


def _QuotePath(path):
  return '"' + path + '"'


class Win32SharedConfiguration(config.base.PlatformConfigBase):
  """Starboard Microsoft Windows platform configuration."""

  def __init__(self, platform):
    super(Win32SharedConfiguration, self).__init__(platform)
    self.sdk = win_sdk_configuration.SdkConfiguration()

  def AdditionalPlatformCompilerOptions(self):
    sdk = self.sdk
    ver = sdk.required_sdk_version
    root_path = sdk.windows_sdk_path + '/References/' + sdk.required_sdk_version

    force_include_files = [
      sdk.vs_install_dir_with_version + '/lib/x86/store/references'
                                        '/platform.winmd',
      root_path + '/Windows.Foundation.FoundationContract/3.0.0.0'
                  '/Windows.Foundation.FoundationContract.winmd',
      root_path + '/Windows.Foundation.UniversalApiContract/6.0.0.0'
                  '/Windows.Foundation.UniversalApiContract.winmd',
    ]
    if 'xb1' in self.GetName():
      # Additional files for xb1/uwp platforms.
      # Xbox One Platform Extension SDK 17095.1000.
      force_include_files = force_include_files + [
        root_path + '/Windows.Xbox.ApplicationResourcesContract/1.0.0.0'
                    '/Windows.Xbox.ApplicationResourcesContract.winmd',
        root_path + '/Windows.UI.ViewManagement'
                    '.ViewManagementViewScalingContract/1.0.0.0'
                    '/Windows.UI.ViewManagement'
                    '.ViewManagementViewScalingContract.winmd',
      ]
    force_include_files = [f.replace('/', '\\') for f in force_include_files]
    missing_files = []
    for f in force_include_files:
      if not os.path.exists(f):
        missing_files.append(f)
    if missing_files:
      logging.critical('\n***** Missing files *****: \n' + \
                       '\n'.join(missing_files) + \
                       '\nCompiling may have problems.\n')
    # /FU"path" will force include that path for every file compiled.
    compiler_options = ['/FU' + _QuotePath(f) for f in force_include_files]
    return compiler_options

  def GetVariables(self, configuration):
    sdk = self.sdk
    variables = super(Win32SharedConfiguration, self).GetVariables(configuration)
    compiler_options = ' '.join(self.AdditionalPlatformCompilerOptions())
    variables.update({
        'visual_studio_install_path': sdk.vs_install_dir_with_version,
        'windows_sdk_path': sdk.windows_sdk_path,
        'windows_sdk_version': sdk.required_sdk_version,
        'additional_platform_compiler_options': compiler_options,
        'ucrtbased_dll_path': sdk.ucrtbased_dll_path,
    })
    return variables

  def GetEnvironmentVariables(self):
    sdk = self.sdk
    cl = _QuotePath(os.path.join(sdk.vs_host_tools_path, 'cl.exe'))
    lib = _QuotePath(os.path.join(sdk.vs_host_tools_path, 'lib.exe'))
    link = _QuotePath(os.path.join(sdk.vs_host_tools_path, 'link.exe'))
    rc = _QuotePath(os.path.join(sdk.windows_sdk_host_tools, 'rc.exe'))
    env_variables = {
        'AR': lib,
        'AR_HOST': lib,
        'CC': cl,
        'CXX': cl,
        'LD': link,
        'RC': rc,
        'VS_INSTALL_DIR': sdk.vs_install_dir,
        'CC_HOST': cl,
        'CXX_HOST': cl,
        'LD_HOST': link,
        'ARFLAGS_HOST': 'rcs',
        'ARTHINFLAGS_HOST': 'rcsT',
    }
    return env_variables

  def GetBuildFormat(self):
    """Returns the desired build format."""
    # The comma means that ninja, msvs_makefile, will be chained and use the
    # same input information so that .gyp files will only have to be parsed
    # once.
    return 'ninja,msvs_makefile'

  def GetGeneratorVariables(self, configuration):
    """Returns a dict of generator variables for the given configuration."""
    _ = configuration
    generator_variables = {
        'msvs_version': 2017,
        'msvs_platform': 'x64',
        'msvs_template_prefix': 'win/',
        'msvs_deploy_dir': '',
        'qtcreator_session_name_prefix': 'cobalt',
    }
    return generator_variables

  def GetToolchain(self):
    sys.path.append(os.path.join(STARBOARD_ROOT, 'shared', 'msvc', 'uwp'))
    from msvc_toolchain import MSVCUWPToolchain  # pylint: disable=g-import-not-at-top,g-bad-import-order
    return MSVCUWPToolchain()

  def IsWin10orHigher(self):
    try:
      # Both Win10 and Win2016-Server will return 10.0+
      major, _, _ = GetWindowsVersion()
      return major >= 10
    except Exception as e:
      print 'Error while getting version for windows: ' + str(e)
    return False

  def GetTestFilters(self):
    """Gets all tests to be excluded from a unit test run.

    Returns:
      A list of initialized TestFilter objects.
    """
    if not self.IsWin10orHigher():
      logging.error('Tests can only be executed on Win10 and higher.')
      return [test_filter.DISABLE_TESTING]

    filters = super(Win32SharedConfiguration, self).GetTestFilters()
    for target, tests in self._FILTERED_TESTS.iteritems():
      filters.extend(test_filter.TestFilter(target, test) for test in tests)
    return filters

  _FILTERED_TESTS = {
      'player_filter_tests': [
          # TODO: debug these failures.
          'AudioDecoderTests/AudioDecoderTest.SingleInvalidInput/0',
          'VideoDecoderTests/VideoDecoderTest.SingleInvalidInput/0',
      ],
  }
