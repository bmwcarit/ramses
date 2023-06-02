#!/usr/bin/env python3

#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import click
import subprocess
import os
import glob
import shutil

import common

# Default is the first entry
CPP_STANDARDS = [
    '17', '20'
]


def to_cmake(property):
    return 'ON' if property else 'OFF'


class BuildConfig(common.CommonConfig):
    def __init__(self, compiler, config, build_dir):
        super(BuildConfig, self).__init__(compiler, config, build_dir)

    def cmake_configure(self,
                        disable_default_window_type, enable_x11, enable_android, enable_wayland_ivi, enable_wayland_wl_shell,
                        flatbuf_gen, android_abi, disable_logic, use_imagemagick,
                        headless, no_full_shared_lib,
                        no_examples, no_demos, no_tests, no_tools, generator,
                        enable_dlt, enable_lto, test_coverage, enable_coverage,
                        package_name, cpp_std, cmake_modules):

        optional_args = []
        if self.compiler == 'llvm':
            optional_args.append("-Dramses-sdk_USE_LINKER_OVERWRITE=lld")

        # Android is special and requires some extra setup
        if android_abi:
            ndk_home = os.environ['ANDROID_NDK_HOME']
            optional_args.append(f'-DCMAKE_TOOLCHAIN_FILE={ndk_home}/build/cmake/android.toolchain.cmake')
            # Android doesn't allow writing files in the source tree -> disable
            flatbuf_gen = False
            # Disable all binaries
            no_examples = True
            no_demos = True
            no_tests = True
            no_tools = True
            optional_args.append('-Dramses-sdk_BUILD_DAEMON=OFF')
            optional_args.append('-Dramses-sdk_BUILD_IVI_TEST_APPS=OFF')
            # TODO These should not be needed, if using Gradle instead of CMake
            optional_args.append('-DANDROID_PLATFORM=21')
            optional_args.append(f'-DANDROID_ABI={android_abi}')

        if self.get_toolchain():
            optional_args.append(f'-DCMAKE_TOOLCHAIN_FILE={self.get_toolchain()}')

        if package_name:
            optional_args.append(f'-DCPACK_PACKAGE_NAME={package_name}')

        if test_coverage:
            optional_args.append('-Dramses-sdk_ENABLE_COVERAGE=1')

        if headless:
            optional_args.append('-Dramses-sdk_BUILD_HEADLESS_SHARED_LIB=1')

        if no_full_shared_lib:
            optional_args.append('-Dramses-sdk_BUILD_FULL_SHARED_LIB=0')

        if disable_logic:
            optional_args.append('-Dramses-sdk_ENABLE_LOGIC=OFF')

        if disable_default_window_type:
            optional_args.append('-Dramses-sdk_ENABLE_DEFAULT_WINDOW_TYPE=0')

        if no_examples:
            optional_args.append('-Dramses-sdk_BUILD_EXAMPLES=0')
        if no_demos:
            optional_args.append('-Dramses-sdk_BUILD_DEMOS=0')
        if no_tests:
            optional_args.append('-Dramses-sdk_BUILD_TESTS=0')
        if no_tools:
            optional_args.append('-Dramses-sdk_BUILD_TOOLS=0')
        if cmake_modules:
            optional_args.append(f'-DCMAKE_MODULE_PATH={cmake_modules}')

        generator = generator
        if generator:
            optional_args += ['-G', generator]

        # error checking
        if self.config != "Debug" and enable_coverage:
            raise Exception("Code coverage should only run with debug build!")

        args = [
            'cmake',
            *optional_args,
            f'-DCMAKE_BUILD_TYPE={self.config}',
            f'-DCMAKE_INSTALL_PREFIX={self.install_dir}',
            f'-Dramses-sdk_ENABLE_WINDOW_TYPE_X11={to_cmake(enable_x11)}',
            f'-Dramses-sdk_ENABLE_WINDOW_TYPE_ANDROID={to_cmake(enable_android)}',
            f'-Dramses-sdk_ENABLE_WINDOW_TYPE_WAYLAND_IVI={to_cmake(enable_wayland_ivi)}',
            f'-Dramses-sdk_ENABLE_WINDOW_TYPE_WAYLAND_WL_SHELL={to_cmake(enable_wayland_wl_shell)}',
            f'-Dramses-sdk_ENABLE_DLT={to_cmake(enable_dlt)}',
            f'-Dramses-sdk_ENABLE_FLATBUFFERS_GENERATION={to_cmake(flatbuf_gen)}',
            f'-Dramses-sdk_BUILD_WITH_LTO={to_cmake(enable_lto)}',
            f'-Dramses-sdk_ENABLE_COVERAGE={to_cmake(enable_coverage)}',
            f'-Dramses-sdk_USE_IMAGEMAGICK={to_cmake(use_imagemagick)}',
            f"-Dramses-sdk_CPP_VERSION={cpp_std}",
            '-DCMAKE_EXPORT_COMPILE_COMMANDS=1',
            '-DCMAKE_INSTALL_MESSAGE=NEVER',
            '-Wdev', '-Werror=dev',
            f'-S{self.src_dir}',
            f'-B{self.build_dir}',
        ]
        print(f'CWD: {self.build_dir}\nRun:', ' '.join(args))
        subprocess.check_call(args, cwd=self.build_dir)

    def cmake_build(self, target):
        args = [
            'cmake',
            '--build', str(self.build_dir),
            '--config', str(self.config),
            '--target', f'{target}',
        ]
        print('Run:', ' '.join(args))
        subprocess.check_call(args, cwd=self.build_dir)

    def copy_package(self, package_target):
        files = glob.iglob(os.path.join(self.build_dir, "*.tar.gz"))
        for file in files:
            if os.path.isfile(file):
                print(f'Copying {file} to {package_target}')
                shutil.copy2(file, package_target)


