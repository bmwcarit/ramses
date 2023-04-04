#  -------------------------------------------------------------------------
#  Copyright (C) 2020 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import re
import subprocess
import time
from itertools import zip_longest
from pathlib import Path


class ClangTidyIssue:
    def __init__(self, file, line, column, summary, checks, text, compdb_entry=None):
        self._file = str(Path(file).resolve())
        self._line = int(line)
        self._column = int(column)
        self._summary = summary
        self._text = text.strip() + '\n'
        self._checks = checks.split(',')
        self._has_warnings_as_error = '-warnings-as-errors' in self._checks
        self._compdb_entry = compdb_entry

    @property
    def file(self):
        return self._file

    @property
    def relative_file(self):
        if not self._compdb_entry or not self._compdb_entry.project_root:
            raise RuntimeError('project_root not available')
        return str(Path(self.file).relative_to(self._compdb_entry.project_root))

    @property
    def check(self):
        return self._checks[0]

    @property
    def all_checks(self):
        return self._checks

    @property
    def has_warnings_as_error(self):
        return self._has_warnings_as_error

    @property
    def line(self):
        return self._line

    @property
    def column(self):
        return self._column

    @property
    def summary(self):
        return self._summary

    @property
    def text(self):
        return self._text

    @property
    def is_in_project(self):
        if not self._compdb_entry or not self._compdb_entry.project_root:
            raise RuntimeError('project_root not available')
        return Path(self._compdb_entry.project_root) in Path(self._file).parents

    @property
    def is_in_build_dir(self):
        if not self._compdb_entry:
            raise RuntimeError('compdb not available')
        return Path(self._compdb_entry.directory) in Path(self._file).parents

    @property
    def key(self):
        return (self.file, self.line, self.column, self.check)

    @property
    def compdb_entry(self):
        return self._compdb_entry

    def __repr__(self):
        return self._text


def parse_issues_from_output(clangtidy_stdout, compdb_entry=None):
    rx = r'^(?P<file>/[^:]+):(?P<line>\d+):(?P<column>\d+):\s+(?P<summary>[^\n]+)\s+\[(?P<checks>[^ \]]+)\]$'
    ms = [m for m in re.finditer(rx, clangtidy_stdout, flags=re.MULTILINE)]
    return [ClangTidyIssue(**cur.groupdict(),
                           text=clangtidy_stdout[cur.span()[0]:
                                                 next.span()[0] - 1 if next else len(clangtidy_stdout)],
                           compdb_entry=compdb_entry)
            for cur, next in zip_longest(ms, ms[1:])]


class ClangTidyResult:
    def __init__(self, command, stdout, stderr, returncode, issues, runtime, compdb_entry):
        self._command = command
        self._stdout = stdout
        self._stderr = stderr
        self._returncode = returncode
        self._issues = issues
        self._runtime = runtime
        self._compdb_entry = compdb_entry

    @property
    def command(self):
        return self._command

    @property
    def stdout(self):
        return self._stdout

    @property
    def stderr(self):
        return self._stderr

    @property
    def returncode(self):
        return self._returncode

    @property
    def issues(self):
        return self._issues

    @property
    def runtime(self):
        return self._runtime

    @property
    def compdb_entry(self):
        return self._compdb_entry


def run_on_file(compdb_entry):
    compdb_path = Path(compdb_entry.compdb_path)
    if not (compdb_path.is_file() and compdb_path.name == 'compile_commands.json'):
        raise RuntimeError(f'compile_commands.json not found in {compdb_path}')

    start_time = time.time()
    cmd = ['clang-tidy', f'-p={compdb_path}', '-quiet', compdb_entry.file]
    p = subprocess.Popen(cmd,
                         shell=False,
                         cwd=compdb_entry.directory,
                         stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE)
    out, err = [o.decode('utf-8') for o in p.communicate()]

    issues = parse_issues_from_output(out, compdb_entry=compdb_entry)
    return ClangTidyResult(command=cmd,
                           stdout=out,
                           stderr=err,
                           returncode=p.returncode,
                           issues=issues,
                           runtime=time.time() - start_time,
                           compdb_entry=compdb_entry)
