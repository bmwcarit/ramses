#!/usr/bin/env python3

#  -------------------------------------------------------------------------
#  Copyright (C) 2020 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import re
import sys
from common_modules import common

g_re_deprecated_mock_syntax = re.compile(r'MOCK_(?:CONST_)?METHOD\d+')
g_re_unwanted_gtest_include = re.compile(r'#\s*include\s*["<]gtest/(?!gtest\.h[">])')
g_re_unwanted_gmock_include = re.compile(r'#\s*include\s*["<]gmock/(?!gmock\.h[">])')
g_re_unwanted_fmt_include = re.compile(r'#\s*include\s*["<]fmt/(?!(?:format|chrono)\.h[">])')
g_re_unwanted_system_include = re.compile(r'#\s*include\s*"[^\.]+"')

g_max_line_length = 350


def check_deprecated(filename, file_contents, clean_file_contents, file_lines, clean_file_lines):
    """ Check for usage of deprecated constructs """

    # check for old mock syntax
    for line_number, line in enumerate(clean_file_lines):
        if g_re_deprecated_mock_syntax.search(line):
            common.log_warning("check_deprecated", filename, line_number + 1,
                               "usage of old googletest mock syntax", file_lines[line_number].strip(" \t\r\n"))

    # check for unwanted includes includes
    for line_number, line in enumerate(file_lines):
        if g_re_unwanted_gtest_include.search(line):
            common.log_warning("check_deprecated", filename, line_number + 1,
                               "usage of non-standard gtest include, use '#include \"gtest/gtest.h\"' instead", file_lines[line_number].strip(" \t\r\n"))
        if g_re_unwanted_gmock_include.search(line):
            common.log_warning("check_deprecated", filename, line_number + 1,
                               "usage of non-standard gmock include, use '#include \"gmock/gmock.h\"' instead", file_lines[line_number].strip(" \t\r\n"))
        if g_re_unwanted_fmt_include.search(line):
            common.log_warning("check_deprecated", filename, line_number + 1,
                               "usage of unwanted fmt include, use '#include \"fmt/format.h\"' instead", file_lines[line_number].strip(" \t\r\n"))

        if g_re_unwanted_system_include.search(line):
            common.log_warning("check_deprecated", filename, line_number + 1,
                               'found system include with "...", use <...> instead"', file_lines[line_number].strip(" \t\r\n"))

        # TODO: Fix offenders and reduce limit until some reasonable length is reached
        if len(line) > g_max_line_length:
            common.log_warning("check_deprecated", filename, line_number + 1,
                               f'line of {len(line)} characters too long (max allowed {g_max_line_length}), add some linebreaks ',
                               file_lines[line_number].strip(" \t\r\n"))


if __name__ == "__main__":
    targets = sys.argv[1:]
    targets = common.get_all_files(targets)

    if len(targets) == 0:
        exit(1)

    for t in targets:
        if t[-2:] == ".h" or t[-4:] == ".cpp" or t[-2] == ".c":
            file_contents, file_lines = common.read_file(t)
            clean_file_contents, clean_file_lines = common.clean_file_content(file_contents)
            check_deprecated(t, file_contents, clean_file_contents, file_lines, clean_file_lines)
