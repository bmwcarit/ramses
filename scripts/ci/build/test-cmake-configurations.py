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

import build


def test_cmake_configuration(cli_context, build_dir, expect_success, **cmake_options):
    print(f"Running test :{cmake_options}")
    test_dir = Path(build_dir) / "cmake-configuration-tests-build"
    if test_dir.is_dir():
        shutil.rmtree(test_dir)
    Path(test_dir).mkdir(parents=True, exist_ok=True)

    test_failure_msg = None
    try:
        cli_context.invoke(build.build, build_dir=test_dir, configure_only=True, disable_default_window_type=True, **cmake_options)
        if not expect_success:
            test_failure_msg = "Test failed! Death test was supposed to fail cmake configure, but it succeeded!"

    except Exception:
        if expect_success:
            test_failure_msg = "Test failed! Configuration did not succeed!"

    if test_failure_msg:
        raise Exception(test_failure_msg)

    print("Test succeeded..\n")
    shutil.rmtree(test_dir)


@click.command(help='Test cmake configure with different cmake option configurations')
@click.option('--build-dir', type=click.Path(exists=True, file_okay=False), required=True, help='Build dir')
@click.pass_context
def main(cli_context, build_dir):
    print("Running cmake configuration tests...")
    start_time = time.time()

    # can not configure if no useful feature is enabled, i.e., user gets a non-usable project without any shared libs or renderer
    test_cmake_configuration(cli_context, build_dir, False)
    test_cmake_configuration(cli_context, build_dir, False, no_full_shared_lib=True)

    # can configure if trying to build shared lib (on by default) while no window types enabled (generates a warning though)
    test_cmake_configuration(cli_context, build_dir, True, headless=True)

    # can configure ramses with headless lib only (without full shared lib, and without any window types enabled)
    test_cmake_configuration(cli_context, build_dir, True, headless=True, no_full_shared_lib=True)
    # can configure ramses with headless lib and full shared lib
    test_cmake_configuration(cli_context, build_dir, True, headless=True, enable_x11=True)

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
    test_cmake_configuration(cli_context, build_dir, True, headless=True, no_full_shared_lib=True,
                             no_examples=True, no_demos=True, no_tests=True, no_tools=True)

    # can configure with imagemagick enabled even if shared lib not available
    test_cmake_configuration(cli_context, build_dir, True, headless=True, no_full_shared_lib=True, use_imagemagick=True)
    test_cmake_configuration(cli_context, build_dir, True, enable_x11=True, no_full_shared_lib=True, use_imagemagick=True)

    # can configure with ramses logic disabled
    test_cmake_configuration(cli_context, build_dir, True, headless=True, disable_logic=True, no_full_shared_lib=True)
    test_cmake_configuration(cli_context, build_dir, True, headless=True, disable_logic=True, no_full_shared_lib=True, use_imagemagick=True)

    elapsed_time = time.time() - start_time
    print(f"Tests finished SUCCESSFULLY...Elapsed time to run cmake configuration tests : {elapsed_time:.2f} seconds")


if __name__ == "__main__":
    main()
