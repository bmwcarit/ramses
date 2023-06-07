#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import sys
import os
from pathlib import Path
import tempfile

sys.path.append(os.path.realpath(os.path.dirname(__file__) + "/.."))
from common import yamlconfig, compilationdb, clangtidy  # noqa E402 module level import not at top of file

import importlib  # noqa E402 module level import not at top of file
ctw = importlib.import_module('clang-tidy-wrapper')  # noqa E402 module level import not at top of file


# --- test utils
def get_clangtidyresult_with_runtime(inp):
    results = []
    for fname, runtime in inp:
        compdb = compilationdb.CompilationEntry({'directory': '/tmp', 'file': '/tmp/' + fname, 'command': ''},
                                                compdb_path=__file__,
                                                project_root='/tmp/')
        results.append(clangtidy.ClangTidyResult('cmd', '', '', 0, [], runtime, compdb))
    return results


def create_compdb_entries_with_files(root_path, inp):
    entries = []
    for fname, fsize in inp:
        (Path(root_path) / fname).write_bytes(b'.' * fsize)
        entries.append(compilationdb.CompilationEntry({'directory': root_path, 'command': 'c', 'file': (Path(root_path) / fname)},
                                                      __file__,
                                                      project_root=root_path))
    return entries


def create_test_issues(inp):
    db_entries = {fname: compilationdb.CompilationEntry({'directory': '/tmp/build', 'command': 'c', 'file': (Path('/tmp') / fname)},
                                                        __file__, project_root='/tmp')
                  for fname, _ in inp}
    entries = []
    for fname, checks in inp:
        ce = db_entries[fname]
        entries.append(clangtidy.ClangTidyIssue(ce.file, 1, 1, 'foobar', checks, f'text;{fname};{checks}', ce))
    return entries


# --- test config loading
def test_can_load_with_default_config():
    config = yamlconfig.read_config_with_defaults(None, ctw.CONFIG_SCHEMA, ctw.CONFIG_DEFAULTS, path_may_be_none=True)
    for entry in ['include', 'exclude', 'sort-order', 'remove-duplicate-sources', 'remove-duplicate-reports', 'filter-headers']:
        assert entry in config


# --- test time functions
def test_can_format_dt():
    assert '0.000' == ctw.format_dt(0)
    assert '0.001' == ctw.format_dt(0.001)
    assert '1.100' == ctw.format_dt(1.1)
    assert '59.000' == ctw.format_dt(59)
    assert '1m00.000' == ctw.format_dt(60)
    assert '2m03.000' == ctw.format_dt(123)
    assert '2m03.456' == ctw.format_dt(123.456)


def test_can_print_slowest_entries(capsys):
    entries = get_clangtidyresult_with_runtime([('aa', 2), ('bb', 1), ('cc', 3)])
    ctw.print_slowest_entries(entries, 'all')
    out, _ = capsys.readouterr()
    assert out == """Timing statistics for all slowest entries
     3.000  cc
     2.000  aa
     1.000  bb
"""
    ctw.print_slowest_entries(entries, 5)
    out, _ = capsys.readouterr()
    assert out == """Timing statistics for 5 slowest entries
     3.000  cc
     2.000  aa
     1.000  bb
"""
    ctw.print_slowest_entries(entries, 2)
    out, _ = capsys.readouterr()
    assert out == """Timing statistics for 2 slowest entries
     3.000  cc
     2.000  aa
"""


def test_can_print_overall_runtime(capsys):
    entries = get_clangtidyresult_with_runtime([('aa', 2), ('bb', 60), ('cc', 3)])
    ctw.print_overall_runtime(entries, 1, 10, end_time=100)
    out, _ = capsys.readouterr()
    assert out == 'Wallclock runtime 1m30.000, total thread runtime 1m05.000, CPU usage 72.2%\n'
    ctw.print_overall_runtime(entries, 1, 10, end_time=75)
    out, _ = capsys.readouterr()
    assert out == 'Wallclock runtime 1m05.000, total thread runtime 1m05.000, CPU usage 100.0%\n'
    ctw.print_overall_runtime(entries, 10, 10, end_time=75)
    out, _ = capsys.readouterr()
    assert out == 'Wallclock runtime 1m05.000, total thread runtime 1m05.000, CPU usage 10.0%\n'


