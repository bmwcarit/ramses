//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererSceneUpdaterTest.h"

namespace ramses_internal {

TEST_F(ARendererSceneUpdater, unrequestsResourcesInUseByMappedSceneWhenSceneGetsUnpublished)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    createRenderable();
    setRenderableResources();
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    expectContextEnable();
    expectRenderableResourcesDeleted();
    unpublishMappedScene();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, unrequestsResourcesInUseByMapRequestedSceneWhenSceneGetsUnpublished)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();

    createRenderable();
    setRenderableResources(0u, InvalidResource1, true);

    // this is necessary for processing the scene on renderer side first
    // otherwise the renderer has not processed the scene actions for creating the scene
    update();

    requestMapScene();
    update();

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    expectContextEnable();
    expectRenderableResourcesDeleted(DisplayHandle1, true, false);
    unpublishMapRequestedScene();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, unrequestsResourcesInUseByRenderedSceneWhenSceneGetsUnpublished)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    createRenderable();
    setRenderableResources();
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded();
    update();

    expectContextEnable();
    expectRenderableResourcesDeleted();
    unpublishShownScene();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, unrequestsResourcesInUseByMappedSceneWhenSceneGetsUnsubscribedByError)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    createRenderable();
    setRenderableResources();
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    expectContextEnable();
    expectRenderableResourcesDeleted();
    EXPECT_CALL(sceneGraphConsumerComponent, unsubscribeScene(_, _));
    unsubscribeMappedScene();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, unrequestsResourcesInUseByMapRequestedSceneWhenSceneGetsUnsubscribedByError)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();

    createRenderable();
    setRenderableResources(0u, InvalidResource1, true);

    // this is necessary for processing the scene on renderer side first
    // otherwise the renderer has not processed the scene actions for creating the scene
    update();

    requestMapScene();
    update();

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    expectContextEnable();
    expectRenderableResourcesDeleted(DisplayHandle1, true, false);
    EXPECT_CALL(sceneGraphConsumerComponent, unsubscribeScene(_, _));
    unsubscribeMapRequestedScene();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, unrequestsResourcesInUseByRenderedSceneWhenSceneGetsUnsubscribedByError)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    createRenderable();
    setRenderableResources();
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded();
    update();

    expectContextEnable();
    expectRenderableResourcesDeleted();

    EXPECT_CALL(sceneGraphConsumerComponent, unsubscribeScene(_, _));
    unsubscribeShownScene();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, unrequestsResourcesInUseByMappedSceneWhenUpdaterDestructed)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    createRenderable();
    setRenderableResources();
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded();
    update();

    expectRenderableResourcesDeleted();

    destroySceneUpdater();
}

TEST_F(ARendererSceneUpdater, confidenceTest_doesNotRequestOrUnrequestResourcesIfSceneNotMapped)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    createRenderable();
    setRenderableResources();

    DisplayStrictMockInfo& displayMock = renderer.getDisplayMock(DisplayHandle1);
    EXPECT_CALL(displayMock.m_renderBackend->deviceMock, uploadShader(_)).Times(0);
    EXPECT_CALL(displayMock.m_renderBackend->deviceMock, uploadBinaryShader(_, _, _, _)).Times(0);
    EXPECT_CALL(displayMock.m_renderBackend->deviceMock, deleteShader(_)).Times(0);

    update();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, uploadsSceneResourcesAtMappingTime)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    createRenderTargetWithBuffers();
    update();

    expectContextEnable();
    expectRenderTargetUploaded();
    mapScene();

    expectContextEnable();
    expectRenderTargetDeleted();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, uploadsResourcesNextUpdateAfterSceneMapped)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    createRenderable();
    setRenderableResources();
    update();

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded();
    mapScene();

    expectContextEnable();
    expectRenderableResourcesDeleted();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, updatesDirtynessOfMappedSceneResources)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    createRenderable();
    setRenderableResources(0u, InvalidResource1);

    expectContextEnable();
    expectResourceRequest();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    setRenderableResources();
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, false, true);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    expectContextEnable();
    expectRenderableResourcesDeleted();
    unmapScene();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, updatesScenesStreamTexturesCache_SingleScene)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();
    createRenderableAndResourcesWithStreamTexture();

    DeviceResourceHandle fakeDeviceResourceHandle(1u);
    expectEmbeddedCompositingManagerReturnsDeviceHandle(fakeDeviceResourceHandle);
    update();
    expectRenderableResourcesClean();

    StreamTextureHandleVector mockUpdatedStreamTexturesForScene({ streamTextureHandle });
    SceneStreamTextures mockUpdatedStreamTextures;
    mockUpdatedStreamTextures.put(getSceneId(), mockUpdatedStreamTexturesForScene);
    EXPECT_CALL(*renderer.getDisplayMock(DisplayHandle1).m_embeddedCompositingManager, dispatchStateChangesOfStreamTexturesAndSources(_, _, _)).WillRepeatedly(SetArgReferee<0>(mockUpdatedStreamTextures));

    expectEmbeddedCompositingManagerReturnsDeviceHandle(DeviceResourceHandle::Invalid());
    update();

    expectRenderableResourcesDirty();

    hideScene();
    expectContextEnable();
    expectRenderableResourcesDeleted(DisplayHandle1, true, true, false, true);
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, updatesScenesStreamTexturesCache_MultipleScenes)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    mapScene(0u);
    mapScene(1u);
    showScene(0u);
    showScene(1u);
    createRenderableAndResourcesWithStreamTexture(0u);
    createRenderableAndResourcesWithStreamTexture(1u, false);

    DeviceResourceHandle fakeDeviceResourceHandle(1u);
    expectEmbeddedCompositingManagerReturnsDeviceHandle(fakeDeviceResourceHandle, 2u);
    update();
    expectRenderableResourcesClean(0u);
    expectRenderableResourcesClean(1u);

    StreamTextureHandleVector mockUpdatedStreamTexturesForScene({ streamTextureHandle });
    SceneStreamTextures mockUpdatedStreamTextures;
    mockUpdatedStreamTextures.put(getSceneId(0u), mockUpdatedStreamTexturesForScene);
    mockUpdatedStreamTextures.put(getSceneId(1u), mockUpdatedStreamTexturesForScene);
    EXPECT_CALL(*renderer.getDisplayMock(DisplayHandle1).m_embeddedCompositingManager, dispatchStateChangesOfStreamTexturesAndSources(_, _, _)).WillRepeatedly(SetArgReferee<0>(mockUpdatedStreamTextures));

    expectEmbeddedCompositingManagerReturnsDeviceHandle(DeviceResourceHandle::Invalid(), 2u);
    update();
    expectRenderableResourcesDirty(0u);
    expectRenderableResourcesDirty(1u);

    hideScene(0u);
    hideScene(1u);
    expectContextEnable();
    expectRenderableResourcesDeleted(DisplayHandle1, false, false, false, true);
    unmapScene(0u);
    expectContextEnable();
    expectRenderableResourcesDeleted(DisplayHandle1, true, true, false, true);
    unmapScene(1u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, unrequestedResourceIsCanceledNextUpdateCycleIfTriggeredBySyncFlush)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    createRenderable();
    setRenderableResources(0u, InvalidResource1);

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    setRenderableResources();
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, false, true);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    expectResourceRequestCancel(InvalidResource1);
    update();

    expectContextEnable();
    expectRenderableResourcesDeleted();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, updatesDirtynessOfRenderedSceneResources)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    createRenderable();
    setRenderableResources(0u, InvalidResource1);

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    setRenderableResources();
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, false, true);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    hideScene();
    expectContextEnable();
    expectRenderableResourcesDeleted();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, syncFlush_appliesPendingFlushesAlwaysIfSceneNotMapped)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    createRenderable(0u, true);
    setRenderableResources(0u, InvalidResource1, true);

    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    performFlush();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    performFlush();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, syncFlush_pendingFlushesAreNotAppliedIfResourcesNotReady)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    createRenderable(0u, true);
    setRenderableResources(0u, InvalidResource1, true);

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    performFlush();
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    performFlush();
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    hideScene();
    expectContextEnable();
    expectRenderableResourcesDeleted(DisplayHandle1, true, false);
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, syncFlush_whenSceneBecomesReadyPendingFlushesAreAppliedInOrderAtOnce)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    createRenderable(0u, true);
    setRenderableResources(0u, InvalidResource1, true);
    expectNoEvent();

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();

    const SceneVersionTag version1(1u);
    const SceneVersionTag version2(2u);
    const SceneVersionTag version3(3u);

    performFlush(0u, true, version1);
    update();
    expectNoEvent();

    performFlush(0u, true, version2);
    update();
    expectNoEvent();

    performFlush(0u, true, version3);
    update();
    expectNoEvent();

    setRenderableResources();
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, false, true);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    expectResourceRequestCancel(InvalidResource1);
    update();

    RendererEventVector events;
    rendererEventCollector.dispatchEvents(events);
    ASSERT_EQ(3u, events.size());
    EXPECT_EQ(ERendererEventType_SceneFlushed, events[0].eventType);
    EXPECT_EQ(ERendererEventType_SceneFlushed, events[1].eventType);
    EXPECT_EQ(ERendererEventType_SceneFlushed, events[2].eventType);
    EXPECT_EQ(EResourceStatus_Uploaded, events[0].resourceStatus);
    EXPECT_EQ(EResourceStatus_Uploaded, events[1].resourceStatus);
    EXPECT_EQ(EResourceStatus_Uploaded, events[2].resourceStatus);
    EXPECT_EQ(version1.getValue(), events[0].sceneVersionTag.getValue());
    EXPECT_EQ(version2.getValue(), events[1].sceneVersionTag.getValue());
    EXPECT_EQ(version3.getValue(), events[2].sceneVersionTag.getValue());

    expectRenderableResourcesDeleted();
    destroySceneUpdater();
}

