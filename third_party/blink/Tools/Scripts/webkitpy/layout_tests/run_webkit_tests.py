# Copyright (C) 2010 Google Inc. All rights reserved.
# Copyright (C) 2010 Gabor Rapcsanyi (rgabor@inf.u-szeged.hu), University of Szeged
# Copyright (C) 2011 Apple Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import logging
import optparse
import sys
import traceback

from webkitpy.common.host import Host
from webkitpy.layout_tests.controllers.manager import Manager
from webkitpy.layout_tests.generate_results_dashboard import DashBoardGenerator
from webkitpy.layout_tests.models import test_run_results
from webkitpy.layout_tests.port.factory import configuration_options, platform_options
from webkitpy.layout_tests.views import buildbot_results
from webkitpy.layout_tests.views import printing

_log = logging.getLogger(__name__)


def main(argv, stdout, stderr):
    options, args = parse_args(argv)

    if options.platform and 'test' in options.platform and not 'browser_test' in options.platform:
        # It's a bit lame to import mocks into real code, but this allows the user
        # to run tests against the test platform interactively, which is useful for
        # debugging test failures.
        from webkitpy.common.host_mock import MockHost
        host = MockHost()
    else:
        host = Host()

    try:
        port = host.port_factory.get(options.platform, options)
    except (NotImplementedError, ValueError) as error:
        # FIXME: is this the best way to handle unsupported port names?
        print >> stderr, str(error)
        return test_run_results.UNEXPECTED_ERROR_EXIT_STATUS

    try:
        return run(port, options, args, stderr, stdout).exit_code

    # We need to still handle KeyboardInterrupt, at least for webkitpy unittest cases.
    except KeyboardInterrupt:
        return test_run_results.INTERRUPTED_EXIT_STATUS
    except test_run_results.TestRunException as error:
        print >> stderr, error.msg
        return error.code
    except BaseException as error:
        if isinstance(error, Exception):
            print >> stderr, '\n%s raised: %s' % (error.__class__.__name__, error)
            traceback.print_exc(file=stderr)
        return test_run_results.UNEXPECTED_ERROR_EXIT_STATUS


