//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "gmock/gmock.h"
#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-renderer-api/WarpingMeshData.h"
#include "ramses-renderer-api/IRendererEventHandler.h"
#include "ramses-framework-api/RamsesFrameworkConfig.h"
#include "ramses-framework-api/RamsesFramework.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"

#include "RendererLib/RendererCommands.h"
#include "RamsesRendererImpl.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "PlatformAbstraction/PlatformEvent.h"
#include "RendererCommandVisitorMock.h"
#include "SceneAPI/RenderState.h"
#include "ramses-renderer-api/IRendererSceneControlEventHandler.h"

using namespace testing;

class SafeThreadWatchdogNotificationMock : public ramses::IThreadWatchdogNotification
{
public:
    explicit SafeThreadWatchdogNotificationMock(ramses_internal::PlatformLock& lock)
        : m_lock(lock)
    {
    }

    MOCK_METHOD(void, safe_notifyThread, (ramses::ERamsesThreadIdentifier));
    MOCK_METHOD(void, safe_registerThread, (ramses::ERamsesThreadIdentifier));
    MOCK_METHOD(void, safe_unregisterThread, (ramses::ERamsesThreadIdentifier));

private:
    void notifyThread(ramses::ERamsesThreadIdentifier threadID) override
    {
        ramses_internal::PlatformGuard g(m_lock);
        safe_notifyThread(threadID);
    }

    void registerThread(ramses::ERamsesThreadIdentifier threadID) override
    {
        ramses_internal::PlatformGuard g(m_lock);
        safe_registerThread(threadID);
    }

    void unregisterThread(ramses::ERamsesThreadIdentifier threadID) override
    {
        ramses_internal::PlatformGuard g(m_lock);
        safe_unregisterThread(threadID);
    }

    ramses_internal::PlatformLock& m_lock;
};


class ARamsesRenderer : public ::testing::Test
{
protected:
    explicit ARamsesRenderer(const ramses::RendererConfig& rendererConfig = ramses::RendererConfig())
        : framework()
        , renderer(*framework.createRenderer(rendererConfig))
        , commandBuffer(renderer.impl.getPendingCommands())
    {
    }

    ramses::displayId_t addDisplay(bool warpingEnabled = false)
    {
        //Create a display
        ramses::DisplayConfig displayConfig;
        if (warpingEnabled)
        {
            displayConfig.enableWarpingPostEffect();
        }
        EXPECT_EQ(ramses::StatusOK, displayConfig.validate());
        return renderer.createDisplay(displayConfig);
    }

protected:
    ramses::RamsesFramework framework;
    ramses::RamsesRenderer& renderer;
    const ramses_internal::RendererCommands& commandBuffer;
    StrictMock<ramses_internal::RendererCommandVisitorMock> cmdVisitor;
};

class ARamsesRendererWithDisplay : public ARamsesRenderer
{
    void SetUp() override
    {
        displayId = addDisplay();
        warpingDisplayId = addDisplay(true);
        EXPECT_NE(ramses::displayId_t::Invalid(), displayId);
        EXPECT_NE(ramses::displayId_t::Invalid(), warpingDisplayId);
        renderer.flush();
    }

    void TearDown() override
    {
        renderer.destroyDisplay(displayId);
        renderer.destroyDisplay(warpingDisplayId);
    }

protected:
    ramses::displayId_t displayId;
    ramses::displayId_t warpingDisplayId;
};

class ARamsesRendererWithSystemCompositorController : public ARamsesRenderer
{
public:
    ARamsesRendererWithSystemCompositorController()
        : ARamsesRenderer(CreateRendererConfigWithSystemCompositor())
    {
    }

    static ramses::RendererConfig CreateRendererConfigWithSystemCompositor()
    {
        ramses::RendererConfig config;
        config.enableSystemCompositorControl();
        return config;
    }
};

TEST_F(ARamsesRenderer, hasNoCommandsOnStartUp)
{
    cmdVisitor.visit(commandBuffer);
}

TEST_F(ARamsesRenderer, canOnlyGetOneSceneControlAPI_RendererSceneControl)
{
    const auto api = renderer.getSceneControlAPI();
    EXPECT_TRUE(api != nullptr);
    EXPECT_EQ(api, renderer.getSceneControlAPI());
    EXPECT_EQ(api, renderer.getSceneControlAPI());

    EXPECT_TRUE(renderer.createDcsmContentControl() == nullptr);
    EXPECT_TRUE(renderer.createDcsmContentControl() == nullptr);
}

TEST_F(ARamsesRenderer, canOnlyGetOneSceneControlAPI_DcsmContentControl)
{
    EXPECT_TRUE(renderer.createDcsmContentControl() != nullptr);

    EXPECT_TRUE(renderer.getSceneControlAPI() == nullptr);
    EXPECT_TRUE(renderer.getSceneControlAPI() == nullptr);
}

TEST_F(ARamsesRenderer, canCreateDcsmContentControlOnlyOnce)
{
    EXPECT_TRUE(renderer.createDcsmContentControl() != nullptr);

    EXPECT_TRUE(renderer.createDcsmContentControl() == nullptr);
    EXPECT_TRUE(renderer.createDcsmContentControl() == nullptr);
}

/*
 * Display
 */
TEST_F(ARamsesRenderer, createsACommandForDisplayCreation)
{
    const ramses::displayId_t displayId = addDisplay();
    EXPECT_NE(ramses::displayId_t::Invalid(), displayId);

    EXPECT_CALL(cmdVisitor, createDisplayContext(_, ramses_internal::DisplayHandle{ displayId.getValue() }, _));
    cmdVisitor.visit(commandBuffer);
}

TEST_F(ARamsesRenderer, createsMultipleCommandsForMultipleDisplayCreation)
{
    EXPECT_NE(ramses::displayId_t::Invalid(), addDisplay());
    EXPECT_NE(ramses::displayId_t::Invalid(), addDisplay());
    EXPECT_NE(ramses::displayId_t::Invalid(), addDisplay());

    EXPECT_CALL(cmdVisitor, createDisplayContext(_, _, _)).Times(3);
    cmdVisitor.visit(commandBuffer);
}

