//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "gmock/gmock.h"
#include "ramses/renderer/RamsesRenderer.h"
#include "ramses/renderer/DisplayConfig.h"
#include "ramses/renderer/IRendererEventHandler.h"
#include "ramses/framework/RamsesFrameworkConfig.h"
#include "ramses/framework/RamsesFramework.h"
#include "ramses/framework/RamsesFrameworkTypes.h"
#include "ramses/client/RamsesClient.h"
#include "ramses/client/Scene.h"

#include "internal/RendererLib/RendererCommands.h"
#include "impl/RamsesRendererImpl.h"
#include "internal/PlatformAbstraction/PlatformThread.h"
#include "internal/PlatformAbstraction/PlatformEvent.h"
#include "RendererCommandVisitorMock.h"
#include "PlatformFactoryMock.h"
#include "internal/SceneGraph/SceneAPI/RenderState.h"
#include "ramses/renderer/IRendererSceneControlEventHandler.h"


namespace ramses::internal
{
    using namespace testing;

    static ramses::RamsesRenderer* CreateRenderer(ramses::RamsesFramework& fw, const ramses::RendererConfig& config)
    {
        auto* createdRenderer = fw.createRenderer(config);
        createdRenderer->impl().getDisplayDispatcher().injectPlatformFactory(std::make_unique<ramses::internal::PlatformFactoryNiceMock>());
        return createdRenderer;
    }

    class SafeThreadWatchdogNotificationMock : public ramses::IThreadWatchdogNotification
    {
    public:
        explicit SafeThreadWatchdogNotificationMock(ramses::internal::PlatformLock& lock)
            : m_lock(lock)
        {
        }

        MOCK_METHOD(void, safe_notifyThread, (ramses::ERamsesThreadIdentifier));
        MOCK_METHOD(void, safe_registerThread, (ramses::ERamsesThreadIdentifier));
        MOCK_METHOD(void, safe_unregisterThread, (ramses::ERamsesThreadIdentifier));

    private:
        void notifyThread(ramses::ERamsesThreadIdentifier threadID) override
        {
            ramses::internal::PlatformGuard g(m_lock);
            safe_notifyThread(threadID);
        }

        void registerThread(ramses::ERamsesThreadIdentifier threadID) override
        {
            ramses::internal::PlatformGuard g(m_lock);
            safe_registerThread(threadID);
        }

        void unregisterThread(ramses::ERamsesThreadIdentifier threadID) override
        {
            ramses::internal::PlatformGuard g(m_lock);
            safe_unregisterThread(threadID);
        }

        ramses::internal::PlatformLock& m_lock;
    };


    class ARamsesRenderer : public ::testing::Test
    {
    protected:
        explicit ARamsesRenderer(const ramses::RendererConfig& rendererConfig = ramses::RendererConfig())
            : framework(ramses::RamsesFrameworkConfig{ramses::EFeatureLevel_Latest})
            , renderer(*CreateRenderer(framework, rendererConfig))
            , commandBuffer(renderer.impl().getPendingCommands())
        {
        }

        ramses::displayId_t addDisplay()
        {
            //Create a display
            ramses::DisplayConfig displayConfig;
            return renderer.createDisplay(displayConfig);
        }

    protected:
        ramses::RamsesFramework framework;
        ramses::RamsesRenderer& renderer;
        const ramses::internal::RendererCommands& commandBuffer;
        StrictMock<ramses::internal::RendererCommandVisitorMock> cmdVisitor;
    };

    class ARamsesRendererWithDisplay : public ARamsesRenderer
    {
        void SetUp() override
        {
            displayId = addDisplay();
            EXPECT_NE(ramses::displayId_t::Invalid(), displayId);
            renderer.flush();
        }

        void TearDown() override
        {
            renderer.destroyDisplay(displayId);
        }

    protected:
        ramses::displayId_t displayId;
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

    TEST_F(ARamsesRenderer, canOnlyGetOneSceneControlAPI)
    {
        const auto api = renderer.getSceneControlAPI();
        EXPECT_TRUE(api != nullptr);
        EXPECT_EQ(api, renderer.getSceneControlAPI());
        EXPECT_EQ(api, renderer.getSceneControlAPI());
    }

    /*
    * Display
    */
    TEST_F(ARamsesRenderer, createsACommandForDisplayCreation)
    {
        const ramses::displayId_t displayId = addDisplay();
        EXPECT_NE(ramses::displayId_t::Invalid(), displayId);

        EXPECT_CALL(cmdVisitor, createDisplayContext(_, ramses::internal::DisplayHandle{ displayId.getValue() }, _));
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
        EXPECT_TRUE(renderer.destroyDisplay(displayId));
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
        EXPECT_TRUE(renderer.logRendererInfo());
        EXPECT_CALL(cmdVisitor, logInfo(_, _, _));
        cmdVisitor.visit(commandBuffer);
    }

    TEST_F(ARamsesRenderer, canCreateDisplayWithEmbeddedCompositingSetOnDisplayConfig)
    {
        ramses::DisplayConfig displayConfig;
        displayConfig.setWaylandEmbeddedCompositingSocketName("ec-socket");
        const ramses::displayId_t displayId = renderer.createDisplay(displayConfig);
        EXPECT_NE(ramses::displayId_t::Invalid(), displayId);

        EXPECT_CALL(cmdVisitor, createDisplayContext(_, ramses::internal::DisplayHandle{ displayId.getValue() }, _));
        cmdVisitor.visit(commandBuffer);
    }

