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
        ~EmbeddedCompositorMock();

        MOCK_METHOD0(init, Bool());
        MOCK_METHOD0(handleRequestsFromClients, void());
        MOCK_CONST_METHOD0(hasUpdatedStreamTextureSources, bool());
        MOCK_METHOD0(dispatchUpdatedStreamTextureSourceIds, StreamTextureSourceIdSet());
        MOCK_METHOD0(dispatchNewStreamTextureSourceIds, StreamTextureSourceIdSet());
        MOCK_METHOD0(dispatchObsoleteStreamTextureSourceIds, StreamTextureSourceIdSet());
        MOCK_METHOD1(endFrame, void(Bool));
        MOCK_METHOD3(uploadCompositingContentForStreamTexture, UInt32(StreamTextureSourceId, DeviceResourceHandle textureHandle, ITextureUploadingAdapter&));
        MOCK_CONST_METHOD1(isContentAvailableForStreamTexture, Bool (StreamTextureSourceId));
        MOCK_CONST_METHOD1(getNumberOfCommitedFramesForWaylandIviSurfaceSinceBeginningOfTime, UInt64(WaylandIviSurfaceId));
        MOCK_CONST_METHOD1(isBufferAttachedToWaylandIviSurface, Bool(WaylandIviSurfaceId));
        MOCK_CONST_METHOD0(getNumberOfCompositorConnections, UInt32());
        MOCK_CONST_METHOD1(hasSurfaceForStreamTexture, Bool (StreamTextureSourceId));
        MOCK_CONST_METHOD1(getTitleOfWaylandIviSurface, String (WaylandIviSurfaceId));
        MOCK_CONST_METHOD1(logInfos, void(RendererLogContext&));
        MOCK_CONST_METHOD0(isRealCompositor, bool()); //TODO Mohamed: remove this when dummy EC is removed
    };

    class EmbeddedCompositorMockWithDestructor : public EmbeddedCompositorMock
    {
    public:
        EmbeddedCompositorMockWithDestructor();
        ~EmbeddedCompositorMockWithDestructor();

        MOCK_METHOD0(Die, void());
    };
}

#endif
