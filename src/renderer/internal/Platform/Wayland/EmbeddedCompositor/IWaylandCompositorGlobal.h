//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/Types.h"

namespace ramses::internal
{
    class IWaylandDisplay;
    class IWaylandClient;

    class IWaylandCompositorGlobal
    {
    public:
        virtual ~IWaylandCompositorGlobal() = default;
        virtual bool init(IWaylandDisplay& serverDisplay) = 0;
        virtual void compositorBind(IWaylandClient& client, uint32_t version, uint32_t id) = 0;
        virtual void destroy() = 0;
    };
}
