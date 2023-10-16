//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/RendererConfig.h"
#include "RenderBackendMock.h"
#include "PlatformMock.h"
#include "internal/RendererLib/RenderingContext.h"
#include "internal/RendererLib/RendererCommandBuffer.h"
#include "internal/RendererLib/RendererCachedScene.h"
#include "internal/RendererLib/DisplayController.h"
#include "internal/RendererLib/RendererScenes.h"
#include "internal/RendererLib/RendererLogContext.h"
#include "internal/RendererLib/SceneExpirationMonitor.h"
#include "internal/RendererLib/RendererEventCollector.h"
#include "DisplayControllerMock.h"
#include "internal/PlatformAbstraction/Collections/Pair.h"
#include "RendererMock.h"
#include "ComponentMocks.h"
#include "TestSceneHelper.h"
#include <algorithm>
#include "internal/Core/Utils/ThreadLocalLog.h"

namespace ramses::internal
{
    class ARenderer : public ::testing::TestWithParam<bool> // parametrized with/out system compositor
    {
    public:
        ARenderer()
            : rendererScenes(rendererEventCollector)
            , expirationMonitor(rendererScenes, rendererEventCollector, rendererStatistics)
            , renderer(DisplayHandle{ 1u }, rendererScenes, rendererEventCollector, expirationMonitor, rendererStatistics)
        {
            // caller is expected to have a display prefix for logs
            ThreadLocalLog::SetPrefix(1);

            sceneRenderInterrupted.incrementRenderableIdx();
            sceneRenderInterrupted2.incrementRenderableIdx();
            sceneRenderInterrupted2.incrementRenderableIdx();

            // Enable/disable SC
            if (GetParam())
                ON_CALL(renderer.m_platform, getSystemCompositorController()).WillByDefault(Return(&renderer.m_platform.systemCompositorControllerMock));
        }

        ~ARenderer() override
        {
            if (renderer.hasDisplayController())
                destroyDisplayController();
            for (const auto& sceneIt : rendererScenes)
                expirationMonitor.onDestroyed(sceneIt.key);
        }

        void createDisplayController()
        {
            ASSERT_FALSE(renderer.hasDisplayController());
            renderer.createDisplayContext({});
        }

        void destroyDisplayController()
        {
            ASSERT_TRUE(renderer.hasDisplayController());
            if (GetParam())
            {
                EXPECT_CALL(*renderer.m_displayController, getRenderBackend());
                EXPECT_CALL(renderer.m_platform.renderBackendMock.windowMock, getWaylandIviSurfaceID());
                EXPECT_CALL(renderer.m_platform.systemCompositorControllerMock, destroySurface(_));
            }
            EXPECT_CALL(renderer.m_platform, destroyRenderBackend());
            renderer.destroyDisplayContext();
        }

        void expectOffscreenBufferCleared(DeviceResourceHandle buffer, ClearFlags clearFlags = EClearFlag::All, const glm::vec4& clearColor = Renderer::DefaultClearColor)
        {
            EXPECT_CALL(*renderer.m_displayController, clearBuffer(buffer, clearFlags, clearColor)).InSequence(SeqRender);
        }

        void expectInterruptibleOffscreenBufferSwapped(DeviceResourceHandle buffer)
        {
            EXPECT_CALL(renderer.m_platform.renderBackendMock.deviceMock, swapDoubleBufferedRenderTarget(buffer)).InSequence(SeqRender);

            // rendering to interruptible buffers calls this uninteresting getter which gets reset with gmock verification
            EXPECT_CALL(*renderer.m_displayController, getRenderBackend()).Times(AnyNumber());
        }

        void expectFrameBufferRendered(bool expectRerender = true, ClearFlags expectRendererClear = EClearFlag::All, const glm::vec4& clearColor = Renderer::DefaultClearColor)
        {
            EXPECT_CALL(*renderer.m_displayController, handleWindowEvents()).InSequence(SeqPreRender);
            EXPECT_CALL(*renderer.m_displayController, canRenderNewFrame()).InSequence(SeqPreRender).WillOnce(Return(true));

            if (expectRerender)
            {
                // normally render executor clears, in some cases (no scene assigned) renderer clears instead
                if (expectRendererClear != EClearFlag::None)
                    EXPECT_CALL(*renderer.m_displayController, clearBuffer(DisplayControllerMock::FakeFrameBufferHandle, expectRendererClear, clearColor));
            }
            else
            {
                EXPECT_CALL(*renderer.m_displayController, getEmbeddedCompositingManager()).InSequence(SeqRender);
                EXPECT_CALL(renderer.m_embeddedCompositingManager, notifyClients()).InSequence(SeqRender);
            }
        }

        void expectSwapBuffers()
        {
            EXPECT_CALL(*renderer.m_displayController, swapBuffers()).InSequence(SeqRender);
            EXPECT_CALL(*renderer.m_displayController, getEmbeddedCompositingManager()).InSequence(SeqRender);
            EXPECT_CALL(renderer.m_embeddedCompositingManager, notifyClients()).InSequence(SeqRender);
        }

        void doOneRendererLoop()
        {
            renderer.getProfilerStatistics().markFrameFinished(std::chrono::microseconds{ 0u });
            renderer.doOneRenderLoop();
        }

        enum class EDiscardDepth
        {
            Allowed,
            Disallowed
        };

        void expectSceneRenderedExt(
            SceneId sceneId,
            DeviceResourceHandle buffer,
            ClearFlags dispBufferClearFlags,
            const glm::vec4& dispBufferClearColor,
            SceneRenderExecutionIterator expectedRenderBegin,
            SceneRenderExecutionIterator iteratorToReturn,
            ClearFlags clearFlagsToModify,
            EDiscardDepth discardAllowed,
            const FrameTimer* frameTimer = nullptr)
        {
            EXPECT_CALL(*renderer.m_displayController, renderScene(Ref(rendererScenes.getScene(sceneId)), _, frameTimer))
                .WillOnce([=](const auto& /*unused*/, RenderingContext& renderContext, const auto* /*unused*/) {
                EXPECT_EQ(buffer, renderContext.displayBufferDeviceHandle);
                EXPECT_EQ(expectedRenderBegin, renderContext.renderFrom);
                EXPECT_EQ(dispBufferClearFlags, renderContext.displayBufferClearPending);
                if (dispBufferClearFlags != EClearFlag::None) // color is relevant only if clearing something
                {
                    EXPECT_EQ(dispBufferClearColor, renderContext.displayBufferClearColor);
                }
                renderContext.displayBufferClearPending = clearFlagsToModify;
                EXPECT_EQ((discardAllowed == EDiscardDepth::Allowed), renderContext.displayBufferDepthDiscard);
                return iteratorToReturn;
            });
        }

        void expectSceneRendered(SceneId sceneId, DeviceResourceHandle buffer = DisplayControllerMock::FakeFrameBufferHandle,
            ClearFlags dispBufferClearFlags = EClearFlag::All, const glm::vec4& dispBufferClearColor = Renderer::DefaultClearColor)
        {
            expectSceneRenderedExt(sceneId, buffer, dispBufferClearFlags, dispBufferClearColor, sceneRenderBegin, sceneRenderBegin, dispBufferClearFlags, EDiscardDepth::Disallowed);
        }

        void expectSceneRenderedWithInterruptionEnabled(SceneId sceneId, DeviceResourceHandle buffer, SceneRenderExecutionIterator expectedRenderBegin,
            SceneRenderExecutionIterator stateToSimulate, ClearFlags dispBufferClearFlags = EClearFlag::All)
        {
            expectSceneRenderedExt(sceneId, buffer, dispBufferClearFlags, Renderer::DefaultClearColor, expectedRenderBegin, stateToSimulate, dispBufferClearFlags, EDiscardDepth::Disallowed, &RendererMock::FrameTimerInstance);
        }

        void expectSceneRendered(SceneId sceneId, DeviceResourceHandle buffer, EDiscardDepth discardAllowed)
        {
            expectSceneRenderedExt(sceneId, buffer, EClearFlag::All, Renderer::DefaultClearColor, sceneRenderBegin, sceneRenderBegin, EClearFlag::All, discardAllowed);
        }

        void expectDisplayControllerReadPixels(DeviceResourceHandle deviceHandle, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
        {
            EXPECT_CALL(*renderer.m_displayController, readPixels(deviceHandle, x, y, width, height, _)).WillOnce(Invoke(
                [](auto /*unused*/, auto /*unused*/, auto /*unused*/, auto w, auto h, auto& dataOut) {
                    dataOut.resize(w * h * 4);
                }
            ));
        }

        IScene& createScene(SceneId sceneId = SceneId())
        {
            rendererScenes.createScene(SceneInfo(sceneId));
            return rendererScenes.getScene(sceneId);
        }

        void assignSceneToDisplayBuffer(SceneId sceneId, int32_t sceneRenderOrder, DeviceResourceHandle displayBuffer = DisplayControllerMock::FakeFrameBufferHandle)
        {
            renderer.assignSceneToDisplayBuffer(sceneId, displayBuffer, sceneRenderOrder);
            EXPECT_EQ(displayBuffer, renderer.getBufferSceneIsAssignedTo(sceneId));
            EXPECT_EQ(sceneRenderOrder, renderer.getSceneGlobalOrder(sceneId));
        }

        void unassignScene(SceneId sceneId)
        {
            renderer.unassignScene(sceneId);
            EXPECT_FALSE(renderer.getBufferSceneIsAssignedTo(sceneId).isValid());
        }

        void showScene(SceneId sceneId)
        {
            renderer.setSceneShown(sceneId, true);
        }

        void hideScene(SceneId sceneId)
        {
            renderer.setSceneShown(sceneId, false);
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

        void scheduleScreenshot(const DeviceResourceHandle bufferHandle, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
        {
            ScreenshotInfo screenshot;
            screenshot.rectangle = { x, y, width, height };
            renderer.scheduleScreenshot(bufferHandle, std::move(screenshot));
        }

    protected:
        RendererCommandBuffer                       rendererCommandBuffer;
        RendererEventCollector                      rendererEventCollector;
        RendererScenes                              rendererScenes;
        SceneExpirationMonitor                      expirationMonitor;
        RendererStatistics                          rendererStatistics;
        StrictMock<RendererMockWithStrictMockDisplay> renderer;

        const SceneRenderExecutionIterator sceneRenderBegin{};
        SceneRenderExecutionIterator sceneRenderInterrupted;
        SceneRenderExecutionIterator sceneRenderInterrupted2;

        // sequence of ordered expectations during render
        Sequence SeqRender;
        // sequence of ordered expectations at beginning of render (display events, can render frame, etc.)
        Sequence SeqPreRender;

        const FlushTime::Clock::time_point currentFakeTime{ std::chrono::milliseconds(1000) };
    };

    INSTANTIATE_TEST_SUITE_P(, ARenderer, ::testing::Values(false, true));

    TEST_P(ARenderer, ListIviSurfacesInSystemCompositorController)
    {
        if (GetParam())
            EXPECT_CALL(renderer.m_platform.systemCompositorControllerMock, listIVISurfaces());
        renderer.systemCompositorListIviSurfaces();
    }

    TEST_P(ARenderer, SetsVisibilityInSystemCompositorController)
    {
        if (GetParam())
        {
            EXPECT_CALL(renderer.m_platform.systemCompositorControllerMock, setSurfaceVisibility(WaylandIviSurfaceId(1), false));
            EXPECT_CALL(renderer.m_platform.systemCompositorControllerMock, setSurfaceVisibility(WaylandIviSurfaceId(2), true));
        }
        renderer.systemCompositorSetIviSurfaceVisibility(WaylandIviSurfaceId(1), false);
        renderer.systemCompositorSetIviSurfaceVisibility(WaylandIviSurfaceId(2), true);
    }

    TEST_P(ARenderer, TakesScreenshotFromSystemCompositorController)
    {
        std::string_view fileName("screenshot.png");
        const int32_t screenIviId = 3;
        if (GetParam())
            EXPECT_CALL(renderer.m_platform.systemCompositorControllerMock, doScreenshot(fileName, screenIviId));
        renderer.systemCompositorScreenshot(fileName, screenIviId);
    }

    TEST_P(ARenderer, rendersOneLoop)
    {
        createDisplayController();
        EXPECT_TRUE(renderer.hasDisplayController());

        expectFrameBufferRendered();
        expectSwapBuffers();
        doOneRendererLoop();
    }

    TEST_P(ARenderer, unregisteredSceneIsNotMapped)
    {
        EXPECT_FALSE(renderer.getBufferSceneIsAssignedTo(SceneId(0u)).isValid());
    }

    TEST_P(ARenderer, doesNotMapCreatedScene)
    {
        const SceneId sceneId(12u);
        createScene(sceneId);
        EXPECT_FALSE(renderer.getBufferSceneIsAssignedTo(sceneId).isValid());
    }

    TEST_P(ARenderer, canMapSceneOnDisplay)
    {
        createDisplayController();

        const SceneId sceneId(12u);
        createScene(sceneId);
        assignSceneToDisplayBuffer(sceneId, 0);
        unassignScene(sceneId);
    }

    TEST_P(ARenderer, assignsSceneToNativeFramebufferOfDisplayWhenMappingScene)
    {
        createDisplayController();

        const SceneId sceneId(12u);
        createScene(sceneId);
        assignSceneToDisplayBuffer(sceneId, 0);

        EXPECT_EQ(DisplayControllerMock::FakeFrameBufferHandle, renderer.getBufferSceneIsAssignedTo(sceneId));
        unassignScene(sceneId);
    }

    TEST_P(ARenderer, assignsSceneToOffscreenBuffer)
    {
        createDisplayController();

        const SceneId sceneId(12u);
        createScene(sceneId);
        const DeviceResourceHandle fakeOffscreenBuffer(313u);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer, 1u, 1u, false);
        assignSceneToDisplayBuffer(sceneId, 0, fakeOffscreenBuffer);
        EXPECT_EQ(fakeOffscreenBuffer, renderer.getBufferSceneIsAssignedTo(sceneId));
        EXPECT_FALSE(renderer.isSceneAssignedToInterruptibleOffscreenBuffer(sceneId));

        unassignScene(sceneId);
    }

    TEST_P(ARenderer, assignsSceneToInterruptibleOffscreenBuffer)
    {
        createDisplayController();

        const SceneId sceneId(12u);
        createScene(sceneId);
        const DeviceResourceHandle fakeOffscreenBuffer(313u);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer, 1u, 1u, true);
        assignSceneToDisplayBuffer(sceneId, 0, fakeOffscreenBuffer);
        EXPECT_EQ(fakeOffscreenBuffer, renderer.getBufferSceneIsAssignedTo(sceneId));
        EXPECT_TRUE(renderer.isSceneAssignedToInterruptibleOffscreenBuffer(sceneId));

        unassignScene(sceneId);
    }