TEST_F(ARamsesRendererWithDisplay, createsCommandForDisplayDestruction)
{
    EXPECT_EQ(ramses::StatusOK, renderer.destroyDisplay(displayId));
    EXPECT_CALL(cmdVisitor, destroyDisplayContext(_));
    cmdVisitor.visit(commandBuffer);
}

TEST_F(ARamsesRenderer, canQueryDisplayFramebufferIDs)
{
    const ramses::displayId_t displayId1 = addDisplay();
    const auto fbId1 = renderer.getDisplayFramebuffer(displayId1);
    EXPECT_TRUE(fbId1.isValid());
}

TEST_F(ARamsesRenderer, displayFramebufferIDsAreUnique)
{
    EXPECT_FALSE(renderer.getDisplayFramebuffer(ramses::displayId_t::Invalid()).isValid());

    const ramses::displayId_t displayId1 = addDisplay();
    const ramses::displayId_t displayId2 = addDisplay();
    const ramses::displayId_t displayId3 = addDisplay();
    const auto fbId1 = renderer.getDisplayFramebuffer(displayId1);
    const auto fbId2 = renderer.getDisplayFramebuffer(displayId2);
    const auto fbId3 = renderer.getDisplayFramebuffer(displayId3);
    EXPECT_TRUE(fbId1.isValid());
    EXPECT_TRUE(fbId2.isValid());
    EXPECT_TRUE(fbId3.isValid());
    EXPECT_NE(fbId1, fbId2);
    EXPECT_NE(fbId1, fbId3);
    EXPECT_NE(fbId2, fbId3);
}

TEST_F(ARamsesRenderer, displayFramebufferIsInvalidIfDisplayDestroyed)
{
    const ramses::displayId_t displayId = addDisplay();
    EXPECT_TRUE(renderer.getDisplayFramebuffer(displayId).isValid());
    renderer.destroyDisplay(displayId);
    EXPECT_FALSE(renderer.getDisplayFramebuffer(displayId).isValid());
}

TEST_F(ARamsesRenderer, createsCommandForLoggingRenderInfo)
{
    EXPECT_EQ(ramses::StatusOK, renderer.logRendererInfo());
    EXPECT_CALL(cmdVisitor, logInfo(_, _, _));
    cmdVisitor.visit(commandBuffer);
}

TEST_F(ARamsesRenderer, canCreateDisplayWithEmbeddedCompositingSetOnRendererConfig)
{
    ramses::RamsesFramework ramsesFramework;
    ramses::RendererConfig rendererConfig;
    rendererConfig.setWaylandEmbeddedCompositingSocketName("ec-socket");
    ramses::RamsesRenderer* ramsesRenderer = ramsesFramework.createRenderer(rendererConfig);

    const ramses::displayId_t displayId = ramsesRenderer->createDisplay({});
    EXPECT_NE(ramses::displayId_t::Invalid(), displayId);

    EXPECT_CALL(cmdVisitor, createDisplayContext(_, ramses_internal::DisplayHandle{ displayId.getValue() }, _));
    cmdVisitor.visit(ramsesRenderer->impl.getPendingCommands());
}

TEST_F(ARamsesRenderer, canCreateDisplayWithEmbeddedCompositingSetOnDisplayConfig)
{
    ramses::DisplayConfig displayConfig;
    displayConfig.setWaylandEmbeddedCompositingSocketName("ec-socket");
    const ramses::displayId_t displayId = renderer.createDisplay(displayConfig);
    EXPECT_NE(ramses::displayId_t::Invalid(), displayId);

    EXPECT_CALL(cmdVisitor, createDisplayContext(_, ramses_internal::DisplayHandle{ displayId.getValue() }, _));
    cmdVisitor.visit(commandBuffer);
}

TEST_F(ARamsesRenderer, canNotCreateDisplayIfEmbeddedCompositingSetOnBothRendererAndDisplayConfig)
{
    ramses::RamsesFramework ramsesFramework;
    ramses::RendererConfig rendererConfig;
    rendererConfig.setWaylandEmbeddedCompositingSocketName("ec-socket");
    ramses::RamsesRenderer* ramsesRenderer = ramsesFramework.createRenderer(rendererConfig);

    ramses::DisplayConfig displayConfig;
    displayConfig.setWaylandEmbeddedCompositingSocketName("ec-socket");
    const ramses::displayId_t displayId = ramsesRenderer->createDisplay(displayConfig);
    EXPECT_EQ(ramses::displayId_t::Invalid(), displayId);

    EXPECT_CALL(cmdVisitor, createDisplayContext(_, ramses_internal::DisplayHandle{ displayId.getValue() }, _)).Times(0u);
    cmdVisitor.visit(ramsesRenderer->impl.getPendingCommands());
}

/*
* Update warping data
*/
TEST_F(ARamsesRenderer, createsNoCommandForWarpingDataUpdateOnInvalidDisplay)
{
    ramses::WarpingMeshData warpingData(0u, nullptr, 0u, nullptr, nullptr);

    EXPECT_NE(ramses::StatusOK, renderer.updateWarpingMeshData(ramses::displayId_t(0u), warpingData));
    cmdVisitor.visit(commandBuffer);
}

TEST_F(ARamsesRendererWithDisplay, createsNoCommandForWarpingDataUpdateOnDisplayWithoutWarping)
{
    ramses::WarpingMeshData warpingData(0u, nullptr, 0u, nullptr, nullptr);

    EXPECT_NE(ramses::StatusOK, renderer.updateWarpingMeshData(displayId, warpingData));
    cmdVisitor.visit(commandBuffer);
}

TEST_F(ARamsesRendererWithDisplay, createsNoCommandForInvalidWarpingDataUpdateOnWarpingDisplay)
{
    ramses::WarpingMeshData warpingData(0u, nullptr, 0u, nullptr, nullptr);

    EXPECT_NE(ramses::StatusOK, renderer.updateWarpingMeshData(warpingDisplayId, warpingData));
    cmdVisitor.visit(commandBuffer);
}

