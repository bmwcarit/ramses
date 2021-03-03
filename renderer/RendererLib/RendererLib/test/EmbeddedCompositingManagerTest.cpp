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
#include "Utils/ThreadLocalLog.h"

using namespace testing;
using namespace ramses_internal;

class AnEmbeddedCompositingManager : public ::testing::Test
{
public:
    AnEmbeddedCompositingManager()
    {
        // caller is expected to have a display prefix for logs
        ThreadLocalLog::SetPrefix(1);
    }

    void expectStreamTexUpload(WaylandIviSurfaceId sourceId, DeviceResourceHandle textureDeviceHandle = compositedTextureDeviceHandle)
    {
        EXPECT_CALL(deviceMock, uploadStreamTexture2D(_, _, _, _, _, _)).WillOnce(Return(textureDeviceHandle));
        EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(sourceId)).WillOnce(Return(false));
    }

    void expectStreamTexUnload(DeviceResourceHandle textureDeviceHandle = compositedTextureDeviceHandle)
    {
        EXPECT_CALL(deviceMock, deleteTexture(textureDeviceHandle));
    }

    void addStreamReference(WaylandIviSurfaceId sourceId, DeviceResourceHandle textureDeviceHandle = compositedTextureDeviceHandle, bool expectTextureUpload = true)
    {
        if (expectTextureUpload)
            expectStreamTexUpload(sourceId, textureDeviceHandle);
        embeddedCompositingManager.refStream(sourceId);
    }

    void removeStreamReference(WaylandIviSurfaceId sourceId, DeviceResourceHandle textureDeviceHandle = compositedTextureDeviceHandle, bool expectTextureUnload = true)
    {
        if (expectTextureUnload)
            expectStreamTexUnload(textureDeviceHandle);
        embeddedCompositingManager.unrefStream(sourceId);
    }

    void expectStreamTextureChangedState(const WaylandIviSurfaceIdVector& expectedStreamsWithStateChange, const WaylandIviSurfaceIdVector& expectedNewStreams, const WaylandIviSurfaceIdVector& expectedObsoleteStreams)
    {
        WaylandIviSurfaceIdVector streamsWithStateChange;
        WaylandIviSurfaceIdVector newStreams;
        WaylandIviSurfaceIdVector obsoleteStreams;
        embeddedCompositingManager.dispatchStateChangesOfSources(streamsWithStateChange, newStreams, obsoleteStreams);

        EXPECT_EQ(expectedStreamsWithStateChange, streamsWithStateChange);
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

    static constexpr DeviceResourceHandle compositedTextureDeviceHandle{ 111u };
};

constexpr DeviceResourceHandle AnEmbeddedCompositingManager::compositedTextureDeviceHandle;

TEST_F(AnEmbeddedCompositingManager, CanUploadAndDeleteStreamTexture)
{
    addStreamReference(streamTextureSourceId);
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillOnce(Return(true));
    EXPECT_EQ(compositedTextureDeviceHandle, embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(streamTextureSourceId));

    removeStreamReference(streamTextureSourceId);
}

TEST_F(AnEmbeddedCompositingManager, ReturnsInvalidDeviceResourceHandleForInvalidStreamSourceId)
{
    const WaylandIviSurfaceId fakeId(234);

    //simulate compositing content is not available
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(fakeId)).WillOnce(Return(false));
    EXPECT_EQ(DeviceResourceHandle::Invalid(), embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(fakeId));
}

TEST_F(AnEmbeddedCompositingManager, CanGetCorrectDeviceHandleForValidStreamTextureId)
{
    addStreamReference(streamTextureSourceId);

    //simulate compositing content is available
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillOnce(Return(true));
    //expect composited texture
    EXPECT_EQ(compositedTextureDeviceHandle, embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(streamTextureSourceId));

    //simulate compositing content is NOT available
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillOnce(Return(false));
    //expect invalid texture handle
    EXPECT_EQ(DeviceResourceHandle::Invalid(), embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(streamTextureSourceId));

    removeStreamReference(streamTextureSourceId);
}

