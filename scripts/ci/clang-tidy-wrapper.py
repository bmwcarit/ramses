#!/usr/bin/env python3

#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import sys
import os
import re
import argparse
import time
from pathlib import Path
import concurrent.futures
from common import compilationdb
from common import repo
from common import lists
from common import clangtidy
from common import yamlconfig

CONFIG_SCHEMA = {
    'type': 'object',
    'properties': {
        'include': {
            'type': 'array',
            'items': {'type': 'string'},
        },
        'exclude': {
            'type': 'array',
            'items': {'type': 'string'},
        },

        'sort-order': {
            'type': 'array',
            'items': {
                'type': 'object',
                'properties': {
                    'pattern': {'type': 'string'},
                    'priority': {'type': 'integer'},
                },
                'additionalProperties': False,
            },
        },

        'check-filter': {
            'type': 'array',
            'items': {
                'type': 'object',
                'properties': {
                    'check': {
                        'anyOf': [
                            {'type': 'string'},
                            {
                                'type': 'array',
                                'items': {'type': 'string'},
                            },
                        ],
                    },
                    'pattern': {'type': 'string'},
                    'include': {
                        'anyOf': [
                            {'type': 'string'},
                            {
                                'type': 'array',
                                'items': {'type': 'string'},
                            },
                        ],
                    },
                    'exclude': {
                        'anyOf': [
                            {'type': 'string'},
                            {
                                'type': 'array',
                                'items': {'type': 'string'},
                            },
                        ],
                    },
                },
                'required': ['check'],
                'additionalProperties': False,
            },
        },

        'remove-duplicate-sources': {'type': 'boolean'},
        'remove-duplicate-reports': {'type': 'boolean'},
        'filter-headers': {'type': 'boolean'},
    },
    'additionalProperties': False,
}

CONFIG_DEFAULTS = {
    'include': ['.*'],
    'exclude': [],
    'sort-order': [],
    'check-filter': [],
    'remove-duplicate-sources': True,
    'remove-duplicate-reports': True,
    'filter-headers': True,
}


def process_compdb_entries(entries, config, includes_filter):
    if config['remove-duplicate-sources']:
        entries = lists.stable_unique(entries, key=lambda e: e.file)
    entries = lists.filter_includes_excludes(entries,
                                             excludes=config['exclude'],
                                             includes=[includes_filter] if includes_filter else config['include'],
                                             key=lambda e: e.relative_file)
    return lists.pattern_priority_sort(entries,
                                       sort_patterns=[(e['pattern'], e['priority']) for e in config['sort-order']],
                                       pattern_key_fun=lambda e: e.relative_file,
                                       fallback_funs=[lambda e: Path(e.file).stat().st_size, lambda e: e.relative_file])


def filter_result_issues(issues, config, unique_issues):
    if config['filter-headers']:
        # ignore files outside project or in build dir
        issues = [issue for issue in issues if issue.is_in_project and not issue.is_in_build_dir]

        issues = lists.filter_includes_excludes(issues,
                                                includes=config['include'],
                                                excludes=config['exclude'],
                                                key=lambda e: e.relative_file)
    result = []
    for issue in issues:
        if not config['remove-duplicate-reports'] or issue.key not in unique_issues:
            unique_issues[issue.key] = issue
            result.append(issue)
    return result


def filter_issues_by_checks(issues, config):
    result = []

    def ensure_list(inp):
        return inp if isinstance(inp, list) else [inp]

    for issue in issues:
        keep_issue = True
        for check_filter in config['check-filter']:
            # skip if check does not match
            if not lists.filter_includes_excludes(issue.all_checks, includes=ensure_list(check_filter['check'])):
                continue

            # keep issue if it passes file filter
            file_includes = ensure_list(check_filter['include']) if 'include' in check_filter else ['.*']
            file_excludes = ensure_list(check_filter['exclude']) if 'exclude' in check_filter else []
            if lists.filter_includes_excludes([issue.relative_file], includes=file_includes, excludes=file_excludes):
                continue

            # keep issue if it matches the pattern or if there is no pattern defined
            if 'pattern' in check_filter:
                rx = re.compile(check_filter['pattern'])
                if not rx.search(issue.text):
                    continue

            # matched by exclude if gets here: discard issue and do not try other checks
            keep_issue = False
            break

        if keep_issue:
            result.append(issue)
    return result