TEST_F(ARamsesRendererWithDisplay, createsCommandForValidWarpingDataUpdateOnWarpingDisplay)
{
    //sample warping data
    const uint16_t indices[] = { 0u, 1u, 2u };
    const float vertices[] = { -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f };
    const float texcoords[] = { 0.0f, 1.0f, 1.0f, 1.0f, 0.5f, 0.0f };
    ramses::WarpingMeshData warpingData(3u, indices, 3u, vertices, texcoords);

    EXPECT_EQ(ramses::StatusOK, renderer.updateWarpingMeshData(warpingDisplayId, warpingData));
    EXPECT_CALL(cmdVisitor, updateWarpingData(ramses_internal::DisplayHandle{ warpingDisplayId.getValue() }, _));
    cmdVisitor.visit(commandBuffer);
}

/*
* Offscreen buffers
*/
TEST_F(ARamsesRendererWithDisplay, createsCommandForOffscreenBufferCreate)
{
    EXPECT_NE(ramses::displayBufferId_t::Invalid(), renderer.createOffscreenBuffer(displayId, 40u, 40u, 4u));
    EXPECT_CALL(cmdVisitor, handleBufferCreateRequest(_, ramses_internal::DisplayHandle{ displayId.getValue() }, 40u, 40u, 4u, false, ramses_internal::ERenderBufferType_DepthStencilBuffer));
    cmdVisitor.visit(commandBuffer);
}

TEST_F(ARamsesRendererWithDisplay, createsCommandForOffscreenBufferCreate_WithoutDepthStencilBuffer)
{
    EXPECT_NE(ramses::displayBufferId_t::Invalid(), ramses::RamsesRenderer::createOffscreenBuffer(renderer, displayId, 40u, 40u, 4u, ramses::EDepthBufferType_None));
    EXPECT_CALL(cmdVisitor, handleBufferCreateRequest(_, ramses_internal::DisplayHandle{ displayId.getValue() }, 40u, 40u, 4u, false, ramses_internal::ERenderBufferType_InvalidBuffer));
    cmdVisitor.visit(commandBuffer);
}

TEST_F(ARamsesRendererWithDisplay, createsCommandForOffscreenBufferCreate_WithDepthBuffer)
{
    EXPECT_NE(ramses::displayBufferId_t::Invalid(), ramses::RamsesRenderer::createOffscreenBuffer(renderer, displayId, 40u, 40u, 4u, ramses::EDepthBufferType_Depth));
    EXPECT_CALL(cmdVisitor, handleBufferCreateRequest(_, ramses_internal::DisplayHandle{ displayId.getValue() }, 40u, 40u, 4u, false, ramses_internal::ERenderBufferType_DepthBuffer));
    cmdVisitor.visit(commandBuffer);
}

TEST_F(ARamsesRendererWithDisplay, createsCommandForOffscreenBufferCreate_WithDepthStencilBuffer)
{
    EXPECT_NE(ramses::displayBufferId_t::Invalid(), ramses::RamsesRenderer::createOffscreenBuffer(renderer, displayId, 40u, 40u, 4u, ramses::EDepthBufferType_DepthStencil));
    EXPECT_CALL(cmdVisitor, handleBufferCreateRequest(_, ramses_internal::DisplayHandle{ displayId.getValue() }, 40u, 40u, 4u, false, ramses_internal::ERenderBufferType_DepthStencilBuffer));
    cmdVisitor.visit(commandBuffer);
}

TEST_F(ARamsesRendererWithDisplay, createsCommandForDmaOffscreenBufferCreate)
{
    constexpr uint32_t fourccFormat = 777u;
    constexpr uint32_t bufferUsageFlags = 888u;
    constexpr uint64_t modifier = 999u;
    EXPECT_NE(ramses::displayBufferId_t::Invalid(), renderer.createDmaOffscreenBuffer(displayId, 40u, 40u, fourccFormat, bufferUsageFlags, modifier));
    EXPECT_CALL(cmdVisitor, handleDmaBufferCreateRequest(_, ramses_internal::DisplayHandle{ displayId.getValue() }, 40u, 40u, ramses_internal::DmaBufferFourccFormat(fourccFormat), ramses_internal::DmaBufferUsageFlags(bufferUsageFlags), ramses_internal::DmaBufferModifiers(modifier)));
    cmdVisitor.visit(commandBuffer);
}

TEST_F(ARamsesRendererWithDisplay, failsToCreateDmaOffscreenBufferIfRendererIsRunningInOwnThread)
{
    renderer.startThread();

    constexpr uint32_t fourccFormat = 777u;
    constexpr uint32_t bufferUsageFlags = 888u;
    constexpr uint64_t modifier = 999u;
    EXPECT_EQ(ramses::displayBufferId_t::Invalid(), renderer.createDmaOffscreenBuffer(displayId, 40u, 40u, fourccFormat, bufferUsageFlags, modifier));
    EXPECT_CALL(cmdVisitor, handleDmaBufferCreateRequest(_, _, _, _, _, _, _)).Times(0);
    cmdVisitor.visit(commandBuffer);
}

TEST_F(ARamsesRendererWithDisplay, canGetDmaOffscreenBufferFDAndStride)
{
    ramses_internal::RendererEvent event;
    event.eventType = ramses_internal::ERendererEventType::OffscreenBufferCreated;
    event.displayHandle = ramses_internal::DisplayHandle{displayId.getValue()};
    event.offscreenBuffer = ramses_internal::OffscreenBufferHandle{ 10u };
    event.dmaBufferFD = 20;
    event.dmaBufferStride = 30u;
    renderer.impl.getDisplayDispatcher().injectRendererEvent(std::move(event));

    ramses::RendererEventHandlerEmpty dummyHandler;
    renderer.dispatchEvents(dummyHandler);
    int resultFD = -1;
    uint32_t resultStride = 0u;
    EXPECT_EQ(ramses::StatusOK, renderer.getDmaOffscreenBufferFDAndStride(displayId, ramses::displayBufferId_t{ 10u }, resultFD, resultStride));
    EXPECT_EQ(20, resultFD);
    EXPECT_EQ(30u, resultStride);
}

