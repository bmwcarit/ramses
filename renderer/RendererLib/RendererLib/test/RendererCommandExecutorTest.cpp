//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "ResourceDeviceHandleAccessorMock.h"
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
#include "RendererLib/RendererSceneControlLogic.h"
#include "RendererEventCollector.h"
#include "ComponentMocks.h"
#include "RendererSceneUpdaterMock.h"
#include "RendererSceneControlLogicMock.h"
#include "SceneReferenceLogicMock.h"
#include "SystemCompositorControllerMock.h"
#include "EmbeddedCompositingManagerMock.h"
#include "RendererSceneEventSenderMock.h"

namespace ramses_internal {

class ARendererCommandExecutor : public ::testing::TestWithParam<bool>
{
public:
    ARendererCommandExecutor()
        : m_platformFactoryMock(GetParam())
        , m_rendererScenes(m_rendererEventCollector)
        , m_expirationMonitor(m_rendererScenes, m_rendererEventCollector)
        , m_renderer(m_platformFactoryMock, m_rendererScenes, m_rendererEventCollector, m_expirationMonitor, m_rendererStatistics)
        , m_sceneStateExecutor(m_renderer, m_sceneEventSender, m_rendererEventCollector)
        , m_sceneUpdater(m_renderer, m_rendererScenes, m_sceneStateExecutor, m_rendererEventCollector, m_frameTimer, m_expirationMonitor, nullptr)
        , m_commandExecutor(m_renderer, m_commandBuffer, m_sceneUpdater, m_sceneControlLogic, m_rendererEventCollector, m_frameTimer)
        , m_resourceUploader(m_renderer.getStatistics())
        , m_renderable(1u)
        , m_filename1("somefilename1")
        , m_filename2("somefilename2")
        , m_filename3("somefilename3")
    {
        m_sceneUpdater.setSceneReferenceLogicHandler(m_sceneRefLogic);
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
        EXPECT_CALL(displayControllerMock, readPixels(_, _, _, _, _, _)).Times(times);
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
        EXPECT_CALL(m_sceneUpdater, createResourceManager(Ref(m_resourceUploader), _, _, displayHandle, _, _));
        m_commandBuffer.createDisplay(dummyConfig, m_resourceUploader, displayHandle);
        doCommandExecutorLoop();

        const RendererEventVector events = consumeRendererEvents();
        EXPECT_EQ(1u, events.size());
        EXPECT_EQ(displayHandle, events.front().displayHandle);
        EXPECT_EQ(ERendererEventType_DisplayCreated, events.front().eventType);

        EXPECT_CALL(getDisplayControllerMock(displayHandle), getRenderBackend()).Times(AnyNumber());
        EXPECT_CALL(getRenderBackendMock(displayHandle).surfaceMock, enable()).Times(AnyNumber());
        EXPECT_CALL(*m_sceneUpdater.m_resourceManagerMocks[displayHandle], hasResourcesToBeUploaded()).Times(AnyNumber());
        EXPECT_CALL(*m_sceneUpdater.m_resourceManagerMocks[displayHandle], referenceResourcesForScene(_, _)).Times(AnyNumber());
        EXPECT_CALL(*m_sceneUpdater.m_resourceManagerMocks[displayHandle], unloadAllSceneResourcesForScene(_)).Times(AnyNumber());
        EXPECT_CALL(*m_sceneUpdater.m_resourceManagerMocks[displayHandle], unreferenceAllResourcesForScene(_)).Times(AnyNumber());

        return displayHandle;
    }

    void removeDisplayController(DisplayHandle handle)
    {
        m_commandBuffer.destroyDisplay(handle);
        doCommandExecutorLoop();

        const RendererEventVector events = consumeRendererEvents();
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(handle, events.front().displayHandle);
        EXPECT_EQ(ERendererEventType_DisplayDestroyed, events.front().eventType);
    }

