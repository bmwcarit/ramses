//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "ResourceProviderMock.h"
#include "RendererMock.h"
#include "DisplayControllerMock.h"
#include "RenderBackendMock.h"
#include "ResourceUploaderMock.h"
#include "RendererLib/RendererCommandExecutor.h"
#include "RendererLib/RendererLogContext.h"
#include "RendererLib/RendererResourceManager.h"
#include "RendererLib/ResourceUploader.h"
#include "RendererLib/RendererCommandBuffer.h"
#include "RendererLib/SceneStateExecutor.h"
#include "RendererLib/RendererScenes.h"
#include "RendererLib/DisplayEventHandlerManager.h"
#include "RendererLib/FrameTimer.h"
#include "RendererLib/SceneExpirationMonitor.h"
#include "RendererEventCollector.h"
#include "ComponentMocks.h"
#include "RendererSceneUpdaterMock.h"
#include "SystemCompositorControllerMock.h"
#include "EmbeddedCompositingManagerMock.h"
#include "gmock/gmock-generated-nice-strict.h"
#include "SceneActionCollectionTestHelpers.h"

namespace ramses_internal {

class ARendererCommandExecutor : public ::testing::TestWithParam<bool>
{
public:
    ARendererCommandExecutor()
        : m_platformFactoryMock(GetParam())
        , m_rendererScenes(m_rendererEventCollector)
        , m_expirationMonitor(m_rendererScenes, m_rendererEventCollector)
        , m_renderer(m_platformFactoryMock, m_rendererScenes, m_rendererEventCollector, m_expirationMonitor, m_rendererStatistics)
        , m_sceneStateExecutor(m_renderer, m_sceneGraphConsumerComponent, m_rendererEventCollector)
        , m_sceneUpdater(m_renderer, m_rendererScenes, m_sceneStateExecutor, m_rendererEventCollector, m_frameTimer, m_expirationMonitor)
        , m_commandExecutor(m_renderer, m_commandBuffer, m_sceneUpdater, m_rendererEventCollector, m_frameTimer)
        , m_resourceUploader(m_renderer.getStatistics())
        , m_renderable(1u)
        , m_filename1("somefilename1")
        , m_filename2("somefilename2")
        , m_filename3("somefilename3")
    {
    }

    virtual ~ARendererCommandExecutor()
    {
    }

    void doCommandExecutorLoop()
    {
        m_renderer.getProfilerStatistics().markFrameFinished(std::chrono::microseconds{ 0u });
        m_commandExecutor.executePendingCommands();
    }

    void expectReadPixels(DisplayHandle handle = DisplayHandle(0u), UInt32 times = 1u)
    {
        StrictMock<DisplayControllerMock>& displayControllerMock = getDisplayControllerMock(handle);
        EXPECT_CALL(displayControllerMock, getDisplayWidth()).Times(times);
        EXPECT_CALL(displayControllerMock, getDisplayHeight()).Times(times);
        EXPECT_CALL(displayControllerMock, readPixels(_, _, _, _, _)).Times(times);
    }

    void expectReadPixelsInRenderLoop(DisplayHandle handle = DisplayHandle(0u))
    {
        StrictMock<DisplayControllerMock>& displayControllerMock = getDisplayControllerMock(handle);
        EXPECT_CALL(displayControllerMock, getDisplayWidth()).Times(AnyNumber());
        EXPECT_CALL(displayControllerMock, getDisplayHeight()).Times(AnyNumber());
        expectSceneRenderingInRenderLoop(handle);
    }

    void expectSceneRenderingInRenderLoop(DisplayHandle handle = DisplayHandle(0u), const Vector4& clearColor = Renderer::DefaultClearColor)
    {
        StrictMock<DisplayControllerMock>& displayControllerMock = getDisplayControllerMock(handle);
        EXPECT_CALL(displayControllerMock, enableContext());
        EXPECT_CALL(displayControllerMock, executePostProcessing());
        EXPECT_CALL(displayControllerMock, swapBuffers());
        EXPECT_CALL(displayControllerMock, clearBuffer(_, clearColor));
        EmbeddedCompositingManagerMock& embeddedCompositorMock = static_cast<EmbeddedCompositingManagerMock&>(displayControllerMock.getEmbeddedCompositingManager());
        EXPECT_CALL(embeddedCompositorMock, notifyClients());
    }

    DisplayHandle addDisplayController()
    {
        static const DisplayConfig dummyConfig;
        const DisplayHandle displayHandle(m_renderer.getDisplayControllerCount());
        m_commandBuffer.createDisplay(dummyConfig, m_resourceProvider, m_resourceUploader, displayHandle);
        doCommandExecutorLoop();

        RendererEventVector events;
        m_rendererEventCollector.dispatchEvents(events);
        EXPECT_EQ(1u, events.size());
        EXPECT_EQ(displayHandle, events.front().displayHandle);
        EXPECT_EQ(ERendererEventType_DisplayCreated, events.front().eventType);

        EXPECT_CALL(getDisplayControllerMock(displayHandle), getRenderBackend()).Times(AnyNumber());
        EXPECT_CALL(getRenderBackendMock(displayHandle).surfaceMock, enable()).Times(AnyNumber());

        return displayHandle;
    }

    void removeDisplayController(DisplayHandle handle)
    {
        m_commandBuffer.destroyDisplay(handle);
        doCommandExecutorLoop();

        RendererEventVector events;
        m_rendererEventCollector.dispatchEvents(events);
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(handle, events.front().displayHandle);
        EXPECT_EQ(ERendererEventType_DisplayDestroyed, events.front().eventType);
    }

