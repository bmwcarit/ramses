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
import re
from pathlib import Path


@click.command(help='Check if there are missing symbols from shared lib')
@click.option('--headless-only', is_flag=True, default=False, help='Check headless shared lib')
@click.option('-b', '--build-dir', type=click.Path(), help='Build dir')
def main(build_dir, headless_only):
    if not build_dir:
        raise Exception("build dir needed")

    client_static_lib_dir = Path(build_dir) / 'src/client/libramses-client.a'
    framework_static_lib_dir = Path(build_dir) / 'src/framework/libramses-framework.a'
    renderer_static_lib_dir = Path(build_dir) / 'src/renderer/libramses-renderer.a'
    headless_shared_lib_dir = Path(build_dir) / 'install/lib/libramses-shared-lib-headless.so'
    full_shared_lib_dir = Path(build_dir) / 'install/lib/libramses-shared-lib.so'

    def file_to_symbols(lib_file, is_shared_lib):
        if not lib_file.exists():
            raise Exception(f'File does not exist: {lib_file}')

        # run "nm" on the lib file (without demangling, since nm has limited demangling capability)
        nm_command = ['nm', '--defined-only', lib_file]
        if is_shared_lib:
            nm_command += ['--dynamic']
        nm_process_result = subprocess.run(nm_command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

        # run c++filt to demangle output from nm
        cppfilt_command = ['c++filt', '-r']
        cppfilt_process_result = subprocess.run(cppfilt_command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, input=nm_process_result.stdout)
        cppfilt_process_output_std = cppfilt_process_result.stdout.decode('utf-8')

        ignore_list = [
            # ignore ramses internal and Impl
            r'ramses::internal::',
        ]

        # Use (?!...) for negative match to filter out symbols in the ignore list
        ignore_regex = ''.join([f'(?!.*{e})' for e in ignore_list])
        # Example of symbol line in output:
        # 0000000000001d30 T ramses::Appearance::unbindInput(ramses::UniformInput const&)
        symbol_regex = re.compile(rf"^([0-9A-Fa-f]+)(\s+)[TB](\s+)({ignore_regex}.*)", re.MULTILINE)

        lib_symbols = [s.group(4) for s in re.finditer(symbol_regex, cppfilt_process_output_std)]

        return lib_symbols

    static_lib_symbols = file_to_symbols(client_static_lib_dir, False)
    static_lib_symbols += file_to_symbols(framework_static_lib_dir, False)
    if not headless_only:
        static_lib_symbols += file_to_symbols(renderer_static_lib_dir, False)

    shared_lib_symbols = file_to_symbols(headless_shared_lib_dir, True)
    if not headless_only:
        shared_lib_symbols += file_to_symbols(full_shared_lib_dir, True)

    missing_symbols = [s for s in static_lib_symbols if s not in shared_lib_symbols]
    if len(missing_symbols) > 0:
        raise Exception(f"FOUND MISSING SYMBOLS: {missing_symbols}")


if __name__ == "__main__":
    main()