@click.command(help='Build wrapper script which makes it more convenient to build ramses with its multitude of options')
@click.option('--compiler', type=click.Choice(common.COMPILERS, case_sensitive=False), default=common.COMPILERS[0])
@click.option('--config', type=click.Choice(common.CONFIGS, case_sensitive=False), default=common.CONFIGS[0])
@click.option('-b', '--build-dir', type=click.Path(exists=True, file_okay=False))
@click.option('--disable-default-window-type', is_flag=True, default=False, help='Disable cmake option for default window type')
@click.option('--enable-x11', is_flag=True, default=False, help='Enable building for creating x11 window')
@click.option('--enable-android', is_flag=True, default=False, help='Enable building for creating android window')
@click.option('--enable-wayland-ivi', is_flag=True, default=False, help='Enable building for creating wayland windows that use ivi_shell')
@click.option('--enable-wayland-wl-shell', is_flag=True, default=False, help='Enable building for creating wayland windows that use wl_shell')
@click.option('--build-target', default='install', help='What CMake target to build')
@click.option('--flatbuf-gen', is_flag=True, default=False, help='Generate flatbuffer file headers')
@click.option('--android-abi', help='Set ABI when building on Android')
@click.option('--headless', is_flag=True, default=False, help='Build headless shared lib without renderer')
@click.option('--no-full-shared-lib', is_flag=True, default=False, help='Disable building full shared lib with renderer')
@click.option('--no-examples', is_flag=True, default=False, help='Dont build examples')
@click.option('--no-demos', is_flag=True, default=False, help='Dont build demos')
@click.option('--no-tests', is_flag=True, default=False, help='Dont build tests')
@click.option('--no-tools', is_flag=True, default=False, help='Dont build tools')
@click.option('--configure-only', is_flag=True, default=False, help='Run only configure, dont build')
@click.option('--generator', default=None, help='CMake generator (passed to -G)')
@click.option('--enable-dlt', is_flag=True, default=False, help='Build with DLT support')
@click.option('--enable-lto', is_flag=True, default=False, help='Build with LTO support')
@click.option('--test-coverage', is_flag=True, default=False, help='Enable test coverage')
@click.option('--enable-coverage', is_flag=True, default=False, help='Enable code coverage')
@click.option('--package-name', default="", help='Use a different package name for CPack than the default')
@click.option('--package-destination', type=click.Path(exists=True, file_okay=False), help='Specify a folder where the package shall be copied')
@click.option('--cpp-std', type=click.Choice(CPP_STANDARDS), default=CPP_STANDARDS[0])
@click.option('--disable-logic', is_flag=True, default=False, help='Disable building ramses logic')
@click.option('--use-imagemagick', is_flag=True, default=False, help='Build tests that use imagemagick')
@click.option('--cmake-modules', help='Sets cmake module path')
def build(compiler, config, build_dir, configure_only, build_target, package_destination, **kwargs):
    conf = BuildConfig(compiler, config, build_dir)
    conf.cmake_configure(**kwargs)

    if configure_only:
        return

    conf.cmake_build(build_target)

    if kwargs.get('package_name'):
        conf.cmake_build(target='package')

        if package_destination:
            conf.copy_package(package_destination)


if __name__ == "__main__":
    build()