    void createScene(SceneId sceneId)
    {
        const Guid clientID(true);
        m_commandBuffer.publishScene(sceneId, clientID, EScenePublicationMode_LocalAndRemote);
        m_commandBuffer.subscribeScene(sceneId);
        m_commandBuffer.receiveScene(SceneInfo(sceneId));

        //receive initial flush
        SceneActionCollection sceneActions;
        SceneActionCollectionCreator creator(sceneActions);
        creator.flush(1u, false, false);
        m_commandBuffer.enqueueActionsForScene(sceneId, sceneActions.copy());

        EXPECT_CALL(m_sceneGraphConsumerComponent, subscribeScene(clientID, sceneId));
        EXPECT_CALL(m_sceneUpdater, handleSceneActions(sceneId, SceneActionCollectionEq(sceneActions)));
        doCommandExecutorLoop();

        RendererEventVector events;
        m_rendererEventCollector.dispatchEvents(events);
        ASSERT_EQ(2u, events.size());
        EXPECT_EQ(ERendererEventType_ScenePublished, events[0].eventType);
        EXPECT_EQ(sceneId, events[0].sceneId);
        EXPECT_EQ(ERendererEventType_SceneSubscribed, events[1].eventType);
        EXPECT_EQ(sceneId, events[1].sceneId);
    }

    void updateScenes()
    {
        m_renderer.getProfilerStatistics().markFrameFinished(std::chrono::microseconds{ 0u });
        m_sceneUpdater.updateScenes();
    }

    void mapScene(SceneId sceneId, DisplayHandle display)
    {
        m_commandBuffer.mapSceneToDisplay(sceneId, display, 0);
        doCommandExecutorLoop();
        updateScenes();
        updateScenes();

        RendererEventVector events;
        m_rendererEventCollector.dispatchEvents(events);
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType_SceneMapped, events[0].eventType);
        EXPECT_EQ(sceneId, events[0].sceneId);
    }

    void showScene(SceneId sceneId)
    {
        m_commandBuffer.showScene(sceneId);
        doCommandExecutorLoop();

        RendererEventVector events;
        m_rendererEventCollector.dispatchEvents(events);
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType_SceneShown, events[0].eventType);
        EXPECT_EQ(sceneId, events[0].sceneId);
    }

    void createOffscreenBuffer(OffscreenBufferHandle buffer, DisplayHandle displayHandle, bool interruptible = false)
    {
        m_commandBuffer.createOffscreenBuffer(buffer, displayHandle, 1u, 1u, interruptible);

        // interruptible OB is double buffered
        const UInt32 callCountExpectation = interruptible ? 2u : 1u;
        EXPECT_CALL(getRenderBackendMock(displayHandle).deviceMock, uploadRenderBuffer(_)).Times(1 + callCountExpectation);
        EXPECT_CALL(getRenderBackendMock(displayHandle).deviceMock, uploadRenderTarget(_)).Times(callCountExpectation);
        EXPECT_CALL(getRenderBackendMock(displayHandle).deviceMock, activateRenderTarget(_)).Times(callCountExpectation);
        EXPECT_CALL(getRenderBackendMock(displayHandle).deviceMock, colorMask(true, true, true, true)).Times(callCountExpectation);
        EXPECT_CALL(getRenderBackendMock(displayHandle).deviceMock, clearColor(Vector4{ 0.f, 0.f, 0.f, 1.f })).Times(callCountExpectation);
        EXPECT_CALL(getRenderBackendMock(displayHandle).deviceMock, depthWrite(EDepthWrite::Enabled)).Times(callCountExpectation);
        RenderState::ScissorRegion scissorRegion{};
        EXPECT_CALL(getRenderBackendMock(displayHandle).deviceMock, scissorTest(EScissorTest::Disabled, scissorRegion)).Times(callCountExpectation);
        EXPECT_CALL(getRenderBackendMock(displayHandle).deviceMock, clear(_)).Times(callCountExpectation);
        if (interruptible)
            EXPECT_CALL(getRenderBackendMock(displayHandle).deviceMock, pairRenderTargetsForDoubleBuffering(_, _));
        doCommandExecutorLoop();

        RendererEventVector events;
        m_rendererEventCollector.dispatchEvents(events);
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType_OffscreenBufferCreated, events.front().eventType);
        EXPECT_EQ(displayHandle, events.front().displayHandle);
        EXPECT_EQ(buffer, events.front().offscreenBuffer);
    }

    void destroyOffscreenBuffer(OffscreenBufferHandle buffer, DisplayHandle displayHandle, bool interruptible = false)
    {
        m_commandBuffer.destroyOffscreenBuffer(buffer, displayHandle);

        // interruptible OB is double buffered
        EXPECT_CALL(getRenderBackendMock(displayHandle).deviceMock, deleteRenderBuffer(_)).Times(interruptible ? 3u : 2u);
        EXPECT_CALL(getRenderBackendMock(displayHandle).deviceMock, deleteRenderTarget(_)).Times(interruptible ? 2u : 1u);
        EXPECT_CALL(getRenderBackendMock(displayHandle).deviceMock, unpairRenderTargets(_)).Times(interruptible ? 1u : 0u);
        doCommandExecutorLoop();

        RendererEventVector events;
        m_rendererEventCollector.dispatchEvents(events);
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType_OffscreenBufferDestroyed, events.front().eventType);
        EXPECT_EQ(displayHandle, events.front().displayHandle);
        EXPECT_EQ(buffer, events.front().offscreenBuffer);
    }

    void hideScene(SceneId sceneId)
    {
        m_commandBuffer.hideScene(sceneId);
        doCommandExecutorLoop();

        RendererEventVector events;
        m_rendererEventCollector.dispatchEvents(events);
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType_SceneHidden, events[0].eventType);
        EXPECT_EQ(sceneId, events[0].sceneId);
    }

    void unmapScene(SceneId sceneId)
    {
        m_commandBuffer.unmapScene(sceneId);
        doCommandExecutorLoop();

        RendererEventVector events;
        m_rendererEventCollector.dispatchEvents(events);
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType_SceneUnmapped, events[0].eventType);
        EXPECT_EQ(sceneId, events[0].sceneId);
    }

    StrictMock<DisplayControllerMock>& getDisplayControllerMock(DisplayHandle handle = DisplayHandle(0u))
    {
        return *m_renderer.getDisplayMock(handle).m_displayController;
    }

    StrictMock<RenderBackendStrictMock>& getRenderBackendMock(DisplayHandle handle = DisplayHandle(0u))
    {
        return *m_renderer.getDisplayMock(handle).m_renderBackend;
    }

    SystemCompositorControllerMock* getSystemCompositorMock()
    {
        return static_cast<SystemCompositorControllerMock*>(m_platformFactoryMock.getSystemCompositorController());
    }