    TEST_F(ARamsesRenderer, displayIsCreatedWithDefaultFramerate)
    {
        ramses::RamsesFrameworkConfig frameworkConfig{ramses::EFeatureLevel_Latest};
        ramses::RamsesFramework ramsesFramework{frameworkConfig};
        ramses::RamsesRenderer* ramsesRenderer = CreateRenderer(ramsesFramework, {});

        ramsesRenderer->startThread();
        const ramses::displayId_t displayId = ramsesRenderer->createDisplay({});
        ramsesRenderer->flush();
        EXPECT_NEAR(60.f, ramsesRenderer->getFramerateLimit(displayId), 0.01f);
        ramsesRenderer->stopThread();
    }

    TEST_F(ARamsesRenderer, canSetAndGetFPSLimitOfNonExistingDisplay)
    {
        ramses::RamsesFrameworkConfig frameworkConfig{ramses::EFeatureLevel_Latest};
        ramses::RamsesFramework ramsesFramework{frameworkConfig};
        ramses::RamsesRenderer* ramsesRenderer = CreateRenderer(ramsesFramework, {});

        constexpr ramses::displayId_t futureDisplay{ 22u };
        EXPECT_NEAR(60.f, ramsesRenderer->getFramerateLimit(futureDisplay), 0.01f);

        EXPECT_TRUE(ramsesRenderer->setFramerateLimit(futureDisplay, 10.f));
        EXPECT_NEAR(10.f, ramsesRenderer->getFramerateLimit(futureDisplay), 0.01f);
    }

    TEST_F(ARamsesRenderer, failsToSetNegativeOrZeroFPSLimit)
    {
        ramses::RamsesFrameworkConfig frameworkConfig{ramses::EFeatureLevel_Latest};
        ramses::RamsesFramework ramsesFramework{frameworkConfig};
        ramses::RamsesRenderer* ramsesRenderer = CreateRenderer(ramsesFramework, {});

        constexpr ramses::displayId_t futureDisplay{ 22u };
        EXPECT_FALSE(ramsesRenderer->setFramerateLimit(futureDisplay, 0.f));
        EXPECT_FALSE(ramsesRenderer->setFramerateLimit(futureDisplay, -10.f));
    }

    TEST_F(ARamsesRenderer, failsToSetFPSLimitInNonThreadedMode)
    {
        ramses::RamsesFrameworkConfig frameworkConfig{ramses::EFeatureLevel_Latest};
        ramses::RamsesFramework ramsesFramework{frameworkConfig};
        ramses::RamsesRenderer* ramsesRenderer = CreateRenderer(ramsesFramework, {});
        ramsesRenderer->doOneLoop();

        constexpr ramses::displayId_t futureDisplay{ 22u };
        EXPECT_FALSE(ramsesRenderer->setFramerateLimit(futureDisplay, 10.f));
    }

    /*
    * Offscreen buffers
    */
    TEST_F(ARamsesRendererWithDisplay, createsCommandForOffscreenBufferCreate_WithDepthStencilBufferAsDefault)
    {
        EXPECT_NE(ramses::displayBufferId_t::Invalid(), renderer.createOffscreenBuffer(displayId, 40u, 40u, 4u));
        EXPECT_CALL(cmdVisitor, handleBufferCreateRequest(_, ramses::internal::DisplayHandle{ displayId.getValue() }, 40u, 40u, 4u, false, ramses::EDepthBufferType::DepthStencil));
        cmdVisitor.visit(commandBuffer);
    }

    TEST_F(ARamsesRendererWithDisplay, createsCommandForOffscreenBufferCreate_WithoutDepthStencilBuffer)
    {
        EXPECT_NE(ramses::displayBufferId_t::Invalid(), renderer.createOffscreenBuffer(displayId, 40u, 40u, 4u, ramses::EDepthBufferType::None));
        EXPECT_CALL(cmdVisitor, handleBufferCreateRequest(_, ramses::internal::DisplayHandle{ displayId.getValue() }, 40u, 40u, 4u, false, ramses::EDepthBufferType::None));
        cmdVisitor.visit(commandBuffer);
    }

    TEST_F(ARamsesRendererWithDisplay, createsCommandForOffscreenBufferCreate_WithDepthBuffer)
    {
        EXPECT_NE(ramses::displayBufferId_t::Invalid(), renderer.createOffscreenBuffer(displayId, 40u, 40u, 4u, ramses::EDepthBufferType::Depth));
        EXPECT_CALL(cmdVisitor, handleBufferCreateRequest(_, ramses::internal::DisplayHandle{ displayId.getValue() }, 40u, 40u, 4u, false, ramses::EDepthBufferType::Depth));
        cmdVisitor.visit(commandBuffer);
    }

