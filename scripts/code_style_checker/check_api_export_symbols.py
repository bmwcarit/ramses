#  -------------------------------------------------------------------------
#  Copyright (C) 2013 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import re
from common_modules import common


def check_api_export_symbols(filename, clean_file_contents):
    """
    Check that public API symbols are exported
    - A class has RAMSES_API prefix before the name, structs and enums don't have it
    """

    is_api = ("ramses-client-api" in filename) or ("ramses-text-api" in filename) or ("ramses-renderer-api" in filename) or ("ramses-framework-api" in filename)
    is_header = (filename[-2:] == ".h") or (filename[-4:] == ".hpp")
    is_api_header = is_api and is_header

    if is_api_header:
        # symbol_def_re matches following patterns:
        #   (correct case) class RAMSES_API SymbolName { [...]
        #   (correct case) class RAMSES_API SymbolName : [...]
        #   (correct case) struct/enum|enum class SymbolName : [...]
        #   (correct case) class SymbolName : [..no symbol exported (but base class can be exported)..]
        #   (wrong case)   class SymbolName { [..no symbol exported..]
        symbol_def_re = re.compile(r'(template [^;]+?)?\s+(enum|class|struct|enum\s+class)(\s+)(\w+)(\s+)(\w*)(\s*)(\{|\:)')

        for symbol_match in re.finditer(symbol_def_re, clean_file_contents):
            line_number = clean_file_contents[:symbol_match.start()].count("\n")

            symbolNameGroups = symbol_match.groups()
            isTemplate = symbolNameGroups[0] is not None
            isEnum = symbolNameGroups[1].strip() == "enum"
            isStruct = symbolNameGroups[1].strip() == "struct"
            isEnumClass = "enum " in symbolNameGroups[1].strip()
            isDerived = (symbolNameGroups[7] is not None) and (symbolNameGroups[7].strip() == ":")
            firstWord = symbolNameGroups[3].strip()
            secondWord = symbolNameGroups[5].strip()

            # check special cases that should NOT have RAMSES_API
            if isEnum:
                if firstWord == "RAMSES_API":
                    common.log_warning("check_api_export_symbols", filename, line_number + 1, "Enum exported as RAMSES_API: " + secondWord)
            elif isEnumClass:
                if firstWord == "RAMSES_API":
                    common.log_warning("check_api_export_symbols", filename, line_number + 1, "Enum class exported as RAMSES_API: " + secondWord)
            elif isStruct:
                if firstWord == "RAMSES_API":
                    common.log_warning("check_api_export_symbols", filename, line_number + 1, "Struct exported as RAMSES_API: " + secondWord)
            elif isTemplate:
                if firstWord == "RAMSES_API":
                    common.log_warning("check_api_export_symbols", filename, line_number + 1, "Template exported as RAMSES_API: " + secondWord)
            else:
                # Ramses SDK uses selective export which is not easy to check via source parsing, this only ensures that classes which are not derived
                # have any symbol exported (basic sanity check that reminds developer to export properly)
                if (firstWord != "RAMSES_API") and ("RAMSES_API" not in clean_file_contents) and (not isDerived):
                    common.log_warning("check_api_export_symbols", filename, line_number + 1, "Public symbol not exported as RAMSES_API: " + firstWord)
    else:
        # Will find occurances of 'RAMSES_API' (whole word) but not containing 'template' (whole word),
        # template instantiations can be in a non-API-header files.
        RAMSES_API_re = re.compile(r'^(?!.*\b(template)\b).*\b(API)\b')
        for symbol_match in re.finditer(RAMSES_API_re, clean_file_contents):
            line_number = clean_file_contents[:symbol_match.start()].count("\n")
            common.log_warning("check_api_export_symbols", filename, line_number + 1,
                               "Exporting API symbol in a non-API-header file! This symbol will be unusable.")
