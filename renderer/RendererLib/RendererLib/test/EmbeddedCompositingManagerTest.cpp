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
#include "Platform_Base/TextureUploadingAdapter_Base.h"

using namespace testing;
using namespace ramses_internal;

class AnEmbeddedCompositingManager : public ::testing::Test
{
public:
    void expectStreamTexUpload(WaylandIviSurfaceId sourceId, DeviceResourceHandle textureDeviceHandle = compositedTextureDeviceHandle)
    {
        EXPECT_CALL(deviceMock, uploadStreamTexture2D(_, _, _, _, _, _)).WillOnce(Return(textureDeviceHandle));
        EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(sourceId)).WillOnce(Return(false));
    }

    void expectStreamTexUnload(DeviceResourceHandle textureDeviceHandle = compositedTextureDeviceHandle)
    {
        EXPECT_CALL(deviceMock, deleteTexture(textureDeviceHandle));
    }

    void addSceneReferenceToStreamTexture(SceneId scene, StreamTextureHandle streamTextureHandle, WaylandIviSurfaceId sourceId, DeviceResourceHandle textureDeviceHandle, bool expectTextureUpload = true)
    {
        if (expectTextureUpload)
            expectStreamTexUpload(sourceId, textureDeviceHandle);
        embeddedCompositingManager.refStream(streamTextureHandle, sourceId, scene);
    }

    void removeSceneReferenceToStreamTexture(SceneId scene, StreamTextureHandle streamTextureHandle, WaylandIviSurfaceId sourceId, DeviceResourceHandle textureDeviceHandle, bool expectTextureUnload = true)
    {
        if (expectTextureUnload)
            expectStreamTexUnload(textureDeviceHandle);
        embeddedCompositingManager.unrefStream(streamTextureHandle, sourceId, scene);
    }

    void addStreamBufferReferenceToStreamTexture(WaylandIviSurfaceId sourceId, DeviceResourceHandle textureDeviceHandle = compositedTextureDeviceHandle, bool expectTextureUpload = true)
    {
        if (expectTextureUpload)
            expectStreamTexUpload(sourceId, textureDeviceHandle);
        embeddedCompositingManager.refStream(sourceId);
    }

    void removeStreamBufferReferenceToStreamTexture(WaylandIviSurfaceId sourceId, DeviceResourceHandle textureDeviceHandle = compositedTextureDeviceHandle, bool expectTextureUnload = true)
    {
        if (expectTextureUnload)
            expectStreamTexUnload(textureDeviceHandle);
        embeddedCompositingManager.unrefStream(sourceId);
    }

    void expectNoStreamTextureChangedState()
    {
        SceneStreamTextures updatedStreamTexturesPerScene;
        WaylandIviSurfaceIdVector newStreams;
        WaylandIviSurfaceIdVector obsoleteStreams;
        embeddedCompositingManager.dispatchStateChangesOfStreamTexturesAndSources(updatedStreamTexturesPerScene, newStreams, obsoleteStreams);

        EXPECT_EQ(0u, newStreams.size());
        EXPECT_EQ(0u, obsoleteStreams.size());
    }

    void expectStreamTextureChangedState(const WaylandIviSurfaceIdVector& expectedNewStreams, const WaylandIviSurfaceIdVector& expectedObsoleteStreams)
    {
        SceneStreamTextures updatedStreamTexturesPerScene;
        WaylandIviSurfaceIdVector newStreams;
        WaylandIviSurfaceIdVector obsoleteStreams;
        embeddedCompositingManager.dispatchStateChangesOfStreamTexturesAndSources(updatedStreamTexturesPerScene, newStreams, obsoleteStreams);

        EXPECT_EQ(expectedNewStreams, newStreams);
        EXPECT_EQ(expectedObsoleteStreams, obsoleteStreams);
    }