    void createScene(SceneId sceneId)
    {
        m_commandBuffer.publishScene(sceneId, EScenePublicationMode_LocalAndRemote);
        m_commandBuffer.subscribeScene(sceneId);
        m_commandBuffer.receiveScene(SceneInfo(sceneId));

        //receive initial flush
        SceneUpdate sceneUpdate;
        SceneActionCollectionCreator creator(sceneUpdate.actions);
        sceneUpdate.flushInfos.flushCounter = 1u;
        sceneUpdate.flushInfos.containsValidInformation = true;
        SceneActionCollection sceneActionCopy = sceneUpdate.actions.copy();
        m_commandBuffer.enqueueActionsForScene(sceneId, std::move(sceneUpdate));

        EXPECT_CALL(m_sceneEventSender, sendSubscribeScene(sceneId));
        EXPECT_CALL(m_sceneUpdater, handleSceneUpdate(sceneId, Field(&SceneUpdate::actions, Eq(ByRef(sceneActionCopy)))));
        doCommandExecutorLoop();
    }

    void updateScenes()
    {
        m_renderer.getProfilerStatistics().markFrameFinished(std::chrono::microseconds{ 0u });
        EXPECT_CALL(m_sceneRefLogic, update());
        m_sceneUpdater.updateScenes();
    }

    void mapScene(SceneId sceneId, DisplayHandle display)
    {
        m_commandBuffer.mapSceneToDisplay(sceneId, display);
        doCommandExecutorLoop();
        updateScenes();
        updateScenes();
    }

    void showScene(SceneId sceneId)
    {
        m_commandBuffer.showScene(sceneId);
        doCommandExecutorLoop();

        const RendererEventVector events = consumeSceneControlEvents();
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType_SceneShown, events[0].eventType);
        EXPECT_EQ(sceneId, events[0].sceneId);
    }

    void createOffscreenBuffer(OffscreenBufferHandle buffer, DisplayHandle displayHandle, bool interruptible = false)
    {
        m_commandBuffer.createOffscreenBuffer(buffer, displayHandle, 1u, 1u, 4u, interruptible);

        // there is check if OB already uploaded before creation, make sure to report it is not
        InSequence seq;
        EXPECT_CALL(*m_sceneUpdater.m_resourceManagerMocks[displayHandle], getOffscreenBufferDeviceHandle(buffer)).WillOnce(Return(DeviceResourceHandle::Invalid()));
        EXPECT_CALL(*m_sceneUpdater.m_resourceManagerMocks[displayHandle], uploadOffscreenBuffer(buffer, 1u, 1u, 4u, interruptible));
        EXPECT_CALL(*m_sceneUpdater.m_resourceManagerMocks[displayHandle], getOffscreenBufferDeviceHandle(buffer)).WillOnce(Return(DeviceMock::FakeRenderTargetDeviceHandle));
        doCommandExecutorLoop();

        const RendererEventVector events = consumeRendererEvents();
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType_OffscreenBufferCreated, events.front().eventType);
        EXPECT_EQ(displayHandle, events.front().displayHandle);
        EXPECT_EQ(buffer, events.front().offscreenBuffer);
    }

    void destroyOffscreenBuffer(OffscreenBufferHandle buffer, DisplayHandle displayHandle)
    {
        m_commandBuffer.destroyOffscreenBuffer(buffer, displayHandle);

        InSequence seq;
        EXPECT_CALL(*m_sceneUpdater.m_resourceManagerMocks[displayHandle], getOffscreenBufferDeviceHandle(buffer)).WillOnce(Return(DeviceMock::FakeRenderTargetDeviceHandle));
        EXPECT_CALL(*m_sceneUpdater.m_resourceManagerMocks[displayHandle], unloadOffscreenBuffer(buffer));
        doCommandExecutorLoop();

        const RendererEventVector events = consumeRendererEvents();
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType_OffscreenBufferDestroyed, events.front().eventType);
        EXPECT_EQ(displayHandle, events.front().displayHandle);
        EXPECT_EQ(buffer, events.front().offscreenBuffer);
    }

    void hideScene(SceneId sceneId)
    {
        m_commandBuffer.hideScene(sceneId);
        doCommandExecutorLoop();

        const RendererEventVector events = consumeSceneControlEvents();
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType_SceneHidden, events[0].eventType);
        EXPECT_EQ(sceneId, events[0].sceneId);
    }

    void unmapScene(SceneId sceneId)
    {
        m_commandBuffer.unmapScene(sceneId);
        doCommandExecutorLoop();
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

    RendererEventVector consumeRendererEvents()
    {
        RendererEventVector events;
        RendererEventVector dummy;
        m_rendererEventCollector.appendAndConsumePendingEvents(events, dummy);
        return events;
    }

    RendererEventVector consumeSceneControlEvents()
    {
        RendererEventVector events;
        RendererEventVector dummy;
        m_rendererEventCollector.appendAndConsumePendingEvents(dummy, events);
        return events;
    }

protected:
    StrictMock<PlatformStrictMock>         m_platformFactoryMock;
    RendererEventCollector                        m_rendererEventCollector;
    RendererScenes                                m_rendererScenes;
    SceneExpirationMonitor                        m_expirationMonitor;
    StrictMock<SceneReferenceLogicMock>           m_sceneRefLogic;
    RendererStatistics                            m_rendererStatistics;
    StrictMock<RendererMockWithStrictMockDisplay> m_renderer;
    StrictMock<RendererSceneEventSenderMock>      m_sceneEventSender;
    RendererCommandBuffer                         m_commandBuffer;
    SceneStateExecutor                            m_sceneStateExecutor;
    FrameTimer                                    m_frameTimer;
    StrictMock<RendererSceneUpdaterFacade>        m_sceneUpdater;
    StrictMock<RendererSceneControlLogicMock>     m_sceneControlLogic;
    RendererCommandExecutor                       m_commandExecutor;

    ResourceUploader m_resourceUploader;

    const RenderableHandle m_renderable;
    const String m_filename1;
    const String m_filename2;
    const String m_filename3;
    uint64_t m_nextGuid = 100;
};

