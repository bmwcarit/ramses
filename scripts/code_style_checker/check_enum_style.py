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


def check_enum_style(filename, clean_file_contents):
    """
    Check that enums have
    - E prefix on name if old enum
    - name is CamelCase
    - enumerators have name prefix if old enum
    - enumerators have no name prefix if enum class
    """

    # enum [class] <name> [: maybe type] { <values> }
    enum_re = re.compile(r'(?<!\w)enum\s+(class\s+)?(\w+)\s*(?::\s*\w+\s*])?{((\s|\S)*?)}')

    for enum_match in re.finditer(enum_re, clean_file_contents):
        line_number = clean_file_contents[:enum_match.start()].count("\n")

        g = enum_match.groups()
        is_enum_class = g[0] is not None
        enum_name = g[1]
        enum_values = [l for l in [l.strip() for l in g[2].split('\n')] if l is not '']

        # old enum must start with an 'E'
        if not is_enum_class and not enum_name.startswith('E'):
            log_warning("check_enum_style", filename, line_number, "enum must begin with 'E': " + enum_name)
        # must be camel case
        if enum_name.upper() == enum_name:
            log_warning("check_enum_style", filename, line_number, "enum must be CamelCase: " + enum_name)

        # old enum has prefix on values, enum class does NOT have name prefix
        for v in enum_values:
            if is_enum_class:
                if v.startswith(enum_name):
                    log_warning("check_enum_style", filename, line_number, "enum class value may not begin with " + enum_name + ": " + v)
            else:
                if not v.startswith(enum_name):
                    log_warning("check_enum_style", filename, line_number, "enum value must begin with " + enum_name + "_ : " + v)

if __name__ == "__main__":
    targets = sys.argv[1:]
    targets = get_all_files(targets)

    if len(targets) == 0:
        print("""
\t**** No input provided ****
\tTakes a list of files/directories as input and performs specific style checking on all files/directories.

\tGives warnings if a file contains more than one class definition or if class name is not identical to file name
""")
        exit(0)

    for t in targets:
        clean_file_contents, _ = clean_file_content(read_file(t)[0])
        check_enum_style(t, clean_file_contents)
