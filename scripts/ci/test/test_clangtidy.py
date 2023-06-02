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
import json
import tempfile

sys.path.append(os.path.realpath(os.path.dirname(__file__) + "/../common"))
import clangtidy  # noqa E402 module level import not at top of file
import compilationdb  # noqa E402 module level import not at top of file


class TestClangTidyIssueParsing:
    def test_can_parse_empty(self):
        assert [] == clangtidy.parse_issues_from_output("")
        assert [] == clangtidy.parse_issues_from_output("   ")
        assert [] == clangtidy.parse_issues_from_output("\n\n\n")

    def test_can_parse_unrealted_text(self):
        assert [] == clangtidy.parse_issues_from_output("fdsfsdfsd")
        assert [] == clangtidy.parse_issues_from_output("fdsfsdf\nfsdfs")
        assert [] == clangtidy.parse_issues_from_output(
            "/usr/bin/../lib/gcc/x86_64-linux-gnu/10/../../../../include/c++/10/bits/move.h:101:5: note: resolves to this declaration\n")

    def test_can_single_parse_entry(self):
        entry = """
/home/foobar/ramses/framework/Ramsh/src/RamshStandardSetup.cpp:10:1: error: #includes are not sorted properly [llvm-include-order]
#include "Ramsh/RamshCommunicationChannelDLT.h"
^        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
         "Ramsh/RamshCommunicationChannelConsole.h"
"""
        result = clangtidy.parse_issues_from_output(entry)
        assert len(result) == 1
        assert result[0].file == '/home/foobar/ramses/framework/Ramsh/src/RamshStandardSetup.cpp'
        with pytest.raises(RuntimeError):
            result[0].relative_file
        assert result[0].check == 'llvm-include-order'
        assert result[0].all_checks == ['llvm-include-order']
        assert result[0].has_warnings_as_error is False
        assert result[0].line == 10
        assert result[0].column == 1
        assert result[0].summary == 'error: #includes are not sorted properly'
        assert result[0].text == entry.strip() + '\n'
        assert result[0].key == ('/home/foobar/ramses/framework/Ramsh/src/RamshStandardSetup.cpp', 10, 1, 'llvm-include-order')
        assert result[0].compdb_entry is None
        assert str(result[0]) == result[0].text

    def test_can_handle_multiline_entry(self):
        entry = """/home/foobar/ramses/framework/Ramsh/src/RamshStandardSetup2.cpp:17:20: error: 'move<ramses_internal::String &>' must resolve to a function declared within the '__llvm_libc' namespace [llvmlibc-callee-namespace]
        , m_prompt(std::move(prompt))
                   ^
/usr/bin/../lib/gcc/x86_64-linux-gnu/10/../../../../include/c++/10/bits/move.h:101:5: note: resolves to this declaration
    move(_Tp&& __t) noexcept
^"""  # noqa E501 allow long string
        result = clangtidy.parse_issues_from_output(entry)
        assert len(result) == 1
        assert result[0].file == '/home/foobar/ramses/framework/Ramsh/src/RamshStandardSetup2.cpp'
        with pytest.raises(RuntimeError):
            result[0].relative_file
        assert result[0].check == 'llvmlibc-callee-namespace'
        assert result[0].all_checks == ['llvmlibc-callee-namespace']
        assert result[0].has_warnings_as_error is False
        assert result[0].line == 17
        assert result[0].column == 20
        assert result[0].summary == "error: 'move<ramses_internal::String &>' must resolve to a function declared within the '__llvm_libc' namespace"
        assert result[0].text == entry + '\n'
        assert result[0].key == ('/home/foobar/ramses/framework/Ramsh/src/RamshStandardSetup2.cpp', 17, 20, 'llvmlibc-callee-namespace')
        assert result[0].compdb_entry is None
        assert str(result[0]) == result[0].text

    def test_can_handle_multiple_checks(self):
        entry = """/home/foobar/ramses/framework/Ramsh/src/RamshStandardSetup.cpp:45:1: error: namespace 'ramses_internal' not terminated with a closing comment [google-readability-namespace-comments,llvm-namespace-comment]"""  # noqa E501 allow long string
        result = clangtidy.parse_issues_from_output(entry)
        assert len(result) == 1
        assert result[0].check == 'google-readability-namespace-comments'
        assert result[0].all_checks == ['google-readability-namespace-comments', 'llvm-namespace-comment']

    def test_can_parse_multiple_entries(self):
        entries = """
/home/foobar/ramses/framework/Ramsh/src/RamshStandardSetup.cpp:10:1: error: #includes are not sorted properly [llvm-include-order]
#include "Ramsh/RamshCommunicationChannelDLT.h"
^        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
         "Ramsh/RamshCommunicationChannelConsole.h"
/home/foobar/ramses/framework/Ramsh/src/RamshStandardSetup.cpp:13:11: error: '__llvm_libc' needs to be the outermost namespace [llvmlibc-implementation-in-namespace]
namespace ramses_internal
          ^
"""  # noqa E501 allow long string
        result = clangtidy.parse_issues_from_output(entries)
        assert len(result) == 2
        assert result[0].file == '/home/foobar/ramses/framework/Ramsh/src/RamshStandardSetup.cpp'
        assert result[0].check == 'llvm-include-order'
        assert result[0].line == 10
        assert result[0].column == 1
        assert result[0].text == """/home/foobar/ramses/framework/Ramsh/src/RamshStandardSetup.cpp:10:1: error: #includes are not sorted properly [llvm-include-order]
#include "Ramsh/RamshCommunicationChannelDLT.h"
^        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
         "Ramsh/RamshCommunicationChannelConsole.h"
"""  # noqa E501 allow long string
        assert result[1].file == '/home/foobar/ramses/framework/Ramsh/src/RamshStandardSetup.cpp'
        assert result[1].check == 'llvmlibc-implementation-in-namespace'
        assert result[1].line == 13
        assert result[1].column == 11
        assert result[1].text == """/home/foobar/ramses/framework/Ramsh/src/RamshStandardSetup.cpp:13:11: error: '__llvm_libc' needs to be the outermost namespace [llvmlibc-implementation-in-namespace]
namespace ramses_internal
          ^
"""  # noqa E501 allow long string

    def test_can_handles_warnings_as_error(self):
        entries = """/home/foobar/ramses/framework/Ramsh/src/RamshStandardSetup.cpp:10:1: error: #includes are not sorted properly [llvm-include-order,-warnings-as-errors]
/home/foobar/ramses/framework/Ramsh/src/RamshStandardSetup.cpp:45:1: error: namespace 'ramses_internal' not terminated with a closing comment [foobar]
/home/foobar/ramses/framework/Ramsh/src/RamshStandardSetup.cpp:45:1: error: namespace 'ramses_internal' not terminated with a closing comment [google-readability-namespace-comments,llvm-namespace-comment,-warnings-as-errors]
"""  # noqa E501 allow long string
        result = clangtidy.parse_issues_from_output(entries)
        assert len(result) == 3
        assert result[0].check == 'llvm-include-order'
        assert result[0].all_checks == ['llvm-include-order', '-warnings-as-errors']
        assert result[0].has_warnings_as_error is True

        assert result[1].check == 'foobar'
        assert result[1].all_checks == ['foobar']
        assert result[1].has_warnings_as_error is False

        assert result[2].check == 'google-readability-namespace-comments'
        assert result[2].all_checks == ['google-readability-namespace-comments', 'llvm-namespace-comment', '-warnings-as-errors']
        assert result[2].has_warnings_as_error is True

    def test_can_use_compdb(self):
        entry = """/home/foobar/ramses/framework/Ramsh/src/RamshStandardSetup.cpp:45:1: error: namespace 'ramses_internal' not terminated with a closing comment [google-readability-namespace-comments,llvm-namespace-comment]"""  # noqa E501 allow long string
        compdb_entry = compilationdb.CompilationEntry({
            'directory': '/home/foobar/ramses/build',
            'command': 'runme',
            'file': '/home/foobar/ramses/framework/Ramsh/src/RamshStandardSetup.cpp'
        }, compdb_path=__file__)
        result = clangtidy.parse_issues_from_output(entry, compdb_entry=compdb_entry)
        assert len(result) == 1
        assert result[0].compdb_entry == compdb_entry
        with pytest.raises(RuntimeError):
            result[0].relative_file

    def test_can_use_compdb_with_project_root(self):
        entry = """/home/foobar/ramses/framework/Ramsh/src/RamshStandardSetup.cpp:45:1: error: namespace 'ramses_internal' not terminated with a closing comment [google-readability-namespace-comments,llvm-namespace-comment]"""  # noqa E501 allow long string
        compdb_entry = compilationdb.CompilationEntry({
            'directory': '/home/foobar/ramses/build',
            'command': 'runme',
            'file': '/home/foobar/ramses/framework/Ramsh/src/RamshStandardSetup.cpp'
        }, compdb_path=__file__, project_root='/home/foobar/ramses/')
        result = clangtidy.parse_issues_from_output(entry, compdb_entry=compdb_entry)
        assert len(result) == 1
        assert result[0].compdb_entry == compdb_entry
        assert result[0].relative_file == 'framework/Ramsh/src/RamshStandardSetup.cpp'

    def test_resolved_file_paths(self):
        entry = """/home/foobar/ramses/build/../framework/Ramsh/src/RamshStandardSetup.cpp:45:1: error: namespace 'ramses_internal' not terminated with a closing comment [google-readability-namespace-comments]"""  # noqa E501 allow long string
        result = clangtidy.parse_issues_from_output(entry)
        assert len(result) == 1
        assert result[0].file == '/home/foobar/ramses/framework/Ramsh/src/RamshStandardSetup.cpp'

    def test_project_and_builddir_predicates(self):
        entry = """/home/foobar/ramses/framework/file.h:45:1: error: namespace 'ramses_internal' not terminated with a closing comment [google-readability-namespace-comments]
/home/foobar/src/other.h:45:1: error: namespace 'ramses_internal' not terminated with a closing comment [google-readability-namespace-comments]
/home/foobar/ramses/build/dir/third.h:45:1: error: namespace 'ramses_internal' not terminated with a closing comment [google-readability-namespace-comments]"""  # noqa E501 allow long string
        compdb_entry = compilationdb.CompilationEntry({
            'directory': '/home/foobar/ramses/build',
            'command': 'runme',
            'file': '/home/foobar/ramses/framework/Ramsh/src/RamshStandardSetup.cpp'
        }, compdb_path=__file__, project_root='/home/foobar/ramses/')
        result = clangtidy.parse_issues_from_output(entry, compdb_entry=compdb_entry)
        assert len(result) == 3

        assert result[0].relative_file == 'framework/file.h'
        assert result[0].is_in_project is True
        assert result[0].is_in_build_dir is False

        assert result[1].file == '/home/foobar/src/other.h'
        assert result[1].is_in_project is False
        assert result[1].is_in_build_dir is False

        assert result[2].relative_file == 'build/dir/third.h'
        assert result[2].is_in_project is True
        assert result[2].is_in_build_dir is True

    def test_can_parse_entry_with_brackets_in_summary(self):
        entry = """
/home/foobar/ramses/framework/Ramsh/src/RamshStandardSetup.cpp:10:1: error: should be marked [[nodiscard]] [modernize-use-nodiscard]
        sceneObjectId_t getSceneObjectId() const;
        ^
        [[nodiscard]]
"""
        result = clangtidy.parse_issues_from_output(entry)
        assert len(result) == 1
        assert result[0].all_checks == ['modernize-use-nodiscard']
        assert result[0].line == 10
        assert result[0].column == 1
        assert result[0].summary == 'error: should be marked [[nodiscard]]'
        assert result[0].text == entry.strip() + '\n'

    def test_can_parse_checks_with_dots(self):
        entry = """
/home/foobar/ramses/framework/Ramsh/src/RamshStandardSetup.cpp:10:1: error: foo [clang-analyzer-cplusplus.NewDeleteLeaks,-warnings-as-errors]
"""
        result = clangtidy.parse_issues_from_output(entry)
        assert len(result) == 1
        assert result[0].all_checks == ['clang-analyzer-cplusplus.NewDeleteLeaks', '-warnings-as-errors']
        assert result[0].text == entry.strip() + '\n'


