//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/Collections/HashSet.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"

#include <string_view>
#include <string>

namespace ramses::internal
{
    class StringUtils
    {
    public:
        /**
        * Return a trimmed string without leading and ending spaces
        * @param string string view to trim
        * @return trimmed String
        */
        static std::string Trim(std::string_view string);

        /**
        * Return a trimmed string view without leading and ending spaces
        * @warning Returns dangling std::string_view if used with temporaries.
        * For ex. TrimView(string + "some text") or TrimView(std::string{"Hello world"})
        * @param string string view to trim
        * @return trimmed string view
        */
        static std::string_view TrimView(std::string_view string);

        /**
        * Split a string into separate tokens
        * @param[in] string string to split
        * @param[out] tokens set of token strings
        */
        static std::vector<std::string> Tokenize(std::string_view string, const char split = ' ');

        /**
        * Split a string into separate tokens
        * @param[in] string string to split
        * @param[out] tokens set of token strings
        */
        static HashSet<std::string> TokenizeToSet(std::string_view string, const char split = ' ');
    };
}
