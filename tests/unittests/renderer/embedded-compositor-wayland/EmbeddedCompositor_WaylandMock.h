//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "gmock/gmock.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IEmbeddedCompositor_Wayland.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandCompositorConnection.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandBuffer.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandSurface.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandRegion.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandBufferResource.h"

namespace ramses::internal
{
    class EmbeddedCompositor_WaylandMock : public IEmbeddedCompositor_Wayland
    {
    public:
        MOCK_METHOD(void, handleBufferDestroyed, (IWaylandBuffer& buffer), (override));
        MOCK_METHOD(void, addWaylandSurface, (IWaylandSurface& waylandSurface), (override));
        MOCK_METHOD(void, removeWaylandSurface, (IWaylandSurface& waylandSurface), (override));
        MOCK_METHOD(IWaylandBuffer&, getOrCreateBuffer, (WaylandBufferResource& bufferResource), (override));
        MOCK_METHOD(void, addWaylandCompositorConnection, (IWaylandCompositorConnection& waylandCompositorConnection), (override));
        MOCK_METHOD(void, removeWaylandCompositorConnection, (IWaylandCompositorConnection& waylandCompositorConnection), (override));
        MOCK_METHOD(void, addWaylandRegion, (IWaylandRegion& waylandRegion), (override));
        MOCK_METHOD(void, removeWaylandRegion, (IWaylandRegion& waylandRegion), (override));
    };
}
