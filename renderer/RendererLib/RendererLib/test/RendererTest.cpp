//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "RendererLib/RendererConfig.h"
#include "RenderBackendMock.h"
#include "ResourceProviderMock.h"
#include "PlatformFactoryMock.h"
#include "RendererLib/RendererCommandBuffer.h"
#include "RendererLib/RendererCachedScene.h"
#include "RendererLib/DisplayController.h"
#include "RendererLib/RendererScenes.h"
#include "RendererLib/RendererLogContext.h"
#include "RendererLib/DisplayEventHandlerManager.h"
#include "RendererLib/SceneExpirationMonitor.h"
#include "RendererEventCollector.h"
#include "DisplayControllerMock.h"
#include "Collections/Pair.h"
#include "RendererMock.h"
#include "ComponentMocks.h"
#include "TestSceneHelper.h"
#include <map>

using namespace ramses_internal;

class ARenderer : public ::testing::TestWithParam<bool>
{
public:
    ARenderer()
        : platformFactoryMock(GetParam())
        , rendererScenes(rendererEventCollector)
        , expirationMonitor(rendererScenes, rendererEventCollector)
        , renderer(platformFactoryMock, rendererScenes, rendererEventCollector, expirationMonitor, rendererStatistics)
    {
        sceneRenderInterrupted.incrementRenderableIdx();
        sceneRenderInterrupted2.incrementRenderableIdx();
        sceneRenderInterrupted2.incrementRenderableIdx();
    }

    ~ARenderer()
    {
        for (const auto& dispIt : createdDisplays)
            renderer.destroyDisplayContext(dispIt.first);
        for (const auto& sceneIt : rendererScenes)
            expirationMonitor.stopMonitoringScene(sceneIt.key);
    }

    DisplayHandle addDisplayController()
    {
        static const DisplayConfig dummyConfig;
        const DisplayHandle handle(static_cast<UInt32>(createdDisplays.size()));
        renderer.createDisplayContext(dummyConfig, handle);
        renderer.getDisplayController(handle).getRenderBackend();
        createdDisplays.emplace(handle, 0);

        return handle;
    }

    void destroyDisplayController(DisplayHandle display)
    {
        renderer.destroyDisplayContext(display);
        createdDisplays.erase(display);
    }

    void expectOffscreenBufferRendered(DisplayHandle display, std::initializer_list<DeviceResourceHandle> buffers, bool withContextEnable = false)
    {
        DisplayStrictMockInfo& displayMock = renderer.getDisplayMock(display);
        if (withContextEnable)
            EXPECT_CALL(*displayMock.m_displayController, enableContext()).InSequence(SeqRender);
        for (auto buffer : buffers)
            EXPECT_CALL(*displayMock.m_displayController, clearBuffer(buffer, Renderer::DefaultClearColor)).InSequence(SeqRender);
    }

    void expectInterruptibleOffscreenBufferRendered(DisplayHandle display, std::initializer_list<DeviceResourceHandle> buffers, const std::vector< std::pair<bool, bool> >& expectClearAndSwapFlags = {}, bool withContextEnable = false)
    {
        DisplayStrictMockInfo& displayMock = renderer.getDisplayMock(display);
        if (withContextEnable)
            EXPECT_CALL(*displayMock.m_displayController, enableContext()).InSequence(SeqRender);

        UInt32 bufferIdx = 0u;
        for (auto buffer : buffers)
        {
            const bool expectClear = expectClearAndSwapFlags.empty() || expectClearAndSwapFlags[bufferIdx].first;
            const bool expectSwap = expectClearAndSwapFlags.empty() || expectClearAndSwapFlags[bufferIdx].second;

            if (expectClear)
                EXPECT_CALL(*displayMock.m_displayController, clearBuffer(buffer, Renderer::DefaultClearColor)).InSequence(SeqRender);
            if (expectSwap)
            {
                EXPECT_CALL(*displayMock.m_displayController, getRenderBackend()).InSequence(SeqRender);
                EXPECT_CALL(displayMock.m_renderBackend->deviceMock, swapDoubleBufferedRenderTarget(buffer)).InSequence(SeqRender);
            }

            ++bufferIdx;
        }
    }

    void expectFrameBufferRendered(DisplayHandle display = DisplayHandle(0u), bool expectContextEnable = true, bool expectRerender = true, const Vector4& clearColor = Renderer::DefaultClearColor)
    {
        DisplayStrictMockInfo& displayMock = renderer.getDisplayMock(display);

        EXPECT_CALL(*displayMock.m_displayController, handleWindowEvents()).InSequence(SeqPreRender);
        EXPECT_CALL(*displayMock.m_displayController, canRenderNewFrame()).InSequence(SeqPreRender).WillOnce(Return(true));

        if (expectContextEnable)
            EXPECT_CALL(*displayMock.m_displayController, enableContext()).InSequence(SeqRender);
        if (expectRerender)
        {
            EXPECT_CALL(*displayMock.m_displayController, clearBuffer(DisplayControllerMock::FakeFrameBufferHandle, clearColor)).InSequence(SeqRender);
            EXPECT_CALL(*displayMock.m_displayController, executePostProcessing()).InSequence(SeqRender);
        }
        else
        {
            EXPECT_CALL(*displayMock.m_displayController, getEmbeddedCompositingManager()).InSequence(SeqRender);
            EXPECT_CALL(*displayMock.m_embeddedCompositingManager, notifyClients()).InSequence(SeqRender);
        }
    }

    void expectSwapBuffers(DisplayHandle display = DisplayHandle(0u), bool withContextEnable = false)
    {
        DisplayStrictMockInfo& displayMock = renderer.getDisplayMock(display);
        if (withContextEnable)
            EXPECT_CALL(*displayMock.m_displayController, enableContext()).InSequence(SeqRender);
        EXPECT_CALL(*displayMock.m_displayController, swapBuffers()).InSequence(SeqRender);
        EXPECT_CALL(*displayMock.m_displayController, getEmbeddedCompositingManager()).InSequence(SeqRender);
        EXPECT_CALL(*displayMock.m_embeddedCompositingManager, notifyClients()).InSequence(SeqRender);
    }

    void doOneRendererLoop()
    {
        if(GetParam())
            EXPECT_CALL(platformFactoryMock.windowEventsPollingManagerMock, pollWindowsTillAnyCanRender()).Times(1u);

        renderer.getProfilerStatistics().markFrameFinished(std::chrono::microseconds{ 0u });
        renderer.doOneRenderLoop();
    }

    void expectSceneRendered(DisplayHandle displayHandle, SceneId sceneId, DeviceResourceHandle buffer = DisplayControllerMock::FakeFrameBufferHandle)
    {
        DisplayStrictMockInfo& displayMock = renderer.getDisplayMock(displayHandle);
        EXPECT_CALL(*displayMock.m_displayController, renderScene(Ref(rendererScenes.getScene(sceneId)), buffer, _, sceneRenderBegin, nullptr));
    }

    void expectSceneRenderedWithInterruptionEnabled(DisplayHandle displayHandle, SceneId sceneId, DeviceResourceHandle buffer, SceneRenderExecutionIterator expectedRenderBegin, SceneRenderExecutionIterator stateToSimulate)
    {
        DisplayStrictMockInfo& displayMock = renderer.getDisplayMock(displayHandle);
        EXPECT_CALL(*displayMock.m_displayController, renderScene(Ref(rendererScenes.getScene(sceneId)), buffer, _, expectedRenderBegin, &renderer.FrameTimerInstance)).WillOnce(Return(stateToSimulate));
    }

    IScene& createScene(SceneId sceneId = SceneId())
    {
        rendererScenes.createScene(SceneInfo(sceneId));
        return rendererScenes.getScene(sceneId);
    }

    void mapSceneToDisplayBuffer(SceneId sceneId, DisplayHandle displayHandle, Int32 sceneRenderOrder, DeviceResourceHandle displayBuffer = DisplayControllerMock::FakeFrameBufferHandle)
    {
        renderer.mapSceneToDisplayBuffer(sceneId, displayHandle, displayBuffer, sceneRenderOrder);
        EXPECT_EQ(displayHandle, renderer.getDisplaySceneIsMappedTo(sceneId));
        DisplayHandle displayMapped;
        EXPECT_EQ(displayBuffer, renderer.getBufferSceneIsMappedTo(sceneId, &displayMapped));
        EXPECT_EQ(displayHandle, displayMapped);
        EXPECT_EQ(sceneRenderOrder, renderer.getSceneGlobalOrder(sceneId));
    }

    void unmapScene(SceneId sceneId)
    {
        renderer.unmapScene(sceneId);
        EXPECT_FALSE(renderer.getDisplaySceneIsMappedTo(sceneId).isValid());
        DisplayHandle displayMapped;
        EXPECT_FALSE(renderer.getBufferSceneIsMappedTo(sceneId, &displayMapped).isValid());
        EXPECT_FALSE(displayMapped.isValid());
    }

    void showScene(SceneId sceneId)
    {
        renderer.setSceneShown(sceneId, true);
    }

    void hideScene(SceneId sceneId)
    {
        renderer.setSceneShown(sceneId, false);
    }

    SystemCompositorControllerMock* getSystemCompositorMock()
    {
        return static_cast<SystemCompositorControllerMock*>(platformFactoryMock.getSystemCompositorController());
    }

    void initiateExpirationMonitoring(std::initializer_list<SceneId> scenes)
    {
        // A workaround to be able to check if scene was reported as rendered.
        // Expiration monitor holds TS for applied flushes and TS for rendered scene.
        // On rendered the TS of rendered scene is simply TS of last applied flush.
        // Rendered scene TS can either be invalid - never reported as rendered,
        // or the value below if reported as rendered
        for (auto sceneId : scenes)
            expirationMonitor.onFlushApplied(sceneId, currentFakeTime, {}, 0);
    }

    void expectScenesReportedToExpirationMonitorAsRendered(std::initializer_list<SceneId> expectedScenesToBeReported)
    {
        for (auto sceneId : expectedScenesToBeReported)
        {
            EXPECT_NE(FlushTime::InvalidTimestamp, expirationMonitor.getExpirationTimestampOfRenderedScene(sceneId));
        }
    }

protected:
    StrictMock<PlatformFactoryStrictMock>       platformFactoryMock;
    RendererCommandBuffer                       rendererCommandBuffer;
    RendererEventCollector                      rendererEventCollector;
    RendererScenes                              rendererScenes;
    SceneExpirationMonitor                      expirationMonitor;
    RendererStatistics                          rendererStatistics;
    StrictMock<RendererMockWithStrictMockDisplay> renderer;

    const SceneRenderExecutionIterator sceneRenderBegin{};
    SceneRenderExecutionIterator sceneRenderInterrupted;
    SceneRenderExecutionIterator sceneRenderInterrupted2;

    // must be ordered map or other sorted container so that order of rendering matches (renderer uses ordered map internally)
    std::map<DisplayHandle, Int32> createdDisplays;

    // sequence of ordered expectations during render
    Sequence SeqRender;
    // sequence of ordered expectations at beginning of render (display events, can render frame, etc.)
    Sequence SeqPreRender;

    const FlushTime::Clock::time_point currentFakeTime{ std::chrono::milliseconds(1000) };
};

INSTANTIATE_TEST_CASE_P(, ARenderer, ::testing::Values(false, true));

TEST_P(ARenderer, ListIviSurfacesInSystemCompositorController)
{
    SystemCompositorControllerMock* systemCompositorMock = getSystemCompositorMock();
    if(systemCompositorMock != nullptr)
    {
        EXPECT_CALL(*systemCompositorMock, listIVISurfaces());
    }
    renderer.systemCompositorListIviSurfaces();
}

TEST_P(ARenderer, SetsVisibilityInSystemCompositorController)
{
    SystemCompositorControllerMock* systemCompositorMock = getSystemCompositorMock();
    if(systemCompositorMock != nullptr)
    {
        EXPECT_CALL(*systemCompositorMock, setSurfaceVisibility(WaylandIviSurfaceId(1), false));
        EXPECT_CALL(*systemCompositorMock, setSurfaceVisibility(WaylandIviSurfaceId(2), true));
    }
    renderer.systemCompositorSetIviSurfaceVisibility(WaylandIviSurfaceId(1), false);
    renderer.systemCompositorSetIviSurfaceVisibility(WaylandIviSurfaceId(2), true);
}

TEST_P(ARenderer, TakesScreenshotFromSystemCompositorController)
{
    String fileName("screenshot.png");
    const int32_t screenIviId = 3;
    SystemCompositorControllerMock* systemCompositorMock = getSystemCompositorMock();
    if(systemCompositorMock != nullptr)
    {
        EXPECT_CALL(*systemCompositorMock, doScreenshot(fileName, screenIviId));
    }
    renderer.systemCompositorScreenshot(fileName, screenIviId);
}

TEST_P(ARenderer, rendersOneLoopWithSingleDisplayController)
{
    addDisplayController();
    EXPECT_EQ(1u, renderer.getDisplayControllerCount());

    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();
}

TEST_P(ARenderer, rendersOneLoopWithTwoDisplayControllers)
{
    const DisplayHandle display1 = addDisplayController();
    const DisplayHandle display2 = addDisplayController();
    EXPECT_EQ(2u, renderer.getDisplayControllerCount());

    expectFrameBufferRendered(display1);
    expectFrameBufferRendered(display2);
    expectSwapBuffers(display2);
    expectSwapBuffers(display1, true);
    doOneRendererLoop();
}

TEST_P(ARenderer, unregisteredSceneIsNotMapped)
{
    EXPECT_FALSE(renderer.getDisplaySceneIsMappedTo(SceneId(0u)).isValid());
}

TEST_P(ARenderer, doesNotMapCreatedScene)
{
    const SceneId sceneId(12u);
    createScene(sceneId);
    EXPECT_FALSE(renderer.getDisplaySceneIsMappedTo(sceneId).isValid());
}

TEST_P(ARenderer, canMapSceneOnDisplay)
{
    const DisplayHandle displayHandle = addDisplayController();

    const SceneId sceneId(12u);
    createScene(sceneId);
    mapSceneToDisplayBuffer(sceneId, displayHandle, 0);
    unmapScene(sceneId);
}

TEST_P(ARenderer, assignsSceneToNativeFramebufferOfDisplayWhenMappingScene)
{
    const DisplayHandle displayHandle = addDisplayController();

    const SceneId sceneId(12u);
    createScene(sceneId);
    mapSceneToDisplayBuffer(sceneId, displayHandle, 0);

    EXPECT_EQ(DisplayControllerMock::FakeFrameBufferHandle, renderer.getBufferSceneIsMappedTo(sceneId));
    unmapScene(sceneId);
}

TEST_P(ARenderer, assignsSceneToOffscreenBuffer)
{
    const DisplayHandle displayHandle = addDisplayController();

    const SceneId sceneId(12u);
    createScene(sceneId);
    const DeviceResourceHandle fakeOffscreenBuffer(313u);
    renderer.registerOffscreenBuffer(displayHandle, fakeOffscreenBuffer, 1u, 1u, false);
    mapSceneToDisplayBuffer(sceneId, displayHandle, 0, fakeOffscreenBuffer);
    EXPECT_EQ(fakeOffscreenBuffer, renderer.getBufferSceneIsMappedTo(sceneId));
    EXPECT_FALSE(renderer.isSceneMappedToInterruptibleOffscreenBuffer(sceneId));

    unmapScene(sceneId);
}

TEST_P(ARenderer, assignsSceneToInterruptibleOffscreenBuffer)
{
    const DisplayHandle displayHandle = addDisplayController();

    const SceneId sceneId(12u);
    createScene(sceneId);
    const DeviceResourceHandle fakeOffscreenBuffer(313u);
    renderer.registerOffscreenBuffer(displayHandle, fakeOffscreenBuffer, 1u, 1u, true);
    mapSceneToDisplayBuffer(sceneId, displayHandle, 0, fakeOffscreenBuffer);
    EXPECT_EQ(fakeOffscreenBuffer, renderer.getBufferSceneIsMappedTo(sceneId));
    EXPECT_TRUE(renderer.isSceneMappedToInterruptibleOffscreenBuffer(sceneId));

    unmapScene(sceneId);
}

TEST_P(ARenderer, assignsSceneFromInterruptibleOffscreenBufferToNormalOB)
{
    const DisplayHandle displayHandle = addDisplayController();

    const SceneId sceneId(12u);
    createScene(sceneId);
    const DeviceResourceHandle ob(313u);
    const DeviceResourceHandle obInterruptible(314u);
    renderer.registerOffscreenBuffer(displayHandle, ob, 1u, 1u, false);
    renderer.registerOffscreenBuffer(displayHandle, obInterruptible, 1u, 1u, true);

    mapSceneToDisplayBuffer(sceneId, displayHandle, 0, obInterruptible);
    EXPECT_EQ(obInterruptible, renderer.getBufferSceneIsMappedTo(sceneId));
    EXPECT_TRUE(renderer.isSceneMappedToInterruptibleOffscreenBuffer(sceneId));

    mapSceneToDisplayBuffer(sceneId, displayHandle, 0, ob);
    EXPECT_EQ(ob, renderer.getBufferSceneIsMappedTo(sceneId));
    EXPECT_FALSE(renderer.isSceneMappedToInterruptibleOffscreenBuffer(sceneId));

    unmapScene(sceneId);
}

