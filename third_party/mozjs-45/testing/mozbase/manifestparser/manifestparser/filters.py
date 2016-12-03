# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.

"""
A filter is a callable that accepts an iterable of test objects and a
dictionary of values, and returns a new iterable of test objects. It is
possible to define custom filters if the built-in ones are not enough.
"""

from collections import defaultdict, MutableSequence
import itertools
import os

from .expression import (
    parse,
    ParseError,
)


# built-in filters

def skip_if(tests, values):
    """
    Sets disabled on all tests containing the `skip-if` tag and whose condition
    is True. This filter is added by default.
    """
    tag = 'skip-if'
    for test in tests:
        if tag in test and parse(test[tag], **values):
            test.setdefault('disabled', '{}: {}'.format(tag, test[tag]))
        yield test


def run_if(tests, values):
    """
    Sets disabled on all tests containing the `run-if` tag and whose condition
    is False. This filter is added by default.
    """
    tag = 'run-if'
    for test in tests:
        if tag in test and not parse(test[tag], **values):
            test.setdefault('disabled', '{}: {}'.format(tag, test[tag]))
        yield test


def fail_if(tests, values):
    """
    Sets expected to 'fail' on all tests containing the `fail-if` tag and whose
    condition is True. This filter is added by default.
    """
    tag = 'fail-if'
    for test in tests:
        if tag in test and parse(test[tag], **values):
            test['expected'] = 'fail'
        yield test


def enabled(tests, values):
    """
    Removes all tests containing the `disabled` key. This filter can be
    added by passing `disabled=False` into `active_tests`.
    """
    for test in tests:
        if 'disabled' not in test:
            yield test


def exists(tests, values):
    """
    Removes all tests that do not exist on the file system. This filter is
    added by default, but can be removed by passing `exists=False` into
    `active_tests`.
    """
    for test in tests:
        if os.path.exists(test['path']):
            yield test


# built-in instance filters

class InstanceFilter(object):
    """
    Generally only one instance of a class filter should be applied at a time.
    Two instances of `InstanceFilter` are considered equal if they have the
    same class name. This ensures only a single instance is ever added to
    `filterlist`. This class also formats filters' __str__ method for easier
    debugging.
    """
    unique = True

    def __init__(self, *args, **kwargs):
        self.fmt_args = ', '.join(itertools.chain(
            [str(a) for a in args],
            ['{}={}'.format(k, v) for k, v in kwargs.iteritems()]))

    def __eq__(self, other):
        if self.unique:
            return self.__class__ == other.__class__
        return self.__hash__() == other.__hash__()

    def __str__(self):
        return "{}({})".format(self.__class__.__name__, self.fmt_args)


class subsuite(InstanceFilter):
    """
    If `name` is None, removes all tests that have a `subsuite` key.
    Otherwise removes all tests that do not have a subsuite matching `name`.

    It is possible to specify conditional subsuite keys using:
       subsuite = foo,condition

    where 'foo' is the subsuite name, and 'condition' is the same type of
    condition used for skip-if.  If the condition doesn't evaluate to true,
    the subsuite designation will be removed from the test.

    :param name: The name of the subsuite to run (default None)
    """
    def __init__(self, name=None):
        InstanceFilter.__init__(self, name=name)
        self.name = name

    def __call__(self, tests, values):
        # Look for conditional subsuites, and replace them with the subsuite
        # itself (if the condition is true), or nothing.
        for test in tests:
            subsuite = test.get('subsuite', '')
            if ',' in subsuite:
                try:
                    subsuite, cond = subsuite.split(',')
                except ValueError:
                    raise ParseError("subsuite condition can't contain commas")
                matched = parse(cond, **values)
                if matched:
                    test['subsuite'] = subsuite
                else:
                    test['subsuite'] = ''

            # Filter on current subsuite
            if self.name is None:
                if not test.get('subsuite'):
                    yield test
            else:
                if test.get('subsuite') == self.name:
                    yield test


