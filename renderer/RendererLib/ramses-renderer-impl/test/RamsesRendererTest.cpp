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
#include "ramses-renderer-api/DcsmContentControlConfig.h"
#include "ramses-framework-api/RamsesFrameworkConfig.h"
#include "ramses-framework-api/RamsesFramework.h"

#include "RendererLib/RendererCommands.h"
#include "RamsesRendererImpl.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "PlatformAbstraction/PlatformEvent.h"

using namespace testing;

class RenderThreadLoopTimingsNotificationMock : public ramses::RendererEventHandlerEmpty
{
public:
    MOCK_METHOD(void, renderThreadLoopTimings, (std::chrono::microseconds maximumLoopTimeMilliseconds, std::chrono::microseconds average), (override));
};

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

    void checkForRendererCommand(uint32_t index, ramses_internal::ERendererCommand commandType)
    {
        ASSERT_LT(index, commandBuffer.getCommands().getTotalCommandCount());
        EXPECT_EQ(commandType, commandBuffer.getCommands().getCommandType(index));
    }

    void checkForRendererCommandCount(uint32_t count)
    {
        EXPECT_EQ(count, commandBuffer.getCommands().getTotalCommandCount());
    }

protected:
    ramses::RamsesFramework framework;
    ramses::RamsesRenderer& renderer;
    const ramses_internal::RendererCommands& commandBuffer;
    ramses::DcsmContentControlConfig m_dcsmContentControlConfig{ { ramses::Category{ 123u }, ramses::DcsmContentControlConfig::CategoryInfo{ ramses::SizeInfo{ 1u, 2u }, ramses::displayId_t{} } } };
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
    checkForRendererCommandCount(0u);
}

TEST_F(ARamsesRenderer, canOnlyGetOneSceneControlAPI_RendererSceneControl)
{
    const auto api = renderer.getSceneControlAPI();
    EXPECT_TRUE(api != nullptr);
    EXPECT_EQ(api, renderer.getSceneControlAPI());
    EXPECT_EQ(api, renderer.getSceneControlAPI());

    EXPECT_TRUE(renderer.getSceneControlAPI_legacy() == nullptr);
    EXPECT_TRUE(renderer.getSceneControlAPI_legacy() == nullptr);
    EXPECT_TRUE(renderer.createDcsmContentControl(m_dcsmContentControlConfig) == nullptr);
    EXPECT_TRUE(renderer.createDcsmContentControl(m_dcsmContentControlConfig) == nullptr);
}

TEST_F(ARamsesRenderer, canOnlyGetOneSceneControlAPI_RendererSceneControl_legacy)
{
    const auto api = renderer.getSceneControlAPI_legacy();
    EXPECT_TRUE(api != nullptr);
    EXPECT_EQ(api, renderer.getSceneControlAPI_legacy());
    EXPECT_EQ(api, renderer.getSceneControlAPI_legacy());

    EXPECT_TRUE(renderer.getSceneControlAPI() == nullptr);
    EXPECT_TRUE(renderer.getSceneControlAPI() == nullptr);
    EXPECT_TRUE(renderer.createDcsmContentControl(m_dcsmContentControlConfig) == nullptr);
    EXPECT_TRUE(renderer.createDcsmContentControl(m_dcsmContentControlConfig) == nullptr);
}

TEST_F(ARamsesRenderer, canOnlyGetOneSceneControlAPI_DcsmContentControl)
{
    EXPECT_TRUE(renderer.createDcsmContentControl(m_dcsmContentControlConfig) != nullptr);

    EXPECT_TRUE(renderer.getSceneControlAPI() == nullptr);
    EXPECT_TRUE(renderer.getSceneControlAPI() == nullptr);
    EXPECT_TRUE(renderer.getSceneControlAPI_legacy() == nullptr);
    EXPECT_TRUE(renderer.getSceneControlAPI_legacy() == nullptr);
}

TEST_F(ARamsesRenderer, canCreateDcsmContentControlOnlyOnce)
{
    EXPECT_TRUE(renderer.createDcsmContentControl(m_dcsmContentControlConfig) != nullptr);

    EXPECT_TRUE(renderer.createDcsmContentControl(m_dcsmContentControlConfig) == nullptr);
    EXPECT_TRUE(renderer.createDcsmContentControl(m_dcsmContentControlConfig) == nullptr);
}