TEST_P(ARenderer, clearsOffscreenBufferIfThereIsSceneAssignedToIt)
{
    const DisplayHandle displayHandle = addDisplayController();

    const SceneId sceneId(12u);
    createScene(sceneId);

    const DeviceResourceHandle fakeOffscreenBuffer(313u);
    renderer.registerOffscreenBuffer(displayHandle, fakeOffscreenBuffer, 1u, 2u, false);
    mapSceneToDisplayBuffer(sceneId, displayHandle, 0, fakeOffscreenBuffer);
    showScene(sceneId);

    expectOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, true);
    expectFrameBufferRendered(displayHandle, false);
    expectSceneRendered(displayHandle, sceneId, fakeOffscreenBuffer);
    expectSwapBuffers();

    doOneRendererLoop();

    hideScene(sceneId);
    unmapScene(sceneId);
}

TEST_P(ARenderer, SetsLayerVisibilityInSystemCompositorController)
{
    SystemCompositorControllerMock* systemCompositorMock = getSystemCompositorMock();
    if(systemCompositorMock != nullptr)
    {
        EXPECT_CALL(*systemCompositorMock, setLayerVisibility(WaylandIviLayerId(18u), false));
        EXPECT_CALL(*systemCompositorMock, setLayerVisibility(WaylandIviLayerId(17u), true));
    }
    renderer.systemCompositorSetIviLayerVisibility(WaylandIviLayerId(18u), false);
    renderer.systemCompositorSetIviLayerVisibility(WaylandIviLayerId(17u), true);
}

TEST_P(ARenderer, doesNotClearOffscreenBufferIfThereIsNoSceneAssignedToIt)
{
    const DisplayHandle displayHandle = addDisplayController();

    const DeviceResourceHandle fakeOffscreenBuffer(313u);
    renderer.registerOffscreenBuffer(displayHandle, fakeOffscreenBuffer, 1u, 2u, false);

    expectFrameBufferRendered();
    // no offscreen buffer clear expectation
    expectSwapBuffers();
    doOneRendererLoop();
}

TEST_P(ARenderer, clearsOffscreenBufferIfThereIsSceneAssignedToItAndNotShown)
{
    const DisplayHandle displayHandle = addDisplayController();

    const SceneId sceneId(12u);
    createScene(sceneId);

    const DeviceResourceHandle fakeOffscreenBuffer(313u);
    renderer.registerOffscreenBuffer(displayHandle, fakeOffscreenBuffer, 1u, 2u, false);
    mapSceneToDisplayBuffer(sceneId, displayHandle, 0, fakeOffscreenBuffer);

    expectOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, true);
    expectFrameBufferRendered(displayHandle, false);
    expectSwapBuffers();
    doOneRendererLoop();

    unmapScene(sceneId);
}

TEST_P(ARenderer, clearsOffscreenBufferAndFramebufferWithRelatedColors)
{
    const Vector4 displayClearColor(.1f, .2f, .3f, .4f);
    const DisplayHandle displayHandle = addDisplayController();
    renderer.setClearColor(displayHandle, displayClearColor);

    const SceneId sceneId(12u);
    createScene(sceneId);

    const DeviceResourceHandle fakeOffscreenBuffer(313u);
    renderer.registerOffscreenBuffer(displayHandle, fakeOffscreenBuffer, 1u, 2u, false);
    mapSceneToDisplayBuffer(sceneId, displayHandle, 0, fakeOffscreenBuffer);

    expectOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, true);
    expectFrameBufferRendered(displayHandle, false, true, displayClearColor);
    expectSwapBuffers();
    doOneRendererLoop();

    unmapScene(sceneId);
}

TEST_P(ARenderer, clearsFramebufferWithCustomClearColor)
{
    const Vector4 displayClearColor(.1f, .2f, .3f, .4f);
    const DisplayHandle displayHandle = addDisplayController();
    renderer.setClearColor(displayHandle, displayClearColor);
    expectFrameBufferRendered(displayHandle, true, true, displayClearColor);
    expectSwapBuffers();
    doOneRendererLoop();
}

TEST_P(ARenderer, twoDisplaysAndTwoOffscreenBuffersWithContentCauseCorrectNumberOfEnableContexts)
{
    const DisplayHandle displayHandle1 = addDisplayController();
    const DisplayHandle displayHandle2 = addDisplayController();

    DeviceResourceHandle fakeOffscreenBuffer1(313u);
    DeviceResourceHandle fakeOffscreenBuffer2(314u);
    renderer.registerOffscreenBuffer(displayHandle1, fakeOffscreenBuffer1, 1u, 2u, false);
    renderer.registerOffscreenBuffer(displayHandle2, fakeOffscreenBuffer2, 1u, 2u, false);

    const SceneId sceneId1(12u);
    const SceneId sceneId2(13u);
    createScene(sceneId1);
    createScene(sceneId2);
    mapSceneToDisplayBuffer(sceneId1, displayHandle1, 0, fakeOffscreenBuffer1);
    mapSceneToDisplayBuffer(sceneId2, displayHandle2, 0, fakeOffscreenBuffer2);
    showScene(sceneId1);
    showScene(sceneId2);

    expectOffscreenBufferRendered(displayHandle1, { fakeOffscreenBuffer1 }, true);
    expectSceneRendered(displayHandle1, sceneId1, fakeOffscreenBuffer1);
    expectFrameBufferRendered(displayHandle1, false);

    expectOffscreenBufferRendered(displayHandle2, { fakeOffscreenBuffer2 }, true);
    expectSceneRendered(displayHandle2, sceneId2, fakeOffscreenBuffer2);
    expectFrameBufferRendered(displayHandle2, false);

    expectSwapBuffers(displayHandle2);
    expectSwapBuffers(displayHandle1, true);
    doOneRendererLoop();

    hideScene(sceneId1);
    hideScene(sceneId2);
    unmapScene(sceneId1);
    unmapScene(sceneId2);
}

TEST_P(ARenderer, confidenceTest_clearsOffscreenBufferIfThereIsSceneAssignedToIt_MultipleDisplaysAndBuffers)
{
    const DisplayHandle displayHandle1 = addDisplayController();
    const DisplayHandle displayHandle2 = addDisplayController();

    const DeviceResourceHandle fakeOffscreenBuffer1(313u);
    const DeviceResourceHandle fakeOffscreenBuffer2(314u);
    const DeviceResourceHandle fakeOffscreenBuffer3(315u);
    const DeviceResourceHandle fakeOffscreenBuffer4(316u);
    renderer.registerOffscreenBuffer(displayHandle1, fakeOffscreenBuffer1, 1u, 2u, false);
    renderer.registerOffscreenBuffer(displayHandle1, fakeOffscreenBuffer2, 1u, 2u, false);
    renderer.registerOffscreenBuffer(displayHandle2, fakeOffscreenBuffer3, 1u, 2u, false);
    renderer.registerOffscreenBuffer(displayHandle2, fakeOffscreenBuffer4, 1u, 2u, false);

    const SceneId sceneId1(12u);
    const SceneId sceneId2(13u);
    createScene(sceneId1);
    createScene(sceneId2);

    mapSceneToDisplayBuffer(sceneId2, displayHandle2, 0, fakeOffscreenBuffer3);
    mapSceneToDisplayBuffer(sceneId1, displayHandle1, 0, fakeOffscreenBuffer2);
    showScene(sceneId1);
    showScene(sceneId2);

    expectOffscreenBufferRendered(displayHandle1, { fakeOffscreenBuffer2 }, true);
    expectSceneRendered(displayHandle1, sceneId1, fakeOffscreenBuffer2);
    expectFrameBufferRendered(displayHandle1, false);

    expectOffscreenBufferRendered(displayHandle2, { fakeOffscreenBuffer3 }, true);
    expectSceneRendered(displayHandle2, sceneId2, fakeOffscreenBuffer3);
    expectFrameBufferRendered(displayHandle2, false);

    expectSwapBuffers(displayHandle2);
    expectSwapBuffers(displayHandle1, true);
    doOneRendererLoop();

    hideScene(sceneId1);
    hideScene(sceneId2);
    unmapScene(sceneId1);
    unmapScene(sceneId2);
}

TEST_P(ARenderer, sceneGetsAssignedToFramebufferIfPreviouslyAssignedToOffscreenBufferAndRemapped)
{
    const DisplayHandle displayHandle = addDisplayController();

    const SceneId sceneId(12u);
    createScene(sceneId);

    const DeviceResourceHandle fakeOffscreenBuffer(313u);
    renderer.registerOffscreenBuffer(displayHandle, fakeOffscreenBuffer, 1u, 2u, false);
    mapSceneToDisplayBuffer(sceneId, displayHandle, 0, fakeOffscreenBuffer);
    showScene(sceneId);

    expectOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, true);
    expectSceneRendered(displayHandle, sceneId, fakeOffscreenBuffer);
    expectFrameBufferRendered(displayHandle, false);
    expectSwapBuffers();
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    hideScene(sceneId);
    unmapScene(sceneId);
    expectOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, true);
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    const DeviceResourceHandle framebuffer = DisplayControllerMock::FakeFrameBufferHandle;
    mapSceneToDisplayBuffer(sceneId, displayHandle, 0, framebuffer);
    showScene(sceneId);
    EXPECT_EQ(framebuffer, renderer.getBufferSceneIsMappedTo(sceneId));
    expectFrameBufferRendered();
    expectSceneRendered(displayHandle, sceneId);
    expectSwapBuffers();
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    hideScene(sceneId);
    unmapScene(sceneId);
}

TEST_P(ARenderer, rendersScenesInOrderAccordingToLocalOrderWithinDisplayBuffer)
{
    const DisplayHandle displayHandle = addDisplayController();

    const DeviceResourceHandle fakeOffscreenBuffer1(313u);
    const DeviceResourceHandle fakeOffscreenBuffer2(314u);
    renderer.registerOffscreenBuffer(displayHandle, fakeOffscreenBuffer1, 1u, 2u, false);
    renderer.registerOffscreenBuffer(displayHandle, fakeOffscreenBuffer2, 1u, 2u, false);

    const SceneId sceneId1(12u);
    const SceneId sceneId2(13u);
    const SceneId sceneId3(14u);
    const SceneId sceneId4(15u);
    createScene(sceneId1);
    createScene(sceneId2);
    createScene(sceneId3);
    createScene(sceneId4);
    mapSceneToDisplayBuffer(sceneId1, displayHandle, 0, fakeOffscreenBuffer1);
    mapSceneToDisplayBuffer(sceneId2, displayHandle, 1, fakeOffscreenBuffer2);
    mapSceneToDisplayBuffer(sceneId3, displayHandle, 2, fakeOffscreenBuffer1);
    mapSceneToDisplayBuffer(sceneId4, displayHandle, 3, fakeOffscreenBuffer2);
    showScene(sceneId1);
    showScene(sceneId2);
    showScene(sceneId3);
    showScene(sceneId4);

    expectOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer1, fakeOffscreenBuffer2 }, true);
    {
        InSequence s;
        expectSceneRendered(displayHandle, sceneId1, fakeOffscreenBuffer1);
        expectSceneRendered(displayHandle, sceneId3, fakeOffscreenBuffer1);
    }
    {
        InSequence s;
        expectSceneRendered(displayHandle, sceneId2, fakeOffscreenBuffer2);
        expectSceneRendered(displayHandle, sceneId4, fakeOffscreenBuffer2);
    }
    expectFrameBufferRendered(displayHandle, false);
    expectSwapBuffers();
    doOneRendererLoop();

    hideScene(sceneId1);
    hideScene(sceneId2);
    hideScene(sceneId3);
    hideScene(sceneId4);
    unmapScene(sceneId1);
    unmapScene(sceneId2);
    unmapScene(sceneId3);
    unmapScene(sceneId4);
}

TEST_P(ARenderer, confidence_assignsSceneToOffscreenBufferAndReassignsToFramebuffer)
{
    const DisplayHandle displayHandle = addDisplayController();

    const SceneId sceneId(12u);
    createScene(sceneId);

    const DeviceResourceHandle fakeOffscreenBuffer(313u);
    renderer.registerOffscreenBuffer(displayHandle, fakeOffscreenBuffer, 1u, 1u, false);
    mapSceneToDisplayBuffer(sceneId, displayHandle, 0, fakeOffscreenBuffer);
    EXPECT_EQ(fakeOffscreenBuffer, renderer.getBufferSceneIsMappedTo(sceneId));
    EXPECT_FALSE(renderer.isSceneMappedToInterruptibleOffscreenBuffer(sceneId));

    const DeviceResourceHandle framebuffer = DisplayControllerMock::FakeFrameBufferHandle;
    mapSceneToDisplayBuffer(sceneId, displayHandle, 0, framebuffer);
    EXPECT_EQ(framebuffer, renderer.getBufferSceneIsMappedTo(sceneId));
    EXPECT_FALSE(renderer.isSceneMappedToInterruptibleOffscreenBuffer(sceneId));

    unmapScene(sceneId);
}

TEST_P(ARenderer, returnsInvalidDisplayWhenQueryingLocationOfUnmappedScene)
{
    const DisplayHandle displayHandle = addDisplayController();

    const SceneId sceneId(12u);
    createScene(sceneId);
    mapSceneToDisplayBuffer(sceneId, displayHandle, 0);
    unmapScene(sceneId);
    EXPECT_FALSE(renderer.getDisplaySceneIsMappedTo(sceneId).isValid());
}

TEST_P(ARenderer, doesNotRenderAMappedScene)
{
    const DisplayHandle displayHandle = addDisplayController();

    const SceneId sceneId(12u);
    createScene(sceneId);
    mapSceneToDisplayBuffer(sceneId, displayHandle, 0);

    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();

    unmapScene(sceneId);
}

TEST_P(ARenderer, rendersMappedAndShownScene)
{
    const DisplayHandle displayHandle = addDisplayController();

    const SceneId sceneId(12u);
    createScene(sceneId);
    mapSceneToDisplayBuffer(sceneId, displayHandle, 0);
    showScene(sceneId);

    expectSceneRendered(displayHandle, sceneId);
    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();

    hideScene(sceneId);
    unmapScene(sceneId);
}

TEST_P(ARenderer, rendersTwoMappedAndShownScenes)
{
    const DisplayHandle displayHandle = addDisplayController();

    const SceneId sceneId1(12u);
    createScene(sceneId1);
    mapSceneToDisplayBuffer(sceneId1, displayHandle, 0);
    showScene(sceneId1);

    const SceneId sceneId2(13u);
    createScene(sceneId2);
    mapSceneToDisplayBuffer(sceneId2, displayHandle, 0);
    showScene(sceneId2);

    expectSceneRendered(displayHandle, sceneId1);
    expectSceneRendered(displayHandle, sceneId2);
    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();

    hideScene(sceneId1);
    hideScene(sceneId2);
    unmapScene(sceneId1);
    unmapScene(sceneId2);
}

TEST_P(ARenderer, rendersTwoMappedAndShownScenesWithAscendingRenderOrder)
{
    const DisplayHandle displayHandle = addDisplayController();

    const SceneId sceneId1(12u);
    createScene(sceneId1);
    mapSceneToDisplayBuffer(sceneId1, displayHandle, 1);
    showScene(sceneId1);

    const SceneId sceneId2(13u);
    createScene(sceneId2);
    mapSceneToDisplayBuffer(sceneId2, displayHandle, 2);
    showScene(sceneId2);

    {
        InSequence seq;
        expectSceneRendered(displayHandle, sceneId1);
        expectSceneRendered(displayHandle, sceneId2);
    }
    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();

    hideScene(sceneId1);
    hideScene(sceneId2);
    unmapScene(sceneId1);
    unmapScene(sceneId2);
}

TEST_P(ARenderer, rendersTwoMappedAndShownScenesWithDescendingRenderOrder)
{
    const DisplayHandle displayHandle = addDisplayController();

    const SceneId sceneId1(12u);
    createScene(sceneId1);
    mapSceneToDisplayBuffer(sceneId1, displayHandle, 2);
    showScene(sceneId1);


    const SceneId sceneId2(13u);
    createScene(sceneId2);
    mapSceneToDisplayBuffer(sceneId2, displayHandle, 1);
    showScene(sceneId2);

    {
        InSequence seq;
        expectSceneRendered(displayHandle, sceneId2);
        expectSceneRendered(displayHandle, sceneId1);
    }
    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();

    hideScene(sceneId1);
    hideScene(sceneId2);
    unmapScene(sceneId1);
    unmapScene(sceneId2);
}


TEST_P(ARenderer, rendersTwoMappedAndShownScenesEachADifferentDisplay)
{
    const DisplayHandle displayHandle1 = addDisplayController();
    const DisplayHandle displayHandle2 = addDisplayController();

    const SceneId sceneId1(12u);
    createScene(sceneId1);
    mapSceneToDisplayBuffer(sceneId1, displayHandle1, 0);
    showScene(sceneId1);

    const SceneId sceneId2(13u);
    createScene(sceneId2);
    mapSceneToDisplayBuffer(sceneId2, displayHandle2, 0);
    showScene(sceneId2);

    expectSceneRendered(displayHandle1, sceneId1);
    expectSceneRendered(displayHandle2, sceneId2);
    expectFrameBufferRendered(displayHandle1);
    expectFrameBufferRendered(displayHandle2);
    expectSwapBuffers(displayHandle2);
    expectSwapBuffers(displayHandle1, true);
    doOneRendererLoop();

    hideScene(sceneId1);
    hideScene(sceneId2);
    unmapScene(sceneId1);
    unmapScene(sceneId2);
}

TEST_P(ARenderer, doesNotRenderSceneThatWasNotMapped)
{
    addDisplayController();
    createScene();
    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();
}

