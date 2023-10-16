//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "gmock/gmock.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandCompositorConnection.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandClient.h"

namespace ramses::internal
{
    class WaylandCompositorConnectionMock : public IWaylandCompositorConnection
    {
    public:
        MOCK_METHOD(void, resourceDestroyed, (), (override));
        MOCK_METHOD(void, compositorCreateSurface, (IWaylandClient& client, uint32_t id), (override));
        MOCK_METHOD(void, compositorCreateRegion, (IWaylandClient& client, uint32_t id), (override));
    };
}
