//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#ifndef RAMSES_IWAYLANDCOMPOSITORCONNECTION_H
#define RAMSES_IWAYLANDCOMPOSITORCONNECTION_H

#include <cstdint>

namespace ramses_internal
{
    class IWaylandClient;

    class IWaylandCompositorConnection
    {
    public:
        virtual ~IWaylandCompositorConnection(){}

        virtual void resourceDestroyed() = 0;
        virtual void compositorCreateSurface(IWaylandClient& client, uint32_t id) = 0;
        virtual void compositorCreateRegion(IWaylandClient& client, uint32_t id) = 0;
    };
}

#endif