def format_dt(dt):
    seconds = int(dt) % 60
    minutes = int(dt) // 60
    millis = int(dt * 1000.) % 1000
    if minutes != 0:
        return f'{minutes}m{seconds:02}.{millis:03}'
    return f'{seconds}.{millis:03}'


def print_slowest_entries(entries, timing_selection):
    entries = sorted(entries, key=lambda e: e.runtime, reverse=True)
    print(f'Timing statistics for {timing_selection} slowest entries')
    num_entries = len(entries) if timing_selection == 'all' else int(timing_selection)
    for e in entries[:num_entries]:
        print(f'{format_dt(e.runtime):>10}  {e.compdb_entry.relative_file}')


def print_overall_runtime(entries, num_threads, start_time, *, end_time=None):
    wallclock_runtime = (end_time or time.monotonic()) - start_time
    total_cpu_runtime = sum([e.runtime for e in entries])
    cpu_usage = ((total_cpu_runtime / num_threads) / wallclock_runtime) * 100
    print(f'Wallclock runtime {format_dt(wallclock_runtime)}, total thread runtime {format_dt(total_cpu_runtime)}, '
          f'CPU usage {cpu_usage:.1f}%')


def main():
    print("Run:", " ".join(sys.argv))
    parser = argparse.ArgumentParser()
    parser.add_argument('compdb', help='Full path of compile_commands.json')
    parser.add_argument('-f', '--filter', default=None,
                        help='filter processed files that match the provided regex')
    parser.add_argument('--timing', default=None, required=False,
                        help='show timing for X longest entries (or "all" for all)')
    parser.add_argument('--threads', default=os.cpu_count(), type=int, required=False,
                        help='Number of CPU cores to use (default: all)')
    parser.add_argument('-c', '--config', default=None, required=False,
                        help='Path to configuration file')
    parser.add_argument('-r', '--repo', default=repo.get_sdkroot(), required=False,
                        help='Repository root')
    parser.add_argument('--fix', help='Apply clang-tidy fixes.', action='store_true')
    args = parser.parse_args()

    start_time = time.monotonic()
    config = yamlconfig.read_config_with_defaults(args.config, CONFIG_SCHEMA, CONFIG_DEFAULTS)
    entries = compilationdb.load_from_file(args.compdb, Path(args.repo).resolve())
    extra_args = []
    print(f'Loaded {len(entries)} entries from {args.compdb}')
    if args.fix:
        # Multiple clang-tidy threads will interfer each other
        args.threads = 1
        extra_args.append('--fix-errors')

    entries = process_compdb_entries(entries, config, args.filter)
    print(f'Check {len(entries)} remaining entries with {args.threads} threads\n')

    unique_filtered_issues = {}
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.threads) as executor:
        result_entries = []
        unique_issues = {}
        result_futures = map(lambda e: executor.submit(clangtidy.run_on_file, e, extra_args), entries)
        for f_result in concurrent.futures.as_completed(result_futures):
            e = f_result.result()
            result_entries.append(e)

            issues = filter_result_issues(e.issues, config, unique_issues)
            issues = filter_issues_by_checks(issues, config)
            for issue in issues:
                unique_filtered_issues[issue.key] = issue
                print(issue.text, flush=True)

    if args.timing:
        print_slowest_entries(result_entries, args.timing)
        print()

    print_overall_runtime(result_entries, args.threads, start_time)

    if len(unique_filtered_issues) == 0:
        print('Done without issues')
        return 0
    else:
        issue_files = set([i.file for i in unique_filtered_issues.values()])
        print(f'Done with {len(unique_filtered_issues)} unique issues in {len(issue_files)} files')
        return 1


if __name__ == '__main__':
    sys.exit(main())