class chunk_by_slice(InstanceFilter):
    """
    Basic chunking algorithm that splits tests evenly across total chunks.

    :param this_chunk: the current chunk, 1 <= this_chunk <= total_chunks
    :param total_chunks: the total number of chunks
    :param disabled: Whether to include disabled tests in the chunking
                     algorithm. If False, each chunk contains an equal number
                     of non-disabled tests. If True, each chunk contains an
                     equal number of tests (default False)
    """

    def __init__(self, this_chunk, total_chunks, disabled=False):
        assert 1 <= this_chunk <= total_chunks
        InstanceFilter.__init__(self, this_chunk, total_chunks,
                                disabled=disabled)
        self.this_chunk = this_chunk
        self.total_chunks = total_chunks
        self.disabled = disabled

    def __call__(self, tests, values):
        tests = list(tests)
        if self.disabled:
            chunk_tests = tests[:]
        else:
            chunk_tests = [t for t in tests if 'disabled' not in t]

        tests_per_chunk = float(len(chunk_tests)) / self.total_chunks
        start = int(round((self.this_chunk - 1) * tests_per_chunk))
        end = int(round(self.this_chunk * tests_per_chunk))

        if not self.disabled:
            # map start and end back onto original list of tests. Disabled
            # tests will still be included in the returned list, but each
            # chunk will contain an equal number of enabled tests.
            if self.this_chunk == 1:
                start = 0
            elif start < len(chunk_tests):
                start = tests.index(chunk_tests[start])

            if self.this_chunk == self.total_chunks:
                end = len(tests)
            elif end < len(chunk_tests):
                end = tests.index(chunk_tests[end])
        return (t for t in tests[start:end])


class chunk_by_dir(InstanceFilter):
    """
    Basic chunking algorithm that splits directories of tests evenly at a
    given depth.

    For example, a depth of 2 means all test directories two path nodes away
    from the base are gathered, then split evenly across the total number of
    chunks. The number of tests in each of the directories is not taken into
    account (so chunks will not contain an even number of tests). All test
    paths must be relative to the same root (typically the root of the source
    repository).

    :param this_chunk: the current chunk, 1 <= this_chunk <= total_chunks
    :param total_chunks: the total number of chunks
    :param depth: the minimum depth of a subdirectory before it will be
                  considered unique
    """

    def __init__(self, this_chunk, total_chunks, depth):
        InstanceFilter.__init__(self, this_chunk, total_chunks, depth)
        self.this_chunk = this_chunk
        self.total_chunks = total_chunks
        self.depth = depth

    def __call__(self, tests, values):
        tests_by_dir = defaultdict(list)
        ordered_dirs = []
        for test in tests:
            path = test['relpath']

            if path.startswith(os.sep):
                path = path[1:]

            dirs = path.split(os.sep)
            dirs = dirs[:min(self.depth, len(dirs)-1)]
            path = os.sep.join(dirs)

            # don't count directories that only have disabled tests in them,
            # but still yield disabled tests that are alongside enabled tests
            if path not in ordered_dirs and 'disabled' not in test:
                ordered_dirs.append(path)
            tests_by_dir[path].append(test)

        tests_per_chunk = float(len(ordered_dirs)) / self.total_chunks
        start = int(round((self.this_chunk - 1) * tests_per_chunk))
        end = int(round(self.this_chunk * tests_per_chunk))

        for i in range(start, end):
            for test in tests_by_dir.pop(ordered_dirs[i]):
                yield test

        # find directories that only contain disabled tests. They still need to
        # be yielded for reporting purposes. Put them all in chunk 1 for
        # simplicity.
        if self.this_chunk == 1:
            disabled_dirs = [v for k, v in tests_by_dir.iteritems()
                             if k not in ordered_dirs]
            for disabled_test in itertools.chain(*disabled_dirs):
                yield disabled_test