# --- test process_compdb_entries
def test_process_compdb_entries_sorts_by_default_by_size_only():
    with tempfile.TemporaryDirectory() as d:
        assert ['baz', 'foobar', 'bar'] == \
            [e.relative_file for e in
             ctw.process_compdb_entries(create_compdb_entries_with_files(d, [('foobar', 10), ('bar', 2), ('baz', 30)]),
                                        {'remove-duplicate-sources': False, 'include': ['.*'], 'exclude': [], 'sort-order': []},
                                        None)]
        assert ['foobar', 'baz', 'bar'] == \
            [e.relative_file for e in
             ctw.process_compdb_entries(create_compdb_entries_with_files(d, [('foobar', 10), ('bar', 2), ('baz', 10)]),
                                        {'remove-duplicate-sources': False, 'include': ['.*'], 'exclude': [], 'sort-order': []},
                                        None)]


def test_process_compdb_entries_remove_duplicates():
    with tempfile.TemporaryDirectory() as d:
        assert ['foobar', 'bar'] == \
            [e.relative_file for e in
             ctw.process_compdb_entries(create_compdb_entries_with_files(d, [('foobar', 10), ('bar', 2), ('foobar', 10)]),
                                        {'remove-duplicate-sources': True, 'include': ['.*'], 'exclude': [], 'sort-order': []},
                                        None)]


def test_process_compdb_entries_with_filters():
    with tempfile.TemporaryDirectory() as d:
        assert ['aa', 'ac'] == \
            [e.relative_file for e in
             ctw.process_compdb_entries(create_compdb_entries_with_files(d, [('aa', 10), ('ab', 2), ('cd', 3), ('ac', 5)]),
                                        {'remove-duplicate-sources': True, 'include': ['^a'], 'exclude': ['b'], 'sort-order': []},
                                        None)]
        assert ['aa', 'ac', 'bb'] == \
            [e.relative_file for e in
             ctw.process_compdb_entries(create_compdb_entries_with_files(d, [('aa', 10), ('cc', 2), ('bb', 3), ('ac', 5)]),
                                        {'remove-duplicate-sources': True, 'include': ['^a', 'b$'], 'exclude': [], 'sort-order': []},
                                        None)]
        assert ['aa'] == \
            [e.relative_file for e in
             ctw.process_compdb_entries(create_compdb_entries_with_files(d, [('aa', 10), ('cc', 2), ('bb', 3), ('ac', 5)]),
                                        {'remove-duplicate-sources': True, 'include': ['.*'], 'exclude': ['b', 'c'], 'sort-order': []},
                                        None)]


def test_process_compdb_entries_with_filter_overriding_config_include_filter():
    with tempfile.TemporaryDirectory() as d:
        assert ['aa', 'ac'] == \
            [e.relative_file for e in
             ctw.process_compdb_entries(create_compdb_entries_with_files(d, [('aa', 10), ('ab', 2), ('cd', 3), ('ac', 5)]),
                                        {'remove-duplicate-sources': True, 'include': ['.*'], 'exclude': ['b'], 'sort-order': []},
                                        '^a')]
        assert ['aa', 'ac', 'bb', 'cc'] == \
            [e.relative_file for e in
             ctw.process_compdb_entries(create_compdb_entries_with_files(d, [('aa', 10), ('cc', 2), ('bb', 3), ('ac', 5)]),
                                        {'remove-duplicate-sources': True, 'include': ['aa'], 'exclude': [], 'sort-order': []},
                                        '.*')]


