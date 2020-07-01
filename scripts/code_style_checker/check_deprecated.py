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

g_re_deprecated_mock_syntax = re.compile(r'MOCK_(?:CONST_)?METHOD\d+')

def check_deprecated(filename, file_contents, clean_file_contents, file_lines, clean_file_lines):
    """ Check for usage of deprecated constructs """

    # check for old mock syntax
    for line_number, line in enumerate(clean_file_lines):
        if g_re_deprecated_mock_syntax.search(line):
            log_warning("check_deprecated", filename, line_number + 1, "usage of old googletest mock syntax", file_lines[line_number].strip(" \t\r\n"))

if __name__ == "__main__":
    targets = sys.argv[1:]
    targets = get_all_files(targets)

    if len(targets) == 0:
        exit(1)

    for t in targets:
        if t[-2:] == ".h" or t[-4:] == ".cpp" or t[-2] == ".c":
            file_contents, file_lines = read_file(t)
            clean_file_contents, clean_file_lines = clean_file_content(file_contents)
            check_deprecated(t, file_contents, clean_file_contents, file_lines, clean_file_lines)
