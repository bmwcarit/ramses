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
    class RamshTools
    {
    public:
        static std::vector<std::string> parseCommandString(const std::string& msg);
        static std::size_t delimiterPosition(const std::string& msg, const std::string& delimiter);
        static std::size_t trailingSpacesPosition(const std::string& msg, std::size_t offset);
        static std::size_t leadingSpacesPosition(const std::string& msg, std::size_t offset);
    };

}

#endif