protected:
    StrictMock<PlatformFactoryStrictMock>         m_platformFactoryMock;
    RendererEventCollector                        m_rendererEventCollector;
    RendererScenes                                m_rendererScenes;
    SceneExpirationMonitor                        m_expirationMonitor;
    RendererStatistics                            m_rendererStatistics;
    StrictMock<RendererMockWithStrictMockDisplay> m_renderer;
    StrictMock<SceneGraphConsumerComponentMock>   m_sceneGraphConsumerComponent;
    RendererCommandBuffer                         m_commandBuffer;
    SceneStateExecutor                            m_sceneStateExecutor;
    FrameTimer                                    m_frameTimer;
    StrictMock<RendererSceneUpdaterFacade>        m_sceneUpdater;
    RendererCommandExecutor                       m_commandExecutor;

    StrictMock<ResourceProviderMock> m_resourceProvider;
    ResourceUploader m_resourceUploader;

    const RenderableHandle m_renderable;
    const String m_filename1;
    const String m_filename2;
    const String m_filename3;
};

INSTANTIATE_TEST_CASE_P(, ARendererCommandExecutor, ::testing::Values(false, true));

TEST_P(ARendererCommandExecutor, mapsSceneOnDisplay)
{
    const DisplayHandle dummyDisplay = addDisplayController();

    const SceneId sceneId(33u);
    createScene(sceneId);

    mapScene(sceneId, dummyDisplay);

    unmapScene(sceneId);
    removeDisplayController(dummyDisplay);
}

TEST_P(ARendererCommandExecutor, updatesWarpingMeshDataOnDisplay)
{
    const DisplayHandle dummyDisplay = addDisplayController();

    const WarpingMeshData warpingMeshData;
    m_commandBuffer.updateWarpingData(dummyDisplay, warpingMeshData);

    StrictMock<DisplayControllerMock>& displayControllerMock = *m_renderer.getDisplayMock(dummyDisplay).m_displayController;
    EXPECT_CALL(displayControllerMock, isWarpingEnabled()).WillRepeatedly(Return(true));
    EXPECT_CALL(displayControllerMock, enableContext());
    EXPECT_CALL(displayControllerMock, setWarpingMeshData(_));
    doCommandExecutorLoop();

    RendererEventVector events;
    m_rendererEventCollector.dispatchEvents(events);
    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(dummyDisplay, events.front().displayHandle);
    EXPECT_EQ(ERendererEventType_WarpingDataUpdated, events.front().eventType);

    removeDisplayController(dummyDisplay);
}

TEST_P(ARendererCommandExecutor, failsToUpdateWarpingMeshDataOnInvalidDisplay)
{
    const DisplayHandle dummyDisplay;

    const WarpingMeshData warpingMeshData;
    m_commandBuffer.updateWarpingData(dummyDisplay, warpingMeshData);
    doCommandExecutorLoop();

    RendererEventVector events;
    m_rendererEventCollector.dispatchEvents(events);
    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(dummyDisplay, events.front().displayHandle);
    EXPECT_EQ(ERendererEventType_WarpingDataUpdateFailed, events.front().eventType);
}

TEST_P(ARendererCommandExecutor, failsToUpdateWarpingMeshDataOnDisplayWithNoWarping)
{
    const DisplayHandle dummyDisplay = addDisplayController();

    const WarpingMeshData warpingMeshData;
    m_commandBuffer.updateWarpingData(dummyDisplay, warpingMeshData);

    StrictMock<DisplayControllerMock>& displayControllerMock = *m_renderer.getDisplayMock(dummyDisplay).m_displayController;
    EXPECT_CALL(displayControllerMock, isWarpingEnabled()).WillOnce(Return(false));
    doCommandExecutorLoop();

    RendererEventVector events;
    m_rendererEventCollector.dispatchEvents(events);
    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(dummyDisplay, events.front().displayHandle);
    EXPECT_EQ(ERendererEventType_WarpingDataUpdateFailed, events.front().eventType);

    removeDisplayController(dummyDisplay);
}

TEST_P(ARendererCommandExecutor, readPixelsFromDisplay)
{
    const DisplayHandle dummyDisplay = addDisplayController();

    const UInt32 x = 1u;
    const UInt32 y = 2u;
    const UInt32 width = 3u;
    const UInt32 height = 4u;

    m_commandBuffer.readPixels(dummyDisplay, "", false, x, y, width, height);
    doCommandExecutorLoop();

    StrictMock<DisplayControllerMock>& displayControllerMock = getDisplayControllerMock(dummyDisplay);
    if(GetParam())
        EXPECT_CALL(m_platformFactoryMock.windowEventsPollingManagerMock, pollWindowsTillAnyCanRender());
    EXPECT_CALL(displayControllerMock, handleWindowEvents());
    EXPECT_CALL(displayControllerMock, canRenderNewFrame()).WillOnce(Return(true));
    EXPECT_CALL(displayControllerMock, readPixels(x, y, width, height, _)).WillOnce(Return(true));
    expectReadPixelsInRenderLoop(); // needed overhead calls
    m_renderer.doOneRenderLoop();

    RendererEventVector events;
    m_rendererEventCollector.dispatchEvents(events);
    EXPECT_EQ(0u, events.size()); // Events are added by WindowedRenderer

    removeDisplayController(dummyDisplay);
}

