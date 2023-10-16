//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "gmock/gmock.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandRegion.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandClient.h"

namespace ramses::internal
{
    class WaylandRegionMock : public IWaylandRegion
    {
    public:
        MOCK_METHOD(void, resourceDestroyed, (), (override));
        MOCK_METHOD(void, regionAdd, (IWaylandClient& client, int32_t x, int32_t y, int32_t width, int32_t height), (override));
        MOCK_METHOD(void, regionSubtract, (IWaylandClient& client, int32_t x, int32_t y, int32_t width, int32_t height), (override));
    };
}
