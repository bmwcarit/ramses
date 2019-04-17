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

def remove_tollerable_braces(clean_file_contents):
    """
    Cleans file contents from code blocks that should not be checked:
        1- Array initialization blocks
        2- Empty functions

    """
    #array assignment begins with a left curly brace ({) followed by any number of characters that is not a semicolon
    #it ends by a right curly brace (}) and a semicolon (;)
    assignment_re = re.compile(r'=(\s*)(\{)((?!;).)*(\})(\s*);', re.DOTALL)
    #remove array assignments from file contents
    no_assig_contents = clean_string_from_regex(clean_file_contents, assignment_re, '')

    #std array initialization:
    # 1. has keyword "array"
    # 2. followed by arrow brackets with the template params in-between
    # 3. followed by curly braces with values in-betweeen
    array_initialization_re = re.compile(r'array\s*\<(?:(?!;).)*\>\s*\{(?:(?!;).)*\}', re.DOTALL)
    #remove std array initialization
    no_array_initialization_contents = clean_string_from_regex(no_assig_contents, array_initialization_re, '')

    #remove braces that are used for implementing empty functions
    empty_func_re = re.compile(r'\{\s*\}')
    no_empty_func_contents = clean_string_from_regex(no_array_initialization_contents, empty_func_re, '')

    return no_empty_func_contents

def check_curly_braces_alone_on_line(filename, file_contents, clean_file_contents, file_lines, clean_file_lines):
    """
    Checks that there is no unnecessary code on same line with curly braces.

    """
    clean_file_contents = remove_tollerable_braces(clean_file_contents)
    clean_file_lines = clean_file_contents.split('\n')

    #keep a stack booleans to indicate where type declarations are made
    typedecl_stack = []

    #a type declaration contains the words struct, class, union or enum
    #it can also have the typdef keyword in the beginning
    typedecl_re = re.compile(r"(?!<\w)(typedef)?(\s*)(struct|class|enum|union)(?!\w)")

    #some statements are allowed to have curly brace on same
    #(do..while) blocks, namespaces and extern blocks
    permissible_re = re.compile(r'''(\s*)
                                (
                                    do|                     # do keyword
                                    (namespace((?!\{).)*)|  # namespace keyword
                                    (extern((?!\{).)*)      # extern keyword
                                )?
                                (\s*)\{(\s*)                # open curly brace
                                (\\?)(\s*)                  # backslash indicates a multiline macro or multiline string
                                $''', re.VERBOSE)
    # allow lambda functions
    inside_expression_open_re = re.compile(r'(,|\(|\w+|)\s*\{')
    inside_expression_close_re = re.compile(r'\}\s*(,|\)|;|\()')
    part_of_lambda_expression_open_re = re.compile(r'\[.*\].*\(.*\)\s*\{')  # this might match too much, rework to whitelist instead of blacklist where {} is allowed

    former_line = ""
    for i in range(len(clean_file_lines)):
        line = clean_file_lines[i]

        if line.count("{"):
            #check if the line contains a type declaration
            typedecl = re.search(typedecl_re, former_line)
            typedecl_stack.append(typedecl)

            #make a warning if line does not follow the permissible set of expressions
            if re.match(permissible_re, line) == None and \
                re.search(inside_expression_open_re, line) == None and \
                re.search(inside_expression_open_re, line) == None and \
                re.search(part_of_lambda_expression_open_re, line) == None:
                log_warning("check_curly_braces_alone_on_line", filename, i + 1, "other code on same line with opening curly brace", file_lines[i].strip(" "))

        if line.count("}"):

            typedecl = 0

            if typedecl_stack.__len__() > 0 :

                typedecl = typedecl_stack.pop()


            if typedecl:
                #if the right curly brace marks the end of type declaration:
                #it is possible to have code AFTER the curly brace in case of type declarations
                #like instantiating a structure at end of structure definition
                if re.match(r"(\s*)\}", line) == None:
                    log_warning("check_curly_braces_alone_on_line", filename, i + 1, "other code on same line with closing curly brace", file_lines[i].strip(" "))
            else:
                #if the right curly brace does not mark the end of type declaration:
                #the line must begin with the right curly brace, can possibly have a while expression and ends by semicolon
                #this has to be the end of line ($) to avoid having a second statement on the same line
                if re.match(r"(\s*)\}(\s*)((while((?!;).)*)?)(;?)(\s*)(\\?)(\s*)$", line) == None and re.search(inside_expression_close_re, line) == None:
                    log_warning("check_curly_braces_alone_on_line", filename, i + 1, "other code on same line with closing curly brace", file_lines[i].strip(" "))

        if line.strip(" \t\n\f\r"):
            former_line = line

if __name__ == "__main__":
    targets = sys.argv[1:]
    targets = get_all_files(targets)

    if len(targets) == 0:
        print("""
\t**** No input provided ****
\tTakes a list of files/directories as input and performs specific style checking on all files/directories

\tGives warnings if unnecessary code exists on the same line with curly braces.
""")
        exit(0)

    for t in targets:
        if t[-2:] == ".h" or t[-4:] == ".cpp" or t[-2] == ".c":
            file_contents, file_lines = read_file(t)
            clean_file_contents, clean_file_lines = clean_file_content(file_contents)
            check_curly_braces_alone_on_line(t, file_contents, clean_file_contents, file_lines, clean_file_lines)
