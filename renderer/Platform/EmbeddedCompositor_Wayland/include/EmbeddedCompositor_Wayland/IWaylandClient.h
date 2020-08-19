//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#ifndef RAMSES_IWAYLANDCLIENT_H
#define RAMSES_IWAYLANDCLIENT_H

#include "EmbeddedCompositor_Wayland/WaylandClientCredentials.h"
#include <cstdint>

struct wl_interface;

namespace ramses_internal
{
    class IWaylandResource;
    class WaylandCallbackResource;

    class IWaylandClient
    {
    public:
        virtual ~IWaylandClient() {}
        virtual WaylandClientCredentials getCredentials() const = 0;
        virtual void postNoMemory()                                                  = 0;
        virtual IWaylandResource* resourceCreate(const wl_interface* interface, int version, uint32_t id)       = 0;
        virtual WaylandCallbackResource* callbackResourceCreate(const wl_interface* interface, int version, uint32_t id) = 0;
    };
}

#endif
