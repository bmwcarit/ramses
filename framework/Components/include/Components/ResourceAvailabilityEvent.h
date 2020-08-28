//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCEAVAILABILITYEVENT_H
#define RAMSES_RESOURCEAVAILABILITYEVENT_H

#include "SceneAPI/SceneId.h"
#include <vector>
#include "SceneAPI/ResourceContentHash.h"

namespace ramses_internal
{
    struct ResourceAvailabilityEvent
    {
        SceneId sceneid;
        std::vector<ResourceContentHash> availableResources;

        void readFromBlob(std::vector<Byte> const& blob);
        void writeToBlob(std::vector<Byte>& blob) const;
    };

}

#endif
