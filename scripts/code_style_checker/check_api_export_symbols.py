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


def check_api_export_symbols(filename, clean_file_contents):
    """
    Check that public API symbols are exported
    - A class has RAMSES_API prefix before the name, structs and enums don't have it
    """

    is_api = ("ramses-client-api" in filename) or ("ramses-text-api" in filename) or ("ramses-renderer-api" in filename) or ("ramses-framework-api" in filename)
    is_header =  (filename[-2:] == ".h") or (filename[-4:] == ".hpp")
    is_api_header = is_api and is_header

    if is_api_header:
        # symbol_def_re matches following patterns:
        #   (correct case) enum|class|struct RAMSES_API SymbolName { [...]
        #   (correct case) enum|class|struct RAMSES_API SymbolName : [...]
        #   (wrong case)   enum|class|struct SymbolName { [...]
        #   (wrong case)   enum|class|struct SymbolName : [...]
        # Groups (?<! is no group):  (x    )(0                )(1  )(2  )(3  )(4  )(5  )(6    )
        symbol_def_re = re.compile(r'(?<!\w)(enum|class|struct)(\s+)(\w+)(\s+)(\w*)(\s*)(\{|\:)')

        for symbol_match in re.finditer(symbol_def_re, clean_file_contents):
            line_number = clean_file_contents[:symbol_match.start()].count("\n")

            symbolNameGroups = symbol_match.groups()
            isEnum     = symbolNameGroups[0].strip() == "enum"
            isStruct   = symbolNameGroups[0].strip() == "struct"
            firstWord  = symbolNameGroups[2].strip()
            secondWord = symbolNameGroups[4].strip()

            if isEnum:
                if firstWord == "RAMSES_API":
                    log_warning("check_api_export_symbols", filename, line_number + 1, "Enum exported as RAMSES_API: " + secondWord)
            elif isStruct:
                if firstWord == "RAMSES_API":
                    log_warning("check_api_export_symbols", filename, line_number + 1, "Struct exported as RAMSES_API: " + secondWord)
            else:
                # Exceptions for the rule - these symbols are not supposed to be exported
                # StronglyTypedValue  -  header only class
                excludedSymbols = ["StronglyTypedValue"]

                if firstWord != "RAMSES_API" and firstWord not in excludedSymbols:
                    log_warning("check_api_export_symbols", filename, line_number + 1, "Public symbol not exported as RAMSES_API: " + firstWord)
    else:
        # Will find occurances of RAMSES_API surrounded by space(s)
        RAMSES_API_re = re.compile(r'(?<!\w)(RAMSES_API)\s')
        for symbol_match in re.finditer(RAMSES_API_re, clean_file_contents):
            line_number = clean_file_contents[:symbol_match.start()].count("\n")
            log_warning("check_api_export_symbols", filename, line_number + 1, "Exporting API symbol in a non-API-header file! This symbol will be unusable.")
