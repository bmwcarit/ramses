//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#pragma once

#include "internal/Platform/Wayland/EmbeddedCompositor/NativeWaylandResource.h"

namespace ramses::internal
{
    class WaylandCallbackResource : public NativeWaylandResource
    {
    public:
        WaylandCallbackResource();
        explicit WaylandCallbackResource(wl_resource* resource);
        virtual void callbackSendDone(uint32_t callbackData);
    };
}
