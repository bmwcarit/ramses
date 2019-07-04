#!/usr/bin/python

#  -------------------------------------------------------------------------
#  Copyright (C) 2013 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import re, string
from common_modules.common import *
from common_modules.config import *

def make_license_re(license_text):
    """
    Makes a regular expression for every line in the license, this would match the license
    text tolerating extra spaces
    """

    license_lines = license_text.split("\n")
    license_lines_re = []

    for license_line in license_lines:

        if len(license_line) > 0:
            esc_line = re.escape(license_line)
            re_text = esc_line.replace(r"\[YYYY\]", r"2\d{3}(-2\d{3})?")
            re_text = re_text.replace(r'XXXX', r'(?:Car IT GmbH|AG|Car IT)')
            re_text = r'^(//|::|#)' + re_text + '$'
            license_lines_re.append(re_text)

    return license_lines_re

# generate license regexes only once
G_RE_LICENSE_TEMPLATE_OPEN = make_license_re(G_LICENSE_TEMPLATE_OPEN)
G_RE_LICENSE_TEMPLATE_PROP = make_license_re(G_LICENSE_TEMPLATE_PROP)


def check_specific_license_in_file(filename, clean_file_contents, license_text, license_re):
    """
    Checks if the file contains a valid license according to the license template provided
    """
    license_lines = license_text.split("\n")
    content = clean_file_contents.split("\n")

    for content_idx in range(0, len(content)):
        # search for first line matching license
        if re.match(license_re[0], content[content_idx]):
            for re_idx in range(0, len(license_re)):
                if not re.match(license_re[re_idx], content[content_idx+re_idx]):
                    return False

            return True
    return False


def check_license_for_file(file_name, file_contents, solution_path):
    """
    Check license for given file name. Default ot open except it is in
    G_PROP_FILES list.
    """
    license_template, license_re = G_LICENSE_TEMPLATE_OPEN, G_RE_LICENSE_TEMPLATE_OPEN

    for pattern in G_PROP_FILES:
        full_pattern = os.path.join(solution_path, pattern)
        if file_name.startswith(full_pattern):
            license_template, license_re = G_LICENSE_TEMPLATE_PROP, G_RE_LICENSE_TEMPLATE_PROP
            break

    if not check_specific_license_in_file(file_name, file_contents, license_template, license_re):
        log_warning("check_license_in_file", file_name, 1, "no valid license found")


def main():
    targets = sys.argv[1:]
    targets = get_all_files(targets)

    if len(targets) == 0:
        print("""
\t**** No input provided ****
\tTakes a list of files/directories as input and performs specific style checking on all files/directories.

\tGives warnings if the file does not contain a valid license text. It does not check if Copyright statements are included.
""")
        exit(0)

    path = os.path.realpath(os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", ".."))
    for t in targets:
        file_contents, _ = read_file(t)
        check_license_for_file(t, file_contents, path)

if __name__ == "__main__":
    main()