INSTANTIATE_TEST_SUITE_P(, ARendererCommandExecutor, ::testing::Values(false, true));

TEST_P(ARendererCommandExecutor, setSceneState)
{
    constexpr SceneId sceneId{ 123 };
    m_commandBuffer.setSceneState(sceneId, RendererSceneState::Ready);

    EXPECT_CALL(m_sceneControlLogic, setSceneState(sceneId, RendererSceneState::Ready));
    doCommandExecutorLoop();
}

TEST_P(ARendererCommandExecutor, setSceneMapping)
{
    constexpr SceneId sceneId{ 123 };
    constexpr DisplayHandle display{ 2 };
    m_commandBuffer.setSceneMapping(sceneId, display);

    EXPECT_CALL(m_sceneControlLogic, setSceneMapping(sceneId, display));
    doCommandExecutorLoop();
}

TEST_P(ARendererCommandExecutor, setSceneDisplayBufferAssignment)
{
    constexpr SceneId sceneId{ 123 };
    constexpr OffscreenBufferHandle ob{ 2 };
    m_commandBuffer.setSceneDisplayBufferAssignment(sceneId, ob, -13);

    EXPECT_CALL(m_sceneControlLogic, setSceneDisplayBufferAssignment(sceneId, ob, -13));
    doCommandExecutorLoop();
}

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

    const RendererEventVector events = consumeRendererEvents();
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

    const RendererEventVector events = consumeRendererEvents();
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

    const RendererEventVector events = consumeRendererEvents();
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

    m_commandBuffer.readPixels(dummyDisplay, {}, "", false, x, y, width, height);
    doCommandExecutorLoop();

    StrictMock<DisplayControllerMock>& displayControllerMock = getDisplayControllerMock(dummyDisplay);
    if(GetParam())
        EXPECT_CALL(m_platformFactoryMock.windowEventsPollingManagerMock, pollWindowsTillAnyCanRender());
    EXPECT_CALL(displayControllerMock, handleWindowEvents());
    EXPECT_CALL(displayControllerMock, canRenderNewFrame()).WillOnce(Return(true));
    EXPECT_CALL(displayControllerMock, readPixels(DisplayControllerMock::FakeFrameBufferHandle, x, y, width, height, _)).WillOnce(Invoke(
        [&](auto, auto, auto, auto, auto, auto& result) {
            result.resize(width * height * 4);
        }));
    expectReadPixelsInRenderLoop(); // needed overhead calls
    m_renderer.doOneRenderLoop();

    EXPECT_TRUE(consumeRendererEvents().empty()); // events are only added by the WindowedRenderer

    removeDisplayController(dummyDisplay);
}