TEST_P(ARenderer, doesNotRenderUnmappedScene)
{
    const DisplayHandle displayHandle = addDisplayController();

    const SceneId sceneId(12u);
    createScene(sceneId);
    mapSceneToDisplayBuffer(sceneId, displayHandle, 0);
    unmapScene(sceneId);

    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();
}

TEST_P(ARenderer, remappingAndReshowingSceneResultsInRenderingOnAnotherDisplayOnly)
{
    const DisplayHandle displayHandle1 = addDisplayController();
    const DisplayHandle displayHandle2 = addDisplayController();

    const SceneId sceneId(12u);
    createScene(sceneId);
    mapSceneToDisplayBuffer(sceneId, displayHandle1, 0);
    showScene(sceneId);

    expectSceneRendered(displayHandle1, sceneId);
    expectFrameBufferRendered(displayHandle1);
    expectFrameBufferRendered(displayHandle2);
    expectSwapBuffers(displayHandle2);
    expectSwapBuffers(displayHandle1, true);
    doOneRendererLoop();

    hideScene(sceneId);
    unmapScene(sceneId);
    mapSceneToDisplayBuffer(sceneId, displayHandle2, 0);
    showScene(sceneId);

    expectSceneRendered(displayHandle2, sceneId);
    expectFrameBufferRendered(displayHandle1);
    expectFrameBufferRendered(displayHandle2);
    expectSwapBuffers(displayHandle2);
    expectSwapBuffers(displayHandle1, true);
    doOneRendererLoop();

    hideScene(sceneId);
    unmapScene(sceneId);
}

TEST_P(ARenderer, confidenceTest_mapAndShowTwoScenesOnTwoDisplaysAndSwapTheirMapping)
{
    const DisplayHandle displayHandle1 = addDisplayController();
    const DisplayHandle displayHandle2 = addDisplayController();

    const SceneId sceneId1(12u);
    createScene(sceneId1);
    mapSceneToDisplayBuffer(sceneId1, displayHandle1, 0);
    showScene(sceneId1);

    const SceneId sceneId2(13u);
    createScene(sceneId2);
    mapSceneToDisplayBuffer(sceneId2, displayHandle2, 0);
    showScene(sceneId2);

    expectSceneRendered(displayHandle1, sceneId1);
    expectSceneRendered(displayHandle2, sceneId2);
    expectFrameBufferRendered(displayHandle1);
    expectFrameBufferRendered(displayHandle2);
    expectSwapBuffers(displayHandle2);
    expectSwapBuffers(displayHandle1, true);
    doOneRendererLoop();

    // exchange their mapping
    hideScene(sceneId1);
    hideScene(sceneId2);
    unmapScene(sceneId1);
    unmapScene(sceneId2);
    mapSceneToDisplayBuffer(sceneId1, displayHandle2, 0);
    mapSceneToDisplayBuffer(sceneId2, displayHandle1, 0);
    showScene(sceneId1);
    showScene(sceneId2);

    expectSceneRendered(displayHandle1, sceneId2);
    expectSceneRendered(displayHandle2, sceneId1);
    expectFrameBufferRendered(displayHandle1);
    expectFrameBufferRendered(displayHandle2);
    expectSwapBuffers(displayHandle2);
    expectSwapBuffers(displayHandle1, true);
    doOneRendererLoop();

    hideScene(sceneId1);
    hideScene(sceneId2);
    unmapScene(sceneId1);
    unmapScene(sceneId2);
}

TEST_P(ARenderer, skipsFrameIfDisplayControllerCanNotRenderNewFrame)
{
    const DisplayHandle displayHandle1 = addDisplayController();
    DisplayStrictMockInfo& displayMock = renderer.getDisplayMock(displayHandle1);

    EXPECT_CALL(*displayMock.m_displayController, handleWindowEvents());
    //mock that disp controller can not render new frame by returning false
    EXPECT_CALL(*displayMock.m_displayController, canRenderNewFrame()).WillRepeatedly(Return(false));

    //renderer must not try to render on that display
    if(GetParam())
        EXPECT_CALL(platformFactoryMock.windowEventsPollingManagerMock, pollWindowsTillAnyCanRender()).Times(1u);
    EXPECT_CALL(*displayMock.m_displayController, enableContext()).Times(0);
    EXPECT_CALL(*displayMock.m_displayController, swapBuffers()).Times(0);
    EXPECT_CALL(*displayMock.m_displayController, getEmbeddedCompositingManager()).Times(0);
    EXPECT_CALL(*displayMock.m_embeddedCompositingManager, notifyClients()).Times(0);

    renderer.doOneRenderLoop();
}

TEST_P(ARenderer, canTakeASingleScreenshot)
{
    const DisplayHandle displayHandle = addDisplayController();
    DisplayStrictMockInfo& displayMock = renderer.getDisplayMock(displayHandle);

    ScreenshotInfo screenshot;
    screenshot.rectangle = { 20u, 30u, 100u, 100u };
    screenshot.display = displayHandle;
    screenshot.filename = "";
    renderer.scheduleScreenshot(screenshot);
    EXPECT_CALL(*displayMock.m_displayController, readPixels(20u, 30u, 100u, 100u, _));
    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();

    ScreenshotInfoVector screenshots;
    renderer.dispatchProcessedScreenshots(screenshots);
    ASSERT_EQ(1u, screenshots.size());
    EXPECT_TRUE(screenshots[0].success);
    EXPECT_EQ(displayHandle, screenshots[0].display);
    EXPECT_FALSE(screenshots[0].pixelData.empty());
    EXPECT_EQ(80u * 70u * 4u, screenshots[0].pixelData.size());

    // check that screenshot request got deleted
    EXPECT_CALL(*displayMock.m_displayController, readPixels(_, _, _, _, _)).Times(0);
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();

    screenshots.clear();
    renderer.dispatchProcessedScreenshots(screenshots);
    EXPECT_EQ(0u, screenshots.size());
}

TEST_P(ARenderer, canTakeMultipleScreenshotsForMultipleDisplays)
{
    const DisplayHandle displayHandle1 = addDisplayController();
    DisplayStrictMockInfo& displayMock1 = renderer.getDisplayMock(displayHandle1);

    const DisplayHandle displayHandle2 = addDisplayController();
    DisplayStrictMockInfo& displayMock2 = renderer.getDisplayMock(displayHandle2);

    ScreenshotInfo screenshot;
    screenshot.filename = "";

    screenshot.rectangle = { 10u, 10u, 110u, 110u };
    screenshot.display = displayHandle1;
    renderer.scheduleScreenshot(screenshot);

    screenshot.rectangle = { 20u, 20u, 120u, 120u };
    screenshot.display = displayHandle2;
    renderer.scheduleScreenshot(screenshot);

    screenshot.rectangle = { 30u, 30u, 130u, 130u };
    screenshot.display = displayHandle1;
    renderer.scheduleScreenshot(screenshot);

    screenshot.rectangle = { 40u, 40u, 140u, 140u };
    screenshot.display = displayHandle2;
    renderer.scheduleScreenshot(screenshot);

    screenshot.rectangle = { 50u, 50u, 150u, 150u };
    screenshot.display = displayHandle1;
    renderer.scheduleScreenshot(screenshot);

    EXPECT_CALL(*displayMock1.m_displayController, readPixels(10u, 10u, 110u, 110u, _));
    EXPECT_CALL(*displayMock2.m_displayController, readPixels(20u, 20u, 120u, 120u, _));
    EXPECT_CALL(*displayMock1.m_displayController, readPixels(30u, 30u, 130u, 130u, _));
    EXPECT_CALL(*displayMock2.m_displayController, readPixels(40u, 40u, 140u, 140u, _));
    EXPECT_CALL(*displayMock1.m_displayController, readPixels(50u, 50u, 150u, 150u, _));
    expectFrameBufferRendered(displayHandle1);
    expectFrameBufferRendered(displayHandle2);
    expectSwapBuffers(displayHandle2);
    expectSwapBuffers(displayHandle1, true);
    doOneRendererLoop();

    ScreenshotInfoVector screenshots;
    renderer.dispatchProcessedScreenshots(screenshots);
    ASSERT_EQ(5u, screenshots.size());
    for (UInt32 i =0; i < screenshots.size(); i++)
    {
        EXPECT_TRUE(screenshots[i].success);
        EXPECT_TRUE(screenshots[i].display == displayHandle1 || screenshots[i].display == displayHandle2);
        EXPECT_FALSE(screenshots[i].pixelData.empty());
    }

    // check that screenshot request got deleted
    EXPECT_CALL(*displayMock1.m_displayController, readPixels(_, _, _, _, _)).Times(0);
    EXPECT_CALL(*displayMock2.m_displayController, readPixels(_, _, _, _, _)).Times(0);
    expectFrameBufferRendered(displayHandle1, false, false);
    expectFrameBufferRendered(displayHandle2, false, false);
    doOneRendererLoop();

    screenshots.clear();
    renderer.dispatchProcessedScreenshots(screenshots);
    EXPECT_EQ(0u, screenshots.size());
}

TEST_P(ARenderer, failsToCreateScreenshot)
{
    const DisplayHandle displayHandle = addDisplayController();
    DisplayStrictMockInfo& displayMock = renderer.getDisplayMock(displayHandle);

    ScreenshotInfo screenshot;
    screenshot.rectangle = { 0u, 0u, 100u, 100u };
    screenshot.display = displayHandle;
    screenshot.filename = "";

    renderer.scheduleScreenshot(screenshot);
    EXPECT_CALL(*displayMock.m_displayController, readPixels(0u, 0u, 100u, 100u, _)).WillOnce(Return(false));
    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();

    ScreenshotInfoVector screenshots;
    renderer.dispatchProcessedScreenshots(screenshots);
    EXPECT_EQ(1u, screenshots.size());
    EXPECT_FALSE(screenshots[0].success);
    EXPECT_EQ(displayHandle, screenshots[0].display);
    EXPECT_TRUE(screenshots[0].pixelData.empty());

    // check that screenshot request got deleted
    EXPECT_CALL(*displayMock.m_displayController, readPixels(_, _, _, _, _)).Times(0);
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();

    screenshots.clear();
    renderer.dispatchProcessedScreenshots(screenshots);
    EXPECT_EQ(0u, screenshots.size());
}

TEST_P(ARenderer, willIgnoreScreenshotIfDisplayIsDestroyedAtTheSameTime)
{
    const DisplayHandle displayHandle = addDisplayController();

    ScreenshotInfo screenshot;
    screenshot.rectangle = { 0u, 0u, 100u, 100u };
    screenshot.display = displayHandle;
    screenshot.filename = "";

    renderer.scheduleScreenshot(screenshot);
    destroyDisplayController(displayHandle);
    doOneRendererLoop();

    ScreenshotInfoVector screenshots;
    renderer.dispatchProcessedScreenshots(screenshots);
    EXPECT_EQ(0u, screenshots.size());

    // ensure screenshot command is not cached
    const DisplayHandle displayHandle2 = addDisplayController();
    EXPECT_EQ(displayHandle, displayHandle2);
    DisplayStrictMockInfo& displayMock2 = renderer.getDisplayMock(displayHandle2);
    EXPECT_CALL(*displayMock2.m_displayController, readPixels(_, _, _, _, _)).Times(0);
    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();

    screenshots.clear();
    renderer.dispatchProcessedScreenshots(screenshots);
    EXPECT_EQ(0u, screenshots.size());
}

TEST_P(ARenderer, marksRenderOncePassesAsRenderedAfterRenderingScene)
{
    const DisplayHandle displayHandle = addDisplayController();

    const SceneId sceneId(12u);
    createScene(sceneId);
    mapSceneToDisplayBuffer(sceneId, displayHandle, 0);
    showScene(sceneId);

    auto& scene = rendererScenes.getScene(sceneId);
    TestSceneHelper sceneHelper(scene);
    const auto dataLayout = sceneHelper.m_sceneAllocator.allocateDataLayout({ {EDataType_Vector2I}, {EDataType_Vector2I} });
    const CameraHandle camera = sceneHelper.m_sceneAllocator.allocateCamera(ECameraProjectionType_Orthographic, sceneHelper.m_sceneAllocator.allocateNode(), sceneHelper.m_sceneAllocator.allocateDataInstance(dataLayout));
    const RenderPassHandle pass = sceneHelper.m_sceneAllocator.allocateRenderPass();
    scene.setRenderPassCamera(pass, camera);

    // render
    scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
    expectSceneRendered(displayHandle, sceneId);
    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();

    // pass in list means it was rendered
    const auto& passesToRender = scene.getSortedRenderingPasses();
    ASSERT_EQ(1u, passesToRender.size());
    EXPECT_EQ(pass, passesToRender[0].getRenderPassHandle());

    // set as render once pass
    // now this is expected to be rendered once only
    scene.setRenderPassRenderOnce(pass, true);

    // render
    scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
    expectSceneRendered(displayHandle, sceneId);
    renderer.markBufferWithMappedSceneAsModified(sceneId);
    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();

    // pass in list means it was rendered
    ASSERT_EQ(1u, passesToRender.size());
    EXPECT_EQ(pass, passesToRender[0].getRenderPassHandle());

    // render
    scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
    expectSceneRendered(displayHandle, sceneId);
    renderer.markBufferWithMappedSceneAsModified(sceneId);
    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();

    // pass not in list means it was not rendered anymore
    EXPECT_TRUE(passesToRender.empty());

    hideScene(sceneId);
    unmapScene(sceneId);
}

TEST_P(ARenderer, doesNotClearAndRerenderIfNoChangeToScene)
{
    const DisplayHandle displayHandle = addDisplayController();

    const SceneId sceneId(12u);
    createScene(sceneId);
    mapSceneToDisplayBuffer(sceneId, displayHandle, 0);
    showScene(sceneId);

    expectSceneRendered(displayHandle, sceneId);
    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();

    // no further expectations for scene render or clear
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();

    hideScene(sceneId);
    unmapScene(sceneId);
}

TEST_P(ARenderer, doesNotSkipClearAndRerenderIfNoChangeToSceneButSkippingUnmodifiedBuffersDisabled)
{
    const DisplayHandle displayHandle = addDisplayController();

    const SceneId sceneId(12u);
    createScene(sceneId);
    mapSceneToDisplayBuffer(sceneId, displayHandle, 0);
    showScene(sceneId);

    renderer.setSkippingOfUnmodifiedBuffers(false);

    expectSceneRendered(displayHandle, sceneId);
    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();

    expectSceneRendered(displayHandle, sceneId);
    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();

    renderer.setSkippingOfUnmodifiedBuffers(true);

    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();

    hideScene(sceneId);
    unmapScene(sceneId);
}

TEST_P(ARenderer, doesNotClearAndRerenderOffscreenBufferIfNoChangeToScene)
{
    const DisplayHandle displayHandle = addDisplayController();

    const SceneId sceneId(12u);
    createScene(sceneId);

    const DeviceResourceHandle fakeOffscreenBuffer(313u);
    renderer.registerOffscreenBuffer(displayHandle, fakeOffscreenBuffer, 1u, 2u, false);
    mapSceneToDisplayBuffer(sceneId, displayHandle, 0, fakeOffscreenBuffer);
    showScene(sceneId);

    expectOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, true);
    expectSceneRendered(displayHandle, sceneId, fakeOffscreenBuffer);
    expectFrameBufferRendered(displayHandle, false);
    expectSwapBuffers();
    doOneRendererLoop();

    // no further expectations for scene render or clear
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();

    hideScene(sceneId);
    unmapScene(sceneId);
}

TEST_P(ARenderer, doesNotSkipClearAndRerenderOffscreenBufferIfNoChangeToSceneButSkippingUnmodifiedBuffersDisabled)
{
    const DisplayHandle displayHandle = addDisplayController();

    const SceneId sceneId(12u);
    createScene(sceneId);

    const DeviceResourceHandle fakeOffscreenBuffer(313u);
    renderer.registerOffscreenBuffer(displayHandle, fakeOffscreenBuffer, 1u, 2u, false);
    mapSceneToDisplayBuffer(sceneId, displayHandle, 0, fakeOffscreenBuffer);
    showScene(sceneId);

    renderer.setSkippingOfUnmodifiedBuffers(false);

    expectOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, true);
    expectSceneRendered(displayHandle, sceneId, fakeOffscreenBuffer);
    expectFrameBufferRendered(displayHandle, false);
    expectSwapBuffers();
    doOneRendererLoop();

    expectOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, true);
    expectSceneRendered(displayHandle, sceneId, fakeOffscreenBuffer);
    expectFrameBufferRendered(displayHandle, false);
    expectSwapBuffers();
    doOneRendererLoop();

    renderer.setSkippingOfUnmodifiedBuffers(true);

    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();

    hideScene(sceneId);
    unmapScene(sceneId);
}

TEST_P(ARenderer, clearAndRerenderIfSceneMarkedAsChanged)
{
    const DisplayHandle displayHandle = addDisplayController();

    const SceneId sceneId(12u);
    createScene(sceneId);
    mapSceneToDisplayBuffer(sceneId, displayHandle, 0);
    showScene(sceneId);

    expectSceneRendered(displayHandle, sceneId);
    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();

    // no change
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();
    // mark change
    renderer.markBufferWithMappedSceneAsModified(sceneId);
    expectSceneRendered(displayHandle, sceneId);
    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();
    // no change
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();
    // mark change
    renderer.markBufferWithMappedSceneAsModified(sceneId);
    expectSceneRendered(displayHandle, sceneId);
    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();

    hideScene(sceneId);
    unmapScene(sceneId);
}

