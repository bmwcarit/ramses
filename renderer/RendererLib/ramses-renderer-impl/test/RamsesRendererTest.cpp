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

#include "RendererLib/RendererCommands.h"
#include "RamsesRendererImpl.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "PlatformAbstraction/PlatformGuard.h"
#include "PlatformAbstraction/PlatformEvent.h"

using namespace testing;

class SafeThreadWatchdogNotificationMock : public ramses::IThreadWatchdogNotification
{
public:
    SafeThreadWatchdogNotificationMock(ramses_internal::PlatformLock& lock)
        : m_lock(lock)
    {
    }

    MOCK_METHOD1(safe_notifyThread, void(ramses::ERamsesThreadIdentifier));
    MOCK_METHOD1(safe_registerThread, void(ramses::ERamsesThreadIdentifier));
    MOCK_METHOD1(safe_unregisterThread, void(ramses::ERamsesThreadIdentifier));

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

ACTION_P(ReleaseSyncCall, syncer)
{
    UNUSED(arg9);
    UNUSED(arg8);
    UNUSED(arg7);
    UNUSED(arg6);
    UNUSED(arg5);
    UNUSED(arg4);
    UNUSED(arg3);
    UNUSED(arg2);
    UNUSED(arg1);
    UNUSED(arg0);
    UNUSED(args);
    syncer->signal();
}

class ARamsesRenderer : public ::testing::Test
{
protected:
    ARamsesRenderer(ramses::RendererConfig rendererConfig = ramses::RendererConfig())
    : framework()
    , renderer(framework, rendererConfig)
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

    static void callAllApiCoreFunctions(ramses::RamsesRenderer& renderer)
    {
        const ramses::DisplayConfig displayConfig;
        ramses::WarpingMeshData warpingMeshData(0, 0, 0, 0, 0);
        ramses::RendererEventHandlerEmpty rendererEventHandler;

        renderer.getStatusMessage(ramses::StatusOK);
        renderer.createDisplay(displayConfig);
        renderer.destroyDisplay(0u);
        renderer.subscribeScene(0u);
        renderer.unsubscribeScene(0u);
        renderer.linkData(0u, 1u, 2u, 3u);
        renderer.unlinkData(0u, 1u);
        renderer.mapScene(0u, 1u);
        renderer.unmapScene(0u);
        renderer.showScene(0u);
        renderer.hideScene(0u);
        renderer.readPixels(0u, 1u, 2u, 3u, 4u);
        renderer.updateWarpingMeshData(0u,warpingMeshData);
        renderer.createOffscreenBuffer(0u, 1u, 1u);
        renderer.destroyOffscreenBuffer(0u, 0u);
        renderer.assignSceneToOffscreenBuffer(0u, 0u);
        renderer.assignSceneToFramebuffer(0u);
        renderer.linkOffscreenBufferToSceneData(0u, 0u, 0u);

        renderer.setSurfaceVisibility(0u , true);
        renderer.setSurfaceOpacity(0u, 1.0f);
        renderer.setSurfaceRectangle(0u, 0, 0, 0, 0);
        renderer.takeSystemCompositorScreenshot("", -1);
        renderer.setFrameTimerLimits(10001u, 10000u, 10000u, 10000u);
        renderer.setLayerVisibility(0u, true);

        renderer.flush();
        renderer.dispatchEvents(rendererEventHandler);
    }

    static void callAllApiFunctionsAndDoOneLoop(ramses::RamsesRenderer& renderer)
    {
        callAllApiCoreFunctions(renderer);
        renderer.doOneLoop();
    }

    static void callAllApiFunctionsInRendererThread(ramses::RamsesRenderer& renderer)
    {
        renderer.startThread();

        EXPECT_NE(ramses::StatusOK, renderer.setMaximumFramerate(0.0f));
        EXPECT_NE(ramses::StatusOK, renderer.setMaximumFramerate(-5.0f));
        EXPECT_EQ(ramses::StatusOK, renderer.setMaximumFramerate(60.0f));

        callAllApiCoreFunctions(renderer);
        renderer.stopThread();
    }

    void runThreadSanitizerConfidenceTest(ramses_internal::Runnable& runnable)
    {
        ramses_internal::PlatformThread thread1("RendThrdConf_t1");
        thread1.start(runnable);

        ramses_internal::PlatformThread thread2("RendThrdConf_t2");
        thread2.start(runnable);

        ramses_internal::PlatformThread thread3("RendThrdConf_t3");
        thread3.start(runnable);

        thread1.join();
        thread2.join();
        thread3.join();
    }

    class CallAllApiFunctionsRunnableUsingDoOneLoop: public ramses_internal::Runnable
    {
    public:
        CallAllApiFunctionsRunnableUsingDoOneLoop(ramses::RamsesRenderer& rendererToCall, ramses_internal::UInt32 callCount = 1u)
            : m_renderer(rendererToCall)
            , m_callCount(callCount)
        {
        }