protected:
    StrictMock<DeviceMock> deviceMock;
    StrictMock<EmbeddedCompositorMock> embeddedCompositorMock;
    TextureUploadingAdapter_Base textureUploadingAdapter = TextureUploadingAdapter_Base(deviceMock);
    EmbeddedCompositingManager embeddedCompositingManager       = EmbeddedCompositingManager(deviceMock, embeddedCompositorMock, textureUploadingAdapter);
    const WaylandIviSurfaceId streamTextureSourceId             = WaylandIviSurfaceId(11u);
    const WaylandIviSurfaceId streamTextureSourceId2            = WaylandIviSurfaceId(12u);
    const SceneId sceneId                                       = SceneId(30u);
    const SceneId sceneId2                                      = SceneId(31u);
    const StreamTextureHandle streamTexture                     = StreamTextureHandle(22u);
    const StreamTextureHandle streamTexture2                    = StreamTextureHandle(23u);

    static constexpr DeviceResourceHandle compositedTextureDeviceHandle{ 111u };
};

constexpr DeviceResourceHandle AnEmbeddedCompositingManager::compositedTextureDeviceHandle;

TEST_F(AnEmbeddedCompositingManager, CanUploadAndDeleteStreamTexture_scene)
{
    addSceneReferenceToStreamTexture(sceneId, streamTexture, streamTextureSourceId, compositedTextureDeviceHandle);
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillOnce(Return(true));
    EXPECT_EQ(compositedTextureDeviceHandle, embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(streamTextureSourceId));

    removeSceneReferenceToStreamTexture(sceneId, streamTexture, streamTextureSourceId, compositedTextureDeviceHandle);
}

TEST_F(AnEmbeddedCompositingManager, CanUploadAndDeleteStreamTexture_streamBuffer)
{
    addStreamBufferReferenceToStreamTexture(streamTextureSourceId);
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillOnce(Return(true));
    EXPECT_EQ(compositedTextureDeviceHandle, embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(streamTextureSourceId));

    removeStreamBufferReferenceToStreamTexture(streamTextureSourceId);
}

TEST_F(AnEmbeddedCompositingManager, ReturnsInvalidDeviceResourceHandleForInvalidStreamSourceId)
{
    const WaylandIviSurfaceId fakeId(234);

    //simulate compositing content is not available
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(fakeId)).WillOnce(Return(false));
    EXPECT_EQ(DeviceResourceHandle::Invalid(), embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(fakeId));
}

TEST_F(AnEmbeddedCompositingManager, CanGetCorrectDeviceHandleForValidStreamTextureId_scene)
{
    addSceneReferenceToStreamTexture(sceneId, streamTexture, streamTextureSourceId, compositedTextureDeviceHandle);

    //simulate compositing content is available
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillOnce(Return(true));
    //expect composited texture
    EXPECT_EQ(compositedTextureDeviceHandle, embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(streamTextureSourceId));

    //simulate compositing content is NOT available
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillOnce(Return(false));
    //expect invalid texture handle
    EXPECT_EQ(DeviceResourceHandle::Invalid(), embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(streamTextureSourceId));

    removeSceneReferenceToStreamTexture(sceneId, streamTexture, streamTextureSourceId, compositedTextureDeviceHandle);
}

TEST_F(AnEmbeddedCompositingManager, CanGetCorrectDeviceHandleForValidStreamTextureId_streamBuffer)
{
    addStreamBufferReferenceToStreamTexture(streamTextureSourceId);

    //simulate compositing content is available
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillOnce(Return(true));
    //expect composited texture
    EXPECT_EQ(compositedTextureDeviceHandle, embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(streamTextureSourceId));

    //simulate compositing content is NOT available
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillOnce(Return(false));
    //expect invalid texture handle
    EXPECT_EQ(DeviceResourceHandle::Invalid(), embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(streamTextureSourceId));

    removeStreamBufferReferenceToStreamTexture(streamTextureSourceId);
}

TEST_F(AnEmbeddedCompositingManager, CanUpdateCompositingResources)
{
    const WaylandIviSurfaceIdSet fakeUpdatedStreamTextureSourceIds{ streamTextureSourceId };

    addSceneReferenceToStreamTexture(sceneId, streamTexture, streamTextureSourceId, compositedTextureDeviceHandle);

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
    ASSERT_EQ(1u, updatedScenes.size());
    EXPECT_EQ(sceneId, *updatedScenes.begin());
}

