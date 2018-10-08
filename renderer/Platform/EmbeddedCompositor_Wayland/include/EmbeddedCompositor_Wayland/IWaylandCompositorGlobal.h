//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IWAYLANDCOMPOSITORGLOBAL_H
#define RAMSES_IWAYLANDCOMPOSITORGLOBAL_H

#include "RendererAPI/Types.h"

namespace ramses_internal
{
    class IWaylandDisplay;
    class IWaylandClient;

    class IWaylandCompositorGlobal
    {
    public:
        virtual ~IWaylandCompositorGlobal(){}
        virtual bool init(IWaylandDisplay& serverDisplay) = 0;
        virtual void compositorBind(IWaylandClient& client, uint32_t version, uint32_t id) = 0;
        virtual void destroy() = 0;
    };
}

#endif
