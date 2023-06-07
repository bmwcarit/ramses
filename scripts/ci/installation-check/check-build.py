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
import shutil
from pathlib import Path


@click.command(help='Builds text executables with ramses installed in INSTALL_DIR')
@click.argument('install_dir', type=click.Path(exists=True, file_okay=False))
@click.option('--system-glm', is_flag=True, default=False, help='glm is not installed with ramses, but needs to be found')
@click.option('--headless', is_flag=True, default=False, help='Build headless shared lib')
@click.option('-b', '--build-dir', type=click.Path(), help='Build dir')
def main(install_dir, build_dir, headless, system_glm):
    if not build_dir:
        build_dir = Path(install_dir).parent / f'install-check-build{"-headless" if headless else ""}'
    print(f"Ramses installation: {install_dir}, Build: {build_dir}, Headless: {headless}, System-GLM:{system_glm}")

    if build_dir.is_dir():
        shutil.rmtree(build_dir)
    build_dir.mkdir(parents=True)

    cmake_conf = ['cmake', f'-DCMAKE_PREFIX_PATH={install_dir}', f'-B{build_dir}']

    script_dir = Path(__file__).absolute().parent
    if headless:
        src_dir = script_dir / 'shared-lib-headless-check'
        cmake_conf += [f'-S{src_dir}']
    else:
        src_dir = script_dir / 'shared-lib-check'
        cmake_conf += [f'-S{src_dir}']

    if system_glm:
        cmake_conf += [f'-DCMAKE_MODULE_PATH={script_dir / "cmake"}', '-DSYSTEM_GLM=TRUE']

    subprocess.check_call(cmake_conf, cwd=build_dir)
    subprocess.check_call(['cmake', '--build', build_dir, '--target', 'run-all'], cwd=build_dir)


if __name__ == "__main__":
    main()