TEST_P(ARendererCommandExecutor, createsReadPixelsFailedEventIfTryingToReadPixelsFromInvalidDisplay)
{
    const DisplayHandle dummyDisplay(1u);

    const UInt32 x = 1u;
    const UInt32 y = 2u;
    const UInt32 width = 3u;
    const UInt32 height = 4u;

    m_commandBuffer.readPixels(dummyDisplay, "", false, x, y, width, height);
    doCommandExecutorLoop();

    RendererEventVector events;
    m_rendererEventCollector.dispatchEvents(events);
    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(dummyDisplay, events.front().displayHandle);
    EXPECT_EQ(ERendererEventType_ReadPixelsFromFramebufferFailed, events.front().eventType);
}

TEST_P(ARendererCommandExecutor, readAndSavePixelsFromDisplay)
{
    const DisplayHandle dummyDisplay = addDisplayController();

    const UInt32 x = 1u;
    const UInt32 y = 2u;
    const UInt32 width = 3u;
    const UInt32 height = 4u;

    m_commandBuffer.readPixels(dummyDisplay, "testScreenshot", false, x, y, width, height);
    doCommandExecutorLoop();

    StrictMock<DisplayControllerMock>& displayControllerMock = getDisplayControllerMock(dummyDisplay);
    if(GetParam())
        EXPECT_CALL(m_platformFactoryMock.windowEventsPollingManagerMock, pollWindowsTillAnyCanRender());
    EXPECT_CALL(displayControllerMock, handleWindowEvents());
    EXPECT_CALL(displayControllerMock, canRenderNewFrame()).WillOnce(Return(true));
    EXPECT_CALL(displayControllerMock, readPixels(x, y, width, height, _)).WillOnce(Return(true));
    expectReadPixelsInRenderLoop(); // needed overhead calls
    m_renderer.doOneRenderLoop();

    RendererEventVector events;
    m_rendererEventCollector.dispatchEvents(events);
    EXPECT_EQ(0u, events.size()); // events are only added by the WindowedRenderer

    removeDisplayController(dummyDisplay);
}

TEST_P(ARendererCommandExecutor, readAndSaveFullscreenPixelsFromDisplay)
{
    const DisplayHandle dummyDisplay = addDisplayController();

    StrictMock<DisplayControllerMock>& displayControllerMock = getDisplayControllerMock(dummyDisplay);

    m_commandBuffer.readPixels(dummyDisplay, "testScreenshot", true, 2u, 33u, 201u, 4u);
    EXPECT_CALL(displayControllerMock, getDisplayWidth());
    EXPECT_CALL(displayControllerMock, getDisplayHeight());
    doCommandExecutorLoop();

    if(GetParam())
        EXPECT_CALL(m_platformFactoryMock.windowEventsPollingManagerMock, pollWindowsTillAnyCanRender());
    EXPECT_CALL(displayControllerMock, handleWindowEvents());
    EXPECT_CALL(displayControllerMock, canRenderNewFrame()).WillOnce(Return(true));
    EXPECT_CALL(displayControllerMock, readPixels(0u, 0u, WindowMock::FakeWidth, WindowMock::FakeHeight, _)).WillOnce(Return(true));
    expectReadPixelsInRenderLoop(); // needed overhead calls
    m_renderer.doOneRenderLoop();

    RendererEventVector events;
    m_rendererEventCollector.dispatchEvents(events);
    EXPECT_EQ(0u, events.size()); // events are only added by the WindowedRenderer

    removeDisplayController(dummyDisplay);
}

TEST_P(ARendererCommandExecutor, createsNoReadPixelsFailedEventIfTryingToReadAndSavePixelsFromInvalidDisplay)
{
    const DisplayHandle dummyDisplay(1u);

    const UInt32 x = 1u;
    const UInt32 y = 2u;
    const UInt32 width = 3u;
    const UInt32 height = 4u;

    m_commandBuffer.readPixels(dummyDisplay, "testScreenshot", false, x, y, width, height);
    doCommandExecutorLoop();

    RendererEventVector events;
    m_rendererEventCollector.dispatchEvents(events);
    EXPECT_EQ(0u, events.size());
}

TEST_P(ARendererCommandExecutor, createOffscreenBufferAndGenerateEvent)
{
    const DisplayHandle displayHandle = addDisplayController();
    const OffscreenBufferHandle buffer(1u);

    createOffscreenBuffer(buffer, displayHandle, false);

    EXPECT_CALL(getRenderBackendMock(displayHandle).deviceMock, deleteRenderBuffer(_)).Times(2);
    EXPECT_CALL(getRenderBackendMock(displayHandle).deviceMock, deleteRenderTarget(_)).Times(1);
    removeDisplayController(displayHandle);
}

TEST_P(ARendererCommandExecutor, createInterruptibleOffscreenBufferAndGenerateEvent)
{
    const DisplayHandle displayHandle = addDisplayController();
    const OffscreenBufferHandle buffer(1u);

    createOffscreenBuffer(buffer, displayHandle, true);

    EXPECT_CALL(getRenderBackendMock(displayHandle).deviceMock, deleteRenderBuffer(_)).Times(3);
    EXPECT_CALL(getRenderBackendMock(displayHandle).deviceMock, unpairRenderTargets(_)).Times(1u);
    EXPECT_CALL(getRenderBackendMock(displayHandle).deviceMock, deleteRenderTarget(_)).Times(2);
    removeDisplayController(displayHandle);
}

TEST_P(ARendererCommandExecutor, failsToCreateOffscreenBufferAndGeneratesFailEvent)
{
    const OffscreenBufferHandle buffer(1u);
    const DisplayHandle invalidDisplay(2u);

    m_commandBuffer.createOffscreenBuffer(buffer, invalidDisplay, 1u, 1u, false);
    doCommandExecutorLoop();

    RendererEventVector events;
    m_rendererEventCollector.dispatchEvents(events);
    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(ERendererEventType_OffscreenBufferCreateFailed, events.front().eventType);
    EXPECT_EQ(invalidDisplay, events.front().displayHandle);
    EXPECT_EQ(buffer, events.front().offscreenBuffer);
}

