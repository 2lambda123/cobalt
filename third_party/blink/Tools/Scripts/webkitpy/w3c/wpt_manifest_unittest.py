# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest

from webkitpy.common.host_mock import MockHost
from webkitpy.w3c.wpt_manifest import WPTManifest


class WPTManifestUnitTest(unittest.TestCase):

    def test_ensure_manifest_copies_new_manifest(self):
        host = MockHost()
        manifest_path = '/mock-checkout/third_party/WebKit/LayoutTests/external/wpt/MANIFEST.json'

        self.assertFalse(host.filesystem.exists(manifest_path))
        WPTManifest.ensure_manifest(host)
        self.assertTrue(host.filesystem.exists(manifest_path))

        webkit_base = '/mock-checkout/third_party/WebKit'
        self.assertEqual(
            host.executive.calls,
            [
                [
                    'python',
                    webkit_base + '/Tools/Scripts/webkitpy/thirdparty/wpt/wpt/manifest',
                    '--work',
                    '--tests-root',
                    webkit_base + '/LayoutTests/external/wpt',
                ]
            ]
        )

    def test_ensure_manifest_updates_manifest_if_it_exists(self):
        host = MockHost()
        manifest_path = '/mock-checkout/third_party/WebKit/LayoutTests/external/wpt/MANIFEST.json'

        host.filesystem.write_binary_file(manifest_path, '{}')
        self.assertTrue(host.filesystem.exists(manifest_path))

        WPTManifest.ensure_manifest(host)
        self.assertTrue(host.filesystem.exists(manifest_path))

        webkit_base = '/mock-checkout/third_party/WebKit'
        self.assertEqual(
            host.executive.calls,
            [
                [
                    'python',
                    webkit_base + '/Tools/Scripts/webkitpy/thirdparty/wpt/wpt/manifest',
                    '--work',
                    '--tests-root',
                    webkit_base + '/LayoutTests/external/wpt',
                ]
            ]
        )
