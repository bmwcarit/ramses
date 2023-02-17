//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererSceneUpdaterTest.h"
#include <array>

namespace ramses_internal {

///////////////////////////
// General tests
///////////////////////////

TEST_F(ARendererSceneUpdater, doesNotGenerateRendererEventForUnnamedFlush)
{
    createPublishAndSubscribeScene();
    expectNoEvent();

    performFlush();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());
    expectNoEvent();
}

TEST_F(ARendererSceneUpdater, generatesRendererEventForNamedFlush)
{
    createPublishAndSubscribeScene();
    expectNoEvent();

    const SceneVersionTag version(15u);
    performFlush(0u, version);
    update();

    RendererEventVector events;
    RendererEventVector dummy;
    rendererEventCollector.appendAndConsumePendingEvents(dummy, events);
    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(ERendererEventType::SceneFlushed, events[0].eventType);
    EXPECT_EQ(version, events[0].sceneVersionTag);
}

TEST_F(ARendererSceneUpdater, lastAppliedFlushVersionTagIsTracked)
{
    createPublishAndSubscribeScene();
    expectNoEvent();

    EXPECT_FALSE(rendererScenes.getStagingInfo(getSceneId()).lastAppliedVersionTag.isValid());

    SceneVersionTag version{ 15u };
    performFlush(0u, version);
    update();
    expectSceneEvent(ERendererEventType::SceneFlushed);
    EXPECT_EQ(version, rendererScenes.getStagingInfo(getSceneId()).lastAppliedVersionTag);

    version = SceneVersionTag{ 1111u };
    performFlush(0u, version);
    update();
    expectSceneEvent(ERendererEventType::SceneFlushed);
    EXPECT_EQ(version, rendererScenes.getStagingInfo(getSceneId()).lastAppliedVersionTag);

    // invalid version tag is also tracked
    performFlush(0u, SceneVersionTag::Invalid());
    update();
    EXPECT_FALSE(rendererScenes.getStagingInfo(getSceneId()).lastAppliedVersionTag.isValid());

    version = SceneVersionTag{ 2222u };
    performFlush(0u, version);
    update();
    expectSceneEvent(ERendererEventType::SceneFlushed);
    EXPECT_EQ(version, rendererScenes.getStagingInfo(getSceneId()).lastAppliedVersionTag);
}

TEST_F(ARendererSceneUpdater, preallocatesSceneForSizesProvidedInFlushInfo)
{
    createPublishAndSubscribeScene();
    expectNoEvent();

    // perform empty flush only with notification of scene size change
    SceneSizeInformation sizeInfo;
    sizeInfo.renderableCount = 12u;
    performFlush(0u, SceneVersionTag::Invalid(), &sizeInfo);
    update();

    const IScene& rendererScene = rendererScenes.getScene(getSceneId());
    EXPECT_EQ(sizeInfo, rendererScene.getSceneSizeInformation());
}

TEST_F(ARendererSceneUpdater, ignoresSceneActionsForNotSubscribedScene)
{
    createStagingScene();
    publishScene();

    performFlush(0u, SceneVersionTag(1u));
    update();
    // there is no scene to apply actions to
    expectNoEvent();
}

TEST_F(ARendererSceneUpdater, ignoresSceneActionsForNotReceivedScene)
{
    createStagingScene();
    publishScene();
    requestSceneSubscription();

    performFlush(0u, SceneVersionTag(1u));
    update();
    // there is no scene to apply actions to
    expectNoEvent();
}

TEST_F(ARendererSceneUpdater, ignoresSceneActionsAddedAfterSceneWasUnsubscribed)
{
    createPublishAndSubscribeScene();

    EXPECT_CALL(sceneEventSender, sendUnsubscribeScene(getSceneId()));
    rendererSceneUpdater->handleSceneUnsubscriptionRequest(getSceneId(), false);
    EXPECT_EQ(ESceneState::Published, sceneStateExecutor.getSceneState(getSceneId()));
    expectInternalSceneStateEvent(ERendererEventType::SceneUnsubscribed);
    EXPECT_FALSE(rendererScenes.hasScene(getSceneId()));

    performFlush(0u, SceneVersionTag(1u));
    update();
    // there is no scene to apply actions to
    expectNoEvent();
}

TEST_F(ARendererSceneUpdater, ignoresSceneActionsAddedAfterSceneWasUnsubscribedByError)
{
    createPublishAndSubscribeScene();

    EXPECT_CALL(sceneEventSender, sendUnsubscribeScene(getSceneId()));
    rendererSceneUpdater->handleSceneUnsubscriptionRequest(getSceneId(), true);
    EXPECT_EQ(ESceneState::Published, sceneStateExecutor.getSceneState(getSceneId()));
    expectInternalSceneStateEvent(ERendererEventType::SceneUnsubscribedIndirect);
    EXPECT_FALSE(rendererScenes.hasScene(getSceneId()));

    performFlush(0u, SceneVersionTag(1u));
    update();
    // there is no scene to apply actions to
    expectNoEvent();
}

TEST_F(ARendererSceneUpdater, ignoresSceneActionsAddedBetweenUnsubscriptionAndResubscription)
{
    createPublishAndSubscribeScene();

    EXPECT_CALL(sceneEventSender, sendUnsubscribeScene(getSceneId()));
    rendererSceneUpdater->handleSceneUnsubscriptionRequest(getSceneId(), false);
    EXPECT_EQ(ESceneState::Published, sceneStateExecutor.getSceneState(getSceneId()));
    expectInternalSceneStateEvent(ERendererEventType::SceneUnsubscribed);

    performFlush(0u, SceneVersionTag(1u));

    EXPECT_CALL(sceneEventSender, sendSubscribeScene(getSceneId()));
    rendererSceneUpdater->handleSceneSubscriptionRequest(getSceneId());
    rendererSceneUpdater->handleSceneReceived(SceneInfo(getSceneId()));

    // named flush ignored
    expectNoEvent();
}

TEST_F(ARendererSceneUpdater, canCreateAndDestroyDisplayContext)
{
    createDisplayAndExpectSuccess();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, createDisplayFailsIfCreationOfResourceUploadRenderBackendFails)
{
    EXPECT_CALL(renderer.m_platform, createResourceUploadRenderBackend()).WillOnce(Return(nullptr));
    EXPECT_CALL(renderer.m_platform, destroyRenderBackend());
    rendererSceneUpdater->createDisplayContext({}, nullptr);
    EXPECT_FALSE(renderer.hasDisplayController());
    expectEvent(ERendererEventType::DisplayCreateFailed);
}

TEST_F(ARendererSceneUpdater, canNotDestroyNonExistantDisplay)
{
    destroyDisplay(true);
}

TEST_F(ARendererSceneUpdater, canNotDestroyDisplayIfItHasMapRequestedScenes)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    requestMapScene();

    destroyDisplay(true);

    unpublishMapRequestedScene();
    destroyDisplay();

    EXPECT_FALSE(renderer.hasDisplayController());
}

TEST_F(ARendererSceneUpdater, canNotDestroyDisplayIfItHasMappedScenes)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    destroyDisplay(true);

    unmapScene();
    destroyDisplay();

    EXPECT_FALSE(renderer.hasDisplayController());
}

TEST_F(ARendererSceneUpdater, canNotDestroyDisplayIfItHasRenderedScenes)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    destroyDisplay(true);
    EXPECT_TRUE(renderer.hasDisplayController());

    hideScene();
    unmapScene();
    destroyDisplay();

    EXPECT_FALSE(renderer.hasDisplayController());
}

TEST_F(ARendererSceneUpdater, canDestroyDisplayIfMappedSceneGetsUnmapped)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canDestroyDisplayIfMappedSceneGetsUnpublished)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    unpublishMappedScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, destroyingSceneUpdaterUnmapsAnyMappedSceneFromRenderer)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    EXPECT_CALL(renderer.m_platform, destroyResourceUploadRenderBackend());
    expectUnloadOfSceneResources();
    destroySceneUpdater();

    EXPECT_FALSE(renderer.getBufferSceneIsAssignedTo(getSceneId()).isValid());
}

TEST_F(ARendererSceneUpdater, destroyingSceneUpdaterDestroysAllDisplayContexts)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    EXPECT_CALL(renderer.m_platform, destroyResourceUploadRenderBackend());
    expectUnloadOfSceneResources();
    destroySceneUpdater();

    EXPECT_FALSE(renderer.hasDisplayController());
}

TEST_F(ARendererSceneUpdater, renderOncePassesAreRetriggeredWhenSceneMapped)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    auto& stageScene = *stagingScene[0];
    const RenderPassHandle pass = stageScene.allocateRenderPass();
    const auto dataLayout = stageScene.allocateDataLayout({ DataFieldInfo{EDataType::Vector2I}, DataFieldInfo{EDataType::Vector2I} }, ResourceContentHash::Invalid());
    const CameraHandle camera = stageScene.allocateCamera(ECameraProjectionType::Orthographic, stageScene.allocateNode(), stageScene.allocateDataInstance(dataLayout));
    stageScene.setRenderPassCamera(pass, camera);
    stageScene.setRenderPassRenderOnce(pass, true);
    performFlush();

    RendererCachedScene& scene = rendererScenes.getScene(stageScene.getSceneId());

    // simulate render frame
    // pass is in list to render
    update();
    scene.markAllRenderOncePassesAsRendered();
    EXPECT_EQ(pass, scene.getSortedRenderingPasses()[0].getRenderPassHandle());

    // simulate render frame
    // pass is not in list to render anymore
    update();
    scene.markAllRenderOncePassesAsRendered();
    EXPECT_TRUE(scene.getSortedRenderingPasses().empty());

    // unmap scene
    hideScene();
    unmapScene();
    update();
    scene.markAllRenderOncePassesAsRendered();

    // remap scene and expect that render once pass is in list to render again
    mapScene();
    showScene();
    update();
    scene.markAllRenderOncePassesAsRendered();
    EXPECT_EQ(pass, scene.getSortedRenderingPasses()[0].getRenderPassHandle());

    // simulate render frame
    // pass is not in list to render anymore
    update();
    scene.markAllRenderOncePassesAsRendered();
    EXPECT_TRUE(scene.getSortedRenderingPasses().empty());

    hideScene();
    unmapScene();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canHideSceneIfNotShownYet)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    update();

    // request show and hide within same frame
    rendererSceneUpdater->handleSceneShowRequest(getSceneId());
    rendererSceneUpdater->handleSceneHideRequest(getSceneId());
    expectInternalSceneStateEvents({ ERendererEventType::SceneShowFailed, ERendererEventType::SceneHidden });
    EXPECT_EQ(ESceneState::Mapped, sceneStateExecutor.getSceneState(getSceneId()));

    // show failed (was canceled) and scene is still in mapped state
    update();
    expectNoEvent();
    EXPECT_EQ(ESceneState::Mapped, sceneStateExecutor.getSceneState(getSceneId()));

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canNotUnmapSceneWhichWasRequestedToBeShown)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    update();

    rendererSceneUpdater->handleSceneShowRequest(getSceneId());

    rendererSceneUpdater->handleSceneUnmappingRequest(getSceneId());
    expectInternalSceneStateEvents({ ERendererEventType::SceneUnmapFailed });
    EXPECT_EQ(ESceneState::RenderRequested, sceneStateExecutor.getSceneState(getSceneId()));

    update();
    expectInternalSceneStateEvent(ERendererEventType::SceneShown);
    EXPECT_EQ(ESceneState::Rendered, sceneStateExecutor.getSceneState(getSceneId()));

    hideScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canUnsubscribeSceneIfSubscriptionRequested)
{
    createStagingScene();
    publishScene();
    requestSceneSubscription();
    // scene is now in subscription requested state, waiting for scene to arrive
    update();

    // request unsubscribe cancels the subscription and reports unsubscribed
    EXPECT_CALL(sceneEventSender, sendUnsubscribeScene(getSceneId()));
    rendererSceneUpdater->handleSceneUnsubscriptionRequest(getSceneId(), false);
    expectInternalSceneStateEvent(ERendererEventType::SceneUnsubscribed);
    EXPECT_EQ(ESceneState::Published, sceneStateExecutor.getSceneState(getSceneId()));

    update();
    expectNoEvent();
    EXPECT_EQ(ESceneState::Published, sceneStateExecutor.getSceneState(getSceneId()));
}

TEST_F(ARendererSceneUpdater, canUnsubscribeSceneIfSubscriptionPending)
{
    createStagingScene();
    publishScene();
    requestSceneSubscription();
    receiveScene();
    // scene is now in subscription pending state, scene arrived, waiting for initial flush
    update();

    // request unsubscribe cancels the subscription and reports unsubscribed
    EXPECT_CALL(sceneEventSender, sendUnsubscribeScene(getSceneId()));
    rendererSceneUpdater->handleSceneUnsubscriptionRequest(getSceneId(), false);
    expectInternalSceneStateEvent(ERendererEventType::SceneUnsubscribed);
    EXPECT_EQ(ESceneState::Published, sceneStateExecutor.getSceneState(getSceneId()));

    update();
    expectNoEvent();
    EXPECT_EQ(ESceneState::Published, sceneStateExecutor.getSceneState(getSceneId()));
}


///////////////////////////
// Offscreen buffer tests
///////////////////////////

TEST_F(ARendererSceneUpdater, canCreateOffscreenBuffer_WithColorBufferOnly)
{
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    expectOffscreenBufferUploaded(buffer, DeviceMock::FakeRenderTargetDeviceHandle, false, ERenderBufferType_InvalidBuffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 1u, 1u, 4u, false, ERenderBufferType_InvalidBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canCreateOffscreenBuffer_WithDepthStencilBuffers)
{
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    expectOffscreenBufferUploaded(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 1u, 1u, 4u, false, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canCreateDoubleBufferedOffscreenBuffer_WithColorBufferOnly)
{
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    expectOffscreenBufferUploaded(buffer, DeviceMock::FakeRenderTargetDeviceHandle, true, ERenderBufferType_InvalidBuffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 1u, 1u, 0u, true, ERenderBufferType_InvalidBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canCreateDoubleBufferedOffscreenBuffer_WithDepthStencilBuffers)
{
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    expectOffscreenBufferUploaded(buffer, DeviceMock::FakeRenderTargetDeviceHandle, true);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 1u, 1u, 0u, true, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canCreateDmaOffscreenBuffer)
{
    createDisplayAndExpectSuccess();

    constexpr OffscreenBufferHandle buffer(1u);
    constexpr DmaBufferFourccFormat fourccFormat{ 123u };
    constexpr DmaBufferUsageFlags usageFlags{ 456u };
    constexpr DmaBufferModifiers modifiers{ 789u };

    expectDmaOffscreenBufferUploaded(buffer, DeviceMock::FakeRenderTargetDeviceHandle, fourccFormat, usageFlags, modifiers);

    constexpr int resultFD = 111;
    constexpr uint32_t resultStride = 222u;
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getDmaOffscreenBufferFD(buffer)).WillOnce(Return(resultFD));
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getDmaOffscreenBufferStride(buffer)).WillOnce(Return(resultStride));
    EXPECT_TRUE(rendererSceneUpdater->handleDmaBufferCreateRequest(buffer, 1u, 1u, fourccFormat, usageFlags, modifiers));

    RendererEventVector rendererEvents;
    RendererEventVector sceneEvents;
    rendererEventCollector.appendAndConsumePendingEvents(rendererEvents, sceneEvents);
    ASSERT_EQ(rendererEvents.size(), 1u);
    EXPECT_EQ(sceneEvents.size(), 0u);

    const auto& event = rendererEvents.front();
    EXPECT_EQ(ERendererEventType::OffscreenBufferCreated, event.eventType);
    EXPECT_EQ(Display, event.displayHandle);
    EXPECT_EQ(buffer, event.offscreenBuffer);
    EXPECT_EQ(resultFD, event.dmaBufferFD);
    EXPECT_EQ(resultStride, event.dmaBufferStride);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, failsToCreateOffscreenBufferOnUnknownDisplay)
{
    const OffscreenBufferHandle buffer(1u);
    EXPECT_FALSE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreateFailed);
}

TEST_F(ARendererSceneUpdater, failsToCreateDmaOffscreenBufferOnUnknownDisplay)
{
    const OffscreenBufferHandle buffer(1u);
    constexpr DmaBufferFourccFormat fourccFormat{ 123u };
    constexpr DmaBufferUsageFlags usageFlags{ 456u };
    constexpr DmaBufferModifiers modifiers{ 789u };
    EXPECT_FALSE(rendererSceneUpdater->handleDmaBufferCreateRequest(buffer, 1u, 1u, fourccFormat, usageFlags, modifiers));
    expectEvent(ERendererEventType::OffscreenBufferCreateFailed);
}

TEST_F(ARendererSceneUpdater, failsToCreateOffscreenBufferWithSameID)
{
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    expectOffscreenBufferUploaded(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);
    EXPECT_FALSE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreateFailed);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, failsToCreateDmaOffscreenBufferWithSameID)
{
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    constexpr DmaBufferFourccFormat fourccFormat{ 123u };
    constexpr DmaBufferUsageFlags usageFlags{ 456u };
    constexpr DmaBufferModifiers modifiers{ 789u };

    expectOffscreenBufferUploaded(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);
    EXPECT_FALSE(rendererSceneUpdater->handleDmaBufferCreateRequest(buffer, 1u, 1u, fourccFormat, usageFlags, modifiers));
    expectEvent(ERendererEventType::OffscreenBufferCreateFailed);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canDestroyOffscreenBuffer)
{
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    expectOffscreenBufferUploaded(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);

    expectOffscreenBufferDeleted(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferDestroyRequest(buffer));

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canDestroyDoubleBufferedOffscreenBuffer)
{
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    expectOffscreenBufferUploaded(buffer, DeviceMock::FakeRenderTargetDeviceHandle, true);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 1u, 1u, 0u, true, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);

    expectOffscreenBufferDeleted(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferDestroyRequest(buffer));

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, failsToDestroyOffscreenBufferOnUnknownDisplay)
{
    const OffscreenBufferHandle buffer(1u);
    EXPECT_FALSE(rendererSceneUpdater->handleBufferDestroyRequest(buffer));
}

