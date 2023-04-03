//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EMBEDDEDCOMPOSITORMOCK_H
#define RAMSES_EMBEDDEDCOMPOSITORMOCK_H

#include "renderer_common_gmock_header.h"
#include "gmock/gmock.h"
#include "RendererAPI/IEmbeddedCompositor.h"
#include "RendererLib/RendererLogContext.h"
#include "RendererAPI/ITextureUploadingAdapter.h"

namespace ramses_internal
{
    class EmbeddedCompositorMock : public IEmbeddedCompositor
    {
    public:
        EmbeddedCompositorMock();
        ~EmbeddedCompositorMock() override;

        MOCK_METHOD(Bool, init, ());
        MOCK_METHOD(void, handleRequestsFromClients, (), (override));
        MOCK_METHOD(bool, hasUpdatedStreamTextureSources, (), (const, override));
        MOCK_METHOD(WaylandIviSurfaceIdSet, dispatchUpdatedStreamTextureSourceIds, (), (override));
        MOCK_METHOD(WaylandIviSurfaceIdSet, dispatchNewStreamTextureSourceIds, (), (override));
        MOCK_METHOD(WaylandIviSurfaceIdSet, dispatchObsoleteStreamTextureSourceIds, (), (override));
        MOCK_METHOD(void, endFrame, (Bool), (override));
        MOCK_METHOD(UInt32, uploadCompositingContentForStreamTexture, (WaylandIviSurfaceId, DeviceResourceHandle textureHandle, ITextureUploadingAdapter&), (override));
        MOCK_METHOD(Bool , isContentAvailableForStreamTexture, (WaylandIviSurfaceId), (const, override));
        MOCK_METHOD(UInt64, getNumberOfCommitedFramesForWaylandIviSurfaceSinceBeginningOfTime, (WaylandIviSurfaceId), (const, override));
        MOCK_METHOD(Bool, isBufferAttachedToWaylandIviSurface, (WaylandIviSurfaceId), (const, override));
        MOCK_METHOD(UInt32, getNumberOfCompositorConnections, (), (const, override));
        MOCK_METHOD(Bool , hasSurfaceForStreamTexture, (WaylandIviSurfaceId), (const, override));
        MOCK_METHOD(String , getTitleOfWaylandIviSurface, (WaylandIviSurfaceId), (const, override));
        MOCK_METHOD(void, logInfos, (RendererLogContext&), (const, override));
        MOCK_METHOD(bool, isRealCompositor, (), (const, override));
    };
}

#endif
