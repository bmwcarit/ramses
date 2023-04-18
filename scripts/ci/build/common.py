#!/usr/bin/env python3

#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

from pathlib import Path
import os

# Default is the first entry
COMPILERS = [
    'default',
    'gcc',
    'llvm',
    'clang-tidy',
]

# Default is the first entry
CONFIGS = [
    'Debug',
    'Release',
    'RelWithDebInfo',
]


class CommonConfig:
    def __init__(self, compiler, config, build_dir):
        self.script_dir = Path(os.path.realpath(os.path.dirname(__file__)))
        self.src_dir = (self.script_dir / '..' / '..' / '..').resolve()
        self.compiler = compiler
        self.config = config
        if build_dir:
            self.build_dir = Path(build_dir)
        else:
            self.build_dir = self.src_dir / f'build-{compiler}-{config}'
            self.build_dir.mkdir(parents=True, exist_ok=True)
        self.install_dir = self.build_dir / 'install'

        if self.compiler == 'clang-tidy' and self.config not in ['Debug', 'RelWithDebInfo']:
            raise Exception('clang-tidy works only when build with debug symbols!')

    def get_toolchain(self):
        if self.compiler == 'gcc':
            return self.src_dir / 'cmake/toolchain/Linux_X86_64.toolchain'
        elif self.compiler in ['llvm', 'clang-tidy']:
            return self.src_dir / 'cmake/toolchain/Linux_X86_64_llvm.toolchain'
        else:
            return None
