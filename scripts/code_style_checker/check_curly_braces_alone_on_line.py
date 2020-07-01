#!/usr/bin/env python3

#  -------------------------------------------------------------------------
#  Copyright (C) 2013 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import sys, re, string
from common_modules.common import *


def check_curly_braces_alone_on_line(filename, file_contents, clean_file_contents, file_lines, clean_file_lines):
    """
    Checks that there is no unnecessary code on same line with curly braces.
    """

    # if: not at end
    re_if = re.compile(r'\W+if\s*\(.*\)\s*(?:\{|.*\}|.*;)$')

    # else: neither before nor after
    re_else = re.compile(r'}\s*else|else\s*{')

    # switch: not at end
    re_switch = re.compile(r'\W+switch\s*\(.*\)\s*(?:\{|.*\}|.*;)$')

    # while: before ok, not after
    re_while = re.compile(r'\W+while\s*\(.*\)\s*{')

    # for: not at end
    re_for = re.compile(r'\W+for\s*\(.*\)\s*(?:\{|.*\}|.*\)\s*;)$')

    # NOTE(tobias) not checking function definitions because too many false positives with lambdas, enums ...

    for i in range(len(clean_file_lines)):
        line = clean_file_lines[i]

        if re_if.search(line):
            log_warning("check_curly_braces_alone_on_line", filename, i + 1, "curly brace or statement on same line with if", file_lines[i].strip(" "))
        elif re_else.search(line):
            log_warning("check_curly_braces_alone_on_line", filename, i + 1, "curly brace on same line with else", file_lines[i].strip(" "))
        elif re_switch.search(line):
            log_warning("check_curly_braces_alone_on_line", filename, i + 1, "curly brace or statement on same line with switch", file_lines[i].strip(" "))
        elif re_while.search(line):
            log_warning("check_curly_braces_alone_on_line", filename, i + 1, "curly brace after while", file_lines[i].strip(" "))
        elif re_for.search(line):
            log_warning("check_curly_braces_alone_on_line", filename, i + 1, "curly brace or statement on same line with for", file_lines[i].strip(" "))


if __name__ == "__main__":
    targets = sys.argv[1:]
    targets = get_all_files(targets)

    if len(targets) == 0:
        print("""
\t**** No input provided ****
\tTakes a list of files/directories as input and performs specific style checking on all files/directories

\tGives warnings if unnecessary code exists on the same line with curly braces.
""")
        exit(0)

    for t in targets:
        if t[-2:] == ".h" or t[-4:] == ".cpp" or t[-2] == ".c":
            file_contents, file_lines = read_file(t)
            clean_file_contents, clean_file_lines = clean_file_content(file_contents)
            check_curly_braces_alone_on_line(t, file_contents, clean_file_contents, file_lines, clean_file_lines)