TEST_F(AnEmbeddedCompositingManager, CanUpdateCompositingResources)
{
    const WaylandIviSurfaceIdSet fakeUpdatedStreamTextureSourceIds{ streamTextureSourceId, streamTextureSourceId2 };

    addStreamReference(streamTextureSourceId);
    addStreamReference(streamTextureSourceId2);

    {
        InSequence s;
        EXPECT_CALL(embeddedCompositorMock, handleRequestsFromClients());
        EXPECT_CALL(embeddedCompositorMock, endFrame(false));
        embeddedCompositingManager.processClientRequests();

        EXPECT_CALL(embeddedCompositorMock, hasUpdatedStreamTextureSources()).WillOnce(Return(true));
        EXPECT_TRUE(embeddedCompositingManager.hasUpdatedContentFromStreamSourcesToUpload());

        EXPECT_CALL(embeddedCompositorMock, dispatchUpdatedStreamTextureSourceIds()).WillOnce(Return(fakeUpdatedStreamTextureSourceIds));
    }

    // these expectations don't necessarily come in same order
    EXPECT_CALL(embeddedCompositorMock, uploadCompositingContentForStreamTexture(streamTextureSourceId, _, _)).WillOnce(Return(13u));
    EXPECT_CALL(embeddedCompositorMock, uploadCompositingContentForStreamTexture(streamTextureSourceId2, _, _)).WillOnce(Return(6u));
    StreamSourceUpdates updates;
    embeddedCompositingManager.uploadResourcesAndGetUpdates(updates);
    ASSERT_EQ(2u, updates.size());
    int idx1 = 0;
    int idx2 = 1;
    // updates might come in different order
    if (updates.front().first != streamTextureSourceId)
        std::swap(idx1, idx2);
    EXPECT_EQ(streamTextureSourceId, updates[idx1].first);
    EXPECT_EQ(13u, updates[idx1].second);
    EXPECT_EQ(streamTextureSourceId2, updates[idx2].first);
    EXPECT_EQ(6u, updates[idx2].second);
}

TEST_F(AnEmbeddedCompositingManager, CanNotifyClients)
{
    InSequence s;
    EXPECT_CALL(embeddedCompositorMock, endFrame(true));
    embeddedCompositingManager.notifyClients();
}

TEST_F(AnEmbeddedCompositingManager, UploadsOneCompositedTextureForTwoEqualStreamTexturesSourceIds)
{
    addStreamReference(streamTextureSourceId, compositedTextureDeviceHandle);
    addStreamReference(streamTextureSourceId, compositedTextureDeviceHandle, false);

    //make sure it returns the composited texture device handle for the stream textures id
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillRepeatedly(Return(true));
    EXPECT_EQ(compositedTextureDeviceHandle, embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(streamTextureSourceId));
}

TEST_F(AnEmbeddedCompositingManager, UploadsTwoCompositedTextureForTwoDifferentStreamTextureSourceIds)
{
    addStreamReference(streamTextureSourceId, compositedTextureDeviceHandle);

    const WaylandIviSurfaceId secondStreamTextureSourceId(22);
    const DeviceResourceHandle secondCompositedTextureDeviceHandle(222);
    addStreamReference(secondStreamTextureSourceId, secondCompositedTextureDeviceHandle);

    //make sure it returns a different composited texture device handle for each stream texture source id
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillOnce(Return(true));
    EXPECT_EQ(compositedTextureDeviceHandle, embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(streamTextureSourceId));

    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(secondStreamTextureSourceId)).WillOnce(Return(true));
    EXPECT_EQ(secondCompositedTextureDeviceHandle, embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(secondStreamTextureSourceId));
}

TEST_F(AnEmbeddedCompositingManager, DeletesCompositedTextureOnlyAfterAllReferencesRemoved)
{
    addStreamReference(streamTextureSourceId, compositedTextureDeviceHandle);
    addStreamReference(streamTextureSourceId, compositedTextureDeviceHandle, false);

    removeStreamReference(streamTextureSourceId, compositedTextureDeviceHandle, false);
    removeStreamReference(streamTextureSourceId, compositedTextureDeviceHandle);
}

TEST_F(AnEmbeddedCompositingManager, CanUploadAndDeleteStreamTextureWithAvailableContent)
{
    EXPECT_CALL(deviceMock, uploadStreamTexture2D(_, _, _, _, _, _)).WillOnce(Return(compositedTextureDeviceHandle));
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillOnce(Return(true));
    EXPECT_CALL(embeddedCompositorMock, uploadCompositingContentForStreamTexture(streamTextureSourceId, _, _));

    embeddedCompositingManager.refStream(streamTextureSourceId);

    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillOnce(Return(true));
    EXPECT_EQ(compositedTextureDeviceHandle, embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(streamTextureSourceId));

    removeStreamReference(streamTextureSourceId, compositedTextureDeviceHandle);
}

