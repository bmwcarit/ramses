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

const DisplayHandle ARendererSceneUpdater::DisplayHandle1(0u);
const DisplayHandle ARendererSceneUpdater::DisplayHandle2(1u);

const ResourceContentHash ARendererSceneUpdater::InvalidResource1(0xff00ff01, 0);
const ResourceContentHash ARendererSceneUpdater::InvalidResource2(0xff00ff02, 0);


///////////////////////////
// General tests
///////////////////////////

TEST_F(ARendererSceneUpdater, processesAsynchronousFlushForSubscribedScene)
{
    createPublishAndSubscribeScene();
    const NodeHandle nodeHandle = performFlushWithCreateNodeAction();
    update();
    const IScene& scene = rendererScenes.getScene(stagingScene[0]->getSceneId());
    EXPECT_TRUE(scene.isNodeAllocated(nodeHandle));
}

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
    performFlush(0u, false, version);
    update();

    RendererEventVector events;
    rendererEventCollector.dispatchEvents(events);
    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(ERendererEventType_SceneFlushed, events[0].eventType);
    EXPECT_EQ(EResourceStatus_Unknown, events[0].resourceStatus);
    EXPECT_EQ(version, events[0].sceneVersionTag);
}

TEST_F(ARendererSceneUpdater, preallocatesSceneForSizesProvidedInFlushInfo)
{
    createPublishAndSubscribeScene();
    expectNoEvent();

    // perform empty flush only with notification of scene size change
    SceneSizeInformation sizeInfo;
    sizeInfo.renderableCount = 12u;
    performFlush(0u, false, SceneVersionTag::DefaultValue(), &sizeInfo);
    update();

    const IScene& rendererScene = rendererScenes.getScene(getSceneId());
    EXPECT_EQ(sizeInfo, rendererScene.getSceneSizeInformation());
}

TEST_F(ARendererSceneUpdater, ignoresSceneActionsForNotSubscribedScene)
{
    createStagingScene();
    publishScene();

    performFlushWithCreateNodeAction();
    update();
    // there is no scene to apply actions to
}

TEST_F(ARendererSceneUpdater, ignoresSceneActionsForNotReceivedScene)
{
    createStagingScene();
    publishScene();
    requestSceneSubscription();

    performFlushWithCreateNodeAction();
    update();
    // there is no scene to apply actions to
}

TEST_F(ARendererSceneUpdater, ignoresSceneActionsAddedAfterSceneWasUnsubscribed)
{
    createPublishAndSubscribeScene();

    EXPECT_CALL(sceneGraphConsumerComponent, unsubscribeScene(_, getSceneId()));
    rendererSceneUpdater->handleSceneUnsubscriptionRequest(getSceneId(), false);
    ASSERT_EQ(ESceneState::Published, sceneStateExecutor.getSceneState(getSceneId()));

    EXPECT_FALSE(rendererScenes.hasScene(getSceneId()));
}

TEST_F(ARendererSceneUpdater, ignoresSceneActionsAddedAfterSceneWasUnsubscribedByError)
{
    createPublishAndSubscribeScene();

    EXPECT_CALL(sceneGraphConsumerComponent, unsubscribeScene(_, getSceneId()));
    rendererSceneUpdater->handleSceneUnsubscriptionRequest(getSceneId(), true);
    ASSERT_EQ(ESceneState::Published, sceneStateExecutor.getSceneState(getSceneId()));

    EXPECT_FALSE(rendererScenes.hasScene(getSceneId()));
}

TEST_F(ARendererSceneUpdater, ignoresSceneActionsAddedBetweenUnsubscriptionAndResubscription)
{
    createPublishAndSubscribeScene();

    EXPECT_CALL(sceneGraphConsumerComponent, unsubscribeScene(_, getSceneId()));
    rendererSceneUpdater->handleSceneUnsubscriptionRequest(getSceneId(), false);
    ASSERT_EQ(ESceneState::Published, sceneStateExecutor.getSceneState(getSceneId()));

    performFlush(0u, false, SceneVersionTag(1u));

    EXPECT_CALL(sceneGraphConsumerComponent, subscribeScene(_, getSceneId()));
    rendererSceneUpdater->handleSceneSubscriptionRequest(getSceneId());
    rendererSceneUpdater->handleSceneReceived(SceneInfo(getSceneId()));

    // named flush ignored
    RendererEventVector events;
    rendererEventCollector.dispatchEvents(events);
    for (const auto& event : events)
    {
        EXPECT_NE(ERendererEventType_SceneFlushed, event.eventType);
    }
}

TEST_F(ARendererSceneUpdater, canCreateAndDestroyDisplayContext)
{
    createDisplayAndExpectSuccess();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canNotDestroyNonExistantDisplay)
{
    destroyDisplay(DisplayHandle1, true);
}

TEST_F(ARendererSceneUpdater, canNotDestroyDisplayIfItHasMapRequestedScenes)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    requestMapScene();

    destroyDisplay(DisplayHandle1, true);

    unpublishMapRequestedScene();
    destroyDisplay();

    EXPECT_FALSE(renderer.hasDisplayController(DisplayHandle1));
}

TEST_F(ARendererSceneUpdater, canNotDestroyDisplayIfItHasMappedScenes)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    destroyDisplay(DisplayHandle1, true);

    expectContextEnable();
    unmapScene();
    destroyDisplay();

    EXPECT_FALSE(renderer.hasDisplayController(DisplayHandle1));
}

TEST_F(ARendererSceneUpdater, canNotDestroyDisplayIfItHasRenderedScenes)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    destroyDisplay(DisplayHandle1, true);
    EXPECT_TRUE(renderer.hasDisplayController(DisplayHandle1));

    hideScene();
    expectContextEnable();
    unmapScene();
    destroyDisplay();

    EXPECT_FALSE(renderer.hasDisplayController(DisplayHandle1));
}

TEST_F(ARendererSceneUpdater, canDestroyDisplayIfMappedSceneGetsUnmapped)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    expectContextEnable();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canDestroyDisplayIfMappedSceneGetsUnpublished)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    expectContextEnable();
    unpublishMappedScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, destroyingSceneUpdaterUnmapsAnyMappedSceneFromRenderer)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    destroySceneUpdater();

    EXPECT_FALSE(renderer.getDisplaySceneIsMappedTo(getSceneId()).isValid());
}

TEST_F(ARendererSceneUpdater, destroyingSceneUpdaterDestroysAllDisplayContexts)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    destroySceneUpdater();

    EXPECT_FALSE(renderer.hasDisplayController(DisplayHandle1));
}

TEST_F(ARendererSceneUpdater, updateScenesWillUpdateRealTimeAnimationSystems)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    IScene& scene = *stagingScene[0u];
    AnimationSystem& animSystem = *new AnimationSystem(EAnimationSystemFlags_Default, AnimationSystemSizeInformation());
    AnimationSystem& animSystemReal = *new AnimationSystem(EAnimationSystemFlags_RealTime, AnimationSystemSizeInformation());

    auto hdl1 = scene.addAnimationSystem(&animSystem);
    auto hdl2 = scene.addAnimationSystem(&animSystemReal);
    performFlush();
    update();

    const IScene& rendererScene = rendererScenes.getScene(getSceneId());
    const IAnimationSystem* rendAnimSystem = rendererScene.getAnimationSystem(hdl1);
    const IAnimationSystem* rendAnimSystemReal = rendererScene.getAnimationSystem(hdl2);
    ASSERT_TRUE(rendAnimSystem != NULL);
    ASSERT_TRUE(rendAnimSystemReal != NULL);

    const AnimationTime time1 = rendAnimSystem->getTime();
    const AnimationTime time1real = rendAnimSystemReal->getTime();
    PlatformThread::Sleep(2u); // needed to make sure there's actual difference
    update();
    const AnimationTime time2 = rendAnimSystem->getTime();
    const AnimationTime time2real = rendAnimSystemReal->getTime();

    EXPECT_TRUE(time1 == time2);
    EXPECT_TRUE(time1real < time2real);

    hideScene();
    expectContextEnable();
    unmapScene();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, renderOncePassesAreRetriggeredWhenSceneMapped)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    auto& stageScene = *stagingScene[0];
    const RenderPassHandle pass = stageScene.allocateRenderPass();
    const auto dataLayout = stageScene.allocateDataLayout({ {EDataType_Vector2I}, {EDataType_Vector2I} });
    const CameraHandle camera = stageScene.allocateCamera(ECameraProjectionType_Orthographic, stageScene.allocateNode(), stageScene.allocateDataInstance(dataLayout));
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
    expectContextEnable();
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
    expectContextEnable();
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
    expectEvents({ ERendererEventType_SceneShowFailed, ERendererEventType_SceneHidden });
    EXPECT_EQ(ESceneState::Mapped, sceneStateExecutor.getSceneState(getSceneId()));

    // show failed (was canceled) and scene is still in mapped state
    update();
    expectNoEvent();
    EXPECT_EQ(ESceneState::Mapped, sceneStateExecutor.getSceneState(getSceneId()));

    // unmap scene
    expectContextEnable();
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
    expectEvents({ ERendererEventType_SceneUnmapFailed });
    EXPECT_EQ(ESceneState::RenderRequested, sceneStateExecutor.getSceneState(getSceneId()));

    update();
    expectEvents({ ERendererEventType_SceneShown });
    EXPECT_EQ(ESceneState::Rendered, sceneStateExecutor.getSceneState(getSceneId()));

    // unmap scene
    hideScene();
    expectContextEnable();
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

    // request unsubscribe should cancel the subscription and report subscription fail and unsubscribed events
    EXPECT_CALL(sceneGraphConsumerComponent, unsubscribeScene(_, getSceneId()));
    rendererSceneUpdater->handleSceneUnsubscriptionRequest(getSceneId(), false);
    expectEvents({ ERendererEventType_SceneSubscribeFailed, ERendererEventType_SceneUnsubscribed });
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

    // request unsubscribe should cancel the subscription and report subscription fail and unsubscribed events
    EXPECT_CALL(sceneGraphConsumerComponent, unsubscribeScene(_, getSceneId()));
    rendererSceneUpdater->handleSceneUnsubscriptionRequest(getSceneId(), false);
    expectEvents({ ERendererEventType_SceneSubscribeFailed, ERendererEventType_SceneUnsubscribed });
    EXPECT_EQ(ESceneState::Published, sceneStateExecutor.getSceneState(getSceneId()));

    update();
    expectNoEvent();
    EXPECT_EQ(ESceneState::Published, sceneStateExecutor.getSceneState(getSceneId()));
}


