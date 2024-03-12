//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WAYLANDSURFACEMOCK_H
#define RAMSES_WAYLANDSURFACEMOCK_H

#include "EmbeddedCompositor_Wayland/IWaylandSurface.h"
#include "EmbeddedCompositor_Wayland/WaylandBufferResource.h"
#include "EmbeddedCompositor_Wayland/IWaylandBuffer.h"
#include "EmbeddedCompositor_Wayland/IWaylandClient.h"
#include "RendererLib/RendererLogContext.h"
#include "gmock/gmock.h"

namespace ramses_internal
{
    class WaylandSurfaceMock : public IWaylandSurface
    {
    public:
        MOCK_METHOD(void, resourceDestroyed, (), (override));
        MOCK_METHOD(void, surfaceAttach, (IWaylandClient& client, WaylandBufferResource& bufferResource, int x, int y), (override));
        MOCK_METHOD(void, surfaceDetach, (IWaylandClient& client), (override));
        MOCK_METHOD(void, surfaceDamage, (IWaylandClient& client, int x, int y, int width, int height), (override));
        MOCK_METHOD(void, surfaceFrame, (IWaylandClient& client, uint32_t id), (override));
        MOCK_METHOD(void, surfaceSetOpaqueRegion, (IWaylandClient& client, INativeWaylandResource* regionResource), (override));
        MOCK_METHOD(void, surfaceSetInputRegion, (IWaylandClient& client, INativeWaylandResource* regionResource), (override));
        MOCK_METHOD(void, surfaceCommit, (IWaylandClient& client), (override));
        MOCK_METHOD(void, surfaceSetBufferTransform, (IWaylandClient& client, int32_t transform), (override));
        MOCK_METHOD(void, surfaceSetBufferScale, (IWaylandClient& client, int32_t scale), (override));
        MOCK_METHOD(void, surfaceDamageBuffer, (IWaylandClient& client, int32_t x, int32_t y, int32_t width, int32_t height), (override));
        MOCK_METHOD(void, setShellSurface, (IWaylandShellSurface* shellSurface), (override));
        MOCK_METHOD(bool, hasShellSurface, (), (const, override));
        MOCK_METHOD(void, logInfos, (RendererLogContext& context, const WaylandEGLExtensionProcs&), (const, override));
        MOCK_METHOD(WaylandIviSurfaceId, getIviSurfaceId, (), (const, override));
        MOCK_METHOD(void, sendFrameCallbacks, (UInt32 time), (override));
        MOCK_METHOD(IWaylandBuffer*, getWaylandBuffer, (), (const, override));
        MOCK_METHOD(UInt32, getNumberOfCommitedFrames, (), (const, override));
        MOCK_METHOD(void, resetNumberOfCommitedFrames, (), (override));
        MOCK_METHOD(UInt64, getNumberOfCommitedFramesSinceBeginningOfTime, (), (const, override));
        MOCK_METHOD(bool, hasPendingBuffer, (), (const, override));
        MOCK_METHOD(const String&, getSurfaceTitle, (), (const, override));
        MOCK_METHOD(void, bufferDestroyed, (IWaylandBuffer& buffer), (override));
        MOCK_METHOD(void, setIviSurface, (IWaylandIVISurface* iviSurface), (override));
        MOCK_METHOD(bool, hasIviSurface, (), (const, override));
        MOCK_METHOD(WaylandClientCredentials, getClientCredentials, (), (const, override));
        MOCK_METHOD(bool, dispatchBufferTypeChanged, (), (override));
    };
}

#endif