TEST_F(AnEmbeddedCompositingManager, CanDispatchStreamTexturesStateChange_NoChange)
{
    const WaylandIviSurfaceIdSet empty;
    EXPECT_CALL(embeddedCompositorMock, dispatchNewStreamTextureSourceIds()).WillOnce(Return(empty));
    EXPECT_CALL(embeddedCompositorMock, dispatchObsoleteStreamTextureSourceIds()).WillOnce(Return(empty));

    expectStreamTextureChangedState({}, {}, {});
}

TEST_F(AnEmbeddedCompositingManager, CanDispatchStreamTexturesStateChange_IfStreamSourceBecomesAvailable)
{
    const WaylandIviSurfaceIdSet empty;
    const WaylandIviSurfaceIdSet newStreams{ streamTextureSourceId };
    EXPECT_CALL(embeddedCompositorMock, dispatchNewStreamTextureSourceIds()).WillOnce(Return(newStreams));
    EXPECT_CALL(embeddedCompositorMock, dispatchObsoleteStreamTextureSourceIds()).WillOnce(Return(empty));

    expectStreamTextureChangedState({}, { streamTextureSourceId }, {});
}

TEST_F(AnEmbeddedCompositingManager, CanDispatchStreamTexturesStateChange_EmptyAfterDispatching)
{
    const WaylandIviSurfaceIdSet empty;
    const WaylandIviSurfaceIdSet newStreams{ streamTextureSourceId };
    EXPECT_CALL(embeddedCompositorMock, dispatchNewStreamTextureSourceIds()).WillOnce(Return(newStreams));
    EXPECT_CALL(embeddedCompositorMock, dispatchObsoleteStreamTextureSourceIds()).WillOnce(Return(empty));
    expectStreamTextureChangedState({}, { streamTextureSourceId }, {});

    EXPECT_CALL(embeddedCompositorMock, dispatchNewStreamTextureSourceIds()).WillOnce(Return(empty));
    EXPECT_CALL(embeddedCompositorMock, dispatchObsoleteStreamTextureSourceIds()).WillOnce(Return(empty));
    expectStreamTextureChangedState({}, {}, {});
}

TEST_F(AnEmbeddedCompositingManager, CanDispatchStreamTexturesStateChange_IfStreamSourceBecomesUnavailable)
{
    const WaylandIviSurfaceIdSet empty;
    const WaylandIviSurfaceIdSet obsoleteStreams{ streamTextureSourceId };
    EXPECT_CALL(embeddedCompositorMock, dispatchNewStreamTextureSourceIds()).WillOnce(Return(empty));
    EXPECT_CALL(embeddedCompositorMock, dispatchObsoleteStreamTextureSourceIds()).WillOnce(Return(obsoleteStreams));

    expectStreamTextureChangedState({}, {}, { streamTextureSourceId });
}

TEST_F(AnEmbeddedCompositingManager, CanDispatchStreamTexturesStateChange_multipleRefs)
{
    addStreamReference(streamTextureSourceId, compositedTextureDeviceHandle);
    addStreamReference(streamTextureSourceId, compositedTextureDeviceHandle, false);

    const WaylandIviSurfaceIdSet empty;
    const WaylandIviSurfaceIdSet newStreams{ streamTextureSourceId };
    EXPECT_CALL(embeddedCompositorMock, dispatchNewStreamTextureSourceIds()).WillOnce(Return(newStreams));
    EXPECT_CALL(embeddedCompositorMock, dispatchObsoleteStreamTextureSourceIds()).WillOnce(Return(empty));
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillOnce(Return(true));

    expectStreamTextureChangedState({ streamTextureSourceId }, { streamTextureSourceId }, {});
}

TEST_F(AnEmbeddedCompositingManager, CanDispatchStreamTexturesStateChange_multipleSources)
{
    addStreamReference(streamTextureSourceId, compositedTextureDeviceHandle);
    addStreamReference(streamTextureSourceId2, compositedTextureDeviceHandle);

    const WaylandIviSurfaceIdSet empty;
    const WaylandIviSurfaceIdSet newStreams{ streamTextureSourceId };
    EXPECT_CALL(embeddedCompositorMock, dispatchNewStreamTextureSourceIds()).WillOnce(Return(newStreams));
    EXPECT_CALL(embeddedCompositorMock, dispatchObsoleteStreamTextureSourceIds()).WillOnce(Return(empty));
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId)).WillOnce(Return(true));
    EXPECT_CALL(embeddedCompositorMock, isContentAvailableForStreamTexture(streamTextureSourceId2)).WillOnce(Return(true));

    // order changed due to hashmap, not important for test
    expectStreamTextureChangedState({ streamTextureSourceId2, streamTextureSourceId }, { streamTextureSourceId }, {});
}