///////////////////////////
// Offscreen buffer tests
///////////////////////////

TEST_F(ARendererSceneUpdater, canCreateOffscreenBuffer)
{
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    expectContextEnable();
    expectRenderTargetUploaded(DisplayHandle1, true);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, DisplayHandle1, 1u, 1u, false));

    expectRenderTargetDeleted();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canCreateDoubleBufferedOffscreenBuffer)
{
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    expectContextEnable();
    expectRenderTargetUploaded(DisplayHandle1, true, DeviceMock::FakeRenderTargetDeviceHandle, true);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, DisplayHandle1, 1u, 1u, true));

    expectRenderTargetDeleted(DisplayHandle1, 1u, true);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, failsToCreateOffscreenBufferOnUnknownDisplay)
{
    const OffscreenBufferHandle buffer(1u);
    EXPECT_FALSE(rendererSceneUpdater->handleBufferCreateRequest(buffer, DisplayHandle1, 1u, 1u, false));
}

TEST_F(ARendererSceneUpdater, failsToCreateOffscreenBufferWithSameID)
{
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    expectContextEnable();
    expectRenderTargetUploaded(DisplayHandle1, true);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, DisplayHandle1, 1u, 1u, false));

    EXPECT_FALSE(rendererSceneUpdater->handleBufferCreateRequest(buffer, DisplayHandle1, 1u, 1u, false));

    expectRenderTargetDeleted();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canDestroyOffscreenBuffer)
{
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    expectContextEnable();
    expectRenderTargetUploaded(DisplayHandle1, true);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, DisplayHandle1, 1u, 1u, false));

    expectContextEnable();
    expectRenderTargetDeleted();
    EXPECT_TRUE(rendererSceneUpdater->handleBufferDestroyRequest(buffer, DisplayHandle1));

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canDestroyDoubleBufferedOffscreenBuffer)
{
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    expectContextEnable();
    expectRenderTargetUploaded(DisplayHandle1, true, DeviceMock::FakeRenderTargetDeviceHandle, true);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, DisplayHandle1, 1u, 1u, true));

    expectContextEnable();
    expectRenderTargetDeleted(DisplayHandle1, 1u, true);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferDestroyRequest(buffer, DisplayHandle1));

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, failsToDestroyOffscreenBufferOnUnknownDisplay)
{
    const OffscreenBufferHandle buffer(1u);
    EXPECT_FALSE(rendererSceneUpdater->handleBufferDestroyRequest(buffer, DisplayHandle1));
}

TEST_F(ARendererSceneUpdater, failsToDestroyUnknownOffscreenBuffer)
{
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    EXPECT_FALSE(rendererSceneUpdater->handleBufferDestroyRequest(buffer, DisplayHandle1));

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canAssignSceneToOffscreenBuffer)
{
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    expectContextEnable();
    expectRenderTargetUploaded(DisplayHandle1, true);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, DisplayHandle1, 1u, 1u, false));

    createPublishAndSubscribeScene();
    mapScene(0u);
    EXPECT_TRUE(assignSceneToOffscreenBuffer(0, buffer));

    expectContextEnable();
    unmapScene(0u);
    expectRenderTargetDeleted();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canAssignSceneToFramebuffer_MultipleTimes)
{
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    expectContextEnable();
    expectRenderTargetUploaded(DisplayHandle1, true);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, DisplayHandle1, 1u, 1u, false));

    createPublishAndSubscribeScene();
    mapScene(0u);
    // Scene can be always assigned to framebuffer
    EXPECT_TRUE(assignSceneToFramebuffer(0));
    EXPECT_TRUE(assignSceneToFramebuffer(0));
    EXPECT_TRUE(assignSceneToFramebuffer(0));

    expectContextEnable();
    unmapScene(0u);
    expectRenderTargetDeleted();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, confidence_failsToDestroyOffscreenBufferIfScenesAreAssignedToIt_DestroysAfterSceneIsAssignedToFramebuffer)
{
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    expectContextEnable();
    expectRenderTargetUploaded(DisplayHandle1, true);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, DisplayHandle1, 1u, 1u, false));

    createPublishAndSubscribeScene();
    mapScene(0u);
    EXPECT_TRUE(assignSceneToOffscreenBuffer(0, buffer));

    EXPECT_FALSE(rendererSceneUpdater->handleBufferDestroyRequest(buffer, DisplayHandle1));

    EXPECT_TRUE(assignSceneToFramebuffer(0));

    expectContextEnable();
    expectRenderTargetDeleted();
    EXPECT_TRUE(rendererSceneUpdater->handleBufferDestroyRequest(buffer, DisplayHandle1));

    expectContextEnable();
    unmapScene(0u);
    destroyDisplay();
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
    expectContextEnable();
    unmapScene(0u);
    expectContextEnable();
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
    expectContextEnable();
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
    expectContextEnable();
    unmapScene(0u);
    expectContextEnable();
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

    expectResourceRequest();
    createTextureSlotsAndLinkThem();
    update();

    hideScene(0u);
    expectContextEnable();
    unmapScene(0u);

    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getSceneLinks().hasAnyLinksToConsumer(getSceneId(0u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getSceneLinks().hasAnyLinksToProvider(getSceneId(0u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getSceneLinks().hasAnyLinksToConsumer(getSceneId(1u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getSceneLinks().hasAnyLinksToProvider(getSceneId(1u)));

    hideScene(1u);
    expectContextEnable();
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

    expectResourceRequest();
    createTextureSlotsAndLinkThem();
    update();

    hideScene(1u);
    expectContextEnable();
    unmapScene(1u);

    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getSceneLinks().hasAnyLinksToConsumer(getSceneId(0u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getSceneLinks().hasAnyLinksToProvider(getSceneId(0u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getSceneLinks().hasAnyLinksToConsumer(getSceneId(1u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getSceneLinks().hasAnyLinksToProvider(getSceneId(1u)));

    hideScene(0u);
    expectContextEnable();
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
    expectContextEnable();
    unmapScene(1u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, failsToCreateTextureLinkIfConsumerSceneNotMapped)
{
    createDisplayAndExpectSuccess();

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();

    mapScene(0u);
    showScene(0u);

    expectResourceRequest();
    createTextureSlotsAndLinkThem(nullptr, 0u, 1u, false);
    update();

    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getSceneLinks().hasAnyLinksToConsumer(getSceneId(0u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getSceneLinks().hasAnyLinksToProvider(getSceneId(0u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getSceneLinks().hasAnyLinksToConsumer(getSceneId(1u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getSceneLinks().hasAnyLinksToProvider(getSceneId(1u)));

    hideScene(0u);
    expectContextEnable();
    unmapScene(0u);
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

TEST_F(ARendererSceneUpdater, failsToCreateTextureLinkIfProviderSceneMappedToDifferentDisplayThanConsumerScene)
{
    createDisplayAndExpectSuccess(DisplayHandle1);
    createDisplayAndExpectSuccess(DisplayHandle2);

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();

    mapScene(0u, DisplayHandle1);
    mapScene(1u, DisplayHandle2);

    expectResourceRequest();
    createTextureSlotsAndLinkThem(nullptr, 0u, 1u, false);
    update();

    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getSceneLinks().hasAnyLinksToConsumer(getSceneId(0u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getSceneLinks().hasAnyLinksToProvider(getSceneId(0u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getSceneLinks().hasAnyLinksToConsumer(getSceneId(1u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getSceneLinks().hasAnyLinksToProvider(getSceneId(1u)));

    expectContextEnable(DisplayHandle1);
    unmapScene(0u);
    expectContextEnable(DisplayHandle2);
    unmapScene(1u);

    destroyDisplay(DisplayHandle1);
    destroyDisplay(DisplayHandle2);
}

TEST_F(ARendererSceneUpdater, triggersRemovalOfBufferLinkWhenBufferDestroyed)
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
    expectContextEnable();
    expectRenderTargetUploaded(DisplayHandle1, true);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, DisplayHandle1, 1u, 1u, false));

    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToConsumer(buffer));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToProvider(getSceneId(0u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToProvider(getSceneId(1u)));

    createBufferLink(buffer, getSceneId(0u), consumerId1);
    createBufferLink(buffer, getSceneId(1u), consumerId2);

    EXPECT_TRUE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToConsumer(buffer));
    EXPECT_TRUE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToProvider(getSceneId(0u)));
    EXPECT_TRUE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToProvider(getSceneId(1u)));

    expectContextEnable();
    expectRenderTargetDeleted();
    EXPECT_TRUE(rendererSceneUpdater->handleBufferDestroyRequest(buffer, DisplayHandle1));

    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToConsumer(buffer));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToProvider(getSceneId(0u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToProvider(getSceneId(1u)));

    hideScene(0u);
    hideScene(1u);
    expectContextEnable(DisplayHandle1, 2u);
    unmapScene(0u);
    unmapScene(1u);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, confidenceTest_triggersRemovalOfBufferLinkWhenConsumerSceneUnmapped_keepsOtherConsumerLinked)
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
    expectContextEnable();
    expectRenderTargetUploaded(DisplayHandle1, true);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, DisplayHandle1, 1u, 1u, false));

    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToConsumer(buffer));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToProvider(getSceneId(0u)));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToProvider(getSceneId(1u)));

    createBufferLink(buffer, getSceneId(0u), consumerId1);
    createBufferLink(buffer, getSceneId(1u), consumerId2);

    EXPECT_TRUE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToConsumer(buffer));
    EXPECT_TRUE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToProvider(getSceneId(0u)));
    EXPECT_TRUE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToProvider(getSceneId(1u)));

    hideScene(0u);
    expectContextEnable();
    unmapScene(0u);

    EXPECT_TRUE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToConsumer(buffer));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToProvider(getSceneId(0u)));
    EXPECT_TRUE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToProvider(getSceneId(1u)));

    hideScene(1u);
    expectContextEnable();
    unmapScene(1u);

    expectRenderTargetDeleted();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, failsToCreateBufferLinkIfConsumerSceneNotMapped)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    const DataSlotId consumerId = createTextureConsumer(0u);

    const OffscreenBufferHandle buffer(1u);
    expectContextEnable();
    expectRenderTargetUploaded(DisplayHandle1, true);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, DisplayHandle1, 1u, 1u, false));

    createBufferLink(buffer, getSceneId(), consumerId, true);

    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToConsumer(buffer));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToProvider(getSceneId()));

    expectRenderTargetDeleted();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, failsToCreateBufferLinkIfProviderBufferIsFromAnotherDisplay)
{
    createDisplayAndExpectSuccess(DisplayHandle1);
    createDisplayAndExpectSuccess(DisplayHandle2);

    createPublishAndSubscribeScene();
    mapScene();

    const DataSlotId consumerId = createTextureConsumer(0u);

    const OffscreenBufferHandle buffer(1u);
    expectContextEnable(DisplayHandle2);
    expectRenderTargetUploaded(DisplayHandle2, true);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, DisplayHandle2, 1u, 1u, false));

    createBufferLink(buffer, getSceneId(), consumerId, true);

    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToConsumer(buffer));
    EXPECT_FALSE(rendererScenes.getSceneLinksManager().getTextureLinkManager().getOffscreenBufferLinks().hasAnyLinksToProvider(getSceneId()));

    expectContextEnable();
    unmapScene();

    destroyDisplay(DisplayHandle1);
    expectRenderTargetDeleted(DisplayHandle2);
    destroyDisplay(DisplayHandle2);
}

