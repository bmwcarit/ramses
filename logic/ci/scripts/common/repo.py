#  -------------------------------------------------------------------------
#  Copyright (C) 2020 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import subprocess
import os


def get_sdkroot(from_dir=None):
    """Get sdk root folder by asking git"""
    return subprocess.check_output(['git', 'rev-parse', '--show-toplevel'],
                                   cwd=from_dir or os.path.dirname(os.path.realpath(__file__))).decode('utf-8').strip()


def get_git_files(path, *, including_submodules, relative_to_path=True, make_absolute=False):
    """Get all files known to git"""
    cmd = ['git', 'ls-files', '-z']
    if including_submodules:
        cmd.append('--recurse-submodules')
    if not relative_to_path:
        cmd.append('--full-name')
    files = [f.decode('utf-8') for f in subprocess.check_output(cmd, cwd=path).split(b'\0') if f]
    if make_absolute:
        base = path if relative_to_path else get_sdkroot(path)
        files = [os.path.join(base, f) for f in files]
    return files


def get_revision_hash(rev='HEAD', repo=None):
    return subprocess.check_output(['git', 'rev-parse', rev], cwd=(repo or get_sdkroot())).decode('utf-8').strip()


def get_commit_count(rev='HEAD', repo=None):
    return subprocess.check_output(['git', 'rev-list', '--count', rev], cwd=(repo or get_sdkroot())).decode('utf-8').strip()


def describe(pattern=None, rev='HEAD', repo=None):
    return subprocess.check_output(['git', 'describe', '--tags', '--match', pattern or '*[0-9].[0-9]*', rev],
                                   cwd=(repo or get_sdkroot())).decode('utf-8').strip()
