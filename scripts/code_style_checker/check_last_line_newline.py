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


def check_last_line_newline(filename, file_lines):
    """
    Checks if file ends in newline
    """
    if len(file_lines) > 0 and file_lines[-1] != "\n":
        log_warning("check_last_line_newline", filename, len(file_lines),
                    "no newline at end of file")

if __name__ == "__main__":
    targets = sys.argv[1:]
    targets = get_all_files(targets)

    if len(targets) == 0:
        print("""
\t**** No input provided ****
\tTakes a list of files/directories as input and performs specific style checking on all files/directories.

\tGives warnings if no newline at end of file
""")
        exit(0)

    for t in targets:
        _, file_lines = read_file(t)
        check_last_line_newline(t, file_lines)
