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

def check_tabs_no_spaces(filename, file_lines):
    """
    Checks if any tab character "\t" exists in the file

    """
    for i in range(len(file_lines)):
        if file_lines[i].count("\t") > 0:
            log_warning("check_tabs_no_spaces", filename, i + 1, "tab character (\\t) found, use 4 spaces instead", file_lines[i].strip(" "))

def check_correct_space_count(filename, file_lines):
    """
    Checks that the number of spaces at the beginning of every line is divisible by four

    """
    previous_indent = 0
    in_multiline_comment = False
    valid_indents_stack = []
    for i in range(len(file_lines)):
        if in_multiline_comment:
            #if end of muli-line comment
            if file_lines[i].count("*/") > 0:
                #just make sure there is no other new multi-line comment starting on the same line (after the current mult-line comment is closed)
                in_multiline_comment = file_lines[i].count("/*") > 0 and file_lines[i].index("/*") > file_lines[i].index("*/")
        else:
            in_multiline_comment = file_lines[i].count("/*") > 0

            #regex searches for the first character that is not a space character
            found_match = re.search("(?! )", file_lines[i])
            if found_match:
                space_count = found_match.start()

                while space_count > 0 and valid_indents_stack and valid_indents_stack[-1] > space_count:
                    valid_indents_stack.pop()

                # check by indent stack
                if space_count in valid_indents_stack and space_count == valid_indents_stack[-1]:
                    ok_by_stack = True
                else:
                    ok_by_stack = False

                # check divisible by 4 OR same indent as previous line or at least properly +-4*x from previous line
                warned = False
                if not ok_by_stack and (space_count % 4 != 0) and (previous_indent != space_count) and (abs(previous_indent - space_count) % 4 != 0):

                    # allow indent to parenthesis in previous line
                    is_smart_indent = False
                    if (i > 0) and (space_count > 0) and (len(file_lines[i-1]) > space_count-1):
                        prev_line = file_lines[i-1]
                        cur_line_without_spaces = file_lines[i][space_count:]
                        if prev_line[space_count-1] in ['(', '['] or prev_line[space_count-4] in ['(', '[']:
                            is_smart_indent = True
                        elif prev_line.endswith(",") or prev_line.endswith("<<") or prev_line.endswith("||") or prev_line.endswith("&&") or \
                             cur_line_without_spaces.startswith("<<") or cur_line_without_spaces.startswith(">>"):
                            is_smart_indent = True

                    if not is_smart_indent:
                        log_warning("check_correct_space_count", filename, i + 1, "number of spaces at beginning of line must be divisible by 4 (or conform to smart indent)")
                        warned = True
                # only previous spaces when not zero
                if space_count != 0:
                    # add to stack when valid and not already in
                    if not warned:
                        if not valid_indents_stack:
                            valid_indents_stack.append(space_count)
                        elif valid_indents_stack[-1] != space_count:
                            valid_indents_stack.append(space_count)
                    previous_indent = space_count
            else:
                previous_indent = 0


def check_no_spacing_line_end(filename, file_lines):
    """
    Checks that lines do not end with unnecessary white space characters

    """
    for i in range(len(file_lines)):
        if re.search(" $", file_lines[i]):
            log_warning("check_no_spacing_line_end", filename, i + 1, "unneeded space(s) at end of line")

def check_tabbing_and_spacing(filename, file_lines):
    """
    Calls other functions that check general issues about tabbing and spacing

    """
    check_tabs_no_spaces(filename, file_lines)
    check_correct_space_count(filename, file_lines)
    check_no_spacing_line_end(filename, file_lines)


if __name__ == "__main__":
    targets = sys.argv[1:]
    targets = get_all_files(targets)

    if len(targets) == 0:
        print("""
\t**** No input provided ****
\tTakes a list of files/directories as input and performs specific style checking on all files/directories.

\tGives warnings if a line contains unneeded spaces at end of line, contains tab characters (\\t)
\tor if spaces at line beginning are not divisible by 4.
""")
        exit(0)

    for t in targets:
        if t[-2:] == ".h" or t[-4:] == ".cpp" or t[-2] == ".c":
            _, file_lines = read_file(t)
            check_tabbing_and_spacing(t, file_lines)