TEST_P(ARendererCommandExecutor, destroysOffscreenBufferAndGenerateEvent)
{
    const DisplayHandle displayHandle = addDisplayController();
    const OffscreenBufferHandle buffer(1u);

    createOffscreenBuffer(buffer, displayHandle, false);
    destroyOffscreenBuffer(buffer, displayHandle, false);

    removeDisplayController(displayHandle);
}

TEST_P(ARendererCommandExecutor, destroysInterruptibleOffscreenBufferAndGenerateEvent)
{
    const DisplayHandle displayHandle = addDisplayController();
    const OffscreenBufferHandle buffer(1u);

    createOffscreenBuffer(buffer, displayHandle, true);
    destroyOffscreenBuffer(buffer, displayHandle, true);

    removeDisplayController(displayHandle);
}

TEST_P(ARendererCommandExecutor, destroysOffscreenBufferAndGenerateFailEvent)
{
    const OffscreenBufferHandle invalidBuffer(1u);
    const DisplayHandle invalidDisplay(2u);

    m_commandBuffer.destroyOffscreenBuffer(invalidBuffer, invalidDisplay);
    doCommandExecutorLoop();

    RendererEventVector events;
    m_rendererEventCollector.dispatchEvents(events);
    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(ERendererEventType_OffscreenBufferDestroyFailed, events.front().eventType);
    EXPECT_EQ(invalidDisplay, events.front().displayHandle);
    EXPECT_EQ(invalidBuffer, events.front().offscreenBuffer);
}

TEST_P(ARendererCommandExecutor, assignsSceneToOffscreenBuffer)
{
    const DisplayHandle displayHandle = addDisplayController();
    const OffscreenBufferHandle buffer(1u);

    createOffscreenBuffer(buffer, displayHandle);

    const SceneId sceneId(33u);
    createScene(sceneId);
    mapScene(sceneId, displayHandle);

    m_commandBuffer.assignSceneToOffscreenBuffer(sceneId, buffer);
    doCommandExecutorLoop();

    RendererEventVector events;
    m_rendererEventCollector.dispatchEvents(events);
    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(ERendererEventType_SceneAssignedToOffscreenBuffer, events.front().eventType);
    EXPECT_EQ(sceneId, events.front().sceneId);
    EXPECT_EQ(buffer, events.front().offscreenBuffer);

    unmapScene(sceneId);
    destroyOffscreenBuffer(buffer, displayHandle);
    removeDisplayController(displayHandle);
}

TEST_P(ARendererCommandExecutor, confidence_reassignsSceneToFramebufferAfterItWasAssignedToOffscreenBuffer_ThenDestroysOffscreenBuffer)
{
    const DisplayHandle displayHandle = addDisplayController();
    const OffscreenBufferHandle buffer(1u);

    createOffscreenBuffer(buffer, displayHandle);

    const SceneId sceneId(33u);
    createScene(sceneId);
    mapScene(sceneId, displayHandle);

    m_commandBuffer.assignSceneToOffscreenBuffer(sceneId, buffer);
    doCommandExecutorLoop();

    RendererEventVector events;
    m_rendererEventCollector.dispatchEvents(events);
    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(ERendererEventType_SceneAssignedToOffscreenBuffer, events.front().eventType);
    EXPECT_EQ(sceneId, events.front().sceneId);
    EXPECT_EQ(buffer, events.front().offscreenBuffer);

    m_commandBuffer.assignSceneToFramebuffer(sceneId);
    doCommandExecutorLoop();

    m_rendererEventCollector.dispatchEvents(events);
    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(ERendererEventType_SceneAssignedToFramebuffer, events.front().eventType);
    EXPECT_EQ(sceneId, events.front().sceneId);
    EXPECT_EQ(OffscreenBufferHandle::Invalid(), events.front().offscreenBuffer);

    destroyOffscreenBuffer(buffer, displayHandle);
    unmapScene(sceneId);
    removeDisplayController(displayHandle);
}

TEST_P(ARendererCommandExecutor, reportsFailEventWhenUnmappedSceneCouldNotBeAssignedToOffscreenBuffer)
{
    const DisplayHandle displayHandle = addDisplayController();
    const OffscreenBufferHandle buffer(1u);

    createOffscreenBuffer(buffer, displayHandle);

    const SceneId sceneId(33u);
    createScene(sceneId);
    // Scene not mapped -> can not be assigned to offscreen buffer

    m_commandBuffer.assignSceneToOffscreenBuffer(sceneId, buffer);
    doCommandExecutorLoop();

    RendererEventVector events;
    m_rendererEventCollector.dispatchEvents(events);
    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(ERendererEventType_SceneAssignedToOffscreenBufferFailed, events.front().eventType);
    EXPECT_EQ(sceneId, events.front().sceneId);
    EXPECT_EQ(buffer, events.front().offscreenBuffer);

    destroyOffscreenBuffer(buffer, displayHandle);
    removeDisplayController(displayHandle);
}

TEST_P(ARendererCommandExecutor, reportsFailEventWhenSceneCouldNotBeAssignedToInvalidOffscreenBuffer)
{
    const DisplayHandle displayHandle = addDisplayController();
    const OffscreenBufferHandle invalidBuffer(1u);

    const SceneId sceneId(33u);
    createScene(sceneId);
    mapScene(sceneId, displayHandle);

    m_commandBuffer.assignSceneToOffscreenBuffer(sceneId, invalidBuffer);
    doCommandExecutorLoop();

    RendererEventVector events;
    m_rendererEventCollector.dispatchEvents(events);
    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(ERendererEventType_SceneAssignedToOffscreenBufferFailed, events.front().eventType);
    EXPECT_EQ(sceneId, events.front().sceneId);
    EXPECT_EQ(invalidBuffer, events.front().offscreenBuffer);

    unmapScene(sceneId);

    removeDisplayController(displayHandle);
}