TEST_P(ARenderer, clearAndRerenderOffscreenBufferIfSceneMarkedAsChanged)
{
    const DisplayHandle displayHandle = addDisplayController();

    const SceneId sceneId(12u);
    createScene(sceneId);

    const DeviceResourceHandle fakeOffscreenBuffer(313u);
    renderer.registerOffscreenBuffer(displayHandle, fakeOffscreenBuffer, 1u, 2u, false);
    mapSceneToDisplayBuffer(sceneId, displayHandle, 0, fakeOffscreenBuffer);
    showScene(sceneId);

    expectOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, true);
    expectSceneRendered(displayHandle, sceneId, fakeOffscreenBuffer);
    expectFrameBufferRendered(displayHandle, false);
    expectSwapBuffers();
    doOneRendererLoop();

    // no change
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();
    // mark change
    renderer.markBufferWithMappedSceneAsModified(sceneId);
    expectOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, true);
    expectSceneRendered(displayHandle, sceneId, fakeOffscreenBuffer);
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop(); // framebuffer not cleared/swapped as there is no scene assigned
    // no change
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();
    // mark change
    renderer.markBufferWithMappedSceneAsModified(sceneId);
    expectOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, true);
    expectSceneRendered(displayHandle, sceneId, fakeOffscreenBuffer);
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop(); // framebuffer not cleared/swapped as there is no scene assigned

    hideScene(sceneId);
    unmapScene(sceneId);
}

TEST_P(ARenderer, clearAndRerenderBothFramebufferAndOffscreenBufferIfSceneAssignedFromOneToTheOther)
{
    const DisplayHandle displayHandle = addDisplayController();

    const SceneId sceneId(12u);
    createScene(sceneId);

    const DeviceResourceHandle fakeOffscreenBuffer(313u);
    renderer.registerOffscreenBuffer(displayHandle, fakeOffscreenBuffer, 1u, 2u, false);
    mapSceneToDisplayBuffer(sceneId, displayHandle, 0, fakeOffscreenBuffer);
    showScene(sceneId);

    expectOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, true);
    expectSceneRendered(displayHandle, sceneId, fakeOffscreenBuffer);
    expectFrameBufferRendered(displayHandle, false);
    expectSwapBuffers();
    doOneRendererLoop();

    // no change
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();

    // assign back to FB causes clear/render of both buffers
    renderer.mapSceneToDisplayBuffer(sceneId, displayHandle, DisplayControllerMock::FakeFrameBufferHandle, 0);
    expectOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, true);
    expectFrameBufferRendered(displayHandle, false, true);
    expectSceneRendered(displayHandle, sceneId, DisplayControllerMock::FakeFrameBufferHandle);
    expectSwapBuffers();
    doOneRendererLoop();

    // no change
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();

    // assign back to offscreen buffer causes clear/render of both buffers
    renderer.mapSceneToDisplayBuffer(sceneId, displayHandle, fakeOffscreenBuffer, 0);
    expectOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, true);
    expectSceneRendered(displayHandle, sceneId, fakeOffscreenBuffer);
    expectFrameBufferRendered(displayHandle, false, true); // framebuffer cleared/swapped
    expectSwapBuffers();
    doOneRendererLoop();

    hideScene(sceneId);
    unmapScene(sceneId);
}

TEST_P(ARenderer, clearAndRerenderBufferIfSceneHidden)
{
    const DisplayHandle displayHandle = addDisplayController();

    const SceneId sceneId(12u);
    createScene(sceneId);
    mapSceneToDisplayBuffer(sceneId, displayHandle, 0);
    showScene(sceneId);

    expectSceneRendered(displayHandle, sceneId);
    expectFrameBufferRendered(displayHandle, true);
    expectSwapBuffers();
    doOneRendererLoop();

    // no change
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();

    // hiding scene causes clear of FB but no render as scene is hidden
    hideScene(sceneId);
    expectFrameBufferRendered(displayHandle, true);
    expectSwapBuffers();
    doOneRendererLoop();

    // no change
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();

    unmapScene(sceneId);
}

TEST_P(ARenderer, clearAndRerenderBufferIfClearColorChanged)
{
    const DisplayHandle displayHandle = addDisplayController();

    const SceneId sceneId(12u);
    createScene(sceneId);
    mapSceneToDisplayBuffer(sceneId, displayHandle, 0);
    showScene(sceneId);

    expectSceneRendered(displayHandle, sceneId);
    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();

    // no change
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();

    // setting clear color causes clear/render
    renderer.setClearColor(displayHandle, { 1, 2, 3, 4 });
    expectSceneRendered(displayHandle, sceneId);
    expectFrameBufferRendered(displayHandle, true, true, { 1, 2, 3, 4 });
    expectSwapBuffers();
    doOneRendererLoop();

    // no change
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();

    hideScene(sceneId);
    unmapScene(sceneId);
}

TEST_P(ARenderer, clearAndSwapInterruptibleOBOnlyOnceIfNoMoreShownScenes)
{
    const DisplayHandle displayHandle = addDisplayController();

    const DeviceResourceHandle fakeOffscreenBuffer(313u);
    renderer.registerOffscreenBuffer(displayHandle, fakeOffscreenBuffer, 1u, 2u, true);

    const SceneId sceneId(12u);
    createScene(sceneId);
    mapSceneToDisplayBuffer(sceneId, displayHandle, 0, fakeOffscreenBuffer);
    showScene(sceneId);

    expectFrameBufferRendered();
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, { { true, true } });
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneId, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderBegin);
    expectSwapBuffers();
    doOneRendererLoop();

    // re-render FB to reflect finished interruptible OB in previous frame
    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();

    // hide scene will trigger re-render of OB
    hideScene(sceneId);
    expectFrameBufferRendered(displayHandle, false, false);
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, { { true, true } }, true);
    doOneRendererLoop();

    // re-render FB to reflect finished interruptible OB in previous frame
    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();

    // no change
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();

    unmapScene(sceneId);
}

TEST_P(ARenderer, clearAndSwapInterruptibleOBOnlyOnceIfNoMoreMappedScenes)
{
    const DisplayHandle displayHandle = addDisplayController();

    const DeviceResourceHandle fakeOffscreenBuffer(313u);
    renderer.registerOffscreenBuffer(displayHandle, fakeOffscreenBuffer, 1u, 2u, true);

    const SceneId sceneId(12u);
    createScene(sceneId);
    mapSceneToDisplayBuffer(sceneId, displayHandle, 0, fakeOffscreenBuffer);
    showScene(sceneId);

    expectFrameBufferRendered();
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, { { true, true } });
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneId, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderBegin);
    expectSwapBuffers();
    doOneRendererLoop();

    // re-render FB to reflect finished interruptible OB in previous frame
    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();

    // hide scene will trigger re-render of OB
    hideScene(sceneId);
    unmapScene(sceneId);
    expectFrameBufferRendered(displayHandle, false, false);
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, { { true, true } }, true);
    doOneRendererLoop();

    // re-render FB to reflect finished interruptible OB in previous frame
    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();

    // no change
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();
}

TEST_P(ARenderer, rerendersFramebufferIfWarpingDataChanged)
{
    const DisplayHandle displayHandle = addDisplayController();
    EXPECT_CALL(*renderer.getDisplayMock(displayHandle).m_displayController, isWarpingEnabled()).WillRepeatedly(Return(true));

    expectFrameBufferRendered(displayHandle, true);
    expectSwapBuffers();
    doOneRendererLoop();

    // no change
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();

    // changing warping data causes re-render
    EXPECT_CALL(*renderer.getDisplayMock(displayHandle).m_displayController, setWarpingMeshData(_));
    renderer.setWarpingMeshData(displayHandle, {});
    expectFrameBufferRendered(displayHandle, true);
    expectSwapBuffers();
    doOneRendererLoop();

    // no change
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();
}

TEST_P(ARenderer, rendersScenesToFBAndOBWithNoInterruption)
{
    const DisplayHandle displayHandle = addDisplayController();
    const DeviceResourceHandle fakeOffscreenBuffer(313u);
    renderer.registerOffscreenBuffer(displayHandle, fakeOffscreenBuffer, 1u, 1u, true);

    const SceneId sceneIdFB(12u);
    const SceneId sceneIdOB(13u);
    createScene(sceneIdFB);
    createScene(sceneIdOB);
    mapSceneToDisplayBuffer(sceneIdFB, displayHandle, 0);
    mapSceneToDisplayBuffer(sceneIdOB, displayHandle, 0, fakeOffscreenBuffer);

    showScene(sceneIdFB);
    showScene(sceneIdOB);

    expectFrameBufferRendered();
    expectSceneRendered(displayHandle, sceneIdFB);
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer });
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneIdOB, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderBegin);
    expectSwapBuffers();
    doOneRendererLoop();

    hideScene(sceneIdOB);
    hideScene(sceneIdFB);
    unmapScene(sceneIdOB);
    unmapScene(sceneIdFB);
}

TEST_P(ARenderer, doesNotSwapOffscreenBufferIfRenderingIntoItInterrupted)
{
    const DisplayHandle displayHandle = addDisplayController();
    const DeviceResourceHandle fakeOffscreenBuffer(313u);
    renderer.registerOffscreenBuffer(displayHandle, fakeOffscreenBuffer, 1u, 1u, true);

    const SceneId sceneIdOB(13u);
    createScene(sceneIdOB);
    mapSceneToDisplayBuffer(sceneIdOB, displayHandle, 0, fakeOffscreenBuffer);
    showScene(sceneIdOB);

    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneIdOB, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderInterrupted);

    expectFrameBufferRendered();
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, { { true, false } }); // expect clear but not swap due to interruption
    expectSwapBuffers();
    doOneRendererLoop();

    renderer.resetRenderInterruptState();
    hideScene(sceneIdOB);
    unmapScene(sceneIdOB);
}

TEST_P(ARenderer, doesNotClearAndPassesPreviousStateWhenInterruptedAndSwapsAfterFinishing)
{
    const DisplayHandle displayHandle = addDisplayController();
    const DeviceResourceHandle fakeOffscreenBuffer(313u);
    renderer.registerOffscreenBuffer(displayHandle, fakeOffscreenBuffer, 1u, 1u, true);

    const SceneId sceneIdOB(13u);
    createScene(sceneIdOB);
    mapSceneToDisplayBuffer(sceneIdOB, displayHandle, 0, fakeOffscreenBuffer);
    showScene(sceneIdOB);

    // start rendering and interrupt
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneIdOB, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderInterrupted);
    expectFrameBufferRendered(displayHandle);
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, { { true, false } }); // expect clear but not swap due to interruption
    expectSwapBuffers();
    doOneRendererLoop();
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    // continue from interruption point and interrupt again
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneIdOB, fakeOffscreenBuffer, sceneRenderInterrupted, sceneRenderInterrupted2);
    expectFrameBufferRendered(displayHandle, false, false);
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, { { false, false } }, true); // no clear and no swap due to interruption before and now again
    doOneRendererLoop();
    EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    // continue from interruption point and finish
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneIdOB, fakeOffscreenBuffer, sceneRenderInterrupted2, sceneRenderBegin);
    expectFrameBufferRendered(displayHandle, false, false);
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, { { false, true } }, true); // no clear but swap after finish
    doOneRendererLoop();
    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    // re-render FB to reflect change happened to OB in previous frame
    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();
    EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    hideScene(sceneIdOB);
    unmapScene(sceneIdOB);
}

TEST_P(ARenderer, rendersScenesToFBEvenIfOBInterrupted)
{
    const DisplayHandle displayHandle = addDisplayController();
    const DeviceResourceHandle fakeOffscreenBuffer(313u);
    renderer.registerOffscreenBuffer(displayHandle, fakeOffscreenBuffer, 1u, 1u, true);

    const SceneId sceneIdFB(12u);
    const SceneId sceneIdOB(13u);
    createScene(sceneIdFB);
    createScene(sceneIdOB);
    mapSceneToDisplayBuffer(sceneIdFB, displayHandle, 0);
    mapSceneToDisplayBuffer(sceneIdOB, displayHandle, 0, fakeOffscreenBuffer);

    showScene(sceneIdFB);
    showScene(sceneIdOB);

    expectSceneRendered(displayHandle, sceneIdFB);
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneIdOB, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderInterrupted);
    expectFrameBufferRendered();
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, { { true, false } });
    expectSwapBuffers();
    doOneRendererLoop();

    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneIdOB, fakeOffscreenBuffer, sceneRenderInterrupted, sceneRenderBegin);
    expectFrameBufferRendered(displayHandle, false ,false);
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, { { false, true } }, true);
    doOneRendererLoop();

    // re-render FB to reflect change happened to OB in previous frame
    expectSceneRendered(displayHandle, sceneIdFB);
    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();

    hideScene(sceneIdOB);
    hideScene(sceneIdFB);
    unmapScene(sceneIdOB);
    unmapScene(sceneIdFB);
}

TEST_P(ARenderer, alwaysRendersScenesToFBWhenModifiedEvenIfOBInterrupted)
{
    const DisplayHandle displayHandle = addDisplayController();
    const DeviceResourceHandle fakeOffscreenBuffer(313u);
    renderer.registerOffscreenBuffer(displayHandle, fakeOffscreenBuffer, 1u, 1u, true);

    const SceneId sceneIdFB(12u);
    const SceneId sceneIdOB(13u);
    createScene(sceneIdFB);
    createScene(sceneIdOB);
    mapSceneToDisplayBuffer(sceneIdFB, displayHandle, 0);
    mapSceneToDisplayBuffer(sceneIdOB, displayHandle, 0, fakeOffscreenBuffer);

    showScene(sceneIdFB);
    showScene(sceneIdOB);

    //FB rendered, OB interrupted
    expectSceneRendered(displayHandle, sceneIdFB);
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneIdOB, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderInterrupted);
    expectFrameBufferRendered();
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, { { true, false } });
    expectSwapBuffers();
    doOneRendererLoop();

    //modify FB scene
    renderer.markBufferWithMappedSceneAsModified(sceneIdFB);
    //FB rendered, OB interrupted
    expectSceneRendered(displayHandle, sceneIdFB);
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneIdOB, fakeOffscreenBuffer, sceneRenderInterrupted, sceneRenderInterrupted2);
    expectFrameBufferRendered(displayHandle);
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, { { false, false } });
    expectSwapBuffers();
    doOneRendererLoop();

    //modify FB scene
    renderer.markBufferWithMappedSceneAsModified(sceneIdFB);
    //FB rendered, OB finished
    expectSceneRendered(displayHandle, sceneIdFB);
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneIdOB, fakeOffscreenBuffer, sceneRenderInterrupted2, sceneRenderBegin);
    expectFrameBufferRendered(displayHandle);
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, { { false, true } });
    expectSwapBuffers();
    doOneRendererLoop();

    // re-render FB to reflect change happened to OB in previous frame
    expectSceneRendered(displayHandle, sceneIdFB);
    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();

    hideScene(sceneIdOB);
    hideScene(sceneIdFB);
    unmapScene(sceneIdOB);
    unmapScene(sceneIdFB);
}

TEST_P(ARenderer, doesNotSkipFramesTillAllInterruptionsFinishedAndRendered)
{
    // 1 FB, 1 OB, 1 scene per OB

    const DisplayHandle displayHandle = addDisplayController();
    const DeviceResourceHandle fakeOffscreenBuffer(313u);
    renderer.registerOffscreenBuffer(displayHandle, fakeOffscreenBuffer, 1u, 1u, true);

    const SceneId sceneIdFB(12u);
    const SceneId sceneIdOB(13u);
    createScene(sceneIdFB);
    createScene(sceneIdOB);
    mapSceneToDisplayBuffer(sceneIdFB, displayHandle, 0);
    mapSceneToDisplayBuffer(sceneIdOB, displayHandle, 0, fakeOffscreenBuffer);

    showScene(sceneIdFB);
    showScene(sceneIdOB);

    // FB rendered, OB interrupted
    expectSceneRendered(displayHandle, sceneIdFB);
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneIdOB, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderInterrupted);
    expectFrameBufferRendered();
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, { { true, false } });
    expectSwapBuffers();
    doOneRendererLoop();

    // FB skipped, OB finished
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneIdOB, fakeOffscreenBuffer, sceneRenderInterrupted, sceneRenderBegin);
    expectFrameBufferRendered(displayHandle, false ,false);
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, { { false, true } }, true);
    doOneRendererLoop();

    // re-render FB to reflect change happened to OB in previous frame
    expectSceneRendered(displayHandle, sceneIdFB);
    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();

    // no change -> nothing rendered
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();

    hideScene(sceneIdOB);
    hideScene(sceneIdFB);
    unmapScene(sceneIdOB);
    unmapScene(sceneIdFB);
}

