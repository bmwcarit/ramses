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
#include "RendererLib/RendererCommandExecutor.h"
#include "RendererLib/RendererLogContext.h"
#include "RendererLib/RendererResourceManager.h"
#include "RendererLib/RendererCommandBuffer.h"
#include "RendererLib/SceneStateExecutor.h"
#include "RendererLib/RendererScenes.h"
#include "RendererLib/DisplayEventHandlerManager.h"
#include "RendererLib/FrameTimer.h"
#include "RendererLib/SceneExpirationMonitor.h"
#include "RendererLib/RendererSceneControlLogic.h"
#include "RendererLib/RendererCommands.h"
#include "RendererEventCollector.h"
#include "ComponentMocks.h"
#include "RendererSceneUpdaterFacade.h"
#include "RendererSceneControlLogicMock.h"
#include "SceneReferenceLogicMock.h"
#include "SystemCompositorControllerMock.h"
#include "EmbeddedCompositingManagerMock.h"
#include "RendererSceneEventSenderMock.h"
#include "RendererSceneUpdaterMock.h"
#include "Utils/ThreadLocalLog.h"

namespace ramses_internal {

class ARendererCommandExecutor : public ::testing::Test
{
public:
    ARendererCommandExecutor()
        : m_rendererScenes(m_rendererEventCollector)
        , m_expirationMonitor(m_rendererScenes, m_rendererEventCollector)
        , m_renderer(m_platformFactoryMock, m_rendererScenes, m_rendererEventCollector, m_expirationMonitor, m_rendererStatistics)
        , m_sceneStateExecutor(m_renderer, m_sceneEventSender, m_rendererEventCollector)
        , m_commandExecutor(m_renderer, m_commandBuffer, m_sceneUpdater, m_sceneControlLogic, m_rendererEventCollector, m_frameTimer)
    {
        // caller is expected to have a display prefix for logs
        ThreadLocalLog::SetPrefix(1);
    }

    void doCommandExecutorLoop()
    {
        m_renderer.getProfilerStatistics().markFrameFinished(std::chrono::microseconds{ 0u });
        m_commandExecutor.executePendingCommands();
    }

    DisplayHandle addDisplayController()
    {
        constexpr DisplayHandle displayHandle{ 1u };
        static const DisplayConfig dummyConfig;
        m_renderer.createDisplayContext(dummyConfig, displayHandle);

        return displayHandle;
    }

    void removeDisplayController(DisplayHandle handle)
    {
        m_renderer.destroyDisplayContext(handle);
    }