TEST_P(ARendererCommandExecutor, reportsFailEventWhenUnmappedSceneCouldNotBeAssignedToFramebuffer)
{
    const DisplayHandle displayHandle = addDisplayController();

    const SceneId sceneId(33u);
    createScene(sceneId);
    // Scene not mapped -> can not be assigned to offscreen buffer

    m_commandBuffer.assignSceneToFramebuffer(sceneId);
    doCommandExecutorLoop();

    RendererEventVector events;
    m_rendererEventCollector.dispatchEvents(events);
    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(ERendererEventType_SceneAssignedToFramebufferFailed, events.front().eventType);
    EXPECT_EQ(sceneId, events.front().sceneId);

    removeDisplayController(displayHandle);
}

TEST_P(ARendererCommandExecutor, triggersLinkOffscreenBufferToConsumer)
{
    const OffscreenBufferHandle buffer(1u);
    const SceneId consumerScene(2u);
    const DataSlotId consumerData(3u);
    m_commandBuffer.linkBufferToSceneData(buffer, consumerScene, consumerData);

    doCommandExecutorLoop();

    RendererEventVector events;
    m_rendererEventCollector.dispatchEvents(events);
    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(ERendererEventType_SceneDataBufferLinkFailed, events.front().eventType);
    EXPECT_EQ(consumerScene, events.front().consumerSceneId);
    EXPECT_EQ(consumerData, events.front().consumerdataId);
    EXPECT_EQ(buffer, events.front().offscreenBuffer);
}

TEST_P(ARendererCommandExecutor, createsAndDestroysDisplay)
{
    const DisplayHandle displayHandle(33u);
    const DisplayConfig displayConfig;
    ResourceProviderMock resourceProvider;
    NiceMock<ResourceUploaderMock> resourceUploader;

    m_commandBuffer.createDisplay(displayConfig, resourceProvider, resourceUploader, displayHandle);
    doCommandExecutorLoop();

    EXPECT_EQ(1u, m_renderer.getDisplayControllerCount());
    EXPECT_TRUE(m_renderer.hasDisplayController(displayHandle));

    m_commandBuffer.destroyDisplay(displayHandle);

    EXPECT_CALL(getDisplayControllerMock(displayHandle), getRenderBackend());
    EXPECT_CALL(getRenderBackendMock(displayHandle).surfaceMock, enable());
    doCommandExecutorLoop();

    EXPECT_EQ(0u, m_renderer.getDisplayControllerCount());
    EXPECT_FALSE(m_renderer.hasDisplayController(displayHandle));
}

TEST_P(ARendererCommandExecutor, linksTransformDataSlots)
{
    const SceneId sceneProviderId(6u);
    const SceneId sceneConsumerId(7u);

    createScene(sceneProviderId);
    createScene(sceneConsumerId);

    SceneActionCollection providerSceneActions;
    SceneActionCollectionCreator providerCreator(providerSceneActions);
    const NodeHandle providerNode(1u);
    const DataSlotId providerSlotId(2u);
    const DataSlotHandle providerSlot(3u);
    providerCreator.allocateNode(0u, providerNode);
    providerCreator.allocateDataSlot({ EDataSlotType_TransformationProvider, providerSlotId, providerNode, DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), TextureSamplerHandle() }, providerSlot);
    providerCreator.flush(1u, false, true, SceneSizeInformation(10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u));
    m_commandBuffer.enqueueActionsForScene(sceneProviderId, std::move(providerSceneActions));

    SceneActionCollection consumerSceneActions;
    SceneActionCollectionCreator consumerCreator(consumerSceneActions);
    const NodeHandle consumerNode(1u);
    const DataSlotId consumerSlotId(2u);
    const DataSlotHandle consumerSlot(3u);
    consumerCreator.allocateNode(0u, consumerNode);
    consumerCreator.allocateDataSlot({ EDataSlotType_TransformationConsumer, consumerSlotId, consumerNode, DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), TextureSamplerHandle() }, consumerSlot);
    consumerCreator.flush(1u, false, true, SceneSizeInformation(10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u));
    m_commandBuffer.enqueueActionsForScene(sceneConsumerId, std::move(consumerSceneActions));

    // handle the previous generated events before the link event
    EXPECT_CALL(m_sceneUpdater, handleSceneActions(sceneConsumerId, _));
    EXPECT_CALL(m_sceneUpdater, handleSceneActions(sceneProviderId, _));
    doCommandExecutorLoop();

    updateScenes();

    RendererEventVector events;
    m_rendererEventCollector.dispatchEvents(events);

    // link transform slots and check the generated event
    m_commandBuffer.linkSceneData(sceneProviderId, providerSlotId, sceneConsumerId, consumerSlotId);
    doCommandExecutorLoop();

    m_rendererEventCollector.dispatchEvents(events);

    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(ERendererEventType_SceneDataLinked, events.front().eventType);
    EXPECT_EQ(sceneProviderId, events.front().providerSceneId);
    EXPECT_EQ(sceneConsumerId, events.front().consumerSceneId);
    EXPECT_EQ(providerSlotId, events.front().providerdataId);
    EXPECT_EQ(consumerSlotId, events.front().consumerdataId);
}