TEST_F(AnEmbeddedCompositingManager, CanNotifyClients)
{
    InSequence s;
    EXPECT_CALL(embeddedCompositorMock, endFrame(true));
    embeddedCompositingManager.notifyClients();
}

TEST_F(AnEmbeddedCompositingManager, UploadsOneCompositedTextureForTwoEqualStreamTexturesSourceIds_scene)
{
    addSceneReferenceToStreamTexture(sceneId, streamTexture, streamTextureSourceId, compositedTextureDeviceHandle);
    addSceneReferenceToStreamTexture(sceneId, streamTexture2, streamTextureSourceId, compositedTextureDeviceHandle, false);

    //make sure it returns the composited texture device handle for the stream textures id
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillRepeatedly(Return(true));
    EXPECT_EQ(compositedTextureDeviceHandle, embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(streamTextureSourceId));
}

TEST_F(AnEmbeddedCompositingManager, UploadsOneCompositedTextureForTwoEqualStreamTexturesSourceIds_streamBuffer)
{
    addStreamBufferReferenceToStreamTexture(streamTextureSourceId, compositedTextureDeviceHandle);
    addStreamBufferReferenceToStreamTexture(streamTextureSourceId, compositedTextureDeviceHandle, false);

    //make sure it returns the composited texture device handle for the stream textures id
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillRepeatedly(Return(true));
    EXPECT_EQ(compositedTextureDeviceHandle, embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(streamTextureSourceId));
}

TEST_F(AnEmbeddedCompositingManager, UploadsOneCompositedTextureForTwoEqualStreamTexturesSourceIds_referencedBySceneAndStreamBuffer)
{
    addSceneReferenceToStreamTexture(sceneId, streamTexture, streamTextureSourceId, compositedTextureDeviceHandle);
    addStreamBufferReferenceToStreamTexture(streamTextureSourceId, compositedTextureDeviceHandle, false);

    //make sure it returns the composited texture device handle for the stream textures id
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillRepeatedly(Return(true));
    EXPECT_EQ(compositedTextureDeviceHandle, embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(streamTextureSourceId));
}

TEST_F(AnEmbeddedCompositingManager, UploadsTwoCompositedTextureForTwoDifferentStreamTextureSourceIds_scene)
{
    addSceneReferenceToStreamTexture(sceneId, streamTexture, streamTextureSourceId, compositedTextureDeviceHandle);

    const WaylandIviSurfaceId secondStreamTextureSourceId(22);
    const DeviceResourceHandle secondCompositedTextureDeviceHandle(222);
    addSceneReferenceToStreamTexture(sceneId, streamTexture2, secondStreamTextureSourceId, secondCompositedTextureDeviceHandle);

    //make sure it returns a different composited texture device handle for each stream texture source id
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillOnce(Return(true));
    EXPECT_EQ(compositedTextureDeviceHandle, embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(streamTextureSourceId));

    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(secondStreamTextureSourceId)).WillOnce(Return(true));
    EXPECT_EQ(secondCompositedTextureDeviceHandle, embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(secondStreamTextureSourceId));
}

TEST_F(AnEmbeddedCompositingManager, UploadsTwoCompositedTextureForTwoDifferentStreamTextureSourceIds_streamBuffer)
{
    addStreamBufferReferenceToStreamTexture(streamTextureSourceId, compositedTextureDeviceHandle);

    const WaylandIviSurfaceId secondStreamTextureSourceId(22);
    const DeviceResourceHandle secondCompositedTextureDeviceHandle(222);
    addStreamBufferReferenceToStreamTexture(secondStreamTextureSourceId, secondCompositedTextureDeviceHandle);

    //make sure it returns a different composited texture device handle for each stream texture source id
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillOnce(Return(true));
    EXPECT_EQ(compositedTextureDeviceHandle, embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(streamTextureSourceId));

    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(secondStreamTextureSourceId)).WillOnce(Return(true));
    EXPECT_EQ(secondCompositedTextureDeviceHandle, embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(secondStreamTextureSourceId));
}