TEST_F(ARendererSceneUpdater, syncFlush_multiplePendingFlushesPerRenderLoopAreAppliedInOrderAtOnce)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    createRenderable(0u, true);
    setRenderableResources(0u, InvalidResource1, true);
    expectNoEvent();

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();

    const SceneVersionTag version1(1u);
    const SceneVersionTag version2(2u);
    const SceneVersionTag version3(3u);

    performFlush(0u, true, version1);
    performFlush(0u, true, version2);
    performFlush(0u, true, version3);

    setRenderableResources();
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, false, true);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    expectResourceRequestCancel(InvalidResource1);
    update();

    RendererEventVector events;
    rendererEventCollector.dispatchEvents(events);
    ASSERT_EQ(3u, events.size());
    EXPECT_EQ(ERendererEventType_SceneFlushed, events[0].eventType);
    EXPECT_EQ(ERendererEventType_SceneFlushed, events[1].eventType);
    EXPECT_EQ(ERendererEventType_SceneFlushed, events[2].eventType);
    EXPECT_EQ(EResourceStatus_Uploaded, events[0].resourceStatus);
    EXPECT_EQ(EResourceStatus_Uploaded, events[1].resourceStatus);
    EXPECT_EQ(EResourceStatus_Uploaded, events[2].resourceStatus);
    EXPECT_EQ(version1.getValue(), events[0].sceneVersionTag.getValue());
    EXPECT_EQ(version2.getValue(), events[1].sceneVersionTag.getValue());
    EXPECT_EQ(version3.getValue(), events[2].sceneVersionTag.getValue());

    expectRenderableResourcesDeleted();
    destroySceneUpdater();
}

TEST_F(ARendererSceneUpdater, syncFlush_whenSceneBecomesReadyAsyncPendingFlushesAreAppliedInOrderAtOnce)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    createRenderable(0u, true);
    setRenderableResources(0u, InvalidResource1, true);
    expectNoEvent();

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();

    const SceneVersionTag version1(1u);
    const SceneVersionTag version2(2u);
    const SceneVersionTag version3(3u);

    performFlush(0u, false, version1);
    update();
    expectNoEvent();

    performFlush(0u, false, version2);
    update();
    expectNoEvent();

    performFlush(0u, false, version3);
    update();
    expectNoEvent();

    setRenderableResources();
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, false, true);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    expectResourceRequestCancel(InvalidResource1);
    update();

    RendererEventVector events;
    rendererEventCollector.dispatchEvents(events);
    ASSERT_EQ(3u, events.size());
    EXPECT_EQ(ERendererEventType_SceneFlushed, events[0].eventType);
    EXPECT_EQ(ERendererEventType_SceneFlushed, events[1].eventType);
    EXPECT_EQ(ERendererEventType_SceneFlushed, events[2].eventType);
    EXPECT_EQ(EResourceStatus_Uploaded, events[0].resourceStatus);
    EXPECT_EQ(EResourceStatus_Uploaded, events[1].resourceStatus);
    EXPECT_EQ(EResourceStatus_Uploaded, events[2].resourceStatus);
    EXPECT_EQ(version1.getValue(), events[0].sceneVersionTag.getValue());
    EXPECT_EQ(version2.getValue(), events[1].sceneVersionTag.getValue());
    EXPECT_EQ(version3.getValue(), events[2].sceneVersionTag.getValue());

    expectRenderableResourcesDeleted();
    destroySceneUpdater();
}

TEST_F(ARendererSceneUpdater, syncFlush_multipleAsyncPendingFlushesPerRenderLoopAreAppliedInOrderAtOnce)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    createRenderable(0u, true);
    setRenderableResources(0u, InvalidResource1, true);
    expectNoEvent();

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();

    const SceneVersionTag version1(1u);
    const SceneVersionTag version2(2u);
    const SceneVersionTag version3(3u);

    performFlush(0u, false, version1);
    performFlush(0u, false, version2);
    performFlush(0u, false, version3);

    setRenderableResources();
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, false, true);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    expectResourceRequestCancel(InvalidResource1);
    update();

    RendererEventVector events;
    rendererEventCollector.dispatchEvents(events);
    ASSERT_EQ(3u, events.size());
    EXPECT_EQ(ERendererEventType_SceneFlushed, events[0].eventType);
    EXPECT_EQ(ERendererEventType_SceneFlushed, events[1].eventType);
    EXPECT_EQ(ERendererEventType_SceneFlushed, events[2].eventType);
    EXPECT_EQ(EResourceStatus_Uploaded, events[0].resourceStatus);
    EXPECT_EQ(EResourceStatus_Uploaded, events[1].resourceStatus);
    EXPECT_EQ(EResourceStatus_Uploaded, events[2].resourceStatus);
    EXPECT_EQ(version1.getValue(), events[0].sceneVersionTag.getValue());
    EXPECT_EQ(version2.getValue(), events[1].sceneVersionTag.getValue());
    EXPECT_EQ(version3.getValue(), events[2].sceneVersionTag.getValue());

    expectRenderableResourcesDeleted();
    destroySceneUpdater();
}

TEST_F(ARendererSceneUpdater, syncFlush_pendingFlushesAreAppliedOnlyAfterResourcesAreReady)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    createRenderable(0u, true);
    setRenderableResources(0u, InvalidResource1, true);

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    setRenderableResources();
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, false, true);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    expectResourceRequestCancel(InvalidResource1);
    update();

    hideScene();
    expectContextEnable();
    expectRenderableResourcesDeleted();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, syncFlush_pendingFlushesAreAppliedIfBlockingDirtyResourceNotUsedAnymore)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    createRenderable(0u, true);
    setRenderableResources(0u, InvalidResource1, true);

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    setRenderableResources(0u, ResourceProviderMock::FakeIndexArrayHash, true);
    expectResourceRequest();
    expectRenderableResourcesUploaded(DisplayHandle1, false, true);
    expectContextEnable();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    expectResourceRequestCancel(InvalidResource1);
    update();

    hideScene();
    expectContextEnable();
    expectRenderableResourcesDeleted();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, syncFlush_multipleFlushesChangingResourceDependencyNotAppliedUntilResolved)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    createRenderable(0u, true);
    setRenderableResources(0u, InvalidResource1, true);

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    setRenderableResources(0u, InvalidResource2);
    expectResourceRequest();
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    setRenderableResources(0u, InvalidResource1);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // set valid available resource
    expectResourceRequestCancel(InvalidResource2);
    setRenderableResources();
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, false, true);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    expectResourceRequestCancel(InvalidResource1);
    update();

    hideScene();
    expectContextEnable();
    expectRenderableResourcesDeleted();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, syncFlush_pendingFlushesWillBeAppliedIfSceneGetsUnmapped)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    createRenderable(0u, true);
    setRenderableResources(0u, InvalidResource1, true);

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    expectContextEnable();
    expectResourceRequestCancel(InvalidResource1);
    unmapScene();

    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    expectRenderableResourcesDeleted(DisplayHandle1, true, false);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, syncFlush_resourceUsedByRenderedSceneUnreferencedInSyncFlushWillBeUnrequestedOnceFlushIsReady)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    createRenderable(0u, true);
    setRenderableResources(0u, ResourceProviderMock::FakeIndexArrayHash, true);

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    setRenderableResources(0u, InvalidResource1, true);
    expectResourceRequest();
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    setRenderableResources(0u, ResourceProviderMock::FakeIndexArrayHash2, true);
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, false, true);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    expectResourceRequestCancel(InvalidResource1);
    update();

    // previously used index buffer was marked as unused
    // and will be unloaded (depending on unload strategy)
    // together with the one currently in use
    EXPECT_CALL(renderer.getDisplayMock(DisplayHandle1).m_renderBackend->deviceMock, deleteIndexBuffer(_)).Times(2u);
    expectRenderableResourcesDeleted(DisplayHandle1, true, false);

    destroySceneUpdater();
}