TEST_P(ARendererCommandExecutor, unlinksTransformDataSlots)
{
    const SceneId sceneProviderId(6u);
    const SceneId sceneConsumerId(7u);

    createScene(sceneProviderId);
    createScene(sceneConsumerId);

    SceneActionCollection providerSceneActions;
    SceneActionCollectionCreator providerCreator(providerSceneActions);
    const NodeHandle providerNode(1u);
    const DataSlotId providerSlotId(2u);
    const DataSlotHandle providerSlot(3u);
    providerCreator.allocateNode(0u, providerNode);
    providerCreator.allocateDataSlot({ EDataSlotType_TransformationProvider, providerSlotId, providerNode, DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), TextureSamplerHandle() }, providerSlot);
    providerCreator.flush(1u, false, true, SceneSizeInformation(10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u));
    m_commandBuffer.enqueueActionsForScene(sceneProviderId, std::move(providerSceneActions));

    SceneActionCollection consumerSceneActions;
    SceneActionCollectionCreator consumerCreator(consumerSceneActions);
    const NodeHandle consumerNode(1u);
    const DataSlotId consumerSlotId(2u);
    const DataSlotHandle consumerSlot(3u);
    consumerCreator.allocateNode(0u, consumerNode);
    consumerCreator.allocateDataSlot({ EDataSlotType_TransformationConsumer, consumerSlotId, consumerNode, DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), TextureSamplerHandle() }, consumerSlot);
    consumerCreator.flush(1u, false, true, SceneSizeInformation(10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u));
    m_commandBuffer.enqueueActionsForScene(sceneConsumerId, std::move(consumerSceneActions));

    EXPECT_CALL(m_sceneUpdater, handleSceneActions(sceneConsumerId, _));
    EXPECT_CALL(m_sceneUpdater, handleSceneActions(sceneProviderId, _));
    doCommandExecutorLoop();
    updateScenes();

    m_commandBuffer.linkSceneData(sceneProviderId, providerSlotId, sceneConsumerId, consumerSlotId);
    doCommandExecutorLoop();

    RendererEventVector events;
    m_rendererEventCollector.dispatchEvents(events);

    m_commandBuffer.unlinkSceneData(sceneConsumerId, consumerSlotId);
    doCommandExecutorLoop();

    m_rendererEventCollector.dispatchEvents(events);
    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(ERendererEventType_SceneDataUnlinked, events.front().eventType);
    EXPECT_EQ(sceneConsumerId, events.front().consumerSceneId);
    EXPECT_EQ(consumerSlotId, events.front().consumerdataId);
}

TEST_P(ARendererCommandExecutor, processesSceneActions)
{
    const SceneId sceneId(7u);
    const NodeHandle node(33u);

    createScene(sceneId);

    SceneActionCollection actions;
    SceneActionCollectionCreator creator(actions);
    creator.allocateNode(0u, node);
    creator.flush(1u, false, true, SceneSizeInformation(10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u));
    creator.flush(1u, false, false);
    creator.flush(1u, false, false);
    creator.flush(1u, false, false);
    creator.flush(1u, false, false);
    m_commandBuffer.enqueueActionsForScene(sceneId, std::move(actions));

    EXPECT_CALL(m_sceneUpdater, handleSceneActions(sceneId, _));
    doCommandExecutorLoop();
}

TEST_P(ARendererCommandExecutor, executionClearsSceneActionsRegardlessOfStateOfTheSceneUsingThem)
{
    const SceneId sceneSubscribedId(7u);
    const SceneId sceneNotSubscribedId(8u);

    createScene(sceneSubscribedId);
    m_commandBuffer.publishScene(sceneNotSubscribedId, Guid(true), EScenePublicationMode_LocalAndRemote);
    doCommandExecutorLoop();

    // add actions for both scenes
    SceneActionCollection actions;
    SceneActionCollectionCreator creator(actions);
    creator.allocateNode(0u, NodeHandle(1u));
    creator.flush(1u, false, true, SceneSizeInformation(10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u));
    m_commandBuffer.enqueueActionsForScene(sceneSubscribedId, actions.copy());
    m_commandBuffer.enqueueActionsForScene(sceneNotSubscribedId, std::move(actions));

    EXPECT_CALL(m_sceneUpdater, handleSceneActions(sceneSubscribedId, _));
    EXPECT_CALL(m_sceneUpdater, handleSceneActions(sceneNotSubscribedId, _));
    doCommandExecutorLoop();

    EXPECT_EQ(0u, m_commandBuffer.getCommands().getTotalCommandCount());
}

TEST_P(ARendererCommandExecutor, setsLayerVisibility)
{
    m_commandBuffer.systemCompositorControllerSetIviLayerVisibility(WaylandIviLayerId(23u),true);
    m_commandBuffer.systemCompositorControllerSetIviLayerVisibility(WaylandIviLayerId(25u),false);

    SystemCompositorControllerMock* systemCompositorMock = getSystemCompositorMock();
    if(systemCompositorMock != nullptr)
    {
        EXPECT_CALL(*systemCompositorMock, setLayerVisibility(WaylandIviLayerId(23u),true));
        EXPECT_CALL(*systemCompositorMock, setLayerVisibility(WaylandIviLayerId(25u),false));
    }
    m_commandExecutor.executePendingCommands();
}

TEST_P(ARendererCommandExecutor, setsSurfaceVisibility)
{
    m_commandBuffer.systemCompositorControllerSetIviSurfaceVisibility(WaylandIviSurfaceId(11u),true);
    m_commandBuffer.systemCompositorControllerSetIviSurfaceVisibility(WaylandIviSurfaceId(22u),false);

    SystemCompositorControllerMock* systemCompositorMock = getSystemCompositorMock();
    if(systemCompositorMock != nullptr)
    {
        EXPECT_CALL(*systemCompositorMock, setSurfaceVisibility(WaylandIviSurfaceId(11u),true));
        EXPECT_CALL(*systemCompositorMock, setSurfaceVisibility(WaylandIviSurfaceId(22u),false));
    }
    doCommandExecutorLoop();
}

TEST_P(ARendererCommandExecutor, listsIviSurfaces)
{
    m_commandBuffer.systemCompositorControllerListIviSurfaces();

    SystemCompositorControllerMock* systemCompositorMock = getSystemCompositorMock();
    if(systemCompositorMock != nullptr)
    {
        EXPECT_CALL(*systemCompositorMock, listIVISurfaces());
    }
    doCommandExecutorLoop();
}

TEST_P(ARendererCommandExecutor, setsSurfaceOpacity)
{
    m_commandBuffer.systemCompositorControllerSetIviSurfaceOpacity(WaylandIviSurfaceId(1u),1.f);
    m_commandBuffer.systemCompositorControllerSetIviSurfaceOpacity(WaylandIviSurfaceId(2u),0.2f);

    SystemCompositorControllerMock* systemCompositorMock = getSystemCompositorMock();
    if(systemCompositorMock != nullptr)
    {
        EXPECT_CALL(*systemCompositorMock, setSurfaceOpacity(WaylandIviSurfaceId(1u),1.f));
        EXPECT_CALL(*systemCompositorMock, setSurfaceOpacity(WaylandIviSurfaceId(2u),0.2f));
    }
    doCommandExecutorLoop();
}