    TEST_P(ARenderer, assignsSceneFromInterruptibleOffscreenBufferToNormalOB)
    {
        createDisplayController();

        const SceneId sceneId(12u);
        createScene(sceneId);
        const DeviceResourceHandle ob(313u);
        const DeviceResourceHandle obInterruptible(314u);
        renderer.registerOffscreenBuffer(ob, 1u, 1u, false);
        renderer.registerOffscreenBuffer(obInterruptible, 1u, 1u, true);

        assignSceneToDisplayBuffer(sceneId, 0, obInterruptible);
        EXPECT_EQ(obInterruptible, renderer.getBufferSceneIsAssignedTo(sceneId));
        EXPECT_TRUE(renderer.isSceneAssignedToInterruptibleOffscreenBuffer(sceneId));

        assignSceneToDisplayBuffer(sceneId, 0, ob);
        EXPECT_EQ(ob, renderer.getBufferSceneIsAssignedTo(sceneId));
        EXPECT_FALSE(renderer.isSceneAssignedToInterruptibleOffscreenBuffer(sceneId));

        unassignScene(sceneId);
    }

    TEST_P(ARenderer, SetsLayerVisibilityInSystemCompositorController)
    {
        if (GetParam())
        {
            EXPECT_CALL(renderer.m_platform.systemCompositorControllerMock, setLayerVisibility(WaylandIviLayerId(18u), false));
            EXPECT_CALL(renderer.m_platform.systemCompositorControllerMock, setLayerVisibility(WaylandIviLayerId(17u), true));
        }
        renderer.systemCompositorSetIviLayerVisibility(WaylandIviLayerId(18u), false);
        renderer.systemCompositorSetIviLayerVisibility(WaylandIviLayerId(17u), true);
    }

    TEST_P(ARenderer, doesNotClearOrRenderToOffscreenBufferIfThereIsNoSceneAssignedToIt)
    {
        createDisplayController();

        const DeviceResourceHandle fakeOffscreenBuffer(313u);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer, 1u, 2u, false);

        expectFrameBufferRendered(true, EClearFlag::All);
        // no offscreen buffer clear expectation
        expectSwapBuffers();
        doOneRendererLoop();
    }

    TEST_P(ARenderer, clearsOffscreenBufferIfThereIsSceneAssignedToItAndNotShown)
    {
        createDisplayController();

        const SceneId sceneId(12u);
        createScene(sceneId);

        const DeviceResourceHandle fakeOffscreenBuffer(313u);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer, 1u, 2u, false);
        assignSceneToDisplayBuffer(sceneId, 0, fakeOffscreenBuffer);

        expectOffscreenBufferCleared(fakeOffscreenBuffer);
        expectFrameBufferRendered(true, EClearFlag::All);
        expectSwapBuffers();
        doOneRendererLoop();