/////////////////////////////////////////////
// Partial scene actions applying tests
/////////////////////////////////////////////

TEST_F(ARendererSceneUpdater, doesNotApplySceneActionsIfUpdateTimeBudgetExceeded)
{
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene(); // need 2 scenes to allow partial flush processing

    performFlush();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    // simulate no time left for update operations
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::SceneActionsApply, 0u);

    performFlushWithCreateNodeAction(0, RendererSceneUpdater::SceneActionsPerChunkToApply + 1);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());
}

TEST_F(ARendererSceneUpdater, interruptsApplyingBigFlushWithLimitedBudgetAndFinishesAfterSeveralUpdates)
{
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene(); // need 2 scenes to allow partial flush processing

    performFlush();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    // simulate no time left for update operations
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::SceneActionsApply, 0u);

    performFlushWithCreateNodeAction(0, RendererSceneUpdater::SceneActionsPerChunkToApply * 3);

    for (int i = 0; i < 6; ++i)
    {
        update();
        EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());
    }

    // eventually applies last chunk of big flush
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());
}

TEST_F(ARendererSceneUpdater, interruptsApplyingManySmallFlushesWithLimitedBudgetAndFinishesAfterSeveralUpdates)
{
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene(); // need 2 scenes to allow partial flush processing

    performFlush();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    // simulate no time left for update operations
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::SceneActionsApply, 0u);

    // make sure there will be no force apply or unsubscribe
    rendererSceneUpdater->setLimitFlushesForceApply(999999);
    rendererSceneUpdater->setLimitFlushesForceUnsubscribe(999999);

    for (int i = 0; i < 3; ++i)
        performFlush();

    for (int i = 0; i < 2; ++i)
    {
        // due to 0 limit at most one small flush is applied per update
        update();
        EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());
    }

    // eventually applies last flush
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());
}

TEST_F(ARendererSceneUpdater, interruptsApplyingSeveralBigFlushesWithLimitedBudgetAndFinishesAfterSeveralUpdates)
{
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene(); // need 2 scenes to allow partial flush processing

    performFlush();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    // simulate no time left for update operations
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::SceneActionsApply, 0u);

    for (int i = 0; i < 3; ++i)
        performFlushWithCreateNodeAction(0, RendererSceneUpdater::SceneActionsPerChunkToApply);

    for (int i = 0; i < 8; ++i)
    {
        update();
        EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());
    }

    // eventually applies last chunk of last big flush
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());
}

TEST_F(ARendererSceneUpdater, continuesApplyingPendingSceneActionsNextUpdateAfterUpdateTimeBudgetExceeded)
{
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene(); // need 2 scenes to allow partial flush processing

    // simulate no time left for update operations
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::SceneActionsApply, 0u);

    performFlushWithCreateNodeAction(0, RendererSceneUpdater::SceneActionsPerChunkToApply * 3);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // simulate some time left to continue with applying scene actions
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::SceneActionsApply, 10000000u);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());
}

TEST_F(ARendererSceneUpdater, continuesApplyingPendingSceneActionsNextUpdateAfterUpdateTimeBudgetExceeded_inMappedState)
{
    createDisplayAndExpectSuccess();

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene(); // need 2 scenes to allow partial flush processing

    mapScene();

    // simulate no time left for update operations
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::SceneActionsApply, 0u);

    performFlushWithCreateNodeAction(0, RendererSceneUpdater::SceneActionsPerChunkToApply * 3);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // simulate some time left to continue with applying scene actions
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::SceneActionsApply, 10000000u);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    expectContextEnable();
    unmapScene();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, ignoresUpdateTimeBudgetIfThereIsNoOtherSceneToBlock)
{
    createPublishAndSubscribeScene();

    performFlush();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    // simulate no time left for update operations
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::SceneActionsApply, 0u);

    performFlushWithCreateNodeAction(0, RendererSceneUpdater::SceneActionsPerChunkToApply + 1);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());
}

TEST_F(ARendererSceneUpdater, ignoresUpdateTimeBudgetIfSceneIsShown)
{
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene(); // need 2 scenes to allow partial flush processing

    performFlush();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    createDisplayAndExpectSuccess();
    mapScene();
    showScene();

    // simulate no time left for update operations
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::SceneActionsApply, 0u);

    performFlushWithCreateNodeAction(0, RendererSceneUpdater::SceneActionsPerChunkToApply + 1);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    hideScene();
    expectContextEnable();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, willMapSceneOnlyAfterAllPartialFlushesApplied)
{
    createDisplayAndExpectSuccess();

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene(); // need 2 scenes to allow partial flush processing

    // simulate no time left for update operations
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::SceneActionsApply, 0u);

    performFlushWithCreateNodeAction(0, RendererSceneUpdater::SceneActionsPerChunkToApply * 10); // will call multiple updates -> make sure there's enough actions to interrupt
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());
    // now has pending flush

    requestMapScene();

    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());
    expectNoEvent();

    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());
    expectNoEvent();

    // simulate some time left to continue with applying scene actions
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::SceneActionsApply, 10000000u);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    // needs one more update to progress through mapping states to final mapped state
    update();
    expectEvent(ERendererEventType_SceneMapped);

    expectContextEnable();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, willShowSceneOnlyAfterAllPartialFlushesApplied)
{
    createDisplayAndExpectSuccess();

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene(); // need 2 scenes to allow partial flush processing

    mapScene();

    // simulate no time left for update operations
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::SceneActionsApply, 0u);

    performFlushWithCreateNodeAction(0, RendererSceneUpdater::SceneActionsPerChunkToApply * 10); // will call multiple updates -> make sure there's enough actions to interrupt
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());
    // now has pending flush

    // request show
    rendererSceneUpdater->handleSceneShowRequest(getSceneId(0u));

    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());
    expectNoEvent();

    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());
    expectNoEvent();

    // simulate some time left to continue with applying scene actions
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::SceneActionsApply, 10000000u);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());
    // only now is scene shown
    expectEvent(ERendererEventType_SceneShown);

    hideScene();
    expectContextEnable();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, willShowSceneAsSoonAsPartialFlushAppliedEvenIfOtherFlushesPending)
{
    createDisplayAndExpectSuccess();

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene(); // need 2 scenes to allow partial flush processing

    mapScene();
    update();

    // simulate no time left for update operations
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::SceneActionsApply, 0u);

    // big flush
    performFlushWithCreateNodeAction(0, RendererSceneUpdater::SceneActionsPerChunkToApply * 3);
    // small flushes
    performFlush();
    performFlush();
    performFlush();

    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());
    // now has pending flushes

    // request show
    rendererSceneUpdater->handleSceneShowRequest(getSceneId(0u));

    for (UInt i = 0; i < 5; ++i)
    {
        update();
        EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());
        expectNoEvent();
    }

    // this update applies last chunk of big flush
    update();
    // now scene could be shown
    expectEvent(ERendererEventType_SceneShown);
    // but there are still pending (not interrupted) flushes
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // scene is shown now, partial applying not allowed -> rest of flushes are applied right away
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    hideScene();
    expectContextEnable();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, confidenceTest_appliesMultipleConsecutiveFlushesBasedOnUpdateTimeBudget)
{
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene(); // need 2 scenes to allow partial flush processing

    for (UInt i = 0u; i < 5u; ++i)
    {
        performFlushWithCreateNodeAction(0, RendererSceneUpdater::SceneActionsPerChunkToApply * 3);

        // no time for update
        frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::SceneActionsApply, 0u);
        update();
        expectNoEvent();
        EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

        // enough time to apply flush scene actions
        frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::SceneActionsApply, 10000000u);
        update();
        EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());
    }
}

TEST_F(ARendererSceneUpdater, generatesNamedFlushEventAfterItIsFullyAppliedWhenInterrupted)
{
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene(); // need 2 scenes to allow partial flush processing

    performFlush();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    // no budget for actions applying
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::SceneActionsApply, 0u);

    const SceneVersionTag versionTag(333);
    IScene& scene = *stagingScene[0];

    // generate big enough flush so that it contains multiple chunks and will be interrupted at least once
    SceneAllocateHelper sceneAllocator(scene);
    for (int i = 0; i < 1000; ++i)
        sceneAllocator.allocateNode();

    performFlush(0, false, versionTag);

    // check it was really interrupted
    update();
    expectNoEvent();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // wait for it to be finished
    while (!lastFlushWasAppliedOnRendererScene())
    {
        // expect no event till flush fully applied
        expectNoEvent();
        update();
    }

    // expect named flush to be reported after flush fully applied
    RendererEventVector events;
    rendererEventCollector.dispatchEvents(events);
    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(ERendererEventType_SceneFlushed, events[0].eventType);
    EXPECT_EQ(versionTag, events[0].sceneVersionTag);
}

/////////////////////////////////////////////
// Other tests
/////////////////////////////////////////////