TEST_P(ARendererCommandExecutor, createOffscreenBufferAndGenerateEvent)
{
    const DisplayHandle displayHandle = addDisplayController();
    const OffscreenBufferHandle buffer(1u);

    createOffscreenBuffer(buffer, displayHandle, false);

    removeDisplayController(displayHandle);
}

TEST_P(ARendererCommandExecutor, createInterruptibleOffscreenBufferAndGenerateEvent)
{
    const DisplayHandle displayHandle = addDisplayController();
    const OffscreenBufferHandle buffer(1u);

    createOffscreenBuffer(buffer, displayHandle, true);

    removeDisplayController(displayHandle);
}

TEST_P(ARendererCommandExecutor, failsToCreateOffscreenBufferAndGeneratesFailEvent)
{
    const OffscreenBufferHandle buffer(1u);
    const DisplayHandle invalidDisplay(2u);

    m_commandBuffer.createOffscreenBuffer(buffer, invalidDisplay, 1u, 1u, 0u, false);
    doCommandExecutorLoop();

    const RendererEventVector events = consumeRendererEvents();
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
    destroyOffscreenBuffer(buffer, displayHandle);

    removeDisplayController(displayHandle);
}

TEST_P(ARendererCommandExecutor, destroysInterruptibleOffscreenBufferAndGenerateEvent)
{
    const DisplayHandle displayHandle = addDisplayController();
    const OffscreenBufferHandle buffer(1u);

    createOffscreenBuffer(buffer, displayHandle, true);
    destroyOffscreenBuffer(buffer, displayHandle);

    removeDisplayController(displayHandle);
}