TEST_F(ARamsesRenderer, failsToCreateDcsmContentControlWithNoCategoryInConfig)
{
    EXPECT_EQ(nullptr, renderer.createDcsmContentControl(ramses::DcsmContentControlConfig{}));
}

/*
 * Display
 */
TEST_F(ARamsesRenderer, createsACommandForDisplayCreation)
{
    ramses::displayId_t displayId = addDisplay();

    EXPECT_NE(ramses::displayId_t::Invalid(), displayId);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_CreateDisplay);
    checkForRendererCommandCount(1u);
}

TEST_F(ARamsesRenderer, createsNoCommandForDisplayCreationWithInvalidConfig)
{
    ramses::DisplayConfig config;
    EXPECT_EQ(ramses::StatusOK, config.setPerspectiveProjection(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f));
    EXPECT_NE(ramses::StatusOK, config.validate());
    ramses::displayId_t displayId = renderer.createDisplay(config);

    EXPECT_EQ(ramses::displayId_t::Invalid(), displayId);
    checkForRendererCommandCount(0u);
}

TEST_F(ARamsesRenderer, createsMultipleCommandsForMultipleDisplayCreation)
{
    checkForRendererCommandCount(0u);
    EXPECT_NE(ramses::displayId_t::Invalid(), addDisplay());
    EXPECT_NE(ramses::displayId_t::Invalid(), addDisplay());
    EXPECT_NE(ramses::displayId_t::Invalid(), addDisplay());

    checkForRendererCommandCount(3u);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_CreateDisplay);
    checkForRendererCommand(1u, ramses_internal::ERendererCommand_CreateDisplay);
    checkForRendererCommand(2u, ramses_internal::ERendererCommand_CreateDisplay);
}

TEST_F(ARamsesRendererWithDisplay, createsCommandForDisplayDestruction)
{
    EXPECT_EQ(ramses::StatusOK, renderer.destroyDisplay(displayId));
    checkForRendererCommandCount(1u);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_DestroyDisplay);
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
    checkForRendererCommandCount(1u);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_LogRendererInfo);
}

/*
* Update warping data
*/
TEST_F(ARamsesRenderer, createsNoCommandForWarpingDataUpdateOnInvalidDisplay)
{
    ramses::WarpingMeshData warpingData(0u, nullptr, 0u, nullptr, nullptr);

    EXPECT_NE(ramses::StatusOK, renderer.updateWarpingMeshData(ramses::displayId_t(0u), warpingData));
    checkForRendererCommandCount(0u);
}

TEST_F(ARamsesRendererWithDisplay, createsNoCommandForWarpingDataUpdateOnDisplayWithoutWarping)
{
    ramses::WarpingMeshData warpingData(0u, nullptr, 0u, nullptr, nullptr);

    EXPECT_NE(ramses::StatusOK, renderer.updateWarpingMeshData(displayId, warpingData));
    checkForRendererCommandCount(0u);
}

TEST_F(ARamsesRendererWithDisplay, createsNoCommandForInvalidWarpingDataUpdateOnWarpingDisplay)
{
    ramses::WarpingMeshData warpingData(0u, nullptr, 0u, nullptr, nullptr);

    EXPECT_NE(ramses::StatusOK, renderer.updateWarpingMeshData(warpingDisplayId, warpingData));
    checkForRendererCommandCount(0u);
}

TEST_F(ARamsesRendererWithDisplay, createsCommandForValidWarpingDataUpdateOnWarpingDisplay)
{
    //sample warping data
    const uint16_t indices[] = { 0u, 1u, 2u };
    const float vertices[] = { -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f };
    const float texcoords[] = { 0.0f, 1.0f, 1.0f, 1.0f, 0.5f, 0.0f };
    ramses::WarpingMeshData warpingData(3u, indices, 3u, vertices, texcoords);

    EXPECT_EQ(ramses::StatusOK, renderer.updateWarpingMeshData(warpingDisplayId, warpingData));
    checkForRendererCommandCount(1u);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_UpdateWarpingData);
}

