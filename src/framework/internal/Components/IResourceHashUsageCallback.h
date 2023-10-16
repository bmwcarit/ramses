//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/ResourceContentHash.h"

namespace ramses::internal
{
    class IResourceHashUsageCallback
    {
    public:
        virtual ~IResourceHashUsageCallback() = default;

        virtual void resourceHashUsageZero(const ResourceContentHash& hash) = 0;
    };
}