TEST_P(ARendererCommandExecutor, destroysOffscreenBufferAndGenerateFailEvent)
{
    const OffscreenBufferHandle invalidBuffer(1u);
    const DisplayHandle invalidDisplay(2u);

    m_commandBuffer.destroyOffscreenBuffer(invalidBuffer, invalidDisplay);
    doCommandExecutorLoop();

    const RendererEventVector events = consumeRendererEvents();
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

    m_commandBuffer.assignSceneToDisplayBuffer(sceneId, buffer, 11);
    EXPECT_CALL(*m_sceneUpdater.m_resourceManagerMocks[displayHandle], getOffscreenBufferDeviceHandle(buffer));
    doCommandExecutorLoop();

    const RendererEventVector events = consumeSceneControlEvents();
    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(ERendererEventType_SceneAssignedToDisplayBuffer, events.front().eventType);
    EXPECT_EQ(sceneId, events.front().sceneId);
    EXPECT_EQ(buffer, events.front().offscreenBuffer);
    EXPECT_EQ(displayHandle, events.front().displayHandle);

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

    m_commandBuffer.assignSceneToDisplayBuffer(sceneId, buffer, 11);
    EXPECT_CALL(*m_sceneUpdater.m_resourceManagerMocks[displayHandle], getOffscreenBufferDeviceHandle(buffer));
    doCommandExecutorLoop();

    RendererEventVector events = consumeSceneControlEvents();
    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(ERendererEventType_SceneAssignedToDisplayBuffer, events.front().eventType);
    EXPECT_EQ(sceneId, events.front().sceneId);
    EXPECT_EQ(buffer, events.front().offscreenBuffer);
    EXPECT_EQ(displayHandle, events.front().displayHandle);

    m_commandBuffer.assignSceneToDisplayBuffer(sceneId, OffscreenBufferHandle::Invalid(), 12);
    doCommandExecutorLoop();

    events = consumeSceneControlEvents();
    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(ERendererEventType_SceneAssignedToDisplayBuffer, events.front().eventType);
    EXPECT_EQ(sceneId, events.front().sceneId);
    EXPECT_EQ(OffscreenBufferHandle::Invalid(), events.front().offscreenBuffer);
    EXPECT_EQ(displayHandle, events.front().displayHandle);

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

    m_commandBuffer.assignSceneToDisplayBuffer(sceneId, buffer, 11);
    doCommandExecutorLoop();

    const RendererEventVector events = consumeSceneControlEvents();
    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(ERendererEventType_SceneAssignedToDisplayBufferFailed, events.front().eventType);
    EXPECT_EQ(sceneId, events.front().sceneId);
    EXPECT_EQ(buffer, events.front().offscreenBuffer);
    EXPECT_FALSE(events.front().displayHandle.isValid());

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

    m_commandBuffer.assignSceneToDisplayBuffer(sceneId, invalidBuffer, 11);
    EXPECT_CALL(*m_sceneUpdater.m_resourceManagerMocks[displayHandle], getOffscreenBufferDeviceHandle(invalidBuffer)).WillOnce(Return(DeviceResourceHandle::Invalid()));
    doCommandExecutorLoop();

    const RendererEventVector events = consumeSceneControlEvents();
    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(ERendererEventType_SceneAssignedToDisplayBufferFailed, events.front().eventType);
    EXPECT_EQ(sceneId, events.front().sceneId);
    EXPECT_EQ(invalidBuffer, events.front().offscreenBuffer);
    EXPECT_EQ(displayHandle, events.front().displayHandle);

    unmapScene(sceneId);

    removeDisplayController(displayHandle);
}