TEST_F(ARendererSceneUpdater, updateSceneStreamTexturesDirtinessGeneratesEventsForNewAndObsoleteStreamSurfaces)
{
    createDisplayAndExpectSuccess();

    const StreamTextureSourceId newStreamId(7563u);
    const StreamTextureSourceId obsoleteStreamId(8883u);
    const StreamTextureSourceIdVector newStreams{ newStreamId };
    const StreamTextureSourceIdVector obsoleteStreams{ obsoleteStreamId };

    expectNoEvent();

    EXPECT_CALL(*renderer.getDisplayMock(DisplayHandle1).m_embeddedCompositingManager, dispatchStateChangesOfStreamTexturesAndSources(_, _, _)).WillOnce(DoAll(SetArgReferee<1>(newStreams), SetArgReferee<2>(obsoleteStreams)));

    update();

    RendererEventVector resultEvents;
    rendererEventCollector.dispatchEvents(resultEvents);
    ASSERT_EQ(2u, resultEvents.size());
    EXPECT_EQ(ERendererEventType_StreamSurfaceAvailable, resultEvents[0].eventType);
    EXPECT_EQ(newStreamId, resultEvents[0].streamSourceId);
    EXPECT_EQ(ERendererEventType_StreamSurfaceUnavailable, resultEvents[1].eventType);
    EXPECT_EQ(obsoleteStreamId, resultEvents[1].streamSourceId);

    destroyDisplay();
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
    const DeviceResourceHandle fakeDeviceResourceHandle(1u);
    expectEmbeddedCompositingManagerReturnsDeviceHandle(fakeDeviceResourceHandle);

    update();
    expectRenderableResourcesClean();

    {
        EXPECT_CALL(*renderer.getDisplayMock(DisplayHandle1).m_embeddedCompositingManager, dispatchStateChangesOfStreamTexturesAndSources(_, _, _));

        update();
        expectNoScenesModified();
    }

    hideScene();
    expectContextEnable();
    expectRenderableResourcesDeleted(DisplayHandle1, true, true, false, true);
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarksSceneAsModified_IfStreamTextureStateIsUpdated)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();
    createRenderableAndResourcesWithStreamTexture();
    const DeviceResourceHandle fakeDeviceResourceHandle(1u);
    expectEmbeddedCompositingManagerReturnsDeviceHandle(fakeDeviceResourceHandle);

    update();
    expectRenderableResourcesClean();

    {
        const SceneId sceneId = stagingScene[0u]->getSceneId();
        SceneStreamTextures updatedStreamTextures;
        updatedStreamTextures.put(sceneId, StreamTextureHandleVector());
        updatedStreamTextures.get(sceneId)->push_back(streamTextureHandle);
        EXPECT_CALL(*renderer.getDisplayMock(DisplayHandle1).m_embeddedCompositingManager, dispatchStateChangesOfStreamTexturesAndSources(_, _, _)).WillOnce(SetArgReferee<0>(updatedStreamTextures));
        expectEmbeddedCompositingManagerReturnsDeviceHandle(fakeDeviceResourceHandle);

        update();
        expectScenesModified();
    }
    {
        EXPECT_CALL(*renderer.getDisplayMock(DisplayHandle1).m_embeddedCompositingManager, dispatchStateChangesOfStreamTexturesAndSources(_, _, _));

        update();
        expectNoScenesModified();
    }

    hideScene();
    expectContextEnable();
    expectRenderableResourcesDeleted(DisplayHandle1, true, true, false, true);
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarksSceneAsModified_IfStreamSourceContentIsUpdated)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();
    createRenderableAndResourcesWithStreamTexture();
    const DeviceResourceHandle fakeDeviceResourceHandle(1u);
    expectEmbeddedCompositingManagerReturnsDeviceHandle(fakeDeviceResourceHandle);

    update();
    expectRenderableResourcesClean();

    {
        const SceneId sceneId = stagingScene[0u]->getSceneId();
        UpdatedSceneIdSet updatedScenes;
        updatedScenes.put(sceneId);
        EXPECT_CALL(*renderer.getDisplayMock(DisplayHandle1).m_embeddedCompositingManager, hasUpdatedContentFromStreamSourcesToUpload()).WillOnce(Return(true));
        EXPECT_CALL(*renderer.getDisplayMock(DisplayHandle1).m_embeddedCompositingManager, uploadResourcesAndGetUpdates(_, _)).WillOnce(SetArgReferee<0>(updatedScenes));

        expectContextEnable();
        update();
        expectScenesModified();
    }
    {
        //SS not updated
        EXPECT_CALL(*renderer.getDisplayMock(DisplayHandle1).m_embeddedCompositingManager, hasUpdatedContentFromStreamSourcesToUpload()).WillOnce(Return(false));

        update();
        expectNoScenesModified();
    }

    hideScene();
    expectContextEnable();
    expectRenderableResourcesDeleted(DisplayHandle1, true, true, false, true);
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarksSceneAsModified_IfNonEmptyFlushApplied)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    update();
    expectNoScenesModified();

    performFlushWithCreateNodeAction();
    update();
    expectScenesModified();

    update();
    expectNoScenesModified();

    hideScene();
    expectContextEnable();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, DoesNotMarkSceneAsModified_IfEmptyAsynchFlushAppliedOnEmptyScene)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    expectNoScenesModified();

    performFlush();
    update();
    expectNoScenesModified();

    hideScene();
    expectContextEnable();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, DoesNotMarkSceneAsModified_IfEmptySynchFlushAppliedOnEmptyScene)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    expectNoScenesModified();

    performFlush(0u, true);
    update();
    expectNoScenesModified();

    hideScene();
    expectContextEnable();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, DoesNotMarkSceneAsModified_IfEmptyAsynchFlushAppliedOnNonEmptyScene)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    performFlushWithCreateNodeAction();
    update();
    expectScenesModified();

    performFlush();
    update();
    expectNoScenesModified();

    hideScene();
    expectContextEnable();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, DoesNotMarkSceneAsModified_IfEmptySynchFlushAppliedOnNonEmptyScene)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    performFlushWithCreateNodeAction();
    update();
    expectScenesModified();

    performFlush(0u, true);
    update();
    expectNoScenesModified();

    hideScene();
    expectContextEnable();
    unmapScene();
    destroyDisplay();
}