TEST_F(ARendererSceneUpdater, failsToDestroyUnknownOffscreenBuffer)
{
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getOffscreenBufferDeviceHandle(buffer)).WillOnce(Return(DeviceResourceHandle::Invalid()));
    EXPECT_FALSE(rendererSceneUpdater->handleBufferDestroyRequest(buffer));

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canAssignSceneToOffscreenBuffer)
{
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    expectOffscreenBufferUploaded(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);

    createPublishAndSubscribeScene();
    mapScene(0u);
    EXPECT_TRUE(assignSceneToDisplayBuffer(0, buffer, 11));

    unmapScene(0u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canAssignSceneToFramebuffer_MultipleTimes)
{
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    expectOffscreenBufferUploaded(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);

    createPublishAndSubscribeScene();
    mapScene(0u);
    // Scene can be always assigned to framebuffer
    EXPECT_TRUE(assignSceneToDisplayBuffer(0));
    EXPECT_TRUE(assignSceneToDisplayBuffer(0));
    EXPECT_TRUE(assignSceneToDisplayBuffer(0));

    unmapScene(0u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, failsToAssignSceneToDisplayBufferOnInvalidDisplay)
{
    createPublishAndSubscribeScene();
    EXPECT_FALSE(assignSceneToDisplayBuffer(0));
}

TEST_F(ARendererSceneUpdater, failsToAssignSceneToNonExistingDisplayBuffer)
{
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);

    createPublishAndSubscribeScene();
    mapScene(0u);
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getOffscreenBufferDeviceHandle(buffer)).WillOnce(Return(DeviceResourceHandle::Invalid()));
    EXPECT_FALSE(assignSceneToDisplayBuffer(0, buffer));

    unmapScene(0u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, confidence_failsToDestroyOffscreenBufferIfScenesAreAssignedToIt_DestroysAfterSceneIsAssignedToFramebuffer)
{
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    expectOffscreenBufferUploaded(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);

    createPublishAndSubscribeScene();
    mapScene(0u);
    EXPECT_TRUE(assignSceneToDisplayBuffer(0, buffer));

    EXPECT_FALSE(rendererSceneUpdater->handleBufferDestroyRequest(buffer));

    EXPECT_TRUE(assignSceneToDisplayBuffer(0));

    expectOffscreenBufferDeleted(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferDestroyRequest(buffer));

    unmapScene(0u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, setsClearFlagsForOB)
{
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    expectOffscreenBufferUploaded(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);

    EXPECT_CALL(renderer, setClearFlags(DeviceMock::FakeRenderTargetDeviceHandle, EClearFlags_Color));
    rendererSceneUpdater->handleSetClearFlags(buffer, EClearFlags_Color);

    expectOffscreenBufferDeleted(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferDestroyRequest(buffer));

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, setsClearFlagsForFBIfNoOBSpecified)
{
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    expectOffscreenBufferUploaded(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);

    EXPECT_CALL(renderer, setClearFlags(renderer.getDisplayController().getDisplayBuffer(), EClearFlags_Color));
    rendererSceneUpdater->handleSetClearFlags(OffscreenBufferHandle::Invalid(), EClearFlags_Color);

    expectOffscreenBufferDeleted(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferDestroyRequest(buffer));

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, doesNotSetClearFlagsIfOBSpecifiedButNotFound)
{
    createDisplayAndExpectSuccess();

    constexpr OffscreenBufferHandle invalidOB{ 1234u };
    EXPECT_CALL(renderer, setClearFlags(_, _)).Times(0u);
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getOffscreenBufferDeviceHandle(invalidOB)).WillOnce(Return(DeviceResourceHandle::Invalid()));
    rendererSceneUpdater->handleSetClearFlags(invalidOB, EClearFlags_Color);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, doesNotSetClearFlagsIfDisplayInvalid)
{
    EXPECT_CALL(renderer, setClearFlags(_, _)).Times(0u);
    rendererSceneUpdater->handleSetClearFlags({}, EClearFlags_Color);
}

TEST_F(ARendererSceneUpdater, setsClearColorForOB)
{
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    expectOffscreenBufferUploaded(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);


    EXPECT_CALL(renderer, setClearColor(DeviceMock::FakeRenderTargetDeviceHandle, Vector4{ 1, 2, 3, 4 }));
    rendererSceneUpdater->handleSetClearColor(buffer, { 1, 2, 3, 4 });

    expectOffscreenBufferDeleted(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferDestroyRequest(buffer));

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, setsClearColorForFBIfNoOBSpecified)
{
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    expectOffscreenBufferUploaded(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);

    EXPECT_CALL(renderer, setClearColor(renderer.getDisplayController().getDisplayBuffer(), Vector4{ 1, 2, 3, 4 }));
    rendererSceneUpdater->handleSetClearColor(OffscreenBufferHandle::Invalid(), { 1, 2, 3, 4 });

    expectOffscreenBufferDeleted(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferDestroyRequest(buffer));

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, doesNotSetClearColorIfOBSpecifiedButNotFound)
{
    createDisplayAndExpectSuccess();

    constexpr OffscreenBufferHandle invalidOB{ 1234u };
    EXPECT_CALL(renderer, setClearColor(_, _)).Times(0u);
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getOffscreenBufferDeviceHandle(invalidOB)).WillOnce(Return(DeviceResourceHandle::Invalid()));
    rendererSceneUpdater->handleSetClearColor(invalidOB, { 1, 2, 3, 4 });

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, doesNotSetClearColorIfDisplayInvalid)
{
    EXPECT_CALL(renderer, setClearColor(_, _)).Times(0u);
    rendererSceneUpdater->handleSetClearColor({}, { 1, 2, 3, 4 });
}

TEST_F(ARendererSceneUpdater, resizesExternallyOwnedDisplayWindow)
{
    createDisplayAndExpectSuccess();

    EXPECT_CALL(renderer.m_platform.renderBackendMock.windowMock, setExternallyOwnedWindowSize(123u, 456u)).WillOnce(Return(true));
    rendererSceneUpdater->handleSetExternallyOwnedWindowSize(123u, 456u);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, doesNotResizeExternallyOwnedDisplayWindowIfDisplayInvalid)
{
    EXPECT_CALL(renderer.m_platform.renderBackendMock.windowMock, setExternallyOwnedWindowSize(_, _)).Times(0);
    rendererSceneUpdater->handleSetExternallyOwnedWindowSize(123u, 456u);
}

TEST_F(ARendererSceneUpdater, readPixelsFromDisplayFramebuffer)
{
    createDisplayAndExpectSuccess();

    const UInt32 x = 1u;
    const UInt32 y = 2u;
    const UInt32 width = 3u;
    const UInt32 height = 4u;
    const Bool fullScreen = false;
    const Bool sendViaDLT = false;
    const String& filename = "";

    readPixels({}, x, y, width, height, fullScreen, sendViaDLT, filename);
    expectDisplayControllerReadPixels(DisplayControllerMock::FakeFrameBufferHandle, x, y, width, height);
    doRenderLoop();
    rendererSceneUpdater->processScreenshotResults();
    expectReadPixelsEvents({ {OffscreenBufferHandle::Invalid(), true} });

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, readPixelsFromOffscreenbuffer)
{
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    expectOffscreenBufferUploaded(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 10u, 10u, 0u, false, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);

    const UInt32 x = 1u;
    const UInt32 y = 2u;
    const UInt32 width = 3u;
    const UInt32 height = 4u;
    const Bool fullScreen = false;
    const Bool sendViaDLT = false;
    const String& filename = "";

    readPixels(buffer, x, y, width, height, fullScreen, sendViaDLT, filename);
    expectDisplayControllerReadPixels(DeviceMock::FakeRenderTargetDeviceHandle, x, y, width, height);
    doRenderLoop();
    rendererSceneUpdater->processScreenshotResults();
    expectReadPixelsEvents({ {buffer, true} });

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, createsReadPixelsFailedEventIfInvalidOffscreenBuffer)
{
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);

    const UInt32 x = 1u;
    const UInt32 y = 2u;
    const UInt32 width = 3u;
    const UInt32 height = 4u;
    const Bool fullScreen = false;
    const Bool sendViaDLT = false;
    const String& filename = "";

    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getOffscreenBufferDeviceHandle(buffer)).WillOnce(Return(DeviceResourceHandle::Invalid()));
    readPixels(buffer, x, y, width, height, fullScreen, sendViaDLT, filename);

    EXPECT_CALL(*renderer.m_displayController, readPixels(_, _, _, _, _, _)).Times(0);
    doRenderLoop();
    expectReadPixelsEvents({ {buffer, false} });

    rendererSceneUpdater->processScreenshotResults();
    expectNoEvent();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, createsReadPixelsFailedEventIfRectangleIsOutOfDisplayBounds)
{
    const auto displayWidth = WindowMock::FakeWidth;
    const auto displayHeight = WindowMock::FakeHeight;
    DisplayConfig dispConfig;
    dispConfig.setDesiredWindowWidth(displayWidth);
    dispConfig.setDesiredWindowHeight(displayHeight);
    createDisplayAndExpectSuccess(dispConfig);

    const Bool fullScreen = false;
    const Bool sendViaDLT = false;
    const String& filename = "";


    readPixels({}, displayWidth, 0u, 1u, 1u, fullScreen, sendViaDLT, filename);
    readPixels({}, 0u, displayHeight, 1u, 1u, fullScreen, sendViaDLT, filename);
    readPixels({}, 0u, 0u, displayWidth + 1u, 1u, fullScreen, sendViaDLT, filename);
    readPixels({}, 0u, 0u, 0u, displayHeight + 1u, fullScreen, sendViaDLT, filename);

    EXPECT_CALL(*renderer.m_displayController, readPixels(_, _, _, _, _, _)).Times(0);
    doRenderLoop();

    expectReadPixelsEvents({ { OffscreenBufferHandle::Invalid(), false },
                            { OffscreenBufferHandle::Invalid(), false },
                            { OffscreenBufferHandle::Invalid(), false },
                            { OffscreenBufferHandle::Invalid(), false } });

    rendererSceneUpdater->processScreenshotResults();
    expectNoEvent();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, createsReadPixelsFailedEventIfRectangleIsOutOfOffscreenBufferBounds)
{
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    expectOffscreenBufferUploaded(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 10u, 10u, 0u, false, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);

    const Bool fullScreen = false;
    const Bool sendViaDLT = false;
    const String& filename = "";

    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getOffscreenBufferDeviceHandle(buffer)).Times(4).WillRepeatedly(Return(DeviceResourceHandle::Invalid()));
    readPixels(buffer, 10, 0u, 1u, 1u, fullScreen, sendViaDLT, filename);
    readPixels(buffer, 0u, 10, 1u, 1u, fullScreen, sendViaDLT, filename);
    readPixels(buffer, 0u, 0u, 11u, 1u, fullScreen, sendViaDLT, filename);
    readPixels(buffer, 0u, 0u, 0u, 11u, fullScreen, sendViaDLT, filename);

    EXPECT_CALL(*renderer.m_displayController, readPixels(_, _, _, _, _, _)).Times(0);
    doRenderLoop();

    expectReadPixelsEvents({ { buffer, false },
                            { buffer, false },
                            { buffer, false },
                            { buffer, false } });

    rendererSceneUpdater->processScreenshotResults();
    expectNoEvent();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, readPixelsFromDisplayAndSaveToFileWithoutGeneratingEvent)
{
    createDisplayAndExpectSuccess();

    const UInt32 x = 1u;
    const UInt32 y = 2u;
    const UInt32 width = 3u;
    const UInt32 height = 4u;
    const Bool fullScreen = false;
    const Bool sendViaDLT = false;
    const String& filename = "testScreenshot";

    readPixels({}, x, y, width, height, fullScreen, sendViaDLT, filename);
    expectDisplayControllerReadPixels(DisplayControllerMock::FakeFrameBufferHandle, x, y, width, height);
    doRenderLoop();
    expectNoEvent();

    rendererSceneUpdater->processScreenshotResults();
    expectNoEvent();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, readPixelsFromDisplayFullscreen)
{
    const auto displayWidth = WindowMock::FakeWidth;
    const auto displayHeight = WindowMock::FakeHeight;
    DisplayConfig dispConfig;
    dispConfig.setDesiredWindowWidth(displayWidth);
    dispConfig.setDesiredWindowHeight(displayHeight);
    createDisplayAndExpectSuccess(dispConfig);

    const UInt32 x = 1u;
    const UInt32 y = 2u;
    const UInt32 width = 3u;
    const UInt32 height = 4u;
    const Bool fullScreen = true;
    const Bool sendViaDLT = false;
    const String& filename = "";

    readPixels({}, x, y, width, height, fullScreen, sendViaDLT, filename);
    expectDisplayControllerReadPixels(DisplayControllerMock::FakeFrameBufferHandle, 0, 0, WindowMock::FakeWidth, WindowMock::FakeHeight);
    doRenderLoop();

    rendererSceneUpdater->processScreenshotResults();
    expectReadPixelsEvents({ {OffscreenBufferHandle::Invalid(), true} });

    destroyDisplay();
}

///////////////////////////
// Stream buffer tests
///////////////////////////

TEST_F(ARendererSceneUpdater, canCreateStreamBuffer)
{
    createDisplayAndExpectSuccess();

    constexpr StreamBufferHandle buffer{ 1u };
    constexpr WaylandIviSurfaceId source{ 2u };
    expectStreamBufferUploaded(buffer, source);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, source));

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, failsToCreateStreamBufferOnUnknownDisplay)
{
    constexpr StreamBufferHandle buffer{ 1u };
    constexpr WaylandIviSurfaceId source{ 2u };
    EXPECT_FALSE(rendererSceneUpdater->handleBufferCreateRequest(buffer, source));
}

TEST_F(ARendererSceneUpdater, canDestroyStreamBuffer)
{
    createDisplayAndExpectSuccess();

    constexpr StreamBufferHandle buffer{ 1u };
    constexpr WaylandIviSurfaceId source{ 2u };
    expectStreamBufferUploaded(buffer, source);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, source));

    expectStreamBufferDeleted(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferDestroyRequest(buffer));

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, failsToDestroyStreamBufferOnUnknownDisplay)
{
    constexpr StreamBufferHandle buffer{ 1u };
    EXPECT_FALSE(rendererSceneUpdater->handleBufferDestroyRequest(buffer));
}

///////////////////////////
// External buffer tests
///////////////////////////

TEST_F(ARendererSceneUpdater, canCreateExternalBuffer)
{
    createDisplayAndExpectSuccess();

    constexpr ExternalBufferHandle buffer{ 1u };
    expectExternalBufferUploaded(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleExternalBufferCreateRequest(buffer));
    expectEvent(ERendererEventType::ExternalBufferCreated);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, FailsToCreateExternalBufferIfResourceManagerFailsToUpload)
{
    createDisplayAndExpectSuccess();

    constexpr ExternalBufferHandle buffer{ 1u };
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, uploadExternalBuffer(buffer));
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getExternalBufferDeviceHandle(buffer)).Times(1u).WillOnce(Return(DeviceResourceHandle::Invalid()));
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getExternalBufferGlId(buffer)).Times(0);

    EXPECT_FALSE(rendererSceneUpdater->handleExternalBufferCreateRequest(buffer));
    expectEvent(ERendererEventType::ExternalBufferCreateFailed);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, failsToCreateExternalBufferOnUnknownDisplay)
{
    constexpr ExternalBufferHandle buffer{ 1u };
    EXPECT_FALSE(rendererSceneUpdater->handleExternalBufferCreateRequest(buffer));
    expectEvent(ERendererEventType::ExternalBufferCreateFailed);
}

TEST_F(ARendererSceneUpdater, canDestroyExternalBuffer)
{
    createDisplayAndExpectSuccess();

    constexpr ExternalBufferHandle buffer{ 1u };
    expectExternalBufferUploaded(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleExternalBufferCreateRequest(buffer));
    expectEvent(ERendererEventType::ExternalBufferCreated);

    expectExternalBufferDeleted(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleExternalBufferDestroyRequest(buffer));
    expectEvent(ERendererEventType::ExternalBufferDestroyed);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, failsToDestroyExternalBufferOnUnknownDisplay)
{
    constexpr ExternalBufferHandle buffer{ 1u };
    EXPECT_FALSE(rendererSceneUpdater->handleExternalBufferDestroyRequest(buffer));
    expectEvent(ERendererEventType::ExternalBufferDestroyFailed);
}

///////////////////////////
// Data linking tests
///////////////////////////

