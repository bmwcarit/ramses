//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSHTOOLS_H
#define RAMSES_RAMSHTOOLS_H

#include "Ramsh/RamshInput.h"

namespace ramses_internal
{
    class String;

    class RamshTools
    {
    public:
        static RamshInput parseCommandString(const String& msg);
        static UInt delimiterPosition(const String& msg, const String& delimiter);
        static UInt trailingSpacesPosition(const String& msg, UInt offset);
        static UInt leadingSpacesPosition(const String& msg, UInt offset);
    };

}

#endif