TEST_F(ARendererSceneUpdater, MarksSceneAsModified_IfSynchFlushAppliedWithExistingClientResources)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    createRenderable();

    expectResourceRequest();
    setRenderableResources();
    expectContextEnable();
    expectRenderableResourcesUploaded();
    update();
    expectRenderableResourcesClean();
    expectScenesModified();

    update();
    expectNoScenesModified();

    hideScene();
    expectContextEnable();
    expectRenderableResourcesDeleted(DisplayHandle1);
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, DoesNotMarkSceneAsModified_IfSynchFlushCouldNotBeAppliedDueToMissingClientResource)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    createRenderable();

    {
        expectResourceRequest();
        setRenderableResources();
        expectContextEnable();
        expectRenderableResourcesUploaded();
        update();
        expectRenderableResourcesClean();
        expectScenesModified();
        EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());
    }

    update();
    expectNoScenesModified();

    {
        resourceProvider1.setIndexArrayAvailability(false);
        setRenderableResources(0u, ResourceProviderMock::FakeIndexArrayHash2, true);
        expectResourceRequest();
        update();
        expectNoScenesModified();
        EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());
    }

    hideScene();
    expectContextEnable();
    expectRenderableResourcesDeleted(DisplayHandle1);
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarkSceneAsModified_IfSynchFlushAppliedAfterMissingClientResourceArrives)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    const ResourceContentHash missingResourceHash = ResourceProviderMock::FakeIndexArrayHash;
    createRenderable();
    setRenderableResources(0u, missingResourceHash, true);

    {
        resourceProvider1.setIndexArrayAvailability(false);

        expectResourceRequest();
        expectContextEnable();
        expectRenderableResourcesUploaded(DisplayHandle1, true, false);
        update();
        EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());
    }

    update();
    expectNoScenesModified();

    {
        resourceProvider1.setIndexArrayAvailability(true);

        expectContextEnable();
        expectRenderableResourcesUploaded(DisplayHandle1, false, true);
        update();
        ASSERT_TRUE(lastFlushWasAppliedOnRendererScene());
        expectRenderableResourcesClean();
        expectScenesModified();
    }

    EXPECT_CALL(resourceProvider1, popArrivedResources(_)).Times(AnyNumber()).WillRepeatedly(Return(ManagedResourceVector()));
    update();
    expectNoScenesModified();

    hideScene();
    expectContextEnable();
    expectRenderableResourcesDeleted(DisplayHandle1);
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarkSceneAsModified_DataLinking_IfSceneIsConsumerAndProviderSceneIsUpdated)
{
    // s0 [modified] -> s1 [modified]
    createDisplayAndExpectSuccess();

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    mapScene(1u);
    showScene(1u);


    DataInstanceHandle consumerDataRef;
    DataInstanceHandle providerDataRef;
    createDataSlotsAndLinkThem(consumerDataRef, 333.f, &providerDataRef);
    update();

    update();
    expectNoScenesModified();

    updateProviderDataSlot(0u, providerDataRef, 777.f);
    performFlush();
    update();
    expectScenesModified({0u, 1u});

    update();
    expectNoScenesModified();

    hideScene(1u);
    expectContextEnable();
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

    update();
    expectNoScenesModified();

    stagingScene[1u]->setDataSingleFloat(consumerDataRef, DataFieldHandle(0u), 324.f);
    performFlush(1u);
    update();
    expectScenesModified({1u}); //only consumer if marked as modified

    update();
    expectNoScenesModified();

    hideScene(1u);
    expectContextEnable();
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
    mapScene(2u);
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

    update();
    expectNoScenesModified();

    updateProviderDataSlot(0u, providerDataRef, 1.0f);
    performFlush();
    update();
    expectScenesModified({0u, 1u, 2u});

    update();
    expectNoScenesModified();

    hideScene(2u);
    expectContextEnable();
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
    mapScene(2u);
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

    update();
    expectNoScenesModified();

    updateProviderDataSlot(1u, providerDataRef, 1.0f);
    performFlush(1u);
    update();
    expectScenesModified({1u, 2u});

    update();
    expectNoScenesModified();

    hideScene(2u);
    expectContextEnable();
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
    showScene(2u);

    {
        //tex. link scene 1 to scene 0
        expectResourceRequest();
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

    update();
    expectNoScenesModified();

    updateProviderDataSlot(1u, providerDataRef, 1.0f);
    performFlush(1u);
    update();
    expectScenesModified({1u, 2u});

    update();
    expectNoScenesModified();

    hideScene(2u);
    expectContextEnable(DisplayHandle1, 3u);
    unmapScene(2u);
    unmapScene(1u);
    unmapScene(0u);
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
    mapScene(3u);
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

    update();
    expectNoScenesModified();

    updateProviderDataSlot(1u, providerDataRef, 1.0f);
    performFlush(1u);
    update();
    expectScenesModified({1u, 2u, 3u});

    update();
    expectNoScenesModified();

    hideScene(3u);
    expectContextEnable();
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
    mapScene(1u);
    showScene(1u);

    DataSlotHandle providerDataSlotHandle;
    expectResourceRequest();
    createTextureSlotsAndLinkThem(&providerDataSlotHandle);
    update();

    update();
    expectNoScenesModified();

    expectResourceRequest();
    updateProviderTextureSlot(0u, providerDataSlotHandle, ResourceContentHash(0x9976u, 0));
    performFlush();
    update();
    expectScenesModified({0u, 1u});

    expectResourceRequestCancel(ResourceContentHash(0x1234u, 0));
    update();
    expectNoScenesModified();

    hideScene(1u);
    expectContextEnable(DisplayHandle1, 2u);
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
    showScene(3u);

    {
        //link scene 1 to scene 0
        expectResourceRequest();
        createTextureSlotsAndLinkThem();
        update();
    }

    DataSlotHandle providerDataSlotHandle;
    {
        //linke scene 2 to scene 1
        createTextureSlotsAndLinkThem(&providerDataSlotHandle, 1u, 2u);
        update();
    }

    {
        //linke scene 3 to scene 2
        createTextureSlotsAndLinkThem(nullptr, 2u, 3u);
        update();
    }

    update();
    expectNoScenesModified();

    expectResourceRequest(DisplayHandle1, 1u);
    updateProviderTextureSlot(1u, providerDataSlotHandle, ResourceContentHash(0x9976u, 0));
    performFlush(1u);
    update();
    expectScenesModified({1u, 2u, 3u});

    update();
    expectNoScenesModified();

    hideScene(3u);
    expectContextEnable(DisplayHandle1, 4u);
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
    showScene(1u);

    TransformHandle providerTransformHandle;
    createTransformationSlotsAndLinkThem(&providerTransformHandle);
    update();

    update();
    expectNoScenesModified();

    stagingScene[0]->setTranslation(providerTransformHandle, {0.f, 1.f, 2.f});
    performFlush();
    update();
    expectScenesModified({0u, 1u});

    update();
    expectNoScenesModified();

    hideScene(1u);
    expectContextEnable(DisplayHandle1, 2u);
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

    update();
    expectNoScenesModified();

    stagingScene[1]->setTranslation(providerTransformHandle, {0.f, 1.f, 2.f});
    performFlush(1u);
    update();
    expectScenesModified({1u, 2u, 3u});

    update();
    expectNoScenesModified();

    hideScene(3u);
    expectContextEnable(DisplayHandle1, 4u);
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
    expectContextEnable();
    expectRenderTargetUploaded(DisplayHandle1, true);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, DisplayHandle1, 1u, 1u, false));

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    mapScene(0u);
    mapScene(1u);
    showScene(0u);
    showScene(1u);

    EXPECT_TRUE(assignSceneToOffscreenBuffer(0, buffer));

    const DataSlotId consumerId = createTextureConsumer(1u);
    createBufferLink(buffer, stagingScene[1u]->getSceneId() , consumerId);
    update();

    update();
    expectNoScenesModified();

    //update scene mapped to buffer
    performFlushWithCreateNodeAction(0u);
    //expect both scenes modified
    update();
    expectScenesModified({0u, 1u});

    update();
    expectNoScenesModified();

    hideScene(0u);
    hideScene(1u);
    expectContextEnable(DisplayHandle1, 2u);
    unmapScene(0u);
    unmapScene(1u);
    expectRenderTargetDeleted();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarkSceneAsModified_OffscreenBufferLinking_IfSceneConsumesFromModifiedOffscreenBufferWithSeveralScenes)
{
    // s0            --
    //                 |-> ob1 -> s2 [modified]
    // s1 [modified] --
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    expectContextEnable();
    expectRenderTargetUploaded(DisplayHandle1, true);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, DisplayHandle1, 1u, 1u, false));

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    mapScene(0u);
    mapScene(1u);
    mapScene(2u);
    showScene(0u);
    showScene(1u);
    showScene(2u);

    EXPECT_TRUE(assignSceneToOffscreenBuffer(0, buffer));
    EXPECT_TRUE(assignSceneToOffscreenBuffer(1, buffer));

    const DataSlotId consumerId = createTextureConsumer(2u);
    createBufferLink(buffer, getSceneId(2u), consumerId);
    update();

    update();
    expectNoScenesModified();

    //update scene mapped to buffer
    performFlushWithCreateNodeAction(1u);

    update();
    expectScenesModified({ 1u, 2u });

    update();
    expectNoScenesModified();

    hideScene(0u);
    hideScene(1u);
    hideScene(2u);
    expectContextEnable(DisplayHandle1, 3u);
    unmapScene(0u);
    unmapScene(1u);
    unmapScene(2u);
    expectRenderTargetDeleted();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, DoesNotMarkSceneAsModified_OffscreenBufferLinking_UnmodifiedProviderToOffscreenBuffer)
{
    // s0 -> ob1 -> s1 [modified]
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    expectContextEnable();
    expectRenderTargetUploaded(DisplayHandle1, true);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, DisplayHandle1, 1u, 1u, false));

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    mapScene(0u);
    mapScene(1u);
    showScene(0u);
    showScene(1u);

    EXPECT_TRUE(assignSceneToOffscreenBuffer(0, buffer));

    const DataSlotId consumerId = createTextureConsumer(1u);
    createBufferLink(buffer, stagingScene[1u]->getSceneId(), consumerId);
    update();

    update();
    expectNoScenesModified();

    //update consumer
    performFlushWithCreateNodeAction(1u);
    //expect only consumer modified
    update();
    expectScenesModified({ 1u });

    update();
    expectNoScenesModified();

    hideScene(0u);
    hideScene(1u);
    expectContextEnable(DisplayHandle1, 2u);
    unmapScene(0u);
    unmapScene(1u);
    expectRenderTargetDeleted();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarkSceneAsModified_OffscreenBufferLinking_TwoConsumersFromModifiedOffscreenBuffer)
{
    //                       --> s1 [modified]
    // s0 [modified] -> ob1-|
    //                       --> s2 [modified]
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    expectContextEnable();
    expectRenderTargetUploaded(DisplayHandle1, true);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, DisplayHandle1, 1u, 1u, false));

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    mapScene(0u);
    mapScene(1u);
    mapScene(2u);
    showScene(0u);
    showScene(1u);
    showScene(2u);

    EXPECT_TRUE(assignSceneToOffscreenBuffer(0, buffer));

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

    update();
    expectNoScenesModified();

    //update scene mapped to buffer
    performFlushWithCreateNodeAction(0u);
    //expect both scenes modified
    update();
    expectScenesModified({0u, 1u, 2u});

    update();
    expectNoScenesModified();

    hideScene(0u);
    hideScene(1u);
    hideScene(2u);
    expectContextEnable(DisplayHandle1, 3u);
    unmapScene(0u);
    unmapScene(1u);
    unmapScene(2u);
    expectRenderTargetDeleted();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarkSceneAsModified_OffscreenBufferLinking_IndirectConsumerFromModifiedOffscreenBuffer)
{
    // s0 [modified] -> ob1 -> s1 [modified] -> ob2 -> s2 [modified]
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer1(1u);
    expectContextEnable();
    expectRenderTargetUploaded(DisplayHandle1, true);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer1, DisplayHandle1, 1u, 1u, false));

    const OffscreenBufferHandle buffer2(2u);
    const DeviceResourceHandle offscreenBufferDeviceHandle(5556u);
    expectContextEnable();
    expectRenderTargetUploaded(DisplayHandle1, true, offscreenBufferDeviceHandle);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer2, DisplayHandle1, 1u, 1u, false));

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    mapScene(0u);
    mapScene(1u);
    mapScene(2u);
    showScene(0u);
    showScene(1u);
    showScene(2u);

    EXPECT_TRUE(assignSceneToOffscreenBuffer(0, buffer1));

    {
        EXPECT_TRUE(assignSceneToOffscreenBuffer(1, buffer2));
        const DataSlotId consumerId = createTextureConsumer(1u);
        createBufferLink(buffer1, stagingScene[1u]->getSceneId() , consumerId);
        update();
    }
    {
        const DataSlotId consumerId = createTextureConsumer(2u);
        createBufferLink(buffer2, stagingScene[2u]->getSceneId() , consumerId);
        update();
    }

    update();
    expectNoScenesModified();

    //update scene mapped to buffer 1
    performFlushWithCreateNodeAction(0u);
    //expect all scenes modified
    update();
    expectScenesModified({0u, 1u, 2u});

    update();
    expectNoScenesModified();

    hideScene(0u);
    hideScene(1u);
    hideScene(2u);
    expectContextEnable(DisplayHandle1, 3u);
    unmapScene(0u);
    unmapScene(1u);
    unmapScene(2u);
    expectRenderTargetDeleted(DisplayHandle1, 2u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, DoesNotMarkSceneAsModified_OffscreenBufferLinking_ConsumerFromUnmodifiedOffscreenBuffer)
{
    // s0 [modified] -> ob1
    // s1 -> ob2 -> s2
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer1(1u);
    expectContextEnable();
    expectRenderTargetUploaded(DisplayHandle1, true);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer1, DisplayHandle1, 1u, 1u, false));

    const OffscreenBufferHandle buffer2(2u);
    const DeviceResourceHandle offscreenBufferDeviceHandle(5556u);
    expectContextEnable();
    expectRenderTargetUploaded(DisplayHandle1, true, offscreenBufferDeviceHandle);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer2, DisplayHandle1, 1u, 1u, false));

    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    mapScene(0u);
    mapScene(1u);
    mapScene(2u);
    showScene(0u);
    showScene(1u);
    showScene(2u);

    EXPECT_TRUE(assignSceneToOffscreenBuffer(0, buffer1));
    EXPECT_TRUE(assignSceneToOffscreenBuffer(1, buffer2));

    const DataSlotId consumerId = createTextureConsumer(2u);
    createBufferLink(buffer2, stagingScene[2u]->getSceneId() , consumerId);
    update();

    update();
    expectNoScenesModified();

    //update scene mapped to buffer 1
    performFlushWithCreateNodeAction(0u);
    //expect only this scene to be modified
    update();
    expectScenesModified({0u});

    update();
    expectNoScenesModified();

    hideScene(0u);
    hideScene(1u);
    hideScene(2u);
    expectContextEnable(DisplayHandle1, 3u);
    unmapScene(0u);
    unmapScene(1u);
    unmapScene(2u);
    expectRenderTargetDeleted(DisplayHandle1, 2u);
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
        expectContextEnable();
        expectRenderTargetUploaded(DisplayHandle1, true, DeviceResourceHandle(buffer.asMemoryHandle()));
        EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, DisplayHandle1, 1u, 1u, false));
    }

    for (UInt32 i = 0u; i < 6; ++i)
    {
        createPublishAndSubscribeScene();
        mapScene(i);
        showScene(i);
    }

    EXPECT_TRUE(assignSceneToOffscreenBuffer(0, buffers[0]));
    EXPECT_TRUE(assignSceneToOffscreenBuffer(2, buffers[1]));
    EXPECT_TRUE(assignSceneToOffscreenBuffer(4, buffers[2]));

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

    update();
    expectNoScenesModified();

    //update s2
    performFlushWithCreateNodeAction(2u);

    update();
    expectScenesModified({ 2u, 3u, 4u, 5u });

    update();
    expectNoScenesModified();


    expectContextEnable(DisplayHandle1, 6u);
    for (UInt32 i = 0u; i < 6; ++i)
    {
        hideScene(i);
        unmapScene(i);
    }

    expectRenderTargetDeleted(DisplayHandle1, 3u);
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
        expectContextEnable();
        expectRenderTargetUploaded(DisplayHandle1, true, DeviceResourceHandle(buffer.asMemoryHandle()));
        EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, DisplayHandle1, 1u, 1u, false));
    }

    for (UInt32 i = 0u; i < 5; ++i)
    {
        createPublishAndSubscribeScene();
        mapScene(i);
        showScene(i);
    }

    EXPECT_TRUE(assignSceneToOffscreenBuffer(0, buffers[0]));
    EXPECT_TRUE(assignSceneToOffscreenBuffer(1, buffers[0]));
    EXPECT_TRUE(assignSceneToOffscreenBuffer(2, buffers[1]));
    EXPECT_TRUE(assignSceneToOffscreenBuffer(3, buffers[2]));

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

    update();
    expectNoScenesModified();

    //update s3
    performFlushWithCreateNodeAction(3u);

    update();
    expectScenesModified({ 3u, 4u });

    update();
    expectNoScenesModified();


    expectContextEnable(DisplayHandle1, 5u);
    for (UInt32 i = 0u; i < 5; ++i)
    {
        hideScene(i);
        unmapScene(i);
    }

    expectRenderTargetDeleted(DisplayHandle1, 3u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, MarksSceneAsModified_IfRealTimeAnimationIsActive)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    const auto animationHandle = createRealTimeActiveAnimation();

    PlatformThread::Sleep(1u);
    update();
    expectScenesModified();

    PlatformThread::Sleep(1u);
    update();
    expectScenesModified();

    stopAnimation(animationHandle);
    update();
    expectNoScenesModified();

    hideScene();
    expectContextEnable();
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
    expectContextEnable();
    expectRenderTargetUploaded(DisplayHandle1, true);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, DisplayHandle1, 1u, 1u, false));
    EXPECT_TRUE(assignSceneToOffscreenBuffer(scene2, buffer));

    const DataSlotId consumer = createTextureConsumer(scene1);

    showScene(scene1);
    showScene(scene2);

    update();
    expectNoScenesModified();

    createBufferLink(buffer, getSceneId(scene1), consumer);
    expectScenesModified({ scene1 });

    update();
    expectNoScenesModified();

    hideScene(scene1);
    hideScene(scene2);
    expectContextEnable();
    unmapScene(scene1);
    expectContextEnable();
    unmapScene(scene2);

    expectContextEnable();
    expectRenderTargetDeleted();
    EXPECT_TRUE(rendererSceneUpdater->handleBufferDestroyRequest(buffer, DisplayHandle1));

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
    expectContextEnable();
    expectRenderTargetUploaded(DisplayHandle1, true);
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, DisplayHandle1, 1u, 1u, false));
    EXPECT_TRUE(assignSceneToOffscreenBuffer(scene2, buffer));

    const DataSlotId consumer = createTextureConsumer(scene1);

    showScene(scene1);
    showScene(scene2);

    update();
    expectNoScenesModified();

    createBufferLink(buffer, getSceneId(scene1), consumer);
    expectScenesModified({ scene1 });

    update();
    expectNoScenesModified();

    unlinkConsumer(getSceneId(scene1), consumer);
    expectScenesModified({ scene1 });

    update();
    expectNoScenesModified();

    hideScene(scene1);
    hideScene(scene2);
    expectContextEnable();
    unmapScene(scene1);
    expectContextEnable();
    unmapScene(scene2);

    expectContextEnable();
    expectRenderTargetDeleted();
    EXPECT_TRUE(rendererSceneUpdater->handleBufferDestroyRequest(buffer, DisplayHandle1));

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
    expectNoScenesModified();

    linkProviderToConsumer(getSceneId(providerScene), providerConsumer.first, getSceneId(consumerScene), providerConsumer.second);
    expectScenesModified({ consumerScene });

    update();
    expectNoScenesModified();

    unlinkConsumer(getSceneId(consumerScene), providerConsumer.second);
    expectScenesModified({ consumerScene });

    update();
    expectNoScenesModified();

    hideScene(providerScene);
    hideScene(consumerScene);
    expectContextEnable();
    unmapScene(providerScene);
    expectContextEnable();
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
    expectNoScenesModified();

    linkProviderToConsumer(getSceneId(providerScene), providerConsumer.first, getSceneId(consumerScene), providerConsumer.second);
    expectScenesModified({ consumerScene });

    update();
    expectNoScenesModified();

    unlinkConsumer(getSceneId(consumerScene), providerConsumer.second);
    expectScenesModified({ consumerScene });

    update();
    expectNoScenesModified();

    hideScene(providerScene);
    hideScene(consumerScene);
    expectContextEnable();
    unmapScene(providerScene);
    expectContextEnable();
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

    expectResourceRequest();
    const auto providerConsumer = createTextureSlots(nullptr, providerScene, consumerScene);
    update();
    expectNoScenesModified();

    linkProviderToConsumer(getSceneId(providerScene), providerConsumer.first, getSceneId(consumerScene), providerConsumer.second);
    expectScenesModified({ consumerScene });

    update();
    expectNoScenesModified();

    unlinkConsumer(getSceneId(consumerScene), providerConsumer.second);
    expectScenesModified({ consumerScene });

    update();
    expectNoScenesModified();

    hideScene(providerScene);
    hideScene(consumerScene);
    expectContextEnable();
    unmapScene(providerScene);
    expectContextEnable();
    unmapScene(consumerScene);
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
    expectContextEnable();
    unmapScene(scene);
    expectContextEnable();
    unmapScene(sceneInterrupted);

    expectRenderTargetDeleted(DisplayHandle1, 1u, true);
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
    expectContextEnable();
    unmapScene(scene);
    expectContextEnable();
    unmapScene(sceneInterrupted);

    expectRenderTargetDeleted(DisplayHandle1, 1u, true);
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
    expectContextEnable();
    unmapScene(scene);
    expectContextEnable();
    unmapScene(sceneInterrupted);
    expectContextEnable();
    unmapScene(sceneToShow);

    expectRenderTargetDeleted(DisplayHandle1, 1u, true);
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
    expectContextEnable();
    unmapScene(scene);
    expectContextEnable();
    unmapScene(sceneInterrupted);
    expectContextEnable();
    unmapScene(sceneToMap);

    expectRenderTargetDeleted(DisplayHandle1, 1u, true);
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

    expectContextEnable();
    unmapScene(sceneToUnmap);
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(scene);
    hideScene(sceneInterrupted);
    expectContextEnable();
    unmapScene(scene);
    expectContextEnable();
    unmapScene(sceneInterrupted);

    expectRenderTargetDeleted(DisplayHandle1, 1u, true);
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

    EXPECT_TRUE(assignSceneToFramebuffer(sceneInterrupted));
    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(scene);
    hideScene(sceneInterrupted);
    expectContextEnable();
    unmapScene(scene);
    expectContextEnable();
    unmapScene(sceneInterrupted);

    expectRenderTargetDeleted(DisplayHandle1, 1u, true);
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
    expectContextEnable();
    expectRenderTargetUploaded(DisplayHandle1, true, DeviceResourceHandle(321u));
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, DisplayHandle1, 1u, 1u, false));
    EXPECT_TRUE(assignSceneToOffscreenBuffer(sceneToAssign, buffer));

    showAndInitiateInterruptedRendering(scene, sceneInterrupted);
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    EXPECT_TRUE(assignSceneToFramebuffer(sceneToAssign));
    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(scene);
    hideScene(sceneInterrupted);
    expectContextEnable();
    unmapScene(scene);
    expectContextEnable();
    unmapScene(sceneInterrupted);
    expectContextEnable();
    unmapScene(sceneToAssign);

    expectContextEnable();
    expectRenderTargetDeleted();
    EXPECT_TRUE(rendererSceneUpdater->handleBufferDestroyRequest(buffer, DisplayHandle1));

    expectRenderTargetDeleted(DisplayHandle1, 1u, true);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, resetsInterruptedRenderingIfAnotherSceneAssignedToOffscreenBuffer)
{
    createDisplayAndExpectSuccess();

    const UInt32 scene = createPublishAndSubscribeScene();
    const UInt32 sceneInterrupted = createPublishAndSubscribeScene();

    mapScene(scene);
    mapScene(sceneInterrupted);

    const OffscreenBufferHandle buffer = showAndInitiateInterruptedRendering(scene, sceneInterrupted);
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    EXPECT_TRUE(assignSceneToOffscreenBuffer(scene, buffer));
    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(scene);
    hideScene(sceneInterrupted);
    expectContextEnable();
    unmapScene(scene);
    expectContextEnable();
    unmapScene(sceneInterrupted);

    expectRenderTargetDeleted(DisplayHandle1, 1u, true);
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

    const OffscreenBufferHandle buffer = showAndInitiateInterruptedRendering(scene, sceneInterrupted);
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    createBufferLink(buffer, getSceneId(scene), texConsumer);
    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(scene);
    hideScene(sceneInterrupted);
    expectContextEnable();
    unmapScene(scene);
    expectContextEnable();
    unmapScene(sceneInterrupted);

    expectRenderTargetDeleted(DisplayHandle1, 1u, true);
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
    expectContextEnable();
    expectRenderTargetUploaded(DisplayHandle1, true, DeviceResourceHandle(321u));
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, DisplayHandle1, 1u, 1u, false));
    const DataSlotId texConsumer = createTextureConsumer(sceneToUnlink);
    createBufferLink(buffer, getSceneId(sceneToUnlink), texConsumer);

    showAndInitiateInterruptedRendering(scene, sceneInterrupted);
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    unlinkConsumer(getSceneId(sceneToUnlink), texConsumer);
    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(scene);
    hideScene(sceneInterrupted);
    expectContextEnable();
    unmapScene(scene);
    expectContextEnable();
    unmapScene(sceneInterrupted);
    expectContextEnable();
    unmapScene(sceneToUnlink);

    expectContextEnable();
    expectRenderTargetDeleted();
    EXPECT_TRUE(rendererSceneUpdater->handleBufferDestroyRequest(buffer, DisplayHandle1));

    expectRenderTargetDeleted(DisplayHandle1, 1u, true);
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
    expectContextEnable();
    unmapScene(scene);
    expectContextEnable();
    unmapScene(sceneInterrupted);

    expectRenderTargetDeleted(DisplayHandle1, 1u, true);
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
    expectContextEnable();
    unmapScene(scene);
    expectContextEnable();
    unmapScene(sceneInterrupted);

    expectRenderTargetDeleted(DisplayHandle1, 1u, true);
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
    expectContextEnable();
    unmapScene(scene);
    expectContextEnable();
    unmapScene(sceneInterrupted);

    expectRenderTargetDeleted(DisplayHandle1, 1u, true);
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
    expectContextEnable();
    unmapScene(scene);
    expectContextEnable();
    unmapScene(sceneInterrupted);

    expectRenderTargetDeleted(DisplayHandle1, 1u, true);
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
    expectResourceRequest();
    const auto providerConsumerId = createTextureSlots(nullptr, scene, sceneInterrupted);

    showAndInitiateInterruptedRendering(scene, sceneInterrupted);
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    linkProviderToConsumer(getSceneId(scene), providerConsumerId.first, getSceneId(sceneInterrupted), providerConsumerId.second);
    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(scene);
    hideScene(sceneInterrupted);
    expectContextEnable();
    unmapScene(scene);
    expectContextEnable();
    unmapScene(sceneInterrupted);

    expectRenderTargetDeleted(DisplayHandle1, 1u, true);
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
    expectResourceRequest();
    const auto providerConsumerId = createTextureSlots(nullptr, scene, sceneInterrupted);
    linkProviderToConsumer(getSceneId(scene), providerConsumerId.first, getSceneId(sceneInterrupted), providerConsumerId.second);

    showAndInitiateInterruptedRendering(scene, sceneInterrupted);
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    unlinkConsumer(getSceneId(sceneInterrupted), providerConsumerId.second);
    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(scene);
    hideScene(sceneInterrupted);
    expectContextEnable();
    unmapScene(scene);
    expectContextEnable();
    unmapScene(sceneInterrupted);

    expectRenderTargetDeleted(DisplayHandle1, 1u, true);
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

    const OffscreenBufferHandle buffer(1u);
    expectContextEnable();
    expectRenderTargetUploaded(DisplayHandle1, true, DeviceResourceHandle(321u));
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, DisplayHandle1, 1u, 1u, false));

    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(scene);
    hideScene(sceneInterrupted);
    expectContextEnable();
    unmapScene(scene);
    expectContextEnable();
    unmapScene(sceneInterrupted);

    expectContextEnable();
    expectRenderTargetDeleted();
    EXPECT_TRUE(rendererSceneUpdater->handleBufferDestroyRequest(buffer, DisplayHandle1));

    expectRenderTargetDeleted(DisplayHandle1, 1u, true);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, resetsInterruptedRenderingIfOffscreenBufferDeleted)
{
    createDisplayAndExpectSuccess();

    const OffscreenBufferHandle buffer(1u);
    expectContextEnable();
    expectRenderTargetUploaded(DisplayHandle1, true, DeviceResourceHandle(321u));
    EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, DisplayHandle1, 1u, 1u, false));

    const UInt32 scene = createPublishAndSubscribeScene();
    const UInt32 sceneInterrupted = createPublishAndSubscribeScene();

    mapScene(scene);
    mapScene(sceneInterrupted);

    showAndInitiateInterruptedRendering(scene, sceneInterrupted);
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    expectContextEnable();
    expectRenderTargetDeleted();
    EXPECT_TRUE(rendererSceneUpdater->handleBufferDestroyRequest(buffer, DisplayHandle1));

    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(scene);
    hideScene(sceneInterrupted);
    expectContextEnable();
    unmapScene(scene);
    expectContextEnable();
    unmapScene(sceneInterrupted);

    expectRenderTargetDeleted(DisplayHandle1, 1u, true);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, resetsInterruptedRenderingIfDisplayCreated)
{
    createDisplayAndExpectSuccess(DisplayHandle1);

    const UInt32 scene = createPublishAndSubscribeScene();
    const UInt32 sceneInterrupted = createPublishAndSubscribeScene();

    mapScene(scene);
    mapScene(sceneInterrupted);

    showAndInitiateInterruptedRendering(scene, sceneInterrupted);
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    createDisplayAndExpectSuccess(DisplayHandle2);

    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(scene);
    hideScene(sceneInterrupted);
    expectContextEnable();
    unmapScene(scene);
    expectContextEnable();
    unmapScene(sceneInterrupted);

    destroyDisplay(DisplayHandle2);
    expectRenderTargetDeleted(DisplayHandle1, 1u, true);
    destroyDisplay(DisplayHandle1);
}

