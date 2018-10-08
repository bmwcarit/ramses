//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESFRAMEWORKTYPESIMPL_H
#define RAMSES_RAMSESFRAMEWORKTYPESIMPL_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-capu/container/Hash.h"

namespace ramses_capu
{
    template<>
    struct Hash<::ramses::resourceId_t>
    {
        uint_t operator()(const ::ramses::resourceId_t& rid)
        {
            return HashValue(rid.lowPart, rid.highPart);
        }
    };
}

#endif
