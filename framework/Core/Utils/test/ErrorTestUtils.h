//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#ifndef RAMSES_ERRORTESTUTILS_H
#define RAMSES_ERRORTESTUTILS_H

#include "PlatformAbstraction/PlatformTypes.h"

namespace ramses_internal
{
    class ComparableObject
    {
    public:
        UInt32 integer;
        ComparableObject* pointer;

        bool operator==(const ComparableObject& other) const
        {
            return integer == other.integer && pointer == other.pointer;
        }

        bool operator!=(const ComparableObject& other) const
        {
            return integer != other.integer || pointer != other.pointer;
        }
    };

}

#endif