TEST_F(ARendererSceneUpdater, updatesDataLinks)
{
    createDisplayAndExpectSuccess();

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();

    mapScene(0u);
    mapScene(1u);

    showScene(0u);
    showScene(1u);

    DataInstanceHandle consumerDataRef;
    const float providedValue(333.f);
    createDataSlotsAndLinkThem(consumerDataRef, providedValue);

    update();

    IScene& scene2 = rendererScenes.getScene(getSceneId(1u));
    const float consumedValue = scene2.getDataSingleFloat(consumerDataRef, DataFieldHandle(0u));
    EXPECT_FLOAT_EQ(providedValue, consumedValue);

    hideScene(0u);
    hideScene(1u);
    unmapScene(0u);
    unmapScene(1u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, updatesDataLinksAlsoIfProviderSceneNotShown)
{
    createDisplayAndExpectSuccess();

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();

    // only consumer scene is mapped and shown
    mapScene(1u);
    showScene(1u);

    DataInstanceHandle consumerDataRef;
    const float providedValue(333.f);
    createDataSlotsAndLinkThem(consumerDataRef, providedValue);

    update();

    IScene& scene2 = rendererScenes.getScene(getSceneId(1u));
    const float consumedValue = scene2.getDataSingleFloat(consumerDataRef, DataFieldHandle(0u));
    EXPECT_FLOAT_EQ(providedValue, consumedValue);

    hideScene(1u);
    unmapScene(1u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, doesNotUpdatesDataLinksIfConsumerSceneNotShown)
{
    createDisplayAndExpectSuccess();

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();

    mapScene(0u);
    mapScene(1u);

    showScene(0u);
    showScene(1u);

    DataInstanceHandle consumerDataRef;
    const float providedValue(333.f);
    createDataSlotsAndLinkThem(consumerDataRef, providedValue);

    hideScene(1u);
    update();

    IScene& scene2 = rendererScenes.getScene(getSceneId(1u));
    const float consumedValue = scene2.getDataSingleFloat(consumerDataRef, DataFieldHandle(0u));
    EXPECT_FLOAT_EQ(0.f, consumedValue);

    hideScene(0u);
    unmapScene(0u);
    unmapScene(1u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, removesTextureLinkWhenProviderSceneUnmapped)
{
    createDisplayAndExpectSuccess();

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();

    mapScene(0u);
    mapScene(1u);

    showScene(0u);
    showScene(1u);

    expectResourcesReferencedAndProvided({ MockResourceHash::TextureHash });
    createTextureSlotsAndLinkThem();
    update();

    hideScene(0u);
    unmapScene(0u);

    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getSceneLinks().hasAnyLinksToConsumer(getSceneId(0u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getSceneLinks().hasAnyLinksToProvider(getSceneId(0u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getSceneLinks().hasAnyLinksToConsumer(getSceneId(1u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getSceneLinks().hasAnyLinksToProvider(getSceneId(1u)));

    hideScene(1u);
    unmapScene(1u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, removesTextureLinkWhenConsumerSceneUnmapped)
{
    createDisplayAndExpectSuccess();

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();

    mapScene(0u);
    mapScene(1u);

    showScene(0u);
    showScene(1u);

    expectResourcesReferencedAndProvided({ MockResourceHash::TextureHash });
    createTextureSlotsAndLinkThem();
    update();

    hideScene(1u);
    unmapScene(1u);

    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getSceneLinks().hasAnyLinksToConsumer(getSceneId(0u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getSceneLinks().hasAnyLinksToProvider(getSceneId(0u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getSceneLinks().hasAnyLinksToConsumer(getSceneId(1u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getSceneLinks().hasAnyLinksToProvider(getSceneId(1u)));

    hideScene(0u);
    unmapScene(0u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, failsToCreateTextureLinkIfProviderSceneNotMapped)
{
    createDisplayAndExpectSuccess();

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();

    mapScene(1u);
    showScene(1u);

    createTextureSlotsAndLinkThem(nullptr, 0u, 1u, false);
    update();

    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getSceneLinks().hasAnyLinksToConsumer(getSceneId(0u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getSceneLinks().hasAnyLinksToProvider(getSceneId(0u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getSceneLinks().hasAnyLinksToConsumer(getSceneId(1u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getSceneLinks().hasAnyLinksToProvider(getSceneId(1u)));

    hideScene(1u);
    unmapScene(1u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, failsToCreateTextureLinkIfBothProviderAndConsumerSceneNotMapped)
{
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();

    createTextureSlotsAndLinkThem(nullptr, 0u, 1u, false);
    update();

    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getSceneLinks().hasAnyLinksToConsumer(getSceneId(0u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getSceneLinks().hasAnyLinksToProvider(getSceneId(0u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getSceneLinks().hasAnyLinksToConsumer(getSceneId(1u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getSceneLinks().hasAnyLinksToProvider(getSceneId(1u)));
}

TEST_F(ARendererSceneUpdater, triggersRemovalOfBufferLinkWhenBufferDestroyed_OB)
{
    createDisplayAndExpectSuccess();

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();

    mapScene(0u);
    mapScene(1u);

    showScene(0u);
    showScene(1u);

    const DataSlotId consumerId1 = createTextureConsumer(0u);
    const DataSlotId consumerId2 = createTextureConsumer(1u);

    const OffscreenBufferHandle buffer(1u);
    expectOffscreenBufferUploaded(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);

    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToConsumer(buffer));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToProvider(getSceneId(0u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToProvider(getSceneId(1u)));

    createBufferLink(buffer, getSceneId(0u), consumerId1);
    createBufferLink(buffer, getSceneId(1u), consumerId2);

    EXPECT_TRUE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToConsumer(buffer));
    EXPECT_TRUE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToProvider(getSceneId(0u)));
    EXPECT_TRUE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToProvider(getSceneId(1u)));

    expectOffscreenBufferDeleted(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferDestroyRequest(buffer));

    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToConsumer(buffer));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToProvider(getSceneId(0u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToProvider(getSceneId(1u)));

    hideScene(0u);
    hideScene(1u);
    unmapScene(0u);
    unmapScene(1u);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, triggersRemovalOfBufferLinkWhenBufferDestroyed_SB)
{
    createDisplayAndExpectSuccess();

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();

    mapScene(0u);
    mapScene(1u);

    showScene(0u);
    showScene(1u);

    const DataSlotId consumerId1 = createTextureConsumer(0u);
    const DataSlotId consumerId2 = createTextureConsumer(1u);

    constexpr StreamBufferHandle buffer{ 1u };
    constexpr WaylandIviSurfaceId source{ 2u };
    expectStreamBufferUploaded(buffer, source);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, source));

    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getStreamBufferLinks().hasAnyLinksToConsumer(buffer));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getStreamBufferLinks().hasAnyLinksToProvider(getSceneId(0u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getStreamBufferLinks().hasAnyLinksToProvider(getSceneId(1u)));

    createBufferLink(buffer, getSceneId(0u), consumerId1);
    createBufferLink(buffer, getSceneId(1u), consumerId2);

    EXPECT_TRUE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getStreamBufferLinks().hasAnyLinksToConsumer(buffer));
    EXPECT_TRUE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getStreamBufferLinks().hasAnyLinksToProvider(getSceneId(0u)));
    EXPECT_TRUE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getStreamBufferLinks().hasAnyLinksToProvider(getSceneId(1u)));

    expectStreamBufferDeleted(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferDestroyRequest(buffer));

    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getStreamBufferLinks().hasAnyLinksToConsumer(buffer));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getStreamBufferLinks().hasAnyLinksToProvider(getSceneId(0u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getStreamBufferLinks().hasAnyLinksToProvider(getSceneId(1u)));

    hideScene(0u);
    hideScene(1u);
    unmapScene(0u);
    unmapScene(1u);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, triggersRemovalOfBufferLinkWhenBufferDestroyed_EB)
{
    createDisplayAndExpectSuccess();

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();

    mapScene(0u);
    mapScene(1u);

    showScene(0u);
    showScene(1u);

    const DataSlotId consumerId1 = createTextureConsumer(0u, TextureSampler::ContentType::ExternalTexture);
    const DataSlotId consumerId2 = createTextureConsumer(1u, TextureSampler::ContentType::ExternalTexture);

    constexpr ExternalBufferHandle buffer{ 1u };
    expectExternalBufferUploaded(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleExternalBufferCreateRequest(buffer));
    expectEvent(ERendererEventType::ExternalBufferCreated);

    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getExternalBufferLinks().hasAnyLinksToConsumer(buffer));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getExternalBufferLinks().hasAnyLinksToProvider(getSceneId(0u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getExternalBufferLinks().hasAnyLinksToProvider(getSceneId(1u)));

    createBufferLink(buffer, getSceneId(0u), consumerId1);
    createBufferLink(buffer, getSceneId(1u), consumerId2);

    EXPECT_TRUE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getExternalBufferLinks().hasAnyLinksToConsumer(buffer));
    EXPECT_TRUE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getExternalBufferLinks().hasAnyLinksToProvider(getSceneId(0u)));
    EXPECT_TRUE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getExternalBufferLinks().hasAnyLinksToProvider(getSceneId(1u)));

    expectExternalBufferDeleted(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleExternalBufferDestroyRequest(buffer));
    expectEvent(ERendererEventType::ExternalBufferDestroyed);

    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getExternalBufferLinks().hasAnyLinksToConsumer(buffer));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getExternalBufferLinks().hasAnyLinksToProvider(getSceneId(0u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getExternalBufferLinks().hasAnyLinksToProvider(getSceneId(1u)));

    hideScene(0u);
    hideScene(1u);
    unmapScene(0u);
    unmapScene(1u);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, triggersRemovalOfBufferLinkWhenConsumerSceneUnmapped_keepsOtherConsumerLinked_OB)
{
    createDisplayAndExpectSuccess();

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();

    mapScene(0u);
    mapScene(1u);

    showScene(0u);
    showScene(1u);

    const DataSlotId consumerId1 = createTextureConsumer(0u);
    const DataSlotId consumerId2 = createTextureConsumer(1u);

    const OffscreenBufferHandle buffer(1u);
    expectOffscreenBufferUploaded(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);

    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToConsumer(buffer));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToProvider(getSceneId(0u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToProvider(getSceneId(1u)));

    createBufferLink(buffer, getSceneId(0u), consumerId1);
    createBufferLink(buffer, getSceneId(1u), consumerId2);

    EXPECT_TRUE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToConsumer(buffer));
    EXPECT_TRUE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToProvider(getSceneId(0u)));
    EXPECT_TRUE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToProvider(getSceneId(1u)));

    hideScene(0u);
    unmapScene(0u);

    EXPECT_TRUE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToConsumer(buffer));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToProvider(getSceneId(0u)));
    EXPECT_TRUE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToProvider(getSceneId(1u)));

    hideScene(1u);
    unmapScene(1u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, triggersRemovalOfBufferLinkWhenConsumerSceneUnmapped_keepsOtherConsumerLinked_SB)
{
    createDisplayAndExpectSuccess();

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();

    mapScene(0u);
    mapScene(1u);

    showScene(0u);
    showScene(1u);

    const DataSlotId consumerId1 = createTextureConsumer(0u);
    const DataSlotId consumerId2 = createTextureConsumer(1u);

    constexpr StreamBufferHandle buffer{ 1u };
    constexpr WaylandIviSurfaceId source{ 2u };
    expectStreamBufferUploaded(buffer, source);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, source));

    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getStreamBufferLinks().hasAnyLinksToConsumer(buffer));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getStreamBufferLinks().hasAnyLinksToProvider(getSceneId(0u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getStreamBufferLinks().hasAnyLinksToProvider(getSceneId(1u)));

    createBufferLink(buffer, getSceneId(0u), consumerId1);
    createBufferLink(buffer, getSceneId(1u), consumerId2);

    EXPECT_TRUE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getStreamBufferLinks().hasAnyLinksToConsumer(buffer));
    EXPECT_TRUE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getStreamBufferLinks().hasAnyLinksToProvider(getSceneId(0u)));
    EXPECT_TRUE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getStreamBufferLinks().hasAnyLinksToProvider(getSceneId(1u)));

    hideScene(0u);
    unmapScene(0u);

    EXPECT_TRUE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getStreamBufferLinks().hasAnyLinksToConsumer(buffer));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getStreamBufferLinks().hasAnyLinksToProvider(getSceneId(0u)));
    EXPECT_TRUE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getStreamBufferLinks().hasAnyLinksToProvider(getSceneId(1u)));

    hideScene(1u);
    unmapScene(1u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, triggersRemovalOfBufferLinkWhenConsumerSceneUnmapped_keepsOtherConsumerLinked_EB)
{
    createDisplayAndExpectSuccess();

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();

    mapScene(0u);
    mapScene(1u);

    showScene(0u);
    showScene(1u);

    const DataSlotId consumerId1 = createTextureConsumer(0u, TextureSampler::ContentType::ExternalTexture);
    const DataSlotId consumerId2 = createTextureConsumer(1u, TextureSampler::ContentType::ExternalTexture);

    constexpr ExternalBufferHandle buffer{ 1u };
    expectExternalBufferUploaded(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleExternalBufferCreateRequest(buffer));
    expectEvent(ERendererEventType::ExternalBufferCreated);

    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getExternalBufferLinks().hasAnyLinksToConsumer(buffer));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getExternalBufferLinks().hasAnyLinksToProvider(getSceneId(0u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getExternalBufferLinks().hasAnyLinksToProvider(getSceneId(1u)));

    createBufferLink(buffer, getSceneId(0u), consumerId1);
    createBufferLink(buffer, getSceneId(1u), consumerId2);

    EXPECT_TRUE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getExternalBufferLinks().hasAnyLinksToConsumer(buffer));
    EXPECT_TRUE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getExternalBufferLinks().hasAnyLinksToProvider(getSceneId(0u)));
    EXPECT_TRUE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getExternalBufferLinks().hasAnyLinksToProvider(getSceneId(1u)));

    hideScene(0u);
    unmapScene(0u);

    EXPECT_TRUE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getExternalBufferLinks().hasAnyLinksToConsumer(buffer));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getExternalBufferLinks().hasAnyLinksToProvider(getSceneId(0u)));
    EXPECT_TRUE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getExternalBufferLinks().hasAnyLinksToProvider(getSceneId(1u)));

    hideScene(1u);
    unmapScene(1u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, failsToCreateBufferLinkIfConsumerSceneNotOnDisplay_OB)
{
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    expectOffscreenBufferUploaded(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);

    createBufferLink(buffer, SceneId{ 666u }, DataSlotId{ 1u }, true);

    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToConsumer(buffer));

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, failsToCreateBufferLinkIfConsumerSceneNotOnDisplay_SB)
{
    createDisplayAndExpectSuccess();

    constexpr StreamBufferHandle buffer{ 1u };
    constexpr WaylandIviSurfaceId source{ 2u };
    expectStreamBufferUploaded(buffer, source);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, source));

    createBufferLink(buffer, SceneId{ 666u }, DataSlotId{ 1u }, true);

    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getStreamBufferLinks().hasAnyLinksToConsumer(buffer));

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, failsToCreateBufferLinkIfConsumerSceneNotOnDisplay_EB)
{
    createDisplayAndExpectSuccess();

    constexpr ExternalBufferHandle buffer{ 1u };
    expectExternalBufferUploaded(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleExternalBufferCreateRequest(buffer));
    expectEvent(ERendererEventType::ExternalBufferCreated);

    createBufferLink(buffer, SceneId{ 666u }, DataSlotId{ 1u }, true);

    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getExternalBufferLinks().hasAnyLinksToConsumer(buffer));

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, failsToCreateBufferLinkIfDisplayInvalid)
{
    createBufferLink(OffscreenBufferHandle{ 3u }, SceneId{ 666u }, DataSlotId{ 1u }, true);
    createBufferLink(StreamBufferHandle{ 3u }, SceneId{ 666u }, DataSlotId{ 1u }, true);
}

TEST_F(ARendererSceneUpdater, failsToCreateBufferLinkIfBufferUnknown_OB)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    constexpr OffscreenBufferHandle buffer{ 123u };
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getOffscreenBufferDeviceHandle(buffer)).WillOnce(Return(DeviceResourceHandle::Invalid()));
    createBufferLink(buffer, getSceneId(), DataSlotId{ 1u }, true);

    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToConsumer(buffer));

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, failsToCreateBufferLinkIfBufferUnknown_SB)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    constexpr StreamBufferHandle buffer{ 123u };
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getStreamBufferDeviceHandle(buffer)).WillOnce(Return(DeviceResourceHandle::Invalid()));
    createBufferLink(buffer, getSceneId(), DataSlotId{ 1u }, true);

    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getStreamBufferLinks().hasAnyLinksToConsumer(buffer));

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, failsToCreateBufferLinkIfBufferUnknown_EB)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    constexpr ExternalBufferHandle buffer{ 123u };
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getExternalBufferDeviceHandle(buffer)).WillOnce(Return(DeviceResourceHandle::Invalid()));
    createBufferLink(buffer, getSceneId(), DataSlotId{ 1u }, true);

    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getExternalBufferLinks().hasAnyLinksToConsumer(buffer));

    unmapScene();
    destroyDisplay();
}

/////////////////////////////////////////////
// Other tests
/////////////////////////////////////////////

TEST_F(ARendererSceneUpdater, updateSceneStreamTexturesDirtinessGeneratesEventsForNewAndObsoleteStreamSurfaces)
{
    createDisplayAndExpectSuccess();

    const WaylandIviSurfaceId newStreamId(7563u);
    const WaylandIviSurfaceId obsoleteStreamId(8883u);
    const WaylandIviSurfaceIdVector newStreams{ newStreamId };
    const WaylandIviSurfaceIdVector obsoleteStreams{ obsoleteStreamId };

    expectNoEvent();

    EXPECT_CALL(renderer.m_embeddedCompositingManager, dispatchStateChangesOfSources(_, _, _)).WillOnce(DoAll(SetArgReferee<1>(newStreams), SetArgReferee<2>(obsoleteStreams)));

    update();

    RendererEventVector resultEvents;
    RendererEventVector dummy;
    rendererEventCollector.appendAndConsumePendingEvents(dummy, resultEvents);
    ASSERT_EQ(2u, resultEvents.size());
    EXPECT_EQ(ERendererEventType::StreamSurfaceAvailable, resultEvents[0].eventType);
    EXPECT_EQ(newStreamId, resultEvents[0].streamSourceId);
    EXPECT_EQ(ERendererEventType::StreamSurfaceUnavailable, resultEvents[1].eventType);
    EXPECT_EQ(obsoleteStreamId, resultEvents[1].streamSourceId);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, willShowSceneEvenIfFlushesPending)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();

    performFlushWithCreateNodeAction();
    mapScene();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    createRenderable();
    setRenderableResources();

    // simulate effect not uploaded yet
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Provided);
    update();

    // scene shown even if flush pending
    showScene();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    hideScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, appliesBigPendingWithinOneUpdate)
{
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();

    performFlushWithCreateNodeAction(0, 10000);
    performFlushWithCreateNodeAction(1, 10000);
    performFlushWithCreateNodeAction(0, 10000);
    performFlushWithCreateNodeAction(1, 10000);
    performFlushWithCreateNodeAction(0, 10000);
    performFlushWithCreateNodeAction(1, 10000);

    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene(0u));
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene(1u));
}

/////////////////////////////////////////////
// Tests for marking scenes as modified
/////////////////////////////////////////////