        virtual void run()
        {
            for(ramses_internal::UInt32 i = 0u; i < m_callCount; ++i)
            {
                callAllApiFunctionsAndDoOneLoop(m_renderer);
            }
        }
    private:
        ramses::RamsesRenderer& m_renderer;
        ramses_internal::UInt32 m_callCount;
    };

    class CallAllApiFunctionsRunnableUsingRendererThread: public ramses_internal::Runnable
    {
    public:
        CallAllApiFunctionsRunnableUsingRendererThread(ramses::RamsesRenderer& rendererToCall, ramses_internal::UInt32 callCount = 1u)
            : m_renderer(rendererToCall)
            , m_callCount(callCount)
        {
        }

        virtual void run()
        {
            for(ramses_internal::UInt32 i = 0u; i < m_callCount; ++i)
            {
                callAllApiFunctionsInRendererThread(m_renderer);
            }
        }
    private:
        ramses::RamsesRenderer& m_renderer;
        ramses_internal::UInt32 m_callCount;
    };

protected:
    ramses::RamsesFramework framework;
    ramses::RamsesRenderer renderer;
    const ramses_internal::RendererCommands& commandBuffer;
};

class ARamsesRendererWithDisplay : public ARamsesRenderer
{

    void SetUp() override
    {
        displayId = addDisplay();
        warpingDisplayId = addDisplay(true);
        EXPECT_NE(ramses::InvalidDisplayId, displayId);
        EXPECT_NE(ramses::InvalidDisplayId, warpingDisplayId);
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

/*
 * Display
 */
TEST_F(ARamsesRenderer, createsACommandForDisplayCreation)
{
    ramses::displayId_t displayId = addDisplay();

    EXPECT_NE(ramses::InvalidDisplayId, displayId);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_CreateDisplay);
    checkForRendererCommandCount(1u);
}

TEST_F(ARamsesRenderer, createsNoCommandForDisplayCreationWithInvalidConfig)
{
    ramses::DisplayConfig config;
    config.enableStereoDisplay();
    config.enableWarpingPostEffect();
    EXPECT_NE(ramses::StatusOK, config.validate());
    ramses::displayId_t displayId = renderer.createDisplay(config);

    EXPECT_EQ(ramses::InvalidDisplayId, displayId);
    checkForRendererCommandCount(0u);
}

TEST_F(ARamsesRenderer, createsMultipleCommandsForMultipleDisplayCreation)
{
    checkForRendererCommandCount(0u);
    EXPECT_NE(ramses::InvalidDisplayId, addDisplay());
    EXPECT_NE(ramses::InvalidDisplayId, addDisplay());
    EXPECT_NE(ramses::InvalidDisplayId, addDisplay());

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

/*
* Flush
*/
TEST_F(ARamsesRenderer, clearsAllPendingCommandsWhenCallingFlush)
{
    addDisplay();
    checkForRendererCommandCount(1u);
    renderer.flush();
    checkForRendererCommandCount(0u);
}

/*
* Scene lifecycle
*/
TEST_F(ARamsesRenderer, createsCommandForAnySceneSubscription)
{
    ramses::sceneId_t scene(1u);
    EXPECT_EQ(ramses::StatusOK, renderer.subscribeScene(scene));
    checkForRendererCommandCount(1u);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_SubscribeScene);
}

TEST_F(ARamsesRenderer, createsCommandForAnySceneUnsubscription)
{
    ramses::sceneId_t scene(23u);
    EXPECT_EQ(ramses::StatusOK, renderer.unsubscribeScene(scene));
    checkForRendererCommandCount(1u);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_UnsubscribeScene);
}

TEST_F(ARamsesRenderer, createsCommandForAnySceneShow)
{
    ramses::sceneId_t scene(23u);
    // to be replaced with HL API call
    EXPECT_EQ(ramses::StatusOK, renderer.impl.showScene(scene));
    checkForRendererCommandCount(1u);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_ShowScene);
}

TEST_F(ARamsesRenderer, createsCommandForAnySceneHide)
{
    ramses::sceneId_t scene(23u);
    EXPECT_EQ(ramses::StatusOK, renderer.hideScene(scene));
    checkForRendererCommandCount(1u);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_HideScene);
}

TEST_F(ARamsesRendererWithDisplay, createsCommandForSceneMapping)
{
    ramses::sceneId_t scene(43u);

    EXPECT_EQ(ramses::StatusOK, renderer.mapScene(displayId, scene));
    checkForRendererCommandCount(1u);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_MapSceneToDisplay);
}

TEST_F(ARamsesRenderer, createsCommandForAnySceneUnmapping)
{
    ramses::sceneId_t scene(23u);

    EXPECT_EQ(ramses::StatusOK, renderer.unmapScene(scene));
    checkForRendererCommandCount(1u);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
}