TEST_F(ARamsesRendererWithDisplay, reportsErrorIfGetingFDAndStrideForUnknownDmaOffscreenBuffer)
{
    int resultFD = -1;
    uint32_t resultStride = 0u;
    EXPECT_NE(ramses::StatusOK, renderer.getDmaOffscreenBufferFDAndStride(displayId, ramses::displayBufferId_t{ 10u }, resultFD, resultStride));
}

TEST_F(ARamsesRendererWithDisplay, createsCommandForOffscreenBufferDestroy)
{
    const ramses::displayBufferId_t bufferId(0u);
    EXPECT_EQ(ramses::StatusOK, renderer.destroyOffscreenBuffer(displayId, bufferId));
    EXPECT_CALL(cmdVisitor, handleBufferDestroyRequest(ramses_internal::OffscreenBufferHandle{ bufferId.getValue() }, ramses_internal::DisplayHandle{ displayId.getValue() }));
    cmdVisitor.visit(commandBuffer);
}

TEST_F(ARamsesRendererWithDisplay, failsToCreateOffscreenBufferWithUnsupportedResolution)
{
    EXPECT_EQ(ramses::displayBufferId_t::Invalid(), renderer.createOffscreenBuffer(displayId, 0u, 1u, 4u));
    EXPECT_EQ(ramses::displayBufferId_t::Invalid(), renderer.createOffscreenBuffer(displayId, 1u, 0u, 4u));
    EXPECT_EQ(ramses::displayBufferId_t::Invalid(), renderer.createOffscreenBuffer(displayId, 0u, 0u, 4u));
    EXPECT_EQ(ramses::displayBufferId_t::Invalid(), renderer.createOffscreenBuffer(displayId, 5000u, 1u, 4u));
    EXPECT_EQ(ramses::displayBufferId_t::Invalid(), renderer.createOffscreenBuffer(displayId, 1u, 5000u, 4u));
    EXPECT_EQ(ramses::displayBufferId_t::Invalid(), renderer.createOffscreenBuffer(displayId, 5000u, 5000u, 4u));
}

TEST_F(ARamsesRendererWithDisplay, failsToCreateDmaOffscreenBufferWithUnsupportedResolution)
{
    constexpr uint32_t fourccFormat = 777u;
    constexpr uint32_t bufferUsageFlags = 888u;
    constexpr uint64_t modifier = 999u;

    EXPECT_EQ(ramses::displayBufferId_t::Invalid(), renderer.createDmaOffscreenBuffer(displayId, 0u, 1u, fourccFormat, bufferUsageFlags, modifier));
    EXPECT_EQ(ramses::displayBufferId_t::Invalid(), renderer.createDmaOffscreenBuffer(displayId, 1u, 0u, fourccFormat, bufferUsageFlags, modifier));
    EXPECT_EQ(ramses::displayBufferId_t::Invalid(), renderer.createDmaOffscreenBuffer(displayId, 0u, 0u, fourccFormat, bufferUsageFlags, modifier));
    EXPECT_EQ(ramses::displayBufferId_t::Invalid(), renderer.createDmaOffscreenBuffer(displayId, 5000u, 1u, fourccFormat, bufferUsageFlags, modifier));
    EXPECT_EQ(ramses::displayBufferId_t::Invalid(), renderer.createDmaOffscreenBuffer(displayId, 1u, 5000u, fourccFormat, bufferUsageFlags, modifier));
    EXPECT_EQ(ramses::displayBufferId_t::Invalid(), renderer.createDmaOffscreenBuffer(displayId, 5000u, 5000u, fourccFormat, bufferUsageFlags, modifier));}

/*
* Stream buffers
*/
TEST_F(ARamsesRendererWithDisplay, createsCommandForStreamBufferCreate)
{
    constexpr ramses::waylandIviSurfaceId_t source{ 123u };
    const auto streamBuffer = renderer.impl.createStreamBuffer(displayId, source);
    EXPECT_CALL(cmdVisitor, handleBufferCreateRequest(ramses_internal::StreamBufferHandle{ streamBuffer.getValue() }, ramses_internal::DisplayHandle{ displayId.getValue() }, ramses_internal::WaylandIviSurfaceId{ source.getValue() }));
    cmdVisitor.visit(commandBuffer);
}

TEST_F(ARamsesRendererWithDisplay, createsCommandForStreamBufferDestroy)
{
    constexpr ramses::streamBufferId_t streamBuffer{ 123u };
    EXPECT_EQ(ramses::StatusOK, renderer.impl.destroyStreamBuffer(displayId, streamBuffer));
    EXPECT_CALL(cmdVisitor, handleBufferDestroyRequest(ramses_internal::StreamBufferHandle{ streamBuffer.getValue() }, ramses_internal::DisplayHandle{ displayId.getValue() }));
    cmdVisitor.visit(commandBuffer);
}

/*
* Read Pixels
*/
TEST_F(ARamsesRendererWithDisplay, createsCommandForReadPixels)
{
    ramses::displayBufferId_t bufferId{ 123u };
    EXPECT_EQ(ramses::StatusOK, renderer.readPixels(displayId, bufferId, 1u, 2u, 3u, 4u));
    EXPECT_CALL(cmdVisitor, handleReadPixels(ramses_internal::DisplayHandle{ displayId.getValue() }, ramses_internal::OffscreenBufferHandle{ bufferId.getValue() }, 1u, 2u, 3u, 4u, false, ramses_internal::String{}));
    cmdVisitor.visit(commandBuffer);
}

TEST_F(ARamsesRendererWithDisplay, reportsErrorIfReadingPixelsForUnknownDisplay)
{
    EXPECT_NE(ramses::StatusOK, renderer.readPixels(ramses::displayId_t{ 999u }, {}, 1u, 2u, 3u, 4u));
}