def test_process_compdb_entries_with_extra_sort_order():
    with tempfile.TemporaryDirectory() as d:
        assert ['bc', 'ab', 'aa', 'cc'] == \
            [e.relative_file for e in
             ctw.process_compdb_entries(create_compdb_entries_with_files(d, [('aa', 2), ('ab', 10), ('bc', 5), ('cc', 20)]),
                                        {'remove-duplicate-sources': True, 'include': ['.*'], 'exclude': [],
                                         'sort-order': [{'pattern': '^a', 'priority': 10},
                                                        {'pattern': '^b', 'priority': 20}]},
                                        None)]


# --- test filter_result_issues
def test_issue_filtering_when_no_filtering_requested():
    issues = create_test_issues([('aa', 'c1'), ('bb', 'c2')])
    unique_issues = {}
    f_issues = ctw.filter_result_issues(issues, {'filter-headers': False, 'remove-duplicate-reports': False}, unique_issues)
    assert issues == f_issues
    assert set(unique_issues.values()) == set(issues)


def test_issue_filtering_keeps_duplicates_when_requested():
    issues = create_test_issues([('aa', 'c1'), ('aa', 'c2'), ('aa', 'c1')])
    unique_issues = {}
    f_issues = ctw.filter_result_issues(issues, {'filter-headers': False, 'remove-duplicate-reports': False}, unique_issues)
    assert issues == f_issues
    assert len(unique_issues) == 2


def test_issue_filtering_removes_duplicates_when_requested():
    issues = create_test_issues([('aa', 'c1'), ('aa', 'c2'), ('aa', 'c1')])
    unique_issues = {}
    f_issues = ctw.filter_result_issues(issues, {'filter-headers': False, 'remove-duplicate-reports': True}, unique_issues)
    assert issues[:2] == f_issues
    assert set(unique_issues.values()) == set(issues[:2])


def test_issue_filtering_ignores_filter_values_when_no_header_filter_requested():
    issues = create_test_issues([('aa', 'c1'), ('aa', 'c2')])
    f_issues = ctw.filter_result_issues(issues, {'filter-headers': False, 'remove-duplicate-reports': False,
                                                 'include': ['text_not_matching_any_issue'], 'exclude': []}, {})
    assert issues == f_issues


def test_issue_filtering_uses_filter_values_when_requested():
    issues = create_test_issues([('aa', 'c1'), ('ab', 'c2'), ('bb', 'c2')])
    f_issues = ctw.filter_result_issues(issues, {'filter-headers': True, 'remove-duplicate-reports': False, 'include': ['^a'], 'exclude': ['b$']}, {})
    assert issues[:1] == f_issues


def test_issue_filtering_ignores_headers_outside_project_based_on_config():
    issues = create_test_issues([('aa', 'c1'), ('/xx/ab', 'c2')])
    f_issues = ctw.filter_result_issues(issues, {'filter-headers': True, 'remove-duplicate-reports': False, 'include': ['.*'], 'exclude': []}, {})
    assert issues[:1] == f_issues


def test_issue_filtering_ignores_headers_in_build_dir_based_on_config():
    issues = create_test_issues([('aa', 'c1'), ('/tmp/build/ab', 'c2')])
    f_issues = ctw.filter_result_issues(issues, {'filter-headers': True, 'remove-duplicate-reports': False, 'include': ['.*'], 'exclude': []}, {})
    assert issues[:1] == f_issues


# --- test filter_issues_by_checks
def test_check_filtering_does_nothing_when_empty():
    issues = create_test_issues([('aa', 'c1'), ('aa', 'c2')])
    assert issues == ctw.filter_issues_by_checks(issues, {'check-filter': []})


def test_check_filtering_does_nothing_when_checks_not_matching():
    issues = create_test_issues([('aa', 'c1'), ('aa', 'c2')])
    assert issues == ctw.filter_issues_by_checks(issues, {'check-filter': [{'check': 'c3', 'exclude': '.*'}]})


def test_check_filtering_filters_out_when_check_matches_exactly():
    issues = create_test_issues([('aa', 'c1'), ('aa', 'c2')])
    assert [issues[0]] == ctw.filter_issues_by_checks(issues, {'check-filter': [{'check': '^c2$', 'exclude': '.*'}]})