    TEST_F(ARamsesRendererWithDisplay, createsCommandForOffscreenBufferCreate_WithDepthStencilBuffer)
    {
        EXPECT_NE(ramses::displayBufferId_t::Invalid(), renderer.createOffscreenBuffer(displayId, 40u, 40u, 4u, ramses::EDepthBufferType::DepthStencil));
        EXPECT_CALL(cmdVisitor, handleBufferCreateRequest(_, ramses::internal::DisplayHandle{ displayId.getValue() }, 40u, 40u, 4u, false, ramses::EDepthBufferType::DepthStencil));
        cmdVisitor.visit(commandBuffer);
    }

    TEST_F(ARamsesRendererWithDisplay, createsCommandForDmaOffscreenBufferCreate)
    {
        constexpr uint32_t fourccFormat = 777u;
        constexpr uint32_t bufferUsageFlags = 888u;
        constexpr uint64_t modifier = 999u;
        EXPECT_NE(ramses::displayBufferId_t::Invalid(), renderer.createDmaOffscreenBuffer(displayId, 40u, 40u, fourccFormat, bufferUsageFlags, modifier));
        EXPECT_CALL(cmdVisitor, handleDmaBufferCreateRequest(_, ramses::internal::DisplayHandle{ displayId.getValue() }, 40u, 40u, ramses::internal::DmaBufferFourccFormat(fourccFormat), ramses::internal::DmaBufferUsageFlags(bufferUsageFlags), ramses::internal::DmaBufferModifiers(modifier)));
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
        ramses::internal::RendererEvent event;
        event.eventType = ramses::internal::ERendererEventType::OffscreenBufferCreated;
        event.displayHandle = ramses::internal::DisplayHandle{displayId.getValue()};
        event.offscreenBuffer = ramses::internal::OffscreenBufferHandle{ 10u };
        event.dmaBufferFD = 20;
        event.dmaBufferStride = 30u;
        renderer.impl().getDisplayDispatcher().injectRendererEvent(std::move(event));

        ramses::RendererEventHandlerEmpty dummyHandler;
        renderer.dispatchEvents(dummyHandler);
        int resultFD = -1;
        uint32_t resultStride = 0u;
        EXPECT_TRUE(renderer.getDmaOffscreenBufferFDAndStride(displayId, ramses::displayBufferId_t{ 10u }, resultFD, resultStride));
        EXPECT_EQ(20, resultFD);
        EXPECT_EQ(30u, resultStride);
    }

    TEST_F(ARamsesRendererWithDisplay, reportsErrorIfGetingFDAndStrideForUnknownDmaOffscreenBuffer)
    {
        int resultFD = -1;
        uint32_t resultStride = 0u;
        EXPECT_FALSE(renderer.getDmaOffscreenBufferFDAndStride(displayId, ramses::displayBufferId_t{ 10u }, resultFD, resultStride));
    }

    TEST_F(ARamsesRendererWithDisplay, createsCommandForOffscreenBufferDestroy)
    {
        const ramses::displayBufferId_t bufferId(0u);
        EXPECT_TRUE(renderer.destroyOffscreenBuffer(displayId, bufferId));
        EXPECT_CALL(cmdVisitor, handleBufferDestroyRequest(ramses::internal::OffscreenBufferHandle{ bufferId.getValue() }, ramses::internal::DisplayHandle{ displayId.getValue() }));
        cmdVisitor.visit(commandBuffer);
    }

    TEST_F(ARamsesRendererWithDisplay, failsToCreateOffscreenBufferWithUnsupportedResolution)
    {
        EXPECT_EQ(ramses::displayBufferId_t::Invalid(), renderer.createOffscreenBuffer(displayId, 0u, 1u, 4u));
        EXPECT_EQ(ramses::displayBufferId_t::Invalid(), renderer.createOffscreenBuffer(displayId, 1u, 0u, 4u));
        EXPECT_EQ(ramses::displayBufferId_t::Invalid(), renderer.createOffscreenBuffer(displayId, 0u, 0u, 4u));
    }

    TEST_F(ARamsesRendererWithDisplay, failsToCreateDmaOffscreenBufferWithUnsupportedResolution)
    {
        constexpr uint32_t fourccFormat = 777u;
        constexpr uint32_t bufferUsageFlags = 888u;
        constexpr uint64_t modifier = 999u;

        EXPECT_EQ(ramses::displayBufferId_t::Invalid(), renderer.createDmaOffscreenBuffer(displayId, 0u, 1u, fourccFormat, bufferUsageFlags, modifier));
        EXPECT_EQ(ramses::displayBufferId_t::Invalid(), renderer.createDmaOffscreenBuffer(displayId, 1u, 0u, fourccFormat, bufferUsageFlags, modifier));
        EXPECT_EQ(ramses::displayBufferId_t::Invalid(), renderer.createDmaOffscreenBuffer(displayId, 0u, 0u, fourccFormat, bufferUsageFlags, modifier));
    }

    /*
    * Stream buffers
    */
    TEST_F(ARamsesRendererWithDisplay, createsCommandForStreamBufferCreate)
    {
        constexpr ramses::waylandIviSurfaceId_t source{ 123u };
        const auto streamBuffer = renderer.createStreamBuffer(displayId, source);
        EXPECT_CALL(cmdVisitor, handleBufferCreateRequest(ramses::internal::StreamBufferHandle{ streamBuffer.getValue() }, ramses::internal::DisplayHandle{ displayId.getValue() }, ramses::internal::WaylandIviSurfaceId{ source.getValue() }));
        cmdVisitor.visit(commandBuffer);
    }

