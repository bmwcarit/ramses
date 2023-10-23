//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#pragma once

#include <cstdint>
#include "internal/RendererLib/Types.h"

namespace ramses::internal
{
    class IWaylandClient;
    class INativeWaylandResource;

    class IWaylandIVIApplicationConnection
    {
    public:
        virtual ~IWaylandIVIApplicationConnection() = default;
        virtual void resourceDestroyed() = 0;
        virtual void iviApplicationIVISurfaceCreate(IWaylandClient& client, WaylandIviSurfaceId iviId, INativeWaylandResource& surfaceResource, uint32_t id) = 0;
    };
}
