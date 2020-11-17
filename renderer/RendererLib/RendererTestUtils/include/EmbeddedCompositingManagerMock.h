
//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EMBEDDEDCOMPOSITINGMANAGERMOCK_H
#define RAMSES_EMBEDDEDCOMPOSITINGMANAGERMOCK_H

#include "renderer_common_gmock_header.h"
#include "gmock/gmock.h"
#include "RendererAPI/IEmbeddedCompositingManager.h"

namespace ramses_internal
{
    using namespace testing;

    class EmbeddedCompositingManagerMock : public IEmbeddedCompositingManager
    {
    public:
        EmbeddedCompositingManagerMock();
        MOCK_METHOD(void, refStream, (StreamTextureHandle handle, WaylandIviSurfaceId source, SceneId sceneId), (override));
        MOCK_METHOD(void, unrefStream, (StreamTextureHandle handle, WaylandIviSurfaceId source, SceneId sceneId), (override));
        MOCK_METHOD(void, refStream, (WaylandIviSurfaceId source), (override));
        MOCK_METHOD(void, unrefStream, (WaylandIviSurfaceId source), (override));
        MOCK_METHOD(void, dispatchStateChangesOfStreamTexturesAndSources, (SceneStreamTextures& updatedStreamTextures, WaylandIviSurfaceIdVector& newStreams, WaylandIviSurfaceIdVector& obsoleteStreams), (override));
        MOCK_METHOD(void, processClientRequests, (), (override));
        MOCK_METHOD(bool, hasUpdatedContentFromStreamSourcesToUpload, (), (const, override));
        MOCK_METHOD(void, uploadResourcesAndGetUpdates, (UpdatedSceneIdSet& updatedScenes, StreamTextureBufferUpdates& updatedStreamTextures), (override));
        MOCK_METHOD(void, notifyClients, (), (override));
        MOCK_METHOD(DeviceResourceHandle, getCompositedTextureDeviceHandleForStreamTexture, (WaylandIviSurfaceId source), (const, override));
        MOCK_METHOD(bool, hasRealCompositor, (), (const, override));
    };
}
#endif