/*
 * Transformation linking
 */
TEST_F(ARamsesRenderer, createsNoCommandForTransformDataLinkingWithinTheSameScene)
{
    ramses::sceneId_t scene(1u);
    ramses::dataProviderId_t providerId(3u);
    ramses::dataConsumerId_t consumerId(4u);

    EXPECT_NE(ramses::StatusOK, renderer.linkData(scene, providerId, scene, consumerId));
    checkForRendererCommandCount(0u);
}

TEST_F(ARamsesRenderer, createsCommandForTransformDataLinkingBetweenDifferentScenes)
{
    ramses::sceneId_t scene1(1u);
    ramses::sceneId_t scene2(2u);
    ramses::dataProviderId_t providerId(3u);
    ramses::dataConsumerId_t consumerId(4u);

    EXPECT_EQ(ramses::StatusOK, renderer.linkData(scene1, providerId, scene2, consumerId));
    checkForRendererCommandCount(1u);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_LinkSceneData);
}

TEST_F(ARamsesRenderer, createsComandForUnlinkTransformDataLink)
{
    ramses::sceneId_t scene(1u);
    ramses::dataConsumerId_t consumerId(4u);

    EXPECT_EQ(ramses::StatusOK, renderer.unlinkData(scene, consumerId));
    checkForRendererCommandCount(1u);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_UnlinkSceneData);
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
    ramses::WarpingMeshData warpingData(0u, NULL, 0u, NULL, NULL);

    EXPECT_NE(ramses::StatusOK, renderer.updateWarpingMeshData(ramses::displayId_t(0u), warpingData));
    checkForRendererCommandCount(0u);
}

TEST_F(ARamsesRendererWithDisplay, createsNoCommandForWarpingDataUpdateOnDisplayWithoutWarping)
{
    ramses::WarpingMeshData warpingData(0u, NULL, 0u, NULL, NULL);

    EXPECT_NE(ramses::StatusOK, renderer.updateWarpingMeshData(displayId, warpingData));
    checkForRendererCommandCount(0u);
}

TEST_F(ARamsesRendererWithDisplay, createsNoCommandForInvalidWarpingDataUpdateOnWarpingDisplay)
{
    ramses::WarpingMeshData warpingData(0u, NULL, 0u, NULL, NULL);

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
    EXPECT_NE(ramses::InvalidOffscreenBufferId, renderer.createOffscreenBuffer(displayId, 40u, 40u));
    checkForRendererCommandCount(1u);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_CreateOffscreenBuffer);
}

TEST_F(ARamsesRendererWithDisplay, createsCommandForOffscreenBufferDestroy)
{
    const ramses::offscreenBufferId_t bufferId(0u);
    EXPECT_EQ(ramses::StatusOK, renderer.destroyOffscreenBuffer(displayId, bufferId));
    checkForRendererCommandCount(1u);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_DestroyOffscreenBuffer);
}

TEST_F(ARamsesRendererWithDisplay, failsToCreateOffscreenBufferWithUnsupportedResolution)
{
    EXPECT_EQ(ramses::InvalidOffscreenBufferId, renderer.createOffscreenBuffer(displayId, 0u, 1u));
    EXPECT_EQ(ramses::InvalidOffscreenBufferId, renderer.createOffscreenBuffer(displayId, 1u, 0u));
    EXPECT_EQ(ramses::InvalidOffscreenBufferId, renderer.createOffscreenBuffer(displayId, 0u, 0u));
    EXPECT_EQ(ramses::InvalidOffscreenBufferId, renderer.createOffscreenBuffer(displayId, 5000u, 1u));
    EXPECT_EQ(ramses::InvalidOffscreenBufferId, renderer.createOffscreenBuffer(displayId, 1u, 5000u));
    EXPECT_EQ(ramses::InvalidOffscreenBufferId, renderer.createOffscreenBuffer(displayId, 5000u, 5000u));
}

TEST_F(ARamsesRendererWithDisplay, createsCommandForAssignSceneToOffscreenBuffer)
{
    const ramses::sceneId_t scene(1u);
    const ramses::offscreenBufferId_t bufferId(0u);
    EXPECT_EQ(ramses::StatusOK, renderer.assignSceneToOffscreenBuffer(scene, bufferId));

    checkForRendererCommandCount(1u);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_AssignSceneToOffscreenBuffer);
}

TEST_F(ARamsesRendererWithDisplay, createsCommandForAssignSceneToFramebuffer)
{
    const ramses::sceneId_t scene(1u);
    EXPECT_EQ(ramses::StatusOK, renderer.assignSceneToFramebuffer(scene));

    checkForRendererCommandCount(1u);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_AssignSceneToFramebuffer);
}