def test_check_filtering_filters_out_when_check_matches_by_partial_regex():
    issues = create_test_issues([('aa', 'c1'), ('aa', 'ca2'), ('aa', 'cb2')])
    assert [issues[0], issues[2]] == ctw.filter_issues_by_checks(issues, {'check-filter': [{'check': 'ca.', 'exclude': '.*'}]})


def test_check_filtering_filters_on_multiple_checks():
    issues = create_test_issues([('aa', 'c1'), ('aa', 'ca2'), ('aa', 'cb2')])
    assert [issues[2]] == ctw.filter_issues_by_checks(issues, {'check-filter': [{'check': ['c1', 'ca.'], 'exclude': '.*'}]})


def test_check_filtering_filters_on_any_of_multiple_checks_in_issue():
    issues = create_test_issues([('aa', 'c1,c2,c3'), ('aa', 'c2,c1'), ('aa', 'c4,c1')])
    assert [issues[2]] == ctw.filter_issues_by_checks(issues, {'check-filter': [{'check': 'c2', 'exclude': '.*'}]})


def test_check_filtering_filters_file_by_include():
    issues = create_test_issues([('aa', 'c1'), ('bb', 'c2')])
    assert [issues[1]] == ctw.filter_issues_by_checks(issues, {'check-filter': [{'check': 'c1', 'include': 'bb'}]})


def test_check_filtering_filters_file_by_include_by_regex():
    issues = create_test_issues([('aa', 'c1'), ('bb', 'c1')])
    assert [issues[0]] == ctw.filter_issues_by_checks(issues, {'check-filter': [{'check': 'c1', 'include': '^a.'}]})


def test_check_filtering_filters_file_by_multiple_include():
    issues = create_test_issues([('aa', 'c1'), ('ba', 'c1'), ('ca', 'c1')])
    assert [issues[0], issues[1]] == ctw.filter_issues_by_checks(issues, {'check-filter': [{'check': 'c1', 'include': ['^a', '^b']}]})


def test_check_filtering_filters_file_by_exclude():
    issues = create_test_issues([('aa', 'c1'), ('bb', 'c1')])
    assert [issues[0]] == ctw.filter_issues_by_checks(issues, {'check-filter': [{'check': 'c1', 'exclude': 'bb'}]})


def test_check_filtering_filters_file_by_exclude_by_regex():
    issues = create_test_issues([('aa', 'c1'), ('bb', 'c1')])
    assert [issues[1]] == ctw.filter_issues_by_checks(issues, {'check-filter': [{'check': 'c1', 'exclude': '^a.'}]})


def test_check_filtering_filters_file_by_multiple_exclude():
    issues = create_test_issues([('aa', 'c1'), ('ba', 'c1'), ('ca', 'c1')])
    assert [issues[2]] == ctw.filter_issues_by_checks(issues, {'check-filter': [{'check': 'c1', 'exclude': ['^a', '^b']}]})


def test_check_filtering_filters_by_include_and_exclude():
    issues = create_test_issues([('aa', 'c1'), ('ba', 'c1'), ('cc', 'c1')])
    assert [issues[0]] == ctw.filter_issues_by_checks(issues, {'check-filter': [{'check': 'c1', 'include': 'a', 'exclude': 'b'}]})


def test_check_filtering_filters_all_in_one():
    issues = create_test_issues([('aa', 'c1,c2,c3'), ('ba', 'c1,c3'), ('cc', 'c1,c4'), ('cd', 'c2,c4'), ('cx', 'c5')])
    assert [issues[0], issues[3], issues[4]] == ctw.filter_issues_by_checks(issues, {'check-filter': [
        {'check': ['c1$', '^c2'], 'include': ['^aa$', 'c']},
        {'check': '^c[34]$', 'include': '.*', 'exclude': ['^cc$', 'x']},
    ]})
