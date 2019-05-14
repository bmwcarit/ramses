//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "gtest/gtest.h"
#include "RendererLib/EmbeddedCompositingManager.h"
#include "DeviceMock.h"
#include "EmbeddedCompositorMock.h"
#include "TextureUploadingAdapterMock.h"

using namespace testing;
using namespace ramses_internal;

class AnEmbeddedCompositingManager : public ::testing::Test
{
public:
    AnEmbeddedCompositingManager()
    {
    }

    void uploadStreamTexture(SceneId scene, StreamTextureHandle streamTextureHandle, StreamTextureSourceId sourceId, DeviceResourceHandle textureDeviceHandle, bool expectTextureUpload = true)
    {
        if (expectTextureUpload)
        {
            EXPECT_CALL(deviceMock, uploadStreamTexture2D(_, _, _, _, _)).WillOnce(Return(textureDeviceHandle));
            EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(sourceId)).WillOnce(Return(false));
        }
        embeddedCompositingManager.uploadStreamTexture(streamTextureHandle, sourceId, scene);
    }

    void deleteStreamTexture(SceneId scene, StreamTextureHandle streamTextureHandle, StreamTextureSourceId sourceId, DeviceResourceHandle textureDeviceHandle, bool expectTextureUnload = true)
    {
        if (expectTextureUnload)
        {
            EXPECT_CALL(deviceMock, deleteTexture(textureDeviceHandle));
        }
        embeddedCompositingManager.deleteStreamTexture(streamTextureHandle, sourceId, scene);
    }

    void expectNoStreamTextureChangedState()
    {
        SceneStreamTextures updatedStreamTexturesPerScene;
        StreamTextureSourceIdVector newStreams;
        StreamTextureSourceIdVector obsoleteStreams;
        embeddedCompositingManager.dispatchStateChangesOfStreamTexturesAndSources(updatedStreamTexturesPerScene, newStreams, obsoleteStreams);

        EXPECT_EQ(0u, newStreams.size());
        EXPECT_EQ(0u, obsoleteStreams.size());
    }

    void expectStreamTextureChangedState(const StreamTextureSourceIdVector& expectedNewStreams, const StreamTextureSourceIdVector& expectedObsoleteStreams)
    {
        SceneStreamTextures updatedStreamTexturesPerScene;
        StreamTextureSourceIdVector newStreams;
        StreamTextureSourceIdVector obsoleteStreams;
        embeddedCompositingManager.dispatchStateChangesOfStreamTexturesAndSources(updatedStreamTexturesPerScene, newStreams, obsoleteStreams);

        EXPECT_EQ(expectedNewStreams, newStreams);
        EXPECT_EQ(expectedObsoleteStreams, obsoleteStreams);
    }

protected:
    StrictMock<DeviceMock> deviceMock;
    StrictMock<EmbeddedCompositorMock> embeddedCompositorMock;
    StrictMock<TextureUploadingAdapterMock> textureUploadingAdapterMock;
    EmbeddedCompositingManager embeddedCompositingManager       = EmbeddedCompositingManager(deviceMock, embeddedCompositorMock, textureUploadingAdapterMock);
    const StreamTextureSourceId streamTextureSourceId           = StreamTextureSourceId(11u);
    const StreamTextureSourceId streamTextureSourceId2          = StreamTextureSourceId(12u);
    const DeviceResourceHandle compositedTextureDeviceHandle    = DeviceResourceHandle(111u);
    const SceneId sceneId                                       = SceneId(30u);
    const SceneId sceneId2                                      = SceneId(31u);
    const StreamTextureHandle streamTexture                     = StreamTextureHandle(22u);
    const StreamTextureHandle streamTexture2                    = StreamTextureHandle(23u);
};

TEST_F(AnEmbeddedCompositingManager, CanUploadAndDeleteStreamTexture)
{
    uploadStreamTexture(sceneId, streamTexture, streamTextureSourceId, compositedTextureDeviceHandle);
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillOnce(Return(true));
    EXPECT_EQ(compositedTextureDeviceHandle, embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(streamTextureSourceId));

    deleteStreamTexture(sceneId, streamTexture, streamTextureSourceId, compositedTextureDeviceHandle);
}

TEST_F(AnEmbeddedCompositingManager, ReturnsInvalidDeviceResourceHandleForInvalidStreamSourceId)
{
    const StreamTextureSourceId fakeId(234);

    //simulate compositing content is not available
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(fakeId)).WillOnce(Return(false));
    EXPECT_EQ(DeviceResourceHandle::Invalid(), embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(fakeId));
}

