#!/usr/bin/python

#  -------------------------------------------------------------------------
#  Copyright (C) 2013 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import re
from common_modules.common import *

def check_header_guards(filename, file_contents):
    """
    Check if a header files contains valid header guards:
        1- #ifndef GUARDNAME directly after license that has a guard name identical to file name in uppercase letters, underscores at beginnings and end are tollerable
        2- directly followed by #define that matches the guard name of #ifndef exactly
        3- the file has to end with an #endif

    """
    is_h_file = filename[-2:] == ".h"
    is_hpp_file = filename[-4:] == ".hpp"
    if not (is_h_file or is_hpp_file):
        return

    #remove strings
    string_re = re.compile(r'"((?!((?<!((?<!\\)\\))")).)*"', re.DOTALL)
    file_contents = clean_string_from_regex(file_contents, string_re, 'x')

    #remove multi-line comments
    ml_comment_re = re.compile(r'(\s*)/\*((?!\*/).)*\*/', re.DOTALL)
    file_contents = clean_string_from_regex(file_contents, ml_comment_re, '')

    #remove single line comments
    sl_comment_re = re.compile("//.*$", re.MULTILINE)
    file_contents = re.sub(sl_comment_re, '', file_contents)

    #remove any padding white space characters
    file_contents = file_contents.strip(" \t\n\f\r")

    def check_good_name(name):
        #good name
        good_name = filename

        #remove path from filename (if needed)
        if good_name.count("/") > 0:
            good_name = good_name[good_name.rfind("/") + 1:]
        if good_name.count("\\") > 0:
            good_name = good_name[good_name.rfind("\\") + 1:]

        #remove .h extension
        good_name = good_name[:good_name.find(".h")]

        #good name is all uppercase
        good_name = good_name.upper()

        # macros must not include '-' character, while files can
        # convert '-' to '_', which is valid for C++ preprocessor
        good_name = good_name.replace('-', '_')

        #a good name has format [PREFIX_]FILENAME_H
        good_name_re = re.compile("RAMSES_" + "([a-zA-Z0-9][a-zA-Z0-9]*_)*" + good_name + "_H$")

        valid = re.match(good_name_re, name) != None

        if not valid:
            log_warning("check_header_guards", filename, 1, "header guard ' " + name + "' has invalid format, expected format: 'RAMSES_[PREFIX_]" + good_name + "_H'")

        return valid

    #find the ifndef guard
    ifndef_re = re.compile(r"(\s*)#ifndef .*", re.MULTILINE)
    ifndef_match = re.match(ifndef_re, file_contents)
    if ifndef_match == None:
        log_warning("check_header_guards", filename, 1, "header guard missing")
        return None

    #get guard name
    ifndef_match_text = file_contents[:ifndef_match.end()]
    ifndef_match_text = ifndef_match_text.strip(" \n\r\f\t")
    splitted_match_text = ifndef_match_text.split(" ")
    ifndef_guard_name = splitted_match_text[1] if len(splitted_match_text) > 1 else ""

    if not check_good_name(ifndef_guard_name):
        return None

    file_contents = file_contents[ifndef_match.end():]

    #find the define guard
    define_re = re.compile(r"(\s*)#define .*")
    define_match = re.match(define_re, file_contents)
    if define_match == None:
        log_warning("check_header_guards", filename, 1, "header file does not contain a #define guard at the beginning that matches #ifndef {0}".format(ifndef_guard_name))
        return None

    #get guard name
    define_match_text = file_contents[:define_match.end()]
    define_match_text = define_match_text.strip(" \n\r\f\t")
    splitted_match_text = define_match_text.split(" ")
    define_guard_name = splitted_match_text[1] if len(splitted_match_text) > 1 else ""

    #check if #ifndef and #define have identical guard names
    if define_guard_name != ifndef_guard_name:
        log_warning("check_header_guards", filename, 1, "header guard names do not match :(#ifndef {0}) and (#define {1})".format(ifndef_guard_name, define_guard_name))

    #find the endif guard
    endif_re = re.compile(r"#endif.*(\s*)$")
    endif_match = re.search(endif_re, file_contents)
    if endif_match == None:
        log_warning("check_header_guards", filename, 1, "header file does not end with #endif at last line")
        return None

if __name__ == "__main__":
    targets = sys.argv[1:]
    targets = get_all_files(targets)

    if len(targets) == 0:
        print("""
\t**** No input provided ****
\tTakes a list of files/directories as input and performs specific style checking on all files/directories

\tGives warnings if header guards does not exist in header files. Header files must have a #ifndef and #define guards directly after the license.
\tThe names of the header guards must match together and match with the name of the file in uppercase letters.
""")
        exit(0)

    for t in targets:
        if t[-2:] == ".h":
            file_contents, _ = read_file(t)
            check_header_guards(t, file_contents)
