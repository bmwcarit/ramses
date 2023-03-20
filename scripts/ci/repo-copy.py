#!/usr/bin/env python3

#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import argparse
import common.repo
import common.lists
import common.yamlconfig
from pathlib import Path
import shutil

CONFIG_SCHEMA = {
    'type': 'object',
    'properties': {
        'source-include': {
            'type': 'array',
            'items': {'type': 'string'},
        },
        'source-exclude': {
            'type': 'array',
            'items': {'type': 'string'},
        },
        'destination-keep': {
            'type': 'array',
            'items': {'type': 'string'},
        },
        'source-git-root': {'type': 'boolean'},
        'destination-git-root': {'type': 'boolean'},
        'destination-subdir': {'type': 'string'},
        'version-file': {'type': 'string'},
        'keep-submodules': {'type': 'boolean'}
    },
    'additionalProperties': False,
}

CONFIG_DEFAULTS = {
    'source-include': ['.*'],
    'source-exclude': [],
    'destination-keep': [],
    'source-git-root': True,
    'destination-git-root': True,
    'destination-subdir': None,
    'version-file': None,
    'keep-submodules': False
}


def get_folders_from_files(files):
    result = set()
    for f in files:
        result |= set(Path(f).parents)
    return result - set([Path('.')])


def remove_files(root, files):
    for f in files:
        p = Path(root) / f
        if p.exists():
            p.unlink()


def remove_empty_directories(root, dirs):
    for d in sorted(dirs, reverse=True):
        p = Path(root) / d
        if p.exists() and not list(p.glob('*')):
            p.rmdir()


def copy_files(source_root, destination_root, files):
    for f in files:
        source_path = Path(source_root) / f
        if not source_path.is_dir():
            destination_path = Path(destination_root) / f
            destination_path.parent.mkdir(parents=True, exist_ok=True)
            if source_path.is_symlink():
                destination_path.unlink()
            shutil.copy(source_path, destination_path, follow_symlinks=False)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-c', '--config', required=True, help='copy config')
    parser.add_argument('-s', '--source', required=False, default=common.repo.get_sdkroot(),
                        help='source repository (default: this)')
    parser.add_argument('-d', '--destination', required=True, help='destination repository')
    args = parser.parse_args()

    # read config
    assert Path(args.config).is_file()
    config = common.yamlconfig.read_config_with_defaults(args.config, CONFIG_SCHEMA, CONFIG_DEFAULTS, path_may_be_none=False)

    # check arguments against config
    assert Path(args.source).is_dir()
    assert Path(args.destination).is_dir()
    assert Path(args.source) != Path(args.destination), 'source and destination must differ'

    if config['source-git-root']:
        assert Path(common.repo.get_sdkroot(from_dir=args.source)) == Path(args.source).resolve(), \
            f'config required {args.source} to be a git root'
    if config['destination-git-root']:
        assert Path(common.repo.get_sdkroot(from_dir=args.destination)) == Path(args.destination).resolve(), \
            f'config required {args.destination} to be a git root'
    keep_submodules = config['keep-submodules']

    # gather source files
    source_files = common.repo.get_git_files(args.source, including_submodules=not keep_submodules, relative_to_path=True)
    source_files = common.lists.filter_includes_excludes(source_files,
                                                         includes=config['source-include'],
                                                         excludes=config['source-exclude'])

    # construct real destination dir
    destination_base = Path(args.destination)
    if config['destination-subdir']:
        subdir = Path(config['destination-subdir'])
        assert not subdir.is_absolute(), 'destination-subdir may not be an absolute path'
        destination_base = destination_base / subdir

    # gather destination files to delete
    destination_files = common.repo.get_git_files(destination_base, including_submodules=not keep_submodules, relative_to_path=True)
    optional_version_file_exclude = [config['version-file']] if config['version-file'] else []
    destination_files_delete = common.lists.filter_includes_excludes(destination_files, excludes=config['destination-keep'] + optional_version_file_exclude)

    # verify no overlap between kept destination files and to copy source files
    overlap_files = (set(common.lists.filter_includes_excludes(destination_files, includes=config['destination-keep'])) &
                     set(source_files))
    assert not overlap_files, ('Found overlap between source files to copy and destination files to keep:\n  ' +
                               "\n  ".join(sorted(overlap_files)))

    if keep_submodules:
        # directories can only be submodules, remove from file lists
        source_files = [f for f in source_files if not (Path(args.source) / f).is_dir()]
        destination_files_delete = [d for d in destination_files_delete if not (Path(destination_base) / d).is_dir()]

    # remove destination files
    destination_delete_all = len(destination_files_delete)
    destination_files_delete = set(destination_files_delete) - set(source_files)
    print(f'Remove {len(destination_files_delete)}/{destination_delete_all} files at {destination_base}')
    remove_files(destination_base, destination_files_delete)

    # copy source -> destination
    print(f'Copy files from {args.source} -> {destination_base}')
    copy_files(args.source, destination_base, source_files)

    # cleanup now empty directories
    print('Cleanup directories')
    remove_empty_directories(destination_base, get_folders_from_files(destination_files))

    if config['version-file']:
        loc = Path(destination_base) / config['version-file']
        loc.write_text(f"""{common.repo.get_revision_hash(repo=args.source)}
{common.repo.get_commit_count(repo=args.source)}
{common.repo.describe(repo=args.source) or "No tags found"}
""")

    print('Done')


if __name__ == "__main__":
    main()
