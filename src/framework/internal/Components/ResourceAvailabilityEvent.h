//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/SceneId.h"
#include <vector>
#include "internal/SceneGraph/SceneAPI/ResourceContentHash.h"

namespace ramses::internal
{
    struct ResourceAvailabilityEvent
    {
        SceneId sceneid;
        std::vector<ResourceContentHash> availableResources;

        void readFromBlob(std::vector<std::byte> const& blob);
        void writeToBlob(std::vector<std::byte>& blob) const;
    };

}
