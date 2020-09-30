//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WAYLANDSHELLSURFACEMOCK_H
#define RAMSES_WAYLANDSHELLSURFACEMOCK_H

#include "gmock/gmock.h"
#include "EmbeddedCompositor_Wayland/IWaylandShellSurface.h"
#include "EmbeddedCompositor_Wayland/IWaylandClient.h"

namespace ramses_internal
{
    class WaylandShellSurfaceMock : public IWaylandShellSurface
    {
    public:
        MOCK_METHOD(void, resourceDestroyed, (), (override));
        MOCK_METHOD(void, shellSurfacePong, (IWaylandClient& client, uint32_t serial), (override));
        MOCK_METHOD(void, shellSurfaceMove, (IWaylandClient& client, INativeWaylandResource& seatResource, uint32_t serial), (override));
        MOCK_METHOD(void, shellSurfaceResize, (IWaylandClient& client, INativeWaylandResource& seatResource, uint32_t serial, uint32_t edges), (override));
        MOCK_METHOD(void, shellSurfaceSetToplevel, (IWaylandClient& client), (override));
        MOCK_METHOD(void, shellSurfaceSetTransient, (IWaylandClient& client, INativeWaylandResource& parentSurfaceResource, int32_t x, int32_t y, uint32_t flags), (override));
        MOCK_METHOD(void, shellSurfaceSetFullscreen, (IWaylandClient& client, uint32_t method, uint32_t framerate), (override));
        MOCK_METHOD(void, shellSurfaceSetPopup, (IWaylandClient& client, INativeWaylandResource& seatResource, uint32_t serial, INativeWaylandResource& parentSurfaceResource, int32_t x, int32_t y, uint32_t flags), (override));
        MOCK_METHOD(void, shellSurfaceSetMaximized, (IWaylandClient& client), (override));
        MOCK_METHOD(void, shellSurfaceSetTitle, (IWaylandClient& client, const char* title), (override));
        MOCK_METHOD(void, shellSurfaceSetClass, (IWaylandClient& client, const char* className), (override));
        MOCK_METHOD(void, surfaceWasDeleted, (), (override));
        MOCK_METHOD(String&, getTitle, (), (const, override));
    };
}

#endif
