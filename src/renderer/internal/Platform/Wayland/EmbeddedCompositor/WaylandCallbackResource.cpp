//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandCallbackResource.h"

namespace ramses::internal
{
    WaylandCallbackResource::WaylandCallbackResource() = default;

    WaylandCallbackResource::WaylandCallbackResource(wl_resource* resource): NativeWaylandResource(resource)
    {
    }

    void WaylandCallbackResource::callbackSendDone(uint32_t callbackData)
    {
        wl_callback_send_done(m_resource, callbackData);
    }
}