TEST_F(AnEmbeddedCompositingManager, DeletesCompositedTextureOnlyAfterAllReferencesRemoved_scene)
{
    addSceneReferenceToStreamTexture(sceneId, streamTexture, streamTextureSourceId, compositedTextureDeviceHandle);
    addSceneReferenceToStreamTexture(sceneId, streamTexture2, streamTextureSourceId, compositedTextureDeviceHandle, false);

    removeSceneReferenceToStreamTexture(sceneId, streamTexture, streamTextureSourceId, compositedTextureDeviceHandle, false);
    removeSceneReferenceToStreamTexture(sceneId, streamTexture2, streamTextureSourceId, compositedTextureDeviceHandle);
}

TEST_F(AnEmbeddedCompositingManager, DeletesCompositedTextureOnlyAfterAllReferencesRemoved_streamBuffer)
{
    addStreamBufferReferenceToStreamTexture(streamTextureSourceId, compositedTextureDeviceHandle);
    addStreamBufferReferenceToStreamTexture(streamTextureSourceId, compositedTextureDeviceHandle, false);

    removeStreamBufferReferenceToStreamTexture(streamTextureSourceId, compositedTextureDeviceHandle, false);
    removeStreamBufferReferenceToStreamTexture(streamTextureSourceId, compositedTextureDeviceHandle);
}

TEST_F(AnEmbeddedCompositingManager, DeletesCompositedTextureOnlyAfterAllReferencesRemoved_referencedBySceneAndStreamBuffer)
{
    addSceneReferenceToStreamTexture(sceneId, streamTexture, streamTextureSourceId, compositedTextureDeviceHandle);
    addSceneReferenceToStreamTexture(sceneId, streamTexture2, streamTextureSourceId, compositedTextureDeviceHandle, false);
    addStreamBufferReferenceToStreamTexture(streamTextureSourceId, compositedTextureDeviceHandle, false);
    addStreamBufferReferenceToStreamTexture(streamTextureSourceId, compositedTextureDeviceHandle, false);

    removeSceneReferenceToStreamTexture(sceneId, streamTexture, streamTextureSourceId, compositedTextureDeviceHandle, false);
    removeSceneReferenceToStreamTexture(sceneId, streamTexture2, streamTextureSourceId, compositedTextureDeviceHandle, false);
    removeStreamBufferReferenceToStreamTexture(streamTextureSourceId, compositedTextureDeviceHandle, false);
    removeStreamBufferReferenceToStreamTexture(streamTextureSourceId, compositedTextureDeviceHandle);
}

TEST_F(AnEmbeddedCompositingManager, DeletesCompositedTextureOnlyAfterAllReferencesRemoved_referencedByStreamBufferAndScene)
{
    addStreamBufferReferenceToStreamTexture(streamTextureSourceId, compositedTextureDeviceHandle);
    addSceneReferenceToStreamTexture(sceneId, streamTexture, streamTextureSourceId, compositedTextureDeviceHandle, false);
    addStreamBufferReferenceToStreamTexture(streamTextureSourceId, compositedTextureDeviceHandle, false);
    addSceneReferenceToStreamTexture(sceneId, streamTexture2, streamTextureSourceId, compositedTextureDeviceHandle, false);

    removeStreamBufferReferenceToStreamTexture(streamTextureSourceId, compositedTextureDeviceHandle, false);
    removeSceneReferenceToStreamTexture(sceneId, streamTexture, streamTextureSourceId, compositedTextureDeviceHandle, false);
    removeStreamBufferReferenceToStreamTexture(streamTextureSourceId, compositedTextureDeviceHandle, false);
    removeSceneReferenceToStreamTexture(sceneId, streamTexture2, streamTextureSourceId, compositedTextureDeviceHandle);
}

TEST_F(AnEmbeddedCompositingManager, CanUploadAndDeleteStreamTextureWithAvailableContent_scene)
{
    EXPECT_CALL(deviceMock, uploadStreamTexture2D(_, _, _, _, _, _)).WillOnce(Return(compositedTextureDeviceHandle));
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillOnce(Return(true));
    EXPECT_CALL(embeddedCompositorMock, uploadCompositingContentForStreamTexture(streamTextureSourceId, _, _));

    embeddedCompositingManager.refStream(streamTexture, streamTextureSourceId, sceneId);

    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillOnce(Return(true));
    EXPECT_EQ(compositedTextureDeviceHandle, embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(streamTextureSourceId));

    removeSceneReferenceToStreamTexture(sceneId, streamTexture, streamTextureSourceId, compositedTextureDeviceHandle);
}