TEST_F(ARamsesRendererWithDisplay, createsNoCommandForEmptyReadPixels)
{
    EXPECT_NE(ramses::StatusOK, renderer.readPixels(displayId, {}, 0u, 0u, 10u, 0u));
    EXPECT_NE(ramses::StatusOK, renderer.readPixels(displayId, {}, 0u, 0u, 0u, 10u));
    cmdVisitor.visit(commandBuffer);
}

/*
* SystemCompositorControl
*/
TEST_F(ARamsesRenderer, createsNoCommandForSystemCompositorControllerIfNotEnabledFromConfig)
{
    EXPECT_NE(ramses::StatusOK, renderer.setSurfaceVisibility(0, true));
    EXPECT_NE(ramses::StatusOK, renderer.setSurfaceOpacity(0, 0.2f));
    EXPECT_NE(ramses::StatusOK, renderer.setSurfaceRectangle(0, 1, 2, 3, 4));
    EXPECT_NE(ramses::StatusOK, renderer.setLayerVisibility(17,true));
    EXPECT_NE(ramses::StatusOK, renderer.takeSystemCompositorScreenshot("unused_name", -1));
    cmdVisitor.visit(commandBuffer);
}

TEST_F(ARamsesRendererWithSystemCompositorController, createsCommandForSystemCompositorControllerSetSurfaceVisibility)
{
    EXPECT_EQ(ramses::StatusOK, renderer.setSurfaceVisibility(1, true));
    EXPECT_CALL(cmdVisitor, systemCompositorSetIviSurfaceVisibility(ramses_internal::WaylandIviSurfaceId{ 1u }, true));
    cmdVisitor.visit(commandBuffer);
}

TEST_F(ARamsesRendererWithSystemCompositorController, createsCommandForSystemCompositorControllerSetSurfaceOpacity)
{
    EXPECT_EQ(ramses::StatusOK, renderer.setSurfaceOpacity(1, 0.2f));
    EXPECT_CALL(cmdVisitor, systemCompositorSetIviSurfaceOpacity(ramses_internal::WaylandIviSurfaceId{ 1u }, 0.2f));
    cmdVisitor.visit(commandBuffer);
}

TEST_F(ARamsesRendererWithSystemCompositorController, createsCommandForSystemCompositorControllerSetSurfaceRectangle)
{
    EXPECT_EQ(ramses::StatusOK, renderer.setSurfaceRectangle(1, 2, 3, 4, 5));
    EXPECT_CALL(cmdVisitor, systemCompositorSetIviSurfaceDestRectangle(ramses_internal::WaylandIviSurfaceId{ 1u }, 2, 3, 4, 5));
    cmdVisitor.visit(commandBuffer);
}

TEST_F(ARamsesRendererWithSystemCompositorController, createsCommandForSystemCompositorControllerAddSurfaceToLayer)
{
    EXPECT_EQ(ramses::StatusOK, renderer.impl.systemCompositorAddIviSurfaceToIviLayer(1, 2));
    EXPECT_CALL(cmdVisitor, systemCompositorAddIviSurfaceToIviLayer(ramses_internal::WaylandIviSurfaceId{ 1u }, ramses_internal::WaylandIviLayerId{ 2u }));
    cmdVisitor.visit(commandBuffer);
}

TEST_F(ARamsesRendererWithSystemCompositorController, createsCommandForSystemCompositorControllerSetLayerVisibility)
{
    EXPECT_EQ(ramses::StatusOK, renderer.setLayerVisibility(17, true));
    EXPECT_CALL(cmdVisitor, systemCompositorSetIviLayerVisibility(ramses_internal::WaylandIviLayerId{ 17u }, true));
    cmdVisitor.visit(commandBuffer);
}

TEST_F(ARamsesRendererWithSystemCompositorController, createsCommandForSystemCompositorControllerTakeScreenshot)
{
    EXPECT_EQ(ramses::StatusOK, renderer.takeSystemCompositorScreenshot("name", -1));
    EXPECT_CALL(cmdVisitor, systemCompositorScreenshot(ramses_internal::String{ "name" }, -1));
    cmdVisitor.visit(commandBuffer);
}

/*
 * Threading and thread sanitizer tests
 */
class SceneStateEventHandler : public ramses::RendererEventHandlerEmpty
{
public:
    explicit SceneStateEventHandler(ramses::RamsesRenderer& renderer)
        : m_renderer(renderer)
    {
    }

    virtual void displayCreated(ramses::displayId_t /*displayId*/, ramses::ERendererEventResult result) override
    {
        m_displayCreated = (result == ramses::ERendererEventResult_OK);
    }

    void waitForDisplayCreationEvent()
    {
        while (!m_displayCreated)
        {
            m_renderer.dispatchEvents(*this);
            std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
        }
    }

private:
    ramses::RamsesRenderer& m_renderer;
    bool m_displayCreated = false;
};

TEST_F(ARamsesRenderer, canRunRendererInItsOwnThread)
{
    addDisplay();
    renderer.flush();
    EXPECT_EQ(ramses::StatusOK, renderer.startThread());
    SceneStateEventHandler eventHandler(renderer);
    eventHandler.waitForDisplayCreationEvent();
    EXPECT_EQ(ramses::StatusOK, renderer.stopThread());
}

TEST_F(ARamsesRenderer, canRunRendererInItsOwnThreadAndCallAPIMethods)
{
    const ramses::displayId_t displayId = addDisplay();
    renderer.flush();
    EXPECT_EQ(ramses::StatusOK, renderer.startThread());
    SceneStateEventHandler eventHandler(renderer);
    eventHandler.waitForDisplayCreationEvent();

    // most of these will fail but the purpose is to create and submit renderer commands for renderer running in another thread
    // thread sanitizer or other analyzer would catch race conditions when running this test
    renderer.getSceneControlAPI()->handlePickEvent(ramses::sceneId_t(0u), 1u, 2u);
    renderer.flush();

    renderer.readPixels(displayId, {}, 1u, 2u, 3u, 4u);
    const auto ob = renderer.createOffscreenBuffer(displayId, 1u, 1u, 4u);
    renderer.destroyOffscreenBuffer(displayId, ob);
    renderer.flush();

    renderer.setSurfaceVisibility(0u, true);
    renderer.setSurfaceOpacity(0u, 1.0f);
    renderer.setSurfaceRectangle(0u, 0, 0, 0, 0);
    renderer.takeSystemCompositorScreenshot("", -1);
    renderer.setFrameTimerLimits(10001u, 10000u, 10000u);
    renderer.setLayerVisibility(0u, true);
    renderer.flush();

    renderer.dispatchEvents(eventHandler);
    EXPECT_EQ(ramses::StatusOK, renderer.stopThread());
}