def parse_args(args):
    option_group_definitions = []

    option_group_definitions.append(
        ('Platform options', platform_options()))

    option_group_definitions.append(
        ('Configuration options', configuration_options()))

    option_group_definitions.append(
        ('Printing Options', printing.print_options()))

    option_group_definitions.append(
        ('Android-specific Options', [
            optparse.make_option(
                '--adb-device',
                action='append',
                default=[],
                dest='adb_devices',
                help='Run Android layout tests on these devices.'),
            # FIXME: Flip this to be off by default once we can log the
            # device setup more cleanly.
            optparse.make_option(
                '--no-android-logging',
                dest='android_logging',
                action='store_false',
                default=True,
                help=('Do not log android-specific debug messages (default is to log as part '
                      'of --debug-rwt-logging')),
        ]))

    option_group_definitions.append(
        ('Results Options', [
            optparse.make_option(
                '--add-platform-exceptions',
                action='store_true',
                default=False,
                help=('Save generated results into the *most-specific-platform* directory rather '
                      'than the *generic-platform* directory')),
            optparse.make_option(
                '--additional-driver-flag',
                '--additional-drt-flag',
                dest='additional_driver_flag',
                action='append',
                default=[],
                help=('Additional command line flag to pass to the driver. Specify multiple '
                      'times to add multiple flags.')),
            optparse.make_option(
                '--additional-expectations',
                action='append',
                default=[],
                help=('Path to a test_expectations file that will override previous '
                      'expectations. Specify multiple times for multiple sets of overrides.')),
            optparse.make_option(
                '--additional-platform-directory',
                action='append',
                default=[],
                help=('Additional directory where to look for test baselines (will take '
                      'precedence over platform baselines). Specify multiple times to add '
                      'multiple search path entries.')),
            optparse.make_option(
                '--build-directory',
                help=('Path to the directory under which build files are kept (should not '
                      'include configuration)')),
            optparse.make_option(
                '--clobber-old-results',
                action='store_true',
                default=False,
                help='Clobbers test results from previous runs.'),
            optparse.make_option(
                '--compare-port',
                action='store',
                default=None,
                help="Use the specified port's baselines first"),
            optparse.make_option(
                '--driver-name',
                type='string',
                help='Alternative driver binary to use'),
            optparse.make_option(
                '--full-results-html',
                action='store_true',
                default=False,
                help='Show all failures in results.html, rather than only regressions'),
            optparse.make_option(
                '--json-test-results',
                action='store',
                help='Path to write the JSON test results to.'),
            optparse.make_option(
                '--new-baseline',
                action='store_true',
                default=False,
                help=('Save generated results as new baselines into the *most-specific-platform* '
                      "directory, overwriting whatever's already there. Equivalent to "
                      '--reset-results --add-platform-exceptions')),
            optparse.make_option(
                '--new-test-results',
                action='store_true',
                default=False,
                help='Create new baselines when no expected results exist'),
            optparse.make_option(
                '--no-show-results',
                dest='show_results',
                action='store_false',
                default=True,
                help="Don't launch a browser with results after the tests are done"),
            optparse.make_option(
                '-p',
                '--pixel',
                '--pixel-tests',
                dest='pixel_tests',
                action='store_true',
                default=True,
                help='Enable pixel-to-pixel PNG comparisons (enabled by default)'),
            optparse.make_option(
                '--no-pixel',
                '--no-pixel-tests',
                dest='pixel_tests',
                action='store_false',
                default=True,
                help='Disable pixel-to-pixel PNG comparisons'),
            # FIXME: we should support a comma separated list with
            # --pixel-test-directory as well.
            optparse.make_option(
                '--pixel-test-directory',
                dest='pixel_test_directories',
                action='append',
                default=[],
                help=('A directory where it is allowed to execute tests as pixel tests. Specify '
                      'multiple times to add multiple directories. This option implies '
                      '--pixel-tests. If specified, only those tests will be executed as pixel '
                      'tests that are located in one of the' ' directories enumerated with the '
                      'option. Some ports may ignore this option while others can have a default '
                      'value that can be overridden here.')),
            optparse.make_option(
                '--reset-results',
                action='store_true',
                default=False,
                help='Reset expectations to the generated results in their existing location.'),
            optparse.make_option(
                '--results-directory',
                help='Location of test results'),
            optparse.make_option(
                '--skip-failing-tests',
                action='store_true',
                default=False,
                help=('Skip tests that are expected to fail. Note: When using this option, '
                      'you might miss new crashes in these tests.')),
            optparse.make_option(
                '--smoke',
                action='store_true',
                help='Run just the SmokeTests'),
            optparse.make_option(
                '--no-smoke',
                dest='smoke',
                action='store_false',
                help='Do not run just the SmokeTests'),
        ]))

    option_group_definitions.append(
        ('Testing Options', [
            optparse.make_option(
                '--additional-env-var',
                type='string',
                action='append',
                default=[],
                help=('Passes that environment variable to the tests '
                      '(--additional-env-var=NAME=VALUE)')),
            optparse.make_option(
                '--batch-size',
                type='int',
                default=None,
                help=('Run a the tests in batches (n), after every n tests, the driver is '
                      'relaunched.')),
            optparse.make_option(
                '--build',
                dest='build',
                action='store_true',
                default=True,
                help=('Check to ensure the build is up to date (default).')),
            optparse.make_option(
                '--no-build',
                dest='build',
                action='store_false',
                help="Don't check to see if the build is up to date."),
            optparse.make_option(
                '--child-processes',
                help='Number of drivers to run in parallel.'),
            optparse.make_option(
                '--disable-breakpad',
                action='store_true',
                help="Don't use breakpad to symbolize unexpected crashes."),
            optparse.make_option(
                '--driver-logging',
                action='store_true',
                help='Print detailed logging of the driver/content_shell'),
            optparse.make_option(
                '--enable-leak-detection',
                action='store_true',
                help='Enable the leak detection of DOM objects.'),
            optparse.make_option(
                '--enable-sanitizer',
                action='store_true',
                help='Only alert on sanitizer-related errors and crashes'),
            optparse.make_option(
                '--exit-after-n-crashes-or-timeouts',
                type='int',
                default=None,
                help='Exit after the first N crashes instead of running all tests'),
            optparse.make_option(
                '--exit-after-n-failures',
                type='int',
                default=None,
                help='Exit after the first N failures instead of running all tests'),
            optparse.make_option(
                '--ignore-builder-category',
                action='store',
                help=('The category of builders to use with the --ignore-flaky-tests option '
                      "('layout' or 'deps').")),
            optparse.make_option(
                '--ignore-flaky-tests',
                action='store',
                help=('Control whether tests that are flaky on the bots get ignored. '
                      "'very-flaky' == Ignore any tests that flaked more than once on the bot. "
                      "'maybe-flaky' == Ignore any tests that flaked once on the bot. "
                      "'unexpected' == Ignore any tests that had unexpected results on the bot.")),
            optparse.make_option(
                '--iterations',
                type='int',
                default=1,
                help='Number of times to run the set of tests (e.g. ABCABCABC)'),
            optparse.make_option(
                '--layout-tests-directory',
                help=('Path to a custom layout tests directory')),
            optparse.make_option(
                '--max-locked-shards',
                type='int',
                default=0,
                help='Set the maximum number of locked shards'),
            optparse.make_option(
                '--nocheck-sys-deps',
                action='store_true',
                default=False,
                help="Don't check the system dependencies (themes)"),
            optparse.make_option(
                '--order',
                action='store',
                default='random',
                help=('Determine the order in which the test cases will be run. '
                      "'none' == use the order in which the tests were listed "
                      'either in arguments or test list, '
                      "'random' == pseudo-random order (default). Seed can be specified "
                      'via --seed, otherwise it will default to the current unix timestamp. '
                      "'natural' == use the natural order")),
            optparse.make_option(
                '--profile',
                action='store_true',
                help='Output per-test profile information.'),
            optparse.make_option(
                '--profiler',
                action='store',
                help='Output per-test profile information, using the specified profiler.'),
            optparse.make_option(
                '--repeat-each',
                type='int',
                default=1,
                help='Number of times to run each test (e.g. AAABBBCCC)'),
            # TODO(joelo): Delete --retry-failures and --no-retry-failures as they
            # are redundant with --num-retries.
            optparse.make_option(
                '--retry-failures',
                action='store_true',
                help=('Re-try any tests that produce unexpected results. Default is to not retry '
                      'if an explicit list of tests is passed to run-webkit-tests.')),
            optparse.make_option(
                '--no-retry-failures',
                dest='retry_failures',
                action='store_false',
                help="Don't re-try any tests that produce unexpected results."),
            optparse.make_option(
                '--num-retries',
                type='int',
                default=3,
                help=('Number of times to retry failures, default is 3. Only relevant when '
                      'failure retries are enabled.')),
            optparse.make_option(
                '--total-shards',
                type=int,
                help=('Total number of shards being used for this test run. '
                      'Must be used with --shard-index. '
                      '(The user of this script is responsible for spawning '
                      'all of the shards.)')),
            optparse.make_option(
                '--shard-index',
                type=int,
                help=('Shard index [0..total_shards) of this test run. '
                      'Must be used with --total-shards.')),
            optparse.make_option(
                '--run-singly',
                action='store_true',
                default=False,
                help='DEPRECATED, same as --batch-size=1 --verbose'),
            optparse.make_option(
                '--seed',
                type='int',
                help=('Seed to use for random test order (default: %default). '
                      'Only applicable in combination with --order=random.')),
            optparse.make_option(
                '--skipped',
                action='store',
                default=None,
                help=('control how tests marked SKIP are run. '
                      "'default' == Skip tests unless explicitly listed on the command line, "
                      "'ignore' == Run them anyway, "
                      "'only' == only run the SKIP tests, "
                      "'always' == always skip, even if listed on the command line.")),
            optparse.make_option(
                '--fastest',
                action='store',
                type='float',
                help='Run the N% fastest tests as well as any tests listed on the command line'),
            optparse.make_option(
                '--test-list',
                action='append',
                metavar='FILE',
                help='read list of tests to run from file'),
            optparse.make_option(
                '--time-out-ms',
                help='Set the timeout for each test'),
            optparse.make_option(
                '--wrapper',
                help=('wrapper command to insert before invocations of the driver; option '
                      "is split on whitespace before running. (Example: --wrapper='valgrind "
                      "--smc-check=all')")),
            # FIXME: Display default number of child processes that will run.
            optparse.make_option(
                '-f', '--fully-parallel',
                action='store_true',
                help='run all tests in parallel'),
            optparse.make_option(
                '-i', '--ignore-tests',
                action='append',
                default=[],
                help='directories or test to ignore (may specify multiple times)'),
            optparse.make_option(
                '-n', '--dry-run',
                action='store_true',
                default=False,
                help='Do everything but actually run the tests or upload results.'),
        ]))

    # FIXME: Move these into json_results_generator.py.
    option_group_definitions.append(
        ('Result JSON Options', [
            optparse.make_option(
                '--build-name',
                default='DUMMY_BUILD_NAME',
                help='The name of the builder used in its path, e.g. webkit-rel.'),
            optparse.make_option(
                '--step-name',
                default='webkit_tests',
                help='The name of the step in a build running this script.'),
            optparse.make_option(
                '--build-number',
                default='DUMMY_BUILD_NUMBER',
                help='The build number of the builder running this script.'),
            optparse.make_option(
                '--builder-name',
                default='',
                help=('The name of the builder shown on the waterfall running this script '
                      'e.g. WebKit.')),
            optparse.make_option(
                '--master-name',
                help='The name of the buildbot master.'),
            optparse.make_option(
                '--test-results-server',
                default='',
                help='If specified, upload results json files to this appengine server.'),
            optparse.make_option(
                '--write-full-results-to',
                help=('If specified, copy full_results.json from the results dir to the '
                      'specified path.')),
        ]))

    option_parser = optparse.OptionParser()

    for group_name, group_options in option_group_definitions:
        option_group = optparse.OptionGroup(option_parser, group_name)
        option_group.add_options(group_options)
        option_parser.add_option_group(option_group)

    return option_parser.parse_args(args)