TEST_P(ARendererCommandExecutor, setsSurfaceRectangle)
{
    m_commandBuffer.systemCompositorControllerSetIviSurfaceDestRectangle(WaylandIviSurfaceId(111u), 0, 0, 640, 480);
    m_commandBuffer.systemCompositorControllerSetIviSurfaceDestRectangle(WaylandIviSurfaceId(222u), -1,-10, 1920, 1080);

    SystemCompositorControllerMock* systemCompositorMock = getSystemCompositorMock();
    if(systemCompositorMock != nullptr)
    {
        EXPECT_CALL(*systemCompositorMock, setSurfaceDestinationRectangle(WaylandIviSurfaceId(111u), 0, 0, 640, 480));
        EXPECT_CALL(*systemCompositorMock, setSurfaceDestinationRectangle(WaylandIviSurfaceId(222u), -1,-10, 1920, 1080));
    }
    doCommandExecutorLoop();
}

TEST_P(ARendererCommandExecutor, callsSystemCompositorAddSurfaceToLayer)
{
    m_commandBuffer.systemCompositorControllerAddIviSurfaceToIviLayer(WaylandIviSurfaceId(222u), WaylandIviLayerId(111u));

    SystemCompositorControllerMock* systemCompositorMock = getSystemCompositorMock();
    if(systemCompositorMock != nullptr)
    {
        EXPECT_CALL(*systemCompositorMock, addSurfaceToLayer(WaylandIviSurfaceId(222u), WaylandIviLayerId(111u)));
    }
    doCommandExecutorLoop();
}

TEST_P(ARendererCommandExecutor, callsSystemCompositorRemoveSurfaceFromLayer)
{
    m_commandBuffer.systemCompositorControllerRemoveIviSurfaceFromIviLayer(WaylandIviSurfaceId(345u), WaylandIviLayerId(123u));

    SystemCompositorControllerMock* scc = getSystemCompositorMock();
    if(scc)
    {
        EXPECT_CALL(*scc, removeSurfaceFromLayer(WaylandIviSurfaceId(345u), WaylandIviLayerId(123u)));
    }
    doCommandExecutorLoop();
}

TEST_P(ARendererCommandExecutor, callsSystemCompositorDestroySurface)
{
    m_commandBuffer.systemCompositorControllerDestroyIviSurface(WaylandIviSurfaceId(51u));

    SystemCompositorControllerMock* scc = getSystemCompositorMock();
    if(scc)
    {
        EXPECT_CALL(*scc, destroySurface(WaylandIviSurfaceId(51u)));
    }
    doCommandExecutorLoop();
}

TEST_P(ARendererCommandExecutor, callsSystemCompositorScreenshot)
{
    const ramses_internal::String firstName("somefilename.png");
    const ramses_internal::String otherName("somethingelse.png");

    m_commandBuffer.systemCompositorControllerScreenshot(firstName, 1);
    m_commandBuffer.systemCompositorControllerScreenshot(otherName, -1);

    SystemCompositorControllerMock* systemCompositorMock = getSystemCompositorMock();
    if(systemCompositorMock != nullptr)
    {
        EXPECT_CALL(*systemCompositorMock, doScreenshot(firstName, 1));
        EXPECT_CALL(*systemCompositorMock, doScreenshot(otherName, -1));
    }
    doCommandExecutorLoop();
}

TEST_P(ARendererCommandExecutor, setClearColor)
{
    const DisplayHandle dummyDisplay = addDisplayController();
    const Vector4 clearColor(1.f, 0.f, 0.2f, 0.3f);

    m_commandBuffer.setClearColor(dummyDisplay, clearColor);
    doCommandExecutorLoop();

    auto& displayControllerMock = getDisplayControllerMock(dummyDisplay);
    if(GetParam())
        EXPECT_CALL(m_platformFactoryMock.windowEventsPollingManagerMock, pollWindowsTillAnyCanRender()).Times(1u);
    EXPECT_CALL(displayControllerMock, handleWindowEvents());
    EXPECT_CALL(displayControllerMock, canRenderNewFrame()).WillOnce(Return(true));
    expectSceneRenderingInRenderLoop(dummyDisplay, clearColor);
    m_renderer.doOneRenderLoop();
}

TEST_P(ARendererCommandExecutor, setFrameTimerLimits)
{
    //default values
    ASSERT_EQ(PlatformTime::InfiniteDuration, m_frameTimer.getTimeBudgetForSection(EFrameTimerSectionBudget::SceneResourcesUpload));
    ASSERT_EQ(PlatformTime::InfiniteDuration, m_frameTimer.getTimeBudgetForSection(EFrameTimerSectionBudget::ClientResourcesUpload));
    ASSERT_EQ(PlatformTime::InfiniteDuration, m_frameTimer.getTimeBudgetForSection(EFrameTimerSectionBudget::SceneActionsApply));
    ASSERT_EQ(PlatformTime::InfiniteDuration, m_frameTimer.getTimeBudgetForSection(EFrameTimerSectionBudget::OffscreenBufferRender));

    m_commandBuffer.setFrameTimerLimits(4u, 1u, 2u, 3u);
    doCommandExecutorLoop();

    EXPECT_EQ(std::chrono::microseconds(4u), m_frameTimer.getTimeBudgetForSection(EFrameTimerSectionBudget::SceneResourcesUpload));
    EXPECT_EQ(std::chrono::microseconds(1u), m_frameTimer.getTimeBudgetForSection(EFrameTimerSectionBudget::ClientResourcesUpload));
    EXPECT_EQ(std::chrono::microseconds(2u), m_frameTimer.getTimeBudgetForSection(EFrameTimerSectionBudget::SceneActionsApply));
    EXPECT_EQ(std::chrono::microseconds(3u), m_frameTimer.getTimeBudgetForSection(EFrameTimerSectionBudget::OffscreenBufferRender));
}
}