TEST_F(ARendererSceneUpdater, DoesNotMarkSceneAsModified_IfStreamTextureStateIsNotUpdated)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();
    createRenderableAndResourcesWithStreamTexture();

    expectStreamTextureUploaded();
    expectVertexArrayUploaded();
    update();
    expectRenderableResourcesClean();

    {
        EXPECT_CALL(renderer.m_embeddedCompositingManager, dispatchStateChangesOfSources(_, _, _));

        expectNoModifiedScenesReportedToRenderer();
        update();
    }

    hideScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarksSceneAsModified_IfStreamSourceContentIsUpdated_usedByScenes)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    mapScene(0u);
    mapScene(1u);
    showScene(0u);
    showScene(1u);

    constexpr WaylandIviSurfaceId source{ 12u };
    constexpr StreamTextureHandle tex1{ 13u };
    constexpr StreamTextureHandle tex2{ 14u };
    const StreamUsage fakeStreamUsage{ { { getSceneId(0u), { tex1 } }, { getSceneId(1u), { tex2 } } }, {} };

    StreamSourceUpdates updates{ {source, 1u } };
    EXPECT_CALL(renderer.m_embeddedCompositingManager, hasUpdatedContentFromStreamSourcesToUpload()).WillOnce(Return(true));
    EXPECT_CALL(renderer.m_embeddedCompositingManager, uploadResourcesAndGetUpdates(_)).WillOnce(SetArgReferee<0>(updates));
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getStreamUsage(source)).WillOnce(ReturnRef(fakeStreamUsage));

    expectModifiedScenesReportedToRenderer({ 0u, 1u });
    update();

    hideScene(0u);
    hideScene(1u);
    unmapScene(0u);
    unmapScene(1u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarksSceneAsModified_IfStreamSourceContentIsUpdated_usedByStreamBufferLinkedToScenes)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    mapScene(0u);
    mapScene(1u);
    showScene(0u);
    showScene(1u);

    constexpr WaylandIviSurfaceId source{ 12u };
    constexpr StreamBufferHandle sb1{ 13u };
    constexpr StreamBufferHandle sb2{ 14u };
    const StreamUsage fakeStreamUsage{ {}, { sb1, sb2 } };

    // link both SBs to scenes
    const auto dataSlotId1 = createTextureConsumer(0u);
    const auto dataSlotId2 = createTextureConsumer(1u);
    createBufferLink(sb1, getSceneId(0u), dataSlotId1);
    createBufferLink(sb2, getSceneId(1u), dataSlotId2);
    update();

    StreamSourceUpdates updates{ {source, 1u } };
    EXPECT_CALL(renderer.m_embeddedCompositingManager, hasUpdatedContentFromStreamSourcesToUpload()).WillOnce(Return(true));
    EXPECT_CALL(renderer.m_embeddedCompositingManager, uploadResourcesAndGetUpdates(_)).WillOnce(SetArgReferee<0>(updates));
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getStreamUsage(source)).WillOnce(ReturnRef(fakeStreamUsage));

    expectModifiedScenesReportedToRenderer({ 0u, 1u });
    update();

    hideScene(0u);
    hideScene(1u);
    unmapScene(0u);
    unmapScene(1u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarksSceneAsModified_IfStreamSourceAvailabilityChanged_usedByScenes)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    mapScene(0u);
    mapScene(1u);
    showScene(0u);
    showScene(1u);

    constexpr WaylandIviSurfaceId source{ 12u };
    constexpr StreamTextureHandle tex1{ 13u };
    constexpr StreamTextureHandle tex2{ 14u };
    const StreamUsage fakeStreamUsage{ { { getSceneId(0u), { tex1 } }, { getSceneId(1u), { tex2 } } }, {} };

    WaylandIviSurfaceIdVector changedSources{ source };
    EXPECT_CALL(renderer.m_embeddedCompositingManager, dispatchStateChangesOfSources(_, _, _)).WillOnce(SetArgReferee<0>(changedSources));
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getStreamUsage(source)).WillOnce(ReturnRef(fakeStreamUsage));
    EXPECT_CALL(renderer.m_embeddedCompositingManager, getCompositedTextureDeviceHandleForStreamTexture(_)).WillOnce(Return(DeviceResourceHandle::Invalid()));

    expectModifiedScenesReportedToRenderer({ 0u, 1u });
    update();

    hideScene(0u);
    hideScene(1u);
    unmapScene(0u);
    unmapScene(1u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarksSceneAsModified_IfStreamSourceAvailabilityChanged_usedByStreamBufferLinkedToScenes)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    mapScene(0u);
    mapScene(1u);
    showScene(0u);
    showScene(1u);

    constexpr WaylandIviSurfaceId source{ 12u };
    constexpr StreamBufferHandle sb1{ 13u };
    constexpr StreamBufferHandle sb2{ 14u };
    const StreamUsage fakeStreamUsage{ {}, { sb1, sb2 } };

    // link both SBs to scenes
    const auto dataSlotId1 = createTextureConsumer(0u);
    const auto dataSlotId2 = createTextureConsumer(1u);
    createBufferLink(sb1, getSceneId(0u), dataSlotId1);
    createBufferLink(sb2, getSceneId(1u), dataSlotId2);
    update();

    WaylandIviSurfaceIdVector changedSources{ source };
    EXPECT_CALL(renderer.m_embeddedCompositingManager, dispatchStateChangesOfSources(_, _, _)).WillOnce(SetArgReferee<0>(changedSources));
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getStreamUsage(source)).WillOnce(ReturnRef(fakeStreamUsage));
    EXPECT_CALL(renderer.m_embeddedCompositingManager, getCompositedTextureDeviceHandleForStreamTexture(_)).WillOnce(Return(DeviceResourceHandle::Invalid()));

    expectModifiedScenesReportedToRenderer({ 0u, 1u });
    update();

    hideScene(0u);
    hideScene(1u);
    unmapScene(0u);
    unmapScene(1u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, WillUnlinkStreamBuffer_IfStreamSourceBecameUnavailable_sameSource)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    mapScene(0u);
    mapScene(1u);
    showScene(0u);
    showScene(1u);

    constexpr WaylandIviSurfaceId source{ 12u };
    constexpr StreamBufferHandle sb1{ 13u };
    constexpr StreamBufferHandle sb2{ 14u };
    const StreamUsage fakeStreamUsage{ {}, { sb1, sb2 } };

    // link both SBs to scenes
    const auto dataSlotId1 = createTextureConsumer(0u);
    const auto dataSlotId2 = createTextureConsumer(1u);
    createBufferLink(sb1, getSceneId(0u), dataSlotId1);
    createBufferLink(sb2, getSceneId(1u), dataSlotId2);
    update();

    WaylandIviSurfaceIdVector changedSources{ source };
    EXPECT_CALL(renderer.m_embeddedCompositingManager, dispatchStateChangesOfSources(_, _, _)).WillOnce(SetArgReferee<0>(changedSources));
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getStreamUsage(source)).WillOnce(ReturnRef(fakeStreamUsage));
    EXPECT_CALL(renderer.m_embeddedCompositingManager, getCompositedTextureDeviceHandleForStreamTexture(_)).WillOnce(Return(DeviceResourceHandle::Invalid()));

    expectModifiedScenesReportedToRenderer({ 0u, 1u });
    update();

    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getStreamBufferLinks().hasAnyLinksToConsumer(sb1));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getStreamBufferLinks().hasAnyLinksToConsumer(sb2));

    hideScene(0u);
    hideScene(1u);
    unmapScene(0u);
    unmapScene(1u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, WillUnlinkStreamBuffer_IfOneOfStreamSourcesBecameUnavailabile)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    mapScene(0u);
    mapScene(1u);
    showScene(0u);
    showScene(1u);

    constexpr WaylandIviSurfaceId source1{ 12u };
    constexpr WaylandIviSurfaceId source2{ 13u };
    constexpr StreamBufferHandle sb1{ 13u };
    constexpr StreamBufferHandle sb2{ 14u };
    const StreamUsage fakeStreamUsage1{ {}, { sb1 } };
    const StreamUsage fakeStreamUsage2{ {}, { sb2 } };

    // link both SBs to scenes
    const auto dataSlotId1 = createTextureConsumer(0u);
    const auto dataSlotId2 = createTextureConsumer(1u);
    createBufferLink(sb1, getSceneId(0u), dataSlotId1);
    createBufferLink(sb2, getSceneId(1u), dataSlotId2);
    update();

    const WaylandIviSurfaceIdVector changedSources{ source1, source2 };
    EXPECT_CALL(renderer.m_embeddedCompositingManager, dispatchStateChangesOfSources(_, _, _)).WillOnce(SetArgReferee<0>(changedSources));
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getStreamUsage(source1)).WillOnce(ReturnRef(fakeStreamUsage1));
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getStreamUsage(source2)).WillOnce(ReturnRef(fakeStreamUsage2));
    // only source2 is unavailable
    EXPECT_CALL(renderer.m_embeddedCompositingManager, getCompositedTextureDeviceHandleForStreamTexture(source1)).WillOnce(Return(DeviceMock::FakeRenderTargetDeviceHandle));
    EXPECT_CALL(renderer.m_embeddedCompositingManager, getCompositedTextureDeviceHandleForStreamTexture(source2)).WillOnce(Return(DeviceResourceHandle::Invalid()));

    expectModifiedScenesReportedToRenderer({ 1u });
    update();

    EXPECT_TRUE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getStreamBufferLinks().hasAnyLinksToConsumer(sb1));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getStreamBufferLinks().hasAnyLinksToConsumer(sb2));

    hideScene(0u);
    hideScene(1u);
    unmapScene(0u);
    unmapScene(1u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarksSceneAsModified_IfNonEmptyFlushApplied)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();
    update();

    performFlushWithCreateNodeAction();
    expectModifiedScenesReportedToRenderer();
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    hideScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, DoesNotMarkSceneAsModified_IfEmptyFlushAppliedOnEmptyScene)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();
    update();

    expectNoModifiedScenesReportedToRenderer();
    performFlush(0u);
    update();

    hideScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, DoesNotMarkSceneAsModified_IfEmptyFlushAppliedOnNonEmptyScene)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    performFlushWithCreateNodeAction();
    expectModifiedScenesReportedToRenderer();
    update();

    expectNoModifiedScenesReportedToRenderer();
    performFlush(0u);
    update();

    hideScene();
    unmapScene();
    destroyDisplay();
}