TEST_F(ARamsesRenderer, createsThreadedDisplayIfThreadStartedRightAfterCreation)
{
    // Confidence test for a pre-existing race between starting threaded renderer and command dispatcher
    // processing createDisplay command before display thread started - ending up in non-threaded display creation.
    // 1. generate createDisplay cmd + flush
    // 2. start 'general renderer' thread
    //   a. threaded mode for displays is enabled
    //   b. start dispatcher thread and process command
    //   c. display is created in threaded mode
    // If a) would come last then display would be created non-threaded and won't be updated (and dispatcher asserts).
    // Current implementation makes sure this does not happen, this test is an attempt (cannot be reliably reproduced) to catch regression.

    EXPECT_TRUE(renderer.createDisplay({}).isValid());
    EXPECT_EQ(ramses::StatusOK, renderer.flush());
    EXPECT_EQ(ramses::StatusOK, renderer.startThread());
    EXPECT_EQ(ramses::StatusOK, renderer.stopThread());
}

TEST(ARamsesRendererWithSeparateRendererThread, canNotifyPerWatchdog)
{
    ramses_internal::PlatformLock expectLock;
    SafeThreadWatchdogNotificationMock notificationMock(expectLock);

    ramses::RamsesFrameworkConfig frameworkConfig;
    EXPECT_EQ(ramses::StatusOK, frameworkConfig.setWatchdogNotificationInterval(ramses::ERamsesThreadIdentifier_Renderer, 500));
    EXPECT_EQ(ramses::StatusOK, frameworkConfig.setWatchdogNotificationCallBack(&notificationMock));
    {
        ramses_internal::PlatformGuard g(expectLock);
        EXPECT_CALL(notificationMock, safe_registerThread(ramses::ERamsesThreadIdentifier_Workers)).Times(1);
        EXPECT_CALL(notificationMock, safe_notifyThread(ramses::ERamsesThreadIdentifier_Workers)).Times(AtLeast(1));
    }
    ramses::RamsesFramework framework(frameworkConfig);

    {
        ramses_internal::PlatformGuard g(expectLock);
        EXPECT_CALL(notificationMock, safe_registerThread(ramses::ERamsesThreadIdentifier_Renderer)).Times(1);
    }

    // syncWaiter must outlive renderer because renderer calls syncWaiter
    // via mock until its dtor has run (only then thread gets really stopped!)
    ramses_internal::PlatformEvent syncWaiter;
    ramses::RamsesRenderer& renderer(*framework.createRenderer(ramses::RendererConfig()));
    {
        ramses_internal::PlatformGuard g(expectLock);
        EXPECT_CALL(notificationMock, safe_notifyThread(ramses::ERamsesThreadIdentifier_Renderer))
            .Times(AtLeast(1))
            .WillRepeatedly(InvokeWithoutArgs([&]() { syncWaiter.signal(); }));
    }
    EXPECT_EQ(ramses::StatusOK, renderer.startThread());

    EXPECT_TRUE(syncWaiter.wait(60000));

    {
        ramses_internal::PlatformGuard g(expectLock);
        EXPECT_CALL(notificationMock, safe_unregisterThread(ramses::ERamsesThreadIdentifier_Workers)).Times(1);
        EXPECT_CALL(notificationMock, safe_unregisterThread(ramses::ERamsesThreadIdentifier_Renderer)).Times(1);
    }
    EXPECT_EQ(ramses::StatusOK, renderer.stopThread());
    framework.destroyRenderer(renderer);
}

class RenderThreadLoopTimingsNotification final : public ramses::RendererEventHandlerEmpty
{
public:
    void renderThreadLoopTimings(std::chrono::microseconds, std::chrono::microseconds) override
    {
        timingReported = true;
    }
    bool timingReported = false;
};

TEST(ARamsesRendererNonThreaded, reportsFrameTimings)
{
    ramses::RamsesFramework framework;
    ramses::RendererConfig rConfig;
    rConfig.setRenderThreadLoopTimingReportingPeriod(std::chrono::milliseconds{ 50 });
    ramses::RamsesRenderer& renderer(*framework.createRenderer(rConfig));

    renderer.createDisplay({});
    renderer.flush();

    // wait for either 60 seconds or until event was received
    RenderThreadLoopTimingsNotification eventHandler;
    const auto startTS = std::chrono::steady_clock::now();
    while (!eventHandler.timingReported && std::chrono::steady_clock::now() - startTS < std::chrono::minutes{ 1 })
    {
        renderer.doOneLoop();
        renderer.dispatchEvents(eventHandler);
    }
    EXPECT_TRUE(eventHandler.timingReported);

    framework.destroyRenderer(renderer);
}

TEST(ARamsesRendererWithSeparateRendererThread, reportsFrameTimings)
{
    ramses::RamsesFramework framework;
    ramses::RendererConfig rConfig;
    rConfig.setRenderThreadLoopTimingReportingPeriod(std::chrono::milliseconds{ 50 });
    ramses::RamsesRenderer& renderer(*framework.createRenderer(rConfig));

    renderer.createDisplay({});
    renderer.flush();

    EXPECT_EQ(ramses::StatusOK, renderer.startThread());

    // wait for either 60 seconds or until event was received
    RenderThreadLoopTimingsNotification eventHandler;
    const auto startTS = std::chrono::steady_clock::now();
    while (!eventHandler.timingReported && std::chrono::steady_clock::now() - startTS < std::chrono::minutes{ 1 })
    {
        renderer.dispatchEvents(eventHandler);
        std::this_thread::sleep_for(std::chrono::milliseconds{ 5 });
    }
    EXPECT_TRUE(eventHandler.timingReported);

    EXPECT_EQ(ramses::StatusOK, renderer.stopThread());
    framework.destroyRenderer(renderer);
}