def _set_up_derived_options(port, options, args):
    """Sets the options values that depend on other options values."""
    if options.batch_size is None:
        options.batch_size = port.default_batch_size()

    if not options.child_processes:
        options.child_processes = port.host.environ.get('WEBKIT_TEST_CHILD_PROCESSES',
                                                        str(port.default_child_processes()))
    if not options.max_locked_shards:
        options.max_locked_shards = int(port.host.environ.get('WEBKIT_TEST_MAX_LOCKED_SHARDS',
                                                              str(port.default_max_locked_shards())))

    if not options.configuration:
        options.configuration = port.default_configuration()

    if not options.time_out_ms:
        options.time_out_ms = str(port.default_timeout_ms())

    options.slow_time_out_ms = str(5 * int(options.time_out_ms))

    if options.additional_platform_directory:
        additional_platform_directories = []
        for path in options.additional_platform_directory:
            additional_platform_directories.append(port.host.filesystem.abspath(path))
        options.additional_platform_directory = additional_platform_directories

    if options.new_baseline:
        options.reset_results = True
        options.add_platform_exceptions = True

    if options.pixel_test_directories:
        options.pixel_tests = True
        verified_dirs = set()
        pixel_test_directories = options.pixel_test_directories
        for directory in pixel_test_directories:
            # FIXME: we should support specifying the directories all the ways we support it for additional
            # arguments specifying which tests and directories to run. We should also move the logic for that
            # to Port.
            filesystem = port.host.filesystem
            if not filesystem.isdir(filesystem.join(port.layout_tests_dir(), directory)):
                _log.warning("'%s' was passed to --pixel-test-directories, which doesn't seem to be a directory", str(directory))
            else:
                verified_dirs.add(directory)

        options.pixel_test_directories = list(verified_dirs)

    if options.run_singly:
        options.batch_size = 1
        options.verbose = True

    if not args and not options.test_list and options.smoke is None:
        options.smoke = port.default_smoke_test_only()
    if options.smoke:
        if not args and not options.test_list and options.retry_failures is None:
            # Retry failures by default if we're doing just a smoke test (no additional tests).
            options.retry_failures = True

        if not options.test_list:
            options.test_list = []
        options.test_list.append(port.host.filesystem.join(port.layout_tests_dir(), 'SmokeTests'))
        if not options.skipped:
            options.skipped = 'always'

    if not options.skipped:
        options.skipped = 'default'

    if not options.total_shards and 'GTEST_TOTAL_SHARDS' in port.host.environ:
        options.total_shards = int(port.host.environ['GTEST_TOTAL_SHARDS'])
    if not options.shard_index and 'GTEST_SHARD_INDEX' in port.host.environ:
        options.shard_index = int(port.host.environ['GTEST_SHARD_INDEX'])

    if not options.seed:
        options.seed = port.host.time()


