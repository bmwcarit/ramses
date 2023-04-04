#!/usr/bin/env python3

#  -------------------------------------------------------------------------
#  Copyright (C) 2020 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import os
import sys
import re
import subprocess
import argparse
import shutil
import git
from pathlib import Path
import datetime
import textwrap


def get_sdkroot():
    """Get sdk root folder by asking git"""
    return subprocess.check_output(['git', 'rev-parse', '--show-toplevel'],
                                   shell=False, cwd=os.path.dirname(os.path.realpath(__file__))).decode('utf-8').strip()


def get_submodule_refs():
    return {
        # Same tag exists in both repos, but not related in any way - can't check ancestry at all
        'external/ramses': {
            'internal': 'refs/tags/27.0.130',
            'public': 'refs/tags/27.0.130',
            'strict_ancestry': False,
        },
        'external/googletest': {
            'internal': 'origin/v1.11.0-with-fixes',
            'public': 'refs/tags/release-1.11.0',
            'strict_ancestry': True,
        },
        'external/google-benchmark': {
            'internal': 'origin/v1.5.2-with-fixes',
            'public': 'refs/tags/v1.5.2',
            'strict_ancestry': True,
        },
        'external/fmt': {
            'internal': 'refs/tags/7.1.3',
            'public': 'refs/tags/7.1.3',
            'strict_ancestry': True,
        },
        'external/flatbuffers': {
            'internal': 'origin/v1.12.0_with_fixes',
            'public': 'refs/tags/v1.12.0',
            'strict_ancestry': True,
        },
        'external/lua': {
            'internal': 'origin/v5.1.1_with_fixes2',
            'public': 'refs/tags/v5.1.1',
            'strict_ancestry': True,
        },
        'external/sol': {
            'internal': 'origin/v3.2.2_release_with_fixes3',
            'public': 'refs/tags/v3.2.2',
            'strict_ancestry': True,
        },
        'external/imgui': {
            'internal': 'origin/v1.74_ramsesfixes',
            'public': 'refs/tags/v1.74',
            'strict_ancestry': True,
        },
        'external/cli11': {
            'internal': 'refs/tags/v2.2.0',
            'public': 'refs/tags/v2.2.0',
            'strict_ancestry': True,
        },
    }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--oss-mirror', required=True, help='Location of oss mirror repo')
    parser.add_argument('--release-name', required=False, default=None,
                        help='Set name/version for release, uses <commit-SNAPSHOT> if not set.')
    parser.add_argument('--no-submodule-init', required=False, default=False, action='store_true',
                        help='Do not initialize or sync submodules in mirror')
    args = parser.parse_args()

    internal_root = Path(get_sdkroot())
    internal = git.Repo(internal_root)
    mirror = git.Repo(args.oss_mirror)
    mirror_root = Path(args.oss_mirror)
    assert not mirror.is_dirty(), 'mirror repo must not be dirty'

    init_submodules = not args.no_submodule_init

    submodule_keys = set(get_submodule_refs().keys())

    # This checks that submodules are the same everywhere (not the same commit, but the same set of keys)
    # Because of the way submodules work it's very difficult to make this work automatic, so if any of these
    # asserts triggers, fix the submodule manually on the respective place with 'submodule add' and re-run the script
    print(' Check that submodules do not differ across all relevant places')
    gitmodules_keys_internal = set(re.findall(r'submodule "([^"]+)"', (internal_root / '.gitmodules').read_text()))
    gitmodules_keys_internal_oss = set(re.findall(r'submodule "([^"]+)"', (internal_root / 'ci/oss/oss.gitmodules').read_text()))
    gitmodules_keys_oss_mirror = set(re.findall(r'submodule "([^"]+)"', (mirror_root / '.gitmodules').read_text()))
    assert submodule_keys == gitmodules_keys_internal, f'internal submodule mismatch at {submodule_keys ^ gitmodules_keys_internal}'
    assert submodule_keys == gitmodules_keys_internal_oss, f'internal oss submodule mismatch at {submodule_keys ^ gitmodules_keys_internal_oss}'
    assert submodule_keys == gitmodules_keys_oss_mirror, f'oss mirror submodule mismatch at {submodule_keys ^ gitmodules_keys_oss_mirror}'

    release_copy_cmd = [sys.executable,
                        'external/ramses/proprietary/scripts/ci-scripts/release-copy.py',
                        '--source', internal_root,
                        '--destination', args.oss_mirror,
                        '--config', 'ci/release-copy/oss.yaml']

    subprocess.check_call(release_copy_cmd, cwd=internal_root)

    print('Copy special oss files')
    special_oss_files = {
        'ci/oss/oss.gitmodules': '.gitmodules',
    }
    for k, v in special_oss_files.items():
        shutil.copy(internal_root / k, mirror_root / v)

    if init_submodules:
        print(' Sync & init submodules in mirror')
        subprocess.check_call(['git', 'submodule', 'sync'], cwd=mirror_root)
        subprocess.check_call(['git', 'submodule', 'init'], cwd=mirror_root)

    print(' Update submodules in mirror')
    subprocess.check_call(['git', 'submodule', 'update'], cwd=mirror_root)
    for rel_path, ref_config in get_submodule_refs().items():
        internal_ref = ref_config['internal']
        public_ref = ref_config['public']
        internal_path = (internal_root / rel_path)
        mirror_path = (mirror_root / rel_path)
        os.makedirs(mirror_path, exist_ok=True)
        submod_internal = internal.submodule(rel_path).module()

        # Update submodules in mirror to target state
        # --prune-tags makes sure local tags are deleted in favor of upstream tags
        # This ensures that we are never taking "our own tags" over upstream tags
        subprocess.check_call(['git', 'fetch', '--prune-tags'], cwd=mirror_path)
        subprocess.check_call(['git', 'checkout', public_ref], cwd=mirror_path)
        submodule_update_cmd = ['git', 'submodule', 'update', '--recursive']
        if init_submodules:
            submodule_update_cmd.append('--init')
        subprocess.check_call(submodule_update_cmd, cwd=mirror_path)

        # Make sure the internal submodule is at the expected commit
        actual_commit = str(submod_internal.head.commit)
        expected_commit = subprocess.check_output(['git', 'rev-list', '-n', '1', internal_ref], cwd=internal_path).decode("utf-8").strip()
        assert expected_commit == actual_commit, \
               f'Submodule {rel_path} is not at ref {internal_ref} (expected commit {expected_commit}, actual commit: {actual_commit})'

        # Make sure internal commit has public commit as ancestor
        # Custom patches on top of public version are OK, but must be a strict successor
        if ref_config['strict_ancestry']:
            public_commit = subprocess.check_output(['git', 'rev-list', '-n', '1', public_ref], cwd=mirror_path).decode("utf-8").strip()
            subprocess.check_call(['git', 'merge-base', '--is-ancestor', public_commit, actual_commit], cwd=(internal_path))

    print('Create commit in mirror')
    date_as_string = datetime.datetime.now().strftime("%Y-%m-%d-%H-%M")
    branch_name = f'feature/oss-release-{date_as_string}'
    commit_sha = internal.head.object.hexsha
    release_name = args.release_name or f"{commit_sha[:6]}-SNAPSHOT"
    commit_msg = textwrap.dedent(f"""
        Oss release {release_name} created {date_as_string}

        see CHANGELOG.md for details

        Original commit sha: {commit_sha}

        Co-authored-by: Askanaz Torosyan <46795157+nVxx@users.noreply.github.com>
        Co-authored-by: Daniel Haas <25718295+bojackHaasman@users.noreply.github.com>
        Co-authored-by: Mohamed Sharaf-El-Deen <769940+mohhsharaf@users.noreply.github.com>
        Co-authored-by: Mirko Sova <64351017+smirko-dev@users.noreply.github.com>
        Co-authored-by: Tobias Hammer <tohammer@users.noreply.github.com>
        Co-authored-by: Violin Yanev <violinyanev@users.noreply.github.com>
    """).strip()

    author_and_committer = git.Actor('Ramses Tech User', '94632088+ramses-tech-user@users.noreply.github.com')
    subprocess.check_call(['git', 'add', '-A', '-f'], cwd=mirror_root)
    mirror.index.commit(commit_msg, author=author_and_committer, committer=author_and_committer)
    mirror.head.reference = mirror.create_head(branch_name)

    print('Done')


if __name__ == '__main__':
    sys.exit(main())
