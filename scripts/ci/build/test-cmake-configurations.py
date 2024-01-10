#!/usr/bin/env python3

#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import click
from pathlib import Path
import shutil
import time
import subprocess
import re

import build


def test_cmake_configuration(cli_context, build_dir, expect_success, **cmake_options):
    print(f"Running test :{cmake_options}")
    test_dir = (Path(build_dir) / "cmake-configuration-tests-build").absolute()
    if test_dir.is_dir():
        shutil.rmtree(test_dir)
    Path(test_dir).mkdir(parents=True, exist_ok=True)

    test_failure_msg = None
    process_result = cli_context.invoke(build.build, build_dir=test_dir, configure_only=True, disable_default_window_type=True, **cmake_options)
    if process_result.returncode == 0:
        if not expect_success:
            test_failure_msg = "Test failed! Death test was supposed to fail cmake configure, but it succeeded!"
    elif expect_success:
        test_failure_msg = "Test failed! Configuration did not succeed!"

    if test_failure_msg:
        raise Exception(test_failure_msg)

    # if configure was expected to fail dont check output of configure
    if not expect_success:
        return

    # check if output from configure had the expected modules, and didnt have the unexpected modules
    process_output = process_result.stdout.decode('utf-8')

    def check_expectations(condition, *expectations):
        # generated cmake modules have the pattern: "+ module-name (TARGET_TYPE)", so the asserts downbelow
        # check if each of the strings passed in `expectations` exists (or does not exist) preceeded
        # by a `+` and followed by a space
        if condition:
            assert all(f'+ {e} ' in process_output for e in expectations)
        else:
            assert all(f'+ {e} ' not in process_output for e in expectations)

    # check expected and non-expected output based on logical consequences of each option, and their combination together
    x11 = "enable_x11" in cmake_options
    wayland_ivi = "enable_wayland_ivi" in cmake_options
    wayland_shell = "enable_wayland_wl_shell" in cmake_options
    tests = "no_tests" not in cmake_options
    examples = "no_examples" not in cmake_options
    tools = "no_tools" not in cmake_options
    logic = "disable_logic" not in cmake_options
    use_imagemagick = "use_imagemagick" in cmake_options
    luajit = "enable_luajit" in cmake_options

    # renderer is enabled iff any window type is enabled
    renderer = x11 or wayland_ivi or wayland_shell
    # full shared lib is enabled iff cmake to disable full shared lib not set AND renderer enabled (any window type enabled)
    full_shared_lib = renderer and "no_full_shared_lib" not in cmake_options

    # always existing
    check_expectations(True, "ramses-client")

    # expect internal lua target unless luajit is configured
    check_expectations(logic and not luajit, "lua (internal)")
    if luajit:
        assert "Found LuaJIT" in process_output

    # shared libs
    check_expectations(True, "ramses-shared-lib-headless")
    check_expectations(full_shared_lib, "ramses-shared-lib")

    # renderer
    check_expectations(x11, "X11")
    check_expectations(wayland_ivi, "Wayland ivi")
    check_expectations(wayland_shell, "Wayland wl_shell")
    check_expectations(renderer, "ramses-renderer-internal", "ramses-renderer")

    # tools
    check_expectations(tools, "ivi-gears", "ivi-simple-dmabuf-egl", "ramses-daemon")
    check_expectations(tools and renderer, "ramses-renderer-standalone", "ramses-stream-viewer", "ramses-imgui")
    check_expectations(tools and logic, "ramses-viewer-headless", "test-asset-producer")
    check_expectations(tools and logic and renderer, "ramses-viewer")

    # tests
    check_expectations(tests, "ramses-framework-test", "ramses-client-test")
    check_expectations(tests and renderer, "ramses-renderer-internal-test", "ramses-renderer-test", "rendering-tests", "ramses-test-client")
    check_expectations(tests and logic, "ramses-logic-benchmarks")
    check_expectations(x11 and tests, "window-x11-test")
    check_expectations(wayland_ivi and tests, "window-wayland-ivi-test")
    check_expectations(wayland_shell and tests, "window-wayland-wl-shell-test")

    # tool tests
    check_expectations(tests and tools and logic, "ramses-viewer-test")
    check_expectations(tests and tools and logic and renderer and use_imagemagick, "ramses-viewer-gui-test")

    # examples
    check_expectations(examples, "ramses-example-basic-geometry")
    check_expectations(examples and full_shared_lib, "ramses-example-local-client")
    check_expectations(examples and logic, "00_minimal")
    check_expectations(examples and full_shared_lib and logic, "13_render_order")

    if tests:
        ctestProcessResult = subprocess.run(["ctest", "--show-only"], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, cwd=test_dir)
        ctestStdOutput = ctestProcessResult.stdout.decode('utf-8')

        # step 1: get all tests from ctest output
        # Example of test entry from ctest output: Test #13: ramses-client-test_UNITTEST
        ctest_test_entry_regex = r"Test(\s+)#(\d+):(\s+)((\w|\-)+)"
        ctest_test_entries = [e.group(4) for e in re.finditer(ctest_test_entry_regex, ctestStdOutput)]

        # step 2: get test count from ctest output and assert step 1 was correct
        # Example of test count from ctest: Total Tests: 3
        ctest_test_count_regex = r"Total(\s+)Tests:(\s+)(\d+)$"
        ctest_test_count = int(re.search(ctest_test_count_regex, ctestStdOutput).group(3))

        assert ctest_test_count == len(ctest_test_entries)

        # step 3: build list of expected tests based on cmake options
        expected_tests = ['ramses-framework-test_UNITTEST', 'ramses-client-test_UNITTEST']

        if full_shared_lib:
            expected_tests += ['ramses-shared-lib-tests_RNDSANDWICHTEST_SWRAST']

        if examples and logic:
            expected_tests += [
                '00_minimal_UNITTEST',
                '01a_primitive_properties_UNITTEST',
                '01b_struct_properties_UNITTEST',
                '01c_array_properties_UNITTEST',
                '02_errors_compile_time_UNITTEST',
                '03_errors_runtime_UNITTEST',
                '05_serialization_UNITTEST',
                '07_links_UNITTEST',
                '09_modules_UNITTEST',
                '10_globals_UNITTEST',
                '11_interfaces_UNITTEST']

        if renderer:
            expected_tests += [
                'ramses-renderer-internal-test_UNITTEST',
                'ramses-renderer-test_UNITTEST',
                'ramses-cli-test_UNITTEST']

        if x11:
            expected_tests += [
                'rendering-tests_x11-gles30_RNDSANDWICHTEST_SWRAST',
                'renderer-lifecycle-tests_x11-gles30_RNDSANDWICHTEST_SWRAST',
                'render-backend-tests_x11-gles30_RNDSANDWICHTEST_SWRAST',
                'window-x11-test_RNDSANDWICHTEST_SWRAST',

                'rendering-tests_x11-gles30_RNDSANDWICHTEST',
                'render-backend-tests_x11-gles30_RNDSANDWICHTEST',
                'resource-stress-tests_x11-gles30_RNDSANDWICHTEST']

        if wayland_ivi:
            expected_tests += [
                'rendering-tests_wayland-ivi-gles30_RNDSANDWICHTEST_SWRAST',
                'renderer-lifecycle-tests_wayland-ivi-gles30_RNDSANDWICHTEST_SWRAST',
                'render-backend-tests_wayland-ivi-gles30_RNDSANDWICHTEST_SWRAST',
                'embedded-compositing-rendering-tests_RNDSANDWICHTEST_SWRAST',
                'system-compositor-controller-wayland-test_RNDSANDWICHTEST_SWRAST',
                'embedded-compositor-wayland-test_RNDSANDWICHTEST_SWRAST',
                'window-wayland-ivi-test_RNDSANDWICHTEST_SWRAST',

                'rendering-tests_wayland-ivi-gles30_RNDSANDWICHTEST_VALGRINDGATE',
                'renderer-lifecycle-tests_wayland-ivi-gles30_RNDSANDWICHTEST_VALGRINDGATE',
                'embedded-compositing-rendering-tests_wayland-ivi-gles30_RNDSANDWICHTEST_VALGRINDGATE',

                'rendering-tests_wayland-ivi-gles30_RNDSANDWICHTEST',
                'render-backend-tests_wayland-ivi-gles30_RNDSANDWICHTEST',
                'resource-stress-tests_wayland-ivi-gles30_RNDSANDWICHTEST']

        if wayland_shell:
            expected_tests += [
                'renderer-lifecycle-tests_wayland-wl-shell-gles30_RNDSANDWICHTEST_SWRAST',
                'embedded-compositor-wayland-test_RNDSANDWICHTEST_SWRAST',
                'window-wayland-wl-shell-test_RNDSANDWICHTEST_SWRAST',

                'rendering-tests_wayland-wl-shell-gles30_RNDSANDWICHTEST',
                'render-backend-tests_wayland-wl-shell-gles30_RNDSANDWICHTEST',
                'resource-stress-tests_wayland-wl-shell-gles30_RNDSANDWICHTEST']

        if logic and tools:
            expected_tests += [
                'ramses-viewer-test_UNITTEST']

        if logic and tools and renderer and use_imagemagick:
            expected_tests += [
                'ramses-viewer-gui-test_RNDSANDWICHTEST_SWRAST']

        # step 4: find missing and/or unexpected tests
        unexpected_tests = [t for t in ctest_test_entries if t not in expected_tests]
        missing_tests = [t for t in expected_tests if t not in ctest_test_entries]

        test_failure_msg = ""
        if len(unexpected_tests) != 0:
            test_failure_msg = f"Unexpected tests found : {unexpected_tests}\n"
        if len(missing_tests) != 0:
            test_failure_msg += f"Missing tests : {missing_tests}\n"

        if test_failure_msg != "":
            raise Exception(test_failure_msg)

        assert len(expected_tests) == len(ctest_test_entries)

    print("Test succeeded..\n")
    shutil.rmtree(test_dir)


