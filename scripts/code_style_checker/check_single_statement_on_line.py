#!/usr/bin/python

#  -------------------------------------------------------------------------
#  Copyright (C) 2013 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import sys, re, string
from common_modules.common import *

def check_single_statement_on_line(filename, file_contents, clean_file_contents, file_lines, clean_file_lines):
    """
    Checks if there are several statements on the same line

    """
    open_parantheses_count = 0
    semicolon_inside_oneline_lambda_re = re.compile(r'\[.*\].*\{.*;.*\}')

    for i in range(len(file_lines)):
        line = clean_file_lines[i]

        open_parantheses_count += line.count("(") - line.count(")")

        #check if there are several semicolons on the line and that is not in the middle of a for statement
        if line.count(";") > 1 and open_parantheses_count == 0:
            #check that there is NO right parentheses after semicolon
            if line.rfind(";") > line.rfind(")") and not re.search(semicolon_inside_oneline_lambda_re, line):
                log_warning("check_single_statement_on_line", filename, i + 1, "several statements on same line", file_lines[i].strip(" "))

if __name__ == "__main__":
    targets = sys.argv[1:]
    targets = get_all_files(targets)

    if len(targets) == 0:
        print("""
\t**** No input provided ****
\tTakes a list of files/directories as input and performs specific style checking on all files/directories.

\tGives warnings if a line contains several statements.
""")
        exit(0)

    for t in targets:
        if t[-2:] == ".h" or t[-4:] == ".cpp" or t[-2] == ".c":
            file_contents, file_lines = read_file(t)
            clean_file_contents, clean_file_lines = clean_file_content(file_contents)
            check_single_statement_on_line(t, file_contents, clean_file_contents, file_lines, clean_file_lines)