def _run_tests(port, options, args, printer):
    _set_up_derived_options(port, options, args)
    manager = Manager(port, options, printer)
    printer.print_config(port.results_directory())
    return manager.run(args)


def run(port, options, args, logging_stream, stdout):
    logger = logging.getLogger()
    logger.setLevel(logging.DEBUG if options.debug_rwt_logging else logging.INFO)

    printer = printing.Printer(port, options, logging_stream, logger=logger)
    try:
        run_details = _run_tests(port, options, args, printer)
        printer.flush()

        if (not options.dry_run and
                (run_details.exit_code not in test_run_results.ERROR_CODES or
                 run_details.exit_code == test_run_results.EARLY_EXIT_STATUS) and
                not run_details.initial_results.keyboard_interrupted):
            bot_printer = buildbot_results.BuildBotPrinter(stdout, options.debug_rwt_logging)
            bot_printer.print_results(run_details)
            stdout.flush()

            _log.debug('Generating dashboard...')
            gen_dash_board = DashBoardGenerator(port)
            gen_dash_board.generate()
            _log.debug('Dashboard generated.')

        _log.debug('')
        _log.debug('Testing completed, Exit status: %d', run_details.exit_code)

        # Temporary process dump for debugging windows timeout issues, see crbug.com/522396.
        _log.debug('')
        _log.debug('Process dump:')
        for process in port.host.executive.process_dump():
            _log.debug('\t%s', process)

        return run_details

    finally:
        printer.cleanup()

if __name__ == '__main__':
    exit_code = main(sys.argv[1:], sys.stdout, sys.stderr)
    sys.exit(exit_code)