TEST_F(AnEmbeddedCompositingManager, CanGetCorrectDeviceHandleForValidStreamTextureId)
{
    uploadStreamTexture(sceneId, streamTexture, streamTextureSourceId, compositedTextureDeviceHandle);

    //simulate compositing content is available
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillOnce(Return(true));
    //expect composited texture
    EXPECT_EQ(compositedTextureDeviceHandle, embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(streamTextureSourceId));

    //simulate compositing content is NOT available
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillOnce(Return(false));
    //expect invalid texture handle
    EXPECT_EQ(DeviceResourceHandle::Invalid(), embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(streamTextureSourceId));

    deleteStreamTexture(sceneId, streamTexture, streamTextureSourceId, compositedTextureDeviceHandle);
}

TEST_F(AnEmbeddedCompositingManager, CanUpdateCompositingResources)
{
    StreamTextureSourceIdSet fakeUpdatedStreamTextureSourceIds;
    fakeUpdatedStreamTextureSourceIds.put(streamTextureSourceId);

    uploadStreamTexture(sceneId, streamTexture, streamTextureSourceId, compositedTextureDeviceHandle);

    InSequence s;
    EXPECT_CALL(embeddedCompositorMock, handleRequestsFromClients());
    EXPECT_CALL(embeddedCompositorMock, endFrame(false));
    embeddedCompositingManager.processClientRequests();

    EXPECT_CALL(embeddedCompositorMock, hasUpdatedStreamTextureSources()).WillOnce(Return(true));
    EXPECT_TRUE(embeddedCompositingManager.hasUpdatedContentFromStreamSourcesToUpload());

    UpdatedSceneIdSet updatedScenes;
    EXPECT_CALL(embeddedCompositorMock, dispatchUpdatedStreamTextureSourceIds()).WillOnce(Return(fakeUpdatedStreamTextureSourceIds));
    EXPECT_CALL(embeddedCompositorMock, uploadCompositingContentForStreamTexture(streamTextureSourceId, _, _));
    StreamTextureBufferUpdates updates;
    embeddedCompositingManager.uploadResourcesAndGetUpdates(updatedScenes, updates);
    ASSERT_EQ(1u, updatedScenes.count());
    EXPECT_EQ(sceneId, *updatedScenes.begin());
}

TEST_F(AnEmbeddedCompositingManager, CanNotifyClients)
{
    InSequence s;
    EXPECT_CALL(embeddedCompositorMock, endFrame(true));
    embeddedCompositingManager.notifyClients();
}

TEST_F(AnEmbeddedCompositingManager, UploadsOneCompositedTextureForTwoEqualStreamTexturesSourceIds)
{
    uploadStreamTexture(sceneId, streamTexture, streamTextureSourceId, compositedTextureDeviceHandle);
    uploadStreamTexture(sceneId, streamTexture2, streamTextureSourceId, compositedTextureDeviceHandle, false);

    //make sure it returns the composited texture device handle for the stream textures id
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillRepeatedly(Return(true));
    EXPECT_EQ(compositedTextureDeviceHandle, embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(streamTextureSourceId));
}

TEST_F(AnEmbeddedCompositingManager, UploadsTwoCompositedTextureForTwoDifferentStreamTextureSourceIds)
{
    uploadStreamTexture(sceneId, streamTexture, streamTextureSourceId, compositedTextureDeviceHandle);

    const StreamTextureSourceId secondStreamTextureSourceId(22);
    const DeviceResourceHandle secondCompositedTextureDeviceHandle(222);
    uploadStreamTexture(sceneId, streamTexture2, secondStreamTextureSourceId, secondCompositedTextureDeviceHandle);

    //make sure it returns a different composited texture device handle for each stream texture source id
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillOnce(Return(true));
    EXPECT_EQ(compositedTextureDeviceHandle, embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(streamTextureSourceId));

    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(secondStreamTextureSourceId)).WillOnce(Return(true));
    EXPECT_EQ(secondCompositedTextureDeviceHandle, embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(secondStreamTextureSourceId));
}

TEST_F(AnEmbeddedCompositingManager, DoesNotDeleteCompositedTextureThatIsStillReferenced)
{
    uploadStreamTexture(sceneId, streamTexture, streamTextureSourceId, compositedTextureDeviceHandle);
    uploadStreamTexture(sceneId, streamTexture2, streamTextureSourceId, compositedTextureDeviceHandle, false);

    deleteStreamTexture(sceneId, streamTexture, streamTextureSourceId, compositedTextureDeviceHandle, false);
}