/*
* Offscreen buffers
*/
TEST_F(ARamsesRendererWithDisplay, createsCommandForOffscreenBufferCreate)
{
    EXPECT_NE(ramses::displayBufferId_t::Invalid(), renderer.createOffscreenBuffer(displayId, 40u, 40u));
    checkForRendererCommandCount(1u);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_CreateOffscreenBuffer);
}

TEST_F(ARamsesRendererWithDisplay, createsCommandForOffscreenBufferDestroy)
{
    const ramses::displayBufferId_t bufferId(0u);
    EXPECT_EQ(ramses::StatusOK, renderer.destroyOffscreenBuffer(displayId, bufferId));
    checkForRendererCommandCount(1u);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_DestroyOffscreenBuffer);
}

TEST_F(ARamsesRendererWithDisplay, failsToCreateOffscreenBufferWithUnsupportedResolution)
{
    EXPECT_EQ(ramses::displayBufferId_t::Invalid(), renderer.createOffscreenBuffer(displayId, 0u, 1u));
    EXPECT_EQ(ramses::displayBufferId_t::Invalid(), renderer.createOffscreenBuffer(displayId, 1u, 0u));
    EXPECT_EQ(ramses::displayBufferId_t::Invalid(), renderer.createOffscreenBuffer(displayId, 0u, 0u));
    EXPECT_EQ(ramses::displayBufferId_t::Invalid(), renderer.createOffscreenBuffer(displayId, 5000u, 1u));
    EXPECT_EQ(ramses::displayBufferId_t::Invalid(), renderer.createOffscreenBuffer(displayId, 1u, 5000u));
    EXPECT_EQ(ramses::displayBufferId_t::Invalid(), renderer.createOffscreenBuffer(displayId, 5000u, 5000u));
}

/*
* Read Pixels
*/
TEST_F(ARamsesRendererWithDisplay, createsCommandForReadPixels)
{
    EXPECT_EQ(ramses::StatusOK, renderer.readPixels(displayId, {}, 0u, 0u, 40u, 40u));
    checkForRendererCommandCount(1u);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_ReadPixels);
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
    checkForRendererCommandCount(0u);
}

TEST_F(ARamsesRendererWithSystemCompositorController, createsCommandForSystemCompositorControllerSetSurfaceVisibility)
{
    EXPECT_EQ(ramses::StatusOK, renderer.setSurfaceVisibility(0, true));
    checkForRendererCommandCount(1u);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_SystemCompositorControllerSetIviSurfaceVisibility);
}

TEST_F(ARamsesRendererWithSystemCompositorController, createsCommandForSystemCompositorControllerSetSurfaceOpacity)
{
    EXPECT_EQ(ramses::StatusOK, renderer.setSurfaceOpacity(0, 0.2f));
    checkForRendererCommandCount(1u);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_SystemCompositorControllerSetIviSurfaceOpacity);
}

TEST_F(ARamsesRendererWithSystemCompositorController, createsCommandForSystemCompositorControllerSetSurfaceRectangle)
{
    EXPECT_EQ(ramses::StatusOK, renderer.setSurfaceRectangle(0, 1, 2, 3, 4));
    checkForRendererCommandCount(1u);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_SystemCompositorControllerSetIviSurfaceDestRectangle);
}

TEST_F(ARamsesRendererWithSystemCompositorController, createsCommandForSystemCompositorControllerAddSurfaceToLayer)
{
    EXPECT_EQ(ramses::StatusOK, renderer.impl.systemCompositorAddIviSurfaceToIviLayer(0, 1));
    checkForRendererCommandCount(1u);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_SystemCompositorControllerAddIviSurfaceToIviLayer);
}

TEST_F(ARamsesRendererWithSystemCompositorController, createsCommandForSystemCompositorControllerSetLayerVisibility)
{
    EXPECT_EQ(ramses::StatusOK, renderer.setLayerVisibility(17, true));
    checkForRendererCommandCount(1u);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_SystemCompositorControllerSetIviLayerVisibility);
}

TEST_F(ARamsesRendererWithSystemCompositorController, createsCommandForSystemCompositorControllerTakeScreenshot)
{
    EXPECT_EQ(ramses::StatusOK, renderer.takeSystemCompositorScreenshot("unused_name", -1));
    checkForRendererCommandCount(1u);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_SystemCompositorControllerScreenshot);
}