TEST_F(AnEmbeddedCompositingManager, CanUploadAndDeleteStreamTextureWithAvailableContent_streamBuffer)
{
    EXPECT_CALL(deviceMock, uploadStreamTexture2D(_, _, _, _, _, _)).WillOnce(Return(compositedTextureDeviceHandle));
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillOnce(Return(true));
    EXPECT_CALL(embeddedCompositorMock, uploadCompositingContentForStreamTexture(streamTextureSourceId, _, _));

    embeddedCompositingManager.refStream(streamTextureSourceId);

    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillOnce(Return(true));
    EXPECT_EQ(compositedTextureDeviceHandle, embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(streamTextureSourceId));

    removeStreamBufferReferenceToStreamTexture(streamTextureSourceId, compositedTextureDeviceHandle);
}

TEST_F(AnEmbeddedCompositingManager, CanDispatchrStreamTexturesStateChange_NoChange)
{
    const WaylandIviSurfaceIdSet empty;
    EXPECT_CALL(embeddedCompositorMock, dispatchNewStreamTextureSourceIds()).WillOnce(Return(empty));
    EXPECT_CALL(embeddedCompositorMock, dispatchObsoleteStreamTextureSourceIds()).WillOnce(Return(empty));

    expectNoStreamTextureChangedState();
}

TEST_F(AnEmbeddedCompositingManager, CanDispatchrStreamTexturesStateChange_IfStreamSourceBecomesAvailable)
{
    const WaylandIviSurfaceIdSet empty;
    const WaylandIviSurfaceIdSet newStreams{ streamTextureSourceId };
    EXPECT_CALL(embeddedCompositorMock, dispatchNewStreamTextureSourceIds()).WillOnce(Return(newStreams));
    EXPECT_CALL(embeddedCompositorMock, dispatchObsoleteStreamTextureSourceIds()).WillOnce(Return(empty));

    expectStreamTextureChangedState({ streamTextureSourceId }, {});
}

TEST_F(AnEmbeddedCompositingManager, CanDispatchrStreamTexturesStateChange_EmptyAfterDispatching)
{
    const WaylandIviSurfaceIdSet empty;
    const WaylandIviSurfaceIdSet newStreams{ streamTextureSourceId };
    EXPECT_CALL(embeddedCompositorMock, dispatchNewStreamTextureSourceIds()).WillOnce(Return(newStreams));
    EXPECT_CALL(embeddedCompositorMock, dispatchObsoleteStreamTextureSourceIds()).WillOnce(Return(empty));

    expectStreamTextureChangedState({ streamTextureSourceId }, {});

    EXPECT_CALL(embeddedCompositorMock, dispatchNewStreamTextureSourceIds()).WillOnce(Return(empty));
    EXPECT_CALL(embeddedCompositorMock, dispatchObsoleteStreamTextureSourceIds()).WillOnce(Return(empty));
    expectNoStreamTextureChangedState();
}

TEST_F(AnEmbeddedCompositingManager, CanDispatchrStreamTexturesStateChange_IfStreamSourceBecomesUnavailable)
{
    const WaylandIviSurfaceIdSet empty;
    const WaylandIviSurfaceIdSet obsoleteStreams{ streamTextureSourceId };
    EXPECT_CALL(embeddedCompositorMock, dispatchNewStreamTextureSourceIds()).WillOnce(Return(empty));
    EXPECT_CALL(embeddedCompositorMock, dispatchObsoleteStreamTextureSourceIds()).WillOnce(Return(obsoleteStreams));

    expectStreamTextureChangedState({}, { streamTextureSourceId });
}