    StrictMock<DisplayControllerMock>& getDisplayControllerMock(DisplayHandle handle = DisplayHandle(0u))
    {
        return *m_renderer.getDisplayMock(handle).m_displayController;
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
    NiceMock<PlatformNiceMock>                    m_platformFactoryMock;
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
    StrictMock<RendererSceneUpdaterMock>          m_sceneUpdater;
    StrictMock<RendererSceneControlLogicMock>     m_sceneControlLogic;
    RendererCommandExecutor                       m_commandExecutor;
};

TEST_F(ARendererCommandExecutor, setSceneState)
{
    constexpr SceneId sceneId{ 123 };
    m_commandBuffer.enqueueCommand(RendererCommand::SetSceneState{ sceneId, RendererSceneState::Ready });

    EXPECT_CALL(m_sceneControlLogic, setSceneState(sceneId, RendererSceneState::Ready));
    doCommandExecutorLoop();
}

TEST_F(ARendererCommandExecutor, setSceneMapping)
{
    constexpr SceneId sceneId{ 123 };
    constexpr DisplayHandle display{ 2 };
    m_commandBuffer.enqueueCommand(RendererCommand::SetSceneMapping{ sceneId, display });

    EXPECT_CALL(m_sceneControlLogic, setSceneMapping(sceneId, display));
    doCommandExecutorLoop();
}

TEST_F(ARendererCommandExecutor, setSceneDisplayBufferAssignment)
{
    constexpr SceneId sceneId{ 123 };
    constexpr OffscreenBufferHandle ob{ 2 };
    m_commandBuffer.enqueueCommand(RendererCommand::SetSceneDisplayBufferAssignment{ sceneId, ob, -13 });

    EXPECT_CALL(m_sceneControlLogic, setSceneDisplayBufferAssignment(sceneId, ob, -13));
    doCommandExecutorLoop();
}

TEST_F(ARendererCommandExecutor, updatesWarpingMeshDataOnDisplay)
{
    const DisplayHandle dummyDisplay = addDisplayController();

    m_commandBuffer.enqueueCommand(RendererCommand::UpdateWarpingData{ dummyDisplay, WarpingMeshData{} });

    StrictMock<DisplayControllerMock>& displayControllerMock = *m_renderer.getDisplayMock(dummyDisplay).m_displayController;
    EXPECT_CALL(displayControllerMock, isWarpingEnabled()).WillRepeatedly(Return(true));
    EXPECT_CALL(displayControllerMock, setWarpingMeshData(_));
    doCommandExecutorLoop();

    const RendererEventVector events = consumeRendererEvents();
    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(dummyDisplay, events.front().displayHandle);
    EXPECT_EQ(ERendererEventType::WarpingDataUpdated, events.front().eventType);

    removeDisplayController(dummyDisplay);
}

TEST_F(ARendererCommandExecutor, failsToUpdateWarpingMeshDataOnInvalidDisplay)
{
    const DisplayHandle dummyDisplay;

    m_commandBuffer.enqueueCommand(RendererCommand::UpdateWarpingData{ dummyDisplay, WarpingMeshData{} });
    doCommandExecutorLoop();

    const RendererEventVector events = consumeRendererEvents();
    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(dummyDisplay, events.front().displayHandle);
    EXPECT_EQ(ERendererEventType::WarpingDataUpdateFailed, events.front().eventType);
}

TEST_F(ARendererCommandExecutor, failsToUpdateWarpingMeshDataOnDisplayWithNoWarping)
{
    const DisplayHandle dummyDisplay = addDisplayController();

    m_commandBuffer.enqueueCommand(RendererCommand::UpdateWarpingData{ dummyDisplay, WarpingMeshData{} });

    StrictMock<DisplayControllerMock>& displayControllerMock = *m_renderer.getDisplayMock(dummyDisplay).m_displayController;
    EXPECT_CALL(displayControllerMock, isWarpingEnabled()).WillOnce(Return(false));
    doCommandExecutorLoop();

    const RendererEventVector events = consumeRendererEvents();
    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(dummyDisplay, events.front().displayHandle);
    EXPECT_EQ(ERendererEventType::WarpingDataUpdateFailed, events.front().eventType);

    removeDisplayController(dummyDisplay);
}

TEST_F(ARendererCommandExecutor, readPixelsFromDisplayBuffer)
{
    constexpr DisplayHandle display{ 1u };
    constexpr OffscreenBufferHandle buffer{ 2u };

    m_commandBuffer.enqueueCommand(RendererCommand::ReadPixels{ display, buffer, 1, 2, 3, 4, true, true, "file" });
    EXPECT_CALL(m_sceneUpdater, handleReadPixels(display, buffer, _)).WillOnce(Invoke([&](auto, auto, auto&& info)
    {
        EXPECT_EQ(1u, info.rectangle.x);
        EXPECT_EQ(2u, info.rectangle.y);
        EXPECT_EQ(3u, info.rectangle.width);
        EXPECT_EQ(4u, info.rectangle.height);
        EXPECT_EQ(String("file"), info.filename);
        EXPECT_TRUE(info.fullScreen);
        EXPECT_TRUE(info.sendViaDLT);
    }));
    doCommandExecutorLoop();

    EXPECT_TRUE(consumeRendererEvents().empty()); // events are only added by the WindowedRenderer
}

TEST_F(ARendererCommandExecutor, createOffscreenBufferAndGenerateEvent)
{
    constexpr DisplayHandle display{ 1 };
    constexpr OffscreenBufferHandle buffer{ 2 };

    m_commandBuffer.enqueueCommand(RendererCommand::CreateOffscreenBuffer{ display, buffer, 1u, 2u, 3u, true, ERenderBufferType_DepthStencilBuffer });
    EXPECT_CALL(m_sceneUpdater, handleBufferCreateRequest(buffer, display, 1u, 2u, 3u, true, ERenderBufferType_DepthStencilBuffer)).WillOnce(Return(true));
    doCommandExecutorLoop();

    const RendererEventVector events = consumeRendererEvents();
    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(ERendererEventType::OffscreenBufferCreated, events.front().eventType);
    EXPECT_EQ(display, events.front().displayHandle);
    EXPECT_EQ(buffer, events.front().offscreenBuffer);
}

TEST_F(ARendererCommandExecutor, createOffscreenBufferAndGenerateEventOnFail)
{
    constexpr DisplayHandle display{ 1 };
    constexpr OffscreenBufferHandle buffer{ 2 };

    m_commandBuffer.enqueueCommand(RendererCommand::CreateOffscreenBuffer{ display, buffer, 1u, 2u, 3u, true, ERenderBufferType_DepthBuffer });
    EXPECT_CALL(m_sceneUpdater, handleBufferCreateRequest(buffer, display, 1u, 2u, 3u, true, ERenderBufferType_DepthBuffer)).WillOnce(Return(false));
    doCommandExecutorLoop();

    const RendererEventVector events = consumeRendererEvents();
    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(ERendererEventType::OffscreenBufferCreateFailed, events.front().eventType);
    EXPECT_EQ(display, events.front().displayHandle);
    EXPECT_EQ(buffer, events.front().offscreenBuffer);
}

TEST_F(ARendererCommandExecutor, destroysOffscreenBufferAndGenerateEvent)
{
    constexpr DisplayHandle display{ 1 };
    constexpr OffscreenBufferHandle buffer{ 2 };

    m_commandBuffer.enqueueCommand(RendererCommand::DestroyOffscreenBuffer{ display, buffer });
    EXPECT_CALL(m_sceneUpdater, handleBufferDestroyRequest(buffer, display)).WillOnce(Return(true));
    doCommandExecutorLoop();

    const RendererEventVector events = consumeRendererEvents();
    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(ERendererEventType::OffscreenBufferDestroyed, events.front().eventType);
    EXPECT_EQ(display, events.front().displayHandle);
    EXPECT_EQ(buffer, events.front().offscreenBuffer);
}

TEST_F(ARendererCommandExecutor, destroysOffscreenBufferAndGenerateEventOnFail)
{
    constexpr DisplayHandle display{ 1 };
    constexpr OffscreenBufferHandle buffer{ 2 };

    m_commandBuffer.enqueueCommand(RendererCommand::DestroyOffscreenBuffer{ display, buffer });
    EXPECT_CALL(m_sceneUpdater, handleBufferDestroyRequest(buffer, display)).WillOnce(Return(false));
    doCommandExecutorLoop();

    const RendererEventVector events = consumeRendererEvents();
    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(ERendererEventType::OffscreenBufferDestroyFailed, events.front().eventType);
    EXPECT_EQ(display, events.front().displayHandle);
    EXPECT_EQ(buffer, events.front().offscreenBuffer);
}

TEST_F(ARendererCommandExecutor, triggersLinkOffscreenBufferToConsumer)
{
    const OffscreenBufferHandle buffer(1u);
    const SceneId consumerScene(2u);
    const DataSlotId consumerData(3u);
    m_commandBuffer.enqueueCommand(RendererCommand::LinkOffscreenBuffer{ buffer, consumerScene, consumerData });

    EXPECT_CALL(m_sceneUpdater, handleBufferToSceneDataLinkRequest(buffer, consumerScene, consumerData));
    doCommandExecutorLoop();
}

TEST_F(ARendererCommandExecutor, createStreamBuffer)
{
    constexpr StreamBufferHandle buffer{ 123u };
    constexpr DisplayHandle display{ 1 };
    constexpr WaylandIviSurfaceId source{ 2 };

    m_commandBuffer.enqueueCommand(RendererCommand::CreateStreamBuffer{ display, buffer, source });
    EXPECT_CALL(m_sceneUpdater, handleBufferCreateRequest(buffer, display, source));
    doCommandExecutorLoop();
}

TEST_F(ARendererCommandExecutor, destroysStreamBuffer)
{
    constexpr StreamBufferHandle buffer{ 123u };
    constexpr DisplayHandle display{ 1 };

    m_commandBuffer.enqueueCommand(RendererCommand::DestroyStreamBuffer{ display, buffer });
    EXPECT_CALL(m_sceneUpdater, handleBufferDestroyRequest(buffer, display));
    doCommandExecutorLoop();
}

TEST_F(ARendererCommandExecutor, linkStreamBufferToConsumer)
{
    constexpr StreamBufferHandle buffer{ 1u };
    constexpr SceneId consumerScene{ 2u };
    constexpr DataSlotId consumerData{ 3u };
    m_commandBuffer.enqueueCommand(RendererCommand::LinkStreamBuffer{ buffer, consumerScene, consumerData });

    EXPECT_CALL(m_sceneUpdater, handleBufferToSceneDataLinkRequest(buffer, consumerScene, consumerData));
    doCommandExecutorLoop();
}

TEST_F(ARendererCommandExecutor, setsStreamBufferState)
{
    constexpr StreamBufferHandle buffer{ 123u };
    constexpr DisplayHandle display{ 1 };

    m_commandBuffer.enqueueCommand(RendererCommand::SetStreamBufferState{ display, buffer, true });
    EXPECT_CALL(m_sceneUpdater, setStreamBufferState(buffer, display, true));
    doCommandExecutorLoop();
}

TEST_F(ARendererCommandExecutor, createsAndDestroysDisplay)
{
    const DisplayHandle displayHandle(33u);
    const DisplayConfig displayConfig;

    m_commandBuffer.enqueueCommand(RendererCommand::CreateDisplay{ displayHandle, displayConfig, nullptr });
    EXPECT_CALL(m_sceneUpdater, createDisplayContext(displayConfig, displayHandle, nullptr));
    doCommandExecutorLoop();

    m_commandBuffer.enqueueCommand(RendererCommand::DestroyDisplay{ displayHandle });
    EXPECT_CALL(m_sceneUpdater, destroyDisplayContext(displayHandle));
    doCommandExecutorLoop();
}

TEST_F(ARendererCommandExecutor, linkData)
{
    constexpr SceneId sceneProviderId(1u);
    constexpr SceneId sceneConsumerId(2u);
    constexpr DataSlotId providerSlotId(3u);
    constexpr DataSlotId consumerSlotId(4u);

    m_commandBuffer.enqueueCommand(RendererCommand::LinkData{ sceneProviderId, providerSlotId, sceneConsumerId, consumerSlotId });
    EXPECT_CALL(m_sceneUpdater, handleSceneDataLinkRequest(sceneProviderId, providerSlotId, sceneConsumerId, consumerSlotId));
    doCommandExecutorLoop();
}

TEST_F(ARendererCommandExecutor, unlinkData)
{
    constexpr SceneId sceneConsumerId(2u);
    constexpr DataSlotId consumerSlotId(4u);

    m_commandBuffer.enqueueCommand(RendererCommand::UnlinkData{ sceneConsumerId, consumerSlotId });
    EXPECT_CALL(m_sceneUpdater, handleDataUnlinkRequest(sceneConsumerId, consumerSlotId));
    doCommandExecutorLoop();
}

TEST_F(ARendererCommandExecutor, processesSceneActions)
{
    const SceneId sceneId(7u);
    const NodeHandle node(33u);

    SceneUpdate actions;
    SceneActionCollectionCreator creator(actions.actions);
    creator.allocateNode(0u, node);
    actions.flushInfos.flushCounter = 1u;
    actions.flushInfos.containsValidInformation = true;
    m_commandBuffer.enqueueCommand(RendererCommand::UpdateScene{ sceneId, std::move(actions) });

    EXPECT_CALL(m_sceneUpdater, handleSceneUpdate(sceneId, _));
    doCommandExecutorLoop();
}

TEST_F(ARendererCommandExecutor, setsLayerVisibility)
{
    m_commandBuffer.enqueueCommand(RendererCommand::SCSetIviLayerVisibility{ WaylandIviLayerId(23u), true });
    m_commandBuffer.enqueueCommand(RendererCommand::SCSetIviLayerVisibility{ WaylandIviLayerId(25u), false });

    SystemCompositorControllerMock* systemCompositorMock = getSystemCompositorMock();
    if(systemCompositorMock != nullptr)
    {
        EXPECT_CALL(*systemCompositorMock, setLayerVisibility(WaylandIviLayerId(23u),true));
        EXPECT_CALL(*systemCompositorMock, setLayerVisibility(WaylandIviLayerId(25u),false));
    }
    m_commandExecutor.executePendingCommands();
}

TEST_F(ARendererCommandExecutor, setsSurfaceVisibility)
{
    m_commandBuffer.enqueueCommand(RendererCommand::SCSetIviSurfaceVisibility{ WaylandIviSurfaceId(11u), true });
    m_commandBuffer.enqueueCommand(RendererCommand::SCSetIviSurfaceVisibility{ WaylandIviSurfaceId(22u), false });

    SystemCompositorControllerMock* systemCompositorMock = getSystemCompositorMock();
    if(systemCompositorMock != nullptr)
    {
        EXPECT_CALL(*systemCompositorMock, setSurfaceVisibility(WaylandIviSurfaceId(11u),true));
        EXPECT_CALL(*systemCompositorMock, setSurfaceVisibility(WaylandIviSurfaceId(22u),false));
    }
    doCommandExecutorLoop();
}

TEST_F(ARendererCommandExecutor, listsIviSurfaces)
{
    m_commandBuffer.enqueueCommand(RendererCommand::SCListIviSurfaces{});

    SystemCompositorControllerMock* systemCompositorMock = getSystemCompositorMock();
    if(systemCompositorMock != nullptr)
    {
        EXPECT_CALL(*systemCompositorMock, listIVISurfaces());
    }
    doCommandExecutorLoop();
}

TEST_F(ARendererCommandExecutor, setsSurfaceOpacity)
{
    m_commandBuffer.enqueueCommand(RendererCommand::SCSetIviSurfaceOpacity{ WaylandIviSurfaceId(1u), 1.f });
    m_commandBuffer.enqueueCommand(RendererCommand::SCSetIviSurfaceOpacity{ WaylandIviSurfaceId(2u), 0.2f });

    SystemCompositorControllerMock* systemCompositorMock = getSystemCompositorMock();
    if(systemCompositorMock != nullptr)
    {
        EXPECT_CALL(*systemCompositorMock, setSurfaceOpacity(WaylandIviSurfaceId(1u),1.f));
        EXPECT_CALL(*systemCompositorMock, setSurfaceOpacity(WaylandIviSurfaceId(2u),0.2f));
    }
    doCommandExecutorLoop();
}

TEST_F(ARendererCommandExecutor, setsSurfaceRectangle)
{
    m_commandBuffer.enqueueCommand(RendererCommand::SCSetIviSurfaceDestRectangle{ WaylandIviSurfaceId(111u), 0, 0, 640, 480 });
    m_commandBuffer.enqueueCommand(RendererCommand::SCSetIviSurfaceDestRectangle{ WaylandIviSurfaceId(222u), -1,-10, 1920, 1080 });

    SystemCompositorControllerMock* systemCompositorMock = getSystemCompositorMock();
    if(systemCompositorMock != nullptr)
    {
        EXPECT_CALL(*systemCompositorMock, setSurfaceDestinationRectangle(WaylandIviSurfaceId(111u), 0, 0, 640, 480));
        EXPECT_CALL(*systemCompositorMock, setSurfaceDestinationRectangle(WaylandIviSurfaceId(222u), -1,-10, 1920, 1080));
    }
    doCommandExecutorLoop();
}

TEST_F(ARendererCommandExecutor, callsSystemCompositorAddSurfaceToLayer)
{
    m_commandBuffer.enqueueCommand(RendererCommand::SCAddIviSurfaceToIviLayer{ WaylandIviSurfaceId(222u), WaylandIviLayerId(111u) });

    SystemCompositorControllerMock* systemCompositorMock = getSystemCompositorMock();
    if(systemCompositorMock != nullptr)
    {
        EXPECT_CALL(*systemCompositorMock, addSurfaceToLayer(WaylandIviSurfaceId(222u), WaylandIviLayerId(111u)));
    }
    doCommandExecutorLoop();
}

TEST_F(ARendererCommandExecutor, callsSystemCompositorRemoveSurfaceFromLayer)
{
    m_commandBuffer.enqueueCommand(RendererCommand::SCRemoveIviSurfaceFromIviLayer{ WaylandIviSurfaceId(345u), WaylandIviLayerId(123u) });

    SystemCompositorControllerMock* scc = getSystemCompositorMock();
    if(scc)
    {
        EXPECT_CALL(*scc, removeSurfaceFromLayer(WaylandIviSurfaceId(345u), WaylandIviLayerId(123u)));
    }
    doCommandExecutorLoop();
}

TEST_F(ARendererCommandExecutor, callsSystemCompositorDestroySurface)
{
    m_commandBuffer.enqueueCommand(RendererCommand::SCDestroyIviSurface{ WaylandIviSurfaceId(51u) });

    SystemCompositorControllerMock* scc = getSystemCompositorMock();
    if(scc)
    {
        EXPECT_CALL(*scc, destroySurface(WaylandIviSurfaceId(51u)));
    }
    doCommandExecutorLoop();
}

TEST_F(ARendererCommandExecutor, callsSystemCompositorScreenshot)
{
    const ramses_internal::String firstName("somefilename.png");
    const ramses_internal::String otherName("somethingelse.png");

    m_commandBuffer.enqueueCommand(RendererCommand::SCScreenshot{ 1, firstName });
    m_commandBuffer.enqueueCommand(RendererCommand::SCScreenshot{ -1, otherName });

    SystemCompositorControllerMock* systemCompositorMock = getSystemCompositorMock();
    if(systemCompositorMock != nullptr)
    {
        EXPECT_CALL(*systemCompositorMock, doScreenshot(firstName, 1));
        EXPECT_CALL(*systemCompositorMock, doScreenshot(otherName, -1));
    }
    doCommandExecutorLoop();
}

TEST_F(ARendererCommandExecutor, setClearFlags)
{
    constexpr DisplayHandle display{ 1 };
    constexpr OffscreenBufferHandle buffer{ 2 };

    m_commandBuffer.enqueueCommand(RendererCommand::SetClearFlags{ display, buffer, EClearFlags_Color });
    EXPECT_CALL(m_sceneUpdater, handleSetClearFlags(display, buffer, EClearFlags_Color));
    doCommandExecutorLoop();
}

TEST_F(ARendererCommandExecutor, setClearColor)
{
    constexpr DisplayHandle display{ 1 };
    constexpr OffscreenBufferHandle buffer{ 2 };
    constexpr Vector4 clearColor(1.f, 0.f, 0.2f, 0.3f);

    m_commandBuffer.enqueueCommand(RendererCommand::SetClearColor{ display, buffer, clearColor });
    EXPECT_CALL(m_sceneUpdater, handleSetClearColor(display, buffer, clearColor));
    doCommandExecutorLoop();
}

TEST_F(ARendererCommandExecutor, setFrameTimerLimits)
{
    //default values
    ASSERT_EQ(PlatformTime::InfiniteDuration, m_frameTimer.getTimeBudgetForSection(EFrameTimerSectionBudget::SceneResourcesUpload));
    ASSERT_EQ(PlatformTime::InfiniteDuration, m_frameTimer.getTimeBudgetForSection(EFrameTimerSectionBudget::ResourcesUpload));
    ASSERT_EQ(PlatformTime::InfiniteDuration, m_frameTimer.getTimeBudgetForSection(EFrameTimerSectionBudget::OffscreenBufferRender));

    m_commandBuffer.enqueueCommand(RendererCommand::SetLimits_FrameBudgets{ 4u, 1u, 3u });
    doCommandExecutorLoop();

    EXPECT_EQ(std::chrono::microseconds(4u), m_frameTimer.getTimeBudgetForSection(EFrameTimerSectionBudget::SceneResourcesUpload));
    EXPECT_EQ(std::chrono::microseconds(1u), m_frameTimer.getTimeBudgetForSection(EFrameTimerSectionBudget::ResourcesUpload));
    EXPECT_EQ(std::chrono::microseconds(3u), m_frameTimer.getTimeBudgetForSection(EFrameTimerSectionBudget::OffscreenBufferRender));
}

TEST_F(ARendererCommandExecutor, forwardsPickEventToSceneUpdater)
{
    const SceneId sceneId{ 123u };
    const Vector2 coords{ 0.1f, 0.2f };
    m_commandBuffer.enqueueCommand(RendererCommand::PickEvent{ sceneId, coords });
    EXPECT_CALL(m_sceneUpdater, handlePickEvent(sceneId, coords));
    doCommandExecutorLoop();
}

TEST_F(ARendererCommandExecutor, triggersLogRinfo)
{
    m_commandBuffer.enqueueCommand(RendererCommand::LogInfo{ ERendererLogTopic::Displays, true, NodeHandle{ 6u } });
    EXPECT_CALL(m_sceneUpdater, logRendererInfo(ERendererLogTopic::Displays, true, NodeHandle{ 6u }));
    doCommandExecutorLoop();
}
}
