//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_STRINGUTILS_H
#define RAMSES_STRINGUTILS_H

#include "Collections/String.h"
#include "Collections/HashSet.h"
#include "Collections/Vector.h"

namespace ramses_internal
{
    using StringVector = std::vector<String>;
    using StringSet = HashSet<String>;

    class StringUtils
    {
    public:
        /**
        * Return a trimmed string without leading and ending spaces
        * @param nativeString string to trim
        * @return trimmed String
        */
        static String Trim(const Char* nativeString);

        /**
        * Split a string into separate tokens
        * @param[in] string string to split
        * @param[out] tokens set of token strings
        */
        static std::vector<String> Tokenize(const String& string, const char split = ' ');

        /**
        * Split a string into separate tokens
        * @param[in] string string to split
        * @param[out] tokens set of token strings
        */
        static HashSet<String> TokenizeToSet(const String& string, const char split = ' ');

        static std::vector<String> TokenizeTrimmed(const ramses_internal::String& line, char split);
    };
}

#endif