TEST_F(ARendererSceneUpdater, syncFlush_resourceUsedByRenderedSceneUnreferencedAndReferencedInSyncFlushesWillMakeFlushReadyAgain)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    createRenderable(0u, true);
    setRenderableResources(0u, ResourceProviderMock::FakeIndexArrayHash, true);

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    setRenderableResources(0u, InvalidResource1, true);
    expectResourceRequest();
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    setRenderableResources(0u, ResourceProviderMock::FakeIndexArrayHash, true);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    expectResourceRequestCancel(InvalidResource1);
    update();

    expectRenderableResourcesDeleted();
    destroySceneUpdater();
}

TEST_F(ARendererSceneUpdater, confidenceTest_syncFlush_mappedSceneWithBlockingSyncFlushGetsUnmappedAndRemapped)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    createRenderable(0u, true);
    setRenderableResources(0u, InvalidResource1, true);

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    expectContextEnable();
    expectResourceRequestCancel(InvalidResource1);
    unmapScene();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    setRenderableResources(0u, ResourceProviderMock::FakeIndexArrayHash, true);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, false, true);
    mapScene();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    expectRenderableResourcesDeleted();
    destroySceneUpdater();
}

TEST_F(ARendererSceneUpdater, newRenderTargetIsUploadedOnlyAfterPendingFlushApplied)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    update();

    // emulate blocking sync flush
    createRenderable(0u, true);
    setRenderableResources(0u, InvalidResource1, true);
    createRenderTargetWithBuffers();
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // unblock pending flushes
    setRenderableResources();
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, false, true);
    expectRenderTargetUploaded();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());
    EXPECT_CALL(renderer.getDisplayMock(DisplayHandle1).m_renderBackend->deviceMock, deleteRenderTarget(_)).Times(0u);

    expectResourceRequestCancel(InvalidResource1);
    update();

    expectContextEnable();
    expectRenderTargetDeleted();
    expectRenderableResourcesDeleted();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, renderTargetIsUnloadedOnlyAfterPendingFlushAppliedIfAlreadyUsedByRendererScene)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    createRenderTargetWithBuffers();
    expectContextEnable();
    expectRenderTargetUploaded();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    // emulate blocking sync flush
    createRenderable(0u, true);
    setRenderableResources(0u, InvalidResource1, true);
    destroyRenderTargetWithBuffers();
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // unblock pending flushes
    setRenderableResources();
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, false, true);
    expectRenderTargetDeleted();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());
    EXPECT_CALL(renderer.getDisplayMock(DisplayHandle1).m_renderBackend->deviceMock, deleteRenderTarget(_)).Times(0u);

    expectResourceRequestCancel(InvalidResource1);
    update();

    expectContextEnable();
    expectRenderableResourcesDeleted();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, renderTargetIsUploadedOnceWhenMappedIfCreatedBeforeMapping)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    createRenderTargetWithBuffers();
    update();

    expectContextEnable();
    expectRenderTargetUploaded();
    mapScene();

    expectContextEnable();
    expectRenderTargetDeleted();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, renderTargetIsUploadedOnceWhenCreatedIfAlreadyMapped)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    createRenderTargetWithBuffers();
    expectContextEnable();
    expectRenderTargetUploaded();
    update();

    expectContextEnable();
    expectRenderTargetDeleted();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, unloadsAndUploadsRenderTargetWithSameHandleDestroyedAndCreatedInSameLoop)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    createRenderTargetWithBuffers();
    expectContextEnable();
    expectRenderTargetUploaded();
    update();

    destroyRenderTargetWithBuffers();
    createRenderTargetWithBuffers();
    expectContextEnable();
    expectRenderTargetDeleted();
    expectRenderTargetUploaded();
    update();

    expectContextEnable();
    expectRenderTargetDeleted();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, doesNotUploadOrUnloadRenderTargetWithSameHandleCreatedAndDestroyedInSameLoop)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    createRenderTargetWithBuffers();
    destroyRenderTargetWithBuffers();
    update();

    expectContextEnable();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, doesNotReuploadResourcesForSceneUnmappedAndMappedToSameDisplay)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    createRenderable();
    setRenderableResources();
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded();
    update();

    expectContextEnable();
    unmapScene();
    update();
    mapScene();

    expectContextEnable();
    unmapScene();

    expectRenderableResourcesDeleted();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, resourcesAreReuploadedAfterSceneRemapped)
{
    createDisplayAndExpectSuccess(DisplayHandle1);
    createDisplayAndExpectSuccess(DisplayHandle2);

    createPublishAndSubscribeScene();
    mapScene(0u, DisplayHandle1);

    createRenderable();
    setRenderableResources();
    expectResourceRequest();
    expectContextEnable(DisplayHandle1);
    expectRenderableResourcesUploaded(DisplayHandle1);
    update();

    expectContextEnable(DisplayHandle1);
    unmapScene();
    update();

    expectResourceRequest(DisplayHandle2);
    expectContextEnable(DisplayHandle2);
    expectRenderableResourcesUploaded(DisplayHandle2);
    mapScene(0u, DisplayHandle2);

    expectContextEnable(DisplayHandle2);
    unmapScene();

    expectRenderableResourcesDeleted(DisplayHandle1);
    destroyDisplay(DisplayHandle1);
    expectRenderableResourcesDeleted(DisplayHandle2);
    destroyDisplay(DisplayHandle2);
}

TEST_F(ARendererSceneUpdater, renderTargetAndBuffersAreReuploadedAfterSceneRemapped)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    createRenderTargetWithBuffers();
    expectContextEnable();
    expectRenderTargetUploaded();
    update();

    expectContextEnable();
    expectRenderTargetDeleted();
    unmapScene();
    update();

    expectContextEnable();
    expectRenderTargetUploaded();
    mapScene();

    expectContextEnable();
    expectRenderTargetDeleted();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, blitPassesReuploadedAfterSceneRemapped)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    createBlitPass();
    expectContextEnable();
    expectBlitPassUploaded();
    update();

    expectContextEnable();
    expectBlitPassDeleted();
    unmapScene();
    update();

    expectContextEnable();
    expectBlitPassUploaded();
    mapScene();

    expectContextEnable();
    expectBlitPassDeleted();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, confidenceTest_syncFlush_twoDisplaysEachWithSceneAndSyncFlushAppliedAfterResourcesReady)
{
    createDisplayAndExpectSuccess(DisplayHandle1);
    createDisplayAndExpectSuccess(DisplayHandle2);

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();

    mapScene(0u, DisplayHandle1);
    mapScene(1u, DisplayHandle2);

    showScene(0u);
    showScene(1u);

    createRenderable(0u, true);
    createRenderable(1u, true);
    setRenderableResources(0u, InvalidResource1, true);
    setRenderableResources(1u, InvalidResource1, true);

    expectContextEnable(DisplayHandle1);
    expectContextEnable(DisplayHandle2);
    expectResourceRequest();
    expectResourceRequest(DisplayHandle2, 1u);
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    expectRenderableResourcesUploaded(DisplayHandle2, true, false);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene(0u));
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene(1u));

    setRenderableResources(0u);
    setRenderableResources(1u);
    expectResourceRequest();
    expectResourceRequest(DisplayHandle2, 1u);
    expectContextEnable(DisplayHandle1, 1u);
    expectContextEnable(DisplayHandle2, 1u);
    expectRenderableResourcesUploaded(DisplayHandle1, false, true);
    expectRenderableResourcesUploaded(DisplayHandle2, false, true);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene(0u));
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene(1u));

    expectResourceRequestCancel(InvalidResource1, DisplayHandle1);
    expectResourceRequestCancel(InvalidResource1, DisplayHandle2);
    update();

    hideScene(0u);
    hideScene(1u);
    expectContextEnable(DisplayHandle1);
    expectContextEnable(DisplayHandle2);
    expectRenderableResourcesDeleted(DisplayHandle1);
    expectRenderableResourcesDeleted(DisplayHandle2);
    unmapScene(0u);
    unmapScene(1u);
    destroyDisplay(DisplayHandle1);
    destroyDisplay(DisplayHandle2);
}