class TestClangTidyRun:
    def write_compdb(self, dir, files):
        db = [{'directory': dir,
               'command': f'clang++ -c {dir}/{f} -o {dir}/{f}.o',
               'file': f'{dir}/{f}'}
              for f in files]
        db_file = Path(dir) / 'compile_commands.json'
        db_file.write_text(json.dumps(db))
        return compilationdb.load_from_file(db_file)

    def write_clang_tidy_config(self, dir, warnings_as_errors=False):
        content = "Checks: 'google-readability-todo, llvm-namespace-comment'\n"
        if warnings_as_errors:
            content += "WarningsAsErrors: '*'\n"
        (Path(dir) / '.clang-tidy').write_text(content)

    def write_cpp(self, dir, file):
        (Path(dir) / file).write_text("""
// TODO this is a test todo for clang-tidy and not a real todo!
namespace a{
}
#
""")

    def test_run_on_file(self):
        with tempfile.TemporaryDirectory() as d:
            self.write_clang_tidy_config(d)
            self.write_cpp(d, 'foo.cpp')
            db = self.write_compdb(d, ['foo.cpp'])
            r = clangtidy.run_on_file(db[0])
        assert 'llvm-namespace-comment' in r.stdout
        assert 'warnings generated.' in r.stderr
        assert r.returncode == 0
        assert r.compdb_entry == db[0]
        assert 'clang-tidy' in r.command
        assert r.runtime > 0
        assert len(r.issues) == 2
        assert ['google-readability-todo', 'llvm-namespace-comment'] == [i.check for i in r.issues]

    def test_run_on_file_with_warning_as_errors(self):
        with tempfile.TemporaryDirectory() as d:
            self.write_clang_tidy_config(d, warnings_as_errors=True)
            self.write_cpp(d, 'foo.cpp')
            db = self.write_compdb(d, ['foo.cpp'])
            r = clangtidy.run_on_file(db[0])
        assert 'llvm-namespace-comment' in r.stdout
        assert 'warnings generated.' in r.stderr
        assert r.returncode == 1
        assert r.compdb_entry == db[0]
        assert 'clang-tidy' in r.command
        assert r.runtime > 0
        assert len(r.issues) == 2
        assert ['google-readability-todo', 'llvm-namespace-comment'] == [i.check for i in r.issues]

    def test_run_on_file_exception_when_no_compile_commands(self):
        with tempfile.TemporaryDirectory() as d:
            self.write_clang_tidy_config(d)
            self.write_cpp(d, 'foo.cpp')
            db = self.write_compdb(d, ['foo.cpp'])
            (Path(d) / 'compile_commands.json').unlink()
            with pytest.raises(RuntimeError):
                clangtidy.run_on_file(db[0])

    def test_run_on_file_without_config_does_nothing(self):
        with tempfile.TemporaryDirectory() as d:
            self.write_cpp(d, 'foo.cpp')
            db = self.write_compdb(d, ['foo.cpp'])
            r = clangtidy.run_on_file(db[0])
            assert not r.stdout
            assert not r.stderr
            assert r.returncode == 0
            assert not r.issues
