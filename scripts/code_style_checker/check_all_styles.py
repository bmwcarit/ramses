#!/usr/bin/env python3

#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

"""
Runs all enabled style checking

"""

import sys
import os
import common_modules.common
import argparse

from check_license import check_license_for_file
from check_header_guards import check_header_guards
# from check_curly_braces_alone_on_line import check_curly_braces_alone_on_line
# from check_single_statement_on_line import check_single_statement_on_line
from check_single_definition_on_line import check_single_definition_on_line
from check_tabbing_and_spacing import check_tabbing_and_spacing
from check_tabbing_and_spacing import check_no_spacing_line_end
from check_tabbing_and_spacing import check_tabs_no_spaces
from check_enum_style import check_enum_style
from check_last_line_newline import check_last_line_newline
from check_comments import check_doxygen_singleline_comments
from check_deprecated import check_deprecated
from check_file_attributes import check_file_attributes
from check_doxygen import check_doxygen


def main():
    sdk_root = os.path.realpath(os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", ".."))
    parser = argparse.ArgumentParser()
    parser.add_argument(dest="basepath", nargs='?', default=sdk_root, help='Path to check (default: repo root)')
    args = parser.parse_args()

    path = os.path.realpath(args.basepath)
    print("Check {}".format(path))

    binary_files = {
        r'\.res$',
        r'\.png$',
        r'\.PNG$',
        r'\.bmp$',
        r'\.BMP$',
        r'\.ttf$',
        r'\.svg$',
        r'\.pyc$',
        r'\.bin$',
        r'\.pac$',
        r'\.rex$',
        r'\.tar\.bz2$',
        r'\.jar$',
        r'\.ramses$',
        r'\.rlogic$',
    }

    # These files are not checked at all
    shared_blacklist = binary_files | {
        r'\.git',
        r'\.gitignore',
        r'\.patch',
        r'gitconfig$',
        r'\.clang-format$',
        r'\.clang-tidy$',
        r'/id_rsa[^\\]*$',
        r'(^|/)build[^/]*/',
    }

    # Whitelist for source files
    src_whitelist = {
        r'\.h$',
        r'\.hpp$',
        r'\.cpp$'
    }

    generated_files = {
        r'^src/client/internal/logic/flatbuffers/generated'
    }

    # Externals are allowed to have their own code style
    src_blacklist = shared_blacklist | {r'^external/'} | generated_files

    src_files = common_modules.common.get_all_files_with_filter(sdk_root, path, src_whitelist, src_blacklist)

    # Check all styles for source files
    for f in src_files:

        file_contents, file_lines = common_modules.common.read_file(f)
        clean_file_contents, clean_file_lines = common_modules.common.clean_file_content(file_contents)

        check_header_guards(f, file_contents)
        check_license_for_file(f, file_contents, sdk_root)
        check_tabbing_and_spacing(f, file_lines)
        # TODO: decide if these checkers make sense still
        # check_curly_braces_alone_on_line(f, file_contents, clean_file_contents, file_lines, clean_file_lines)
        # check_single_statement_on_line(f, file_contents, clean_file_contents, file_lines, clean_file_lines)
        check_single_definition_on_line(f, file_contents, clean_file_contents, file_lines, clean_file_lines)
        check_deprecated(f, file_contents, clean_file_contents, file_lines, clean_file_lines)
        check_enum_style(f, clean_file_contents)
        check_last_line_newline(f, file_contents)
        check_doxygen_singleline_comments(f, file_lines)

    shared_blacklist_non_src_files = shared_blacklist | generated_files | {
        # Externals allowed to have own formatting
        r'^external',
        # created by Android Studio
        r'.*/\.idea/',
        r'.*/gradle/wrapper',
        r'.*/gradle\.properties$',
        r'.*/gradlew$',
        r'.*/gradlew\.bat$',
        # Android demo
        r'^demo/android/ramses-renderer-android-app/app/src/main/res/',
        r'^demo/android/ramses-renderer-android-app/build\.gradle',
        r'^demo/android/ramses-renderer-android-app-native-activity/app/src/main/res/',
        r'^demo/android/ramses-renderer-android-app-native-activity/build\.gradle',
        # iOS demo
        r'Info\.plist$',
        r'\.storyboard$',
    }

    blacklist_files_formatting = shared_blacklist_non_src_files | {
        # Formatting intended the way it is
        r'with-additional-spaces-and-empty-lines.*config$',
    }

    files_formatting = common_modules.common.get_all_files_with_filter(sdk_root, path, {r'.*'}, blacklist_files_formatting)

    # Check subset of the rules for non-source files
    for f in files_formatting:

        file_contents, file_lines = common_modules.common.read_file(f)

        check_tabs_no_spaces(f, file_lines)
        check_no_spacing_line_end(f, file_lines)
        check_last_line_newline(f, file_contents)

    blacklist_license = shared_blacklist_non_src_files | generated_files | {
        # Can be safely excluded, don't need license header because trivial
        r'__init__\.py$',
        r'asan_suppressions\.txt$',
        r'lsan_suppressions\.txt$',
        r'tsan_blacklist\.txt$',
        r'ubsan_blacklist\.txt$',
        r'maven_settings\.xml$',
        r'\.config$',
        r'\.conf$',
        r'^demo/android/DemoRamsesAndroidModule/src/main/AndroidManifest\.xml',
        # Excluded on purpose - add new lines reasonibly here!
        r'\.tmpl$',         # File content hash-dependent, can't modify
        r'\.md$',                   # Markdown files never need license
        r'.*/AndroidManifest\.xml$',  # formatting different due to xml restrictions
        r'^CHANGELOG\.md$',  # Doesn't need a license
        r'^LICENSE\.txt$',   # Contains license info, not related to code/content
        r'^proprietary/oss/LICENSE\.txt$',  # Contains oss license info, not related to code/content
        r'^proprietary/oss/CHANGELOG\.md$',  # Doesn't need a license
        r'^.lfsconfig$',     # Doesn't need a license
        r'valgrind/suppressions$',
        r'\.spdx$',
        r'requirements\.txt',
        r'^doc/old_ramses/doxygen/DoxygenLayout.xml$'
    }
    files_license_header = common_modules.common.get_all_files_with_filter(sdk_root, path, {r'.*'}, blacklist_license)

    # Check subset of the rules for non-source files
    for f in files_license_header:
        file_contents, file_lines = common_modules.common.read_file(f)
        check_license_for_file(f, file_contents, sdk_root)

    files_attribute_checking = common_modules.common.get_all_files_with_filter(sdk_root, path, {r'.*'}, {r'\.sh$', r'\.py$', r'/gradlew$', r'\.bat$'})
    for f in files_attribute_checking:
        check_file_attributes(f)

    files_doxgen_checking = common_modules.common.get_all_files_with_filter(sdk_root, path, {r'\.dox$'}, {})
    for f in files_doxgen_checking:
        file_contents, file_lines = common_modules.common.read_file(f)
        check_doxygen(f, file_contents, file_lines)

    print('checked {0} files'.format(len(set(src_files) | set(files_formatting) | set(files_license_header))))

    if 0 == common_modules.common.G_WARNING_COUNT:
        print("your style is awesome! no style guide violations detected.")
        return 0
    else:
        print("detected {0} style guide issues".format(common_modules.common.G_WARNING_COUNT))
        return 1


sys.exit(main())