TEST_F(AnEmbeddedCompositingManager, DeletesCompositedTextureThatHadSeveralReferencesWhenItHasNoMoreReferences)
{
    uploadStreamTexture(sceneId, streamTexture, streamTextureSourceId, compositedTextureDeviceHandle);
    uploadStreamTexture(sceneId, streamTexture2, streamTextureSourceId, compositedTextureDeviceHandle, false);

    deleteStreamTexture(sceneId, streamTexture, streamTextureSourceId, compositedTextureDeviceHandle, false);
    deleteStreamTexture(sceneId, streamTexture2, streamTextureSourceId, compositedTextureDeviceHandle);
}

TEST_F(AnEmbeddedCompositingManager, CanUploadAndDeleteStreamTextureWithAvailableContent)
{
    EXPECT_CALL(deviceMock, uploadStreamTexture2D(_, _, _, _, _)).WillOnce(Return(compositedTextureDeviceHandle));
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillOnce(Return(true));
    EXPECT_CALL(embeddedCompositorMock, uploadCompositingContentForStreamTexture(streamTextureSourceId, _, _));

    embeddedCompositingManager.uploadStreamTexture(streamTexture, streamTextureSourceId, sceneId);

    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillOnce(Return(true));
    EXPECT_EQ(compositedTextureDeviceHandle, embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(streamTextureSourceId));

    deleteStreamTexture(sceneId, streamTexture, streamTextureSourceId, compositedTextureDeviceHandle);
}

TEST_F(AnEmbeddedCompositingManager, CanDispatchrStreamTexturesStateChange_NoChange)
{
    const StreamTextureSourceIdSet empty;
    EXPECT_CALL(embeddedCompositorMock, dispatchNewStreamTextureSourceIds()).WillOnce(Return(empty));
    EXPECT_CALL(embeddedCompositorMock, dispatchObsoleteStreamTextureSourceIds()).WillOnce(Return(empty));

    expectNoStreamTextureChangedState();
}

TEST_F(AnEmbeddedCompositingManager, CanDispatchrStreamTexturesStateChange_IfStreamSourceBecomesAvailable)
{
    const StreamTextureSourceIdSet empty;
    StreamTextureSourceIdSet newStreams;
    newStreams.put(streamTextureSourceId);
    EXPECT_CALL(embeddedCompositorMock, dispatchNewStreamTextureSourceIds()).WillOnce(Return(newStreams));
    EXPECT_CALL(embeddedCompositorMock, dispatchObsoleteStreamTextureSourceIds()).WillOnce(Return(empty));

    expectStreamTextureChangedState({ streamTextureSourceId }, StreamTextureSourceIdVector());
}

TEST_F(AnEmbeddedCompositingManager, CanDispatchrStreamTexturesStateChange_EmptyAfterDispatching)
{
    const StreamTextureSourceIdSet empty;
    StreamTextureSourceIdSet newStreams;
    newStreams.put(streamTextureSourceId);
    EXPECT_CALL(embeddedCompositorMock, dispatchNewStreamTextureSourceIds()).WillOnce(Return(newStreams));
    EXPECT_CALL(embeddedCompositorMock, dispatchObsoleteStreamTextureSourceIds()).WillOnce(Return(empty));

    expectStreamTextureChangedState({ streamTextureSourceId }, StreamTextureSourceIdVector());

    EXPECT_CALL(embeddedCompositorMock, dispatchNewStreamTextureSourceIds()).WillOnce(Return(empty));
    EXPECT_CALL(embeddedCompositorMock, dispatchObsoleteStreamTextureSourceIds()).WillOnce(Return(empty));
    expectNoStreamTextureChangedState();
}

TEST_F(AnEmbeddedCompositingManager, CanDispatchrStreamTexturesStateChange_IfStreamSourceBecomesUnavailable)
{
    const StreamTextureSourceIdSet empty;
    StreamTextureSourceIdSet obsoleteStreams;
    obsoleteStreams.put(streamTextureSourceId);
    EXPECT_CALL(embeddedCompositorMock, dispatchNewStreamTextureSourceIds()).WillOnce(Return(empty));
    EXPECT_CALL(embeddedCompositorMock, dispatchObsoleteStreamTextureSourceIds()).WillOnce(Return(obsoleteStreams));

    expectStreamTextureChangedState(StreamTextureSourceIdVector(), { streamTextureSourceId });
}