TEST_F(ARendererSceneUpdater, resetsInterruptedRenderingIfDisplayDestroyed)
{
    createDisplayAndExpectSuccess(DisplayHandle1);
    createDisplayAndExpectSuccess(DisplayHandle2);

    const UInt32 scene = createPublishAndSubscribeScene();
    const UInt32 sceneInterrupted = createPublishAndSubscribeScene();

    mapScene(scene);
    mapScene(sceneInterrupted);

    // just to match mocks for the second display
    DisplayStrictMockInfo& displayMock = renderer.getDisplayMock(DisplayHandle2);
    EXPECT_CALL(*displayMock.m_displayController, handleWindowEvents());
    EXPECT_CALL(*displayMock.m_displayController, canRenderNewFrame()).WillOnce(Return(false));

    showAndInitiateInterruptedRendering(scene, sceneInterrupted);
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    destroyDisplay(DisplayHandle2);

    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(scene);
    hideScene(sceneInterrupted);
    expectContextEnable();
    unmapScene(scene);
    expectContextEnable();
    unmapScene(sceneInterrupted);

    expectRenderTargetDeleted(DisplayHandle1, 1u, true);
    destroyDisplay(DisplayHandle1);
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

    performFlush(sceneInterrupted); //async flush
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene(sceneInterrupted));

    performFlush(sceneInterrupted, true); //sync flush
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene(sceneInterrupted));

    renderer.resetRenderInterruptState();
    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());

    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene(sceneInterrupted));

    hideScene(scene);
    hideScene(sceneInterrupted);
    expectContextEnable();
    unmapScene(scene);
    expectContextEnable();
    unmapScene(sceneInterrupted);

    expectRenderTargetDeleted(DisplayHandle1, 1u, true);
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

    performFlush(scene); //async flush
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene(sceneInterrupted));

    performFlush(scene, true); //sync flush
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene(sceneInterrupted));

    renderer.resetRenderInterruptState();
    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());

    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene(scene));

    hideScene(scene);
    hideScene(sceneInterrupted);
    expectContextEnable();
    unmapScene(scene);
    expectContextEnable();
    unmapScene(sceneInterrupted);

    expectRenderTargetDeleted(DisplayHandle1, 1u, true);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, resetsInterruptedRenderingIfMaximumNumberOfPendingFlushesReached)
{
    createDisplayAndExpectSuccess();
    const UInt32 scene = createPublishAndSubscribeScene();
    const UInt32 sceneInterrupted = createPublishAndSubscribeScene();

    createRenderable(sceneInterrupted);
    setRenderableResources(sceneInterrupted);

    expectResourceRequest(DisplayHandle1, sceneInterrupted);
    expectContextEnable();
    expectRenderableResourcesUploaded();

    mapScene(scene);
    mapScene(sceneInterrupted);

    showAndInitiateInterruptedRendering(scene, sceneInterrupted);
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    // add blocking sync flush so that upcoming flushes are queuing up
    setRenderableResources(sceneInterrupted, InvalidResource1, true);
    expectResourceRequest(DisplayHandle1, sceneInterrupted);
    update();

    // flushes are blocked due to unresolved resource
    const SceneVersionTag pendingFlushTag(124u);
    performFlush(sceneInterrupted, true, pendingFlushTag);
    update();
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

    for (UInt i = 0u; i < ForceApplyFlushesLimit + 1u; ++i)
    {
        performFlush(sceneInterrupted, true);
        update();
    }

    // after maximum of pending flushes was reached the flushes were applied regardless of missing resource
    expectEvent(ERendererEventType_SceneFlushed);
    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());

    hideScene(scene);
    hideScene(sceneInterrupted);
    expectContextEnable();
    expectRenderableResourcesDeleted();
    unmapScene(scene);
    expectContextEnable();
    unmapScene(sceneInterrupted);

    expectRenderTargetDeleted(DisplayHandle1, 1u, true);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, reportsExpirationForSubscribedSceneButWithNoFurtherFlushes)
{
    const UInt32 scene = createPublishAndSubscribeScene(false);
    // initial flush to init monitoring
    performFlushWithExpiration(scene, 1000u + 1u);
    expectEvents({ ERendererEventType_SceneSubscribed });

    // no further flush, no expiration updates
    for (UInt32 i = 0u; i < 10u; ++i)
    {
        update();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }

    expectEvent(ERendererEventType_SceneExpired);
}

