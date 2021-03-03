//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSHTOOLS_H
#define RAMSES_RAMSHTOOLS_H

#include <vector>
#include <string>

namespace ramses_internal
{
    class String;

    class RamshTools
    {
    public:
        static std::vector<std::string> parseCommandString(const String& msg);
        static std::size_t delimiterPosition(const String& msg, const String& delimiter);
        static std::size_t trailingSpacesPosition(const String& msg, std::size_t offset);
        static std::size_t leadingSpacesPosition(const String& msg, std::size_t offset);
    };

}

#endif