TEST_P(ARenderer, interruptingEveryFrameGetsToAllRenderedState_withMultipleScenes)
{
    // 1 FB, 1 OB, 2 scenes per OB

    const DisplayHandle displayHandle = addDisplayController();
    const DeviceResourceHandle fakeOffscreenBuffer(313u);
    renderer.registerOffscreenBuffer(displayHandle, fakeOffscreenBuffer, 1u, 1u, true);

    const SceneId sceneIdFB(12u);
    const SceneId sceneId1OB(13u);
    const SceneId sceneId2OB(14u);
    createScene(sceneIdFB);
    createScene(sceneId1OB);
    createScene(sceneId2OB);
    mapSceneToDisplayBuffer(sceneId2OB, displayHandle, 1, fakeOffscreenBuffer);
    mapSceneToDisplayBuffer(sceneId1OB, displayHandle, 0, fakeOffscreenBuffer);
    mapSceneToDisplayBuffer(sceneIdFB, displayHandle, 0);

    showScene(sceneIdFB);
    showScene(sceneId1OB);
    showScene(sceneId2OB);

    // FB rendered, OB scene1 interrupted, OB scene2 skipped
    expectSceneRendered(displayHandle, sceneIdFB);
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneId1OB, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderInterrupted);
    expectFrameBufferRendered();
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, { { true, false } });
    expectSwapBuffers();
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    // FB skipped, OB scene1 finished, OB scene2 interrupted
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneId1OB, fakeOffscreenBuffer, sceneRenderInterrupted, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneId2OB, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderInterrupted);
    expectFrameBufferRendered(displayHandle, false, false);
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, { { false, false } }, true);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    // FB skipped (as OB is not done renderirng yet), OB scene1 skipped, OB scene2 finished
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneId2OB, fakeOffscreenBuffer, sceneRenderInterrupted, sceneRenderBegin);
    expectFrameBufferRendered(displayHandle, false ,false);
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, { { false, true } }, true);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    // re-render FB to reflect change happened to OB scene 1 and scene 2 in previous frames
    expectSceneRendered(displayHandle, sceneIdFB);
    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    // no change -> nothing rendered
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    hideScene(sceneId2OB);
    hideScene(sceneId1OB);
    hideScene(sceneIdFB);
    unmapScene(sceneId2OB);
    unmapScene(sceneId1OB);
    unmapScene(sceneIdFB);
}

TEST_P(ARenderer, interruptingEveryFrameGetsToAllRenderedState_withMultipleScenes_FirstSceneNeverInterrupted)
{
    // 1 FB, 1 OB, 2 scenes per OB

    const DisplayHandle displayHandle = addDisplayController();
    const DeviceResourceHandle fakeOffscreenBuffer(313u);
    renderer.registerOffscreenBuffer(displayHandle, fakeOffscreenBuffer, 1u, 1u, true);

    const SceneId sceneIdFB(12u);
    const SceneId sceneId1OB(13u);
    const SceneId sceneId2OB(14u);
    createScene(sceneIdFB);
    createScene(sceneId1OB);
    createScene(sceneId2OB);
    mapSceneToDisplayBuffer(sceneId2OB, displayHandle, 1, fakeOffscreenBuffer);
    mapSceneToDisplayBuffer(sceneId1OB, displayHandle, 0, fakeOffscreenBuffer);
    mapSceneToDisplayBuffer(sceneIdFB, displayHandle, 0);

    showScene(sceneIdFB);
    showScene(sceneId1OB);
    showScene(sceneId2OB);

    // FB rendered, OB scene1 fully rendered, OB scene2 interrupted
    expectSceneRendered(displayHandle, sceneIdFB);
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneId1OB, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneId2OB, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderInterrupted);
    expectFrameBufferRendered();
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, { { true, false } });
    expectSwapBuffers();
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    // FB skipped, OB scene1 skipped, OB scene2 finished
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneId2OB, fakeOffscreenBuffer, sceneRenderInterrupted, sceneRenderBegin);
    expectFrameBufferRendered(displayHandle, false ,false);
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, { { false, true } }, true);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    // re-render FB to reflect change happened to OB scene 1 and scene 2 in previous frames
    expectSceneRendered(displayHandle, sceneIdFB);
    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    // no change -> nothing rendered
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    hideScene(sceneId2OB);
    hideScene(sceneId1OB);
    hideScene(sceneIdFB);
    unmapScene(sceneId2OB);
    unmapScene(sceneId1OB);
    unmapScene(sceneIdFB);
}

TEST_P(ARenderer, interruptingEveryFrameGetsToAllRenderedState_withMultipleOffscreenBuffers)
{
    // 1 FB, 2 OB, 1 scene per OB

    const DisplayHandle displayHandle = addDisplayController();
    const DeviceResourceHandle fakeOffscreenBuffer1(313u);
    const DeviceResourceHandle fakeOffscreenBuffer2(314u);
    renderer.registerOffscreenBuffer(displayHandle, fakeOffscreenBuffer1, 1u, 1u, true);
    renderer.registerOffscreenBuffer(displayHandle, fakeOffscreenBuffer2, 1u, 1u, true);

    const SceneId sceneIdFB(12u);
    const SceneId sceneIdOB1(13u);
    const SceneId sceneIdOB2(14u);
    createScene(sceneIdFB);
    createScene(sceneIdOB1);
    createScene(sceneIdOB2);
    mapSceneToDisplayBuffer(sceneIdOB2, displayHandle, 0, fakeOffscreenBuffer2);
    mapSceneToDisplayBuffer(sceneIdOB1, displayHandle, 0, fakeOffscreenBuffer1);
    mapSceneToDisplayBuffer(sceneIdFB, displayHandle, 0);

    showScene(sceneIdFB);
    showScene(sceneIdOB1);
    showScene(sceneIdOB2);

    // FB rendered, OB1 interrupted, OB2 skipped
    expectSceneRendered(displayHandle, sceneIdFB);
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneIdOB1, fakeOffscreenBuffer1, sceneRenderBegin, sceneRenderInterrupted);
    expectFrameBufferRendered();
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer1 }, { { true, false } });
    expectSwapBuffers();
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    // FB skipped, OB1 finished, OB2 interrupted
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneIdOB1, fakeOffscreenBuffer1, sceneRenderInterrupted, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneIdOB2, fakeOffscreenBuffer2, sceneRenderBegin, sceneRenderInterrupted);
    expectFrameBufferRendered(displayHandle, false, false);
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer1, fakeOffscreenBuffer2 }, { { false, true }, { true, false } }, true);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    // FB re-rendered to reflect changed in OB1, OB1 skipped, OB2 finished
    expectSceneRendered(displayHandle, sceneIdFB);
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneIdOB2, fakeOffscreenBuffer2, sceneRenderInterrupted, sceneRenderBegin);
    expectFrameBufferRendered();
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer2 }, { { false, true } });
    expectSwapBuffers();
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    // re-render FB to reflect change happened to OB2 in previous frame
    expectSceneRendered(displayHandle, sceneIdFB);
    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    // no change -> nothing rendered
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    hideScene(sceneIdOB2);
    hideScene(sceneIdOB1);
    hideScene(sceneIdFB);
    unmapScene(sceneIdOB2);
    unmapScene(sceneIdOB1);
    unmapScene(sceneIdFB);
}

TEST_P(ARenderer, interruptingEveryFrameGetsToAllRenderedState_withMultipleOffscreenBuffersAndScenes)
{
    // 1 FB, 2 OB, 2 scene per OB

    const DisplayHandle displayHandle = addDisplayController();
    const DeviceResourceHandle fakeOffscreenBuffer1(313u);
    const DeviceResourceHandle fakeOffscreenBuffer2(314u);
    renderer.registerOffscreenBuffer(displayHandle, fakeOffscreenBuffer1, 1u, 1u, true);
    renderer.registerOffscreenBuffer(displayHandle, fakeOffscreenBuffer2, 1u, 1u, true);

    const SceneId sceneIdFB(12u);
    const SceneId sceneId1OB1(13u);
    const SceneId sceneId2OB1(14u);
    const SceneId sceneId1OB2(15u);
    const SceneId sceneId2OB2(16u);
    createScene(sceneIdFB);
    createScene(sceneId1OB1);
    createScene(sceneId2OB1);
    createScene(sceneId1OB2);
    createScene(sceneId2OB2);
    mapSceneToDisplayBuffer(sceneId1OB2, displayHandle, 0, fakeOffscreenBuffer2);
    mapSceneToDisplayBuffer(sceneId2OB2, displayHandle, 1, fakeOffscreenBuffer2);
    mapSceneToDisplayBuffer(sceneId1OB1, displayHandle, 0, fakeOffscreenBuffer1);
    mapSceneToDisplayBuffer(sceneId2OB1, displayHandle, 1, fakeOffscreenBuffer1);
    mapSceneToDisplayBuffer(sceneIdFB, displayHandle, 0);

    showScene(sceneIdFB);
    showScene(sceneId1OB1);
    showScene(sceneId2OB1);
    showScene(sceneId1OB2);
    showScene(sceneId2OB2);

    // FB rendered, OB1 scene1 interrupted, rest skipped
    expectSceneRendered(displayHandle, sceneIdFB);
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneId1OB1, fakeOffscreenBuffer1, sceneRenderBegin, sceneRenderInterrupted);
    expectFrameBufferRendered();
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer1 }, { { true, false } });
    expectSwapBuffers();
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    // FB skipped, OB1 scene1 finished, OB1 scene2 interrupted, rest skipped
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneId1OB1, fakeOffscreenBuffer1, sceneRenderInterrupted, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneId2OB1, fakeOffscreenBuffer1, sceneRenderBegin, sceneRenderInterrupted);
    expectFrameBufferRendered(displayHandle, false, false);
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer1 }, { { false, false } }, true);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    // FB skipped, OB1 scene1 skipped, OB1 scene2 finished, OB2 scene1 interrupted, rest skipped
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneId2OB1, fakeOffscreenBuffer1, sceneRenderInterrupted, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneId1OB2, fakeOffscreenBuffer2, sceneRenderBegin, sceneRenderInterrupted);
    expectFrameBufferRendered(displayHandle, false, false);
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer1, fakeOffscreenBuffer2 }, { { false, true },{ true, false } }, true);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    // FB re-rendered to reflect changes in OB1, OB1 skipped, OB2 scene1 finished, OB2 scene2 interrupted
    expectSceneRendered(displayHandle, sceneIdFB);
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneId1OB2, fakeOffscreenBuffer2, sceneRenderInterrupted, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneId2OB2, fakeOffscreenBuffer2, sceneRenderBegin, sceneRenderInterrupted);
    expectFrameBufferRendered();
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer2 }, { { false, false } });
    expectSwapBuffers();
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    // FB skipped, OB1 skipped, OB2 scene2 finished
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneId2OB2, fakeOffscreenBuffer2, sceneRenderInterrupted, sceneRenderBegin);
    expectFrameBufferRendered(displayHandle, false, false);
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer2 }, { { false, true } }, true);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    // FB re-rendered to reflect changes in OB2, rest skipped
    expectSceneRendered(displayHandle, sceneIdFB);
    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();

    // no change -> nothing rendered
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    hideScene(sceneId2OB2);
    hideScene(sceneId1OB2);
    hideScene(sceneId2OB1);
    hideScene(sceneId1OB1);
    hideScene(sceneIdFB);
    unmapScene(sceneId2OB2);
    unmapScene(sceneId1OB2);
    unmapScene(sceneId2OB1);
    unmapScene(sceneId1OB1);
    unmapScene(sceneIdFB);
}

TEST_P(ARenderer, interruptingEveryFrameGetsToAllRenderedState_withMultipleDisplays)
{
    // 2 FB, 1 OB per display, 1 scene per OB

    const DisplayHandle displayHandle1 = addDisplayController();
    const DisplayHandle displayHandle2 = addDisplayController();
    DeviceResourceHandle disp1OB(313u);
    DeviceResourceHandle disp2OB(314u);
    renderer.registerOffscreenBuffer(displayHandle1, disp1OB, 1u, 1u, true);
    renderer.registerOffscreenBuffer(displayHandle2, disp2OB, 1u, 1u, true);

    const SceneId sceneIdDisp1FB(12u);
    const SceneId sceneIdDisp1OB(13u);
    const SceneId sceneIdDisp2FB(14u);
    const SceneId sceneIdDisp2OB(15u);
    createScene(sceneIdDisp1FB);
    createScene(sceneIdDisp1OB);
    createScene(sceneIdDisp2OB);
    createScene(sceneIdDisp2FB);
    mapSceneToDisplayBuffer(sceneIdDisp1FB, displayHandle1, 0);
    mapSceneToDisplayBuffer(sceneIdDisp1OB, displayHandle1, 0, disp1OB);
    mapSceneToDisplayBuffer(sceneIdDisp2FB, displayHandle2, 0);
    mapSceneToDisplayBuffer(sceneIdDisp2OB, displayHandle2, 0, disp2OB);

    showScene(sceneIdDisp1FB);
    showScene(sceneIdDisp1OB);
    showScene(sceneIdDisp2OB);
    showScene(sceneIdDisp2FB);

    // FB scenes rendered, FB1 OB scene interrupted, rest skipped
    expectSceneRendered(displayHandle1, sceneIdDisp1FB);
    expectSceneRendered(displayHandle2, sceneIdDisp2FB);
    expectSceneRenderedWithInterruptionEnabled(displayHandle1, sceneIdDisp1OB, disp1OB, sceneRenderBegin, sceneRenderInterrupted);
    expectFrameBufferRendered(displayHandle1, true);
    expectFrameBufferRendered(displayHandle2, true);
    expectInterruptibleOffscreenBufferRendered(displayHandle1, { disp1OB }, { { true, false } }, true);
    expectSwapBuffers(displayHandle1);
    expectSwapBuffers(displayHandle2, true);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle1).m_displayController);
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle2).m_displayController);

    // FB scenes skipped, FB1 OB scene finished, FB2 OB scene interrupted
    expectSceneRenderedWithInterruptionEnabled(displayHandle1, sceneIdDisp1OB, disp1OB, sceneRenderInterrupted, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle2, sceneIdDisp2OB, disp2OB, sceneRenderBegin, sceneRenderInterrupted);
    expectFrameBufferRendered(displayHandle1, false, false);
    expectFrameBufferRendered(displayHandle2, false, false);
    expectInterruptibleOffscreenBufferRendered(displayHandle1, { disp1OB }, { { false, true } }, true);
    expectInterruptibleOffscreenBufferRendered(displayHandle2, { disp2OB }, { { true, false } }, true);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle1).m_displayController);
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle2).m_displayController);

    // FB1 scene rendered to reflect change in OB from previous frame, FB2 scene skipped, FB1 OB scene skipped, FB2 OB scene finished
    expectSceneRendered(displayHandle1, sceneIdDisp1FB);
    expectSceneRenderedWithInterruptionEnabled(displayHandle2, sceneIdDisp2OB, disp2OB, sceneRenderInterrupted, sceneRenderBegin);
    expectFrameBufferRendered(displayHandle1, true);
    expectFrameBufferRendered(displayHandle2, false, false);
    expectInterruptibleOffscreenBufferRendered(displayHandle2, { disp2OB }, { { false, true } }, true);
    expectSwapBuffers(displayHandle1, true);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle1).m_displayController);
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle2).m_displayController);

    // FB1 scene skipped, FB2 scene re-rendered, FB1 OB scene skipped, FB2 OB scene skipped
    expectSceneRendered(displayHandle2, sceneIdDisp2FB);
    expectFrameBufferRendered(displayHandle1, false, false);
    expectFrameBufferRendered(displayHandle2, true);
    expectSwapBuffers(displayHandle2, false);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle1).m_displayController);
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle2).m_displayController);

    // no change -> nothing rendered
    expectFrameBufferRendered(displayHandle1, false, false);
    expectFrameBufferRendered(displayHandle2, false, false);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle1).m_displayController);
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle2).m_displayController);

    hideScene(sceneIdDisp1FB);
    hideScene(sceneIdDisp1OB);
    hideScene(sceneIdDisp2OB);
    hideScene(sceneIdDisp2FB);

    unmapScene(sceneIdDisp1FB);
    unmapScene(sceneIdDisp1OB);
    unmapScene(sceneIdDisp2OB);
    unmapScene(sceneIdDisp2FB);
}