/*
 * Threading and thread sanitizer tests
 */
TEST_F(ARamsesRenderer, canRunRendererInItsOwnThread)
{
    class SceneStateEventHandler : public ramses::RendererEventHandlerEmpty
    {
    public:
        explicit SceneStateEventHandler(ramses::RamsesRenderer& renderer)
            : m_renderer(renderer)
            , m_displayCreationEventFired(false)
        {
        }

        virtual void displayCreated(ramses::displayId_t /*displayId*/, ramses::ERendererEventResult /*result*/)
        {
            m_displayCreationEventFired = true;
        }

        void waitForDisplayCreationEvent()
        {
            do {
                m_renderer.dispatchEvents(*this);
            } while (!m_displayCreationEventFired);
        }

    private:
        ramses::RamsesRenderer& m_renderer;
        ramses_internal::Bool m_displayCreationEventFired;
    };

    const ramses::displayId_t displayId = addDisplay();
    renderer.flush();
    ASSERT_FALSE(renderer.impl.getRenderer().getRenderer().hasDisplayController(ramses_internal::DisplayHandle(displayId.getValue())));
    EXPECT_EQ(ramses::StatusOK, renderer.startThread());
    SceneStateEventHandler eventHandler(renderer);
    eventHandler.waitForDisplayCreationEvent();

    EXPECT_TRUE(renderer.impl.getRenderer().getRenderer().hasDisplayController(ramses_internal::DisplayHandle(displayId.getValue())));
    EXPECT_EQ(ramses::StatusOK, renderer.stopThread());
}

TEST_F(ARamsesRenderer, canRunRendererInItsOwnThreadAndCallAPIMethods)
{
    class SceneStateEventHandler : public ramses::RendererEventHandlerEmpty
    {
    public:
        explicit SceneStateEventHandler(ramses::RamsesRenderer& renderer)
            : m_renderer(renderer)
            , m_displayCreationEventFired(false)
        {
        }

        virtual void displayCreated(ramses::displayId_t /*displayId*/, ramses::ERendererEventResult /*result*/)
        {
            m_displayCreationEventFired = true;
        }

        void waitForDisplayCreationEvent()
        {
            do {
                m_renderer.dispatchEvents(*this);
                std::this_thread::sleep_for(std::chrono::milliseconds{5});
            } while (!m_displayCreationEventFired);
        }

    private:
        ramses::RamsesRenderer& m_renderer;
        ramses_internal::Bool m_displayCreationEventFired;
    };

    const ramses::displayId_t displayId = addDisplay();
    renderer.flush();
    ASSERT_FALSE(renderer.impl.getRenderer().getRenderer().hasDisplayController(ramses_internal::DisplayHandle(displayId.getValue())));
    EXPECT_EQ(ramses::StatusOK, renderer.startThread());
    SceneStateEventHandler eventHandler(renderer);
    eventHandler.waitForDisplayCreationEvent();
    EXPECT_TRUE(renderer.impl.getRenderer().getRenderer().hasDisplayController(ramses_internal::DisplayHandle(displayId.getValue())));

    // most of these will fail but the purpose is to create and submit renderer commands for renderer running in another thread
    // thread sanitizer or other analyzer would catch race conditions when running this test
    renderer.getSceneControlAPI()->handlePickEvent(ramses::sceneId_t(0u), 1u, 2u);
    renderer.flush();

    renderer.readPixels(displayId, {}, 1u, 2u, 3u, 4u);
    const auto ob = renderer.createOffscreenBuffer(displayId, 1u, 1u);
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

TEST(ARamsesRendererWithSeparateRendererThread, firesRenderThreadPeriodTimingsEvent)
{
    RenderThreadLoopTimingsNotificationMock notificationMock;

    ramses::RamsesFramework framework;
    ramses::RendererConfig rConfig;
    std::chrono::milliseconds period{ 50 };
    rConfig.setRenderThreadLoopTimingReportingPeriod(period);
    ramses::RamsesRenderer& renderer(*framework.createRenderer(rConfig));
    // syncWaiter must outlive renderer because renderer calls syncWaiter
        // via mock until its dtor has run (only then thread gets really stopped!)
    ramses_internal::PlatformEvent syncWaiter;

    EXPECT_CALL(notificationMock, renderThreadLoopTimings(_, _)).Times(AtLeast(1)).WillRepeatedly(InvokeWithoutArgs([&]() { syncWaiter.signal(); }));
    EXPECT_EQ(ramses::StatusOK, renderer.startThread());

    bool eventReceived = false;
    // wait for either 60 seconds or until event was received (signalled)
    int i = 0;
    const auto timesToRunToWaitFor60Seconds = 60 * 1000 / period.count();
    while (i < timesToRunToWaitFor60Seconds && !eventReceived)
    {
        renderer.dispatchEvents(notificationMock);
        eventReceived = syncWaiter.wait(1);
        i++;
    }

    EXPECT_EQ(ramses::StatusOK, renderer.stopThread());
    framework.destroyRenderer(renderer);
}

TEST_F(ARamsesRendererWithDisplay, createsCommandForSettingFrameTimerLimits)
{
    EXPECT_EQ(ramses::StatusOK, renderer.setFrameTimerLimits(10001u, 10000u, 10000u));

    checkForRendererCommandCount(1u);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_SetFrameTimerLimits);
}