TEST_F(ARendererSceneUpdater, doesNotReportExpirationForNotShownSceneBeingFlushedRegularly)
{
    const UInt32 scene = createPublishAndSubscribeScene();
    update();

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
    update();

    // flush once only to set limit
    performFlushWithExpiration(scene, 1000u + 2u);

    for (UInt32 i = 0u; i < 10u; ++i)
    {
        update();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }

    expectEvent(ERendererEventType_SceneExpired);
}

TEST_F(ARendererSceneUpdater, reportsRecoveryAfterExpiredForNotShownScene)
{
    const UInt32 scene = createPublishAndSubscribeScene();
    update();

    // flush once only to set limit
    performFlushWithExpiration(scene, 1000u + 2u);

    for (UInt32 i = 0u; i < 10u; ++i)
    {
        update();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }
    // expect exactly one event
    expectEvent(ERendererEventType_SceneExpired);

    for (UInt32 i = 10u; i < 20u; ++i)
    {
        performFlushWithExpiration(scene, 1000u + i + 2u);
        update();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }
    // expect exactly one event
    expectEvent(ERendererEventType_SceneRecoveredFromExpiration);
}

TEST_F(ARendererSceneUpdater, doesNotReportExpirationForSceneBeingFlushedAndRenderedRegularly)
{
    createDisplayAndExpectSuccess();
    const UInt32 scene = createPublishAndSubscribeScene();
    mapScene();
    showScene();

    for (UInt32 i = 0u; i < 10u; ++i)
    {
        performFlushWithExpiration(scene, 1000u + i + 2u);
        update();
        expirationMonitor.onRendered(getSceneId());
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
        expectNoEvent();
    }

    hideScene();
    expectContextEnable();
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
    expectEvent(ERendererEventType_SceneExpired);

    hideScene();
    expectContextEnable();
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

    expectContextEnable();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, reportsExpirationForSceneNotBeingFlushedOnlyRendered)
{
    const UInt32 scene = createPublishAndSubscribeScene();
    update();

    // flush once only to set limit
    performFlushWithExpiration(scene, 1000u + 2u);

    for (UInt32 i = 0u; i < 10u; ++i)
    {
        update();
        expirationMonitor.onRendered(getSceneId());
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }

    expectEvent(ERendererEventType_SceneExpired);
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
    expectEvent(ERendererEventType_SceneExpired);

    for (UInt32 i = 0u; i < ForceApplyFlushesLimit / 2u; ++i)
    {
        performFlushWithExpiration(scene, 1000u + i + 2u);
        update();
        expirationMonitor.onRendered(getSceneId());
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }
    expectEvent(ERendererEventType_SceneRecoveredFromExpiration);

    hideScene();
    expectContextEnable();
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
    expectEvent(ERendererEventType_SceneExpired);

    hideScene();

    for (UInt32 i = 0u; i < ForceApplyFlushesLimit / 2u; ++i)
    {
        iscene.setTranslation(transform, {}); // so that flush is not 'empty' - modifies scene
        performFlushWithExpiration(scene, 1000u + i + 2u);
        update();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }
    expectEvent(ERendererEventType_SceneRecoveredFromExpiration);

    expectContextEnable();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, doesNotReportExpirationForSceneBeingFlushedRegularlyButSkippedRenderingDueToEmptyFlushes)
{
    createDisplayAndExpectSuccess();
    const UInt32 scene = createPublishAndSubscribeScene();
    mapScene();
    showScene();

    for (UInt32 i = 0u; i < ForceApplyFlushesLimit / 2u; ++i)
    {
        performFlushWithExpiration(scene, 1000u + i + 2u);
        update();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
        expectNoEvent();
    }

    hideScene();
    expectContextEnable();
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

    createRenderable(scene);
    setRenderableResources(scene, InvalidResource1, true);
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();

    for (UInt32 i = 0u; i < ForceApplyFlushesLimit / 2u; ++i)
    {
        performFlushWithExpiration(scene, 1000u + i + 2u); // blocked due to missing resource
        update();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }
    expectEvent(ERendererEventType_SceneExpired);

    expectContextEnable();
    expectRenderableResourcesDeleted(DisplayHandle1, true, false);
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

    createRenderable(scene);
    setRenderableResources(scene, InvalidResource1, true);
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();

    for (UInt32 i = 0u; i < ForceApplyFlushesLimit / 2; ++i)
    {
        performFlushWithExpiration(scene, 1000u + i + 2u); // blocked due to missing resource
        update();
        expirationMonitor.onRendered(getSceneId());
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }
    expectEvent(ERendererEventType_SceneExpired);

    hideScene();
    expectContextEnable();
    expectRenderableResourcesDeleted(DisplayHandle1, true, false);
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, reportsRecoveryAfterExpirationForSceneBeingFlushedAndRenderedButBlockedByMissingResource)
{
    createDisplayAndExpectSuccess();
    const UInt32 scene = createPublishAndSubscribeScene();
    mapScene();
    showScene();

    // flush with expiration info to initiate monitoring
    performFlushWithExpiration(scene, 1000u + 2u);
    update();

    createRenderable(scene);
    setRenderableResources(scene, InvalidResource1, true);
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, true, false);
    update();

    for (UInt32 i = 0u; i < ForceApplyFlushesLimit / 2u; ++i)
    {
        performFlushWithExpiration(scene, 1000u + i + 2u); // blocked due to missing resource
        update();
        expirationMonitor.onRendered(getSceneId());
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }
    expectEvent(ERendererEventType_SceneExpired);

    setRenderableResources(); // simulate upload
    expectResourceRequest();
    expectContextEnable();
    expectRenderableResourcesUploaded(DisplayHandle1, false, true);
    update();
    expectResourceRequestCancel(InvalidResource1);

    for (UInt32 i = ForceApplyFlushesLimit / 2u; i < ForceApplyFlushesLimit; ++i)
    {
        performFlushWithExpiration(scene, 1000u + i + 2u); // not blocked anymore
        update();
        expirationMonitor.onRendered(getSceneId());
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }
    expectEvent(ERendererEventType_SceneRecoveredFromExpiration);

    hideScene();
    expectContextEnable();
    expectRenderableResourcesDeleted();
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

    for (UInt32 i = 0u; i < 10u; ++i)
    {
        update();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(i)));
    }
    expectNoEvent();

    // disable expiration
    performFlushWithExpiration(scene, 0u);

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

    for (UInt32 i = 0u; i < 10u; ++i)
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

    for (UInt32 i = 0u; i < 10u; ++i)
    {
        update();
        doRenderLoop();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(i)));
    }
    expectNoEvent();

    // disable expiration
    performFlushWithExpiration(scene, 0u);

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
    expectContextEnable();
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

    for (UInt32 i = 0u; i < 10u; ++i)
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
    expectContextEnable();
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

    for (UInt32 i = 0u; i < 10u; ++i)
    {
        update();
        doRenderLoop();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(i)));
    }
    expectNoEvent();

    hideScene(scene);

    // disable expiration
    performFlushWithExpiration(scene, 0u);

    for (UInt32 i = 0u; i < 10u; ++i)
    {
        update();
        doRenderLoop();
        // check with TS after initial TS
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(initialTS + i)));
    }
    // expect no expiration as last flush disabled expiration
    expectNoEvent();

    expectContextEnable();
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

    for (UInt32 i = 0u; i < 10u; ++i)
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

    for (UInt32 i = 0u; i < 10u; ++i)
    {
        update();
        doRenderLoop();
        // check with TS after initial TS
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(initialTS + i)));
    }
    // expect no expiration as last flush disabled expiration
    expectNoEvent();

    expectContextEnable();
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

    for (UInt32 i = 10u; i < 20u; ++i)
    {
        update();
        doRenderLoop();
        // check with TS after initial TS
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(initialTS + i)));
    }
    // expect no expiration as last flush disabled expiration
    expectNoEvent();

    expectContextEnable();
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

    for (UInt32 i = 10u; i < 20u; ++i)
    {
        update();
        doRenderLoop();
        // check with TS after initial TS
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(initialTS + i)));
    }
    // expect no expiration as last flush disabled expiration
    expectNoEvent();

    expectContextEnable();
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

    for (UInt32 i = 10u; i < 20u; ++i)
    {
        update();
        doRenderLoop();
        // check with TS after initial TS
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(initialTS + i)));
    }
    // expect no expiration as last flush disabled expiration
    expectNoEvent();

    expectContextEnable();
    unmapScene(scene);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, reportsRecoveryAfterExpiration_byDisablingExpiration)
{
    const UInt32 scene = createPublishAndSubscribeScene();

    for (UInt32 i = 0u; i < 5u; ++i)
    {
        performFlushWithExpiration(scene, 1000u + 2u); // will expire
        update();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }
    expectEvent(ERendererEventType_SceneExpired);

    // disable expiration
    performFlushWithExpiration(scene, 0u);

    for (UInt32 i = 0u; i < 5u; ++i)
    {
        update();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }
    expectEvent(ERendererEventType_SceneRecoveredFromExpiration);
}

