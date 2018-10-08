//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WAYLANDCOMPOSITORCONNECTIONMOCK_H
#define RAMSES_WAYLANDCOMPOSITORCONNECTIONMOCK_H

#include "gmock/gmock.h"
#include "EmbeddedCompositor_Wayland/IWaylandCompositorConnection.h"
#include "EmbeddedCompositor_Wayland/IWaylandClient.h"

namespace ramses_internal
{
    class WaylandCompositorConnectionMock : public IWaylandCompositorConnection
    {
    public:
        MOCK_METHOD0(resourceDestroyed, void());
        MOCK_METHOD2(compositorCreateSurface, void(IWaylandClient& client, uint32_t id));
        MOCK_METHOD2(compositorCreateRegion, void(IWaylandClient& client, uint32_t id));
    };
}

#endif