TEST(ARamsesRendererWithSeparateRendererThread, willNotReportsFrameTimingsIfDisabled)
{
    ramses::RamsesFramework framework;
    ramses::RendererConfig rConfig;
    rConfig.setRenderThreadLoopTimingReportingPeriod(std::chrono::milliseconds{ 0 });
    ramses::RamsesRenderer& renderer(*framework.createRenderer(rConfig));

    renderer.createDisplay({});
    renderer.flush();

    EXPECT_EQ(ramses::StatusOK, renderer.startThread());

    RenderThreadLoopTimingsNotification eventHandler;
    const auto startTS = std::chrono::steady_clock::now();
    while (!eventHandler.timingReported && std::chrono::steady_clock::now() - startTS < std::chrono::seconds{ 10 })
    {
        renderer.dispatchEvents(eventHandler);
        std::this_thread::sleep_for(std::chrono::milliseconds{ 5 });
    }
    EXPECT_FALSE(eventHandler.timingReported);

    EXPECT_EQ(ramses::StatusOK, renderer.stopThread());
    framework.destroyRenderer(renderer);
}

TEST(ARamsesRendererWithSeparateRendererThread, TSAN_periodicallyDispatchEvents)
{
    // this test is meant for TSAN
    // - run renderer with display in thread with 30fps
    // - keep dispatching events with higher frequency

    ramses::RamsesFramework framework;
    ramses::RamsesRenderer& renderer(*framework.createRenderer({}));
    ramses::RamsesClient& client(*framework.createClient({}));
    framework.connect();

    auto scene = client.createScene(ramses::sceneId_t{ 321u });
    scene->createNode();
    scene->flush();
    scene->publish(ramses::EScenePublicationMode_LocalOnly);

    renderer.setFrameTimerLimits(std::numeric_limits<uint64_t>::max(), std::numeric_limits<uint64_t>::max(), std::numeric_limits<uint64_t>::max());
    renderer.setLoopMode(ramses::ELoopMode_UpdateOnly);
    renderer.setMaximumFramerate(30.f);
    renderer.createDisplay({});
    renderer.flush();

    renderer.startThread();

    ramses::RendererEventHandlerEmpty dummyHandler;
    ramses::RendererSceneControlEventHandlerEmpty dummyHandler2;
    const auto startTS = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - startTS < std::chrono::seconds{ 5 })
    {
        renderer.dispatchEvents(dummyHandler);
        renderer.getSceneControlAPI()->dispatchEvents(dummyHandler2);
        std::this_thread::sleep_for(std::chrono::milliseconds{ 5 });
    }

    renderer.stopThread();
    framework.disconnect();
    framework.destroyRenderer(renderer);
}

TEST_F(ARamsesRendererWithDisplay, createsCommandForSettingFrameTimerLimits)
{
    EXPECT_EQ(ramses::StatusOK, renderer.setFrameTimerLimits(10001u, 10002u, 10003u));
    EXPECT_CALL(cmdVisitor, setLimitsFrameBudgets(10001u, 10002u, 10003u));
    cmdVisitor.visit(commandBuffer);
}

TEST_F(ARamsesRendererWithDisplay, createsCommandForSettingSkippingUnmodifiedBuffersFeature)
{
    EXPECT_EQ(ramses::StatusOK, renderer.setSkippingOfUnmodifiedBuffers(true));
    EXPECT_CALL(cmdVisitor, setSkippingOfUnmodifiedBuffers(true));
    cmdVisitor.visit(commandBuffer);
}

TEST_F(ARamsesRendererWithDisplay, createsCommandForSettingClearFlags_FB)
{
    EXPECT_EQ(ramses::StatusOK, renderer.setDisplayBufferClearFlags(renderer, displayId, renderer.getDisplayFramebuffer(displayId), ramses::EClearFlags_Color));
    EXPECT_CALL(cmdVisitor, handleSetClearFlags(ramses_internal::DisplayHandle{ displayId.getValue() }, ramses_internal::OffscreenBufferHandle{}, ramses::EClearFlags_Color));
    cmdVisitor.visit(commandBuffer);
}

TEST_F(ARamsesRendererWithDisplay, createsCommandForSettingClearFlags_FBImplicitlyUsingInvalidDisplayBuffer)
{
    EXPECT_EQ(ramses::StatusOK, renderer.setDisplayBufferClearFlags(renderer, displayId, ramses::displayBufferId_t::Invalid(), ramses::EClearFlags_Color));
    EXPECT_CALL(cmdVisitor, handleSetClearFlags(ramses_internal::DisplayHandle{ displayId.getValue() }, ramses_internal::OffscreenBufferHandle{}, ramses::EClearFlags_Color));
    cmdVisitor.visit(commandBuffer);
}

TEST_F(ARamsesRendererWithDisplay, createsCommandForSettingClearFlags_OB)
{
    EXPECT_EQ(ramses::StatusOK, renderer.setDisplayBufferClearFlags(renderer, displayId, ramses::displayBufferId_t{ 666u }, ramses::EClearFlags_Color));
    EXPECT_CALL(cmdVisitor, handleSetClearFlags(ramses_internal::DisplayHandle{ displayId.getValue() }, ramses_internal::OffscreenBufferHandle{ 666u }, ramses::EClearFlags_Color));
    cmdVisitor.visit(commandBuffer);
}

TEST_F(ARamsesRendererWithDisplay, reportsErrorIfSettingClearFlagsForUnknownDisplay)
{
    EXPECT_NE(ramses::StatusOK, renderer.setDisplayBufferClearFlags(renderer, ramses::displayId_t{ 999u }, {}, ramses::EClearFlags_Color));
}

