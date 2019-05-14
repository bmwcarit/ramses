//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_UTFUTILS_H
#define RAMSES_UTFUTILS_H

#include "ramses-framework-api/APIExport.h"
#include <stdint.h>
#include <string>

namespace ramses
{
    /**
    * @brief Stores an extracted Unicode/UTF32 code-point and corresponding meta-data resulting from the conversion to Unicode (see also UtfUtils)
    */
    struct ExtractedUnicodePoint
    {
        /// true if the extraction was successful. Otherwise: false and codePoint is undefined
        bool extractionSuccessful;
        /// The unicode-point storing the unique id of a character (UTF32)
        uint32_t codePoint;
        /// Stores how many input characters were read to resolve the unicode point (bytes in UTF8 case and words in UTF16 case)
        size_t inputCharsConsumed;
    };

    /**
    * @brief Converts UTF and Unicode according to the Unicode standard
    */
    namespace UtfUtils
    {
        /**
        * @brief Converts a UTF8-encoded string to UTF32-encoded string
        * @param[in] utf8String The string in UTF8 encoding
        * @return The string in UTF32 encoding
        */
        RAMSES_API std::u32string ConvertUtf8ToUtf32(const std::string& utf8String);

        /**
        * @brief Converts a UTF32-encoded character to UTF8-encoded string
        * @param[in] convertChar The character in UTF32 encoding
        * @return The string in UTF8 encoding
        */
        RAMSES_API std::string ConvertCharUtf32ToUtf8(char32_t convertChar);

        /**
        * @brief Converts a UTF32-encoded string to UTF8-encoded string
        * @param[in] utf32String The string in UTF32 encoding
        * @return The string in UTF8 encoding
        */
        RAMSES_API std::string ConvertStrUtf32ToUtf8(const std::u32string& utf32String);

        /**
        * @brief Extracts a UTF32 character from a UTF8 string
        * @param[in] strBegin iterator to the first character to parse
        * @param[in] strEnd the end of the string - used for sanity check and prevent reading beyond the end of the string
        * @return The extracted unicode point
        */
        RAMSES_API ExtractedUnicodePoint ExtractUnicodePointFromUTF8(std::string::const_iterator strBegin, std::string::const_iterator strEnd);

        /**
        * @brief Converts a UTF32-encoded string to UTF8-encoded string
        * @param[in] utf32String The string in UTF32 encoding
        * @return The string in UTF8 encoding
        */
        RAMSES_API std::string ConvertUtf32ToUtf8String(const std::u32string& utf32String);

        /**
        * @brief Extracts a UTF32 character from a UTF16 string
        * @param[in] strBegin iterator to the first character to parse
        * @param[in] strEnd the end of the string - used for sanity check and prevent reading beyond the end of the string
        * @return The extracted unicode point
        */
        RAMSES_API ExtractedUnicodePoint ExtractUnicodePointFromUTF16(std::u16string::const_iterator strBegin, std::u16string::const_iterator strEnd);
    }
}

#endif