        unassignScene(sceneId);
    }

    TEST_P(ARenderer, clearsOffscreenBufferAndFramebufferWithRelatedColors)
    {
        const glm::vec4 displayClearColor(.1f, .2f, .3f, .4f);
        createDisplayController();
        renderer.setClearColor(DisplayControllerMock::FakeFrameBufferHandle, displayClearColor);

        const SceneId sceneId(12u);
        createScene(sceneId);

        const DeviceResourceHandle fakeOffscreenBuffer(313u);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer, 1u, 2u, false);
        assignSceneToDisplayBuffer(sceneId, 0, fakeOffscreenBuffer);

        expectOffscreenBufferCleared(fakeOffscreenBuffer);
        expectFrameBufferRendered(true, EClearFlag::All, displayClearColor);
        expectSwapBuffers();
        doOneRendererLoop();

        unassignScene(sceneId);
    }

    TEST_P(ARenderer, clearsFramebufferWithCustomClearColor)
    {
        const glm::vec4 displayClearColor(.1f, .2f, .3f, .4f);
        createDisplayController();
        renderer.setClearColor(DisplayControllerMock::FakeFrameBufferHandle, displayClearColor);
        expectFrameBufferRendered(true, EClearFlag::All, displayClearColor);
        expectSwapBuffers();
        doOneRendererLoop();
    }

    TEST_P(ARenderer, clearsOffscreenBufferWithCustomClearColor)
    {
        createDisplayController();

        const glm::vec4 obClearColor(.1f, .2f, .3f, .4f);
        const SceneId sceneId(12u);
        createScene(sceneId);

        const DeviceResourceHandle fakeOffscreenBuffer(313u);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer, 1u, 2u, false);
        renderer.setClearColor(fakeOffscreenBuffer, obClearColor);

        assignSceneToDisplayBuffer(sceneId, 0, fakeOffscreenBuffer);

        expectOffscreenBufferCleared(fakeOffscreenBuffer, EClearFlag::All, obClearColor);
        expectFrameBufferRendered();
        expectSwapBuffers();
        doOneRendererLoop();

        unassignScene(sceneId);
    }

    TEST_P(ARenderer, clearsBothFramebufferAndOffscreenBufferWithDifferentClearColors)
    {
        createDisplayController();
        const glm::vec4 displayClearColor(.4f, .3f, .2f, .1f);
        renderer.setClearColor(DisplayControllerMock::FakeFrameBufferHandle, displayClearColor);

        const glm::vec4 obClearColor(.1f, .2f, .3f, .4f);
        const SceneId sceneId(12u);
        createScene(sceneId);

        const DeviceResourceHandle fakeOffscreenBuffer(313u);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer, 1u, 2u, false);
        renderer.setClearColor(fakeOffscreenBuffer, obClearColor);

        assignSceneToDisplayBuffer(sceneId, 0, fakeOffscreenBuffer);

        expectOffscreenBufferCleared(fakeOffscreenBuffer, EClearFlag::All, obClearColor);
        expectFrameBufferRendered(true, EClearFlag::All, displayClearColor);
        expectSwapBuffers();
        doOneRendererLoop();

        unassignScene(sceneId);
    }

    TEST_P(ARenderer, clearsFBIfNoSceneAssigned)
    {
        createDisplayController();

        // use some non-default clear flags
        EXPECT_CALL(renderer, setClearFlags(DisplayControllerMock::FakeFrameBufferHandle, ClearFlags(EClearFlag::Depth)));
        renderer.setClearFlags(DisplayControllerMock::FakeFrameBufferHandle, EClearFlag::Depth);

        expectFrameBufferRendered(true, EClearFlag::Depth);
        expectSwapBuffers();
        doOneRendererLoop();
    }

    TEST_P(ARenderer, clearsFBIfNoShownSceneAssigned)
    {
        createDisplayController();

        // assign scene to trigger render of OB
        constexpr SceneId sceneId1{ 12u };
        constexpr SceneId sceneId2{ 13u };
        createScene(sceneId1);
        createScene(sceneId2);
        assignSceneToDisplayBuffer(sceneId1, 0);
        assignSceneToDisplayBuffer(sceneId2, 0);

        // use some non-default clear flags
        EXPECT_CALL(renderer, setClearFlags(DisplayControllerMock::FakeFrameBufferHandle, ClearFlags(EClearFlag::Depth)));
        renderer.setClearFlags(DisplayControllerMock::FakeFrameBufferHandle, EClearFlag::Depth);

        expectFrameBufferRendered(true, EClearFlag::Depth);
        expectSwapBuffers();
        doOneRendererLoop();
    }

    TEST_P(ARenderer, clearsOBOnRerenderIfNoSceneAssigned)
    {
        createDisplayController();

        constexpr DeviceResourceHandle fakeOffscreenBuffer1{ 313u };
        constexpr DeviceResourceHandle fakeOffscreenBuffer2{ 314u };
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer1, 1u, 2u, false);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer2, 1u, 2u, true);

        // use some non-default clear flags
        EXPECT_CALL(renderer, setClearFlags(fakeOffscreenBuffer1, ClearFlags(EClearFlag::Depth)));
        EXPECT_CALL(renderer, setClearFlags(fakeOffscreenBuffer2, ClearFlags(EClearFlag::Depth)));
        EXPECT_CALL(renderer, setClearColor(fakeOffscreenBuffer1, glm::vec4{ 1,2,3,4 }));
        EXPECT_CALL(renderer, setClearColor(fakeOffscreenBuffer2, Renderer::DefaultClearColor));
        renderer.setClearFlags(fakeOffscreenBuffer1, EClearFlag::Depth);
        renderer.setClearFlags(fakeOffscreenBuffer2, EClearFlag::Depth);
        renderer.setClearColor(fakeOffscreenBuffer1, glm::vec4{ 1,2,3,4 });
        renderer.setClearColor(fakeOffscreenBuffer2, Renderer::DefaultClearColor);

        expectOffscreenBufferCleared(fakeOffscreenBuffer1, EClearFlag::Depth, glm::vec4{ 1,2,3,4 });
        expectFrameBufferRendered();
        expectOffscreenBufferCleared(fakeOffscreenBuffer2, EClearFlag::Depth);
        expectInterruptibleOffscreenBufferSwapped(fakeOffscreenBuffer2);
        expectSwapBuffers();
        doOneRendererLoop();
    }

    TEST_P(ARenderer, clearsOBOnRerenderIfNoShownSceneAssigned)
    {
        createDisplayController();

        constexpr DeviceResourceHandle fakeOffscreenBuffer1{ 313u };
        constexpr DeviceResourceHandle fakeOffscreenBuffer2{ 314u };
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer1, 1u, 2u, false);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer2, 1u, 2u, true);

        // assign scene to trigger render of OB
        constexpr SceneId sceneId1{ 12u };
        constexpr SceneId sceneId2{ 13u };
        createScene(sceneId1);
        createScene(sceneId2);
        assignSceneToDisplayBuffer(sceneId1, 0, fakeOffscreenBuffer1);
        assignSceneToDisplayBuffer(sceneId2, 0, fakeOffscreenBuffer2);

        // use some non-default clear flags
        EXPECT_CALL(renderer, setClearFlags(fakeOffscreenBuffer1, ClearFlags(EClearFlag::Depth)));
        EXPECT_CALL(renderer, setClearFlags(fakeOffscreenBuffer2, ClearFlags(EClearFlag::Depth)));
        renderer.setClearFlags(fakeOffscreenBuffer1, EClearFlag::Depth);
        renderer.setClearFlags(fakeOffscreenBuffer2, EClearFlag::Depth);

        expectOffscreenBufferCleared(fakeOffscreenBuffer1, EClearFlag::Depth);
        expectFrameBufferRendered();
        expectOffscreenBufferCleared(fakeOffscreenBuffer2, EClearFlag::Depth);
        expectInterruptibleOffscreenBufferSwapped(fakeOffscreenBuffer2);
        expectSwapBuffers();
        doOneRendererLoop();

        unassignScene(sceneId1);
        unassignScene(sceneId2);
    }

    TEST_P(ARenderer, rendersTwoOffscreenBuffersWithContentInCorrectOrder)
    {
        createDisplayController();

        DeviceResourceHandle fakeOffscreenBuffer1(313u);
        DeviceResourceHandle fakeOffscreenBuffer2(314u);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer1, 1u, 2u, false);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer2, 1u, 2u, false);

        const SceneId sceneId1(12u);
        const SceneId sceneId2(13u);
        createScene(sceneId1);
        createScene(sceneId2);
        assignSceneToDisplayBuffer(sceneId1, 0, fakeOffscreenBuffer1);
        assignSceneToDisplayBuffer(sceneId2, 0, fakeOffscreenBuffer2);
        showScene(sceneId1);
        showScene(sceneId2);

        expectSceneRendered(sceneId1, fakeOffscreenBuffer1, EDiscardDepth::Allowed);
        expectSceneRendered(sceneId2, fakeOffscreenBuffer2, EDiscardDepth::Allowed);
        expectFrameBufferRendered();

        expectSwapBuffers();
        doOneRendererLoop();

        hideScene(sceneId1);
        hideScene(sceneId2);
        unassignScene(sceneId1);
        unassignScene(sceneId2);
    }

    TEST_P(ARenderer, assignSceneToFramebufferFromPreviouslyAssignedToOffscreenBuffer)
    {
        createDisplayController();

        const SceneId sceneId(12u);
        createScene(sceneId);

        const DeviceResourceHandle fakeOffscreenBuffer(313u);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer, 1u, 2u, false);
        assignSceneToDisplayBuffer(sceneId, 0, fakeOffscreenBuffer);
        showScene(sceneId);

        expectSceneRendered(sceneId, fakeOffscreenBuffer, EDiscardDepth::Allowed);
        expectFrameBufferRendered();
        expectSwapBuffers();
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        hideScene(sceneId);
        unassignScene(sceneId);
        expectOffscreenBufferCleared(fakeOffscreenBuffer); // will also clear OB after scene is removed from it
        expectFrameBufferRendered(false);
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        const DeviceResourceHandle framebuffer = DisplayControllerMock::FakeFrameBufferHandle;
        assignSceneToDisplayBuffer(sceneId, 0, framebuffer);
        showScene(sceneId);
        EXPECT_EQ(framebuffer, renderer.getBufferSceneIsAssignedTo(sceneId));
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSceneRendered(sceneId);
        expectSwapBuffers();
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        hideScene(sceneId);
        unassignScene(sceneId);
    }

    TEST_P(ARenderer, rendersScenesInOrderAccordingToLocalOrderWithinDisplayBuffer)
    {
        createDisplayController();

        const DeviceResourceHandle fakeOffscreenBuffer1(313u);
        const DeviceResourceHandle fakeOffscreenBuffer2(314u);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer1, 1u, 2u, false);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer2, 1u, 2u, false);

        const SceneId sceneId1(12u);
        const SceneId sceneId2(13u);
        const SceneId sceneId3(14u);
        const SceneId sceneId4(15u);
        createScene(sceneId1);
        createScene(sceneId2);
        createScene(sceneId3);
        createScene(sceneId4);
        assignSceneToDisplayBuffer(sceneId1, 0, fakeOffscreenBuffer1);
        assignSceneToDisplayBuffer(sceneId2, 1, fakeOffscreenBuffer2);
        assignSceneToDisplayBuffer(sceneId3, 2, fakeOffscreenBuffer1);
        assignSceneToDisplayBuffer(sceneId4, 3, fakeOffscreenBuffer2);
        showScene(sceneId1);
        showScene(sceneId2);
        showScene(sceneId3);
        showScene(sceneId4);

        {
            InSequence s;
            expectSceneRendered(sceneId1, fakeOffscreenBuffer1, EDiscardDepth::Disallowed);
            expectSceneRendered(sceneId3, fakeOffscreenBuffer1, EDiscardDepth::Allowed);
        }
        {
            InSequence s;
            expectSceneRendered(sceneId2, fakeOffscreenBuffer2, EDiscardDepth::Disallowed);
            expectSceneRendered(sceneId4, fakeOffscreenBuffer2, EDiscardDepth::Allowed);
        }
        expectFrameBufferRendered();
        expectSwapBuffers();
        doOneRendererLoop();

        hideScene(sceneId1);
        hideScene(sceneId2);
        hideScene(sceneId3);
        hideScene(sceneId4);
        unassignScene(sceneId1);
        unassignScene(sceneId2);
        unassignScene(sceneId3);
        unassignScene(sceneId4);
    }

    TEST_P(ARenderer, confidence_assignsSceneToOffscreenBufferAndReassignsToFramebuffer)
    {
        createDisplayController();

        const SceneId sceneId(12u);
        createScene(sceneId);

        const DeviceResourceHandle fakeOffscreenBuffer(313u);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer, 1u, 1u, false);
        assignSceneToDisplayBuffer(sceneId, 0, fakeOffscreenBuffer);
        EXPECT_EQ(fakeOffscreenBuffer, renderer.getBufferSceneIsAssignedTo(sceneId));
        EXPECT_FALSE(renderer.isSceneAssignedToInterruptibleOffscreenBuffer(sceneId));

        const DeviceResourceHandle framebuffer = DisplayControllerMock::FakeFrameBufferHandle;
        assignSceneToDisplayBuffer(sceneId, 0, framebuffer);
        EXPECT_EQ(framebuffer, renderer.getBufferSceneIsAssignedTo(sceneId));
        EXPECT_FALSE(renderer.isSceneAssignedToInterruptibleOffscreenBuffer(sceneId));

        unassignScene(sceneId);
    }

    TEST_P(ARenderer, returnsInvalidDisplayWhenQueryingLocationOfUnmappedScene)
    {
        createDisplayController();

        const SceneId sceneId(12u);
        createScene(sceneId);
        assignSceneToDisplayBuffer(sceneId, 0);
        unassignScene(sceneId);
        EXPECT_FALSE(renderer.getBufferSceneIsAssignedTo(sceneId).isValid());
    }

    TEST_P(ARenderer, doesNotRenderAMappedScene)
    {
        createDisplayController();

        const SceneId sceneId(12u);
        createScene(sceneId);
        assignSceneToDisplayBuffer(sceneId, 0);

        expectFrameBufferRendered();
        expectSwapBuffers();
        doOneRendererLoop();

        unassignScene(sceneId);
    }

    TEST_P(ARenderer, rendersMappedAndShownScene)
    {
        createDisplayController();

        const SceneId sceneId(12u);
        createScene(sceneId);
        assignSceneToDisplayBuffer(sceneId, 0);
        showScene(sceneId);

        expectSceneRendered(sceneId);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        hideScene(sceneId);
        unassignScene(sceneId);
    }

    TEST_P(ARenderer, rendersTwoMappedAndShownScenes)
    {
        createDisplayController();

        const SceneId sceneId1(12u);
        createScene(sceneId1);
        assignSceneToDisplayBuffer(sceneId1, 0);
        showScene(sceneId1);

        const SceneId sceneId2(13u);
        createScene(sceneId2);
        assignSceneToDisplayBuffer(sceneId2, 0);
        showScene(sceneId2);

        expectSceneRendered(sceneId1);
        expectSceneRendered(sceneId2);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        hideScene(sceneId1);
        hideScene(sceneId2);
        unassignScene(sceneId1);
        unassignScene(sceneId2);
    }

    TEST_P(ARenderer, rendersTwoMappedAndShownScenesWithAscendingRenderOrder)
    {
        createDisplayController();

        const SceneId sceneId1(12u);
        createScene(sceneId1);
        assignSceneToDisplayBuffer(sceneId1, 1);
        showScene(sceneId1);

        const SceneId sceneId2(13u);
        createScene(sceneId2);
        assignSceneToDisplayBuffer(sceneId2, 2);
        showScene(sceneId2);

        {
            InSequence seq;
            expectSceneRendered(sceneId1);
            expectSceneRendered(sceneId2);
        }
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        hideScene(sceneId1);
        hideScene(sceneId2);
        unassignScene(sceneId1);
        unassignScene(sceneId2);
    }

    TEST_P(ARenderer, rendersTwoMappedAndShownScenesWithDescendingRenderOrder)
    {
        createDisplayController();

        const SceneId sceneId1(12u);
        createScene(sceneId1);
        assignSceneToDisplayBuffer(sceneId1, 2);
        showScene(sceneId1);


        const SceneId sceneId2(13u);
        createScene(sceneId2);
        assignSceneToDisplayBuffer(sceneId2, 1);
        showScene(sceneId2);

        {
            InSequence seq;
            expectSceneRendered(sceneId2);
            expectSceneRendered(sceneId1);
        }
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        hideScene(sceneId1);
        hideScene(sceneId2);
        unassignScene(sceneId1);
        unassignScene(sceneId2);
    }

    TEST_P(ARenderer, doesNotRenderSceneThatWasNotMapped)
    {
        createDisplayController();
        createScene();
        expectFrameBufferRendered();
        expectSwapBuffers();
        doOneRendererLoop();
    }

    TEST_P(ARenderer, doesNotRenderUnmappedScene)
    {
        createDisplayController();

        const SceneId sceneId(12u);
        createScene(sceneId);
        assignSceneToDisplayBuffer(sceneId, 0);
        unassignScene(sceneId);

        expectFrameBufferRendered();
        expectSwapBuffers();
        doOneRendererLoop();
    }

    TEST_P(ARenderer, skipsFrameIfDisplayControllerCanNotRenderNewFrame)
    {
        createDisplayController();

        EXPECT_CALL(*renderer.m_displayController, handleWindowEvents());
        //mock that disp controller can not render new frame by returning false
        EXPECT_CALL(*renderer.m_displayController, canRenderNewFrame()).WillRepeatedly(Return(false));

        //renderer must not try to render on that display
        EXPECT_CALL(*renderer.m_displayController, swapBuffers()).Times(0);
        EXPECT_CALL(*renderer.m_displayController, getEmbeddedCompositingManager()).Times(0);
        EXPECT_CALL(renderer.m_embeddedCompositingManager, notifyClients()).Times(0);

        renderer.doOneRenderLoop();
    }

    TEST_P(ARenderer, canTakeASingleScreenshot_Framebuffer)
    {
        createDisplayController();

        scheduleScreenshot(DisplayControllerMock::FakeFrameBufferHandle, 20u, 30u, 100u, 100u);

        expectDisplayControllerReadPixels(DisplayControllerMock::FakeFrameBufferHandle, 20u, 30u, 100u, 100u);
        expectFrameBufferRendered();
        expectSwapBuffers();
        doOneRendererLoop();

        auto screenshots = renderer.dispatchProcessedScreenshots();
        ASSERT_EQ(1u, screenshots.size());
        EXPECT_EQ(DisplayControllerMock::FakeFrameBufferHandle, screenshots.begin()->first);

        // check that screenshot request got deleted
        EXPECT_CALL(*renderer.m_displayController, readPixels(_, _, _, _, _, _)).Times(0);
        expectFrameBufferRendered(false);
        doOneRendererLoop();

        screenshots = renderer.dispatchProcessedScreenshots();
        EXPECT_EQ(0u, screenshots.size());
    }

    TEST_P(ARenderer, canTakeASingleScreenshot_Offscreenbuffer)
    {
        createDisplayController();
        const DeviceResourceHandle obDeviceHandle{ 567u };
        renderer.registerOffscreenBuffer(obDeviceHandle, 10u, 20u, false);

        scheduleScreenshot(obDeviceHandle, 1u, 2u, 3u, 4u);

        expectDisplayControllerReadPixels(obDeviceHandle, 1u, 2u, 3u, 4u);
        expectOffscreenBufferCleared(obDeviceHandle);
        expectFrameBufferRendered();
        expectSwapBuffers();
        doOneRendererLoop();

        auto screenshots = renderer.dispatchProcessedScreenshots();
        ASSERT_EQ(1u, screenshots.size());
        EXPECT_EQ(obDeviceHandle, screenshots.begin()->first);

        // check that screenshot request got deleted
        EXPECT_CALL(*renderer.m_displayController, readPixels(_, _, _, _, _, _)).Times(0);
        expectFrameBufferRendered(false);
        doOneRendererLoop();

        screenshots = renderer.dispatchProcessedScreenshots();
        EXPECT_EQ(0u, screenshots.size());
    }

    TEST_P(ARenderer, canTakeASingleScreenshot_InterruptibleOffscreenbuffer)
    {
        createDisplayController();
        const DeviceResourceHandle obDeviceHandle{ 567u };
        renderer.registerOffscreenBuffer(obDeviceHandle, 10u, 20u, true);

        scheduleScreenshot(obDeviceHandle, 1u, 2u, 3u, 4u);

        expectDisplayControllerReadPixels(obDeviceHandle, 1u, 2u, 3u, 4u);
        expectFrameBufferRendered();
        expectOffscreenBufferCleared(obDeviceHandle);
        expectInterruptibleOffscreenBufferSwapped(obDeviceHandle);
        expectSwapBuffers();
        doOneRendererLoop();

        auto screenshots = renderer.dispatchProcessedScreenshots();
        ASSERT_EQ(1u, screenshots.size());
        EXPECT_EQ(obDeviceHandle, screenshots.begin()->first);

        // check that screenshot request got deleted
        EXPECT_CALL(*renderer.m_displayController, readPixels(_, _, _, _, _, _)).Times(0);
        expectFrameBufferRendered();
        expectSwapBuffers();
        doOneRendererLoop();

        screenshots = renderer.dispatchProcessedScreenshots();
        EXPECT_EQ(0u, screenshots.size());
    }

    TEST_P(ARenderer, takeMultipleScreenshotsOfADisplayOverritesPreviousScreenshot)
    {
        createDisplayController();

        scheduleScreenshot(DisplayControllerMock::FakeFrameBufferHandle, 10u, 10u, 110u, 110u);
        scheduleScreenshot(DisplayControllerMock::FakeFrameBufferHandle, 30u, 30u, 130u, 130u);

        expectDisplayControllerReadPixels(DisplayControllerMock::FakeFrameBufferHandle, 30u, 30u, 130u, 130u);
        expectFrameBufferRendered();
        expectSwapBuffers();
        doOneRendererLoop();

        auto screenshots1 = renderer.dispatchProcessedScreenshots();
        ASSERT_EQ(1u, screenshots1.size());
        const auto& screenshots1FB = std::find_if(std::cbegin(screenshots1), std::cend(screenshots1), [&](const auto& p) {return p.first == DisplayControllerMock::FakeFrameBufferHandle; });
        ASSERT_NE(screenshots1.cend(), screenshots1FB);

        // check that screenshot request got deleted
        EXPECT_CALL(*renderer.m_displayController, readPixels(_, _, _, _, _, _)).Times(0);
        expectFrameBufferRendered(false);
        doOneRendererLoop();

        screenshots1 = renderer.dispatchProcessedScreenshots();
        ASSERT_EQ(0u, screenshots1.size());
    }

    TEST_P(ARenderer, marksRenderOncePassesAsRenderedAfterRenderingScene)
    {
        createDisplayController();

        const SceneId sceneId(12u);
        createScene(sceneId);
        assignSceneToDisplayBuffer(sceneId, 0);
        showScene(sceneId);

        auto& scene = rendererScenes.getScene(sceneId);
        TestSceneHelper sceneHelper(scene);
        const auto dataLayout = sceneHelper.m_sceneAllocator.allocateDataLayout({ DataFieldInfo{ramses::internal::EDataType::Vector2I}, DataFieldInfo{ramses::internal::EDataType::Vector2I} }, ResourceContentHash::Invalid());
        const CameraHandle camera = sceneHelper.m_sceneAllocator.allocateCamera(ECameraProjectionType::Orthographic, sceneHelper.m_sceneAllocator.allocateNode(), sceneHelper.m_sceneAllocator.allocateDataInstance(dataLayout));
        const RenderPassHandle pass = sceneHelper.m_sceneAllocator.allocateRenderPass();
        scene.setRenderPassCamera(pass, camera);

        // render
        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager);
        expectSceneRendered(sceneId);
        expectFrameBufferRendered(true, EClearFlag::None);
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
        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager);
        expectSceneRendered(sceneId);
        renderer.markBufferWithSceneForRerender(sceneId);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        // pass in list means it was rendered
        ASSERT_EQ(1u, passesToRender.size());
        EXPECT_EQ(pass, passesToRender[0].getRenderPassHandle());

        // render
        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager);
        expectSceneRendered(sceneId);
        renderer.markBufferWithSceneForRerender(sceneId);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        // pass not in list means it was not rendered anymore
        EXPECT_TRUE(passesToRender.empty());

        hideScene(sceneId);
        unassignScene(sceneId);
    }

    TEST_P(ARenderer, doesNotClearAndRerenderIfNoChangeToScene)
    {
        createDisplayController();

        const SceneId sceneId(12u);
        createScene(sceneId);
        assignSceneToDisplayBuffer(sceneId, 0);
        showScene(sceneId);

        expectSceneRendered(sceneId);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        // no further expectations for scene render or clear
        expectFrameBufferRendered(false);
        doOneRendererLoop();
        expectFrameBufferRendered(false);
        doOneRendererLoop();

        hideScene(sceneId);
        unassignScene(sceneId);
    }

    TEST_P(ARenderer, doesNotClearAndRerenderOffscreenBufferIfNoChangeToScene)
    {
        createDisplayController();

        const SceneId sceneId(12u);
        createScene(sceneId);

        const DeviceResourceHandle fakeOffscreenBuffer(313u);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer, 1u, 2u, false);
        assignSceneToDisplayBuffer(sceneId, 0, fakeOffscreenBuffer);
        showScene(sceneId);

        expectSceneRendered(sceneId, fakeOffscreenBuffer, EDiscardDepth::Allowed);
        expectFrameBufferRendered();
        expectSwapBuffers();
        doOneRendererLoop();

        // no further expectations for scene render or clear
        expectFrameBufferRendered(false);
        doOneRendererLoop();
        expectFrameBufferRendered(false);
        doOneRendererLoop();

        hideScene(sceneId);
        unassignScene(sceneId);
    }

    TEST_P(ARenderer, clearAndRerenderIfSceneMarkedAsChanged)
    {
        createDisplayController();

        const SceneId sceneId(12u);
        createScene(sceneId);
        assignSceneToDisplayBuffer(sceneId, 0);
        showScene(sceneId);

        expectSceneRendered(sceneId);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        // no change
        expectFrameBufferRendered(false);
        doOneRendererLoop();
        // mark change
        renderer.markBufferWithSceneForRerender(sceneId);
        expectSceneRendered(sceneId);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();
        // no change
        expectFrameBufferRendered(false);
        doOneRendererLoop();
        // mark change
        renderer.markBufferWithSceneForRerender(sceneId);
        expectSceneRendered(sceneId);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        hideScene(sceneId);
        unassignScene(sceneId);
    }

    TEST_P(ARenderer, clearAndRerenderOffscreenBufferIfSceneMarkedAsChanged)
    {
        createDisplayController();

        const SceneId sceneId(12u);
        createScene(sceneId);

        const DeviceResourceHandle fakeOffscreenBuffer(313u);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer, 1u, 2u, false);
        assignSceneToDisplayBuffer(sceneId, 0, fakeOffscreenBuffer);
        showScene(sceneId);

        expectSceneRendered(sceneId, fakeOffscreenBuffer, EDiscardDepth::Allowed);
        expectFrameBufferRendered();
        expectSwapBuffers();
        doOneRendererLoop();

        // no change
        expectFrameBufferRendered(false);
        doOneRendererLoop();
        // mark change
        renderer.markBufferWithSceneForRerender(sceneId);
        expectSceneRendered(sceneId, fakeOffscreenBuffer, EDiscardDepth::Allowed);
        expectFrameBufferRendered(false);
        doOneRendererLoop(); // framebuffer not cleared/swapped as there is no scene assigned
        // no change
        expectFrameBufferRendered(false);
        doOneRendererLoop();
        // mark change
        renderer.markBufferWithSceneForRerender(sceneId);
        expectSceneRendered(sceneId, fakeOffscreenBuffer, EDiscardDepth::Allowed);
        expectFrameBufferRendered(false);
        doOneRendererLoop(); // framebuffer not cleared/swapped as there is no scene assigned

        hideScene(sceneId);
        unassignScene(sceneId);
    }

    TEST_P(ARenderer, clearAndRerenderBothFramebufferAndOffscreenBufferIfSceneAssignedFromOneToTheOther)
    {
        createDisplayController();

        const SceneId sceneId(12u);
        createScene(sceneId);

        const DeviceResourceHandle fakeOffscreenBuffer(313u);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer, 1u, 2u, false);
        assignSceneToDisplayBuffer(sceneId, 0, fakeOffscreenBuffer);
        showScene(sceneId);

        expectSceneRendered(sceneId, fakeOffscreenBuffer, EDiscardDepth::Allowed);
        expectFrameBufferRendered();
        expectSwapBuffers();
        doOneRendererLoop();

        // no change
        expectFrameBufferRendered(false);
        doOneRendererLoop();
        expectFrameBufferRendered(false);
        doOneRendererLoop();

        // assign back to FB causes clear/render of both buffers
        renderer.assignSceneToDisplayBuffer(sceneId, DisplayControllerMock::FakeFrameBufferHandle, 0);
        expectOffscreenBufferCleared(fakeOffscreenBuffer);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSceneRendered(sceneId, DisplayControllerMock::FakeFrameBufferHandle);
        expectSwapBuffers();
        doOneRendererLoop();

        // no change
        expectFrameBufferRendered(false);
        doOneRendererLoop();

        // assign back to offscreen buffer causes clear/render of both buffers
        renderer.assignSceneToDisplayBuffer(sceneId, fakeOffscreenBuffer, 0);
        expectSceneRendered(sceneId, fakeOffscreenBuffer, EDiscardDepth::Allowed);
        expectFrameBufferRendered(true); // framebuffer cleared/swapped
        expectSwapBuffers();
        doOneRendererLoop();

        hideScene(sceneId);
        unassignScene(sceneId);
    }

    TEST_P(ARenderer, clearAndRerenderBufferIfSceneHidden)
    {
        createDisplayController();

        const SceneId sceneId(12u);
        createScene(sceneId);
        assignSceneToDisplayBuffer(sceneId, 0);
        showScene(sceneId);

        expectSceneRendered(sceneId);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        // no change
        expectFrameBufferRendered(false);
        doOneRendererLoop();

        // hiding scene causes clear of FB but no render as scene is hidden
        hideScene(sceneId);
        expectFrameBufferRendered(true);
        expectSwapBuffers();
        doOneRendererLoop();

        // no change
        expectFrameBufferRendered(false);
        doOneRendererLoop();

        unassignScene(sceneId);
    }

    TEST_P(ARenderer, clearAndRerenderBufferIfClearColorChanged)
    {
        createDisplayController();

        const SceneId sceneId(12u);
        createScene(sceneId);
        assignSceneToDisplayBuffer(sceneId, 0);
        showScene(sceneId);

        expectSceneRendered(sceneId);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        // no change
        expectFrameBufferRendered(false);
        doOneRendererLoop();

        // setting clear color causes clear/render
        renderer.setClearColor(DisplayControllerMock::FakeFrameBufferHandle, { 1, 2, 3, 4 });
        expectSceneRendered(sceneId, DisplayControllerMock::FakeFrameBufferHandle, EClearFlag::All, { 1, 2, 3, 4 });
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        // no change
        expectFrameBufferRendered(false);
        doOneRendererLoop();

        hideScene(sceneId);
        unassignScene(sceneId);
    }

    TEST_P(ARenderer, clearAndRerenderBothFramebufferAndOffscreenBufferIfOBClearColorChanges)
    {
        createDisplayController();

        const SceneId sceneId(12u);
        createScene(sceneId);

        const glm::vec4 obClearColor1(.1f, .2f, .3f, .4f);
        const DeviceResourceHandle fakeOffscreenBuffer(313u);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer, 1u, 2u, false);
        renderer.setClearColor(fakeOffscreenBuffer, obClearColor1);
        assignSceneToDisplayBuffer(sceneId, 0, fakeOffscreenBuffer);
        showScene(sceneId);

        expectSceneRenderedExt(sceneId, fakeOffscreenBuffer, EClearFlag::All, obClearColor1, {}, {}, EClearFlag::All, EDiscardDepth::Allowed);
        expectFrameBufferRendered();
        expectSwapBuffers();
        doOneRendererLoop();

        // no change
        expectFrameBufferRendered(false);
        doOneRendererLoop();
        expectFrameBufferRendered(false);
        doOneRendererLoop();

        // change clear color
        const glm::vec4 obClearColor2(.2f, .3f, .4f, .5f);
        renderer.setClearColor(fakeOffscreenBuffer, obClearColor2);
        expectFrameBufferRendered(true);
        expectSceneRenderedExt(sceneId, fakeOffscreenBuffer, EClearFlag::All, obClearColor2, {}, {}, EClearFlag::All, EDiscardDepth::Allowed);
        expectSwapBuffers();
        doOneRendererLoop();

        // no change
        expectFrameBufferRendered(false);
        doOneRendererLoop();

        hideScene(sceneId);
        unassignScene(sceneId);
    }

    TEST_P(ARenderer, clearAndRerenderBuffersIfExternallyOwnedWindowResized)
    {
        createDisplayController();

        const SceneId sceneId(12u);
        createScene(sceneId);
        assignSceneToDisplayBuffer(sceneId, 0);
        showScene(sceneId);

        expectSceneRendered(sceneId);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        // no change
        expectFrameBufferRendered(false);
        doOneRendererLoop();

        // external resizing causes clear/render
        EXPECT_CALL(renderer.m_platform.renderBackendMock.windowMock, setExternallyOwnedWindowSize(1u, 2u)).WillOnce(Return(true));
        renderer.setExternallyOwnedWindowSize(1u, 2u);
        expectSceneRendered(sceneId);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        // no change
        expectFrameBufferRendered(false);
        doOneRendererLoop();

        hideScene(sceneId);
        unassignScene(sceneId);
    }

    TEST_P(ARenderer, doesNotClearAndRerenderBuffersIfExternallyOwnedWindowResizeFails)
    {
        createDisplayController();

        const SceneId sceneId(12u);
        createScene(sceneId);
        assignSceneToDisplayBuffer(sceneId, 0);
        showScene(sceneId);

        expectSceneRendered(sceneId);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        // no change
        expectFrameBufferRendered(false);
        doOneRendererLoop();

        // failed external resizing causes clear/render
        EXPECT_CALL(renderer.m_platform.renderBackendMock.windowMock, setExternallyOwnedWindowSize(1u, 2u)).WillOnce(Return(false));
        renderer.setExternallyOwnedWindowSize(1u, 2u);

        // no change
        expectFrameBufferRendered(false);
        doOneRendererLoop();

        hideScene(sceneId);
        unassignScene(sceneId);
    }

    TEST_P(ARenderer, clearAndSwapInterruptibleOBOnlyOnceIfNoMoreShownScenes)
    {
        createDisplayController();

        const DeviceResourceHandle fakeOffscreenBuffer(313u);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer, 1u, 2u, true);

        const SceneId sceneId(12u);
        createScene(sceneId);
        assignSceneToDisplayBuffer(sceneId, 0, fakeOffscreenBuffer);
        showScene(sceneId);

        expectFrameBufferRendered();
        expectInterruptibleOffscreenBufferSwapped(fakeOffscreenBuffer);
        expectSceneRenderedWithInterruptionEnabled(sceneId, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderBegin, EClearFlag::All);
        expectSwapBuffers();
        doOneRendererLoop();

        // re-render FB to reflect finished interruptible OB in previous frame
        expectFrameBufferRendered();
        expectSwapBuffers();
        doOneRendererLoop();

        // hide scene will trigger re-render and extra clear of OB
        hideScene(sceneId);
        expectFrameBufferRendered(false);
        expectOffscreenBufferCleared(fakeOffscreenBuffer);
        expectInterruptibleOffscreenBufferSwapped(fakeOffscreenBuffer);
        doOneRendererLoop();

        // re-render FB to reflect finished interruptible OB in previous frame
        expectFrameBufferRendered();
        expectSwapBuffers();
        doOneRendererLoop();

        // no change
        expectFrameBufferRendered(false);
        doOneRendererLoop();
        expectFrameBufferRendered(false);
        doOneRendererLoop();

        unassignScene(sceneId);
    }

    TEST_P(ARenderer, clearAndSwapInterruptibleOBOnlyOnceIfNoMoreMappedScenes)
    {
        createDisplayController();

        const DeviceResourceHandle fakeOffscreenBuffer(313u);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer, 1u, 2u, true);

        const SceneId sceneId(12u);
        createScene(sceneId);
        assignSceneToDisplayBuffer(sceneId, 0, fakeOffscreenBuffer);
        showScene(sceneId);

        expectFrameBufferRendered();
        expectInterruptibleOffscreenBufferSwapped(fakeOffscreenBuffer);
        expectSceneRenderedWithInterruptionEnabled(sceneId, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderBegin);
        expectSwapBuffers();
        doOneRendererLoop();

        // re-render FB to reflect finished interruptible OB in previous frame
        expectFrameBufferRendered();
        expectSwapBuffers();
        doOneRendererLoop();

        // hide scene will trigger re-render of OB
        hideScene(sceneId);
        unassignScene(sceneId);
        expectFrameBufferRendered(false);
        expectOffscreenBufferCleared(fakeOffscreenBuffer);
        expectInterruptibleOffscreenBufferSwapped(fakeOffscreenBuffer);
        doOneRendererLoop();

        // re-render FB to reflect finished interruptible OB in previous frame
        expectFrameBufferRendered();
        expectSwapBuffers();
        doOneRendererLoop();

        // no change
        expectFrameBufferRendered(false);
        doOneRendererLoop();
        expectFrameBufferRendered(false);
        doOneRendererLoop();
    }

    TEST_P(ARenderer, rendersScenesToFBAndOBWithNoInterruption)
    {
        createDisplayController();
        const DeviceResourceHandle fakeOffscreenBuffer(313u);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer, 1u, 1u, true);

        const SceneId sceneIdFB(12u);
        const SceneId sceneIdOB(13u);
        createScene(sceneIdFB);
        createScene(sceneIdOB);
        assignSceneToDisplayBuffer(sceneIdFB, 0);
        assignSceneToDisplayBuffer(sceneIdOB, 0, fakeOffscreenBuffer);

        showScene(sceneIdFB);
        showScene(sceneIdOB);

        expectFrameBufferRendered(true, EClearFlag::None);
        expectSceneRendered(sceneIdFB);
        expectInterruptibleOffscreenBufferSwapped(fakeOffscreenBuffer);
        expectSceneRenderedWithInterruptionEnabled(sceneIdOB, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderBegin);
        expectSwapBuffers();
        doOneRendererLoop();

        hideScene(sceneIdOB);
        hideScene(sceneIdFB);
        unassignScene(sceneIdOB);
        unassignScene(sceneIdFB);
    }

    TEST_P(ARenderer, doesNotSwapOffscreenBufferIfRenderingIntoItInterrupted)
    {
        createDisplayController();
        const DeviceResourceHandle fakeOffscreenBuffer(313u);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer, 1u, 1u, true);

        const SceneId sceneIdOB(13u);
        createScene(sceneIdOB);
        assignSceneToDisplayBuffer(sceneIdOB, 0, fakeOffscreenBuffer);
        showScene(sceneIdOB);

        expectSceneRenderedWithInterruptionEnabled(sceneIdOB, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderInterrupted, EClearFlag::All); // expect clear

        expectFrameBufferRendered();
        // expect no OB swap due to interruption
        expectSwapBuffers();
        doOneRendererLoop();

        renderer.resetRenderInterruptState();
        hideScene(sceneIdOB);
        unassignScene(sceneIdOB);
    }

    TEST_P(ARenderer, doesNotClearAndPassesPreviousStateWhenInterruptedAndSwapsAfterFinishing)
    {
        createDisplayController();
        const DeviceResourceHandle fakeOffscreenBuffer(313u);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer, 1u, 1u, true);

        const SceneId sceneIdOB(13u);
        createScene(sceneIdOB);
        assignSceneToDisplayBuffer(sceneIdOB, 0, fakeOffscreenBuffer);
        showScene(sceneIdOB);

        // start rendering and interrupt
        expectSceneRenderedWithInterruptionEnabled(sceneIdOB, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderInterrupted, EClearFlag::All); // expect clear
        expectFrameBufferRendered();
        // expect no OB swap due to interruption
        expectSwapBuffers();
        doOneRendererLoop();
        EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());
        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        // continue from interruption point and interrupt again
        expectSceneRenderedWithInterruptionEnabled(sceneIdOB, fakeOffscreenBuffer, sceneRenderInterrupted, sceneRenderInterrupted2, EClearFlag::None); // no clear due to previous interruption
        expectFrameBufferRendered(false);
        // no OB swap due to interruption again
        doOneRendererLoop();
        EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());
        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        // continue from interruption point and finish
        expectSceneRenderedWithInterruptionEnabled(sceneIdOB, fakeOffscreenBuffer, sceneRenderInterrupted2, sceneRenderBegin, EClearFlag::None); // no clear due to previous interruption
        expectFrameBufferRendered(false);
        expectInterruptibleOffscreenBufferSwapped(fakeOffscreenBuffer); // swap after finish
        doOneRendererLoop();
        EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());
        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        // re-render FB to reflect change happened to OB in previous frame
        expectFrameBufferRendered();
        expectSwapBuffers();
        doOneRendererLoop();
        EXPECT_FALSE(renderer.hasAnyBufferWithInterruptedRendering());
        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        hideScene(sceneIdOB);
        unassignScene(sceneIdOB);
    }

    TEST_P(ARenderer, rendersScenesToFBEvenIfOBInterrupted)
    {
        createDisplayController();
        const DeviceResourceHandle fakeOffscreenBuffer(313u);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer, 1u, 1u, true);

        const SceneId sceneIdFB(12u);
        const SceneId sceneIdOB(13u);
        createScene(sceneIdFB);
        createScene(sceneIdOB);
        assignSceneToDisplayBuffer(sceneIdFB, 0);
        assignSceneToDisplayBuffer(sceneIdOB, 0, fakeOffscreenBuffer);

        showScene(sceneIdFB);
        showScene(sceneIdOB);

        expectSceneRendered(sceneIdFB);
        expectSceneRenderedWithInterruptionEnabled(sceneIdOB, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderInterrupted);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        expectSceneRenderedWithInterruptionEnabled(sceneIdOB, fakeOffscreenBuffer, sceneRenderInterrupted, sceneRenderBegin, EClearFlag::None);
        expectFrameBufferRendered(false, EClearFlag::None);
        expectInterruptibleOffscreenBufferSwapped(fakeOffscreenBuffer);
        doOneRendererLoop();

        // re-render FB to reflect change happened to OB in previous frame
        expectSceneRendered(sceneIdFB);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        hideScene(sceneIdOB);
        hideScene(sceneIdFB);
        unassignScene(sceneIdOB);
        unassignScene(sceneIdFB);
    }

    TEST_P(ARenderer, alwaysRendersScenesToFBWhenModifiedEvenIfOBInterrupted)
    {
        createDisplayController();
        const DeviceResourceHandle fakeOffscreenBuffer(313u);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer, 1u, 1u, true);

        const SceneId sceneIdFB(12u);
        const SceneId sceneIdOB(13u);
        createScene(sceneIdFB);
        createScene(sceneIdOB);
        assignSceneToDisplayBuffer(sceneIdFB, 0);
        assignSceneToDisplayBuffer(sceneIdOB, 0, fakeOffscreenBuffer);

        showScene(sceneIdFB);
        showScene(sceneIdOB);

        //FB rendered, OB interrupted
        expectSceneRendered(sceneIdFB);
        expectSceneRenderedWithInterruptionEnabled(sceneIdOB, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderInterrupted);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        //modify FB scene
        renderer.markBufferWithSceneForRerender(sceneIdFB);
        //FB rendered, OB interrupted
        expectSceneRendered(sceneIdFB);
        expectSceneRenderedWithInterruptionEnabled(sceneIdOB, fakeOffscreenBuffer, sceneRenderInterrupted, sceneRenderInterrupted2, EClearFlag::None);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        //modify FB scene
        renderer.markBufferWithSceneForRerender(sceneIdFB);
        //FB rendered, OB finished
        expectSceneRendered(sceneIdFB);
        expectSceneRenderedWithInterruptionEnabled(sceneIdOB, fakeOffscreenBuffer, sceneRenderInterrupted2, sceneRenderBegin, EClearFlag::None);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectInterruptibleOffscreenBufferSwapped(fakeOffscreenBuffer);
        expectSwapBuffers();
        doOneRendererLoop();

        // re-render FB to reflect change happened to OB in previous frame
        expectSceneRendered(sceneIdFB);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        hideScene(sceneIdOB);
        hideScene(sceneIdFB);
        unassignScene(sceneIdOB);
        unassignScene(sceneIdFB);
    }

    TEST_P(ARenderer, doesNotSkipFramesTillAllInterruptionsFinishedAndRendered)
    {
        // 1 FB, 1 OB, 1 scene per OB

        createDisplayController();
        const DeviceResourceHandle fakeOffscreenBuffer(313u);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer, 1u, 1u, true);

        const SceneId sceneIdFB(12u);
        const SceneId sceneIdOB(13u);
        createScene(sceneIdFB);
        createScene(sceneIdOB);
        assignSceneToDisplayBuffer(sceneIdFB, 0);
        assignSceneToDisplayBuffer(sceneIdOB, 0, fakeOffscreenBuffer);

        showScene(sceneIdFB);
        showScene(sceneIdOB);

        // FB rendered, OB interrupted
        expectSceneRendered(sceneIdFB);
        expectSceneRenderedWithInterruptionEnabled(sceneIdOB, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderInterrupted);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        // FB skipped, OB finished
        expectSceneRenderedWithInterruptionEnabled(sceneIdOB, fakeOffscreenBuffer, sceneRenderInterrupted, sceneRenderBegin, EClearFlag::None);
        expectFrameBufferRendered(false);
        expectInterruptibleOffscreenBufferSwapped(fakeOffscreenBuffer);
        doOneRendererLoop();

        // re-render FB to reflect change happened to OB in previous frame
        expectSceneRendered(sceneIdFB);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        // no change -> nothing rendered
        expectFrameBufferRendered(false);
        doOneRendererLoop();

        hideScene(sceneIdOB);
        hideScene(sceneIdFB);
        unassignScene(sceneIdOB);
        unassignScene(sceneIdFB);
    }

    TEST_P(ARenderer, interruptingEveryFrameGetsToAllRenderedState_withMultipleScenes)
    {
        // 1 FB, 1 OB, 2 scenes per OB

        createDisplayController();
        const DeviceResourceHandle fakeOffscreenBuffer(313u);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer, 1u, 1u, true);

        const SceneId sceneIdFB(12u);
        const SceneId sceneId1OB(13u);
        const SceneId sceneId2OB(14u);
        createScene(sceneIdFB);
        createScene(sceneId1OB);
        createScene(sceneId2OB);
        assignSceneToDisplayBuffer(sceneId2OB, 1, fakeOffscreenBuffer);
        assignSceneToDisplayBuffer(sceneId1OB, 0, fakeOffscreenBuffer);
        assignSceneToDisplayBuffer(sceneIdFB, 0);

        showScene(sceneIdFB);
        showScene(sceneId1OB);
        showScene(sceneId2OB);

        // FB rendered, OB scene1 interrupted, OB scene2 skipped
        expectSceneRendered(sceneIdFB);
        expectSceneRenderedWithInterruptionEnabled(sceneId1OB, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderInterrupted);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        // FB skipped, OB scene1 finished, OB scene2 interrupted
        expectSceneRenderedWithInterruptionEnabled(sceneId1OB, fakeOffscreenBuffer, sceneRenderInterrupted, sceneRenderBegin, EClearFlag::None);
        expectSceneRenderedWithInterruptionEnabled(sceneId2OB, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderInterrupted, EClearFlag::None);
        expectFrameBufferRendered(false);
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        // FB skipped (as OB is not done rendering yet), OB scene1 skipped, OB scene2 finished
        expectSceneRenderedWithInterruptionEnabled(sceneId2OB, fakeOffscreenBuffer, sceneRenderInterrupted, sceneRenderBegin, EClearFlag::None);
        expectFrameBufferRendered(false);
        expectInterruptibleOffscreenBufferSwapped(fakeOffscreenBuffer);
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        // re-render FB to reflect change happened to OB scene 1 and scene 2 in previous frames
        expectSceneRendered(sceneIdFB);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        // no change -> nothing rendered
        expectFrameBufferRendered(false);
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        hideScene(sceneId2OB);
        hideScene(sceneId1OB);
        hideScene(sceneIdFB);
        unassignScene(sceneId2OB);
        unassignScene(sceneId1OB);
        unassignScene(sceneIdFB);
    }

    TEST_P(ARenderer, interruptingEveryFrameGetsToAllRenderedState_withMultipleScenes_FirstSceneNeverInterrupted)
    {
        // 1 FB, 1 OB, 2 scenes per OB

        createDisplayController();
        const DeviceResourceHandle fakeOffscreenBuffer(313u);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer, 1u, 1u, true);

        const SceneId sceneIdFB(12u);
        const SceneId sceneId1OB(13u);
        const SceneId sceneId2OB(14u);
        createScene(sceneIdFB);
        createScene(sceneId1OB);
        createScene(sceneId2OB);
        assignSceneToDisplayBuffer(sceneId2OB, 1, fakeOffscreenBuffer);
        assignSceneToDisplayBuffer(sceneId1OB, 0, fakeOffscreenBuffer);
        assignSceneToDisplayBuffer(sceneIdFB, 0);

        showScene(sceneIdFB);
        showScene(sceneId1OB);
        showScene(sceneId2OB);

        // FB rendered, OB scene1 fully rendered, OB scene2 interrupted
        expectSceneRendered(sceneIdFB);
        expectSceneRenderedWithInterruptionEnabled(sceneId1OB, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderBegin);
        expectSceneRenderedWithInterruptionEnabled(sceneId2OB, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderInterrupted);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        // FB skipped, OB scene1 skipped, OB scene2 finished
        expectSceneRenderedWithInterruptionEnabled(sceneId2OB, fakeOffscreenBuffer, sceneRenderInterrupted, sceneRenderBegin, EClearFlag::None);
        expectFrameBufferRendered(false);
        expectInterruptibleOffscreenBufferSwapped(fakeOffscreenBuffer);
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        // re-render FB to reflect change happened to OB scene 1 and scene 2 in previous frames
        expectSceneRendered(sceneIdFB);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        // no change -> nothing rendered
        expectFrameBufferRendered(false);
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        hideScene(sceneId2OB);
        hideScene(sceneId1OB);
        hideScene(sceneIdFB);
        unassignScene(sceneId2OB);
        unassignScene(sceneId1OB);
        unassignScene(sceneIdFB);
    }

    TEST_P(ARenderer, interruptingEveryFrameGetsToAllRenderedState_withMultipleOffscreenBuffers)
    {
        // 1 FB, 2 OB, 1 scene per OB

        createDisplayController();
        const DeviceResourceHandle fakeOffscreenBuffer1(313u);
        const DeviceResourceHandle fakeOffscreenBuffer2(314u);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer1, 1u, 1u, true);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer2, 1u, 1u, true);

        const SceneId sceneIdFB(12u);
        const SceneId sceneIdOB1(13u);
        const SceneId sceneIdOB2(14u);
        createScene(sceneIdFB);
        createScene(sceneIdOB1);
        createScene(sceneIdOB2);
        assignSceneToDisplayBuffer(sceneIdOB2, 0, fakeOffscreenBuffer2);
        assignSceneToDisplayBuffer(sceneIdOB1, 0, fakeOffscreenBuffer1);
        assignSceneToDisplayBuffer(sceneIdFB, 0);

        showScene(sceneIdFB);
        showScene(sceneIdOB1);
        showScene(sceneIdOB2);

        // FB rendered, OB1 interrupted, OB2 skipped
        expectSceneRendered(sceneIdFB);
        expectSceneRenderedWithInterruptionEnabled(sceneIdOB1, fakeOffscreenBuffer1, sceneRenderBegin, sceneRenderInterrupted);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        // FB skipped, OB1 finished, OB2 interrupted
        expectSceneRenderedWithInterruptionEnabled(sceneIdOB1, fakeOffscreenBuffer1, sceneRenderInterrupted, sceneRenderBegin, EClearFlag::None);
        expectSceneRenderedWithInterruptionEnabled(sceneIdOB2, fakeOffscreenBuffer2, sceneRenderBegin, sceneRenderInterrupted);
        expectFrameBufferRendered(false);
        expectInterruptibleOffscreenBufferSwapped(fakeOffscreenBuffer1);
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        // FB re-rendered to reflect changed in OB1, OB1 skipped, OB2 finished
        expectSceneRendered(sceneIdFB);
        expectSceneRenderedWithInterruptionEnabled(sceneIdOB2, fakeOffscreenBuffer2, sceneRenderInterrupted, sceneRenderBegin, EClearFlag::None);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectInterruptibleOffscreenBufferSwapped(fakeOffscreenBuffer2);
        expectSwapBuffers();
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        // re-render FB to reflect change happened to OB2 in previous frame
        expectSceneRendered(sceneIdFB);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        // no change -> nothing rendered
        expectFrameBufferRendered(false);
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        hideScene(sceneIdOB2);
        hideScene(sceneIdOB1);
        hideScene(sceneIdFB);
        unassignScene(sceneIdOB2);
        unassignScene(sceneIdOB1);
        unassignScene(sceneIdFB);
    }

    TEST_P(ARenderer, interruptingEveryFrameGetsToAllRenderedState_withMultipleOffscreenBuffersAndScenes)
    {
        // 1 FB, 2 OB, 2 scene per OB

        createDisplayController();
        const DeviceResourceHandle fakeOffscreenBuffer1(313u);
        const DeviceResourceHandle fakeOffscreenBuffer2(314u);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer1, 1u, 1u, true);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer2, 1u, 1u, true);

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
        assignSceneToDisplayBuffer(sceneId1OB2, 0, fakeOffscreenBuffer2);
        assignSceneToDisplayBuffer(sceneId2OB2, 1, fakeOffscreenBuffer2);
        assignSceneToDisplayBuffer(sceneId1OB1, 0, fakeOffscreenBuffer1);
        assignSceneToDisplayBuffer(sceneId2OB1, 1, fakeOffscreenBuffer1);
        assignSceneToDisplayBuffer(sceneIdFB, 0);

        showScene(sceneIdFB);
        showScene(sceneId1OB1);
        showScene(sceneId2OB1);
        showScene(sceneId1OB2);
        showScene(sceneId2OB2);

        // FB rendered, OB1 scene1 interrupted, rest skipped
        expectSceneRendered(sceneIdFB);
        expectSceneRenderedWithInterruptionEnabled(sceneId1OB1, fakeOffscreenBuffer1, sceneRenderBegin, sceneRenderInterrupted);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        // FB skipped, OB1 scene1 finished, OB1 scene2 interrupted, rest skipped
        expectSceneRenderedWithInterruptionEnabled(sceneId1OB1, fakeOffscreenBuffer1, sceneRenderInterrupted, sceneRenderBegin, EClearFlag::None);
        expectSceneRenderedWithInterruptionEnabled(sceneId2OB1, fakeOffscreenBuffer1, sceneRenderBegin, sceneRenderInterrupted, EClearFlag::None);
        expectFrameBufferRendered(false);
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        // FB skipped, OB1 scene1 skipped, OB1 scene2 finished, OB2 scene1 interrupted, rest skipped
        expectSceneRenderedWithInterruptionEnabled(sceneId2OB1, fakeOffscreenBuffer1, sceneRenderInterrupted, sceneRenderBegin, EClearFlag::None);
        expectSceneRenderedWithInterruptionEnabled(sceneId1OB2, fakeOffscreenBuffer2, sceneRenderBegin, sceneRenderInterrupted);
        expectFrameBufferRendered(false);
        expectInterruptibleOffscreenBufferSwapped(fakeOffscreenBuffer1);
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        // FB re-rendered to reflect changes in OB1, OB1 skipped, OB2 scene1 finished, OB2 scene2 interrupted
        expectSceneRendered(sceneIdFB);
        expectSceneRenderedWithInterruptionEnabled(sceneId1OB2, fakeOffscreenBuffer2, sceneRenderInterrupted, sceneRenderBegin, EClearFlag::None);
        expectSceneRenderedWithInterruptionEnabled(sceneId2OB2, fakeOffscreenBuffer2, sceneRenderBegin, sceneRenderInterrupted, EClearFlag::None);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        // FB skipped, OB1 skipped, OB2 scene2 finished
        expectSceneRenderedWithInterruptionEnabled(sceneId2OB2, fakeOffscreenBuffer2, sceneRenderInterrupted, sceneRenderBegin, EClearFlag::None);
        expectFrameBufferRendered(false);
        expectInterruptibleOffscreenBufferSwapped(fakeOffscreenBuffer2);
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        // FB re-rendered to reflect changes in OB2, rest skipped
        expectSceneRendered(sceneIdFB);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        // no change -> nothing rendered
        expectFrameBufferRendered(false);
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        hideScene(sceneId2OB2);
        hideScene(sceneId1OB2);
        hideScene(sceneId2OB1);
        hideScene(sceneId1OB1);
        hideScene(sceneIdFB);
        unassignScene(sceneId2OB2);
        unassignScene(sceneId1OB2);
        unassignScene(sceneId2OB1);
        unassignScene(sceneId1OB1);
        unassignScene(sceneIdFB);
    }

    TEST_P(ARenderer, willRerenderSceneThatWasRenderedAndModifiedWhileOtherSceneInterrupted)
    {
        createDisplayController();
        const DeviceResourceHandle fakeOffscreenBuffer(313u);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer, 1u, 1u, true);

        const SceneId sceneIdFB(12u);
        const SceneId sceneId1OB(13u);
        const SceneId sceneId2OB(14u);
        createScene(sceneIdFB);
        createScene(sceneId1OB);
        createScene(sceneId2OB);
        assignSceneToDisplayBuffer(sceneId2OB, 1, fakeOffscreenBuffer);
        assignSceneToDisplayBuffer(sceneId1OB, 0, fakeOffscreenBuffer);
        assignSceneToDisplayBuffer(sceneIdFB, 0);

        showScene(sceneIdFB);
        showScene(sceneId1OB);
        showScene(sceneId2OB);

        // FB rendered, OB scene1 fully rendered, OB scene2 interrupted
        expectSceneRendered(sceneIdFB);
        expectSceneRenderedWithInterruptionEnabled(sceneId1OB, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderBegin);
        expectSceneRenderedWithInterruptionEnabled(sceneId2OB, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderInterrupted);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        // modify OB scene1
        renderer.markBufferWithSceneForRerender(sceneId1OB);

        // FB skipped, OB scene1 skipped, OB scene2 finished
        expectSceneRenderedWithInterruptionEnabled(sceneId2OB, fakeOffscreenBuffer, sceneRenderInterrupted, sceneRenderBegin, EClearFlag::None);
        expectFrameBufferRendered(false);
        expectInterruptibleOffscreenBufferSwapped(fakeOffscreenBuffer);
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        // re-render FB to reflect changes
        // re-render OB scene1 (and OB scene2 as they share buffer) also as it was modified while interrupted
        expectSceneRendered(sceneIdFB);
        expectSceneRenderedWithInterruptionEnabled(sceneId1OB, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderBegin);
        expectSceneRenderedWithInterruptionEnabled(sceneId2OB, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderBegin);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectInterruptibleOffscreenBufferSwapped(fakeOffscreenBuffer);
        expectSwapBuffers();
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        // re-render FB one more time to reflect changes
        expectSceneRendered(sceneIdFB);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        // no change -> nothing rendered
        expectFrameBufferRendered(false);
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        hideScene(sceneId2OB);
        hideScene(sceneId1OB);
        hideScene(sceneIdFB);
        unassignScene(sceneId2OB);
        unassignScene(sceneId1OB);
        unassignScene(sceneIdFB);
    }

    TEST_P(ARenderer, willRerenderSceneThatWasRenderedAndModifiedWhileOtherSceneOnAnotherInterruptibleOBInterrupted)
    {
        createDisplayController();
        const DeviceResourceHandle fakeOffscreenBuffer1(313u);
        const DeviceResourceHandle fakeOffscreenBuffer2(314u);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer1, 1u, 1u, true);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer2, 1u, 1u, true);

        const SceneId sceneIdFB(12u);
        const SceneId sceneId1OB(13u);
        const SceneId sceneId2OB(14u);
        createScene(sceneIdFB);
        createScene(sceneId1OB);
        createScene(sceneId2OB);
        assignSceneToDisplayBuffer(sceneId2OB, 0, fakeOffscreenBuffer2);
        assignSceneToDisplayBuffer(sceneId1OB, 1, fakeOffscreenBuffer1);
        assignSceneToDisplayBuffer(sceneIdFB, 0);

        showScene(sceneIdFB);
        showScene(sceneId1OB);
        showScene(sceneId2OB);

        // FB rendered, OB1 scene fully rendered, OB2 scene interrupted
        expectSceneRendered(sceneIdFB);
        expectSceneRenderedWithInterruptionEnabled(sceneId1OB, fakeOffscreenBuffer1, sceneRenderBegin, sceneRenderBegin);
        expectSceneRenderedWithInterruptionEnabled(sceneId2OB, fakeOffscreenBuffer2, sceneRenderBegin, sceneRenderInterrupted);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectInterruptibleOffscreenBufferSwapped(fakeOffscreenBuffer1);
        expectSwapBuffers();
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        // modify OB1 scene
        renderer.markBufferWithSceneForRerender(sceneId1OB);

        // FB re-rendered to reflect finished OB1, OB1 scene skipped due to interruption, OB2 scene finished
        expectSceneRendered(sceneIdFB);
        expectSceneRenderedWithInterruptionEnabled(sceneId2OB, fakeOffscreenBuffer2, sceneRenderInterrupted, sceneRenderBegin, EClearFlag::None);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectInterruptibleOffscreenBufferSwapped(fakeOffscreenBuffer2);
        expectSwapBuffers();
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        // FB re-rendered to reflect finished OB2, OB1 scene re-rendered due to modification, OB2 scene skipped
        expectSceneRendered(sceneIdFB);
        expectSceneRenderedWithInterruptionEnabled(sceneId1OB, fakeOffscreenBuffer1, sceneRenderBegin, sceneRenderBegin);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectInterruptibleOffscreenBufferSwapped(fakeOffscreenBuffer1);
        expectSwapBuffers();
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        // FB re-rendered once more to reflect changes from OB1
        expectSceneRendered(sceneIdFB);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        // no change -> nothing rendered
        expectFrameBufferRendered(false);
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        hideScene(sceneId2OB);
        hideScene(sceneId1OB);
        hideScene(sceneIdFB);
        unassignScene(sceneId2OB);
        unassignScene(sceneId1OB);
        unassignScene(sceneIdFB);
    }

    TEST_P(ARenderer, willRenderAllScenesFromAllBuffersInOneFrameIfWithinBudget)
    {
        // 1 OB, 2 interruptible OBs, 2 scenes per OB

        createDisplayController();
        DeviceResourceHandle disp1OB(313u);
        DeviceResourceHandle disp1OBint1(315u);
        DeviceResourceHandle disp1OBint2(316u);
        renderer.registerOffscreenBuffer(disp1OB, 1u, 1u, false);
        renderer.registerOffscreenBuffer(disp1OBint1, 1u, 1u, true);
        renderer.registerOffscreenBuffer(disp1OBint2, 1u, 1u, true);

        const SceneId sceneIdDisp1FB(12u);
        const SceneId sceneIdDisp1OBscene(14u);
        const SceneId sceneIdDisp1OBint1scene1(16u);
        const SceneId sceneIdDisp1OBint1scene2(17u);
        const SceneId sceneIdDisp1OBint2scene1(18u);
        const SceneId sceneIdDisp1OBint2scene2(19u);

        createScene(sceneIdDisp1FB);
        createScene(sceneIdDisp1OBscene);
        createScene(sceneIdDisp1OBint1scene1);
        createScene(sceneIdDisp1OBint1scene2);
        createScene(sceneIdDisp1OBint2scene1);
        createScene(sceneIdDisp1OBint2scene2);

        assignSceneToDisplayBuffer(sceneIdDisp1FB, 0);
        assignSceneToDisplayBuffer(sceneIdDisp1OBscene, 0, disp1OB);
        assignSceneToDisplayBuffer(sceneIdDisp1OBint2scene1, 0, disp1OBint2);
        assignSceneToDisplayBuffer(sceneIdDisp1OBint2scene2, 1, disp1OBint2);
        assignSceneToDisplayBuffer(sceneIdDisp1OBint1scene1, 0, disp1OBint1);
        assignSceneToDisplayBuffer(sceneIdDisp1OBint1scene2, 1, disp1OBint1);

        showScene(sceneIdDisp1FB);
        showScene(sceneIdDisp1OBscene);
        showScene(sceneIdDisp1OBint1scene1);
        showScene(sceneIdDisp1OBint1scene2);
        showScene(sceneIdDisp1OBint2scene1);
        showScene(sceneIdDisp1OBint2scene2);

        // all rendered
        expectSceneRendered(sceneIdDisp1OBscene, disp1OB, EDiscardDepth::Allowed);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSceneRendered(sceneIdDisp1FB);

        expectInterruptibleOffscreenBufferSwapped(disp1OBint1);
        expectInterruptibleOffscreenBufferSwapped(disp1OBint2);
        expectSceneRenderedWithInterruptionEnabled(sceneIdDisp1OBint1scene1, disp1OBint1, sceneRenderBegin, sceneRenderBegin);
        expectSceneRenderedWithInterruptionEnabled(sceneIdDisp1OBint1scene2, disp1OBint1, sceneRenderBegin, sceneRenderBegin);
        expectSceneRenderedWithInterruptionEnabled(sceneIdDisp1OBint2scene1, disp1OBint2, sceneRenderBegin, sceneRenderBegin);
        expectSceneRenderedWithInterruptionEnabled(sceneIdDisp1OBint2scene2, disp1OBint2, sceneRenderBegin, sceneRenderBegin);

        expectSwapBuffers();

        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        // FB has to be re-rendered because there were some interruptible OBs finished last frame,
        // interruptible OBs are rendered at end of frame so the FBs have to be rendered again next frame
        // in order to use the latest state of the OBs wherever they are used in FBs
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSceneRendered(sceneIdDisp1FB);
        expectSwapBuffers();

        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        // no change -> nothing rendered
        expectFrameBufferRendered(false);
        doOneRendererLoop();

        Mock::VerifyAndClearExpectations(renderer.m_displayController);

        hideScene(sceneIdDisp1FB);
        hideScene(sceneIdDisp1OBscene);
        hideScene(sceneIdDisp1OBint1scene1);
        hideScene(sceneIdDisp1OBint1scene2);
        hideScene(sceneIdDisp1OBint2scene1);
        hideScene(sceneIdDisp1OBint2scene2);

        unassignScene(sceneIdDisp1FB);
        unassignScene(sceneIdDisp1OBscene);
        unassignScene(sceneIdDisp1OBint1scene1);
        unassignScene(sceneIdDisp1OBint1scene2);
        unassignScene(sceneIdDisp1OBint2scene1);
        unassignScene(sceneIdDisp1OBint2scene2);
    }

    TEST_P(ARenderer, canMapSceneWhileThereIsInterruption)
    {
        createDisplayController();
        const DeviceResourceHandle fakeOffscreenBuffer(313u);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer, 1u, 1u, true);

        const SceneId sceneIdFB(12u);
        const SceneId sceneIdOB(13u);
        const SceneId sceneId2(14u);
        createScene(sceneIdFB);
        createScene(sceneIdOB);
        createScene(sceneId2);
        assignSceneToDisplayBuffer(sceneIdFB, 0);
        assignSceneToDisplayBuffer(sceneIdOB, 0, fakeOffscreenBuffer);

        showScene(sceneIdFB);
        showScene(sceneIdOB);

        // FB rendered, OB interrupted
        expectSceneRendered(sceneIdFB);
        expectSceneRenderedWithInterruptionEnabled(sceneIdOB, fakeOffscreenBuffer, sceneRenderBegin, sceneRenderInterrupted);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        // map other scene to FB while interrupted
        assignSceneToDisplayBuffer(sceneId2, 0);

        // FB rendered because of new scene mapped, OB finished
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSceneRendered(sceneIdFB);
        expectSceneRenderedWithInterruptionEnabled(sceneIdOB, fakeOffscreenBuffer, sceneRenderInterrupted, sceneRenderBegin, EClearFlag::None);
        expectInterruptibleOffscreenBufferSwapped(fakeOffscreenBuffer);
        expectSwapBuffers();
        doOneRendererLoop();

        // re-render FB to reflect change happened to OB in previous frame
        expectSceneRendered(sceneIdFB);
        expectFrameBufferRendered(true, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        // no change -> nothing rendered
        expectFrameBufferRendered(false);
        doOneRendererLoop();

        hideScene(sceneIdOB);
        hideScene(sceneIdFB);
        unassignScene(sceneIdOB);
        unassignScene(sceneIdFB);
        unassignScene(sceneId2);
    }

    TEST_P(ARenderer, doesNotReportSceneIfNotRenderedToExpirationMonitor)
    {
        createDisplayController();
        const SceneId sceneIdFB(1u);
        const SceneId sceneIdOB(2u);
        const SceneId sceneIdOBint(3u);
        createScene(sceneIdFB);
        createScene(sceneIdOB);
        createScene(sceneIdOBint);

        initiateExpirationMonitoring({ sceneIdOB, sceneIdFB, sceneIdOBint });

        DeviceResourceHandle ob(316u);
        DeviceResourceHandle obInt(317u);
        renderer.registerOffscreenBuffer(ob, 1u, 1u, false);
        renderer.registerOffscreenBuffer(obInt, 1u, 1u, true);

        assignSceneToDisplayBuffer(sceneIdFB, 0);
        assignSceneToDisplayBuffer(sceneIdOB, 0, ob);
        assignSceneToDisplayBuffer(sceneIdOBint, 0, obInt);

        expectOffscreenBufferCleared(ob);
        expectFrameBufferRendered();
        expectOffscreenBufferCleared(obInt);
        expectInterruptibleOffscreenBufferSwapped(obInt);
        expectSwapBuffers();
        doOneRendererLoop();

        expectScenesReportedToExpirationMonitorAsRendered({});

        unassignScene(sceneIdFB);
        unassignScene(sceneIdOB);
        unassignScene(sceneIdOBint);
        expirationMonitor.onDestroyed(sceneIdFB);
        expirationMonitor.onDestroyed(sceneIdOB);
        expirationMonitor.onDestroyed(sceneIdOBint);
    }

    TEST_P(ARenderer, reportsSceneAsRenderedToExpirationMonitor)
    {
        createDisplayController();
        const SceneId sceneIdFB(1u);
        const SceneId sceneIdOB(2u);
        const SceneId sceneIdOBint(3u);
        createScene(sceneIdFB);
        createScene(sceneIdOB);
        createScene(sceneIdOBint);

        initiateExpirationMonitoring({ sceneIdOB, sceneIdFB, sceneIdOBint });

        DeviceResourceHandle ob(316u);
        DeviceResourceHandle obInt(317u);
        renderer.registerOffscreenBuffer(ob, 1u, 1u, false);
        renderer.registerOffscreenBuffer(obInt, 1u, 1u, true);

        assignSceneToDisplayBuffer(sceneIdFB, 0);
        assignSceneToDisplayBuffer(sceneIdOB, 0, ob);
        assignSceneToDisplayBuffer(sceneIdOBint, 0, obInt);

        showScene(sceneIdFB);
        showScene(sceneIdOB);
        showScene(sceneIdOBint);

        expectFrameBufferRendered(true, EClearFlag::None);
        expectInterruptibleOffscreenBufferSwapped(obInt);
        expectSceneRendered(sceneIdOB, ob, EDiscardDepth::Allowed);
        expectSceneRendered(sceneIdFB);
        expectSceneRenderedWithInterruptionEnabled(sceneIdOBint, obInt, sceneRenderBegin, sceneRenderBegin);
        expectSwapBuffers();
        doOneRendererLoop();

        expectScenesReportedToExpirationMonitorAsRendered({ sceneIdOB, sceneIdFB, sceneIdOBint });

        hideScene(sceneIdFB);
        hideScene(sceneIdOB);
        hideScene(sceneIdOBint);
        unassignScene(sceneIdFB);
        unassignScene(sceneIdOB);
        unassignScene(sceneIdOBint);
        expirationMonitor.onDestroyed(sceneIdFB);
        expirationMonitor.onDestroyed(sceneIdOB);
        expirationMonitor.onDestroyed(sceneIdOBint);
    }

    TEST_P(ARenderer, reportsSceneAsRenderedToExpirationMonitorOnlyAfterFullyRenderedAndNotDuringInterruption)
    {
        createDisplayController();
        const SceneId sceneIdFB(1u);
        const SceneId sceneIdOBint(3u);
        createScene(sceneIdFB);
        createScene(sceneIdOBint);

        initiateExpirationMonitoring({ sceneIdFB, sceneIdOBint });

        DeviceResourceHandle obInt(317u);
        renderer.registerOffscreenBuffer(obInt, 1u, 1u, true);

        assignSceneToDisplayBuffer(sceneIdFB, 0);
        assignSceneToDisplayBuffer(sceneIdOBint, 0, obInt);

        showScene(sceneIdFB);
        showScene(sceneIdOBint);

        expectFrameBufferRendered(true, EClearFlag::None);
        expectSceneRendered(sceneIdFB);
        expectSceneRenderedWithInterruptionEnabled(sceneIdOBint, obInt, sceneRenderBegin, sceneRenderInterrupted);
        expectSwapBuffers();
        doOneRendererLoop();

        // only FB scene reported as rendered, OB scene is interrupted
        expectScenesReportedToExpirationMonitorAsRendered({ sceneIdFB });

        expectFrameBufferRendered(false);
        expectInterruptibleOffscreenBufferSwapped(obInt);
        expectSceneRenderedWithInterruptionEnabled(sceneIdOBint, obInt, sceneRenderInterrupted, sceneRenderBegin, EClearFlag::None);
        doOneRendererLoop();

        // OB scene is reported now as it was fully rendered
        expectScenesReportedToExpirationMonitorAsRendered({ sceneIdOBint });

        hideScene(sceneIdFB);
        hideScene(sceneIdOBint);
        unassignScene(sceneIdFB);
        unassignScene(sceneIdOBint);
        expirationMonitor.onDestroyed(sceneIdFB);
        expirationMonitor.onDestroyed(sceneIdOBint);
    }

    TEST_P(ARenderer, pendingClearFlagPersistsAcrossScenes_FB)
    {
        createDisplayController();
        constexpr SceneId scene1{ 1u };
        constexpr SceneId scene2{ 3u };
        createScene(scene1);
        createScene(scene2);

        assignSceneToDisplayBuffer(scene1, 0);
        assignSceneToDisplayBuffer(scene2, 0);

        showScene(scene1);
        showScene(scene2);

        expectFrameBufferRendered(true, EClearFlag::None);
        // simulate that the render executor cleared and reset pending clear in context
        expectSceneRenderedExt(scene1, DisplayControllerMock::FakeFrameBufferHandle, EClearFlag::All, Renderer::DefaultClearColor, {}, {}, EClearFlag::None, EDiscardDepth::Disallowed);
        // next scene render will pass the modified flags
        expectSceneRendered(scene2, DisplayControllerMock::FakeFrameBufferHandle, EClearFlag::None);
        expectSwapBuffers();
        doOneRendererLoop();

        hideScene(scene1);
        hideScene(scene2);
        unassignScene(scene1);
        unassignScene(scene2);
    }

    TEST_P(ARenderer, pendingClearFlagPersistsAcrossScenes_OB)
    {
        createDisplayController();
        constexpr SceneId scene1{ 1u };
        constexpr SceneId scene2{ 3u };
        createScene(scene1);
        createScene(scene2);

        constexpr DeviceResourceHandle ob{ 317u };
        renderer.registerOffscreenBuffer(ob, 1u, 1u, false);

        assignSceneToDisplayBuffer(scene1, 0, ob);
        assignSceneToDisplayBuffer(scene2, 0, ob);

        showScene(scene1);
        showScene(scene2);

        expectFrameBufferRendered();
        // simulate that the render executor cleared and reset pending clear in context
        expectSceneRenderedExt(scene1, ob, EClearFlag::All, Renderer::DefaultClearColor, {}, {}, EClearFlag::None, EDiscardDepth::Disallowed);
        // next scene render will pass the modified flags
        expectSceneRenderedExt(scene2, ob, EClearFlag::None, Renderer::DefaultClearColor, {}, {}, EClearFlag::None, EDiscardDepth::Allowed);
        expectSwapBuffers();
        doOneRendererLoop();

        hideScene(scene1);
        hideScene(scene2);
        unassignScene(scene1);
        unassignScene(scene2);
    }

    TEST_P(ARenderer, pendingClearFlagPersistsAcrossScenes_OBinterruptible)
    {
        createDisplayController();
        constexpr SceneId scene1{ 1u };
        constexpr SceneId scene2{ 3u };
        createScene(scene1);
        createScene(scene2);

        constexpr DeviceResourceHandle ob{ 317u };
        renderer.registerOffscreenBuffer(ob, 1u, 1u, true);

        assignSceneToDisplayBuffer(scene1, 0, ob);
        assignSceneToDisplayBuffer(scene2, 0, ob);

        showScene(scene1);
        showScene(scene2);

        expectFrameBufferRendered();
        // simulate that the render executor cleared and reset pending clear in context
        expectSceneRenderedExt(scene1, ob, EClearFlag::All, Renderer::DefaultClearColor, {}, {}, EClearFlag::None, EDiscardDepth::Disallowed, &RendererMock::FrameTimerInstance);
        // next scene render will pass the modified flags
        expectSceneRenderedWithInterruptionEnabled(scene2, ob, {}, {}, EClearFlag::None);
        expectInterruptibleOffscreenBufferSwapped(ob);
        expectSwapBuffers();
        doOneRendererLoop();

        hideScene(scene1);
        hideScene(scene2);
        unassignScene(scene1);
        unassignScene(scene2);
    }

    TEST_P(ARenderer, allowDepthDiscardOnlyForLastRenderedSceneToOffscreenbuffer)
    {
        createDisplayController();

        const DeviceResourceHandle fakeOffscreenBuffer1(313u);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer1, 1u, 2u, false);

        const SceneId sceneId1(12u);
        const SceneId sceneId2(13u);
        const SceneId sceneId3(14u);
        createScene(sceneId1);
        createScene(sceneId2);
        createScene(sceneId3);
        assignSceneToDisplayBuffer(sceneId1, 0, fakeOffscreenBuffer1);
        assignSceneToDisplayBuffer(sceneId2, 1, fakeOffscreenBuffer1);
        assignSceneToDisplayBuffer(sceneId3, 2, fakeOffscreenBuffer1);
        showScene(sceneId1);
        showScene(sceneId2);
        // sceneId3 not shown

        expectSceneRendered(sceneId1, fakeOffscreenBuffer1, EDiscardDepth::Disallowed);
        expectSceneRendered(sceneId2, fakeOffscreenBuffer1, EDiscardDepth::Allowed);
        expectFrameBufferRendered();
        expectSwapBuffers();
        doOneRendererLoop();

        hideScene(sceneId1);
        hideScene(sceneId2);
        unassignScene(sceneId1);
        unassignScene(sceneId2);
        unassignScene(sceneId3);
    }

    TEST_P(ARenderer, allowDepthDiscardOnlyIfBothDepthAndStencilClearEnabled)
    {
        createDisplayController();

        const DeviceResourceHandle fakeOffscreenBuffer1(313u);
        const DeviceResourceHandle fakeOffscreenBuffer2(314u);
        const DeviceResourceHandle fakeOffscreenBuffer3(315u);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer1, 1u, 2u, false);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer2, 1u, 2u, false);
        renderer.registerOffscreenBuffer(fakeOffscreenBuffer3, 1u, 2u, false);
        EXPECT_CALL(renderer, setClearFlags(fakeOffscreenBuffer1, EClearFlag::Color | EClearFlag::Stencil));
        EXPECT_CALL(renderer, setClearFlags(fakeOffscreenBuffer2, EClearFlag::Color | EClearFlag::Depth));
        EXPECT_CALL(renderer, setClearFlags(fakeOffscreenBuffer3, EClearFlag::Depth | EClearFlag::Stencil));
        renderer.setClearFlags(fakeOffscreenBuffer1, EClearFlag::Color | EClearFlag::Stencil);
        renderer.setClearFlags(fakeOffscreenBuffer2, EClearFlag::Color | EClearFlag::Depth);
        renderer.setClearFlags(fakeOffscreenBuffer3, EClearFlag::Depth | EClearFlag::Stencil);

        const SceneId sceneId1(12u);
        const SceneId sceneId2(13u);
        const SceneId sceneId3(14u);
        createScene(sceneId1);
        createScene(sceneId2);
        createScene(sceneId3);
        assignSceneToDisplayBuffer(sceneId1, 0, fakeOffscreenBuffer1);
        assignSceneToDisplayBuffer(sceneId2, 0, fakeOffscreenBuffer2);
        assignSceneToDisplayBuffer(sceneId3, 0, fakeOffscreenBuffer3);
        showScene(sceneId1);
        showScene(sceneId2);
        showScene(sceneId3);

        expectSceneRenderedExt(sceneId1, fakeOffscreenBuffer1, EClearFlag::Color | EClearFlag::Stencil, Renderer::DefaultClearColor, {}, {}, {}, EDiscardDepth::Disallowed);
        expectSceneRenderedExt(sceneId2, fakeOffscreenBuffer2, EClearFlag::Color | EClearFlag::Depth, Renderer::DefaultClearColor, {}, {}, {}, EDiscardDepth::Disallowed);
        expectSceneRenderedExt(sceneId3, fakeOffscreenBuffer3, EClearFlag::Depth | EClearFlag::Stencil, Renderer::DefaultClearColor, {}, {}, {}, EDiscardDepth::Allowed);
        expectFrameBufferRendered();
        expectSwapBuffers();
        doOneRendererLoop();

        hideScene(sceneId1);
        hideScene(sceneId2);
        hideScene(sceneId3);
        unassignScene(sceneId1);
        unassignScene(sceneId2);
        unassignScene(sceneId3);
    }

    TEST_P(ARenderer, neverAllowsDepthDiscardForFBOrInterruptibleOB)
    {
        createDisplayController();
        const SceneId sceneIdFB(1u);
        const SceneId sceneIdOBint(3u);
        createScene(sceneIdFB);
        createScene(sceneIdOBint);

        DeviceResourceHandle obInt(317u);
        renderer.registerOffscreenBuffer(obInt, 1u, 1u, true);

        assignSceneToDisplayBuffer(sceneIdFB, 0);
        assignSceneToDisplayBuffer(sceneIdOBint, 0, obInt);

        showScene(sceneIdFB);
        showScene(sceneIdOBint);

        expectFrameBufferRendered(true, EClearFlag::None);
        expectSceneRendered(sceneIdFB, DisplayControllerMock::FakeFrameBufferHandle, EDiscardDepth::Disallowed);
        expectSceneRenderedExt(sceneIdOBint, obInt, EClearFlag::All, Renderer::DefaultClearColor, {}, {}, {}, EDiscardDepth::Disallowed, &RendererMock::FrameTimerInstance);
        expectInterruptibleOffscreenBufferSwapped(obInt);
        expectSwapBuffers();
        doOneRendererLoop();

        hideScene(sceneIdFB);
        hideScene(sceneIdOBint);
        unassignScene(sceneIdFB);
        unassignScene(sceneIdOBint);
    }
}
