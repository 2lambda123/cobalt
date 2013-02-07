#!/usr/bin/env python
#
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Process Android library resources to generate R.java and crunched images."""

import optparse
import os
import subprocess


BUILD_ANDROID_DIR = os.path.dirname(__file__)


def ParseArgs():
  """Parses command line options.

  Returns:
    An options object as from optparse.OptionsParser.parse_args()
  """
  parser = optparse.OptionParser()
  parser.add_option('--android-sdk', help='path to the Android SDK folder')
  parser.add_option('--android-sdk-tools',
                    help='path to the Android SDK platform tools folder')
  parser.add_option('--R-package', help='Java package for generated R.java')
  parser.add_option('--R-dir', help='directory to hold generated R.java')
  parser.add_option('--res-dir', help='directory containing resources')
  parser.add_option('--crunched-res-dir',
                    help='directory to hold crunched resources')
  (options, args) = parser.parse_args()

  if args:
    parser.error('No positional arguments should be given.')

  # Check that required options have been provided.
  required_options = ('android_sdk', 'android_sdk_tools', 'R_package',
                      'R_dir', 'res_dir', 'crunched_res_dir')
  for option_name in required_options:
    if getattr(options, option_name) is None:
      parser.error('--%s is required' % option_name.replace('_', '-'))

  return options


def main():
  options = ParseArgs()
  android_jar = os.path.join(options.android_sdk, 'android.jar')
  aapt = os.path.join(options.android_sdk_tools, 'aapt')
  dummy_manifest = os.path.join(BUILD_ANDROID_DIR, 'AndroidManifest.xml')

  # Generate R.java. This R.java contains non-final constants and is used only
  # while compiling the library jar (e.g. chromium_content.jar). When building
  # an apk, a new R.java file with the correct resource -> ID mappings will be
  # generated by merging the resources from all libraries and the main apk
  # project.
  subprocess.check_call([aapt,
                         'package',
                         '-m',
                         '--non-constant-id',
                         '--custom-package', options.R_package,
                         '-M', dummy_manifest,
                         '-S', options.res_dir,
                         '-I', android_jar,
                         '-J', options.R_dir])

  # Crunch image resources. This shrinks png files and is necessary for 9-patch
  # images to display correctly.
  subprocess.check_call([aapt,
                         'crunch',
                         '-S', options.res_dir,
                         '-C', options.crunched_res_dir])


if __name__ == '__main__':
  main()