TEST_F(AnEmbeddedCompositingManager, CanDispatchrStreamTexturesStateChange_MultipleStreamTexturesInSameScene)
{
    addSceneReferenceToStreamTexture(sceneId, streamTexture, streamTextureSourceId, compositedTextureDeviceHandle);
    addSceneReferenceToStreamTexture(sceneId, streamTexture2, streamTextureSourceId, compositedTextureDeviceHandle, false);

    const WaylandIviSurfaceIdSet empty;
    const WaylandIviSurfaceIdSet newStreams{ streamTextureSourceId };
    EXPECT_CALL(embeddedCompositorMock, dispatchNewStreamTextureSourceIds()).WillOnce(Return(newStreams));
    EXPECT_CALL(embeddedCompositorMock, dispatchObsoleteStreamTextureSourceIds()).WillOnce(Return(empty));
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillOnce(Return(true));

    SceneStreamTextures updatedStreamTexturesPerScene;
    WaylandIviSurfaceIdVector newStreamsResult;
    WaylandIviSurfaceIdVector obsoleteStreamsResult;
    embeddedCompositingManager.dispatchStateChangesOfStreamTexturesAndSources(updatedStreamTexturesPerScene, newStreamsResult, obsoleteStreamsResult);

    ASSERT_EQ(1u, updatedStreamTexturesPerScene.size());
    ASSERT_TRUE(updatedStreamTexturesPerScene.contains(sceneId));
    EXPECT_EQ(streamTexture, (*updatedStreamTexturesPerScene.get(sceneId))[0]);
    EXPECT_EQ(streamTexture2, (*updatedStreamTexturesPerScene.get(sceneId))[1]);

    ASSERT_EQ(1u, newStreamsResult.size());
    EXPECT_EQ(streamTextureSourceId, newStreamsResult[0]);
    EXPECT_EQ(0u, obsoleteStreamsResult.size());
}

TEST_F(AnEmbeddedCompositingManager, CanDispatchrStreamTexturesStateChange_MultipleStreamTexturesInMultipleScenes)
{
    addSceneReferenceToStreamTexture(sceneId, streamTexture, streamTextureSourceId, compositedTextureDeviceHandle);
    addSceneReferenceToStreamTexture(sceneId, streamTexture2, streamTextureSourceId2, compositedTextureDeviceHandle);

    addSceneReferenceToStreamTexture(sceneId2, streamTexture, streamTextureSourceId2, compositedTextureDeviceHandle, false);
    addSceneReferenceToStreamTexture(sceneId2, streamTexture2, streamTextureSourceId, compositedTextureDeviceHandle, false);


    const WaylandIviSurfaceIdSet empty;
    const WaylandIviSurfaceIdSet newStreams{ streamTextureSourceId, streamTextureSourceId2 };
    EXPECT_CALL(embeddedCompositorMock, dispatchNewStreamTextureSourceIds()).WillOnce(Return(newStreams));
    EXPECT_CALL(embeddedCompositorMock, dispatchObsoleteStreamTextureSourceIds()).WillOnce(Return(empty));
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillOnce(Return(true));
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId2)).WillOnce(Return(false));

    SceneStreamTextures updatedStreamTexturesPerScene;
    WaylandIviSurfaceIdVector newStreamsResult;
    WaylandIviSurfaceIdVector obsoleteStreamsResult;
    embeddedCompositingManager.dispatchStateChangesOfStreamTexturesAndSources(updatedStreamTexturesPerScene, newStreamsResult, obsoleteStreamsResult);

    ASSERT_EQ(2u, updatedStreamTexturesPerScene.size());
    ASSERT_TRUE(updatedStreamTexturesPerScene.contains(sceneId));
    EXPECT_EQ(streamTexture, (*updatedStreamTexturesPerScene.get(sceneId))[0]);

    ASSERT_TRUE(updatedStreamTexturesPerScene.contains(sceneId2));
    EXPECT_EQ(streamTexture2, (*updatedStreamTexturesPerScene.get(sceneId2))[0]);

    ASSERT_EQ(2u, newStreamsResult.size());
    EXPECT_NE(newStreamsResult.end(), find_c(newStreamsResult, streamTextureSourceId));
    EXPECT_NE(newStreamsResult.end(), find_c(newStreamsResult, streamTextureSourceId2));
    EXPECT_EQ(0u, obsoleteStreamsResult.size());
}
