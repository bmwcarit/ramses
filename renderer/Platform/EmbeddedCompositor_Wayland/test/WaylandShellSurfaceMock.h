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
        MOCK_METHOD0(resourceDestroyed, void());
        MOCK_METHOD2(shellSurfacePong, void(IWaylandClient& client, uint32_t serial));
        MOCK_METHOD3(shellSurfaceMove, void(IWaylandClient& client, IWaylandResource& seatResource, uint32_t serial));
        MOCK_METHOD4(shellSurfaceResize, void(IWaylandClient& client, IWaylandResource& seatResource, uint32_t serial, uint32_t edges));
        MOCK_METHOD1(shellSurfaceSetToplevel, void(IWaylandClient& client));
        MOCK_METHOD5(shellSurfaceSetTransient, void(IWaylandClient& client, IWaylandResource& parentSurfaceResource, int32_t x, int32_t y, uint32_t flags));
        MOCK_METHOD3(shellSurfaceSetFullscreen, void(IWaylandClient& client, uint32_t method, uint32_t framerate));
        MOCK_METHOD7(shellSurfaceSetPopup, void(IWaylandClient& client, IWaylandResource& seatResource, uint32_t serial, IWaylandResource& parentSurfaceResource, int32_t x, int32_t y, uint32_t flags));
        MOCK_METHOD1(shellSurfaceSetMaximized, void(IWaylandClient& client));
        MOCK_METHOD2(shellSurfaceSetTitle, void(IWaylandClient& client, const char* title));
        MOCK_METHOD2(shellSurfaceSetClass, void(IWaylandClient& client, const char* className));
        MOCK_METHOD0(surfaceWasDeleted, void());
        MOCK_CONST_METHOD0(getTitle, String&());
    };
}

#endif
