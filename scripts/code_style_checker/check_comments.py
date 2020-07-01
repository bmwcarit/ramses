#!/usr/bin/env python3

#  -------------------------------------------------------------------------
#  Copyright (C) 2020 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import re
from common_modules.common import *

def check_doxygen_singleline_comments(filename, file_lines):
    rx = re.compile(r'^(?:\s*///[^<])|(?:.*///<)')
    for i in range(len(file_lines)):
        if "///" in file_lines[i]:
            if not rx.match(file_lines[i]):
                log_warning("check_doxygen_singleline_comments", filename, i + 1, "wrong use of doxygen /// comment found (intended to use ///< ?)", file_lines[i].strip(" "))


if __name__ == "__main__":
    targets = sys.argv[1:]
    targets = get_all_files(targets)

    if len(targets) == 0:
        print("""
\t**** No input provided ****
""")
        exit(0)

    for t in targets:
        if t[-2:] == ".h" or t[-4:] == ".cpp" or t[-2] == ".c":
            _, file_lines = read_file(t)
            check_doxygen_singleline_comments(t, file_lines)