TEST_F(ARendererSceneUpdater, syncFlush_waitingForResourceAndUnmapWillRequestThatResourceWithNextMap)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    createRenderable(0u, true);
    setRenderableResources(0u, InvalidResource1, true);

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    expectContextEnable();
    unmapScene();

    expectResourceRequestCancel(InvalidResource1);
    update();

    requestMapScene();
    update();

    expectResourceRequest();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene()); // flush applied already here because scene is not mapped

    setRenderableResources();
    //expectResourceRequestCancel(InvalidResource1);
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, false, true);
    update();
    expectEvent(ERendererEventType_SceneMapped);
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    expectResourceRequestCancel(InvalidResource1);
    update();

    expectContextEnable();
    expectRenderableResourcesDeleted();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canDestroyDisplayIfThereArePendingUploadsDueToLimitedUploadTimeBudget)
{
    //set upload budget to Zero so it always succeeds to upload only 1 resource
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::ClientResourcesUpload, 0u);

    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    createRenderable(0u, false, true);
    setRenderableResources();
    setRenderableVertexArray();

    // expect effect uploaded
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false, false);
    update();

    // rest of resources are pending to be uploaded

    expectContextEnable();
    expectRenderableResourcesDeleted(DisplayHandle1, true, false, false);
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, onlyMapsASceneIfAllNeededResourcesAreUploaded)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();

    createRenderable();
    setRenderableResources(0u, InvalidResource1, true);

    requestMapScene();
    EXPECT_EQ(ESceneState::MapRequested, sceneStateExecutor.getSceneState(getSceneId()));

    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    setRenderableResources(); // simulate upload

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, false, true);
    update();
    EXPECT_EQ(ESceneState::Mapped, sceneStateExecutor.getSceneState(getSceneId()));
    expectEvent(ERendererEventType_SceneMapped);

    expectResourceRequestCancel(InvalidResource1);
    update();

    expectContextEnable();
    expectRenderableResourcesDeleted();
    unmapScene();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, onlyMapsASceneIfAllNeededResourcesAreUploaded_WithTwoDifferentInvalidResourcesUsed)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();

    createRenderable();
    setRenderableResources(0u, InvalidResource1, true);

    requestMapScene();
    EXPECT_EQ(ESceneState::MapRequested, sceneStateExecutor.getSceneState(getSceneId()));

    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    expectResourceRequest(DisplayHandle1);
    setRenderableResources(0u, InvalidResource2, true);
    update();

    setRenderableResources(); // simulate upload
    expectResourceRequestCancel(InvalidResource1);
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, false, true);
    update();
    EXPECT_EQ(ESceneState::Mapped, sceneStateExecutor.getSceneState(getSceneId()));
    expectEvent(ERendererEventType_SceneMapped);

    expectResourceRequestCancel(InvalidResource2);
    update();

    expectContextEnable();
    expectRenderableResourcesDeleted();
    unmapScene();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canUnmapSceneWhenSceneIsMapRequestedWithMapFailedEvent)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();

    requestMapScene();
    EXPECT_EQ(ESceneState::MapRequested, sceneStateExecutor.getSceneState(getSceneId()));

    rendererSceneUpdater->handleSceneUnmappingRequest(getSceneId());
    expectEvents({ ERendererEventType_SceneMapFailed, ERendererEventType_SceneUnmapped });
    EXPECT_EQ(ESceneState::Subscribed, sceneStateExecutor.getSceneState(getSceneId()));

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canUnmapSceneWhenSceneIsMappingAndUploadingWithMapFailedEvent)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();

    requestMapScene();
    EXPECT_EQ(ESceneState::MapRequested, sceneStateExecutor.getSceneState(getSceneId()));

    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    expectContextEnable();
    rendererSceneUpdater->handleSceneUnmappingRequest(getSceneId());
    expectEvents({ ERendererEventType_SceneMapFailed, ERendererEventType_SceneUnmapped });
    EXPECT_EQ(ESceneState::Subscribed, sceneStateExecutor.getSceneState(getSceneId()));

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, unmappingSceneWhenSceneIsMappingAndUploadingWillUnloadItsResources)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();

    createRenderable();
    setRenderableResources(0u, InvalidResource1, true);

    requestMapScene();
    EXPECT_EQ(ESceneState::MapRequested, sceneStateExecutor.getSceneState(getSceneId()));

    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    expectContextEnable();
    expectRenderableResourcesDeleted(DisplayHandle1, true, false);
    rendererSceneUpdater->handleSceneUnmappingRequest(getSceneId());
    expectEvents({ ERendererEventType_SceneMapFailed, ERendererEventType_SceneUnmapped });
    EXPECT_EQ(ESceneState::Subscribed, sceneStateExecutor.getSceneState(getSceneId()));

    expectResourceRequestCancel(InvalidResource1);
    update();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canUnmapSceneWhenSceneIsMappingAndUploadingAndInMiddleOfPartialFlush)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene(); // need 2 scenes to allow partial flush processing

    createRenderable();
    setRenderableResources(0u, InvalidResource1, true);

    requestMapScene();
    EXPECT_EQ(ESceneState::MapRequested, sceneStateExecutor.getSceneState(getSceneId(0u)));

    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId(0u)));

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId(0u)));

    // simulate no time left for update operations
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::SceneActionsApply, 0u);

    performFlushWithCreateNodeAction(0, RendererSceneUpdater::SceneActionsPerChunkToApply + 1);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());
    // now has pending flush

    expectContextEnable();
    expectRenderableResourcesDeleted(DisplayHandle1, true, false);
    rendererSceneUpdater->handleSceneUnmappingRequest(getSceneId(0u));
    expectEvents({ ERendererEventType_SceneMapFailed, ERendererEventType_SceneUnmapped });
    EXPECT_EQ(ESceneState::Subscribed, sceneStateExecutor.getSceneState(getSceneId(0u)));

    expectResourceRequestCancel(InvalidResource1);
    update();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canHideSceneWhenSceneIsRequestedToBeShownButInMiddleOfPartialFlush_WillTriggerFailShowEvent)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene(); // need 2 scenes to allow partial flush processing

    createRenderable();
    setRenderableResources();

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded();
    mapScene();

    // request show
    rendererSceneUpdater->handleSceneShowRequest(getSceneId(0u));
    // simulate no time left for update operations
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::SceneActionsApply, 0u);
    performFlushWithCreateNodeAction(0, RendererSceneUpdater::SceneActionsPerChunkToApply * 3);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());
    // now has pending flush

    rendererSceneUpdater->handleSceneHideRequest(getSceneId(0u));
    expectEvents({ ERendererEventType_SceneShowFailed, ERendererEventType_SceneHidden });
    EXPECT_EQ(ESceneState::Mapped, sceneStateExecutor.getSceneState(getSceneId(0u)));

    update();
    expectNoEvent();

    expectContextEnable();
    expectRenderableResourcesDeleted();
    unmapScene();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, confidenceTest_canRemapSceneWhenSceneIsMappingAndUploadingAndInMiddleOfPartialFlush)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene(); // need 2 scenes to allow partial flush processing

    createRenderable();
    setRenderableResources(0u, InvalidResource1, true);

    requestMapScene();
    EXPECT_EQ(ESceneState::MapRequested, sceneStateExecutor.getSceneState(getSceneId(0u)));

    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId(0u)));

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId(0u)));

    // simulate no time left for update operations
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::SceneActionsApply, 0u);

    performFlushWithCreateNodeAction(0, RendererSceneUpdater::SceneActionsPerChunkToApply * 3);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());
    // now has pending flush

    expectContextEnable();
    rendererSceneUpdater->handleSceneUnmappingRequest(getSceneId(0u));
    {
        expectEvents({ ERendererEventType_SceneMapFailed, ERendererEventType_SceneUnmapped });
        EXPECT_EQ(ESceneState::Subscribed, sceneStateExecutor.getSceneState(getSceneId(0u)));
    }

    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::SceneActionsApply, 0xffffffff); // unblock update time budget
    expectResourceRequestCancel(InvalidResource1);
    update();

    //////////////////
    // remap the scene
    requestMapScene();
    EXPECT_EQ(ESceneState::MapRequested, sceneStateExecutor.getSceneState(getSceneId(0u)));
    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId(0u)));

    // unblock resource
    setRenderableResources(); // simulate upload
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, false, true);
    update();
    EXPECT_EQ(ESceneState::Mapped, sceneStateExecutor.getSceneState(getSceneId(0u)));
    expectEvent(ERendererEventType_SceneMapped);

    expectContextEnable();
    expectRenderableResourcesDeleted();
    unmapScene();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, renderTargetIsUploadedInCorrectOrderRightAfterUnblockedMappingBeforeResourceCacheUpdate)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    createRenderable();

    requestMapScene();
    EXPECT_EQ(ESceneState::MapRequested, sceneStateExecutor.getSceneState(getSceneId()));
    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));
    // scene with RT is now internally mapped but RT is not uploaded yet

    // blocking sync flush
    createRenderTargetWithBuffers();
    setRenderableResources(0u, InvalidResource1, true);
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    // unblock flush by making resource available
    setRenderableResources(); // simulate upload

    // only now is RT uploaded
    expectResourceRequest();
    expectContextEnable();
    expectRenderTargetUploaded();
    expectRenderableResourcesUploaded(DisplayHandle1, false, true);
    update();
    EXPECT_EQ(ESceneState::Mapped, sceneStateExecutor.getSceneState(getSceneId()));
    expectEvent(ERendererEventType_SceneMapped);

    expectResourceRequestCancel(InvalidResource1);
    update();

    expectContextEnable();
    expectRenderTargetDeleted();
    expectRenderableResourcesDeleted();
    unmapScene();

    destroyDisplay();
}