TEST_P(ARendererCommandExecutor, reportsFailEventWhenUnmappedSceneCouldNotBeAssignedToFramebuffer)
{
    const DisplayHandle displayHandle = addDisplayController();

    const SceneId sceneId(33u);
    createScene(sceneId);
    // Scene not mapped -> can not be assigned to offscreen buffer

    m_commandBuffer.assignSceneToDisplayBuffer(sceneId, OffscreenBufferHandle::Invalid(), 11);
    doCommandExecutorLoop();

    const RendererEventVector events = consumeSceneControlEvents();
    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(ERendererEventType_SceneAssignedToDisplayBufferFailed, events.front().eventType);
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

    const RendererEventVector events = consumeSceneControlEvents();
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
    NiceMock<ResourceUploaderMock> resourceUploader;

    m_commandBuffer.createDisplay(displayConfig, resourceUploader, displayHandle);
    EXPECT_CALL(m_sceneUpdater, createResourceManager(Ref(resourceUploader), _, _, displayHandle, _, _));
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

    SceneUpdate providerSceneActions;
    SceneActionCollectionCreator providerCreator(providerSceneActions.actions);
    const NodeHandle providerNode(1u);
    const DataSlotId providerSlotId(2u);
    const DataSlotHandle providerSlot(3u);
    providerCreator.allocateNode(0u, providerNode);
    providerCreator.allocateDataSlot({ EDataSlotType_TransformationProvider, providerSlotId, providerNode, DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), TextureSamplerHandle() }, providerSlot);
    providerSceneActions.flushInfos.flushCounter = 1u;
    providerSceneActions.flushInfos.containsValidInformation = true;
    providerSceneActions.flushInfos.hasSizeInfo = true;
    providerSceneActions.flushInfos.sizeInfo = SceneSizeInformation(10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u);
    m_commandBuffer.enqueueActionsForScene(sceneProviderId, std::move(providerSceneActions));

    SceneUpdate consumerSceneActions;
    SceneActionCollectionCreator consumerCreator(consumerSceneActions.actions);
    const NodeHandle consumerNode(1u);
    const DataSlotId consumerSlotId(2u);
    const DataSlotHandle consumerSlot(3u);
    consumerCreator.allocateNode(0u, consumerNode);
    consumerCreator.allocateDataSlot({ EDataSlotType_TransformationConsumer, consumerSlotId, consumerNode, DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), TextureSamplerHandle() }, consumerSlot);
    consumerSceneActions.flushInfos.flushCounter = 1u;
    consumerSceneActions.flushInfos.containsValidInformation = true;
    consumerSceneActions.flushInfos.hasSizeInfo = true;
    consumerSceneActions.flushInfos.sizeInfo = SceneSizeInformation(10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u);
    m_commandBuffer.enqueueActionsForScene(sceneConsumerId, std::move(consumerSceneActions));

    // handle the previous generated events before the link event
    EXPECT_CALL(m_sceneUpdater, handleSceneUpdate(sceneConsumerId, _));
    EXPECT_CALL(m_sceneUpdater, handleSceneUpdate(sceneProviderId, _));
    doCommandExecutorLoop();

    updateScenes();
    consumeSceneControlEvents();

    // link transform slots and check the generated event
    m_commandBuffer.linkSceneData(sceneProviderId, providerSlotId, sceneConsumerId, consumerSlotId);
    doCommandExecutorLoop();

    const RendererEventVector events = consumeSceneControlEvents();
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

    SceneUpdate providerSceneActions;
    SceneActionCollectionCreator providerCreator(providerSceneActions.actions);
    const NodeHandle providerNode(1u);
    const DataSlotId providerSlotId(2u);
    const DataSlotHandle providerSlot(3u);
    providerCreator.allocateNode(0u, providerNode);
    providerCreator.allocateDataSlot({ EDataSlotType_TransformationProvider, providerSlotId, providerNode, DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), TextureSamplerHandle() }, providerSlot);
    providerSceneActions.flushInfos.flushCounter = 1u;
    providerSceneActions.flushInfos.containsValidInformation = true;
    providerSceneActions.flushInfos.hasSizeInfo = true;
    providerSceneActions.flushInfos.sizeInfo = SceneSizeInformation(10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u);
    m_commandBuffer.enqueueActionsForScene(sceneProviderId, std::move(providerSceneActions));

    SceneUpdate consumerSceneActions;
    SceneActionCollectionCreator consumerCreator(consumerSceneActions.actions);
    const NodeHandle consumerNode(1u);
    const DataSlotId consumerSlotId(2u);
    const DataSlotHandle consumerSlot(3u);
    consumerCreator.allocateNode(0u, consumerNode);
    consumerCreator.allocateDataSlot({ EDataSlotType_TransformationConsumer, consumerSlotId, consumerNode, DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), TextureSamplerHandle() }, consumerSlot);
    consumerSceneActions.flushInfos.flushCounter = 1u;
    consumerSceneActions.flushInfos.containsValidInformation = true;
    consumerSceneActions.flushInfos.hasSizeInfo = true;
    consumerSceneActions.flushInfos.sizeInfo = SceneSizeInformation(10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u);
    m_commandBuffer.enqueueActionsForScene(sceneConsumerId, std::move(consumerSceneActions));

    EXPECT_CALL(m_sceneUpdater, handleSceneUpdate(sceneConsumerId, _));
    EXPECT_CALL(m_sceneUpdater, handleSceneUpdate(sceneProviderId, _));
    doCommandExecutorLoop();
    updateScenes();

    m_commandBuffer.linkSceneData(sceneProviderId, providerSlotId, sceneConsumerId, consumerSlotId);
    doCommandExecutorLoop();

    consumeSceneControlEvents();

    m_commandBuffer.unlinkSceneData(sceneConsumerId, consumerSlotId);
    doCommandExecutorLoop();

    const RendererEventVector events = consumeSceneControlEvents();
    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(ERendererEventType_SceneDataUnlinked, events.front().eventType);
    EXPECT_EQ(sceneConsumerId, events.front().consumerSceneId);
    EXPECT_EQ(consumerSlotId, events.front().consumerdataId);
    EXPECT_EQ(sceneProviderId, events.front().providerSceneId);
}