TEST_F(ARamsesRendererWithDisplay, createsCommandForSettingSkippingUnmodifiedBuffersFeature)
{
    EXPECT_EQ(ramses::StatusOK, renderer.setSkippingOfUnmodifiedBuffers(true));

    checkForRendererCommandCount(1u);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_SetSkippingOfUnmodifiedBuffers);
}

TEST_F(ARamsesRendererWithDisplay, createsCommandForSettingClearColor_FB)
{
    EXPECT_EQ(ramses::StatusOK, renderer.setDisplayBufferClearColor(displayId, renderer.getDisplayFramebuffer(displayId), 1, 2, 3, 4));

    checkForRendererCommandCount(1u);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_SetClearColor);
    const auto& cmd = commandBuffer.getCommands().getCommandData<ramses_internal::SetClearColorCommand>(0u);
    EXPECT_EQ(displayId.getValue(), cmd.displayHandle.asMemoryHandle());
    EXPECT_EQ(ramses_internal::OffscreenBufferHandle::Invalid(), cmd.obHandle); // invalid OB means display's FB internally
    EXPECT_EQ(ramses_internal::Vector4(1, 2, 3, 4), cmd.clearColor);
}

TEST_F(ARamsesRendererWithDisplay, createsCommandForSettingClearColor_FBImplicitlyUsingInvalidDisplayBuffer)
{
    EXPECT_EQ(ramses::StatusOK, renderer.setDisplayBufferClearColor(displayId, ramses::displayBufferId_t::Invalid(), 1, 2, 3, 4));

    checkForRendererCommandCount(1u);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_SetClearColor);
    const auto& cmd = commandBuffer.getCommands().getCommandData<ramses_internal::SetClearColorCommand>(0u);
    EXPECT_EQ(displayId.getValue(), cmd.displayHandle.asMemoryHandle());
    EXPECT_EQ(ramses_internal::OffscreenBufferHandle::Invalid(), cmd.obHandle); // invalid OB means display's FB internally
    EXPECT_EQ(ramses_internal::Vector4(1, 2, 3, 4), cmd.clearColor);
}

TEST_F(ARamsesRendererWithDisplay, createsCommandForSettingClearColor_OB)
{
    EXPECT_EQ(ramses::StatusOK, renderer.setDisplayBufferClearColor(displayId, ramses::displayBufferId_t{ 666u }, 1, 2, 3, 4));

    checkForRendererCommandCount(1u);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_SetClearColor);
    const auto& cmd = commandBuffer.getCommands().getCommandData<ramses_internal::SetClearColorCommand>(0u);
    EXPECT_EQ(displayId.getValue(), cmd.displayHandle.asMemoryHandle());
    EXPECT_EQ(666u, cmd.obHandle.asMemoryHandle());
    EXPECT_EQ(ramses_internal::Vector4(1, 2, 3, 4), cmd.clearColor);
}

TEST_F(ARamsesRendererWithDisplay, reportsErrorIfSettingClearColorForUnknownDisplay)
{
    EXPECT_NE(ramses::StatusOK, renderer.setDisplayBufferClearColor(ramses::displayId_t{ 999u }, {}, 1, 2, 3, 4));
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