TEST_F(AnEmbeddedCompositingManager, CanDispatchrStreamTexturesStateChange_MultipleStreamTexturesInSameScene)
{
    uploadStreamTexture(sceneId, streamTexture, streamTextureSourceId, compositedTextureDeviceHandle);
    uploadStreamTexture(sceneId, streamTexture2, streamTextureSourceId, compositedTextureDeviceHandle, false);

    const StreamTextureSourceIdSet empty;
    StreamTextureSourceIdSet newStreams;
    newStreams.put(streamTextureSourceId);
    EXPECT_CALL(embeddedCompositorMock, dispatchNewStreamTextureSourceIds()).WillOnce(Return(newStreams));
    EXPECT_CALL(embeddedCompositorMock, dispatchObsoleteStreamTextureSourceIds()).WillOnce(Return(empty));
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillOnce(Return(true));

    SceneStreamTextures updatedStreamTexturesPerScene;
    StreamTextureSourceIdVector newStreamsResult;
    StreamTextureSourceIdVector obsoleteStreamsResult;
    embeddedCompositingManager.dispatchStateChangesOfStreamTexturesAndSources(updatedStreamTexturesPerScene, newStreamsResult, obsoleteStreamsResult);

    ASSERT_EQ(1u, updatedStreamTexturesPerScene.count());
    ASSERT_TRUE(updatedStreamTexturesPerScene.contains(sceneId));
    EXPECT_EQ(streamTexture, (*updatedStreamTexturesPerScene.get(sceneId))[0]);
    EXPECT_EQ(streamTexture2, (*updatedStreamTexturesPerScene.get(sceneId))[1]);

    ASSERT_EQ(1u, newStreamsResult.size());
    EXPECT_EQ(streamTextureSourceId, newStreamsResult[0]);
    EXPECT_EQ(0u, obsoleteStreamsResult.size());
}

TEST_F(AnEmbeddedCompositingManager, CanDispatchrStreamTexturesStateChange_MultipleStreamTexturesInMultipleScenes)
{
    uploadStreamTexture(sceneId, streamTexture, streamTextureSourceId, compositedTextureDeviceHandle);
    uploadStreamTexture(sceneId, streamTexture2, streamTextureSourceId2, compositedTextureDeviceHandle);

    uploadStreamTexture(sceneId2, streamTexture, streamTextureSourceId2, compositedTextureDeviceHandle, false);
    uploadStreamTexture(sceneId2, streamTexture2, streamTextureSourceId, compositedTextureDeviceHandle, false);


    const StreamTextureSourceIdSet empty;
    StreamTextureSourceIdSet newStreams;
    newStreams.put(streamTextureSourceId);
    newStreams.put(streamTextureSourceId2);
    EXPECT_CALL(embeddedCompositorMock, dispatchNewStreamTextureSourceIds()).WillOnce(Return(newStreams));
    EXPECT_CALL(embeddedCompositorMock, dispatchObsoleteStreamTextureSourceIds()).WillOnce(Return(empty));
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillOnce(Return(true));
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId2)).WillOnce(Return(false));

    SceneStreamTextures updatedStreamTexturesPerScene;
    StreamTextureSourceIdVector newStreamsResult;
    StreamTextureSourceIdVector obsoleteStreamsResult;
    embeddedCompositingManager.dispatchStateChangesOfStreamTexturesAndSources(updatedStreamTexturesPerScene, newStreamsResult, obsoleteStreamsResult);

    ASSERT_EQ(2u, updatedStreamTexturesPerScene.count());
    ASSERT_TRUE(updatedStreamTexturesPerScene.contains(sceneId));
    EXPECT_EQ(streamTexture, (*updatedStreamTexturesPerScene.get(sceneId))[0]);

    ASSERT_TRUE(updatedStreamTexturesPerScene.contains(sceneId2));
    EXPECT_EQ(streamTexture2, (*updatedStreamTexturesPerScene.get(sceneId2))[0]);

    ASSERT_EQ(2u, newStreamsResult.size());
    EXPECT_NE(newStreamsResult.end(), find_c(newStreamsResult, streamTextureSourceId));
    EXPECT_NE(newStreamsResult.end(), find_c(newStreamsResult, streamTextureSourceId2));
    EXPECT_EQ(0u, obsoleteStreamsResult.size());
}