TEST_P(ARendererCommandExecutor, processesSceneActions)
{
    const SceneId sceneId(7u);
    const NodeHandle node(33u);

    createScene(sceneId);

    SceneUpdate actions;
    SceneActionCollectionCreator creator(actions.actions);
    creator.allocateNode(0u, node);
    actions.flushInfos.flushCounter = 1u;
    actions.flushInfos.containsValidInformation = true;
    m_commandBuffer.enqueueActionsForScene(sceneId, std::move(actions));

    EXPECT_CALL(m_sceneUpdater, handleSceneUpdate(sceneId, _));
    doCommandExecutorLoop();
}

TEST_P(ARendererCommandExecutor, executionClearsSceneActionsRegardlessOfStateOfTheSceneUsingThem)
{
    const SceneId sceneSubscribedId(7u);
    const SceneId sceneNotSubscribedId(8u);

    createScene(sceneSubscribedId);
    m_commandBuffer.publishScene(sceneNotSubscribedId, EScenePublicationMode_LocalAndRemote);
    doCommandExecutorLoop();

    // add actions for both scenes
    SceneUpdate actions;
    SceneActionCollectionCreator creator(actions.actions);
    SceneUpdate copy;
    creator.allocateNode(0u, NodeHandle(1u));
    actions.flushInfos.flushCounter = 1u;
    actions.flushInfos.containsValidInformation = true;
    actions.flushInfos.hasSizeInfo = true;
    actions.flushInfos.sizeInfo = SceneSizeInformation(10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u);
    copy.actions = actions.actions.copy();
    copy.flushInfos.flushCounter = 1u;
    copy.flushInfos.containsValidInformation = true;
    copy.flushInfos.hasSizeInfo = true;
    copy.flushInfos.sizeInfo = SceneSizeInformation(10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u);
    m_commandBuffer.enqueueActionsForScene(sceneSubscribedId, std::move(copy));
    m_commandBuffer.enqueueActionsForScene(sceneNotSubscribedId, std::move(actions));

    EXPECT_CALL(m_sceneUpdater, handleSceneUpdate(sceneSubscribedId, _));
    EXPECT_CALL(m_sceneUpdater, handleSceneUpdate(sceneNotSubscribedId, _));
    doCommandExecutorLoop();

    RendererCommandContainer cmds;
    m_commandBuffer.swapCommandContainer(cmds);
    EXPECT_EQ(0u, cmds.getTotalCommandCount());
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

    m_commandBuffer.setClearColor(dummyDisplay, {}, clearColor);
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
    ASSERT_EQ(PlatformTime::InfiniteDuration, m_frameTimer.getTimeBudgetForSection(EFrameTimerSectionBudget::ResourcesUpload));
    ASSERT_EQ(PlatformTime::InfiniteDuration, m_frameTimer.getTimeBudgetForSection(EFrameTimerSectionBudget::OffscreenBufferRender));

    m_commandBuffer.setFrameTimerLimits(4u, 1u, 3u);
    doCommandExecutorLoop();

    EXPECT_EQ(std::chrono::microseconds(4u), m_frameTimer.getTimeBudgetForSection(EFrameTimerSectionBudget::SceneResourcesUpload));
    EXPECT_EQ(std::chrono::microseconds(1u), m_frameTimer.getTimeBudgetForSection(EFrameTimerSectionBudget::ResourcesUpload));
    EXPECT_EQ(std::chrono::microseconds(3u), m_frameTimer.getTimeBudgetForSection(EFrameTimerSectionBudget::OffscreenBufferRender));
}

TEST_P(ARendererCommandExecutor, forwardsPickEventToSceneUpdater)
{
    const SceneId sceneId{ 123u };
    const Vector2 coords{ 0.1f, 0.2f };
    m_commandBuffer.handlePickEvent(sceneId, coords);
    EXPECT_CALL(m_sceneUpdater, handlePickEvent(sceneId, coords));
    doCommandExecutorLoop();
}
}
