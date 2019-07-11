#!/usr/bin/env python3

#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

"""
Runs all enabled style checking

"""

import sys, re, string
from common_modules.common import *
from common_modules import config

from check_license import check_license_for_file
from check_header_guards import check_header_guards
from check_curly_braces_alone_on_line import check_curly_braces_alone_on_line
from check_single_definition_on_line import check_single_definition_on_line
from check_single_statement_on_line import check_single_statement_on_line
from check_tabbing_and_spacing import check_tabbing_and_spacing
from check_tabbing_and_spacing import check_no_spacing_line_end
from check_tabbing_and_spacing import check_tabs_no_spaces
from check_enum_style import check_enum_style
from check_last_line_newline import check_last_line_newline
from check_api_export_symbols import check_api_export_symbols


def main():

    if len(sys.argv) > 2:
        print("""
Usage: check_all_styles.py [<path>]

Takes a path as input and runs style/license header checks with filters where necessary.

""")
        exit(-1)

    if len(sys.argv) < 2:
        path = os.path.realpath(os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", ".."))
    else:
        path = os.path.realpath(sys.argv[1])

    print("Check {}".format(path))

    binary_files = {
        r'\.res$',
        r'\.pptx$',
        r'\.png$',
        r'\.PNG$',
        r'\.bmp$',
        r'\.BMP$',
        r'\.ttf$',
        r'\.pyc$',
        r'\.bin$',
        r'\.pac$',
        r'\.rex$',
        r'\.ctm$',
        r'\.tar\.bz2$',
    }

    # These files are not checked at all
    shared_blacklist = binary_files | {
        r'\.git',
        r'\.gitignore',
        r'\.patch',
        r'gitconfig$',
        r'\.clang-format$',
        r'\.clang-tidy$',
        # Protobuffer files
        r'\.pb.h$',
        r'\.pb.cc$',
        r'\.proto$',
        r'\.ptx$',
        r'(^|/)build[^/]*/',
    }

    # Whitelist for source files
    src_whitelist   = {
        r'\.h$',
        r'\.hpp$',
        r'\.cpp$'
    }

    # Externals are allowed to have their own code style
    src_blacklist = shared_blacklist | {r'^external/'}

    src_files = get_all_files_with_filter(path, src_whitelist, src_blacklist)

    # Check all styles for source files
    for f in src_files:

        file_contents, file_lines = read_file(f)
        clean_file_contents, clean_file_lines = clean_file_content(file_contents)

        check_header_guards                 (f, file_contents)
        check_license_for_file              (f, file_contents, path)
        check_tabbing_and_spacing           (f, file_lines)
        check_curly_braces_alone_on_line    (f, file_contents, clean_file_contents, file_lines, clean_file_lines)
        check_single_statement_on_line      (f, file_contents, clean_file_contents, file_lines, clean_file_lines)
        check_single_definition_on_line     (f, file_contents, clean_file_contents, file_lines, clean_file_lines)
        check_enum_style                    (f, clean_file_contents)
        check_last_line_newline             (f, file_contents)
        check_api_export_symbols            (f, clean_file_contents)

    shared_blacklist_non_src_files = shared_blacklist | {
        # Externals allowed to have own formatting and license
        r'^external',
        r'valgrind/suppressions$',
        r'\.spdx$',
        r'^CHANGELOG\.txt$', # Doesn't need a license
        r'^LICENSE\.txt$',   # Contains license info, not related to code/content
        r'^.lfsconfig$',     # Doesn't need a license
        # Only on the build server, needs to be blacklisted
        r'^envVar\.txt$',
    }

    blacklist_files_formatting = shared_blacklist_non_src_files | {
        # Formatting intended the way it is
        r'with-additional-spaces-and-empty-lines.*config$',
    }

    files_formatting = get_all_files_with_filter(path, {r'.*'}, blacklist_files_formatting)

    # Check subset of the rules for non-source files
    for f in files_formatting:

        file_contents, file_lines = read_file(f)

        check_tabs_no_spaces                (f, file_lines)
        check_no_spacing_line_end           (f, file_lines)
        check_last_line_newline             (f, file_contents)

    blacklist_license = shared_blacklist_non_src_files | {
        # Can be safely excluded, don't need license header because trivial
        r'__init__\.py$',
        r'asan_suppressions\.txt$',
        r'lsan_suppressions\.txt$',
        r'tsan_blacklist\.txt$',
        r'DOCKER_TAG$',
        r'maven_settings\.xml$',
        r'\.config$',
        r'\.conf$',
        r'\.filepathesconfig$',
        # Excluded on purpose - add new lines reasonibly here!
        r'\.patch$',        # License headers can't be added to patch files
        r'\.tmpl$',         # File content hash-dependent, can't modify
        r'^README\.txt$',                   # Doesn't need a license
        r'^proprietary/oss/README\.md$',    # Doesn't need a license
        r'^integration/TestContent/res/BigString\.txt$', # Test file with random content - doesn't need license
        r'^cmake/templates/ramses-version\.in$', # Just a template, doesn't need license
    }
    files_license_header = get_all_files_with_filter(path, {r'.*'}, blacklist_license)

    # Check subset of the rules for non-source files
    for f in files_license_header:

        file_contents, file_lines = read_file(f)

        check_license_for_file               (f, file_contents, path)

    print('checked {0} files'.format(len(set(src_files) | set(files_formatting) | set(files_license_header))))

    if 0 == config.G_WARNING_COUNT:
        print("your style is awesome! no style guide violations detected.")
    else:
        print("detected {0} style guide issues".format(config.G_WARNING_COUNT))

    exit(-config.G_WARNING_COUNT)

sys.exit(main())