    TEST_F(ARamsesRendererWithDisplay, createsCommandForStreamBufferDestroy)
    {
        constexpr ramses::streamBufferId_t streamBuffer{ 123u };
        EXPECT_TRUE(renderer.destroyStreamBuffer(displayId, streamBuffer));
        EXPECT_CALL(cmdVisitor, handleBufferDestroyRequest(ramses::internal::StreamBufferHandle{ streamBuffer.getValue() }, ramses::internal::DisplayHandle{ displayId.getValue() }));
        cmdVisitor.visit(commandBuffer);
    }

    /*
    * External buffers
    */
    TEST_F(ARamsesRendererWithDisplay, createsCommandForExternalBufferCreate)
    {
        const ramses::externalBufferId_t externalBuffer = renderer.createExternalBuffer(displayId);
        EXPECT_NE(ramses::externalBufferId_t::Invalid(), externalBuffer);
        EXPECT_CALL(cmdVisitor, handleExternalBufferCreateRequest(ramses::internal::ExternalBufferHandle{ externalBuffer.getValue() }, ramses::internal::DisplayHandle{ displayId.getValue() }));
        cmdVisitor.visit(commandBuffer);
    }

    TEST_F(ARamsesRendererWithDisplay, failsToCreateExternalBufferIfRendererIsRunningInOwnThread)
    {
        renderer.startThread();

        EXPECT_FALSE(renderer.createExternalBuffer(displayId).isValid());
        EXPECT_CALL(cmdVisitor, handleExternalBufferCreateRequest(_, _)).Times(0);
        cmdVisitor.visit(commandBuffer);
    }

    TEST_F(ARamsesRendererWithDisplay, createsCommandForExternalBufferDestroy)
    {
        ramses::externalBufferId_t externalBuffer{ 123u };
        EXPECT_TRUE(renderer.destroyExternalBuffer(displayId, externalBuffer));
        EXPECT_CALL(cmdVisitor, handleExternalBufferDestroyRequest(ramses::internal::ExternalBufferHandle{ externalBuffer.getValue() }, ramses::internal::DisplayHandle{ displayId.getValue() }));
        cmdVisitor.visit(commandBuffer);
    }

    /*
    * Read Pixels
    */
    TEST_F(ARamsesRendererWithDisplay, createsCommandForReadPixels)
    {
        ramses::displayBufferId_t bufferId{ 123u };
        EXPECT_TRUE(renderer.readPixels(displayId, bufferId, 1u, 2u, 3u, 4u));
        EXPECT_CALL(cmdVisitor, handleReadPixels(ramses::internal::DisplayHandle{ displayId.getValue() }, ramses::internal::OffscreenBufferHandle{ bufferId.getValue() }, 1u, 2u, 3u, 4u, false, std::string_view{}));
        cmdVisitor.visit(commandBuffer);
    }

    TEST_F(ARamsesRendererWithDisplay, reportsErrorIfReadingPixelsForUnknownDisplay)
    {
        EXPECT_FALSE(renderer.readPixels(ramses::displayId_t{ 999u }, {}, 1u, 2u, 3u, 4u));
    }

    TEST_F(ARamsesRendererWithDisplay, createsNoCommandForEmptyReadPixels)
    {
        EXPECT_FALSE(renderer.readPixels(displayId, {}, 0u, 0u, 10u, 0u));
        EXPECT_FALSE(renderer.readPixels(displayId, {}, 0u, 0u, 0u, 10u));
        cmdVisitor.visit(commandBuffer);
    }

    /*
    * SystemCompositorControl
    */
    TEST_F(ARamsesRenderer, createsNoCommandForSystemCompositorControllerIfNotEnabledFromConfig)
    {
        EXPECT_FALSE(renderer.setSurfaceVisibility(0, true));
        EXPECT_FALSE(renderer.setSurfaceOpacity(0, 0.2f));
        EXPECT_FALSE(renderer.setSurfaceRectangle(0, 1, 2, 3, 4));
        EXPECT_FALSE(renderer.setLayerVisibility(17,true));
        EXPECT_FALSE(renderer.takeSystemCompositorScreenshot("unused_name", -1));
        cmdVisitor.visit(commandBuffer);
    }

    TEST_F(ARamsesRendererWithSystemCompositorController, createsCommandForSystemCompositorControllerSetSurfaceVisibility)
    {
        EXPECT_TRUE(renderer.setSurfaceVisibility(1, true));
        EXPECT_CALL(cmdVisitor, systemCompositorSetIviSurfaceVisibility(ramses::internal::WaylandIviSurfaceId{ 1u }, true));
        cmdVisitor.visit(commandBuffer);
    }

    TEST_F(ARamsesRendererWithSystemCompositorController, createsCommandForSystemCompositorControllerSetSurfaceOpacity)
    {
        EXPECT_TRUE(renderer.setSurfaceOpacity(1, 0.2f));
        EXPECT_CALL(cmdVisitor, systemCompositorSetIviSurfaceOpacity(ramses::internal::WaylandIviSurfaceId{ 1u }, 0.2f));
        cmdVisitor.visit(commandBuffer);
    }

