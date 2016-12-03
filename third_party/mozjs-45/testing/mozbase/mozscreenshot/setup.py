# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.

from setuptools import setup


PACKAGE_NAME = 'mozscreenshot'
PACKAGE_VERSION = '0.1'


setup(
    name=PACKAGE_NAME,
    version=PACKAGE_VERSION,
    description="Library for taking screenshots in tests harness",
    long_description="see http://mozbase.readthedocs.org/",
    classifiers=[], # Get strings from http://pypi.python.org/pypi?%3Aaction=list_classifiers
    keywords='mozilla',
    author='Mozilla Automation and Tools team',
    author_email='tools@lists.mozilla.org',
    url='https://wiki.mozilla.org/Auto-tools/Projects/Mozbase',
    license='MPL',
    packages=['mozscreenshot'],
    zip_safe=False,
    install_requires=['mozlog', 'mozinfo'],
)