@click.command(help='Test cmake configure with different cmake option configurations')
@click.option('--build-dir', type=click.Path(exists=True, file_okay=False), required=True, help='Build dir')
@click.pass_context
def main(cli_context, build_dir):
    print("Running cmake configuration tests...")
    start_time = time.time()

    # can configure with (external) luajit
    test_cmake_configuration(cli_context, build_dir, True, enable_luajit=True)
    test_cmake_configuration(cli_context, build_dir, True, enable_x11=True, enable_luajit=True)

    # can configure if trying to build shared lib (on by default) while no window types enabled (generates a warning though)
    test_cmake_configuration(cli_context, build_dir, True)
    test_cmake_configuration(cli_context, build_dir, True, no_full_shared_lib=True)

    # can configure ramses with headless lib only (without full shared lib, and without any window types enabled)
    test_cmake_configuration(cli_context, build_dir, True, no_full_shared_lib=True)
    # can configure ramses with headless lib and full shared lib
    test_cmake_configuration(cli_context, build_dir, True, enable_x11=True)

    # can configure with any window type enabled (full shared lib on by default)
    test_cmake_configuration(cli_context, build_dir, True, enable_x11=True)
    test_cmake_configuration(cli_context, build_dir, True, enable_wayland_ivi=True)
    test_cmake_configuration(cli_context, build_dir, True, enable_wayland_wl_shell=True)

    # can configure with any of examples, demos, tests or tool disabled
    test_cmake_configuration(cli_context, build_dir, True, enable_x11=True, no_examples=True)
    test_cmake_configuration(cli_context, build_dir, True, enable_x11=True, no_demos=True)
    test_cmake_configuration(cli_context, build_dir, True, enable_x11=True, no_tests=True)
    test_cmake_configuration(cli_context, build_dir, True, enable_x11=True, no_tools=True)
    test_cmake_configuration(cli_context, build_dir, True, enable_x11=True, no_full_shared_lib=True,
                             no_examples=True, no_demos=True, no_tests=True, no_tools=True)
    test_cmake_configuration(cli_context, build_dir, True, no_full_shared_lib=True,
                             no_examples=True, no_demos=True, no_tests=True, no_tools=True)

    # can configure with imagemagick enabled even if shared lib not available
    test_cmake_configuration(cli_context, build_dir, True, no_full_shared_lib=True, use_imagemagick=True)
    test_cmake_configuration(cli_context, build_dir, True, enable_x11=True, no_full_shared_lib=True, use_imagemagick=True)

    # can configure with ramses logic disabled
    test_cmake_configuration(cli_context, build_dir, True, disable_logic=True, no_full_shared_lib=True)
    test_cmake_configuration(cli_context, build_dir, True, disable_logic=True, no_full_shared_lib=True, use_imagemagick=True)

    elapsed_time = time.time() - start_time
    print(f"Tests finished SUCCESSFULLY...Elapsed time to run cmake configuration tests : {elapsed_time:.2f} seconds")


if __name__ == "__main__":
    main()