    TEST_F(ARamsesRendererWithSystemCompositorController, createsCommandForSystemCompositorControllerSetSurfaceRectangle)
    {
        EXPECT_TRUE(renderer.setSurfaceRectangle(1, 2, 3, 4, 5));
        EXPECT_CALL(cmdVisitor, systemCompositorSetIviSurfaceDestRectangle(ramses::internal::WaylandIviSurfaceId{ 1u }, 2, 3, 4, 5));
        cmdVisitor.visit(commandBuffer);
    }

    TEST_F(ARamsesRendererWithSystemCompositorController, createsCommandForSystemCompositorControllerAddSurfaceToLayer)
    {
        EXPECT_TRUE(renderer.impl().systemCompositorAddIviSurfaceToIviLayer(1, 2));
        EXPECT_CALL(cmdVisitor, systemCompositorAddIviSurfaceToIviLayer(ramses::internal::WaylandIviSurfaceId{ 1u }, ramses::internal::WaylandIviLayerId{ 2u }));
        cmdVisitor.visit(commandBuffer);
    }

    TEST_F(ARamsesRendererWithSystemCompositorController, createsCommandForSystemCompositorControllerSetLayerVisibility)
    {
        EXPECT_TRUE(renderer.setLayerVisibility(17, true));
        EXPECT_CALL(cmdVisitor, systemCompositorSetIviLayerVisibility(ramses::internal::WaylandIviLayerId{ 17u }, true));
        cmdVisitor.visit(commandBuffer);
    }

