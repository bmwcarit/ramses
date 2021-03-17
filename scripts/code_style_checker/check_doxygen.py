#!/usr/bin/env python3

#  -------------------------------------------------------------------------
#  Copyright (C) 2021 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import re
import common_modules.common as cc

g_re_header_tag = re.compile(r'^#.*<\s*[a-z]+\s*>')


def check_doxygen(filename, file_contents, file_lines):
    """ Check doxygen files """

    for line_number, line in enumerate(file_lines):
        if g_re_header_tag.search(line):
            cc.log_warning("check_doxygen", filename, line_number + 1,
                           "usage html tag in doxygen header line", file_lines[line_number].strip(" \t\r\n"))
