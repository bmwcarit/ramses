//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WAYLANDSURFACEMOCK_H
#define RAMSES_WAYLANDSURFACEMOCK_H

#include "gmock/gmock.h"
#include "EmbeddedCompositor_Wayland/IWaylandSurface.h"
#include "EmbeddedCompositor_Wayland/WaylandBufferResource.h"
#include "EmbeddedCompositor_Wayland/IWaylandBuffer.h"
#include "EmbeddedCompositor_Wayland/IWaylandClient.h"
#include "RendererLib/RendererLogContext.h"

namespace ramses_internal
{
    class WaylandSurfaceMock : public IWaylandSurface
    {
    public:
        MOCK_METHOD0(resourceDestroyed, void());
        MOCK_METHOD4(surfaceAttach, void(IWaylandClient& client, WaylandBufferResource& bufferResource, int x, int y));
        MOCK_METHOD1(surfaceDetach, void(IWaylandClient& client));
        MOCK_METHOD5(surfaceDamage, void(IWaylandClient& client, int x, int y, int width, int height));
        MOCK_METHOD2(surfaceFrame, void(IWaylandClient& client, uint32_t id));
        MOCK_METHOD2(surfaceSetOpaqueRegion, void(IWaylandClient& client, IWaylandResource* regionResource));
        MOCK_METHOD2(surfaceSetInputRegion, void(IWaylandClient& client, IWaylandResource* regionResource));
        MOCK_METHOD1(surfaceCommit, void(IWaylandClient& client));
        MOCK_METHOD2(surfaceSetBufferTransform, void(IWaylandClient& client, int32_t transform));
        MOCK_METHOD2(surfaceSetBufferScale, void(IWaylandClient& client, int32_t scale));
        MOCK_METHOD5(surfaceDamageBuffer, void(IWaylandClient& client, int32_t x, int32_t y, int32_t width, int32_t height));
        MOCK_METHOD1(setShellSurface, void(IWaylandShellSurface* shellSurface));
        MOCK_CONST_METHOD0(hasShellSurface, bool());
        MOCK_CONST_METHOD1(logInfos, void(RendererLogContext& context));
        MOCK_CONST_METHOD0(getIviSurfaceId, WaylandIviSurfaceId());
        MOCK_METHOD1(sendFrameCallbacks, void(UInt32 time));
        MOCK_CONST_METHOD0(getWaylandBuffer, IWaylandBuffer*());
        MOCK_CONST_METHOD0(getNumberOfCommitedFrames, UInt64());
        MOCK_METHOD0(resetNumberOfCommitedFrames, void());
        MOCK_CONST_METHOD0(getNumberOfCommitedFramesSinceBeginningOfTime, UInt64());
        MOCK_CONST_METHOD0(hasPendingBuffer, bool());
        MOCK_CONST_METHOD0(getSurfaceTitle, const String&());
        MOCK_CONST_METHOD0(getProcessId, pid_t());
        MOCK_METHOD1(bufferDestroyed, void(IWaylandBuffer& buffer));
        MOCK_METHOD1(setIviSurface, void(IWaylandIVISurface* iviSurface));
        MOCK_CONST_METHOD0(hasIviSurface, bool());
    };
}

#endif