TEST_F(ARamsesRendererWithDisplay, createsCommandForOffscreenBufferLink)
{
    const ramses::offscreenBufferId_t bufferId(0u);
    const ramses::sceneId_t consumerScene(1u);
    const ramses::dataConsumerId_t dataConsumer(2u);

    EXPECT_EQ(ramses::StatusOK, renderer.linkOffscreenBufferToSceneData(bufferId, consumerScene, dataConsumer));
    checkForRendererCommandCount(1u);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_LinkBufferToSceneData);
}

/*
* Read Pixels
*/
TEST_F(ARamsesRendererWithDisplay, createsCommandForReadPixels)
{
    EXPECT_EQ(ramses::StatusOK, renderer.readPixels(displayId, 0u, 0u, 40u, 40u));
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
TEST_F(ARamsesRenderer, doesNotHaveRaceConditionsOrDeadlocks_UsingDoOneLoop)
{
    callAllApiFunctionsAndDoOneLoop(renderer);

    CallAllApiFunctionsRunnableUsingDoOneLoop runnable(renderer);
    ramses_internal::PlatformThread thread("RendThrdSanitiz");
    thread.start(runnable);
    thread.join();
}

TEST_F(ARamsesRenderer, doesNotHaveRaceConditionsOrDeadlocks_UsingDoOneLoop_confidenceTest)
{
    const ramses_internal::UInt32 callCount = 5;
    CallAllApiFunctionsRunnableUsingDoOneLoop runnable(renderer, callCount);

    runThreadSanitizerConfidenceTest(runnable);
}

TEST_F(ARamsesRenderer, doesNotHaveRaceConditionsOrDeadlocks_UsingRendererThread)
{
    callAllApiFunctionsInRendererThread(renderer);

    CallAllApiFunctionsRunnableUsingRendererThread runnable(renderer);
    ramses_internal::PlatformThread thread("RendThrdSanitiz");
    thread.start(runnable);

    thread.join();
}

TEST_F(ARamsesRenderer, doesNotHaveRaceConditionsOrDeadlocks_UsingRendererThread_confidenceTest)
{
    const ramses_internal::UInt32 callCount = 5;
    CallAllApiFunctionsRunnableUsingRendererThread runnable(renderer, callCount);

    runThreadSanitizerConfidenceTest(runnable);
}

TEST_F(ARamsesRenderer, canRunRendererInItsOwnThread)
{
    class SceneStateEventHandler : public ramses::RendererEventHandlerEmpty
    {
    public:
        SceneStateEventHandler(ramses::RamsesRenderer& renderer)
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
    ASSERT_FALSE(renderer.impl.getRenderer().getRenderer().hasDisplayController(ramses_internal::DisplayHandle(displayId)));
    EXPECT_EQ(ramses::StatusOK, renderer.startThread());
    SceneStateEventHandler eventHandler(renderer);
    eventHandler.waitForDisplayCreationEvent();

    EXPECT_TRUE(renderer.impl.getRenderer().getRenderer().hasDisplayController(ramses_internal::DisplayHandle(displayId)));
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
    ramses::RamsesRenderer renderer(framework, ramses::RendererConfig());
    {
        ramses_internal::PlatformGuard g(expectLock);
        EXPECT_CALL(notificationMock, safe_notifyThread(ramses::ERamsesThreadIdentifier_Renderer)).Times(AtLeast(1)).WillRepeatedly(ReleaseSyncCall(&syncWaiter));
    }
    EXPECT_EQ(ramses::StatusOK, renderer.startThread());

    const ramses_internal::EStatus status = syncWaiter.wait(60000);
    EXPECT_EQ(ramses_internal::EStatus_RAMSES_OK, status);

    {
        ramses_internal::PlatformGuard g(expectLock);
        EXPECT_CALL(notificationMock, safe_unregisterThread(ramses::ERamsesThreadIdentifier_Workers)).Times(1);
        EXPECT_CALL(notificationMock, safe_unregisterThread(ramses::ERamsesThreadIdentifier_Renderer)).Times(1);
    }
    EXPECT_EQ(ramses::StatusOK, renderer.stopThread());
}

TEST_F(ARamsesRendererWithDisplay, createsCommandForSettingFrameTimerLimits)
{
    EXPECT_EQ(ramses::StatusOK, renderer.setFrameTimerLimits(10001u, 10000u, 10000u, 10000u));

    checkForRendererCommandCount(1u);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_SetFrameTimerLimits);
}

TEST_F(ARamsesRendererWithDisplay, createsCommandForSettingSkippingUnmodifiedBuffersFeature)
{
    EXPECT_EQ(ramses::StatusOK, renderer.setSkippingOfUnmodifiedBuffers(true));

    checkForRendererCommandCount(1u);
    checkForRendererCommand(0u, ramses_internal::ERendererCommand_SetSkippingOfUnmodifiedBuffers);
}
