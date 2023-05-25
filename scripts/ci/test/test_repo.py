#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import sys
import os
import pytest
from pathlib import Path
import subprocess

sys.path.append(os.path.realpath(os.path.dirname(__file__) + "/../common"))
import repo  # noqa E402 module level import not at top of file

HERE = Path(os.path.realpath(os.path.dirname(__file__)))


class TestGetSdkroot:
    def test_get_sdkroot(self):
        p = repo.get_sdkroot()
        assert Path(p).is_absolute()
        assert Path(p) == (HERE / '..' / '..' / '..').resolve()

    def test_get_sdkroot_from_path(self):
        p = repo.get_sdkroot(HERE / '..')
        assert Path(p).is_absolute()
        assert Path(p) == (HERE / '..' / '..' / '..').resolve()

    def test_get_sdkroot_fails_when_no_repo(self):
        with pytest.raises(subprocess.CalledProcessError):
            repo.get_sdkroot('/tmp')


class TestGetGitFiles:
    def test_git_get_files_no_submodule(self):
        files = repo.get_git_files(HERE, including_submodules=False, relative_to_path=True, make_absolute=False)
        assert len(files) > 0
        assert not Path(files[0]).is_absolute()
        assert (HERE / files[0]).is_file()

    def test_git_get_files_relative_to_repo(self):
        root = Path(repo.get_sdkroot())
        files = repo.get_git_files(HERE, including_submodules=False, relative_to_path=False, make_absolute=False)
        assert len(files) > 0
        assert not Path(files[0]).is_absolute()
        assert (root / files[0]).is_file()

    def test_git_get_files_make_absolute(self):
        files = repo.get_git_files(HERE, including_submodules=False, relative_to_path=True, make_absolute=True)
        assert len(files) > 0
        assert Path(files[0]).is_absolute()
        assert Path(files[0]).is_file()

    def test_git_get_files_relative_to_repo_and_make_absolute(self):
        files = repo.get_git_files(HERE, including_submodules=False, relative_to_path=False, make_absolute=True)
        assert len(files) > 0
        assert Path(files[0]).is_absolute()
        assert Path(files[0]).is_file()

    def test_git_get_files_with_submodule(self):
        root = repo.get_sdkroot()
        files_with = repo.get_git_files(root, including_submodules=True, relative_to_path=True, make_absolute=False)
        files_without = repo.get_git_files(root, including_submodules=False, relative_to_path=True, make_absolute=False)
        assert len(files_with) > len(files_without)

    def test_git_get_files_fails_outside_repo(self):
        with pytest.raises(subprocess.CalledProcessError):
            repo.get_git_files('/tmp', including_submodules=False, relative_to_path=True, make_absolute=True)