TEST_F(ARendererSceneUpdater, MarksSceneAsModified_IfFlushAppliedWithResources)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    createRenderable();
    setRenderableResources();

    expectModifiedScenesReportedToRenderer();
    expectVertexArrayUploaded();
    update();
    expectRenderableResourcesClean();

    expectNoModifiedScenesReportedToRenderer();
    update();

    hideScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, DoesNotMarkSceneAsModified_IfPendingFlushNotApplied)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    createRenderable();

    {
        setRenderableResources();
        expectModifiedScenesReportedToRenderer();
        expectVertexArrayUploaded();
        update();
        expectRenderableResourcesClean();
        EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());
    }

    expectNoModifiedScenesReportedToRenderer();
    update();

    {
        expectResourcesReferencedAndProvided({ MockResourceHash::IndexArrayHash2 });
        setRenderableResources(0u, MockResourceHash::IndexArrayHash2);
        reportResourceAs(MockResourceHash::IndexArrayHash2, EResourceStatus::Provided);

        expectNoModifiedScenesReportedToRenderer();
        update();
        EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());
    }

    hideScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarkSceneAsModified_AfterPendingFlushApplied)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    {
        expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
        createRenderable();
        setRenderableResources();
        expectModifiedScenesReportedToRenderer();
        expectVertexArrayUploaded();
        update();
        expectRenderableResourcesClean();
    }
    expectNoModifiedScenesReportedToRenderer();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    {
        // blocking flush
        expectResourcesReferencedAndProvided({ MockResourceHash::IndexArrayHash2 });
        setRenderableResources(0u, MockResourceHash::IndexArrayHash2);
        reportResourceAs(MockResourceHash::IndexArrayHash2, EResourceStatus::Provided);
        EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getResourceDeviceHandle(MockResourceHash::IndexArrayHash2)).WillOnce(Return(DeviceResourceHandle::Invalid())).RetiresOnSaturation();
    }
    expectNoModifiedScenesReportedToRenderer();
    expectVertexArrayUnloaded();
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    {
        // unblock flush
        expectResourcesUnreferenced({ MockResourceHash::IndexArrayHash });
        reportResourceAs(MockResourceHash::IndexArrayHash2, EResourceStatus::Uploaded);
    }
    expectModifiedScenesReportedToRenderer();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    hideScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarkSceneAsModified_DataLinking_IfSceneIsConsumerAndProviderSceneIsUpdated)
{
    // s0 [modified] -> s1 [modified]
    createDisplayAndExpectSuccess();

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    mapScene(0u);
    mapScene(1u);
    showScene(0u);
    showScene(1u);

    DataInstanceHandle consumerDataRef;
    DataInstanceHandle providerDataRef;
    createDataSlotsAndLinkThem(consumerDataRef, 333.f, &providerDataRef);
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    updateProviderDataSlot(0u, providerDataRef, 777.f);
    performFlush();
    expectModifiedScenesReportedToRenderer({0u, 1u});
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    hideScene(0u);
    hideScene(1u);
    unmapScene(0u);
    unmapScene(1u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, DoesNotMarkSceneAsModified_DataLinking_IfSceneIsProviderAndConsumerIsUpdated)
{
    // s0 -> s1 [modified]
    createDisplayAndExpectSuccess();

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    mapScene(1u);
    showScene(1u);

    DataInstanceHandle consumerDataRef;
    const float providedValue(333.f);
    createDataSlotsAndLinkThem(consumerDataRef, providedValue);
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    stagingScene[1u]->setDataSingleFloat(consumerDataRef, DataFieldHandle(0u), 324.f);
    performFlush(1u);
    expectModifiedScenesReportedToRenderer({1u}); //only consumer if marked as modified
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    hideScene(1u);
    unmapScene(1u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarkSceneAsModified_DataLinking_IndirectlyDependantConsumersIfProviderUpdated)
{
    // s0 [modified] -> s1 [modified] -> s2 [modified]
    createDisplayAndExpectSuccess();

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    mapScene(0u);
    mapScene(1u);
    mapScene(2u);
    showScene(0u);
    showScene(1u);
    showScene(2u);

    DataInstanceHandle providerDataRef;
    {
        //link scene 1 to scene 0
        DataInstanceHandle consumerDataRef;
        createDataSlotsAndLinkThem(consumerDataRef, 333.f, &providerDataRef);
        update();
    }

    {
        //linke scene 2 to scene 1
        DataInstanceHandle consumerDataRef2;
        createDataSlotsAndLinkThem(consumerDataRef2, 666.f, nullptr, 1u, 2u);
        update();
    }

    expectNoModifiedScenesReportedToRenderer();
    update();

    updateProviderDataSlot(0u, providerDataRef, 1.0f);
    performFlush();
    expectModifiedScenesReportedToRenderer({0u, 1u, 2u});
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    hideScene(0u);
    hideScene(1u);
    hideScene(2u);
    unmapScene(0u);
    unmapScene(1u);
    unmapScene(2u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarkSceneAsModified_DataLinking_OnlyDependantConsumersIfProviderUpdated)
{
    // s0 -> s1 [modified] -> s2 [modified]
    createDisplayAndExpectSuccess();

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    mapScene(0u);
    mapScene(1u);
    mapScene(2u);
    showScene(0u);
    showScene(1u);
    showScene(2u);

    {
        //link scene 1 to scene 0
        DataInstanceHandle consumerDataRef;
        createDataSlotsAndLinkThem(consumerDataRef, 333.f);
        update();
    }

    DataInstanceHandle providerDataRef;
    {
        //linke scene 2 to scene 1
        DataInstanceHandle consumerDataRef2;
        createDataSlotsAndLinkThem(consumerDataRef2, 666.f, &providerDataRef, 1u, 2u);
        update();
    }

    expectNoModifiedScenesReportedToRenderer();
    update();

    updateProviderDataSlot(1u, providerDataRef, 1.0f);
    performFlush(1u);
    expectModifiedScenesReportedToRenderer({1u, 2u});
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    hideScene(0u);
    hideScene(1u);
    hideScene(2u);
    unmapScene(0u);
    unmapScene(1u);
    unmapScene(2u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarkSceneAsModified_DataLinking_IfSceneConsumesFromAnotherConsumerOfDifferentLinkingType)
{
    // tex linking      : s0 -> s1
    // data ref linking :       s1 [modified] -> s2 [modified]
    createDisplayAndExpectSuccess();

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    mapScene(0u);
    mapScene(1u);
    mapScene(2u);
    showScene(0u);
    showScene(1u);
    showScene(2u);

    {
        //tex. link scene 1 to scene 0
        expectResourcesReferencedAndProvided({ MockResourceHash::TextureHash });
        createTextureSlotsAndLinkThem();
        update();
    }

    DataInstanceHandle providerDataRef;
    {
        //data ref. linke scene 2 to scene 1
        DataInstanceHandle consumerDataRef2;
        createDataSlotsAndLinkThem(consumerDataRef2, 666.f, &providerDataRef, 1u, 2u);
        update();
    }

    expectNoModifiedScenesReportedToRenderer();
    update();

    updateProviderDataSlot(1u, providerDataRef, 1.0f);
    performFlush(1u);
    expectModifiedScenesReportedToRenderer({1u, 2u});
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    hideScene(0u);
    hideScene(1u);
    hideScene(2u);
    unmapScene(0u);
    unmapScene(1u);
    unmapScene(2u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarkSceneAsModified_DataLinking_ConfidenceTest)
{
    // s0 -> s1 [modified] -> s2 [modified] -> s3 [modified]
    createDisplayAndExpectSuccess();

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    mapScene(1u);
    mapScene(2u);
    mapScene(3u);
    showScene(1u);
    showScene(2u);
    showScene(3u);

    {
        //link scene 1 to scene 0
        DataInstanceHandle consumerDataRef;
        createDataSlotsAndLinkThem(consumerDataRef, 333.f);
        update();
    }

    DataInstanceHandle providerDataRef;
    {
        //linke scene 2 to scene 1
        DataInstanceHandle consumerDataRef;
        createDataSlotsAndLinkThem(consumerDataRef, 666.f, &providerDataRef, 1u, 2u);
        update();
    }

    {
        //linke scene 3 to scene 2
        DataInstanceHandle consumerDataRef;
        createDataSlotsAndLinkThem(consumerDataRef, 766.f, nullptr, 2u, 3u);
        update();
    }

    expectNoModifiedScenesReportedToRenderer();
    update();

    updateProviderDataSlot(1u, providerDataRef, 1.0f);
    performFlush(1u);
    expectModifiedScenesReportedToRenderer({1u, 2u, 3u});
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    hideScene(1u);
    hideScene(2u);
    hideScene(3u);
    unmapScene(1u);
    unmapScene(2u);
    unmapScene(3u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarkSceneAsModified_TextureLinking_IfSceneIsConsumerAndProviderSceneIsUpdated)
{
    // s0 [modified] -> s1 [modified]
    createDisplayAndExpectSuccess();

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    mapScene(0u);
    showScene(0u);
    mapScene(1u);
    showScene(1u);

    DataSlotHandle providerDataSlotHandle;
    expectResourcesReferencedAndProvided({ MockResourceHash::TextureHash });
    createTextureSlotsAndLinkThem(&providerDataSlotHandle);
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    expectResourcesUnreferenced({ MockResourceHash::TextureHash });
    expectResourcesReferencedAndProvided({ MockResourceHash::TextureHash2 });
    updateProviderTextureSlot(0u, providerDataSlotHandle, MockResourceHash::TextureHash2);
    performFlush();
    expectModifiedScenesReportedToRenderer({0u, 1u});
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    hideScene(0u);
    hideScene(1u);
    unmapScene(0u);
    unmapScene(1u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarkSceneAsModified_TextureLinking_ConfidenceTest)
{
    // s0 -> s1 [modified] -> s2 [modified] -> s3 [modified]
    createDisplayAndExpectSuccess();

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    mapScene();
    mapScene(1u);
    mapScene(2u);
    mapScene(3u);
    showScene(1u);
    showScene(2u);
    showScene(3u);

    {
        //link scene 1 to scene 0
        expectResourcesReferencedAndProvided({ MockResourceHash::TextureHash });
        createTextureSlotsAndLinkThem();
        update();
    }

    DataSlotHandle providerDataSlotHandle;
    {
        //link scene 2 to scene 1
        expectResourcesReferencedAndProvided({ MockResourceHash::TextureHash }, 1u);
        createTextureSlotsAndLinkThem(&providerDataSlotHandle, 1u, 2u);
        update();
    }

    {
        //link scene 3 to scene 2
        expectResourcesReferencedAndProvided({ MockResourceHash::TextureHash }, 2u);
        createTextureSlotsAndLinkThem(nullptr, 2u, 3u);
        update();
    }

    expectNoModifiedScenesReportedToRenderer();
    update();

    expectResourcesUnreferenced({ MockResourceHash::TextureHash }, 1u);
    expectResourcesReferencedAndProvided({ MockResourceHash::TextureHash2 }, 1u);
    updateProviderTextureSlot(1u, providerDataSlotHandle, MockResourceHash::TextureHash2);
    performFlush(1u);
    expectModifiedScenesReportedToRenderer({1u, 2u, 3u});
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    hideScene(1u);
    hideScene(2u);
    hideScene(3u);
    unmapScene();
    unmapScene(1u);
    unmapScene(2u);
    unmapScene(3u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarkSceneAsModified_TransformationLinking_IfSceneIsConsumerAndProviderSceneIsUpdated)
{
    // s0 [modified] -> s1 [modified]
    createDisplayAndExpectSuccess();

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    mapScene(0u);
    mapScene(1u);
    showScene(0u);
    showScene(1u);

    TransformHandle providerTransformHandle;
    createTransformationSlotsAndLinkThem(&providerTransformHandle);
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    stagingScene[0]->setTranslation(providerTransformHandle, {0.f, 1.f, 2.f});
    performFlush();
    expectModifiedScenesReportedToRenderer({0u, 1u});
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    hideScene(0u);
    hideScene(1u);
    unmapScene(0u);
    unmapScene(1u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarkSceneAsModified_TransformationLinking_ConfidenceTest)
{
    // s0 -> s1 [modified] -> s2 [modified] -> s3 [modified]
    createDisplayAndExpectSuccess();

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    mapScene();
    mapScene(1u);
    mapScene(2u);
    mapScene(3u);
    showScene(1u);
    showScene(2u);
    showScene(3u);

    {
        //link scene 1 to scene 0
        createTransformationSlotsAndLinkThem();
        update();
    }

    TransformHandle providerTransformHandle;
    {
        //linke scene 2 to scene 1
        createTransformationSlotsAndLinkThem(&providerTransformHandle, 1u, 2u);
        update();
    }

    {
        //linke scene 3 to scene 2
        createTransformationSlotsAndLinkThem(nullptr, 2u, 3u);
        update();
    }

    expectNoModifiedScenesReportedToRenderer();
    update();

    stagingScene[1]->setTranslation(providerTransformHandle, {0.f, 1.f, 2.f});
    performFlush(1u);
    expectModifiedScenesReportedToRenderer({1u, 2u, 3u});
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    hideScene(1u);
    hideScene(2u);
    hideScene(3u);
    unmapScene();
    unmapScene(1u);
    unmapScene(2u);
    unmapScene(3u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarkSceneAsModified_OffscreenBufferLinking_IfSceneConsumesFromModifiedOffscreenBuffer)
{
    // s0 [modified] -> ob1 -> s1 [modified]
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    expectOffscreenBufferUploaded(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    mapScene(0u);
    mapScene(1u);
    showScene(0u);
    showScene(1u);

    EXPECT_TRUE(assignSceneToDisplayBuffer(0, buffer));

    const DataSlotId consumerId = createTextureConsumer(1u);
    createBufferLink(buffer, stagingScene[1u]->getSceneId() , consumerId);
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    //update scene mapped to buffer
    performFlushWithCreateNodeAction(0u);
    //expect both scenes modified
    expectModifiedScenesReportedToRenderer({0u, 1u});
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    hideScene(0u);
    hideScene(1u);
    unmapScene(0u);
    unmapScene(1u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarkSceneAsModified_OffscreenBufferLinking_IfSceneConsumesFromModifiedOffscreenBufferWithSeveralScenes)
{
    // s0            --
    //                 |-> ob1 -> s2 [modified]
    // s1 [modified] --
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    expectOffscreenBufferUploaded(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    mapScene(0u);
    mapScene(1u);
    mapScene(2u);
    showScene(0u);
    showScene(1u);
    showScene(2u);

    EXPECT_TRUE(assignSceneToDisplayBuffer(0, buffer));
    EXPECT_TRUE(assignSceneToDisplayBuffer(1, buffer));

    const DataSlotId consumerId = createTextureConsumer(2u);
    createBufferLink(buffer, getSceneId(2u), consumerId);
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    //update scene mapped to buffer
    performFlushWithCreateNodeAction(1u);

    expectModifiedScenesReportedToRenderer({ 1u, 2u });
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    hideScene(0u);
    hideScene(1u);
    hideScene(2u);
    unmapScene(0u);
    unmapScene(1u);
    unmapScene(2u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, DoesNotMarkSceneAsModified_OffscreenBufferLinking_UnmodifiedProviderToOffscreenBuffer)
{
    // s0 -> ob1 -> s1 [modified]
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    expectOffscreenBufferUploaded(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    mapScene(0u);
    mapScene(1u);
    showScene(0u);
    showScene(1u);

    EXPECT_TRUE(assignSceneToDisplayBuffer(0, buffer));

    const DataSlotId consumerId = createTextureConsumer(1u);
    createBufferLink(buffer, stagingScene[1u]->getSceneId(), consumerId);
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    //update consumer
    performFlushWithCreateNodeAction(1u);
    //expect only consumer modified
    expectModifiedScenesReportedToRenderer({ 1u });
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    hideScene(0u);
    hideScene(1u);
    unmapScene(0u);
    unmapScene(1u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarkSceneAsModified_OffscreenBufferLinking_TwoConsumersFromModifiedOffscreenBuffer)
{
    //                       --> s1 [modified]
    // s0 [modified] -> ob1-|
    //                       --> s2 [modified]
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    expectOffscreenBufferUploaded(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    mapScene(0u);
    mapScene(1u);
    mapScene(2u);
    showScene(0u);
    showScene(1u);
    showScene(2u);

    EXPECT_TRUE(assignSceneToDisplayBuffer(0, buffer));

    {
        const DataSlotId consumerId = createTextureConsumer(1u);
        createBufferLink(buffer, stagingScene[1u]->getSceneId() , consumerId);
        update();
    }
    {
        const DataSlotId consumerId = createTextureConsumer(2u);
        createBufferLink(buffer, stagingScene[2u]->getSceneId() , consumerId);
        update();
    }

    expectNoModifiedScenesReportedToRenderer();
    update();

    //update scene mapped to buffer
    performFlushWithCreateNodeAction(0u);
    //expect both scenes modified
    expectModifiedScenesReportedToRenderer({0u, 1u, 2u});
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    hideScene(0u);
    hideScene(1u);
    hideScene(2u);
    unmapScene(0u);
    unmapScene(1u);
    unmapScene(2u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarkSceneAsModified_OffscreenBufferLinking_IndirectConsumerFromModifiedOffscreenBuffer)
{
    // s0 [modified] -> ob1 -> s1 [modified] -> ob2 -> s2 [modified]
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer1(1u);
    expectOffscreenBufferUploaded(buffer1);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer1, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);

    const OffscreenBufferHandle buffer2(2u);
    const DeviceResourceHandle offscreenBufferDeviceHandle(5556u);
    expectOffscreenBufferUploaded(buffer2, offscreenBufferDeviceHandle);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer2, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    mapScene(0u);
    mapScene(1u);
    mapScene(2u);
    showScene(0u);
    showScene(1u);
    showScene(2u);

    EXPECT_TRUE(assignSceneToDisplayBuffer(0, buffer1));

    {
        EXPECT_TRUE(assignSceneToDisplayBuffer(1, buffer2));
        const DataSlotId consumerId = createTextureConsumer(1u);
        createBufferLink(buffer1, stagingScene[1u]->getSceneId() , consumerId);
        update();
    }
    {
        const DataSlotId consumerId = createTextureConsumer(2u);
        createBufferLink(buffer2, stagingScene[2u]->getSceneId() , consumerId);
        update();
    }

    expectNoModifiedScenesReportedToRenderer();
    update();

    //update scene mapped to buffer 1
    performFlushWithCreateNodeAction(0u);
    //expect all scenes modified
    expectModifiedScenesReportedToRenderer({0u, 1u, 2u});
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    hideScene(0u);
    hideScene(1u);
    hideScene(2u);
    unmapScene(0u);
    unmapScene(1u);
    unmapScene(2u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, DoesNotMarkSceneAsModified_OffscreenBufferLinking_ConsumerFromUnmodifiedOffscreenBuffer)
{
    // s0 [modified] -> ob1
    // s1 -> ob2 -> s2
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer1(1u);
    expectOffscreenBufferUploaded(buffer1);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer1, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);

    const OffscreenBufferHandle buffer2(2u);
    const DeviceResourceHandle offscreenBufferDeviceHandle(5556u);
    expectOffscreenBufferUploaded(buffer2, offscreenBufferDeviceHandle);

    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer2, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    mapScene(0u);
    mapScene(1u);
    mapScene(2u);
    showScene(0u);
    showScene(1u);
    showScene(2u);

    EXPECT_TRUE(assignSceneToDisplayBuffer(0, buffer1));
    EXPECT_TRUE(assignSceneToDisplayBuffer(1, buffer2));

    const DataSlotId consumerId = createTextureConsumer(2u);
    createBufferLink(buffer2, stagingScene[2u]->getSceneId() , consumerId);
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    //update scene mapped to buffer 1
    performFlushWithCreateNodeAction(0u);
    //expect only this scene to be modified
    expectModifiedScenesReportedToRenderer({0u});
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    hideScene(0u);
    hideScene(1u);
    hideScene(2u);
    unmapScene(0u);
    unmapScene(1u);
    unmapScene(2u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarkSceneAsModified_OffscreenBufferLinking_ConfidenceTest1)
{
    //             --> s1
    // s0 -> ob0 -|                           --> s3 [modified]
    //             --> s2 [modified] -> ob1 -|
    //                                        --> s4 [modified] -> ob2 -> s5 [modified]

    createDisplayAndExpectSuccess();

    const std::array<OffscreenBufferHandle, 3> buffers{ { OffscreenBufferHandle{ 1u }, OffscreenBufferHandle{ 2u }, OffscreenBufferHandle{ 3u } } };
    for (const auto buffer : buffers)
    {
        expectOffscreenBufferUploaded(buffer, DeviceResourceHandle(buffer.asMemoryHandle()));
        EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer));
        expectEvent(ERendererEventType::OffscreenBufferCreated);
    }

    for (UInt32 i = 0u; i < 6; ++i)
    {
        createPublishAndSubscribeScene();
        mapScene(i);
        showScene(i);
    }

    EXPECT_TRUE(assignSceneToDisplayBuffer(0, buffers[0]));
    EXPECT_TRUE(assignSceneToDisplayBuffer(2, buffers[1]));
    EXPECT_TRUE(assignSceneToDisplayBuffer(4, buffers[2]));

    {
        const DataSlotId consumerId = createTextureConsumer(1u);
        createBufferLink(buffers[0u], getSceneId(1u), consumerId);
    }
    {
        const DataSlotId consumerId = createTextureConsumer(2u);
        createBufferLink(buffers[0u], getSceneId(2u), consumerId);
    }
    {
        const DataSlotId consumerId = createTextureConsumer(3u);
        createBufferLink(buffers[1u], getSceneId(3u), consumerId);
    }
    {
        const DataSlotId consumerId = createTextureConsumer(4u);
        createBufferLink(buffers[1u], getSceneId(4u), consumerId);
    }
    {
        const DataSlotId consumerId = createTextureConsumer(5u);
        createBufferLink(buffers[2u], getSceneId(5u), consumerId);
    }
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    //update s2
    performFlushWithCreateNodeAction(2u);

    expectModifiedScenesReportedToRenderer({ 2u, 3u, 4u, 5u });
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();


    for (UInt32 i = 0u; i < 6; ++i)
    {
        hideScene(i);
        unmapScene(i);
    }

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarkSceneAsModified_OffscreenBufferLinking_ConfidenceTest2)
{
    // s0 --
    //      |-> ob0 -> s2 ------------> ob1 --
    // s1 --                                  |-> s4 [modified]
    //                 s3 [modified] -> ob2 --

    createDisplayAndExpectSuccess();

    const std::array<OffscreenBufferHandle, 3> buffers{ { OffscreenBufferHandle{ 1u }, OffscreenBufferHandle{ 2u }, OffscreenBufferHandle{ 3u } } };
    for (const auto buffer : buffers)
    {
        expectOffscreenBufferUploaded(buffer, DeviceResourceHandle(buffer.asMemoryHandle()));
        EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer));
        expectEvent(ERendererEventType::OffscreenBufferCreated);
    }

    for (UInt32 i = 0u; i < 5; ++i)
    {
        createPublishAndSubscribeScene();
        mapScene(i);
        showScene(i);
    }

    EXPECT_TRUE(assignSceneToDisplayBuffer(0, buffers[0]));
    EXPECT_TRUE(assignSceneToDisplayBuffer(1, buffers[0]));
    EXPECT_TRUE(assignSceneToDisplayBuffer(2, buffers[1]));
    EXPECT_TRUE(assignSceneToDisplayBuffer(3, buffers[2]));

    {
        const DataSlotId consumerId = createTextureConsumer(2u);
        createBufferLink(buffers[0u], getSceneId(2u), consumerId);
    }
    {
        const DataSlotId consumerId = createTextureConsumer(4u);
        createBufferLink(buffers[1u], getSceneId(4u), consumerId);
    }
    {
        const DataSlotId consumerId = createTextureConsumer(4u);
        createBufferLink(buffers[2u], getSceneId(4u), consumerId);
    }
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    //update s3
    performFlushWithCreateNodeAction(3u);

    expectModifiedScenesReportedToRenderer({ 3u, 4u });
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    for (UInt32 i = 0u; i < 5; ++i)
    {
        hideScene(i);
        unmapScene(i);
    }

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarksSceneAsModified_IfShaderAnimationIsActive)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });

    createRenderable();
    setRenderableResources();

    expectModifiedScenesReportedToRenderer();
    expectVertexArrayUploaded();
    update();
    expectRenderableResourcesClean();

    // simulate rendering
    auto& rendererScene = rendererScenes.getScene(stagingScene[0]->getSceneId());
    EXPECT_FALSE(rendererScene.hasActiveShaderAnimation());
    rendererScene.setActiveShaderAnimation(true);

    expectModifiedScenesReportedToRenderer();
    update();

    // flush resets shader animation
    EXPECT_TRUE(rendererScene.hasActiveShaderAnimation());
    performFlush();
    // empty flush marks scene modified if shader animation was active before
    expectModifiedScenesReportedToRenderer();
    update();
    EXPECT_FALSE(rendererScene.hasActiveShaderAnimation());

    performFlush();
    update();

    hideScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, DoesNotMarkSceneAsModified_IfOtherSceneHasShaderAnimation)
{
    createDisplayAndExpectSuccess();

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();

    mapScene(0);
    mapScene(1);

    showScene(0);
    showScene(1);

    // simulate rendering
    auto& rendererScene1 = rendererScenes.getScene(getSceneId(0));
    auto& rendererScene2 = rendererScenes.getScene(getSceneId(1));

    EXPECT_FALSE(rendererScene1.hasActiveShaderAnimation());
    EXPECT_FALSE(rendererScene2.hasActiveShaderAnimation());
    rendererScene2.setActiveShaderAnimation(true);

    expectModifiedScenesReportedToRenderer({1});
    update();

    EXPECT_FALSE(rendererScene1.hasActiveShaderAnimation());
    EXPECT_TRUE(rendererScene2.hasActiveShaderAnimation());

    hideScene(0);
    hideScene(1);
    unmapScene(0);
    unmapScene(1);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarksSceneAsModified_IfEffectTimeIsSynced)
{
    createDisplayAndExpectSuccess();
    const auto scene = createPublishAndSubscribeScene();
    mapScene();
    showScene();

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });

    createRenderable();
    setRenderableResources();

    expectModifiedScenesReportedToRenderer();
    expectVertexArrayUploaded();
    update();
    expectRenderableResourcesClean();

    const auto& rendererScene = rendererScenes.getScene(stagingScene[0]->getSceneId());
    EXPECT_EQ(FlushTime::InvalidTimestamp, rendererScene.getEffectTimeSync());
    performFlushWithUniformTimeSync(scene, 1000u);

    expectModifiedScenesReportedToRenderer();
    update();
    EXPECT_EQ(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u)), rendererScene.getEffectTimeSync());

    hideScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarksSceneAsModified_IfOffscreenBufferLinkedToScene)
{
    createDisplayAndExpectSuccess();
    const UInt32 scene1 = createPublishAndSubscribeScene();
    const UInt32 scene2 = createPublishAndSubscribeScene();
    mapScene(scene1);
    mapScene(scene2);

    const OffscreenBufferHandle buffer(1u);
    expectOffscreenBufferUploaded(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);
    EXPECT_TRUE(assignSceneToDisplayBuffer(scene2, buffer));

    const DataSlotId consumer = createTextureConsumer(scene1);

    showScene(scene1);
    showScene(scene2);

    expectNoModifiedScenesReportedToRenderer();
    update();

    createBufferLink(buffer, getSceneId(scene1), consumer);
    expectModifiedScenesReportedToRenderer({ scene1 });
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    hideScene(scene1);
    hideScene(scene2);
    unmapScene(scene1);
    unmapScene(scene2);

    expectOffscreenBufferDeleted(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferDestroyRequest(buffer));

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarksSceneAsModified_IfOffscreenBufferUnlinkedFromScene)
{
    createDisplayAndExpectSuccess();
    const UInt32 scene1 = createPublishAndSubscribeScene();
    const UInt32 scene2 = createPublishAndSubscribeScene();
    mapScene(scene1);
    mapScene(scene2);

    const OffscreenBufferHandle buffer(1u);
    expectOffscreenBufferUploaded(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);
    EXPECT_TRUE(assignSceneToDisplayBuffer(scene2, buffer));

    const DataSlotId consumer = createTextureConsumer(scene1);

    showScene(scene1);
    showScene(scene2);

    expectNoModifiedScenesReportedToRenderer();
    update();

    createBufferLink(buffer, getSceneId(scene1), consumer);
    expectModifiedScenesReportedToRenderer({ scene1 });
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    unlinkConsumer(getSceneId(scene1), consumer);
    expectModifiedScenesReportedToRenderer({ scene1 });
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    hideScene(scene1);
    hideScene(scene2);
    unmapScene(scene1);
    unmapScene(scene2);

    expectOffscreenBufferDeleted(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferDestroyRequest(buffer));

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarksSceneAsModified_WhenProviderSceneAssignedToOBIsShownAfterOBLinked)
{
    createDisplayAndExpectSuccess();
    const UInt32 scene1 = createPublishAndSubscribeScene();
    const UInt32 scene2 = createPublishAndSubscribeScene();
    const DataSlotId consumer = createTextureConsumer(scene1);
    mapScene(scene1);
    mapScene(scene2);
    showScene(scene1);

    const OffscreenBufferHandle buffer(1u);
    expectOffscreenBufferUploaded(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);
    EXPECT_TRUE(assignSceneToDisplayBuffer(scene2, buffer));

    expectNoModifiedScenesReportedToRenderer();
    update();

    createBufferLink(buffer, getSceneId(scene1), consumer);
    expectModifiedScenesReportedToRenderer({ scene1 });
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    expectModifiedScenesReportedToRenderer({ scene1, scene2 });
    showScene(scene2);

    expectNoModifiedScenesReportedToRenderer();
    update();

    hideScene(scene1);
    hideScene(scene2);
    unmapScene(scene1);
    unmapScene(scene2);

    expectOffscreenBufferDeleted(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferDestroyRequest(buffer));

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarkSceneAsModified_IfTransformationConsumerAndProviderLinkedOrUnlinked)
{
    createDisplayAndExpectSuccess();

    const UInt32 providerScene = createPublishAndSubscribeScene();
    const UInt32 consumerScene = createPublishAndSubscribeScene();
    mapScene(providerScene);
    mapScene(consumerScene);
    showScene(providerScene);
    showScene(consumerScene);

    const auto providerConsumer = createTransformationSlots(nullptr, providerScene, consumerScene);
    update();

    linkProviderToConsumer(getSceneId(providerScene), providerConsumer.first, getSceneId(consumerScene), providerConsumer.second);
    expectModifiedScenesReportedToRenderer({ consumerScene });
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    unlinkConsumer(getSceneId(consumerScene), providerConsumer.second);
    expectModifiedScenesReportedToRenderer({ consumerScene });
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    hideScene(providerScene);
    hideScene(consumerScene);
    unmapScene(providerScene);
    unmapScene(consumerScene);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarkSceneAsModified_IfDataConsumerAndProviderLinkedOrUnlinked)
{
    createDisplayAndExpectSuccess();

    const UInt32 providerScene = createPublishAndSubscribeScene();
    const UInt32 consumerScene = createPublishAndSubscribeScene();
    mapScene(providerScene);
    mapScene(consumerScene);
    showScene(providerScene);
    showScene(consumerScene);

    DataInstanceHandle dataRef;
    const auto providerConsumer = createDataSlots(dataRef, 1.f, nullptr, providerScene, consumerScene);
    update();

    linkProviderToConsumer(getSceneId(providerScene), providerConsumer.first, getSceneId(consumerScene), providerConsumer.second);
    expectModifiedScenesReportedToRenderer({ consumerScene });
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    unlinkConsumer(getSceneId(consumerScene), providerConsumer.second);
    expectModifiedScenesReportedToRenderer({ consumerScene });
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    hideScene(providerScene);
    hideScene(consumerScene);
    unmapScene(providerScene);
    unmapScene(consumerScene);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarkSceneAsModified_IfTextureConsumerAndProviderLinkedOrUnlinked)
{
    createDisplayAndExpectSuccess();

    const UInt32 providerScene = createPublishAndSubscribeScene();
    const UInt32 consumerScene = createPublishAndSubscribeScene();
    mapScene(providerScene);
    mapScene(consumerScene);
    showScene(providerScene);
    showScene(consumerScene);

    expectResourcesReferencedAndProvided({ MockResourceHash::TextureHash }, providerScene);
    const auto providerConsumer = createTextureSlots(nullptr, providerScene, consumerScene);
    update();

    linkProviderToConsumer(getSceneId(providerScene), providerConsumer.first, getSceneId(consumerScene), providerConsumer.second);
    expectModifiedScenesReportedToRenderer({ consumerScene });
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    unlinkConsumer(getSceneId(consumerScene), providerConsumer.second);
    expectModifiedScenesReportedToRenderer({ consumerScene });
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    hideScene(providerScene);
    hideScene(consumerScene);
    unmapScene(providerScene);
    unmapScene(consumerScene);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, DoesNotMarkSceneAsModified_IfModifiedSceneNotShown)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    update();

    performFlushWithCreateNodeAction();
    expectNoModifiedScenesReportedToRenderer();
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarksSceneAsModified_IfSceneNotModifiedButSkippingDisabled)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();
    update();

    performFlushWithCreateNodeAction();
    expectModifiedScenesReportedToRenderer();
    update();

    expectNoModifiedScenesReportedToRenderer();
    update();

    // no change to scene but disabled skipping feature
    rendererSceneUpdater->setSkippingOfUnmodifiedScenes(false);
    expectModifiedScenesReportedToRenderer();
    update();

    expectModifiedScenesReportedToRenderer();
    update();

    rendererSceneUpdater->setSkippingOfUnmodifiedScenes(true);
    expectNoModifiedScenesReportedToRenderer();
    update();

    hideScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, DoesNotMarkSceneAsModified_IfSkippingDisabledButSceneNotShown)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    update();

    performFlushWithCreateNodeAction();
    expectNoModifiedScenesReportedToRenderer();
    update();

    // no change to scene but disabled skipping feature
    rendererSceneUpdater->setSkippingOfUnmodifiedScenes(false);
    // still not reported to re-render because not shown
    expectNoModifiedScenesReportedToRenderer();
    update();

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, resetsInterruptedRenderingIfInterruptedSceneIsHidden)
{
    createDisplayAndExpectSuccess();

    const UInt32 scene = createPublishAndSubscribeScene();
    const UInt32 sceneInterrupted = createPublishAndSubscribeScene();

    mapScene(scene);
    mapScene(sceneInterrupted);

    showAndInitiateInterruptedRendering(scene, sceneInterrupted);
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(sceneInterrupted);
    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(scene);
    unmapScene(scene);
    unmapScene(sceneInterrupted);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, resetsInterruptedRenderingIfAnotherSceneIsHidden)
{
    createDisplayAndExpectSuccess();

    const UInt32 scene = createPublishAndSubscribeScene();
    const UInt32 sceneInterrupted = createPublishAndSubscribeScene();

    mapScene(scene);
    mapScene(sceneInterrupted);

    showAndInitiateInterruptedRendering(scene, sceneInterrupted);
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(scene);
    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(sceneInterrupted);
    unmapScene(scene);
    unmapScene(sceneInterrupted);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, resetsInterruptedRenderingIfAnotherSceneIsShown)
{
    createDisplayAndExpectSuccess();

    const UInt32 scene = createPublishAndSubscribeScene();
    const UInt32 sceneInterrupted = createPublishAndSubscribeScene();
    const UInt32 sceneToShow = createPublishAndSubscribeScene();

    mapScene(scene);
    mapScene(sceneInterrupted);
    mapScene(sceneToShow);

    showAndInitiateInterruptedRendering(scene, sceneInterrupted);
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    showScene(sceneToShow);
    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(sceneToShow);
    hideScene(scene);
    hideScene(sceneInterrupted);
    unmapScene(scene);
    unmapScene(sceneInterrupted);
    unmapScene(sceneToShow);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, doesNotResetInterruptedRenderingIfAnotherSceneIsMapped)
{
    createDisplayAndExpectSuccess();

    const UInt32 scene = createPublishAndSubscribeScene();
    const UInt32 sceneInterrupted = createPublishAndSubscribeScene();
    const UInt32 sceneToMap = createPublishAndSubscribeScene();

    mapScene(scene);
    mapScene(sceneInterrupted);

    showAndInitiateInterruptedRendering(scene, sceneInterrupted);
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    mapScene(sceneToMap);
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(scene);
    hideScene(sceneInterrupted);
    unmapScene(scene);
    unmapScene(sceneInterrupted);
    unmapScene(sceneToMap);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, doesNotResetInterruptedRenderingIfAnotherSceneIsUnmapped)
{
    createDisplayAndExpectSuccess();

    const UInt32 scene = createPublishAndSubscribeScene();
    const UInt32 sceneInterrupted = createPublishAndSubscribeScene();
    const UInt32 sceneToUnmap = createPublishAndSubscribeScene();

    mapScene(scene);
    mapScene(sceneInterrupted);
    mapScene(sceneToUnmap);

    showAndInitiateInterruptedRendering(scene, sceneInterrupted);
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    unmapScene(sceneToUnmap);
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(scene);
    hideScene(sceneInterrupted);
    unmapScene(scene);
    unmapScene(sceneInterrupted);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, resetsInterruptedRenderingIfInterruptedSceneAssignedToFramebuffer)
{
    createDisplayAndExpectSuccess();

    const UInt32 scene = createPublishAndSubscribeScene();
    const UInt32 sceneInterrupted = createPublishAndSubscribeScene();

    mapScene(scene);
    mapScene(sceneInterrupted);

    showAndInitiateInterruptedRendering(scene, sceneInterrupted);
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    EXPECT_TRUE(assignSceneToDisplayBuffer(sceneInterrupted));
    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(scene);
    hideScene(sceneInterrupted);
    unmapScene(scene);
    unmapScene(sceneInterrupted);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, resetsInterruptedRenderingIfAnotherSceneAssignedToFramebuffer)
{
    createDisplayAndExpectSuccess();

    const UInt32 scene = createPublishAndSubscribeScene();
    const UInt32 sceneInterrupted = createPublishAndSubscribeScene();
    const UInt32 sceneToAssign = createPublishAndSubscribeScene();

    mapScene(scene);
    mapScene(sceneInterrupted);
    mapScene(sceneToAssign);

    const OffscreenBufferHandle buffer(1u);
    expectOffscreenBufferUploaded(buffer, DeviceResourceHandle(321u));
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);
    EXPECT_TRUE(assignSceneToDisplayBuffer(sceneToAssign, buffer));

    showAndInitiateInterruptedRendering(scene, sceneInterrupted);
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    EXPECT_TRUE(assignSceneToDisplayBuffer(sceneToAssign));
    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(scene);
    hideScene(sceneInterrupted);
    unmapScene(scene);
    unmapScene(sceneInterrupted);
    unmapScene(sceneToAssign);

    expectOffscreenBufferDeleted(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferDestroyRequest(buffer));

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, resetsInterruptedRenderingIfAnotherSceneAssignedToOffscreenBuffer)
{
    createDisplayAndExpectSuccess();

    const UInt32 scene = createPublishAndSubscribeScene();
    const UInt32 sceneInterrupted = createPublishAndSubscribeScene();

    mapScene(scene);
    mapScene(sceneInterrupted);

    const auto buffer = showAndInitiateInterruptedRendering(scene, sceneInterrupted);
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    EXPECT_TRUE(assignSceneToDisplayBuffer(scene, buffer));
    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(scene);
    hideScene(sceneInterrupted);
    unmapScene(scene);
    unmapScene(sceneInterrupted);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, resetsInterruptedRenderingIfAnotherSceneLinkedToOffscreenBuffer)
{
    createDisplayAndExpectSuccess();

    const UInt32 scene = createPublishAndSubscribeScene();
    const UInt32 sceneInterrupted = createPublishAndSubscribeScene();

    mapScene(scene);
    mapScene(sceneInterrupted);

    const DataSlotId texConsumer = createTextureConsumer(scene);

    const auto buffer = showAndInitiateInterruptedRendering(scene, sceneInterrupted);
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    createBufferLink(buffer, getSceneId(scene), texConsumer);
    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(scene);
    hideScene(sceneInterrupted);
    unmapScene(scene);
    unmapScene(sceneInterrupted);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, resetsInterruptedRenderingIfAnotherSceneUnlinkedFromOffscreenBuffer)
{
    createDisplayAndExpectSuccess();

    const UInt32 scene = createPublishAndSubscribeScene();
    const UInt32 sceneInterrupted = createPublishAndSubscribeScene();
    const UInt32 sceneToUnlink = createPublishAndSubscribeScene();

    mapScene(scene);
    mapScene(sceneInterrupted);
    mapScene(sceneToUnlink);

    const OffscreenBufferHandle buffer(1u);
    expectOffscreenBufferUploaded(buffer, DeviceResourceHandle(321u));
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);
    const DataSlotId texConsumer = createTextureConsumer(sceneToUnlink);
    createBufferLink(buffer, getSceneId(sceneToUnlink), texConsumer);

    showAndInitiateInterruptedRendering(scene, sceneInterrupted);
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    unlinkConsumer(getSceneId(sceneToUnlink), texConsumer);
    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(scene);
    hideScene(sceneInterrupted);
    unmapScene(scene);
    unmapScene(sceneInterrupted);
    unmapScene(sceneToUnlink);

    expectOffscreenBufferDeleted(buffer);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferDestroyRequest(buffer));

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, resetsInterruptedRenderingIfTransformationLinked)
{
    createDisplayAndExpectSuccess();

    const UInt32 scene = createPublishAndSubscribeScene();
    const UInt32 sceneInterrupted = createPublishAndSubscribeScene();

    mapScene(scene);
    mapScene(sceneInterrupted);

    // provider/consumer order does not matter here
    const auto providerConsumerId = createTransformationSlots(nullptr, scene, sceneInterrupted);

    showAndInitiateInterruptedRendering(scene, sceneInterrupted);
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    linkProviderToConsumer(getSceneId(scene), providerConsumerId.first, getSceneId(sceneInterrupted), providerConsumerId.second);
    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(scene);
    hideScene(sceneInterrupted);
    unmapScene(scene);
    unmapScene(sceneInterrupted);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, resetsInterruptedRenderingIfTransformationUnlinked)
{
    createDisplayAndExpectSuccess();

    const UInt32 scene = createPublishAndSubscribeScene();
    const UInt32 sceneInterrupted = createPublishAndSubscribeScene();

    mapScene(scene);
    mapScene(sceneInterrupted);

    // provider/consumer order does not matter here
    const auto providerConsumerId = createTransformationSlots(nullptr, scene, sceneInterrupted);
    linkProviderToConsumer(getSceneId(scene), providerConsumerId.first, getSceneId(sceneInterrupted), providerConsumerId.second);

    showAndInitiateInterruptedRendering(scene, sceneInterrupted);
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    unlinkConsumer(getSceneId(sceneInterrupted), providerConsumerId.second);
    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(scene);
    hideScene(sceneInterrupted);
    unmapScene(scene);
    unmapScene(sceneInterrupted);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, resetsInterruptedRenderingIfDataLinked)
{
    createDisplayAndExpectSuccess();

    const UInt32 scene = createPublishAndSubscribeScene();
    const UInt32 sceneInterrupted = createPublishAndSubscribeScene();

    mapScene(scene);
    mapScene(sceneInterrupted);

    // provider/consumer order does not matter here
    DataInstanceHandle dataRef;
    const auto providerConsumerId = createDataSlots(dataRef, 1.f, nullptr, scene, sceneInterrupted);

    showAndInitiateInterruptedRendering(scene, sceneInterrupted);
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    linkProviderToConsumer(getSceneId(scene), providerConsumerId.first, getSceneId(sceneInterrupted), providerConsumerId.second);
    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(scene);
    hideScene(sceneInterrupted);
    unmapScene(scene);
    unmapScene(sceneInterrupted);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, resetsInterruptedRenderingIfDataUnlinked)
{
    createDisplayAndExpectSuccess();

    const UInt32 scene = createPublishAndSubscribeScene();
    const UInt32 sceneInterrupted = createPublishAndSubscribeScene();

    mapScene(scene);
    mapScene(sceneInterrupted);

    // provider/consumer order does not matter here
    DataInstanceHandle dataRef;
    const auto providerConsumerId = createDataSlots(dataRef, 1.f, nullptr, scene, sceneInterrupted);
    linkProviderToConsumer(getSceneId(scene), providerConsumerId.first, getSceneId(sceneInterrupted), providerConsumerId.second);

    showAndInitiateInterruptedRendering(scene, sceneInterrupted);
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    unlinkConsumer(getSceneId(sceneInterrupted), providerConsumerId.second);
    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(scene);
    hideScene(sceneInterrupted);
    unmapScene(scene);
    unmapScene(sceneInterrupted);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, resetsInterruptedRenderingIfTexturesLinked)
{
    createDisplayAndExpectSuccess();

    const UInt32 scene = createPublishAndSubscribeScene();
    const UInt32 sceneInterrupted = createPublishAndSubscribeScene();

    mapScene(scene);
    mapScene(sceneInterrupted);

    // provider/consumer order does not matter here
    expectResourcesReferencedAndProvided({ MockResourceHash::TextureHash }, scene);
    const auto providerConsumerId = createTextureSlots(nullptr, scene, sceneInterrupted);

    showAndInitiateInterruptedRendering(scene, sceneInterrupted);
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    linkProviderToConsumer(getSceneId(scene), providerConsumerId.first, getSceneId(sceneInterrupted), providerConsumerId.second);
    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(scene);
    hideScene(sceneInterrupted);
    unmapScene(scene);
    unmapScene(sceneInterrupted);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, resetsInterruptedRenderingIfTexturesUnlinked)
{
    createDisplayAndExpectSuccess();

    const UInt32 scene = createPublishAndSubscribeScene();
    const UInt32 sceneInterrupted = createPublishAndSubscribeScene();

    mapScene(scene);
    mapScene(sceneInterrupted);

    // provider/consumer order does not matter here
    expectResourcesReferencedAndProvided({ MockResourceHash::TextureHash }, scene);
    const auto providerConsumerId = createTextureSlots(nullptr, scene, sceneInterrupted);
    linkProviderToConsumer(getSceneId(scene), providerConsumerId.first, getSceneId(sceneInterrupted), providerConsumerId.second);

    showAndInitiateInterruptedRendering(scene, sceneInterrupted);
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    unlinkConsumer(getSceneId(sceneInterrupted), providerConsumerId.second);
    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(scene);
    hideScene(sceneInterrupted);
    unmapScene(scene);
    unmapScene(sceneInterrupted);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, resetsInterruptedRenderingIfOffscreenBufferCreated)
{
    createDisplayAndExpectSuccess();

    const UInt32 scene = createPublishAndSubscribeScene();
    const UInt32 sceneInterrupted = createPublishAndSubscribeScene();

    mapScene(scene);
    mapScene(sceneInterrupted);

    showAndInitiateInterruptedRendering(scene, sceneInterrupted);
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    const OffscreenBufferHandle buffer2(1u);
    expectOffscreenBufferUploaded(buffer2, DeviceResourceHandle(321u));
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer2, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);

    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(scene);
    hideScene(sceneInterrupted);
    unmapScene(scene);
    unmapScene(sceneInterrupted);

    expectOffscreenBufferDeleted(buffer2);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferDestroyRequest(buffer2));

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, resetsInterruptedRenderingIfOffscreenBufferDeleted)
{
    createDisplayAndExpectSuccess();

    constexpr OffscreenBufferHandle buffer(1u);
    constexpr DeviceResourceHandle bufferDeviceHandle{ 321u };
    expectOffscreenBufferUploaded(buffer, bufferDeviceHandle);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer));
    expectEvent(ERendererEventType::OffscreenBufferCreated);

    const UInt32 scene = createPublishAndSubscribeScene();
    const UInt32 sceneInterrupted = createPublishAndSubscribeScene();

    mapScene(scene);
    mapScene(sceneInterrupted);

    showAndInitiateInterruptedRendering(scene, sceneInterrupted);
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    expectOffscreenBufferDeleted(buffer, bufferDeviceHandle);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferDestroyRequest(buffer));

    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(scene);
    hideScene(sceneInterrupted);
    unmapScene(scene);
    unmapScene(sceneInterrupted);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, blocksFlushesForSceneAssignedToInterruptibleOBWhenThereIsInterruption)
{
    createDisplayAndExpectSuccess();
    const UInt32 scene = createPublishAndSubscribeScene();
    const UInt32 sceneInterrupted = createPublishAndSubscribeScene();

    mapScene(scene);
    mapScene(sceneInterrupted);

    performFlush(sceneInterrupted);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene(sceneInterrupted));

    showAndInitiateInterruptedRendering(scene, sceneInterrupted);
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    performFlush(sceneInterrupted);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene(sceneInterrupted));

    performFlush(sceneInterrupted);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene(sceneInterrupted));

    renderer.resetRenderInterruptState();
    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());

    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene(sceneInterrupted));

    hideScene(scene);
    hideScene(sceneInterrupted);
    unmapScene(scene);
    unmapScene(sceneInterrupted);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, doesNotBlockFlushesForSceneAssignedToNormalOBWhenThereIsInterruption)
{
    createDisplayAndExpectSuccess();
    const UInt32 scene = createPublishAndSubscribeScene();
    const UInt32 sceneInterrupted = createPublishAndSubscribeScene();

    mapScene(scene);
    mapScene(sceneInterrupted);

    performFlush(sceneInterrupted);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene(scene));

    showAndInitiateInterruptedRendering(scene, sceneInterrupted);
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    performFlush(scene);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene(sceneInterrupted));

    performFlush(scene);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene(sceneInterrupted));

    renderer.resetRenderInterruptState();
    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());

    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene(scene));

    hideScene(scene);
    hideScene(sceneInterrupted);
    unmapScene(scene);
    unmapScene(sceneInterrupted);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, resetsInterruptedRenderingIfMaximumNumberOfPendingFlushesReached)
{
    createDisplayAndExpectSuccess();
    const UInt32 scene = createPublishAndSubscribeScene();
    const UInt32 sceneInterrupted = createPublishAndSubscribeScene();

    createRenderable(sceneInterrupted);
    setRenderableResources(sceneInterrupted);

    expectResourcesReferencedAndProvided_altogether({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash }, sceneInterrupted);
    mapScene(scene);
    mapScene(sceneInterrupted);

    expectVertexArrayUploaded(sceneInterrupted);
    showAndInitiateInterruptedRendering(scene, sceneInterrupted);

    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());
    // add blocking flush so that upcoming flushes are queuing up
    expectResourcesReferencedAndProvided({ MockResourceHash::IndexArrayHash2 }, sceneInterrupted);
    setRenderableResources(sceneInterrupted, MockResourceHash::IndexArrayHash2);
    reportResourceAs(MockResourceHash::IndexArrayHash2, EResourceStatus::Provided);
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getResourceDeviceHandle(MockResourceHash::IndexArrayHash2)).Times(AnyNumber()).WillRepeatedly(Return(DeviceResourceHandle::Invalid()));
    update();

    // flushes are blocked due to unresolved resource
    const SceneVersionTag pendingFlushTag(124u);
    performFlush(sceneInterrupted, pendingFlushTag);
    update();
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    // will force apply and log blocking resources
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getResourceType(_)).Times(AnyNumber());
    expectResourcesUnreferenced({ MockResourceHash::IndexArrayHash }, sceneInterrupted);
    expectVertexArrayUnloaded(sceneInterrupted);
    for (UInt i = 0u; i < ForceApplyFlushesLimit + 1u; ++i)
    {
        performFlushWithCreateNodeAction(sceneInterrupted);
        update();
    }

    // after maximum of pending flushes was reached the flushes were applied regardless of missing resource
    expectSceneEvent(ERendererEventType::SceneFlushed);
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene(sceneInterrupted));
    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(scene);
    hideScene(sceneInterrupted);
    unmapScene(scene);
    unmapScene(sceneInterrupted);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, reportsExpirationForSubscribedSceneButWithNoFurtherFlushes)
{
    const UInt32 scene = createPublishAndSubscribeScene(false);
    // initial flush and enable monitoring
    performFlushWithExpiration(scene, 1000u + 1u);
    expectInternalSceneStateEvent(ERendererEventType::SceneSubscribed);

    update();
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringEnabled);

    // no further flush, no expiration updates
    for (UInt32 i = 0u; i < 10u; ++i)
    {
        update();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }

    expectSceneEvent(ERendererEventType::SceneExpired);
}

