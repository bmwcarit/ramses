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

def check_single_definition_on_line(filename, file_contents, clean_file_contents, file_lines, clean_file_lines):
    """
    Checks if a line contains several variable definitions

    """
    #remove arrays
    #an array regex starts with "{", ends with "}" and contains anything in between
    #an array of arrays is removed recursively inside out
    array_re = re.compile(r'\{[^ {};]*\}')
    clean_file_contents = clean_string_from_regex(clean_file_contents, array_re, '')

    #remove brackets
    array_brackets_re = re.compile(r'\[((?!\[|\]|;).)*\]')
    clean_file_contents = clean_string_from_regex(clean_file_contents, array_brackets_re, '')

    #remove angle brackets
    template_re = re.compile(r'<((?![<>;]).)*>')
    clean_file_contents = clean_string_from_regex(clean_file_contents, template_re, '')

    #variable name can be preceeded by * for pointers or & for references, can be followed by an assignment
    var_decl_re_text = r"""(
                                (?:(?:&|\*+)\s*)?               # can start by & (for reference defs) and/or any number of * (for pointer defs)
                                \w+                             # followed by identifier
                                (?:\s*\([\w\s,]*\))?            # can be followed by constructor call
                                (?:\s*=\w+\s*|\s*\{[\w\s,]*\})? # or can be followed by an assignment of simple values, or value lists (for arrays)
                            )"""

    #types can have qualifiers, be nested or in namespaces
    type_re_text = r"(?:const\s+|volatile\s+)?\w+(?:::\w+)*(?:&?|\**)"

    #it is enough to have
    several_defs_re_text = r"""
                                \b\s*{0}
                                \s+{1}
                                (?:\s*,\s*{1})+;
                            """.format(type_re_text, var_decl_re_text)

    several_defs_re = re.compile(several_defs_re_text, re.MULTILINE | re.VERBOSE)

    inside_for_header_re = re.compile(r"^\s*for \s*\(")

    for match in re.finditer(several_defs_re, clean_file_contents):
        line_number = clean_file_contents.count("\n", 0, match.end())
        if re.search(inside_for_header_re, file_lines[line_number]) == None:
            log_warning("check_single_definition_on_line", filename, line_number + 1, "several definitions on same line", file_lines[line_number].strip(" \t\r\n"))

if __name__ == "__main__":
    targets = sys.argv[1:]
    targets = get_all_files(targets)

    if len(targets) == 0:
        print("""
\t**** No input provided ****
\tTakes a list of files/directories as input and performs specific style checking on all files/directories.

\tGives warnings if a line contains several variable definitions (This does not include lines that contain template or array declarations).
""")
        exit(0)

    for t in targets:
        if t[-2:] == ".h" or t[-4:] == ".cpp" or t[-2] == ".c":
            file_contents, file_lines = read_file(t)
            clean_file_contents, clean_file_lines = clean_file_content(file_contents)
            check_single_definition_on_line(t, file_contents, clean_file_contents, file_lines, clean_file_lines)