// This is to reproduce and prove fix for a crash that happens in very very special constellation
TEST_F(ARendererSceneUpdater, renderTargetIsUploadedInCorrectOrderAfterSceneMappedWithShowRequestAndBlockingFlushComingAtSameTime)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();

    // create renderable with invalid (unresolvable) resource
    createRenderTargetWithBuffers();
    createRenderable();
    setRenderableResources(0u, InvalidResource1, true);

    // map scene
    requestMapScene();
    EXPECT_EQ(ESceneState::MapRequested, sceneStateExecutor.getSceneState(getSceneId()));
    expectContextEnable();
    expectRenderTargetUploaded();
    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    // do one loop in mapped state that will upload resolvable resources
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    // swap invalid resource for another invalid (this will put the first invalid into 'to be unreferenced' list in current implementation)
    expectResourceRequest();
    setRenderableResources(0u, InvalidResource2, true);
    expectResourceRequestCancel(InvalidResource1);
    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    // unblock mapping by switching to a valid resource
    setRenderableResources(); // switch to valid resource
    expectResourceRequestCancel(InvalidResource2);

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, false, true);
    update();
    EXPECT_EQ(ESceneState::Mapped, sceneStateExecutor.getSceneState(getSceneId()));
    expectEvent(ERendererEventType_SceneMapped);

    //////////////////
    // unmap scene will unload render target, invalid resources is still in 'to be unreferenced' list
    expectContextEnable();
    expectRenderTargetDeleted();
    unmapScene();
    update();

    // request map
    requestMapScene();
    EXPECT_EQ(ESceneState::MapRequested, sceneStateExecutor.getSceneState(getSceneId()));
    expectContextEnable();
    expectRenderTargetUploaded();
    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    // the scene will get to mapped state and within that very frame do:
    // 1. add dummy sync flush
    // 2. switch to rendered state (show scene)
    performFlush(0u, true);
    update();
    EXPECT_EQ(ESceneState::Mapped, sceneStateExecutor.getSceneState(getSceneId()));
    expectEvent(ERendererEventType_SceneMapped);
    // request show scene
    showScene();

    // if render target (or any other scene resources) is not uploaded, this would crash/assert
    update();

    hideScene();
    expectContextEnable();
    expectRenderTargetDeleted();
    expectRenderableResourcesDeleted();
    unmapScene();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, syncFlush_multipleFlushesWithRemoveAndAddResourceReferenceWhileAlreadyBlockedByInvalidResource)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    // create blocking sync flush (using invalid resource)
    createRenderable(0u, true, true);
    setRenderableResources(0u, InvalidResource1, true);

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // add new resource reference
    setRenderableVertexArray(0u, ResourceProviderMock::FakeVertArrayHash, true);
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, false, false, true);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // remove and add again the previously added resource reference
    setRenderableVertexArray(0u, InvalidResource1, true);
    setRenderableVertexArray(0u, ResourceProviderMock::FakeVertArrayHash, true);

    // update
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // unblock flushes
    expectResourceRequestCancel(InvalidResource1);
    setRenderableResources(0u, ResourceProviderMock::FakeIndexArrayHash, true);
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, false, true, false);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    // update
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    // uninit
    hideScene();
    expectContextEnable();
    expectRenderableResourcesDeleted(DisplayHandle1, true, true, true);
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, syncFlush_multipleFlushesWithAddAndRemoveResourceReferenceWhileAlreadyBlockedByInvalidResource)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    // create blocking sync flush (using invalid resource)
    createRenderable(0u, true, true);
    setRenderableResources(0u, InvalidResource1, true);

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // add new resource reference
    setRenderableVertexArray(0u, ResourceProviderMock::FakeVertArrayHash, true);
    setRenderableVertexArray(0u, InvalidResource1, true);

    // pending resources is uploaded even if it is replaced in another pending flush
    expectContextEnable();
    expectResourceRequest();
    expectRenderableResourcesUploaded(DisplayHandle1, false, false, true);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // unblock flushes
    setRenderableResources(0u, ResourceProviderMock::FakeIndexArrayHash, true);
    setRenderableVertexArray(0u, ResourceProviderMock::FakeVertArrayHash, true);
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, false, true, false);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    // update
    expectResourceRequestCancel(InvalidResource1);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    // uninit
    hideScene();
    expectContextEnable();
    expectRenderableResourcesDeleted(DisplayHandle1, true, true, true);
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, syncFlush_multipleFlushesWithRemoveAndAddAndRemoveResourceReferenceWhileAlreadyBlockedByInvalidResource)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    // create blocking sync flush (using invalid resource)
    createRenderable(0u, true, true);
    setRenderableResources(0u, InvalidResource1, true);

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // add new resource reference
    setRenderableVertexArray(0u, ResourceProviderMock::FakeVertArrayHash, true);
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, false, false, true);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // remove, add and remove again the previously added resource reference
    setRenderableVertexArray(0u, InvalidResource1, true);
    setRenderableVertexArray(0u, ResourceProviderMock::FakeVertArrayHash, true);
    setRenderableVertexArray(0u, InvalidResource1, true);

    // update and expect no change
    // the vertex array resource would be unloaded once sync flushes are unblocked
    // unless it is referenced again which is the case in this test in the following section
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // unblock flushes
    setRenderableResources(0u, ResourceProviderMock::FakeIndexArrayHash, true);
    setRenderableVertexArray(0u, ResourceProviderMock::FakeVertArrayHash, true);
    expectResourceRequestCancel(InvalidResource1);
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, false, true, false);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    // update
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    // uninit
    hideScene();
    expectContextEnable();
    expectRenderableResourcesDeleted(DisplayHandle1, true, true, true);
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, syncFlush_multipleFlushesWithAddAndRemoveAndAddResourceReferenceWhileAlreadyBlockedByInvalidResource)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    // create blocking sync flush (using invalid resource)
    createRenderable(0u, true, true);
    setRenderableResources(0u, InvalidResource1, true);

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // add new resource reference
    setRenderableVertexArray(0u, ResourceProviderMock::FakeVertArrayHash, true);
    setRenderableVertexArray(0u, InvalidResource1, true);
    setRenderableVertexArray(0u, ResourceProviderMock::FakeVertArrayHash, true);

    // update and expect exactly one resource request/upload
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, false, false, true);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // unblock flushes
    expectResourceRequestCancel(InvalidResource1);
    setRenderableResources(0u, ResourceProviderMock::FakeIndexArrayHash, true);
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, false, true, false);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    // update
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    // uninit
    hideScene();
    expectContextEnable();
    expectRenderableResourcesDeleted(DisplayHandle1, true, true, true);
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, pendingResourceAndProvidedResourceAreBothUnreferencedAtTheSameTime_async)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    update();

    createRenderable();
    setRenderableResources(0u, InvalidResource1);

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();

    removeRenderableResources();

    expectContextEnable();
    expectRenderableResourcesDeleted(DisplayHandle1, true, false);
    update();

    performFlush();
    expectResourceRequestCancel(InvalidResource1);
    update();

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, queriesRendererResourceCacheBeforeRequestingResources)
{
    // These are the (default) resources provided by "setRenderableResources()". In this test we are just
    // interested in verifying that the SceneUpdater calls the cache. The actual cache flow is tested elsewhere.
    EXPECT_CALL(rendererResourceCacheMock, hasResource(ResourceProviderMock::FakeEffectHash, _)).Times(1);
    EXPECT_CALL(rendererResourceCacheMock, hasResource(ResourceProviderMock::FakeIndexArrayHash, _)).Times(1);

    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    createRenderable();
    setRenderableResources();
    update();

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded();
    mapScene();

    expectContextEnable();
    expectRenderableResourcesDeleted();
    unmapScene();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, willMapSceneAfterMaximumNumberOfPendingFlushesReached)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();

    createRenderable();
    setRenderableResources(0u, InvalidResource1, true);
    createRenderTargetWithBuffers();

    requestMapScene();
    EXPECT_EQ(ESceneState::MapRequested, sceneStateExecutor.getSceneState(getSceneId()));

    expectContextEnable();
    expectRenderTargetUploaded();
    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    // add blocking sync flush so that upcoming flushes are queuing up
    setRenderableResources(0u, InvalidResource2, true);
    expectResourceRequestCancel(InvalidResource1);

    EXPECT_CALL(resourceProvider1, requestResourceAsyncronouslyFromFramework(_, _, getSceneId())).Times(AnyNumber());
    for (UInt i = 0u; i < ForceApplyFlushesLimit + 1u; ++i)
    {
        performFlush(0u, true);
        update();
    }
    EXPECT_EQ(ESceneState::Mapped, sceneStateExecutor.getSceneState(getSceneId()));
    expectEvent(ERendererEventType_SceneMapped);

    setRenderableResources(); // simulate upload

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, false, true);
    update();

    expectResourceRequestCancel(InvalidResource2);
    update();

    expectContextEnable();
    expectRenderableResourcesDeleted();
    expectRenderTargetDeleted();
    unmapScene();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, forceAppliesPendingFlushesAfterMaximumNumberReachedWhenSceneMappedOrRendered)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();

    createRenderable();
    setRenderableResources();

    requestMapScene();
    EXPECT_EQ(ESceneState::MapRequested, sceneStateExecutor.getSceneState(getSceneId()));
    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded();
    update();
    EXPECT_EQ(ESceneState::Mapped, sceneStateExecutor.getSceneState(getSceneId()));
    expectEvent(ERendererEventType_SceneMapped);

    // add blocking sync flush so that upcoming flushes are queuing up
    setRenderableResources(0u, InvalidResource1, true);
    createRenderTargetWithBuffers();
    expectResourceRequest();
    update();

    // mapped state
    {
        // flushes are blocked due to unresolved resource
        const SceneVersionTag pendingFlushTag(123u);
        performFlush(0u, true, pendingFlushTag);
        update();
        expectNoEvent();

        expectContextEnable();
        expectRenderTargetUploaded();
        EXPECT_CALL(resourceProvider1, requestResourceAsyncronouslyFromFramework(_, _, getSceneId())).Times(AnyNumber());
        for (UInt i = 0u; i < ForceApplyFlushesLimit + 1u; ++i)
        {
            performFlush(0u, true);
            update();
        }

        // after maximum of pending flushes was reached the flushes were applied regardless of missing resource
        expectEvent(ERendererEventType_SceneFlushed);
    }

    showScene();

    // rendered state
    {
        // add blocking sync flush so that upcoming flushes are queuing up
        setRenderableResources(0u, InvalidResource2, true);
        expectResourceRequestCancel(InvalidResource1);

        // flushes are blocked due to unresolved resource
        const SceneVersionTag pendingFlushTag(124u);
        performFlush(0u, true, pendingFlushTag);
        update();
        expectNoEvent();

        for (UInt i = 0u; i < ForceApplyFlushesLimit + 1u; ++i)
        {
            performFlush(0u, true);
            update();
        }

        // after maximum of pending flushes was reached the flushes were applied regardless of missing resource
        expectEvent(ERendererEventType_SceneFlushed);
    }

    hideScene();
    expectContextEnable();
    expectRenderableResourcesDeleted();
    expectRenderTargetDeleted();
    unmapScene();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, reactsOnDynamicChangesOfFlushForceApplyLimit)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();

    createRenderable();
    setRenderableResources();

    requestMapScene();
    EXPECT_EQ(ESceneState::MapRequested, sceneStateExecutor.getSceneState(getSceneId()));
    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded();
    update();
    EXPECT_EQ(ESceneState::Mapped, sceneStateExecutor.getSceneState(getSceneId()));
    expectEvent(ERendererEventType_SceneMapped);

    // add blocking sync flush so that upcoming flushes are queuing up
    setRenderableResources(0u, InvalidResource1, true);
    createRenderTargetWithBuffers();
    expectResourceRequest();
    update();

    // Reduce flush limit -> expect force flush earlier
    constexpr UInt newShorterFlushLimit = ForceApplyFlushesLimit / 2u;
    rendererSceneUpdater->setLimitFlushesForceApply(newShorterFlushLimit);

    // mapped state
    {
        // flushes are blocked due to unresolved resource
        const SceneVersionTag pendingFlushTag(123u);
        performFlush(0u, true, pendingFlushTag);
        update();
        expectNoEvent();

        expectContextEnable();
        expectRenderTargetUploaded();

        EXPECT_CALL(resourceProvider1, requestResourceAsyncronouslyFromFramework(_, _, getSceneId())).Times(AnyNumber());
        for (UInt i = 0u; i < ForceApplyFlushesLimit + 1u; ++i)
        {
            performFlush(0u, true);
            update();
            if (i == newShorterFlushLimit - 1)
            {
                // after newly set maximum of pending flushes was reached the flushes were applied regardless of missing resource
                expectEvent(ERendererEventType_SceneFlushed);
            }
        }

        // Remaining flushes still blocked, because resource still missing
        expectNoEvent();
    }

    showScene();

    // rendered state
    {
        // add blocking sync flush so that upcoming flushes are queuing up
        setRenderableResources(0u, InvalidResource2, true);
        expectResourceRequestCancel(InvalidResource1);

        // flushes are blocked due to unresolved resource
        const SceneVersionTag pendingFlushTag(124u);
        performFlush(0u, true, pendingFlushTag);
        update();
        expectNoEvent();

        for (UInt i = 0u; i < ForceApplyFlushesLimit + 1u; ++i)
        {
            performFlush(0u, true);
            update();
            if (i == newShorterFlushLimit - 1)
            {
                // after newly set maximum of pending flushes was reached the flushes were applied regardless of missing resource
                expectEvent(ERendererEventType_SceneFlushed);
            }
        }

        // Remaining flushes still blocked, because resource still missing
        expectNoEvent();
    }

    hideScene();
    expectContextEnable();
    expectRenderableResourcesDeleted();
    expectRenderTargetDeleted();
    unmapScene();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, applyingPendingFlushesAfterMaximumNumberOfPendingFlushesReachedDoesNotAffectOtherScene)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();

    createRenderable(0u);
    createRenderable(1u);
    setRenderableResources(0u);
    setRenderableResources(1u);

    expectResourceRequest(DisplayHandle1, 0u);
    expectResourceRequest(DisplayHandle1, 1u);
    expectContextEnable();
    expectRenderableResourcesUploaded();
    mapScene(0u);
    mapScene(1u);
    showScene(0u);
    showScene(1u);

    setRenderableResources(0u, InvalidResource1, true);
    setRenderableResources(1u, InvalidResource1, true);
    expectResourceRequest();
    update();

    {
        // flushes are blocked due to unresolved resource
        const SceneVersionTag pendingFlushTag0(111u);
        const SceneVersionTag pendingFlushTag1(222u);
        performFlush(0u, true, pendingFlushTag0);
        performFlush(1u, true, pendingFlushTag1);
        update();
        expectNoEvent();

        for (UInt i = 0u; i < ForceApplyFlushesLimit + 1u; ++i)
        {
            performFlush(1u, true);
            update();
        }

        // after maximum of pending flushes was reached the flushes were applied regardless of missing resource
        // but only for scene 1, scene 0 stays blocked as the number of pending flushes there is below the maximum treshold
        RendererEventVector events;
        rendererEventCollector.dispatchEvents(events);
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType_SceneFlushed, events.front().eventType);
        EXPECT_EQ(getSceneId(1u), events.front().sceneId);
        EXPECT_EQ(pendingFlushTag1, events.front().sceneVersionTag);

        // repeat for scene 0
        for (UInt i = 0u; i < ForceApplyFlushesLimit + 1u; ++i)
        {
            performFlush(0u, true);
            update();
        }

        // after maximum of pending flushes was reached the flushes were applied regardless of missing resource
        // but only for scene 0 now
        events.clear();
        rendererEventCollector.dispatchEvents(events);
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType_SceneFlushed, events.front().eventType);
        EXPECT_EQ(getSceneId(0u), events.front().sceneId);
        EXPECT_EQ(pendingFlushTag0, events.front().sceneVersionTag);
    }

    hideScene(0u);
    hideScene(1u);
    expectContextEnable(DisplayHandle1, 2u);
    expectRenderableResourcesDeleted();
    unmapScene(0u);
    unmapScene(1u);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, forceUnsubscribesSceneIfItHasTooManyPendingFlushesDueToNoTimeToApplyThem_Subscribed)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene(); // create second scene to enable partial scene actions applying

    // give no time budget to apply flushes
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::SceneActionsApply, 0u);

    performFlushWithCreateNodeAction(0, RendererSceneUpdater::SceneActionsPerChunkToApply * ForceUnsubscribeFlushLimit * 2, true);

    ASSERT_TRUE(ForceApplyFlushesLimit < ForceUnsubscribeFlushLimit); // adjust test logic!
    for (UInt i = 0u; i < ForceUnsubscribeFlushLimit - 1u; ++i)
    {
        performFlush(0u, true);
        update();
    }

    // even though flushes were 'force applied' at least once, no flush was actually applied due to no time left for scene actions applying
    expectNoEvent();

    // expect scene unsubscribe, explained below
    EXPECT_CALL(sceneGraphConsumerComponent, unsubscribeScene(_, _));

    // 1 more flush to trigger limit
    performFlush(0u, true);
    update();
    // at this point maximum number of pending flushes reached to 'kill' the scene,
    // we give up on trying to apply flushes, scene is unsubscribed to prevent more harm
    expectEvent(ERendererEventType_SceneUnsubscribedIndirect);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, forceUnsubscribesSceneIfItHasTooManyPendingFlushesDueToNoTimeToApplyThem_Mapped)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene(); // create second scene to enable partial scene actions applying

    createRenderable();
    setRenderableResources();

    requestMapScene();
    EXPECT_EQ(ESceneState::MapRequested, sceneStateExecutor.getSceneState(getSceneId()));
    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded();
    update();
    EXPECT_EQ(ESceneState::Mapped, sceneStateExecutor.getSceneState(getSceneId()));
    expectEvent(ERendererEventType_SceneMapped);

    // add blocking sync flush so that upcoming flushes are queuing up
    setRenderableResources(0u, InvalidResource1, true);
    createRenderTargetWithBuffers();
    expectResourceRequest();
    update();

    // flushes are blocked due to unresolved resource
    performFlushWithCreateNodeAction(0, RendererSceneUpdater::SceneActionsPerChunkToApply * ForceUnsubscribeFlushLimit * 2, true);
    update();
    expectNoEvent();

    EXPECT_CALL(resourceProvider1, requestResourceAsyncronouslyFromFramework(_, _, getSceneId())).Times(AnyNumber());

    // give no time budget to apply flushes
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::SceneActionsApply, 0u);

    ASSERT_TRUE(ForceApplyFlushesLimit < ForceUnsubscribeFlushLimit); // Fix test logic

    for (UInt i = 0u; i < ForceUnsubscribeFlushLimit - 1u; ++i)
    {
        performFlush(0u, true);
        update();
    }

    // even though flushes were 'force applied' at least once, no flush was actually applied due to no time left for scene actions applying
    expectNoEvent();

    // expect scene unsubscribe, explained below
    expectContextEnable();
    expectResourceRequestCancel(InvalidResource1);
    expectRenderableResourcesDeleted();
    EXPECT_CALL(sceneGraphConsumerComponent, unsubscribeScene(_, _));

    // 1 more flush to trigger limit
    performFlush(0u, true);
    update();
    // at this point maximum number of pending flushes reached to 'kill' the scene,
    // we give up on trying to apply flushes, scene is unsubscribed to prevent more harm
    expectEvents({ ERendererEventType_SceneUnmappedIndirect, ERendererEventType_SceneUnsubscribedIndirect });

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, reactsToChangesToForceUnsubscribeFlushLimit)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene(); // create second scene to enable partial scene actions applying

    createRenderable();
    setRenderableResources();

    requestMapScene();
    EXPECT_EQ(ESceneState::MapRequested, sceneStateExecutor.getSceneState(getSceneId()));
    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded();
    update();
    EXPECT_EQ(ESceneState::Mapped, sceneStateExecutor.getSceneState(getSceneId()));
    expectEvent(ERendererEventType_SceneMapped);

    // add blocking sync flush so that upcoming flushes are queuing up
    setRenderableResources(0u, InvalidResource1, true);
    createRenderTargetWithBuffers();
    expectResourceRequest();
    update();

    // flushes are blocked due to unresolved resource
    const SceneVersionTag pendingFlushTag(123u);
    performFlush(0u, true, pendingFlushTag);
    update();
    expectNoEvent();

    EXPECT_CALL(resourceProvider1, requestResourceAsyncronouslyFromFramework(_, _, getSceneId())).Times(AnyNumber());

    // give no time budget to apply flushes
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::SceneActionsApply, 0u);

    // Reduce flush limit -> scene will be thrown out earlier
    constexpr UInt newShorterForceUnsubscribeLimit = ForceUnsubscribeFlushLimit / 2u;

    ASSERT_TRUE(ForceApplyFlushesLimit < ForceUnsubscribeFlushLimit); // Fix test logic
    // -3 because there are some hidden flushes in the test methods above
    for (UInt i = 0u; i < newShorterForceUnsubscribeLimit - 3u; ++i)
    {
        performFlush(0u, true);
        update();
    }

    // even though flushes were 'force applied' at least once, no flush was actually applied due to no time left for scene actions applying
    expectNoEvent();

    rendererSceneUpdater->setLimitFlushesForceUnsubscribe(newShorterForceUnsubscribeLimit);

    // still no event, need one more flush to trigger unsubscribe
    expectNoEvent();

    // expect scene unsubscribe, explained below
    expectContextEnable();
    expectResourceRequestCancel(InvalidResource1);
    expectRenderableResourcesDeleted();
    EXPECT_CALL(sceneGraphConsumerComponent, unsubscribeScene(_, _));

    // 1 more flush to trigger limit
    performFlush(0u, true);
    update();
    // at this point maximum number of pending flushes reached to 'kill' the scene,
    // we give up on trying to apply flushes, scene is unsubscribed to prevent more harm
    expectEvents({ ERendererEventType_SceneUnmappedIndirect, ERendererEventType_SceneUnsubscribedIndirect });

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, nonBlockingFlushesGetAppliedEvenIfSceneIsBlockedToBeMappedDueToInvalidResourceFromBefore)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();

    createRenderable();
    setRenderableResources(0u, InvalidResource1, true);

    requestMapScene();
    EXPECT_EQ(ESceneState::MapRequested, sceneStateExecutor.getSceneState(getSceneId()));

    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));
    // scene is now blocked to be mapped due to use of InvalidResource1

    // blocking sync flush cannot be applied
    createRenderTargetWithBuffers();
    expectResourceRequest();
    setRenderableResources(0u, InvalidResource2, true);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());
    // scene still cannot be mapped because it still uses InvalidResource1, the switch to InvalidResource2 is pending
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    // switch back to InvalidResource1 meaning that the pending flushes combined are non-blocking
    setRenderableResources(0u, InvalidResource1, true);
    expectContextEnable();
    expectRenderTargetUploaded();
    update();
    // pending flushes were applied even though scene is waiting to be mapped
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());
    // scene still cannot be mapped because it still uses InvalidResource1
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    expectResourceRequestCancel(InvalidResource2);
    update();

    // remove InvalidResource1 and allow scene to get mapped
    setRenderableResources(); // simulate upload
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, false, true);
    update();
    expectEvent(ERendererEventType_SceneMapped);
    EXPECT_EQ(ESceneState::Mapped, sceneStateExecutor.getSceneState(getSceneId()));

    expectResourceRequestCancel(InvalidResource1);
    update();

    expectContextEnable();
    expectRenderableResourcesDeleted();
    expectRenderTargetDeleted();
    unmapScene();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canUnmapSceneWithPendingFlushAndRequestMapInSingleLoop)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    createRenderable();
    setRenderableResources(0u, InvalidResource1, true);

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();
    // there are pending flushes due to use of InvalidResource1
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // unmap and request map without update in between
    expectContextEnable();
    unmapScene();
    requestMapScene();
    EXPECT_EQ(ESceneState::MapRequested, sceneStateExecutor.getSceneState(getSceneId()));

    // InvalidResource1 was unrequested when unmapped and cancel processed in next update
    // however it is in use by scene and will be requested again when scene is mapped
    expectResourceRequestCancel(InvalidResource1);
    // scene is not mapped so pending flushes are applied to scene
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());
    // and scene switches to mapping/uploading state
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    // next update causes all resources in use to be requested and uploaded if available
    expectResourceRequest();
    // normally resources would be re-uploaded here but due to resource manager implementation
    // unreferenced resources are unloaded only when new resource needs to be uploaded
    // which is not the case here therefore no upload happens
    //expectContextEnable();
    //expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();
    // scene is now blocked to be mapped due to use of InvalidResource1
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    // remove InvalidResource1 and allow scene to get mapped
    setRenderableResources(); // simulate upload
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, false, true);
    update();
    expectEvent(ERendererEventType_SceneMapped);
    EXPECT_EQ(ESceneState::Mapped, sceneStateExecutor.getSceneState(getSceneId()));

    expectResourceRequestCancel(InvalidResource1);
    update();

    expectContextEnable();
    expectRenderableResourcesDeleted();
    unmapScene();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canUnmapSceneWithPendingFlushAndRequestMapAndAddAnotherPendingFlushInSingleLoop)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    createRenderable();
    setRenderableResources(0u, InvalidResource1, true);

    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();
    // there are pending flushes due to use of InvalidResource1
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // unmap and request map without update in between
    expectContextEnable();
    unmapScene();
    requestMapScene();
    // in addition add one more blocking flush
    setRenderableResources(0u, InvalidResource2, true);
    EXPECT_EQ(ESceneState::MapRequested, sceneStateExecutor.getSceneState(getSceneId()));

    // InvalidResource1 was unrequested when unmapped and cancel processed in next update
    // it will be also replaced with InvalidResource2 when flush from above gets applied so it will not be requested again when mapped
    expectResourceRequestCancel(InvalidResource1);
    // scene is not mapped so pending flushes are applied to scene
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());
    // and scene switches to mapping/uploading state
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    // next update causes all resources in use to be requested and uploaded if available
    expectResourceRequest();
    // normally resources would be re-uploaded here but due to resource manager implementation
    // unreferenced resources are unloaded only when new resource needs to be uploaded
    // which is not the case here therefore no upload happens
    //expectContextEnable();
    //expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();
    // scene is now blocked to be mapped due to use of InvalidResource1
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    // remove InvalidResource1 and allow scene to get mapped
    setRenderableResources(); // simulate upload
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, false, true);
    update();
    expectEvent(ERendererEventType_SceneMapped);
    EXPECT_EQ(ESceneState::Mapped, sceneStateExecutor.getSceneState(getSceneId()));

    expectResourceRequestCancel(InvalidResource2);
    update();
    update();

    expectContextEnable();
    expectRenderableResourcesDeleted();
    unmapScene();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, forceUnsubscribesSceneIfSceneResourcesUploadExceedsBudgetWhileMapping)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();

    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::SceneResourcesUpload, 0u);

    // create many scene resources (if not enough scene resources the budget is not even checked)
    for (UInt32 i = 0; i < 40; ++i)
    {
        createRenderTargetWithBuffers(0u, false, RenderTargetHandle{ i }, RenderBufferHandle{ i * 2 }, RenderBufferHandle{ i * 2 + 1 });
        update();
        expectNoEvent();
    }

    // expect scene unsubscribe
    requestMapScene();

    // context is enabled twice, first before uploading, second when unloading due to forced unsubscribe/destroy
    expectContextEnable(DisplayHandle1, 2u);
    EXPECT_CALL(renderer.getDisplayMock(DisplayHandle1).m_renderBackend->deviceMock, uploadRenderBuffer(_)).Times(AtLeast(2));
    EXPECT_CALL(renderer.getDisplayMock(DisplayHandle1).m_renderBackend->deviceMock, uploadRenderTarget(_)).Times(AnyNumber());
    // render buffers are collected first therefore render targets might or might not be uploaded before interruption, depending on checking frequency (internal logic of scene resources uploader)

    EXPECT_CALL(sceneGraphConsumerComponent, unsubscribeScene(_, _));
    EXPECT_CALL(renderer.getDisplayMock(DisplayHandle1).m_renderBackend->deviceMock, deleteRenderBuffer(_)).Times(AtLeast(2));
    EXPECT_CALL(renderer.getDisplayMock(DisplayHandle1).m_renderBackend->deviceMock, deleteRenderTarget(_)).Times(AnyNumber());

    update();
    expectEvents({ ERendererEventType_SceneMapFailed, ERendererEventType_SceneUnsubscribedIndirect });

    update();
    expectNoEvent();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, forceUnsubscribesSceneIfSceneResourcesUploadExceedsBudgetWhileMapping_doesNotAffectOtherAlreadyRenderedScene)
{
    createDisplayAndExpectSuccess();
    const auto sceneIdx1 = createPublishAndSubscribeScene();

    // create scene with some resources and render it
    mapScene(sceneIdx1);
    createRenderable(sceneIdx1);
    setRenderableResources(sceneIdx1);
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded();
    update();
    showScene(sceneIdx1);

    // next one is malicious scene with too many scene resources
    const auto sceneIdx2 = createPublishAndSubscribeScene();
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::SceneResourcesUpload, 0u);

    // create many scene resources (if not enough scene resources the budget is not even checked)
    for (UInt32 i = 0; i < 40; ++i)
    {
        createRenderTargetWithBuffers(sceneIdx2, false, RenderTargetHandle{ i }, RenderBufferHandle{ i * 2 }, RenderBufferHandle{ i * 2 + 1 });
        update();
        expectNoEvent();
    }

    // expect scene unsubscribe
    requestMapScene(sceneIdx2);

    // context is enabled twice, first before uploading, second when unloading due to forced unsubscribe/destroy
    expectContextEnable(DisplayHandle1, 2u);
    EXPECT_CALL(renderer.getDisplayMock(DisplayHandle1).m_renderBackend->deviceMock, uploadRenderBuffer(_)).Times(AtLeast(2));
    EXPECT_CALL(renderer.getDisplayMock(DisplayHandle1).m_renderBackend->deviceMock, uploadRenderTarget(_)).Times(AnyNumber());
    // render buffers are collected first therefore render targets might or might not be uploaded before interruption, depending on checking frequency (internal logic of scene resources uploader)

    EXPECT_CALL(sceneGraphConsumerComponent, unsubscribeScene(_, _));
    EXPECT_CALL(renderer.getDisplayMock(DisplayHandle1).m_renderBackend->deviceMock, deleteRenderBuffer(_)).Times(AtLeast(2));
    EXPECT_CALL(renderer.getDisplayMock(DisplayHandle1).m_renderBackend->deviceMock, deleteRenderTarget(_)).Times(AnyNumber());

    update();
    expectEvents({ ERendererEventType_SceneMapFailed, ERendererEventType_SceneUnsubscribedIndirect });

    update();
    expectNoEvent();

    // hide and unmap first scene
    hideScene(sceneIdx1);
    expectContextEnable();
    expectRenderableResourcesDeleted();
    unmapScene(sceneIdx1);

    destroyDisplay();
}

}