TEST_F(ARendererSceneUpdater, doesNotReportExpirationForNotShownSceneBeingFlushedRegularly)
{
    const UInt32 scene = createPublishAndSubscribeScene();
    // enable monitoring
    performFlushWithExpiration(scene, 2000);
    update();
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringEnabled);

    for (UInt32 i = 0u; i < 10u; ++i)
    {
        performFlushWithExpiration(scene, 1000u + i + 2u);
        update();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
        expectNoEvent();
    }
}

TEST_F(ARendererSceneUpdater, reportsExpirationForNotShownSceneNotBeingFlushed)
{
    const UInt32 scene = createPublishAndSubscribeScene();
    // flush once only to set limit
    performFlushWithExpiration(scene, 1000u + 2u);
    update();
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringEnabled);


    for (UInt32 i = 0u; i < 10u; ++i)
    {
        update();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }

    expectSceneEvent(ERendererEventType::SceneExpired);
}

TEST_F(ARendererSceneUpdater, reportsRecoveryAfterExpiredForNotShownScene)
{
    const UInt32 scene = createPublishAndSubscribeScene();
    // flush once only to set limit
    performFlushWithExpiration(scene, 1000u + 2u);
    update();
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringEnabled);

    for (UInt32 i = 0u; i < 10u; ++i)
    {
        update();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }
    // expect exactly one event
    expectSceneEvent(ERendererEventType::SceneExpired);

    for (UInt32 i = 10u; i < 20u; ++i)
    {
        performFlushWithExpiration(scene, 1000u + i + 2u);
        update();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }
    // expect exactly one event
    expectSceneEvent(ERendererEventType::SceneRecoveredFromExpiration);
}

