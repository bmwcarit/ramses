#!/usr/bin/python

#  -------------------------------------------------------------------------
#  Copyright (C) 2013 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

"""
Contains utility functions used by other modules
"""

import re
import sys
import os
import subprocess
import common_modules.config

def clean_string_from_regex(s, regex, marker):
    """
    Removes all occurances of the passed regex from a string s, and puts instances of marker at the beginnings and ends
    of remved substring

    """
    result = s
    found_match = re.search(regex, result)

    while found_match != None:
        matched_string = result[found_match.start(): found_match.end()]
        #replace by x...x where the ... is the \n chars in the matched string (to keep number of lines constant)
        result = result[:found_match.start()] + marker + "\n" * matched_string.count("\n") + marker + result[found_match.end():]

        found_match = re.search(regex, result)

    return result

def get_clean_file_contents(file_contents):
    """
    Removes strings, C style and C++ comments style comments and macros from file_contents

    """

    clean_file_contents = file_contents

    #remove all strings
    #a string is something that starts with a (") and ends with a ("), can have any character inside it except a (") that is not preceded by a slash (\) that is not preceded by another slash
    #this means a string can contains a \\ escape sequence
    #a string can contain a \" escape sequence
    #a string can end with  \\", but it does not end with \"
    string_re = re.compile(r'"((?!((?<!((?<!\\)\\))")).)*"', re.DOTALL)

    #'x' is passed as a marker, to mark the beginning and of strings
    #to preserve spacing around string beginning and end
    clean_file_contents = clean_string_from_regex(clean_file_contents, string_re, 'x')

    #remove character literals
    #a character literal is defined like a string (replacing single quotes with double quotes)
    #this is a big relaxation of the problem for C/C++
    char_re = re.compile(r'\'((?!((?<!((?<!\\)\\))\')).)*\'')

    clean_file_contents = clean_string_from_regex(clean_file_contents, char_re, 'x')

    #remove multi-line comments
    ml_comment_re = re.compile(r'(\s*)/\*((?!\*/).)*\*/', re.DOTALL)
    clean_file_contents = clean_string_from_regex(clean_file_contents, ml_comment_re, '')

    #remove single line comments
    sl_comment_re = re.compile(r'[ \t\r\f]*//.*$', re.MULTILINE)
    clean_file_contents = re.sub(sl_comment_re, '', clean_file_contents)

    #remove single-line and multi-line macros
    #a macro is #define statement followed by a few statements that have a slash in the line before it
    macro_re = re.compile(r'#define(\s+)(((?!\\).)+)((\\\n(((?!\\).)+))*)')
    clean_file_contents = clean_string_from_regex(clean_file_contents, macro_re, '')

    return clean_file_contents


def read_file(filename):
    """
    Reads contents of the file, returns a 2-tuple containing:
        1- raw file contents
        2- raw file contents as lines
    """
    file_contents = open(filename, encoding='utf-8-sig').read()
    file_lines = file_contents.split("\n")
    return file_contents, file_lines


def clean_file_content(file_contents):
    """
    Reads contents of the file, returns a 2-tuple containing:
        1- file contents without comments, strings and macros
        2- file contents without comments, strings and macros as lines
    """
    clean_file_contents = get_clean_file_contents(file_contents)
    clean_file_lines = clean_file_contents.split("\n")
    return clean_file_contents, clean_file_lines


def get_all_files_with_filter(root, positive, negative):
    """
    Iterates over targets and gets names of all files that do not match positive and negative filter
    """

    positive_re = re.compile("|".join(["({})".format(p) for p in positive]))
    negative_re = re.compile("|".join(["({})".format(n) for n in negative]))

    filenames = []
    for f in subprocess.check_output(['git', 'ls-files', root], cwd=root, shell=False).decode('utf-8').split('\n'):
        if os.path.isfile(os.path.join(root, f)):
            pos_match = False
            neg_match = False
            relative_path = f.replace('\\', '/')
            pos_match = positive_re.search(relative_path)
            neg_match = negative_re.search(relative_path)
            if pos_match and not neg_match:
                filenames.append(os.path.join(root,relative_path))

    return filenames

def get_all_files(targets):
    """
    Iterates over targets and gets names of all files

    """
    filenames = []
    for t in targets:
        #if directory: handle every file
        if os.path.isdir(t):
            for (path, _, files) in os.walk(t, followlinks=True):
                for f in files:
                    filenames.append(os.path.abspath(os.path.join(path, f)))

        #if file: process directly
        else:
            filenames.append(os.path.abspath(t))

    return filenames

def log_warning(testname, filename, line_number, description, line_content = None):
    """
    Prints a warning to the standard output
    the warning is printed 2 times:
    1. visual studio format, so visual studio will pick up warnings
    2. gcc format, so gcc based tools pick up the warnings

    """
    if line_content == None:
        # visual studio / msbuild format
        print("{0}({1}): warning PRJ9999: {2} [{3}]".format(filename, line_number, description, testname), file=sys.stderr)
        # gcc format
        print("{0}:{1}: warning: {2} [{3}]".format(filename, line_number, description, testname), file=sys.stderr)
    else:
        # visual studio / msbuild format
        print("{0}({1}): warning PRJ9999: {2}: {3} [{4}]".format(filename, line_number, description, line_content, testname), file=sys.stderr)
        # gcc format
        print("{0}:{1}: warning: {2}: {3} [{4}]".format(filename, line_number, description, line_content, testname), file=sys.stderr)

    print("", file=sys.stderr)

    common_modules.config.G_WARNING_COUNT += 1

def get_warning_count():
    """
    Returns the number of reported warnings

    """
    return common_modules.config.G_WARNING_COUNT