    TEST_F(ARamsesRendererWithSystemCompositorController, createsCommandForSystemCompositorControllerTakeScreenshot)
    {
        EXPECT_TRUE(renderer.takeSystemCompositorScreenshot("name", -1));
        EXPECT_CALL(cmdVisitor, systemCompositorScreenshot(std::string_view{"name"}, -1));
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

        void displayCreated(ramses::displayId_t /*displayId*/, ramses::ERendererEventResult result) override
        {
            m_displayCreated = (result == ramses::ERendererEventResult::Ok);
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
        EXPECT_TRUE(renderer.startThread());
        SceneStateEventHandler eventHandler(renderer);
        eventHandler.waitForDisplayCreationEvent();
        EXPECT_TRUE(renderer.stopThread());
    }

    TEST_F(ARamsesRenderer, canRunRendererInItsOwnThreadAndCallAPIMethods)
    {
        const ramses::displayId_t displayId = addDisplay();
        renderer.flush();
        EXPECT_TRUE(renderer.startThread());
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
        EXPECT_TRUE(renderer.stopThread());
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
        EXPECT_TRUE(renderer.flush());
        EXPECT_TRUE(renderer.startThread());
        EXPECT_TRUE(renderer.stopThread());
    }

    TEST(ARamsesRendererWithSeparateRendererThread, canNotifyPerWatchdog)
    {
        ramses::internal::PlatformLock expectLock;
        SafeThreadWatchdogNotificationMock notificationMock(expectLock);

        ramses::RamsesFrameworkConfig frameworkConfig{ramses::EFeatureLevel_Latest};
        EXPECT_TRUE(frameworkConfig.setWatchdogNotificationInterval(ramses::ERamsesThreadIdentifier::Renderer, 500));
        EXPECT_TRUE(frameworkConfig.setWatchdogNotificationCallBack(&notificationMock));
        {
            ramses::internal::PlatformGuard g(expectLock);
            EXPECT_CALL(notificationMock, safe_registerThread(ramses::ERamsesThreadIdentifier::Workers)).Times(1);
            EXPECT_CALL(notificationMock, safe_notifyThread(ramses::ERamsesThreadIdentifier::Workers)).Times(AtLeast(1));
        }
        ramses::RamsesFramework framework(frameworkConfig);

        {
            ramses::internal::PlatformGuard g(expectLock);
            EXPECT_CALL(notificationMock, safe_registerThread(ramses::ERamsesThreadIdentifier::Renderer)).Times(1);
        }

        // syncWaiter must outlive renderer because renderer calls syncWaiter
        // via mock until its dtor has run (only then thread gets really stopped!)
        ramses::internal::PlatformEvent syncWaiter;
        ramses::RamsesRenderer& renderer(*framework.createRenderer(ramses::RendererConfig()));
        {
            ramses::internal::PlatformGuard g(expectLock);
            EXPECT_CALL(notificationMock, safe_notifyThread(ramses::ERamsesThreadIdentifier::Renderer))
                .Times(AtLeast(1))
                .WillRepeatedly(InvokeWithoutArgs([&]() { syncWaiter.signal(); }));
        }
        EXPECT_TRUE(renderer.startThread());

        EXPECT_TRUE(syncWaiter.wait(60000));

        {
            ramses::internal::PlatformGuard g(expectLock);
            EXPECT_CALL(notificationMock, safe_unregisterThread(ramses::ERamsesThreadIdentifier::Workers)).Times(1);
            EXPECT_CALL(notificationMock, safe_unregisterThread(ramses::ERamsesThreadIdentifier::Renderer)).Times(1);
        }
        EXPECT_TRUE(renderer.stopThread());
        framework.destroyRenderer(renderer);
    }

    class RenderThreadLoopTimingsNotification final : public ramses::RendererEventHandlerEmpty
    {
    public:
        void renderThreadLoopTimings(ramses::displayId_t displayId, std::chrono::microseconds /*maximumLoopTime*/, std::chrono::microseconds /*averageLooptime*/) override
        {
            m_timeReports[displayId]++;
        }

        bool displaysReported(std::initializer_list<ramses::displayId_t> displays, size_t minCount = 2u)
        {
            return m_timeReports.size() == displays.size()
                && std::all_of(displays.begin(), displays.end(), [&](const auto d) { return m_timeReports[d] >= minCount; });
        }

    private:
        std::unordered_map<ramses::displayId_t, size_t> m_timeReports;
    };

    TEST(ARamsesRendererNonThreaded, reportsFrameTimings)
    {
        ramses::RamsesFrameworkConfig frameworkConfig{ramses::EFeatureLevel_Latest};
        ramses::RamsesFramework framework{frameworkConfig};
        ramses::RendererConfig rConfig;
        rConfig.setRenderThreadLoopTimingReportingPeriod(std::chrono::milliseconds{ 50 });
        ramses::RamsesRenderer& renderer(*CreateRenderer(framework, rConfig));

        const auto display1 = renderer.createDisplay({});
        const auto display2 = renderer.createDisplay({});
        renderer.flush();

        // wait for either 60 seconds or until event was received
        RenderThreadLoopTimingsNotification eventHandler;
        const auto startTS = std::chrono::steady_clock::now();
        while (!eventHandler.displaysReported({ display1, display2 })
            && std::chrono::steady_clock::now() - startTS < std::chrono::minutes{ 1 })
        {
            renderer.doOneLoop();
            renderer.dispatchEvents(eventHandler);
        }
        EXPECT_TRUE(eventHandler.displaysReported({ display1, display2 }));

        framework.destroyRenderer(renderer);
    }

    TEST(ARamsesRendererWithSeparateRendererThread, reportsFrameTimings)
    {
        ramses::RamsesFrameworkConfig frameworkConfig{ramses::EFeatureLevel_Latest};
        ramses::RamsesFramework framework{frameworkConfig};
        ramses::RendererConfig rConfig;
        rConfig.setRenderThreadLoopTimingReportingPeriod(std::chrono::milliseconds{ 50 });
        ramses::RamsesRenderer& renderer(*CreateRenderer(framework, rConfig));

        const auto display1 = renderer.createDisplay({});
        const auto display2 = renderer.createDisplay({});
        renderer.flush();

        EXPECT_TRUE(renderer.startThread());

        // wait for either 60 seconds or until event was received
        RenderThreadLoopTimingsNotification eventHandler;
        const auto startTS = std::chrono::steady_clock::now();
        while (!eventHandler.displaysReported({ display1, display2 })
            && std::chrono::steady_clock::now() - startTS < std::chrono::minutes{ 1 })
        {
            renderer.dispatchEvents(eventHandler);
            std::this_thread::sleep_for(std::chrono::milliseconds{ 5 });
        }
        EXPECT_TRUE(eventHandler.displaysReported({ display1, display2 }));

        EXPECT_TRUE(renderer.stopThread());
        framework.destroyRenderer(renderer);
    }

    TEST(ARamsesRendererWithSeparateRendererThread, willNotReportsFrameTimingsIfDisabled)
    {
        ramses::RamsesFrameworkConfig frameworkConfig{ramses::EFeatureLevel_Latest};
        ramses::RamsesFramework framework{frameworkConfig};
        ramses::RendererConfig rConfig;
        rConfig.setRenderThreadLoopTimingReportingPeriod(std::chrono::milliseconds{ 0 });
        ramses::RamsesRenderer& renderer(*CreateRenderer(framework, rConfig));

        const auto display1 = renderer.createDisplay({});
        renderer.flush();

        EXPECT_TRUE(renderer.startThread());

        RenderThreadLoopTimingsNotification eventHandler;
        const auto startTS = std::chrono::steady_clock::now();
        while (!eventHandler.displaysReported({ display1 }) && std::chrono::steady_clock::now() - startTS < std::chrono::seconds{ 5 })
        {
            renderer.dispatchEvents(eventHandler);
            std::this_thread::sleep_for(std::chrono::milliseconds{ 5 });
        }
        EXPECT_FALSE(eventHandler.displaysReported({ display1 }));

        EXPECT_TRUE(renderer.stopThread());
        framework.destroyRenderer(renderer);
    }

    TEST(ARamsesRendererWithSeparateRendererThread, TSAN_periodicallyDispatchEvents)
    {
        // this test is meant for TSAN
        // - run renderer with display in thread with 30fps
        // - keep dispatching events with higher frequency

        ramses::RamsesFrameworkConfig frameworkConfig{ramses::EFeatureLevel_Latest};
        ramses::RamsesFramework framework{frameworkConfig};
        ramses::RamsesRenderer& renderer(*CreateRenderer(framework, {}));
        ramses::RamsesClient& client(*framework.createClient({}));
        framework.connect();

        auto scene = client.createScene(ramses::sceneId_t{ 321u });
        scene->createNode();
        scene->flush();
        scene->publish(ramses::EScenePublicationMode::LocalOnly);

        renderer.setFrameTimerLimits(std::numeric_limits<uint64_t>::max(), std::numeric_limits<uint64_t>::max(), std::numeric_limits<uint64_t>::max());
        renderer.setLoopMode(ramses::ELoopMode::UpdateOnly);
        const auto displayId = renderer.createDisplay({});
        renderer.setFramerateLimit(displayId, 30.f);
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
        EXPECT_TRUE(renderer.setFrameTimerLimits(10001u, 10002u, 10003u));
        EXPECT_CALL(cmdVisitor, setLimitsFrameBudgets(10001u, 10002u, 10003u));
        cmdVisitor.visit(commandBuffer);
    }

    TEST_F(ARamsesRendererWithDisplay, createsCommandForSettingSkippingUnmodifiedBuffersFeature)
    {
        EXPECT_TRUE(renderer.setSkippingOfUnmodifiedBuffers(true));
        EXPECT_CALL(cmdVisitor, setSkippingOfUnmodifiedBuffers(true));
        cmdVisitor.visit(commandBuffer);
    }

    TEST_F(ARamsesRendererWithDisplay, createsCommandForSettingClearFlags_FB)
    {
        EXPECT_TRUE(renderer.setDisplayBufferClearFlags(displayId, renderer.getDisplayFramebuffer(displayId), ramses::EClearFlag::Color));
        EXPECT_CALL(cmdVisitor, handleSetClearFlags(ramses::internal::DisplayHandle{ displayId.getValue() }, ramses::internal::OffscreenBufferHandle{}, ramses::ClearFlags(ramses::EClearFlag::Color)));
        cmdVisitor.visit(commandBuffer);
    }

    TEST_F(ARamsesRendererWithDisplay, createsCommandForSettingClearFlags_FBImplicitlyUsingInvalidDisplayBuffer)
    {
        EXPECT_TRUE(renderer.setDisplayBufferClearFlags(displayId, ramses::displayBufferId_t::Invalid(), ramses::EClearFlag::Color));
        EXPECT_CALL(cmdVisitor, handleSetClearFlags(ramses::internal::DisplayHandle{ displayId.getValue() }, ramses::internal::OffscreenBufferHandle{}, ramses::ClearFlags(ramses::EClearFlag::Color)));
        cmdVisitor.visit(commandBuffer);
    }

    TEST_F(ARamsesRendererWithDisplay, createsCommandForSettingClearFlags_OB)
    {
        EXPECT_TRUE(renderer.setDisplayBufferClearFlags(displayId, ramses::displayBufferId_t{ 666u }, ramses::EClearFlag::Color));
        EXPECT_CALL(cmdVisitor, handleSetClearFlags(ramses::internal::DisplayHandle{ displayId.getValue() }, ramses::internal::OffscreenBufferHandle{ 666u }, ramses::ClearFlags(ramses::EClearFlag::Color)));
        cmdVisitor.visit(commandBuffer);
    }

    TEST_F(ARamsesRendererWithDisplay, reportsErrorIfSettingClearFlagsForUnknownDisplay)
    {
        EXPECT_FALSE(renderer.setDisplayBufferClearFlags(ramses::displayId_t{ 999u }, {}, ramses::EClearFlag::Color));
    }

    TEST_F(ARamsesRendererWithDisplay, createsCommandForSettingClearColor_FB)
    {
        EXPECT_TRUE(renderer.setDisplayBufferClearColor(displayId, renderer.getDisplayFramebuffer(displayId), {1, 2, 3, 4}));
        EXPECT_CALL(cmdVisitor, handleSetClearColor(ramses::internal::DisplayHandle{ displayId.getValue() }, ramses::internal::OffscreenBufferHandle{}, glm::vec4{ 1, 2, 3, 4 }));
        cmdVisitor.visit(commandBuffer);
    }

    TEST_F(ARamsesRendererWithDisplay, createsCommandForSettingClearColor_FBImplicitlyUsingInvalidDisplayBuffer)
    {
        EXPECT_TRUE(renderer.setDisplayBufferClearColor(displayId, ramses::displayBufferId_t::Invalid(), {1, 2, 3, 4}));
        EXPECT_CALL(cmdVisitor, handleSetClearColor(ramses::internal::DisplayHandle{ displayId.getValue() }, ramses::internal::OffscreenBufferHandle{}, glm::vec4{ 1, 2, 3, 4 }));
        cmdVisitor.visit(commandBuffer);
    }

    TEST_F(ARamsesRendererWithDisplay, createsCommandForSettingClearColor_OB)
    {
        EXPECT_TRUE(renderer.setDisplayBufferClearColor(displayId, ramses::displayBufferId_t{666u}, {1, 2, 3, 4}));
        EXPECT_CALL(cmdVisitor, handleSetClearColor(ramses::internal::DisplayHandle{ displayId.getValue() }, ramses::internal::OffscreenBufferHandle{ 666u }, glm::vec4{ 1, 2, 3, 4 }));
        cmdVisitor.visit(commandBuffer);
    }

    TEST_F(ARamsesRendererWithDisplay, reportsErrorIfSettingClearColorForUnknownDisplay)
    {
        EXPECT_FALSE(renderer.setDisplayBufferClearColor(ramses::displayId_t{999u}, {}, {1, 2, 3, 4}));
    }

    TEST_F(ARamsesRendererWithDisplay, createsCommandForSettingExternallyOwnedWindowSize)
    {
        EXPECT_TRUE(renderer.setExternallyOwnedWindowSize(displayId, 123u, 456u));
        EXPECT_CALL(cmdVisitor, handleSetExternallyOwnedWindowSize(ramses::internal::DisplayHandle{ displayId.getValue() }, 123u, 456u));
        cmdVisitor.visit(commandBuffer);
    }

    TEST_F(ARamsesRendererWithDisplay, reportsErrorIfSettingExternallyOwnedWindowSizeForUnknownDisplay)
    {
        EXPECT_FALSE(renderer.setExternallyOwnedWindowSize({}, 123u, 456u));
    }

    TEST(ARamsesFrameworkInARendererLib, canCreateARenderer)
    {
        ramses::RamsesFrameworkConfig fwConfig{ramses::EFeatureLevel_Latest};
        ramses::RamsesFramework fw{fwConfig};

        auto renderer = fw.createRenderer(ramses::RendererConfig());
        EXPECT_NE(nullptr, renderer);
    }

    TEST(ARamsesFrameworkInARendererLib, canNotCreateMultipleRenderer)
    {
        ramses::RamsesFrameworkConfig fwConfig{ramses::EFeatureLevel_Latest};
        ramses::RamsesFramework fw{fwConfig};

        auto renderer1 = fw.createRenderer(ramses::RendererConfig());
        auto renderer2 = fw.createRenderer(ramses::RendererConfig());
        EXPECT_NE(nullptr, renderer1);
        EXPECT_EQ(nullptr, renderer2);
    }

    TEST(ARamsesFrameworkInARendererLib, acceptsLocallyCreatedRendererForDestruction)
    {
        ramses::RamsesFrameworkConfig fwConfig{ramses::EFeatureLevel_Latest};
        ramses::RamsesFramework fw{fwConfig};

        auto renderer = fw.createRenderer(ramses::RendererConfig());
        EXPECT_TRUE(fw.destroyRenderer(*renderer));
    }

    TEST(ARamsesFrameworkInARendererLib, doesNotAcceptForeignCreatedRendererForDestruction)
    {
        ramses::RamsesFrameworkConfig fwConfig{ramses::EFeatureLevel_Latest};
        ramses::RamsesFramework fw1{fwConfig};
        ramses::RamsesFramework fw2{fwConfig};

        auto renderer1 = fw1.createRenderer(ramses::RendererConfig());
        auto renderer2 = fw2.createRenderer(ramses::RendererConfig());
        EXPECT_FALSE(fw2.destroyRenderer(*renderer1));
        EXPECT_FALSE(fw1.destroyRenderer(*renderer2));
    }

    TEST(ARamsesFrameworkInARendererLib, doesNotAcceptSameRendererTwiceForDestruction)
    {
        ramses::RamsesFrameworkConfig fwConfig{ramses::EFeatureLevel_Latest};
        ramses::RamsesFramework fw{fwConfig};

        auto renderer = fw.createRenderer(ramses::RendererConfig());
        EXPECT_TRUE(fw.destroyRenderer(*renderer));
        EXPECT_FALSE(fw.destroyRenderer(*renderer));
    }

    TEST(ARamsesFrameworkInARendererLib, canCreateDestroyAndRecreateARenderer)
    {
        ramses::RamsesFrameworkConfig fwConfig{ramses::EFeatureLevel_Latest};
        ramses::RamsesFramework fw{fwConfig};

        auto renderer = fw.createRenderer(ramses::RendererConfig());
        EXPECT_NE(nullptr, renderer);
        EXPECT_TRUE(fw.destroyRenderer(*renderer));
        renderer = fw.createRenderer(ramses::RendererConfig());
        EXPECT_NE(nullptr, renderer);
    }

    TEST(ARamsesFrameworkInARendererLib, createRendererFailsWhenConnected)
    {
        ramses::RamsesFrameworkConfig frameworkConfig{ramses::EFeatureLevel_Latest};
        ramses::RamsesFramework framework{frameworkConfig};
        EXPECT_TRUE(framework.connect());
        EXPECT_EQ(framework.createRenderer(ramses::RendererConfig()), nullptr);
    }

    TEST(ARamsesFrameworkInARendererLib, destroyRendererFailsWhenConnected)
    {
        ramses::RamsesFrameworkConfig frameworkConfig{ramses::EFeatureLevel_Latest};
        ramses::RamsesFramework framework{frameworkConfig};
        auto* renderer = framework.createRenderer(ramses::RendererConfig());
        ASSERT_NE(renderer, nullptr);
        EXPECT_TRUE(framework.connect());
        EXPECT_FALSE(framework.destroyRenderer(*renderer));
        EXPECT_TRUE(framework.disconnect());
        EXPECT_TRUE(framework.destroyRenderer(*renderer));
    }
}
