//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformInterface/IEmbeddedCompositor.h"
#include "gmock/gmock.h"

namespace ramses::internal
{
    class EmbeddedCompositorMock : public IEmbeddedCompositor
    {
    public:
        EmbeddedCompositorMock();
        ~EmbeddedCompositorMock() override;

        MOCK_METHOD(bool, init, ());
        MOCK_METHOD(void, handleRequestsFromClients, (), (override));
        MOCK_METHOD(bool, hasUpdatedStreamTextureSources, (), (const, override));
        MOCK_METHOD(WaylandIviSurfaceIdSet, dispatchUpdatedStreamTextureSourceIds, (), (override));
        MOCK_METHOD(WaylandIviSurfaceIdSet, dispatchNewStreamTextureSourceIds, (), (override));
        MOCK_METHOD(WaylandIviSurfaceIdSet, dispatchObsoleteStreamTextureSourceIds, (), (override));
        MOCK_METHOD(void, endFrame, (bool), (override));
        MOCK_METHOD(uint32_t, uploadCompositingContentForStreamTexture, (WaylandIviSurfaceId, DeviceResourceHandle textureHandle, ITextureUploadingAdapter&), (override));
        MOCK_METHOD(bool , isContentAvailableForStreamTexture, (WaylandIviSurfaceId), (const, override));
        MOCK_METHOD(uint64_t, getNumberOfCommitedFramesForWaylandIviSurfaceSinceBeginningOfTime, (WaylandIviSurfaceId), (const, override));
        MOCK_METHOD(bool, isBufferAttachedToWaylandIviSurface, (WaylandIviSurfaceId), (const, override));
        MOCK_METHOD(uint32_t, getNumberOfCompositorConnections, (), (const, override));
        MOCK_METHOD(bool , hasSurfaceForStreamTexture, (WaylandIviSurfaceId), (const, override));
        MOCK_METHOD(std::string, getTitleOfWaylandIviSurface, (WaylandIviSurfaceId), (const, override));
        MOCK_METHOD(void, logInfos, (RendererLogContext&), (const, override));
        MOCK_METHOD(void, logPeriodicInfo, (StringOutputStream&), (const, override));
        MOCK_METHOD(bool, isRealCompositor, (), (const, override));
    };
}
