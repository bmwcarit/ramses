
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
        MOCK_METHOD3(uploadStreamTexture, void(StreamTextureHandle handle, StreamTextureSourceId source, SceneId sceneId));
        MOCK_METHOD3(deleteStreamTexture, void(StreamTextureHandle handle, StreamTextureSourceId source, SceneId sceneId));
        MOCK_METHOD3(dispatchStateChangesOfStreamTexturesAndSources, void(SceneStreamTextures& updatedStreamTextures, StreamTextureSourceIdVector& newStreams, StreamTextureSourceIdVector& obsoleteStreams));
        MOCK_METHOD0(processClientRequests, void());
        MOCK_CONST_METHOD0(hasUpdatedContentFromStreamSourcesToUpload, bool());
        MOCK_METHOD2(uploadResourcesAndGetUpdates, void(UpdatedSceneIdSet& updatedScenes, StreamTextureBufferUpdates& updatedStreamTextures));
        MOCK_METHOD0(notifyClients, void());
        MOCK_CONST_METHOD1(getCompositedTextureDeviceHandleForStreamTexture, DeviceResourceHandle(StreamTextureSourceId source));
        MOCK_CONST_METHOD0(hasRealCompositor, bool()); //TODO Mohamed: remove this as soon as EC dummy is removed
    };

    typedef ::testing::NiceMock<EmbeddedCompositingManagerMock> EmbeddedCompositingManagerMockNiceMock;
    typedef ::testing::StrictMock<EmbeddedCompositingManagerMock> EmbeddedCompositingManagerMockStrictMock;
}
#endif