TEST_P(ARenderer, interruptingEveryFrameGetsToAllRenderedState_withMultipleDisplaysMultipleOBMulitpleScenes)
{
    // 2 FB, 2 OB per display, 2 scene per OB

    const DisplayHandle displayHandle1 = addDisplayController();
    const DisplayHandle displayHandle2 = addDisplayController();
    DeviceResourceHandle disp1OB1(313u);
    DeviceResourceHandle disp1OB2(314u);
    DeviceResourceHandle disp2OB1(315u);
    DeviceResourceHandle disp2OB2(316u);
    renderer.registerOffscreenBuffer(displayHandle1, disp1OB1, 1u, 1u, true);
    renderer.registerOffscreenBuffer(displayHandle1, disp1OB2, 1u, 1u, true);
    renderer.registerOffscreenBuffer(displayHandle2, disp2OB1, 1u, 1u, true);
    renderer.registerOffscreenBuffer(displayHandle2, disp2OB2, 1u, 1u, true);

    const SceneId sceneIdDisp1FB(12u);
    const SceneId sceneIdDisp2FB(13u);
    const SceneId sceneIdDisp1OB1scene1(14u);
    const SceneId sceneIdDisp1OB1scene2(15u);
    const SceneId sceneIdDisp1OB2scene1(16u);
    const SceneId sceneIdDisp1OB2scene2(17u);
    const SceneId sceneIdDisp2OB1scene1(18u);
    const SceneId sceneIdDisp2OB1scene2(19u);
    const SceneId sceneIdDisp2OB2scene1(20u);
    const SceneId sceneIdDisp2OB2scene2(21u);

    createScene(sceneIdDisp1FB);
    createScene(sceneIdDisp2FB);
    createScene(sceneIdDisp1OB1scene1);
    createScene(sceneIdDisp1OB1scene2);
    createScene(sceneIdDisp1OB2scene1);
    createScene(sceneIdDisp1OB2scene2);
    createScene(sceneIdDisp2OB1scene1);
    createScene(sceneIdDisp2OB1scene2);
    createScene(sceneIdDisp2OB2scene1);
    createScene(sceneIdDisp2OB2scene2);

    mapSceneToDisplayBuffer(sceneIdDisp1FB, displayHandle1, 0);
    mapSceneToDisplayBuffer(sceneIdDisp2FB, displayHandle2, 0);
    mapSceneToDisplayBuffer(sceneIdDisp1OB2scene1, displayHandle1, 0, disp1OB2);
    mapSceneToDisplayBuffer(sceneIdDisp1OB2scene2, displayHandle1, 1, disp1OB2);
    mapSceneToDisplayBuffer(sceneIdDisp1OB1scene1, displayHandle1, 0, disp1OB1);
    mapSceneToDisplayBuffer(sceneIdDisp1OB1scene2, displayHandle1, 1, disp1OB1);
    mapSceneToDisplayBuffer(sceneIdDisp2OB2scene1, displayHandle2, 0, disp2OB2);
    mapSceneToDisplayBuffer(sceneIdDisp2OB2scene2, displayHandle2, 1, disp2OB2);
    mapSceneToDisplayBuffer(sceneIdDisp2OB1scene1, displayHandle2, 0, disp2OB1);
    mapSceneToDisplayBuffer(sceneIdDisp2OB1scene2, displayHandle2, 1, disp2OB1);

    showScene(sceneIdDisp1FB);
    showScene(sceneIdDisp2FB);
    showScene(sceneIdDisp1OB1scene1);
    showScene(sceneIdDisp1OB1scene2);
    showScene(sceneIdDisp1OB2scene1);
    showScene(sceneIdDisp1OB2scene2);
    showScene(sceneIdDisp2OB1scene1);
    showScene(sceneIdDisp2OB1scene2);
    showScene(sceneIdDisp2OB2scene1);
    showScene(sceneIdDisp2OB2scene2);

    // FB scenes rendered on both displays, FB1 OB1 scene1 interrupted, rest skipped
    expectSceneRendered(displayHandle1, sceneIdDisp1FB);
    expectSceneRendered(displayHandle2, sceneIdDisp2FB);
    expectSceneRenderedWithInterruptionEnabled(displayHandle1, sceneIdDisp1OB1scene1, disp1OB1, sceneRenderBegin, sceneRenderInterrupted);
    expectFrameBufferRendered(displayHandle1);
    expectFrameBufferRendered(displayHandle2);
    expectInterruptibleOffscreenBufferRendered(displayHandle1, { disp1OB1 }, { { true, false } }, true);
    expectSwapBuffers(displayHandle1);
    expectSwapBuffers(displayHandle2, true);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle1).m_displayController);
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle2).m_displayController);

    // FB scenes skipped on both displays, FB1 OB1 scene1 finished, FB1 OB1 scene2 interrupted, rest skipped
    expectSceneRenderedWithInterruptionEnabled(displayHandle1, sceneIdDisp1OB1scene1, disp1OB1, sceneRenderInterrupted, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle1, sceneIdDisp1OB1scene2, disp1OB1, sceneRenderBegin, sceneRenderInterrupted);
    expectFrameBufferRendered(displayHandle1, false, false);
    expectFrameBufferRendered(displayHandle2, false, false);
    expectInterruptibleOffscreenBufferRendered(displayHandle1, { disp1OB1 }, { { false, false } }, true);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle1).m_displayController);
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle2).m_displayController);

    // FB scenes skipped on both displays, FB1 OB1 scene2 finished, FB1 OB2 scene1 interrupted, rest skipped
    expectSceneRenderedWithInterruptionEnabled(displayHandle1, sceneIdDisp1OB1scene2, disp1OB1, sceneRenderInterrupted, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle1, sceneIdDisp1OB2scene1, disp1OB2, sceneRenderBegin, sceneRenderInterrupted);
    expectFrameBufferRendered(displayHandle1, false, false);
    expectFrameBufferRendered(displayHandle2, false, false);
    expectInterruptibleOffscreenBufferRendered(displayHandle1, { disp1OB1, disp1OB2 }, { { false, true },{ true, false } }, true);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle1).m_displayController);
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle2).m_displayController);

    // FB1 scene re-rendered, FB2 scene skipped, FB1 OB2 scene1 finished, FB1 OB2 scene2 interrupted, rest skipped
    expectSceneRendered(displayHandle1, sceneIdDisp1FB);
    expectSceneRenderedWithInterruptionEnabled(displayHandle1, sceneIdDisp1OB2scene1, disp1OB2, sceneRenderInterrupted, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle1, sceneIdDisp1OB2scene2, disp1OB2, sceneRenderBegin, sceneRenderInterrupted);
    expectFrameBufferRendered(displayHandle1);
    expectFrameBufferRendered(displayHandle2, false, false);
    expectInterruptibleOffscreenBufferRendered(displayHandle1, { disp1OB2 }, { { false, false } });
    expectSwapBuffers(displayHandle1);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle1).m_displayController);
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle2).m_displayController);

    // FB scenes skipped on both displays, FB1 OB2 scene2 finished, FB2 OB1 scene1 interrupted, rest skipped
    expectSceneRenderedWithInterruptionEnabled(displayHandle1, sceneIdDisp1OB2scene2, disp1OB2, sceneRenderInterrupted, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle2, sceneIdDisp2OB1scene1, disp2OB1, sceneRenderBegin, sceneRenderInterrupted);
    expectFrameBufferRendered(displayHandle1, false, false);
    expectFrameBufferRendered(displayHandle2, false, false);
    expectInterruptibleOffscreenBufferRendered(displayHandle1, { disp1OB2 }, { { false, true } }, true);
    expectInterruptibleOffscreenBufferRendered(displayHandle2, { disp2OB1 }, { { true, false } }, true);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle1).m_displayController);
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle2).m_displayController);

    // FB1 scene re-rendered, FB2 scenes skipped, FB2 OB1 scene1 finished, FB2 OB1 scene2 interrupted, rest skipped
    expectSceneRendered(displayHandle1, sceneIdDisp1FB);
    expectSceneRenderedWithInterruptionEnabled(displayHandle2, sceneIdDisp2OB1scene1, disp2OB1, sceneRenderInterrupted, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle2, sceneIdDisp2OB1scene2, disp2OB1, sceneRenderBegin, sceneRenderInterrupted);
    expectFrameBufferRendered(displayHandle1);
    expectFrameBufferRendered(displayHandle2, false, false);
    expectInterruptibleOffscreenBufferRendered(displayHandle2, { disp2OB1 }, { { false, false } }, true);
    expectSwapBuffers(displayHandle1, true);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle1).m_displayController);
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle2).m_displayController);

    // FB scenes skipped on both displays, FB2 OB1 scene2 finished, FB2 OB2 scene1 interrupted, rest skipped
    expectSceneRenderedWithInterruptionEnabled(displayHandle2, sceneIdDisp2OB1scene2, disp2OB1, sceneRenderInterrupted, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle2, sceneIdDisp2OB2scene1, disp2OB2, sceneRenderBegin, sceneRenderInterrupted);
    expectFrameBufferRendered(displayHandle1, false, false);
    expectFrameBufferRendered(displayHandle2, false, false);
    expectInterruptibleOffscreenBufferRendered(displayHandle2, { disp2OB1, disp2OB2 }, { { false, true },{ true, false } }, true);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle1).m_displayController);
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle2).m_displayController);

    // FB1 scene skipped, FB2 scene re-rendered, FB2 OB2 scene1 finished, FB2 OB2 scene2 interrupted, rest skipped
    expectSceneRendered(displayHandle2, sceneIdDisp2FB);
    expectSceneRenderedWithInterruptionEnabled(displayHandle2, sceneIdDisp2OB2scene1, disp2OB2, sceneRenderInterrupted, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle2, sceneIdDisp2OB2scene2, disp2OB2, sceneRenderBegin, sceneRenderInterrupted);
    expectFrameBufferRendered(displayHandle1, false, false);
    expectFrameBufferRendered(displayHandle2);
    expectInterruptibleOffscreenBufferRendered(displayHandle2, { disp2OB2 }, { { false, false } });
    expectSwapBuffers(displayHandle2);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle1).m_displayController);
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle2).m_displayController);

    // FB scenes skipped on both displays, FB2 OB2 scene2 finished, rest skipped
    expectSceneRenderedWithInterruptionEnabled(displayHandle2, sceneIdDisp2OB2scene2, disp2OB2, sceneRenderInterrupted, sceneRenderBegin);
    expectFrameBufferRendered(displayHandle1, false, false);
    expectFrameBufferRendered(displayHandle2, false, false);
    expectInterruptibleOffscreenBufferRendered(displayHandle2, { disp2OB2 }, { { false, true } }, true);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle1).m_displayController);
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle2).m_displayController);

    // FB2 scene re-rendered (to reflect changed on disp2OB2)
    expectSceneRendered(displayHandle2, sceneIdDisp2FB);
    expectFrameBufferRendered(displayHandle1, false, false);
    expectFrameBufferRendered(displayHandle2);
    expectSwapBuffers(displayHandle2);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle1).m_displayController);
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle2).m_displayController);

    // no change -> nothing rendered
    expectFrameBufferRendered(displayHandle1, false, false);
    expectFrameBufferRendered(displayHandle2, false, false);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle1).m_displayController);
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle2).m_displayController);

    hideScene(sceneIdDisp1FB);
    hideScene(sceneIdDisp2FB);
    hideScene(sceneIdDisp1OB1scene1);
    hideScene(sceneIdDisp1OB1scene2);
    hideScene(sceneIdDisp1OB2scene1);
    hideScene(sceneIdDisp1OB2scene2);
    hideScene(sceneIdDisp2OB1scene1);
    hideScene(sceneIdDisp2OB1scene2);
    hideScene(sceneIdDisp2OB2scene1);
    hideScene(sceneIdDisp2OB2scene2);

    unmapScene(sceneIdDisp1FB);
    unmapScene(sceneIdDisp2FB);
    unmapScene(sceneIdDisp1OB1scene1);
    unmapScene(sceneIdDisp1OB1scene2);
    unmapScene(sceneIdDisp1OB2scene1);
    unmapScene(sceneIdDisp1OB2scene2);
    unmapScene(sceneIdDisp2OB1scene1);
    unmapScene(sceneIdDisp2OB1scene2);
    unmapScene(sceneIdDisp2OB2scene1);
    unmapScene(sceneIdDisp2OB2scene2);
}

TEST_P(ARenderer, interruptingEveryFrameGetsToAllRenderedState_withMultipleDisplaysWithNormalOBAndInterruptibleOB)
{
    // 2 FB, 2 OB (interruptible+normal) per display, 1 scene per OB

    const DisplayHandle displayHandle1 = addDisplayController();
    const DisplayHandle displayHandle2 = addDisplayController();
    DeviceResourceHandle disp1OBint(313u);
    DeviceResourceHandle disp1OBnon(314u);
    DeviceResourceHandle disp2OBnon(315u);
    DeviceResourceHandle disp2OBint(316u);
    renderer.registerOffscreenBuffer(displayHandle1, disp1OBint, 1u, 1u, true);
    renderer.registerOffscreenBuffer(displayHandle1, disp1OBnon, 1u, 1u, false);
    renderer.registerOffscreenBuffer(displayHandle2, disp2OBnon, 1u, 1u, false);
    renderer.registerOffscreenBuffer(displayHandle2, disp2OBint, 1u, 1u, true);

    const SceneId sceneIdDisp1FB(12u);
    const SceneId sceneIdDisp2FB(13u);
    const SceneId sceneIdDisp1OBintScene(14u);
    const SceneId sceneIdDisp1OBnonScene(16u);
    const SceneId sceneIdDisp2OBnonScene(18u);
    const SceneId sceneIdDisp2OBintScene(20u);

    createScene(sceneIdDisp1FB);
    createScene(sceneIdDisp2FB);
    createScene(sceneIdDisp1OBintScene);
    createScene(sceneIdDisp1OBnonScene);
    createScene(sceneIdDisp2OBnonScene);
    createScene(sceneIdDisp2OBintScene);

    mapSceneToDisplayBuffer(sceneIdDisp1FB, displayHandle1, 0);
    mapSceneToDisplayBuffer(sceneIdDisp2FB, displayHandle2, 0);
    mapSceneToDisplayBuffer(sceneIdDisp1OBnonScene, displayHandle1, 0, disp1OBnon);
    mapSceneToDisplayBuffer(sceneIdDisp1OBintScene, displayHandle1, 0, disp1OBint);
    mapSceneToDisplayBuffer(sceneIdDisp2OBintScene, displayHandle2, 0, disp2OBint);
    mapSceneToDisplayBuffer(sceneIdDisp2OBnonScene, displayHandle2, 0, disp2OBnon);

    showScene(sceneIdDisp1FB);
    showScene(sceneIdDisp2FB);
    showScene(sceneIdDisp1OBintScene);
    showScene(sceneIdDisp1OBnonScene);
    showScene(sceneIdDisp2OBintScene);
    showScene(sceneIdDisp2OBnonScene);

    // FB and non-interruptible OB scenes rendered on both displays, FB1 OB scene interrupted, rest skipped
    expectOffscreenBufferRendered(displayHandle1, { disp1OBnon }, true);
    expectSceneRendered(displayHandle1, sceneIdDisp1OBnonScene, disp1OBnon);
    expectFrameBufferRendered(displayHandle1, false);
    expectSceneRendered(displayHandle1, sceneIdDisp1FB);

    expectOffscreenBufferRendered(displayHandle2, { disp2OBnon }, true);
    expectSceneRendered(displayHandle2, sceneIdDisp2OBnonScene, disp2OBnon);
    expectFrameBufferRendered(displayHandle2, false);
    expectSceneRendered(displayHandle2, sceneIdDisp2FB);

    expectInterruptibleOffscreenBufferRendered(displayHandle1, { disp1OBint }, { { true, false } }, true);
    expectSceneRenderedWithInterruptionEnabled(displayHandle1, sceneIdDisp1OBintScene, disp1OBint, sceneRenderBegin, sceneRenderInterrupted);

    expectSwapBuffers(displayHandle1);
    expectSwapBuffers(displayHandle2, true);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle1).m_displayController);
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle2).m_displayController);

    //FB1 OB scene finished, FB2 OB scene interrupted, rest skipped
    expectSceneRenderedWithInterruptionEnabled(displayHandle1, sceneIdDisp1OBintScene, disp1OBint, sceneRenderInterrupted, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle2, sceneIdDisp2OBintScene, disp2OBint, sceneRenderBegin, sceneRenderInterrupted);
    expectFrameBufferRendered(displayHandle1, false, false);
    expectFrameBufferRendered(displayHandle2, false, false);
    expectInterruptibleOffscreenBufferRendered(displayHandle1, { disp1OBint }, { { false, true } }, true);
    expectInterruptibleOffscreenBufferRendered(displayHandle2, { disp2OBint }, { { true, false } }, true);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle1).m_displayController);
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle2).m_displayController);

    // FB1 re-rendered, FB2 OB scene finished, rest skipped
    expectSceneRendered(displayHandle1, sceneIdDisp1FB);
    expectSceneRenderedWithInterruptionEnabled(displayHandle2, sceneIdDisp2OBintScene, disp2OBint, sceneRenderInterrupted, sceneRenderBegin);
    expectFrameBufferRendered(displayHandle1);
    expectFrameBufferRendered(displayHandle2, false, false);
    expectInterruptibleOffscreenBufferRendered(displayHandle2, { disp2OBint }, { { false, true } }, true);
    expectSwapBuffers(displayHandle1, true);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle1).m_displayController);
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle2).m_displayController);

    // FB2 re-rendered, rest skipped
    expectSceneRendered(displayHandle2, sceneIdDisp2FB);
    expectFrameBufferRendered(displayHandle1, false, false);
    expectFrameBufferRendered(displayHandle2);
    expectSwapBuffers(displayHandle2, false);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle1).m_displayController);
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle2).m_displayController);

    // no change -> nothing rendered
    expectFrameBufferRendered(displayHandle1, false, false);
    expectFrameBufferRendered(displayHandle2, false, false);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle1).m_displayController);
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle2).m_displayController);

    hideScene(sceneIdDisp1FB);
    hideScene(sceneIdDisp2FB);
    hideScene(sceneIdDisp1OBintScene);
    hideScene(sceneIdDisp1OBnonScene);
    hideScene(sceneIdDisp2OBnonScene);
    hideScene(sceneIdDisp2OBintScene);

    unmapScene(sceneIdDisp1FB);
    unmapScene(sceneIdDisp2FB);
    unmapScene(sceneIdDisp1OBintScene);
    unmapScene(sceneIdDisp1OBnonScene);
    unmapScene(sceneIdDisp2OBnonScene);
    unmapScene(sceneIdDisp2OBintScene);
}