TEST_F(ARendererSceneUpdater, doesNotReportExpirationForSceneBeingFlushedAndRenderedRegularly)
{
    createDisplayAndExpectSuccess();
    const UInt32 scene = createPublishAndSubscribeScene();
    mapScene();
    showScene();

    // enable monitoring
    performFlushWithExpiration(scene, 2000);
    update();
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringEnabled);

    for (UInt32 i = 0u; i < 10u; ++i)
    {
        performFlushWithExpiration(scene, 1000u + i + 2u);
        update();
        expirationMonitor.onRendered(getSceneId());
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
        expectNoEvent();
    }

    hideScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, reportsExpirationForSceneBeingFlushedButNotRenderedRegularly)
{
    createDisplayAndExpectSuccess();
    const UInt32 scene = createPublishAndSubscribeScene();
    mapScene();
    showScene();

    // scene must be rendered at least once with valid expiration, only then it can expire
    performFlushWithExpiration(scene, 1000u + 2u);
    update();
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringEnabled);

    const NodeHandle nodeHandle(3u);
    const TransformHandle transform(2u);
    IScene& iscene = *stagingScene[scene];
    SceneAllocateHelper sceneAllocator(iscene);
    sceneAllocator.allocateNode(0u, nodeHandle);
    sceneAllocator.allocateTransform(nodeHandle, transform);

    for (UInt32 i = 0u; i < 10u; ++i)
    {
        iscene.setTranslation(transform, {}); // so that flush is not 'empty' - modifies scene
        performFlushWithExpiration(scene, 1000u + i + 2u);
        update();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }
    expectSceneEvent(ERendererEventType::SceneExpired);

    hideScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, doesNotReportExpirationForSceneBeingFlushedButNotRenderedBecauseItIsHidden)
{
    createDisplayAndExpectSuccess();
    const UInt32 scene = createPublishAndSubscribeScene();
    mapScene();
    showScene();

    // scene must be rendered at least once with valid expiration, only then it can expire
    performFlushWithExpiration(scene, 1000u + 2u);
    update();
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringEnabled);

    hideScene();

    const NodeHandle nodeHandle(3u);
    const TransformHandle transform(2u);
    IScene& iscene = *stagingScene[scene];
    SceneAllocateHelper sceneAllocator(iscene);
    sceneAllocator.allocateNode(0u, nodeHandle);
    sceneAllocator.allocateTransform(nodeHandle, transform);

    for (UInt32 i = 0u; i < 10u; ++i)
    {
        iscene.setTranslation(transform, {}); // so that flush is not 'empty' - modifies scene
        performFlushWithExpiration(scene, 1000u + i + 2u);
        update();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }
    expectNoEvent();

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, reportsExpirationForSceneNotBeingFlushedOnlyRendered)
{
    const UInt32 scene = createPublishAndSubscribeScene();
    // flush once only to set limit
    performFlushWithExpiration(scene, 1000u + 2u);
    update();
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringEnabled);

    for (UInt32 i = 0u; i < 10u; ++i)
    {
        update();
        expirationMonitor.onRendered(getSceneId());
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }

    expectSceneEvent(ERendererEventType::SceneExpired);
}

TEST_F(ARendererSceneUpdater, reportsRecoveryAfterExpirationForSceneNotBeingRenderedRegularly_byRenderingRegularlyAgain)
{
    createDisplayAndExpectSuccess();
    const UInt32 scene = createPublishAndSubscribeScene();
    mapScene();
    showScene();

    // scene must be rendered at least once with valid expiration, only then it can expire
    performFlushWithExpiration(scene, 1000u + 2u);
    update();
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringEnabled);

    const NodeHandle nodeHandle(3u);
    const TransformHandle transform(2u);
    IScene& iscene = *stagingScene[scene];
    SceneAllocateHelper sceneAllocator(iscene);
    sceneAllocator.allocateNode(0u, nodeHandle);
    sceneAllocator.allocateTransform(nodeHandle, transform);

    for (UInt32 i = 0u; i < ForceApplyFlushesLimit / 2u; ++i)
    {
        iscene.setTranslation(transform, {}); // so that flush is not 'empty' - modifies scene
        performFlushWithExpiration(scene, 1000u + i + 2u);
        update();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }
    expectSceneEvent(ERendererEventType::SceneExpired);

    for (UInt32 i = 0u; i < ForceApplyFlushesLimit / 2u; ++i)
    {
        performFlushWithExpiration(scene, 1000u + i + 2u);
        update();
        expirationMonitor.onRendered(getSceneId());
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }
    expectSceneEvent(ERendererEventType::SceneRecoveredFromExpiration);

    hideScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, reportsRecoveryAfterExpirationForSceneNotBeingRenderedRegularly_byHidingSceneAndKeepingRegularFlushes)
{
    createDisplayAndExpectSuccess();
    const UInt32 scene = createPublishAndSubscribeScene();
    mapScene();
    showScene();

    // scene must be rendered at least once with valid expiration, only then it can expire
    performFlushWithExpiration(scene, 1000u + 2u);
    update();
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringEnabled);

    const NodeHandle nodeHandle(3u);
    const TransformHandle transform(2u);
    IScene& iscene = *stagingScene[scene];
    SceneAllocateHelper sceneAllocator(iscene);
    sceneAllocator.allocateNode(0u, nodeHandle);
    sceneAllocator.allocateTransform(nodeHandle, transform);

    for (UInt32 i = 0u; i < ForceApplyFlushesLimit / 2u; ++i)
    {
        iscene.setTranslation(transform, {}); // so that flush is not 'empty' - modifies scene
        performFlushWithExpiration(scene, 1000u + i + 2u);
        update();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }
    expectSceneEvent(ERendererEventType::SceneExpired);

    hideScene();

    for (UInt32 i = 0u; i < ForceApplyFlushesLimit / 2u; ++i)
    {
        iscene.setTranslation(transform, {}); // so that flush is not 'empty' - modifies scene
        performFlushWithExpiration(scene, 1000u + i + 2u);
        update();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }
    expectSceneEvent(ERendererEventType::SceneRecoveredFromExpiration);

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, doesNotReportExpirationForSceneBeingFlushedRegularlyButSkippedRenderingDueToEmptyFlushes)
{
    createDisplayAndExpectSuccess();
    const UInt32 scene = createPublishAndSubscribeScene();
    mapScene();
    showScene();

    // enable monitoring
    performFlushWithExpiration(scene, 2000);
    update();
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringEnabled);

    for (UInt32 i = 0u; i < ForceApplyFlushesLimit / 2u; ++i)
    {
        performFlushWithExpiration(scene, 1000u + i + 2u);
        update();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
        expectNoEvent();
    }

    hideScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, reportsExpirationForNotShownSceneBeingFlushedButBlockedByMissingResource)
{
    createDisplayAndExpectSuccess();
    const UInt32 scene = createPublishAndSubscribeScene();
    mapScene();

    // flush with expiration info to initiate monitoring
    performFlushWithExpiration(scene, 1000u + 2u);
    update();
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringEnabled);

    // blocking flush
    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash });
    createRenderable(scene);
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Provided);
    update();

    for (UInt32 i = 0u; i < ForceApplyFlushesLimit / 2u; ++i)
    {
        performFlushWithExpiration(scene, 1000u + i + 2u); // blocked due to missing resource
        update();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }
    expectSceneEvent(ERendererEventType::SceneExpired);

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, reportsExpirationSceneBeingFlushedAndRenderedButBlockedByMissingResource)
{
    createDisplayAndExpectSuccess();
    const UInt32 scene = createPublishAndSubscribeScene();
    mapScene();
    showScene();

    // flush with expiration info to initiate monitoring
    performFlushWithExpiration(scene, 1000u + 2u);
    update();
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringEnabled);

    // blocking flush
    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash });
    createRenderable(scene);
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Provided);
    update();

    for (UInt32 i = 0u; i < ForceApplyFlushesLimit / 2; ++i)
    {
        performFlushWithExpiration(scene, 1000u + i + 2u); // blocked due to missing resource
        update();
        expirationMonitor.onRendered(getSceneId());
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }
    expectSceneEvent(ERendererEventType::SceneExpired);

    hideScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, reportsRecoveryAfterExpirationForSceneBeingFlushedAndRenderedButBlockedByMissingResource)
{
    createDisplayAndExpectSuccess();
    const UInt32 scene = createPublishAndSubscribeScene();
    mapScene();
    showScene();

    uint32_t expTS = 1000u + 2u;

    // flush with expiration info to initiate monitoring
    performFlushWithExpiration(scene, expTS);
    update();
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringEnabled);

    // blocking flush
    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash });
    createRenderableNoFlush(scene);
    performFlushWithExpiration(scene, expTS);
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Provided);
    expectVertexArrayUploaded();
    update();

    for (UInt32 i = 0u; i < ForceApplyFlushesLimit / 2u; ++i)
    {
        performFlushWithExpiration(scene, expTS++); // blocked due to missing resource
        update();
        expirationMonitor.onRendered(getSceneId());
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }
    expectSceneEvent(ERendererEventType::SceneExpired);

    // simulate upload
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Uploaded);
    update();

    for (UInt32 i = ForceApplyFlushesLimit / 2u; i < ForceApplyFlushesLimit; ++i)
    {
        performFlushWithExpiration(scene, expTS++); // not blocked anymore
        update();
        expirationMonitor.onRendered(getSceneId());
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }
    expectSceneEvent(ERendererEventType::SceneRecoveredFromExpiration);

    hideScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canDisableExpirationChecking_withEmptyFlush)
{
    const UInt32 scene = createPublishAndSubscribeScene();
    update();

    // flush once only to set limit
    const UInt32 initialTS = 1000u;
    performFlushWithExpiration(scene, initialTS);
    update();
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringEnabled);

    for (UInt32 i = 1u; i < 10u; ++i)
    {
        update();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(i)));
    }
    expectNoEvent();

    // disable expiration
    performFlushWithExpiration(scene, 0u);
    update();
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringDisabled);

    for (UInt32 i = 0u; i < 10u; ++i)
    {
        update();
        // check with TS after initial TS
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(initialTS + i)));
    }
    // expect no expiration as last flush disabled expiration
    expectNoEvent();
}

TEST_F(ARendererSceneUpdater, canDisableExpirationChecking_withNonEmptyFlush)
{
    const UInt32 scene = createPublishAndSubscribeScene();
    update();

    // flush once only to set limit
    const UInt32 initialTS = 1000u;
    performFlushWithExpiration(scene, initialTS);
    update();
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringEnabled);

    for (UInt32 i = 1u; i < 10u; ++i)
    {
        update();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(i)));
    }
    expectNoEvent();

    // disable expiration together with some scene changes
    const NodeHandle nodeHandle(3u);
    IScene& iscene = *stagingScene[scene];
    SceneAllocateHelper sceneAllocator(iscene);
    sceneAllocator.allocateNode(0u, nodeHandle);
    performFlushWithExpiration(scene, 0u);
    update();
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringDisabled);

    for (UInt32 i = 0u; i < 10u; ++i)
    {
        update();
        // check with TS after initial TS
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(initialTS + i)));
    }
    // expect no expiration as last flush disabled expiration
    expectNoEvent();
}

TEST_F(ARendererSceneUpdater, canDisableExpirationChecking_withEmptyFlush_rendered)
{
    createDisplayAndExpectSuccess();
    const UInt32 scene = createPublishAndSubscribeScene();
    mapScene(scene);
    showScene(scene);

    update();
    doRenderLoop();

    // flush once only to set limit
    const UInt32 initialTS = 1000u;
    performFlushWithExpiration(scene, initialTS);
    update();
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringEnabled);

    for (UInt32 i = 1u; i < 10u; ++i)
    {
        update();
        doRenderLoop();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(i)));
    }
    expectNoEvent();

    // disable expiration
    performFlushWithExpiration(scene, 0u);
    update();
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringDisabled);

    for (UInt32 i = 0u; i < 10u; ++i)
    {
        update();
        doRenderLoop();
        // check with TS after initial TS
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(initialTS + i)));
    }
    // expect no expiration as last flush disabled expiration
    expectNoEvent();

    hideScene(scene);
    unmapScene(scene);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canDisableExpirationChecking_withNonEmptyFlush_rendered)
{
    createDisplayAndExpectSuccess();
    const UInt32 scene = createPublishAndSubscribeScene();
    mapScene(scene);
    showScene(scene);

    update();
    doRenderLoop();

    // flush once only to set limit
    const UInt32 initialTS = 1000u;
    performFlushWithExpiration(scene, initialTS);
    update();
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringEnabled);

    for (UInt32 i = 1u; i < 10u; ++i)
    {
        update();
        doRenderLoop();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(i)));
    }
    expectNoEvent();

    // disable expiration together with some scene changes
    const NodeHandle nodeHandle(3u);
    IScene& iscene = *stagingScene[scene];
    SceneAllocateHelper sceneAllocator(iscene);
    sceneAllocator.allocateNode(0u, nodeHandle);
    performFlushWithExpiration(scene, 0u);
    update();
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringDisabled);

    for (UInt32 i = 0u; i < 10u; ++i)
    {
        update();
        doRenderLoop();
        // check with TS after initial TS
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(initialTS + i)));
    }
    // expect no expiration as last flush disabled expiration
    expectNoEvent();

    hideScene(scene);
    unmapScene(scene);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canDisableExpirationChecking_withEmptyFlush_hiddenAfterRendered)
{
    createDisplayAndExpectSuccess();
    const UInt32 scene = createPublishAndSubscribeScene();
    mapScene(scene);
    showScene(scene);

    update();
    doRenderLoop();

    // flush once only to set limit
    const UInt32 initialTS = 1000u;
    performFlushWithExpiration(scene, initialTS);
    update();
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringEnabled);

    for (UInt32 i = 1u; i < 10u; ++i)
    {
        update();
        doRenderLoop();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(i)));
    }
    expectNoEvent();

    hideScene(scene);

    // disable expiration
    performFlushWithExpiration(scene, 0u);
    update();
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringDisabled);

    for (UInt32 i = 0u; i < 10u; ++i)
    {
        update();
        doRenderLoop();
        // check with TS after initial TS
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(initialTS + i)));
    }
    // expect no expiration as last flush disabled expiration
    expectNoEvent();

    unmapScene(scene);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canDisableExpirationChecking_withNonEmptyFlush_hiddenAfterRendered)
{
    createDisplayAndExpectSuccess();
    const UInt32 scene = createPublishAndSubscribeScene();
    mapScene(scene);
    showScene(scene);

    update();
    doRenderLoop();

    // flush once only to set limit
    const UInt32 initialTS = 1000u;
    performFlushWithExpiration(scene, initialTS);
    update();
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringEnabled);

    for (UInt32 i = 1u; i < 10u; ++i)
    {
        update();
        doRenderLoop();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(i)));
    }
    expectNoEvent();

    hideScene(scene);

    // disable expiration together with some scene changes
    const NodeHandle nodeHandle(3u);
    IScene& iscene = *stagingScene[scene];
    SceneAllocateHelper sceneAllocator(iscene);
    sceneAllocator.allocateNode(0u, nodeHandle);
    performFlushWithExpiration(scene, 0u);
    update();
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringDisabled);

    for (UInt32 i = 0u; i < 10u; ++i)
    {
        update();
        doRenderLoop();
        // check with TS after initial TS
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(initialTS + i)));
    }
    // expect no expiration as last flush disabled expiration
    expectNoEvent();

    unmapScene(scene);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canDisableExpirationChecking_whileModifyingScene_confidenceTest)
{
    createDisplayAndExpectSuccess();
    const UInt32 scene = createPublishAndSubscribeScene();
    mapScene(scene);
    showScene(scene);

    update();
    doRenderLoop();

    // flush once only to set limit
    const UInt32 initialTS = 1000u;
    performFlushWithExpiration(scene, initialTS);
    update();
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringEnabled);

    for (UInt32 i = 0u; i < 10u; ++i)
    {
        // make change
        const NodeHandle nodeHandle(i);
        IScene& iscene = *stagingScene[scene];
        SceneAllocateHelper sceneAllocator(iscene);
        sceneAllocator.allocateNode(0u, nodeHandle);
        performFlushWithExpiration(scene, initialTS + i);

        update();
        doRenderLoop();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(initialTS + i - 1)));
    }
    expectNoEvent();

    hideScene(scene);

    // disable expiration together with some scene changes
    const NodeHandle nodeHandle(10u);
    IScene& iscene = *stagingScene[scene];
    SceneAllocateHelper sceneAllocator(iscene);
    sceneAllocator.allocateNode(0u, nodeHandle);
    performFlushWithExpiration(scene, 0u);
    update();
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringDisabled);

    for (UInt32 i = 10u; i < 20u; ++i)
    {
        update();
        doRenderLoop();
        // check with TS after initial TS
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(initialTS + i)));
    }
    // expect no expiration as last flush disabled expiration
    expectNoEvent();

    unmapScene(scene);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canDisableExpirationChecking_whenHiddenInBetweenFlushes_confidenceTest)
{
    createDisplayAndExpectSuccess();
    const UInt32 scene = createPublishAndSubscribeScene();
    mapScene(scene);
    showScene(scene);

    update();
    doRenderLoop();

    // flush once only to set limit
    const UInt32 initialTS = 1000u;
    performFlushWithExpiration(scene, initialTS);
    update();
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringEnabled);

    for (UInt32 i = 0u; i < 10u; ++i)
    {
        // make change
        const NodeHandle nodeHandle(i);
        IScene& iscene = *stagingScene[scene];
        SceneAllocateHelper sceneAllocator(iscene);
        sceneAllocator.allocateNode(0u, nodeHandle);
        performFlushWithExpiration(scene, initialTS + i);

        update();
        doRenderLoop();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(initialTS + i - 1)));

        // hide scene in between applying flushes with timestamps
        if (i == 8)
            hideScene(scene);
    }
    expectNoEvent();

    // disable expiration
    performFlushWithExpiration(scene, 0u);
    update();
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringDisabled);

    for (UInt32 i = 10u; i < 20u; ++i)
    {
        update();
        doRenderLoop();
        // check with TS after initial TS
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(initialTS + i)));
    }
    // expect no expiration as last flush disabled expiration
    expectNoEvent();

    unmapScene(scene);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canDisableExpirationChecking_whenHiddenInBetweenEmptyFlushes_confidenceTest)
{
    createDisplayAndExpectSuccess();
    const UInt32 scene = createPublishAndSubscribeScene();
    mapScene(scene);
    showScene(scene);

    update();
    doRenderLoop();

    // flush once only to set limit
    const UInt32 initialTS = 1000u;
    performFlushWithExpiration(scene, initialTS);
    update();
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringEnabled);

    for (UInt32 i = 0u; i < 10u; ++i)
    {
        performFlushWithExpiration(scene, initialTS + i);

        update();
        doRenderLoop();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(initialTS + i - 1)));

        // hide scene in between applying flushes with timestamps
        if (i == 8)
            hideScene(scene);
    }
    expectNoEvent();

    // disable expiration together with some scene changes
    const NodeHandle nodeHandle(10u);
    IScene& iscene = *stagingScene[scene];
    SceneAllocateHelper sceneAllocator(iscene);
    sceneAllocator.allocateNode(0u, nodeHandle);
    performFlushWithExpiration(scene, 0u);
    update();
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringDisabled);

    for (UInt32 i = 10u; i < 20u; ++i)
    {
        update();
        doRenderLoop();
        // check with TS after initial TS
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(initialTS + i)));
    }
    // expect no expiration as last flush disabled expiration
    expectNoEvent();

    unmapScene(scene);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canDisableExpirationCheckingWhileAlreadyExpired)
{
    const UInt32 scene = createPublishAndSubscribeScene();

    for (UInt32 i = 0u; i < 5u; ++i)
    {
        performFlushWithExpiration(scene, 1000u + 2u); // will expire
        update();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }
    expectSceneEvents({ ERendererEventType::SceneExpirationMonitoringEnabled, ERendererEventType::SceneExpired });

    // disable expiration
    performFlushWithExpiration(scene, 0u);

    for (UInt32 i = 0u; i < 5u; ++i)
    {
        update();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringDisabled);
}