TEST_F(ARendererSceneUpdater, reportsRecoveryAfterExpirationOfRenderedContent_byDisablingExpiration)
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
    expectEvent(ERendererEventType_SceneExpired);

    // disable expiration
    performFlushWithExpiration(scene, 0u);

    for (UInt32 i = 0u; i < 5u; ++i)
    {
        update();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }
    expectEvent(ERendererEventType_SceneRecoveredFromExpiration);

    hideScene(scene);
    expectContextEnable();
    unmapScene(scene);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, reportsRecoveryAfterExpiration_byDisablingExpirationWithNonEmptyFlush)
{
    const UInt32 scene = createPublishAndSubscribeScene();

    for (UInt32 i = 0u; i < 5u; ++i)
    {
        performFlushWithExpiration(scene, 1000u + 2u); // will expire
        update();
        expirationMonitor.checkExpiredScenes(FlushTime::Clock::time_point(std::chrono::milliseconds(1000u + i)));
    }
    expectEvent(ERendererEventType_SceneExpired);

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
    expectEvent(ERendererEventType_SceneRecoveredFromExpiration);
}

TEST_F(ARendererSceneUpdater, reportsRecoveryAfterExpirationOfRenderedContent_byDisablingExpirationWithNonEmptyFlush)
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
    expectEvent(ERendererEventType_SceneExpired);

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
    expectEvent(ERendererEventType_SceneRecoveredFromExpiration);

    hideScene(scene);
    expectContextEnable();
    unmapScene(scene);
    destroyDisplay();
}

}
