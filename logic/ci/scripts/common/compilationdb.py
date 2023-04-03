#  -------------------------------------------------------------------------
#  Copyright (C) 2020 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import json
import re
from pathlib import Path


class CompilationEntry:
    def __init__(self, compdb_entry, compdb_path, project_root=None):
        self._directory = compdb_entry['directory']
        self._command = compdb_entry['command']
        self._abs_file = compdb_entry['file']
        self._project_root = project_root
        self._compdb_path = compdb_path

        assert Path(self._directory).is_absolute()
        assert Path(self._abs_file).is_absolute()
        if self._project_root:
            assert Path(self._project_root).is_absolute()
        assert Path(compdb_path).is_file()

    @property
    def directory(self):
        return self._directory

    @property
    def command(self):
        return self._command

    @property
    def file(self):
        return self._abs_file

    @property
    def relative_file(self):
        if not self._project_root:
            raise RuntimeError('project_root not set')
        return str(Path(self.file).relative_to(self._project_root))

    @property
    def includes(self):
        return re.findall(r'-I([^ "]+)', self.command)

    @property
    def absolute_includes(self):
        return [str((Path(self.directory) / i).resolve()) for i in self.includes]

    @property
    def system_includes(self):
        return re.findall(r'-isystem\s+([^ "]+)', self.command)

    @property
    def absolute_system_includes(self):
        return [str((Path(self.directory) / i).resolve()) for i in self.system_includes]

    @property
    def defines(self):
        return re.findall(r'\s-D([^ "]+)', self.command)

    @property
    def output_file(self):
        m = re.search(r'\s-o\s+([^ "]+)', self.command)
        if not m:
            raise RuntimeError('output_file not found')
        return m.group(1)

    @property
    def absolute_output_file(self):
        return str((Path(self.directory) / self.output_file).resolve())

    @property
    def compdb_path(self):
        return self._compdb_path

    @property
    def project_root(self):
        return self._project_root


def load_from_file(path, project_root=None):
    with open(path, 'r') as f:
        return [CompilationEntry(e, path, project_root) for e in json.load(f)]