TEST_F(ARendererSceneUpdater, canDisableExpirationCheckingWhileAlreadyExpired_rendered)
{
    createDisplayAndExpectSuccess();
    const UInt32 scene = createPublishAndSubscribeScene();
    mapScene(scene);
    showScene(scene);

    for (UInt32 i = 0u; i < 5u; ++i)
    {
        performFlushWithExpiration(scene, 1000u + 2u); // will expire
        update();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }
    expectSceneEvents({ ERendererEventType::SceneExpirationMonitoringEnabled, ERendererEventType::SceneExpired });

    // disable expiration
    performFlushWithExpiration(scene, 0u);

    for (UInt32 i = 0u; i < 5u; ++i)
    {
        update();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringDisabled);

    hideScene(scene);
    unmapScene(scene);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canDisableExpirationCheckingWhileAlreadyExpired_nonEmptyFlush)
{
    const UInt32 scene = createPublishAndSubscribeScene();

    for (UInt32 i = 0u; i < 5u; ++i)
    {
        performFlushWithExpiration(scene, 1000u + 2u); // will expire
        update();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }
    expectSceneEvents({ ERendererEventType::SceneExpirationMonitoringEnabled, ERendererEventType::SceneExpired });

    // disable expiration together with some scene changes
    const NodeHandle nodeHandle(10u);
    IScene& iscene = *stagingScene[scene];
    SceneAllocateHelper sceneAllocator(iscene);
    sceneAllocator.allocateNode(0u, nodeHandle);
    performFlushWithExpiration(scene, 0u);

    for (UInt32 i = 0u; i < 5u; ++i)
    {
        update();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringDisabled);
}

TEST_F(ARendererSceneUpdater, canDisableExpirationCheckingWhileAlreadyExpired_rendered_nonEmptyFlush)
{
    createDisplayAndExpectSuccess();
    const UInt32 scene = createPublishAndSubscribeScene();
    mapScene(scene);
    showScene(scene);

    for (UInt32 i = 0u; i < 5u; ++i)
    {
        performFlushWithExpiration(scene, 1000u + 2u); // will expire
        update();
        doRenderLoop();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }
    expectSceneEvents({ ERendererEventType::SceneExpirationMonitoringEnabled, ERendererEventType::SceneExpired });

    // disable expiration together with some scene changes
    const NodeHandle nodeHandle(10u);
    IScene& iscene = *stagingScene[scene];
    SceneAllocateHelper sceneAllocator(iscene);
    sceneAllocator.allocateNode(0u, nodeHandle);
    performFlushWithExpiration(scene, 0u);

    for (UInt32 i = 0u; i < 5u; ++i)
    {
        update();
        doRenderLoop();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }
    expectSceneEvent(ERendererEventType::SceneExpirationMonitoringDisabled);

    hideScene(scene);
    unmapScene(scene);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, ignoresPickEventForUnknownScene)
{
    rendererSceneUpdater->handlePickEvent(SceneId{ 123u }, { 0, 0 });
    expectNoEvent();
}

TEST_F(ARendererSceneUpdater, reportsNoPickedObjectsForSceneWithNoPickableObjects)
{
    createPublishAndSubscribeScene();
    rendererSceneUpdater->handlePickEvent(getSceneId(), { 0, 0 });
    expectNoEvent();
}

TEST_F(ARendererSceneUpdater, reportsPickedObjects)
{
    DisplayConfig config;
    config.setDesiredWindowWidth(1280u);
    config.setDesiredWindowHeight(480u);
    createDisplayAndExpectSuccess(config);
    const auto sceneIdx = createPublishAndSubscribeScene();
    mapScene();

    // create scene with 2 pickable triangles around origin
    IScene& iscene = *stagingScene[sceneIdx];
    SceneAllocateHelper sceneAllocator(iscene);
    const NodeHandle nodeHandle(0u);
    sceneAllocator.allocateNode(0u, nodeHandle);
    const std::array<float, 9> geomData{ -1.f, 0.f, -0.5f, 0.f, 1.f, -0.5f, 0.f, 0.f, -0.5f };
    const auto geomHandle = sceneAllocator.allocateDataBuffer(EDataBufferType::VertexBuffer, EDataType::Vector3F, UInt32(geomData.size() * sizeof(float)));
    iscene.updateDataBuffer(geomHandle, 0, UInt32(geomData.size() * sizeof(float)), reinterpret_cast<const Byte*>(geomData.data()));

    const auto dataLayout = sceneAllocator.allocateDataLayout({ DataFieldInfo{EDataType::DataReference}, DataFieldInfo{EDataType::DataReference}, DataFieldInfo{EDataType::DataReference}, DataFieldInfo{EDataType::DataReference} }, {});
    const auto dataInstance = sceneAllocator.allocateDataInstance(dataLayout);
    const auto vpDataRefLayout = sceneAllocator.allocateDataLayout({ DataFieldInfo{EDataType::Vector2I} }, {});
    const auto vpOffsetInstance = sceneAllocator.allocateDataInstance(vpDataRefLayout);
    const auto vpSizeInstance = sceneAllocator.allocateDataInstance(vpDataRefLayout);
    const auto frustumPlanesLayout = sceneAllocator.allocateDataLayout({ DataFieldInfo{EDataType::Vector4F} }, {});
    const auto frustumPlanes = sceneAllocator.allocateDataInstance(frustumPlanesLayout);
    const auto frustumNearFarLayout = sceneAllocator.allocateDataLayout({ DataFieldInfo{EDataType::Vector2F} }, {});
    const auto frustumNearFar = sceneAllocator.allocateDataInstance(frustumNearFarLayout);
    iscene.setDataReference(dataInstance, Camera::ViewportOffsetField, vpOffsetInstance);
    iscene.setDataReference(dataInstance, Camera::ViewportSizeField, vpSizeInstance);
    iscene.setDataReference(dataInstance, Camera::FrustumPlanesField, frustumPlanes);
    iscene.setDataReference(dataInstance, Camera::FrustumNearFarPlanesField, frustumNearFar);
    const auto cameraHandle = sceneAllocator.allocateCamera(ECameraProjectionType::Orthographic, nodeHandle, dataInstance);

    iscene.setDataSingleVector2i(vpOffsetInstance, ramses_internal::DataFieldHandle{ 0 }, { 0, 0 });
    iscene.setDataSingleVector2i(vpSizeInstance, ramses_internal::DataFieldHandle{ 0 }, { 1280, 480 });
    const ProjectionParams params = ramses_internal::ProjectionParams::Frustum(ECameraProjectionType::Orthographic, -1.f, 1.f, -1.f, 1.f, 0.1f, 100.f);
    iscene.setDataSingleVector4f(frustumPlanes, DataFieldHandle{ 0 }, { params.leftPlane, params.rightPlane, params.bottomPlane, params.topPlane });
    iscene.setDataSingleVector2f(frustumNearFar, DataFieldHandle{ 0 }, { params.nearPlane, params.farPlane });

    const PickableObjectId id1{ 666u };
    const PickableObjectId id2{ 667u };
    const auto pickableHandle1 = sceneAllocator.allocatePickableObject(geomHandle, nodeHandle, id1);
    const auto pickableHandle2 = sceneAllocator.allocatePickableObject(geomHandle, nodeHandle, id2);
    iscene.setPickableObjectCamera(pickableHandle1, cameraHandle);
    iscene.setPickableObjectCamera(pickableHandle2, cameraHandle);
    performFlush();

    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, uploadDataBuffer(_, _, _, _, _));
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, updateDataBuffer(_, _, _, _));
    update();

    EXPECT_CALL(*rendererSceneUpdater, handlePickEvent(_, _));
    rendererSceneUpdater->handlePickEvent(getSceneId(), { -0.375000f, 0.250000f });
    expectSceneEvent(ERendererEventType::ObjectsPicked);

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, emitsSequenceOfSceneStateChangesWhenRepublished_fromSubscribed)
{
    createPublishAndSubscribeScene();

    const SceneId sceneId = stagingScene[0]->getSceneId();
    EXPECT_CALL(sceneEventSender, sendUnsubscribeScene(getSceneId()));
    rendererSceneUpdater->handleSceneUnpublished(sceneId);
    rendererSceneUpdater->handleScenePublished(sceneId, EScenePublicationMode_LocalAndRemote);
    expectInternalSceneStateEvents({ ERendererEventType::SceneUnsubscribedIndirect, ERendererEventType::SceneUnpublished, ERendererEventType::ScenePublished });
}

TEST_F(ARendererSceneUpdater, emitsSequenceOfSceneStateChangesWhenRepublished_fromMapped)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    expectUnloadOfSceneResources();

    const SceneId sceneId = stagingScene[0]->getSceneId();
    EXPECT_CALL(sceneEventSender, sendUnsubscribeScene(sceneId));
    rendererSceneUpdater->handleSceneUnpublished(sceneId);
    rendererSceneUpdater->handleScenePublished(sceneId, EScenePublicationMode_LocalAndRemote);
    expectInternalSceneStateEvents({ ERendererEventType::SceneUnmappedIndirect, ERendererEventType::SceneUnsubscribedIndirect, ERendererEventType::SceneUnpublished, ERendererEventType::ScenePublished });
    EXPECT_FALSE(renderer.getBufferSceneIsAssignedTo(sceneId).isValid());

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, emitsSequenceOfSceneStateChangesWhenRepublished_fromShown)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    expectUnloadOfSceneResources();

    const SceneId sceneId = stagingScene[0]->getSceneId();
    EXPECT_CALL(sceneEventSender, sendUnsubscribeScene(sceneId));
    rendererSceneUpdater->handleSceneUnpublished(sceneId);
    rendererSceneUpdater->handleScenePublished(sceneId, EScenePublicationMode_LocalAndRemote);
    expectInternalSceneStateEvents({ ERendererEventType::SceneHiddenIndirect, ERendererEventType::SceneUnmappedIndirect, ERendererEventType::SceneUnsubscribedIndirect, ERendererEventType::SceneUnpublished, ERendererEventType::ScenePublished });
    EXPECT_FALSE(renderer.getBufferSceneIsAssignedTo(sceneId).isValid());

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, propagatesGeneratedSceneReferenceActionsToSceneReferenceControl)
{
    createPublishAndSubscribeScene();

    constexpr SceneReferenceAction action1{ SceneReferenceActionType::LinkData, SceneReferenceHandle{1}, DataSlotId{2}, SceneReferenceHandle{3}, DataSlotId{4} };
    constexpr SceneReferenceAction action2{ SceneReferenceActionType::UnlinkData, SceneReferenceHandle{5}, DataSlotId{6}, SceneReferenceHandle{7}, DataSlotId{8} };
    const SceneReferenceActionVector sceneRefActions{ action1, action2 };

    performFlush(0u, {}, nullptr, {}, sceneRefActions);

    EXPECT_CALL(sceneReferenceLogic, addActions(getSceneId(), _)).WillOnce([&](auto, const auto& actions)
    {
        ASSERT_EQ(2u, actions.size());
        EXPECT_EQ(action1.type, actions[0].type);
        EXPECT_EQ(action1.providerScene, actions[0].providerScene);
        EXPECT_EQ(action1.providerId, actions[0].providerId);
        EXPECT_EQ(action1.consumerScene, actions[0].consumerScene);
        EXPECT_EQ(action1.consumerId, actions[0].consumerId);
        EXPECT_EQ(action2.type, actions[1].type);
        EXPECT_EQ(action2.providerScene, actions[1].providerScene);
        EXPECT_EQ(action2.providerId, actions[1].providerId);
        EXPECT_EQ(action2.consumerScene, actions[1].consumerScene);
        EXPECT_EQ(action2.consumerId, actions[1].consumerId);
    });
    update();

    //make empty flush to make sure actions were consumed
    performFlush(0u, {}, nullptr, {}, {});
    EXPECT_CALL(sceneReferenceLogic, addActions(_, _)).Times(0u);
    update();
}

TEST_F(ARendererSceneUpdater, propagatesGeneratedSceneReferenceActionsToSceneReferenceControlOnlyAfterFlushApplied)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    // simulate blocking flush on missing resource
    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash });
    createRenderable();
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Provided);
    expectVertexArrayUploaded();
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // send scene reference actions in flush
    constexpr SceneReferenceAction action1{ SceneReferenceActionType::LinkData, SceneReferenceHandle{1}, DataSlotId{2}, SceneReferenceHandle{3}, DataSlotId{4} };
    constexpr SceneReferenceAction action2{ SceneReferenceActionType::UnlinkData, SceneReferenceHandle{5}, DataSlotId{6}, SceneReferenceHandle{7}, DataSlotId{8} };
    const SceneReferenceActionVector sceneRefActions{ action1, action2 };
    performFlush(0u, {}, nullptr, {}, sceneRefActions);
    EXPECT_CALL(sceneReferenceLogic, addActions(_, _)).Times(0);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // unblock pending flushes by providing resource
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Uploaded);

    EXPECT_CALL(sceneReferenceLogic, addActions(getSceneId(), _)).WillOnce([&](auto, const auto& actions)
    {
        ASSERT_EQ(2u, actions.size());
        EXPECT_EQ(action1.type, actions[0].type);
        EXPECT_EQ(action1.providerScene, actions[0].providerScene);
        EXPECT_EQ(action1.providerId, actions[0].providerId);
        EXPECT_EQ(action1.consumerScene, actions[0].consumerScene);
        EXPECT_EQ(action1.consumerId, actions[0].consumerId);
        EXPECT_EQ(action2.type, actions[1].type);
        EXPECT_EQ(action2.providerScene, actions[1].providerScene);
        EXPECT_EQ(action2.providerId, actions[1].providerId);
        EXPECT_EQ(action2.consumerScene, actions[1].consumerScene);
        EXPECT_EQ(action2.consumerId, actions[1].consumerId);
    });
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    //make empty flush to make sure actions were consumed
    performFlush(0u, {}, nullptr, {}, {});
    EXPECT_CALL(sceneReferenceLogic, addActions(_, _)).Times(0u);
    update();

    hideScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, queuesAndPropagatesSceneReferenceActionsFromMultipleFlushesToSceneReferenceControl)
{
    createPublishAndSubscribeScene();

    constexpr SceneReferenceAction action1{ SceneReferenceActionType::LinkData, SceneReferenceHandle{1}, DataSlotId{2}, SceneReferenceHandle{3}, DataSlotId{4} };
    constexpr SceneReferenceAction action2{ SceneReferenceActionType::UnlinkData, SceneReferenceHandle{5}, DataSlotId{6}, SceneReferenceHandle{7}, DataSlotId{8} };

    performFlush(0u, {}, nullptr, {}, { action1 });
    performFlush(0u, {}, nullptr, {}, {});
    performFlush(0u, {}, nullptr, {}, { action2 });

    EXPECT_CALL(sceneReferenceLogic, addActions(getSceneId(), _)).WillOnce([&](auto, const auto& actions)
    {
        ASSERT_EQ(2u, actions.size());
        EXPECT_EQ(action1.type, actions[0].type);
        EXPECT_EQ(action1.providerScene, actions[0].providerScene);
        EXPECT_EQ(action1.providerId, actions[0].providerId);
        EXPECT_EQ(action1.consumerScene, actions[0].consumerScene);
        EXPECT_EQ(action1.consumerId, actions[0].consumerId);
        EXPECT_EQ(action2.type, actions[1].type);
        EXPECT_EQ(action2.providerScene, actions[1].providerScene);
        EXPECT_EQ(action2.providerId, actions[1].providerId);
        EXPECT_EQ(action2.consumerScene, actions[1].consumerScene);
        EXPECT_EQ(action2.consumerId, actions[1].consumerId);
    });
    update();

    //make empty flush to make sure actions were consumed
    performFlush(0u, {}, nullptr, {}, {});
    EXPECT_CALL(sceneReferenceLogic, addActions(_, _)).Times(0u);
    update();
}

}