class chunk_by_runtime(InstanceFilter):
    """
    Chunking algorithm that attempts to group tests into chunks based on their
    average runtimes. It keeps manifests of tests together and pairs slow
    running manifests with fast ones.

    :param this_chunk: the current chunk, 1 <= this_chunk <= total_chunks
    :param total_chunks: the total number of chunks
    :param runtimes: dictionary of test runtime data, of the form
                     {<test path>: <average runtime>}
    :param default_runtime: value in seconds to assign tests that don't exist
                            in the runtimes file
    """

    def __init__(self, this_chunk, total_chunks, runtimes, default_runtime=0):
        InstanceFilter.__init__(self, this_chunk, total_chunks, runtimes,
                                default_runtime=default_runtime)
        self.this_chunk = this_chunk
        self.total_chunks = total_chunks

        # defaultdict(lambda:<int>) assigns all non-existent keys the value of
        # <int>. This means all tests we encounter that don't exist in the
        # runtimes file will be assigned `default_runtime`.
        self.runtimes = defaultdict(lambda: default_runtime)
        self.runtimes.update(runtimes)

    def __call__(self, tests, values):
        tests = list(tests)
        manifests = set(t['manifest'] for t in tests)

        def total_runtime(tests):
            return sum(self.runtimes[t['relpath']] for t in tests
                       if 'disabled' not in t)

        tests_by_manifest = []
        for manifest in manifests:
            mtests = [t for t in tests if t['manifest'] == manifest]
            tests_by_manifest.append((total_runtime(mtests), mtests))
        tests_by_manifest.sort(reverse=True)

        tests_by_chunk = [[0, []] for i in range(self.total_chunks)]
        for runtime, batch in tests_by_manifest:
            # sort first by runtime, then by number of tests in case of a tie.
            # This guarantees the chunk with the fastest runtime will always
            # get the next batch of tests.
            tests_by_chunk.sort(key=lambda x: (x[0], len(x[1])))
            tests_by_chunk[0][0] += runtime
            tests_by_chunk[0][1].extend(batch)

        return (t for t in tests_by_chunk[self.this_chunk-1][1])


class tags(InstanceFilter):
    """
    Removes tests that don't contain any of the given tags. This overrides
    InstanceFilter's __eq__ method, so multiple instances can be added.
    Multiple tag filters is equivalent to joining tags with the AND operator.

    To define a tag in a manifest, add a `tags` attribute to a test or DEFAULT
    section. Tests can have multiple tags, in which case they should be
    whitespace delimited. For example:

    [test_foobar.html]
    tags = foo bar

    :param tags: A tag or list of tags to filter tests on
    """
    unique = False

    def __init__(self, tags):
        InstanceFilter.__init__(self, tags)
        if isinstance(tags, basestring):
            tags = [tags]
        self.tags = tags

    def __call__(self, tests, values):
        for test in tests:
            if 'tags' not in test:
                continue

            test_tags = [t.strip() for t in test['tags'].split()]
            if any(t in self.tags for t in test_tags):
                yield test


class pathprefix(InstanceFilter):
    """
    Removes tests that don't start with any of the given test paths.

    :param paths: A list of test paths to filter on
    """

    def __init__(self, paths):
        InstanceFilter.__init__(self, paths)
        if isinstance(paths, basestring):
            paths = [paths]
        self.paths = paths

    def __call__(self, tests, values):
        for test in tests:
            for tp in self.paths:
                tp = os.path.normpath(tp)
                if not os.path.normpath(test['relpath']).startswith(tp):
                    continue

                # any test path that points to a single file will be run no
                # matter what, even if it's disabled
                if 'disabled' in test and os.path.normpath(test['relpath']) == tp:
                    del test['disabled']
                yield test
                break


# filter container

DEFAULT_FILTERS = (
    skip_if,
    run_if,
    fail_if,
)
"""
By default :func:`~.active_tests` will run the :func:`~.skip_if`,
:func:`~.run_if` and :func:`~.fail_if` filters.
"""


class filterlist(MutableSequence):
    """
    A MutableSequence that raises TypeError when adding a non-callable and
    ValueError if the item is already added.
    """

    def __init__(self, items=None):
        self.items = []
        if items:
            self.items = list(items)

    def _validate(self, item):
        if not callable(item):
            raise TypeError("Filters must be callable!")
        if item in self:
            raise ValueError("Filter {} is already applied!".format(item))

    def __getitem__(self, key):
        return self.items[key]

    def __setitem__(self, key, value):
        self._validate(value)
        self.items[key] = value

    def __delitem__(self, key):
        del self.items[key]

    def __len__(self):
        return len(self.items)

    def insert(self, index, value):
        self._validate(value)
        self.items.insert(index, value)