TEST_P(ARenderer, willRerenderSceneThatWasRenderedAndModifiedWhileOtherSceneInterrupted)
{
    const DisplayHandle displayHandle = addDisplayController();
    const DeviceResourceHandle fakeOffscreenBuffer(313u);
    renderer.registerOffscreenBuffer(displayHandle, fakeOffscreenBuffer, 1u, 1u, true);

    const SceneId sceneIdFB(12u);
    const SceneId sceneId1OB(13u);
    const SceneId sceneId2OB(14u);
    createScene(sceneIdFB);
    createScene(sceneId1OB);
    createScene(sceneId2OB);
    mapSceneToDisplayBuffer(sceneId2OB, displayHandle, 1, fakeOffscreenBuffer);
    mapSceneToDisplayBuffer(sceneId1OB, displayHandle, 0, fakeOffscreenBuffer);
    mapSceneToDisplayBuffer(sceneIdFB, displayHandle, 0);

    showScene(sceneIdFB);
    showScene(sceneId1OB);
    showScene(sceneId2OB);

    // FB rendered, OB scene1 fully rendered, OB scene2 interrupted
    expectSceneRendered(displayHandle, sceneIdFB);
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneId1OB, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneId2OB, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderInterrupted);
    expectFrameBufferRendered();
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, { { true, false } });
    expectSwapBuffers();
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    // modify OB scene1
    renderer.markBufferWithMappedSceneAsModified(sceneId1OB);

    // FB skipped, OB scene1 skipped, OB scene2 finished
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneId2OB, fakeOffscreenBuffer, sceneRenderInterrupted, sceneRenderBegin);
    expectFrameBufferRendered(displayHandle, false, false);
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, { { false, true } }, true);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    // re-render FB to reflect changes
    // re-render OB scene1 (and OB scene2 as they share buffer) also as it was modified while interrupted
    expectSceneRendered(displayHandle, sceneIdFB);
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneId1OB, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneId2OB, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderBegin);
    expectFrameBufferRendered();
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, { { true, true } });
    expectSwapBuffers();
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    // re-render FB one more time to reflect changes
    expectSceneRendered(displayHandle, sceneIdFB);
    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    // no change -> nothing rendered
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    hideScene(sceneId2OB);
    hideScene(sceneId1OB);
    hideScene(sceneIdFB);
    unmapScene(sceneId2OB);
    unmapScene(sceneId1OB);
    unmapScene(sceneIdFB);
}

TEST_P(ARenderer, willRerenderSceneThatWasRenderedAndModifiedWhileOtherSceneOnAnotherInterruptibleOBInterrupted)
{
    const DisplayHandle displayHandle = addDisplayController();
    const DeviceResourceHandle fakeOffscreenBuffer1(313u);
    const DeviceResourceHandle fakeOffscreenBuffer2(314u);
    renderer.registerOffscreenBuffer(displayHandle, fakeOffscreenBuffer1, 1u, 1u, true);
    renderer.registerOffscreenBuffer(displayHandle, fakeOffscreenBuffer2, 1u, 1u, true);

    const SceneId sceneIdFB(12u);
    const SceneId sceneId1OB(13u);
    const SceneId sceneId2OB(14u);
    createScene(sceneIdFB);
    createScene(sceneId1OB);
    createScene(sceneId2OB);
    mapSceneToDisplayBuffer(sceneId2OB, displayHandle, 0, fakeOffscreenBuffer2);
    mapSceneToDisplayBuffer(sceneId1OB, displayHandle, 1, fakeOffscreenBuffer1);
    mapSceneToDisplayBuffer(sceneIdFB, displayHandle, 0);

    showScene(sceneIdFB);
    showScene(sceneId1OB);
    showScene(sceneId2OB);

    // FB rendered, OB1 scene fully rendered, OB2 scene interrupted
    expectSceneRendered(displayHandle, sceneIdFB);
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneId1OB, fakeOffscreenBuffer1, sceneRenderBegin, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneId2OB, fakeOffscreenBuffer2, sceneRenderBegin, sceneRenderInterrupted);
    expectFrameBufferRendered();
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer1, fakeOffscreenBuffer2 }, { { true, true }, { true, false } });
    expectSwapBuffers();
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    // modify OB1 scene
    renderer.markBufferWithMappedSceneAsModified(sceneId1OB);

    // FB re-rendered to reflect finished OB1, OB1 scene skipped due to interruption, OB2 scene finished
    expectSceneRendered(displayHandle, sceneIdFB);
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneId2OB, fakeOffscreenBuffer2, sceneRenderInterrupted, sceneRenderBegin);
    expectFrameBufferRendered();
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer2 }, { { false, true } });
    expectSwapBuffers();
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    // FB re-rendered to reflect finished OB2, OB1 scene re-rendered due to modification, OB2 scene skipped
    expectSceneRendered(displayHandle, sceneIdFB);
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneId1OB, fakeOffscreenBuffer1, sceneRenderBegin, sceneRenderBegin);
    expectFrameBufferRendered();
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer1 }, { { true, true } });
    expectSwapBuffers();
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    // FB re-rendered once more to reflect changes from OB1
    expectSceneRendered(displayHandle, sceneIdFB);
    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    // no change -> nothing rendered
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle).m_displayController);

    hideScene(sceneId2OB);
    hideScene(sceneId1OB);
    hideScene(sceneIdFB);
    unmapScene(sceneId2OB);
    unmapScene(sceneId1OB);
    unmapScene(sceneIdFB);
}

TEST_P(ARenderer, willRenderAllScenesFromAllDisplaysAndBuffersInOneFrameIfWithinBudget)
{
    // 2 displays, 1 OB per display, 2 interruptible OBs per display, 2 scenes per OB

    const DisplayHandle displayHandle1 = addDisplayController();
    const DisplayHandle displayHandle2 = addDisplayController();
    DeviceResourceHandle disp1OB(313u);
    DeviceResourceHandle disp2OB(314u);
    DeviceResourceHandle disp1OBint1(315u);
    DeviceResourceHandle disp1OBint2(316u);
    DeviceResourceHandle disp2OBint1(317u);
    DeviceResourceHandle disp2OBint2(318u);
    renderer.registerOffscreenBuffer(displayHandle1, disp1OB, 1u, 1u, false);
    renderer.registerOffscreenBuffer(displayHandle2, disp2OB, 1u, 1u, false);
    renderer.registerOffscreenBuffer(displayHandle1, disp1OBint1, 1u, 1u, true);
    renderer.registerOffscreenBuffer(displayHandle1, disp1OBint2, 1u, 1u, true);
    renderer.registerOffscreenBuffer(displayHandle2, disp2OBint1, 1u, 1u, true);
    renderer.registerOffscreenBuffer(displayHandle2, disp2OBint2, 1u, 1u, true);

    const SceneId sceneIdDisp1FB(12u);
    const SceneId sceneIdDisp2FB(13u);
    const SceneId sceneIdDisp1OBscene(14u);
    const SceneId sceneIdDisp2OBscene(15u);
    const SceneId sceneIdDisp1OBint1scene1(16u);
    const SceneId sceneIdDisp1OBint1scene2(17u);
    const SceneId sceneIdDisp1OBint2scene1(18u);
    const SceneId sceneIdDisp1OBint2scene2(19u);
    const SceneId sceneIdDisp2OBint1scene1(20u);
    const SceneId sceneIdDisp2OBint1scene2(21u);
    const SceneId sceneIdDisp2OBint2scene1(22u);
    const SceneId sceneIdDisp2OBint2scene2(23u);

    createScene(sceneIdDisp1FB);
    createScene(sceneIdDisp2FB);
    createScene(sceneIdDisp1OBscene);
    createScene(sceneIdDisp2OBscene);
    createScene(sceneIdDisp1OBint1scene1);
    createScene(sceneIdDisp1OBint1scene2);
    createScene(sceneIdDisp1OBint2scene1);
    createScene(sceneIdDisp1OBint2scene2);
    createScene(sceneIdDisp2OBint1scene1);
    createScene(sceneIdDisp2OBint1scene2);
    createScene(sceneIdDisp2OBint2scene1);
    createScene(sceneIdDisp2OBint2scene2);

    mapSceneToDisplayBuffer(sceneIdDisp1FB, displayHandle1, 0);
    mapSceneToDisplayBuffer(sceneIdDisp2FB, displayHandle2, 0);
    mapSceneToDisplayBuffer(sceneIdDisp1OBscene, displayHandle1, 0, disp1OB);
    mapSceneToDisplayBuffer(sceneIdDisp2OBscene, displayHandle2, 0, disp2OB);
    mapSceneToDisplayBuffer(sceneIdDisp1OBint2scene1, displayHandle1, 0, disp1OBint2);
    mapSceneToDisplayBuffer(sceneIdDisp1OBint2scene2, displayHandle1, 1, disp1OBint2);
    mapSceneToDisplayBuffer(sceneIdDisp1OBint1scene1, displayHandle1, 0, disp1OBint1);
    mapSceneToDisplayBuffer(sceneIdDisp1OBint1scene2, displayHandle1, 1, disp1OBint1);
    mapSceneToDisplayBuffer(sceneIdDisp2OBint2scene1, displayHandle2, 0, disp2OBint2);
    mapSceneToDisplayBuffer(sceneIdDisp2OBint2scene2, displayHandle2, 1, disp2OBint2);
    mapSceneToDisplayBuffer(sceneIdDisp2OBint1scene1, displayHandle2, 0, disp2OBint1);
    mapSceneToDisplayBuffer(sceneIdDisp2OBint1scene2, displayHandle2, 1, disp2OBint1);

    showScene(sceneIdDisp1FB);
    showScene(sceneIdDisp2FB);
    showScene(sceneIdDisp1OBscene);
    showScene(sceneIdDisp2OBscene);
    showScene(sceneIdDisp1OBint1scene1);
    showScene(sceneIdDisp1OBint1scene2);
    showScene(sceneIdDisp1OBint2scene1);
    showScene(sceneIdDisp1OBint2scene2);
    showScene(sceneIdDisp2OBint1scene1);
    showScene(sceneIdDisp2OBint1scene2);
    showScene(sceneIdDisp2OBint2scene1);
    showScene(sceneIdDisp2OBint2scene2);

    // all rendered
    expectOffscreenBufferRendered(displayHandle1, { disp1OB }, true);
    expectSceneRendered(displayHandle1, sceneIdDisp1OBscene, disp1OB);
    expectFrameBufferRendered(displayHandle1, false);
    expectSceneRendered(displayHandle1, sceneIdDisp1FB);

    expectOffscreenBufferRendered(displayHandle2, { disp2OB }, true);
    expectSceneRendered(displayHandle2, sceneIdDisp2OBscene, disp2OB);
    expectFrameBufferRendered(displayHandle2, false);
    expectSceneRendered(displayHandle2, sceneIdDisp2FB);

    expectInterruptibleOffscreenBufferRendered(displayHandle1, { disp1OBint1, disp1OBint2 }, { { true, true },{ true, true } }, true);
    expectSceneRenderedWithInterruptionEnabled(displayHandle1, sceneIdDisp1OBint1scene1, disp1OBint1, sceneRenderBegin, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle1, sceneIdDisp1OBint1scene2, disp1OBint1, sceneRenderBegin, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle1, sceneIdDisp1OBint2scene1, disp1OBint2, sceneRenderBegin, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle1, sceneIdDisp1OBint2scene2, disp1OBint2, sceneRenderBegin, sceneRenderBegin);

    expectInterruptibleOffscreenBufferRendered(displayHandle2, { disp2OBint1, disp2OBint2 }, { { true, true },{ true, true } }, true);
    expectSceneRenderedWithInterruptionEnabled(displayHandle2, sceneIdDisp2OBint1scene1, disp2OBint1, sceneRenderBegin, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle2, sceneIdDisp2OBint1scene2, disp2OBint1, sceneRenderBegin, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle2, sceneIdDisp2OBint2scene1, disp2OBint2, sceneRenderBegin, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle2, sceneIdDisp2OBint2scene2, disp2OBint2, sceneRenderBegin, sceneRenderBegin);

    expectSwapBuffers(displayHandle2);
    expectSwapBuffers(displayHandle1, true);

    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle1).m_displayController);
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle2).m_displayController);

    // FBs have to be re-rendered because there were some interruptible OBs finished last frame,
    // interruptible OBs are rendered at end of frame so the FBs have to be rendered again next frame
    // in order to use the latest state of the OBs wherever they are used in FBs
    expectFrameBufferRendered(displayHandle1);
    expectSceneRendered(displayHandle1, sceneIdDisp1FB);
    expectFrameBufferRendered(displayHandle2);
    expectSceneRendered(displayHandle2, sceneIdDisp2FB);
    expectSwapBuffers(displayHandle2);
    expectSwapBuffers(displayHandle1, true);

    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle1).m_displayController);
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle2).m_displayController);

    // no change -> nothing rendered
    expectFrameBufferRendered(displayHandle1, false, false);
    expectFrameBufferRendered(displayHandle2, false, false);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle1).m_displayController);
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle2).m_displayController);

    hideScene(sceneIdDisp1FB);
    hideScene(sceneIdDisp2FB);
    hideScene(sceneIdDisp1OBscene);
    hideScene(sceneIdDisp2OBscene);
    hideScene(sceneIdDisp1OBint1scene1);
    hideScene(sceneIdDisp1OBint1scene2);
    hideScene(sceneIdDisp1OBint2scene1);
    hideScene(sceneIdDisp1OBint2scene2);
    hideScene(sceneIdDisp2OBint1scene1);
    hideScene(sceneIdDisp2OBint1scene2);
    hideScene(sceneIdDisp2OBint2scene1);
    hideScene(sceneIdDisp2OBint2scene2);

    unmapScene(sceneIdDisp1FB);
    unmapScene(sceneIdDisp2FB);
    unmapScene(sceneIdDisp1OBscene);
    unmapScene(sceneIdDisp2OBscene);
    unmapScene(sceneIdDisp1OBint1scene1);
    unmapScene(sceneIdDisp1OBint1scene2);
    unmapScene(sceneIdDisp1OBint2scene1);
    unmapScene(sceneIdDisp1OBint2scene2);
    unmapScene(sceneIdDisp2OBint1scene1);
    unmapScene(sceneIdDisp2OBint1scene2);
    unmapScene(sceneIdDisp2OBint2scene1);
    unmapScene(sceneIdDisp2OBint2scene2);
}

