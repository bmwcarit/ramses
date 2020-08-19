//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#ifndef RAMSES_WAYLANDCALLBACKRESOURCE_H
#define RAMSES_WAYLANDCALLBACKRESOURCE_H

#include "EmbeddedCompositor_Wayland/WaylandResource.h"

namespace ramses_internal
{
    class WaylandCallbackResource : public WaylandResource
    {
    public:
        WaylandCallbackResource();
        WaylandCallbackResource(wl_resource* resource, bool ownership);
        virtual void callbackSendDone(uint32_t callbackData);
    };
}

#endif
