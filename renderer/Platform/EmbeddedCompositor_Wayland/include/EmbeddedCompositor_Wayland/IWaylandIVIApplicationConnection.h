//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#ifndef RAMSES_IWAYLANDIVIAPPLICATIONCONNECTION_H
#define RAMSES_IWAYLANDIVIAPPLICATIONCONNECTION_H

#include <cstdint>
#include <SceneAPI/WaylandIviSurfaceId.h>

namespace ramses_internal
{
    class IWaylandClient;
    class INativeWaylandResource;

    class IWaylandIVIApplicationConnection
    {
    public:
        virtual ~IWaylandIVIApplicationConnection(){}
        virtual void resourceDestroyed() = 0;
        virtual void iviApplicationIVISurfaceCreate(IWaylandClient& client, WaylandIviSurfaceId iviId, INativeWaylandResource& surfaceResource, uint32_t id) = 0;
    };
}

#endif