TEST_F(ARamsesRendererWithDisplay, createsCommandForSettingClearColor_FB)
{
    EXPECT_EQ(ramses::StatusOK, renderer.setDisplayBufferClearColor(displayId, renderer.getDisplayFramebuffer(displayId), 1, 2, 3, 4));
    EXPECT_CALL(cmdVisitor, handleSetClearColor(ramses_internal::DisplayHandle{ displayId.getValue() }, ramses_internal::OffscreenBufferHandle{}, ramses_internal::Vector4{ 1, 2, 3, 4 }));
    cmdVisitor.visit(commandBuffer);
}

TEST_F(ARamsesRendererWithDisplay, createsCommandForSettingClearColor_FBImplicitlyUsingInvalidDisplayBuffer)
{
    EXPECT_EQ(ramses::StatusOK, renderer.setDisplayBufferClearColor(displayId, ramses::displayBufferId_t::Invalid(), 1, 2, 3, 4));
    EXPECT_CALL(cmdVisitor, handleSetClearColor(ramses_internal::DisplayHandle{ displayId.getValue() }, ramses_internal::OffscreenBufferHandle{}, ramses_internal::Vector4{ 1, 2, 3, 4 }));
    cmdVisitor.visit(commandBuffer);
}

TEST_F(ARamsesRendererWithDisplay, createsCommandForSettingClearColor_OB)
{
    EXPECT_EQ(ramses::StatusOK, renderer.setDisplayBufferClearColor(displayId, ramses::displayBufferId_t{ 666u }, 1, 2, 3, 4));
    EXPECT_CALL(cmdVisitor, handleSetClearColor(ramses_internal::DisplayHandle{ displayId.getValue() }, ramses_internal::OffscreenBufferHandle{ 666u }, ramses_internal::Vector4{ 1, 2, 3, 4 }));
    cmdVisitor.visit(commandBuffer);
}

TEST_F(ARamsesRendererWithDisplay, reportsErrorIfSettingClearColorForUnknownDisplay)
{
    EXPECT_NE(ramses::StatusOK, renderer.setDisplayBufferClearColor(ramses::displayId_t{ 999u }, {}, 1, 2, 3, 4));
}

TEST_F(ARamsesRendererWithDisplay, createsCommandForSettingExternallyOwnedWindowSize)
{
    EXPECT_EQ(ramses::StatusOK, renderer.setExternallyOwnedWindowSize(displayId, 123u, 456u));
    EXPECT_CALL(cmdVisitor, handleSetExternallyOwnedWindowSize(ramses_internal::DisplayHandle{ displayId.getValue() }, 123u, 456u));
    cmdVisitor.visit(commandBuffer);
}

TEST_F(ARamsesRendererWithDisplay, reportsErrorIfSettingExternallyOwnedWindowSizeForUnknownDisplay)
{
    EXPECT_NE(ramses::StatusOK, renderer.setExternallyOwnedWindowSize({}, 123u, 456u));
}

TEST(ARamsesFrameworkInARendererLib, canCreateARenderer)
{
    ramses::RamsesFramework fw;

    auto renderer = fw.createRenderer(ramses::RendererConfig());
    EXPECT_NE(nullptr, renderer);
}

TEST(ARamsesFrameworkInARendererLib, canNotCreateMultipleRenderer)
{
    ramses::RamsesFramework fw;

    auto renderer1 = fw.createRenderer(ramses::RendererConfig());
    auto renderer2 = fw.createRenderer(ramses::RendererConfig());
    EXPECT_NE(nullptr, renderer1);
    EXPECT_EQ(nullptr, renderer2);
}

TEST(ARamsesFrameworkInARendererLib, acceptsLocallyCreatedRendererForDestruction)
{
    ramses::RamsesFramework fw;

    auto renderer = fw.createRenderer(ramses::RendererConfig());
    EXPECT_EQ(ramses::StatusOK, fw.destroyRenderer(*renderer));
}

TEST(ARamsesFrameworkInARendererLib, doesNotAcceptForeignCreatedRendererForDestruction)
{
    ramses::RamsesFramework fw1;
    ramses::RamsesFramework fw2;

    auto renderer1 = fw1.createRenderer(ramses::RendererConfig());
    auto renderer2 = fw2.createRenderer(ramses::RendererConfig());
    EXPECT_NE(ramses::StatusOK, fw2.destroyRenderer(*renderer1));
    EXPECT_NE(ramses::StatusOK, fw1.destroyRenderer(*renderer2));
}

TEST(ARamsesFrameworkInARendererLib, doesNotAcceptSameRendererTwiceForDestruction)
{
    ramses::RamsesFramework fw;

    auto renderer = fw.createRenderer(ramses::RendererConfig());
    EXPECT_EQ(ramses::StatusOK, fw.destroyRenderer(*renderer));
    EXPECT_NE(ramses::StatusOK, fw.destroyRenderer(*renderer));
}

TEST(ARamsesFrameworkInARendererLib, canCreateDestroyAndRecreateARenderer)
{
    ramses::RamsesFramework fw;

    auto renderer = fw.createRenderer(ramses::RendererConfig());
    EXPECT_NE(nullptr, renderer);
    EXPECT_EQ(fw.destroyRenderer(*renderer), ramses::StatusOK);
    renderer = fw.createRenderer(ramses::RendererConfig());
    EXPECT_NE(nullptr, renderer);
}

TEST(ARamsesFrameworkInARendererLib, createRendererFailsWhenConnected)
{
    ramses::RamsesFramework framework;
    EXPECT_EQ(ramses::StatusOK, framework.connect());
    EXPECT_EQ(framework.createRenderer(ramses::RendererConfig()), nullptr);
}

TEST(ARamsesFrameworkInARendererLib, destroyRendererFailsWhenConnected)
{
    ramses::RamsesFramework framework;
    auto* renderer = framework.createRenderer(ramses::RendererConfig());
    ASSERT_NE(renderer, nullptr);
    EXPECT_EQ(ramses::StatusOK, framework.connect());
    EXPECT_NE(ramses::StatusOK, framework.destroyRenderer(*renderer));
    EXPECT_EQ(ramses::StatusOK, framework.disconnect());
    EXPECT_EQ(ramses::StatusOK, framework.destroyRenderer(*renderer));
}