TEST_P(ARenderer, confidenceTest_combinationOfMultipleScenesAndOBsWhereScenesAreModifiedAtDifferentStatesOfInterruptionsAndExpectedToBeReRendered)
{
    // 2 FB, 2 interruptible OBs per display, 2 scenes per OB
    // OB1 never interrupts, OB2 scene1 never interrupts
    // OB1 scenes get modified sometime while interrupted

    const DisplayHandle displayHandle1 = addDisplayController();
    const DisplayHandle displayHandle2 = addDisplayController();
    DeviceResourceHandle disp1OB1(313u);
    DeviceResourceHandle disp1OB2(314u);
    DeviceResourceHandle disp2OB1(315u);
    DeviceResourceHandle disp2OB2(316u);
    renderer.registerOffscreenBuffer(displayHandle1, disp1OB1, 1u, 1u, true);
    renderer.registerOffscreenBuffer(displayHandle1, disp1OB2, 1u, 1u, true);
    renderer.registerOffscreenBuffer(displayHandle2, disp2OB1, 1u, 1u, true);
    renderer.registerOffscreenBuffer(displayHandle2, disp2OB2, 1u, 1u, true);

    const SceneId sceneIdDisp1FB(12u);
    const SceneId sceneIdDisp2FB(13u);
    const SceneId sceneIdDisp1OB1scene1(14u);
    const SceneId sceneIdDisp1OB1scene2(15u);
    const SceneId sceneIdDisp1OB2scene1(16u);
    const SceneId sceneIdDisp1OB2scene2(17u);
    const SceneId sceneIdDisp2OB1scene1(18u);
    const SceneId sceneIdDisp2OB1scene2(19u);
    const SceneId sceneIdDisp2OB2scene1(20u);
    const SceneId sceneIdDisp2OB2scene2(21u);

    createScene(sceneIdDisp1FB);
    createScene(sceneIdDisp2FB);
    createScene(sceneIdDisp1OB1scene1);
    createScene(sceneIdDisp1OB1scene2);
    createScene(sceneIdDisp1OB2scene1);
    createScene(sceneIdDisp1OB2scene2);
    createScene(sceneIdDisp2OB1scene1);
    createScene(sceneIdDisp2OB1scene2);
    createScene(sceneIdDisp2OB2scene1);
    createScene(sceneIdDisp2OB2scene2);

    mapSceneToDisplayBuffer(sceneIdDisp1FB, displayHandle1, 0);
    mapSceneToDisplayBuffer(sceneIdDisp2FB, displayHandle2, 0);
    mapSceneToDisplayBuffer(sceneIdDisp1OB2scene1, displayHandle1, 0, disp1OB2);
    mapSceneToDisplayBuffer(sceneIdDisp1OB2scene2, displayHandle1, 1, disp1OB2);
    mapSceneToDisplayBuffer(sceneIdDisp1OB1scene1, displayHandle1, 0, disp1OB1);
    mapSceneToDisplayBuffer(sceneIdDisp1OB1scene2, displayHandle1, 1, disp1OB1);
    mapSceneToDisplayBuffer(sceneIdDisp2OB2scene1, displayHandle2, 0, disp2OB2);
    mapSceneToDisplayBuffer(sceneIdDisp2OB2scene2, displayHandle2, 1, disp2OB2);
    mapSceneToDisplayBuffer(sceneIdDisp2OB1scene1, displayHandle2, 0, disp2OB1);
    mapSceneToDisplayBuffer(sceneIdDisp2OB1scene2, displayHandle2, 1, disp2OB1);

    showScene(sceneIdDisp1FB);
    showScene(sceneIdDisp2FB);
    showScene(sceneIdDisp1OB1scene1);
    showScene(sceneIdDisp1OB1scene2);
    showScene(sceneIdDisp1OB2scene1);
    showScene(sceneIdDisp1OB2scene2);
    showScene(sceneIdDisp2OB1scene1);
    showScene(sceneIdDisp2OB1scene2);
    showScene(sceneIdDisp2OB2scene1);
    showScene(sceneIdDisp2OB2scene2);

    // FBs rendered, disp1 OB1 rendered, disp1 OB2 scene1 rendered, disp1 OB2 scene2 interrupted, rest skipped
    expectSceneRendered(displayHandle1, sceneIdDisp1FB);
    expectSceneRendered(displayHandle2, sceneIdDisp2FB);
    expectFrameBufferRendered(displayHandle1);
    expectFrameBufferRendered(displayHandle2);
    expectSceneRenderedWithInterruptionEnabled(displayHandle1, sceneIdDisp1OB1scene1, disp1OB1, sceneRenderBegin, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle1, sceneIdDisp1OB1scene2, disp1OB1, sceneRenderBegin, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle1, sceneIdDisp1OB2scene1, disp1OB2, sceneRenderBegin, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle1, sceneIdDisp1OB2scene2, disp1OB2, sceneRenderBegin, sceneRenderInterrupted);
    expectInterruptibleOffscreenBufferRendered(displayHandle1, { disp1OB1, disp1OB2 }, { { true, true }, { true, false } }, true);
    expectSwapBuffers(displayHandle1);
    expectSwapBuffers(displayHandle2, true);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle1).m_displayController);
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle2).m_displayController);

    // FB1 rendered to reflect disp1 OB1 rendered, disp1 OB1 skipped, disp1 OB2 scene1 skipped, disp1 OB2 scene2 finished, disp2 OB1 scene1 interrupted, rest skipped
    expectSceneRendered(displayHandle1, sceneIdDisp1FB);
    expectFrameBufferRendered(displayHandle1);
    expectFrameBufferRendered(displayHandle2, false, false);
    expectSceneRenderedWithInterruptionEnabled(displayHandle1, sceneIdDisp1OB2scene2, disp1OB2, sceneRenderInterrupted, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle2, sceneIdDisp2OB1scene1, disp2OB1, sceneRenderBegin, sceneRenderInterrupted);
    expectInterruptibleOffscreenBufferRendered(displayHandle1, { disp1OB2 }, { { false, true } });
    expectInterruptibleOffscreenBufferRendered(displayHandle2, { disp2OB1 }, { { true, false } }, true);
    expectSwapBuffers(displayHandle1, true);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle1).m_displayController);
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle2).m_displayController);

    // modify some scenes
    renderer.markBufferWithMappedSceneAsModified(sceneIdDisp1OB1scene2);
    renderer.markBufferWithMappedSceneAsModified(sceneIdDisp1OB2scene1);

    // FB1 rendered to reflect disp1 OB2 rendered, disp1 OBs skipped due to interruption, disp2 OB1 scene1 finished, disp2 OB1 scene2 interrupted, rest skipped
    expectSceneRendered(displayHandle1, sceneIdDisp1FB);
    expectFrameBufferRendered(displayHandle1);
    expectFrameBufferRendered(displayHandle2, false, false);
    expectSceneRenderedWithInterruptionEnabled(displayHandle2, sceneIdDisp2OB1scene1, disp2OB1, sceneRenderInterrupted, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle2, sceneIdDisp2OB1scene2, disp2OB1, sceneRenderBegin, sceneRenderInterrupted);
    expectInterruptibleOffscreenBufferRendered(displayHandle2, { disp2OB1 }, { { false, false } }, true);
    expectSwapBuffers(displayHandle1, true);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle1).m_displayController);
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle2).m_displayController);

    // modify just finished scene on still interrupted OB
    renderer.markBufferWithMappedSceneAsModified(sceneIdDisp2OB1scene1);

    // FBs skipped due to no change, disp1 skipped due to interruption, disp2 OB1 scene2 finished, disp2 OB2 scene1 interrupted, rest skipped
    expectFrameBufferRendered(displayHandle1, false, false);
    expectFrameBufferRendered(displayHandle2, false, false);
    expectSceneRenderedWithInterruptionEnabled(displayHandle2, sceneIdDisp2OB1scene2, disp2OB1, sceneRenderInterrupted, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle2, sceneIdDisp2OB2scene1, disp2OB2, sceneRenderBegin, sceneRenderInterrupted);
    expectInterruptibleOffscreenBufferRendered(displayHandle2, { disp2OB1, disp2OB2 }, { { false, true }, { true, false } }, true);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle1).m_displayController);
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle2).m_displayController);

    // modify just started and unfinished scene on newly interrupted OB
    renderer.markBufferWithMappedSceneAsModified(sceneIdDisp2OB2scene1);

    // FB1 skipped due to no change, FB2 re-rendered to reflect disp2 OB1 rendered, disp2 OB2 scene1 finished, disp2 OB2 scene2 interrupted, rest skipped
    expectFrameBufferRendered(displayHandle1, false, false);
    expectFrameBufferRendered(displayHandle2);
    expectSceneRendered(displayHandle2, sceneIdDisp2FB);
    expectSceneRenderedWithInterruptionEnabled(displayHandle2, sceneIdDisp2OB2scene1, disp2OB2, sceneRenderInterrupted, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle2, sceneIdDisp2OB2scene2, disp2OB2, sceneRenderBegin, sceneRenderInterrupted);
    expectInterruptibleOffscreenBufferRendered(displayHandle2, { disp2OB2 }, { { false, false } });
    expectSwapBuffers(displayHandle2);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle1).m_displayController);
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle2).m_displayController);

    // FBs skipped due to no change, disp2 OB2 scene2 finished
    expectFrameBufferRendered(displayHandle1, false, false);
    expectFrameBufferRendered(displayHandle2, false, false);
    expectSceneRenderedWithInterruptionEnabled(displayHandle2, sceneIdDisp2OB2scene2, disp2OB2, sceneRenderInterrupted, sceneRenderBegin);
    expectInterruptibleOffscreenBufferRendered(displayHandle2, { disp2OB2 }, { { false, true } }, true);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle1).m_displayController);
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle2).m_displayController);

    // only now there is no interruption between frames, renderer will re-render all the buffers with scenes modified during interruption:
    //  expect disp1 OB1 and OB2 re-rendered
    //  expect disp2 OB1 and OB2 re-rendered

    // FB1 skipped due to no change, FB2 re-rendered to reflect disp2 OB2 rendered, disp1 re-rendered, disp2 OB1 re-rendered, disp2 OB2 skipped due to no change
    expectFrameBufferRendered(displayHandle1, false, false);
    expectFrameBufferRendered(displayHandle2);
    expectSceneRendered(displayHandle2, sceneIdDisp2FB);
    expectSceneRenderedWithInterruptionEnabled(displayHandle1, sceneIdDisp1OB1scene1, disp1OB1, sceneRenderBegin, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle1, sceneIdDisp1OB1scene2, disp1OB1, sceneRenderBegin, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle1, sceneIdDisp1OB2scene1, disp1OB2, sceneRenderBegin, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle1, sceneIdDisp1OB2scene2, disp1OB2, sceneRenderBegin, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle2, sceneIdDisp2OB1scene1, disp2OB1, sceneRenderBegin, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle2, sceneIdDisp2OB1scene2, disp2OB1, sceneRenderBegin, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle2, sceneIdDisp2OB2scene1, disp2OB2, sceneRenderBegin, sceneRenderBegin);
    expectSceneRenderedWithInterruptionEnabled(displayHandle2, sceneIdDisp2OB2scene2, disp2OB2, sceneRenderBegin, sceneRenderBegin);
    expectInterruptibleOffscreenBufferRendered(displayHandle1, { disp1OB1, disp1OB2 }, { { true, true }, { true, true } }, true);
    expectInterruptibleOffscreenBufferRendered(displayHandle2, { disp2OB1, disp2OB2 }, { { true, true }, { true, true } }, true);
    expectSwapBuffers(displayHandle2);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle1).m_displayController);
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle2).m_displayController);

    // Both FBs re-rendered to reflect changes on both display's OBs
    expectFrameBufferRendered(displayHandle1);
    expectFrameBufferRendered(displayHandle2);
    expectSceneRendered(displayHandle1, sceneIdDisp1FB);
    expectSceneRendered(displayHandle2, sceneIdDisp2FB);
    expectSwapBuffers(displayHandle2);
    expectSwapBuffers(displayHandle1, true);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle1).m_displayController);
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle2).m_displayController);

    // no change -> nothing rendered
    expectFrameBufferRendered(displayHandle1, false, false);
    expectFrameBufferRendered(displayHandle2, false, false);
    doOneRendererLoop();

    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle1).m_displayController);
    Mock::VerifyAndClearExpectations(renderer.getDisplayMock(displayHandle2).m_displayController);

    hideScene(sceneIdDisp1FB);
    hideScene(sceneIdDisp2FB);
    hideScene(sceneIdDisp1OB1scene1);
    hideScene(sceneIdDisp1OB1scene2);
    hideScene(sceneIdDisp1OB2scene1);
    hideScene(sceneIdDisp1OB2scene2);
    hideScene(sceneIdDisp2OB1scene1);
    hideScene(sceneIdDisp2OB1scene2);
    hideScene(sceneIdDisp2OB2scene1);
    hideScene(sceneIdDisp2OB2scene2);

    unmapScene(sceneIdDisp1FB);
    unmapScene(sceneIdDisp2FB);
    unmapScene(sceneIdDisp1OB1scene1);
    unmapScene(sceneIdDisp1OB1scene2);
    unmapScene(sceneIdDisp1OB2scene1);
    unmapScene(sceneIdDisp1OB2scene2);
    unmapScene(sceneIdDisp2OB1scene1);
    unmapScene(sceneIdDisp2OB1scene2);
    unmapScene(sceneIdDisp2OB2scene1);
    unmapScene(sceneIdDisp2OB2scene2);
}

TEST_P(ARenderer, canMapSceneWhileThereIsInterruption)
{
    const DisplayHandle displayHandle = addDisplayController();
    const DeviceResourceHandle fakeOffscreenBuffer(313u);
    renderer.registerOffscreenBuffer(displayHandle, fakeOffscreenBuffer, 1u, 1u, true);

    const SceneId sceneIdFB(12u);
    const SceneId sceneIdOB(13u);
    const SceneId sceneId2(14u);
    createScene(sceneIdFB);
    createScene(sceneIdOB);
    createScene(sceneId2);
    mapSceneToDisplayBuffer(sceneIdFB, displayHandle, 0);
    mapSceneToDisplayBuffer(sceneIdOB, displayHandle, 0, fakeOffscreenBuffer);

    showScene(sceneIdFB);
    showScene(sceneIdOB);

    // FB rendered, OB interrupted
    expectSceneRendered(displayHandle, sceneIdFB);
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneIdOB, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderInterrupted);
    expectFrameBufferRendered();
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, { { true, false } });
    expectSwapBuffers();
    doOneRendererLoop();

    // map other scene to FB while interrupted
    mapSceneToDisplayBuffer(sceneId2, displayHandle, 0);

    // FB rendered because of new scene mapped, OB finished
    expectFrameBufferRendered();
    expectSceneRendered(displayHandle, sceneIdFB);
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneIdOB, fakeOffscreenBuffer, sceneRenderInterrupted, sceneRenderBegin);
    expectInterruptibleOffscreenBufferRendered(displayHandle, { fakeOffscreenBuffer }, { { false, true } });
    expectSwapBuffers();
    doOneRendererLoop();

    // re-render FB to reflect change happened to OB in previous frame
    expectSceneRendered(displayHandle, sceneIdFB);
    expectFrameBufferRendered();
    expectSwapBuffers();
    doOneRendererLoop();

    // no change -> nothing rendered
    expectFrameBufferRendered(displayHandle, false, false);
    doOneRendererLoop();

    hideScene(sceneIdOB);
    hideScene(sceneIdFB);
    unmapScene(sceneIdOB);
    unmapScene(sceneIdFB);
    unmapScene(sceneId2);
}

TEST_P(ARenderer, doesNotReportSceneIfNotRenderedToExpirationMonitor)
{
    const DisplayHandle displayHandle = addDisplayController();
    const SceneId sceneIdFB(1u);
    const SceneId sceneIdOB(2u);
    const SceneId sceneIdOBint(3u);
    createScene(sceneIdFB);
    createScene(sceneIdOB);
    createScene(sceneIdOBint);

    initiateExpirationMonitoring({ sceneIdOB, sceneIdFB, sceneIdOBint });

    DeviceResourceHandle ob(316u);
    DeviceResourceHandle obInt(317u);
    renderer.registerOffscreenBuffer(displayHandle, ob, 1u, 1u, false);
    renderer.registerOffscreenBuffer(displayHandle, obInt, 1u, 1u, true);

    mapSceneToDisplayBuffer(sceneIdFB, displayHandle, 0);
    mapSceneToDisplayBuffer(sceneIdOB, displayHandle, 0, ob);
    mapSceneToDisplayBuffer(sceneIdOBint, displayHandle, 0, obInt);

    expectOffscreenBufferRendered(displayHandle, { ob }, true);
    expectFrameBufferRendered(displayHandle, false);
    expectInterruptibleOffscreenBufferRendered(displayHandle, { obInt }, { { true, true } });
    expectSwapBuffers();
    doOneRendererLoop();

    expectScenesReportedToExpirationMonitorAsRendered({});

    unmapScene(sceneIdFB);
    unmapScene(sceneIdOB);
    unmapScene(sceneIdOBint);
    expirationMonitor.stopMonitoringScene(sceneIdFB);
    expirationMonitor.stopMonitoringScene(sceneIdOB);
    expirationMonitor.stopMonitoringScene(sceneIdOBint);
}

TEST_P(ARenderer, reportsSceneAsRenderedToExpirationMonitor)
{
    const DisplayHandle displayHandle = addDisplayController();
    const SceneId sceneIdFB(1u);
    const SceneId sceneIdOB(2u);
    const SceneId sceneIdOBint(3u);
    createScene(sceneIdFB);
    createScene(sceneIdOB);
    createScene(sceneIdOBint);

    initiateExpirationMonitoring({ sceneIdOB, sceneIdFB, sceneIdOBint });

    DeviceResourceHandle ob(316u);
    DeviceResourceHandle obInt(317u);
    renderer.registerOffscreenBuffer(displayHandle, ob, 1u, 1u, false);
    renderer.registerOffscreenBuffer(displayHandle, obInt, 1u, 1u, true);

    mapSceneToDisplayBuffer(sceneIdFB, displayHandle, 0);
    mapSceneToDisplayBuffer(sceneIdOB, displayHandle, 0, ob);
    mapSceneToDisplayBuffer(sceneIdOBint, displayHandle, 0, obInt);

    showScene(sceneIdFB);
    showScene(sceneIdOB);
    showScene(sceneIdOBint);

    expectOffscreenBufferRendered(displayHandle, { ob }, true);
    expectFrameBufferRendered(displayHandle, false);
    expectInterruptibleOffscreenBufferRendered(displayHandle, { obInt }, { { true, true } });
    expectSceneRendered(displayHandle, sceneIdOB, ob);
    expectSceneRendered(displayHandle, sceneIdFB);
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneIdOBint, obInt, sceneRenderBegin, sceneRenderBegin);
    expectSwapBuffers();
    doOneRendererLoop();

    expectScenesReportedToExpirationMonitorAsRendered({ sceneIdOB, sceneIdFB, sceneIdOBint });

    hideScene(sceneIdFB);
    hideScene(sceneIdOB);
    hideScene(sceneIdOBint);
    unmapScene(sceneIdFB);
    unmapScene(sceneIdOB);
    unmapScene(sceneIdOBint);
    expirationMonitor.stopMonitoringScene(sceneIdFB);
    expirationMonitor.stopMonitoringScene(sceneIdOB);
    expirationMonitor.stopMonitoringScene(sceneIdOBint);
}

TEST_P(ARenderer, reportsSceneAsRenderedToExpirationMonitorOnlyAfterFullyRenderedAndNotDuringInterruption)
{
    const DisplayHandle displayHandle = addDisplayController();
    const SceneId sceneIdFB(1u);
    const SceneId sceneIdOBint(3u);
    createScene(sceneIdFB);
    createScene(sceneIdOBint);

    initiateExpirationMonitoring({ sceneIdFB, sceneIdOBint });

    DeviceResourceHandle obInt(317u);
    renderer.registerOffscreenBuffer(displayHandle, obInt, 1u, 1u, true);

    mapSceneToDisplayBuffer(sceneIdFB, displayHandle, 0);
    mapSceneToDisplayBuffer(sceneIdOBint, displayHandle, 0, obInt);

    showScene(sceneIdFB);
    showScene(sceneIdOBint);

    expectFrameBufferRendered();
    expectInterruptibleOffscreenBufferRendered(displayHandle, { obInt }, { { true, false } });
    expectSceneRendered(displayHandle, sceneIdFB);
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneIdOBint, obInt, sceneRenderBegin, sceneRenderInterrupted);
    expectSwapBuffers();
    doOneRendererLoop();

    // only FB scene reported as rendered, OB scene is interrupted
    expectScenesReportedToExpirationMonitorAsRendered({ sceneIdFB });

    expectFrameBufferRendered(displayHandle, false, false);
    expectInterruptibleOffscreenBufferRendered(displayHandle, { obInt }, { { false, true } }, true);
    expectSceneRenderedWithInterruptionEnabled(displayHandle, sceneIdOBint, obInt, sceneRenderInterrupted, sceneRenderBegin);
    doOneRendererLoop();

    // OB scene is reported now as it was fully rendered
    expectScenesReportedToExpirationMonitorAsRendered({ sceneIdOBint });

    hideScene(sceneIdFB);
    hideScene(sceneIdOBint);
    unmapScene(sceneIdFB);
    unmapScene(sceneIdOBint);
    expirationMonitor.stopMonitoringScene(sceneIdFB);
    expirationMonitor.stopMonitoringScene(sceneIdOBint);
}
