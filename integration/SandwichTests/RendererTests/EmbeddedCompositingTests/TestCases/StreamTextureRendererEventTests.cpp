//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-renderer-api/IRendererEventHandler.h"
#include "ramses-renderer-api/IRendererSceneControlEventHandler.h"
#include "StreamTextureRendererEventTests.h"
#include "TestScenes/EmbeddedCompositorScene.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    void StreamTextureRendererEventTests::setUpEmbeddedCompositingTestCases(EmbeddedCompositingTestsFramework& testFramework)
    {
        ramses::DisplayConfig displayConfig = RendererTestUtils::CreateTestDisplayConfig(0u, true);
        displayConfig.setWindowRectangle(0u, 0u, IntegrationScene::DefaultViewportWidth, IntegrationScene::DefaultViewportHeight);
        displayConfig.setWaylandEmbeddedCompositingSocketName(EmbeddedCompositingTestsFramework::TestEmbeddedCompositingDisplayName.c_str());

        testFramework.createTestCase(SurfaceAvailableEventGeneratedWhenBufferAttached, *this, "SurfaceAvailableEventGeneratedWhenBufferAttached").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(SurfaceUnavailableEventGeneratedWhenBufferDetached, *this, "SurfaceUnavailableEventGeneratedWhenBufferDetached").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(SurfaceUnavailableEventGeneratedWhenSurfaceDestroyed, *this, "SurfaceUnavailableEventGeneratedWhenSurfaceDestroyed").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(SurfaceUnavailableEventGeneratedWhenIviSurfaceDestroyed, *this, "SurfaceUnavailableEventGeneratedWhenIviSurfaceDestroyed").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(SurfaceUnavailableEventGeneratedWhenClientIsKilled, *this, "SurfaceUnavailableEventGeneratedWhenClientIsKilled").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(NoSurfaceEventGeneratedWhenBufferAttachedAndDetachedInSameLoop, *this, "NoSurfaceEventGeneratedWhenBufferAttachedAndDetachedInSameLoop").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(SurfaceAvailableAndUnavailableEventsGeneratedWhenBufferAttachedAndDetachedInDifferentLoops, *this, "SurfaceAvailableAndUnavailableEventsGeneratedWhenBufferAttachedAndDetachedInDifferentLoops").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(SurfaceAvailableEventsGeneratedTwiceWhenBufferReattached, *this, "SurfaceAvailableEventsGeneratedTwiceWhenBufferReattached").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(SurfaceAvailableEventGeneratedWhenBufferAttached_NoSceneAvailable, *this, "SurfaceAvailableEventGeneratedWhenBufferAttached_NoSceneAvailable").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(SurfaceUnavailableEventGeneratedWhenBufferDetached_NoSceneAvailable, *this, "SurfaceUnavailableEventGeneratedWhenBufferDetached_NoSceneAvailable").m_displayConfigs.push_back(displayConfig);
    }

    bool StreamTextureRendererEventTests::runEmbeddedCompositingTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase)
    {
        const bool testResult = runTestCase(testFramework, testCase);

        //get rid of any renderer events still lingering from running the test case
        renderAndGetSurfaceAvailabilityChangeEvents(testFramework, WaylandIviSurfaceId(0u));

        return testResult;
    }

    bool StreamTextureRendererEventTests::runTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase)
    {
        bool testResultValue = true;
        constexpr WaylandIviSurfaceId waylandSurfaceIviId1{571u};

        switch(testCase.m_id)
        {
        case SurfaceAvailableEventGeneratedWhenBufferAttached:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, IntegrationScene::DefaultViewportWidth, IntegrationScene::DefaultViewportHeight);
            testFramework.startTestApplicationAndWaitUntilConnected();
            testResultValue &= renderAndExpectNoStreamSurfaceAvailabilityChanged(testFramework, waylandSurfaceIviId1);

            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testResultValue &= renderAndExpectNoStreamSurfaceAvailabilityChanged(testFramework, waylandSurfaceIviId1);

            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testResultValue &= renderAndExpectNoStreamSurfaceAvailabilityChanged(testFramework, waylandSurfaceIviId1);

            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);
            testResultValue &= testFramework.waitForStreamSurfaceAvailabilityChange(waylandSurfaceIviId1, true);

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case SurfaceUnavailableEventGeneratedWhenBufferDetached:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, IntegrationScene::DefaultViewportWidth, IntegrationScene::DefaultViewportHeight);
            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);
            testResultValue &= testFramework.waitForStreamSurfaceAvailabilityChange(waylandSurfaceIviId1, true);

            testFramework.sendDetachBufferFromSurfaceToTestApplication(surfaceId);
            testFramework.waitForUnavailablilityOfContentOnStreamTexture(waylandSurfaceIviId1);
            testResultValue &= testFramework.waitForStreamSurfaceAvailabilityChange(waylandSurfaceIviId1, false);

            testFramework.stopTestApplicationAndWaitUntilDisconnected();

            break;
        }

        case SurfaceUnavailableEventGeneratedWhenSurfaceDestroyed:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, IntegrationScene::DefaultViewportWidth, IntegrationScene::DefaultViewportHeight);
            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);
            testResultValue &= testFramework.waitForStreamSurfaceAvailabilityChange(waylandSurfaceIviId1, true);

            testFramework.sendDestroySurfaceToTestApplication(surfaceId);
            testFramework.waitForUnavailablilityOfContentOnStreamTexture(waylandSurfaceIviId1);
            testResultValue &= testFramework.waitForStreamSurfaceAvailabilityChange(waylandSurfaceIviId1, false);

            testFramework.stopTestApplicationAndWaitUntilDisconnected();

            break;
        }

        case SurfaceUnavailableEventGeneratedWhenIviSurfaceDestroyed:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, IntegrationScene::DefaultViewportWidth, IntegrationScene::DefaultViewportHeight);
            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);
            testResultValue &= testFramework.waitForStreamSurfaceAvailabilityChange(waylandSurfaceIviId1, true);

            testFramework.sendDestroyIVISurfaceToTestApplication(surfaceId);
            testFramework.waitForUnavailablilityOfContentOnStreamTexture(waylandSurfaceIviId1);
            testResultValue &= testFramework.waitForStreamSurfaceAvailabilityChange(waylandSurfaceIviId1, false);

            testFramework.sendDestroySurfaceToTestApplication(surfaceId);
            testFramework.stopTestApplicationAndWaitUntilDisconnected();

            break;
        }

        case SurfaceUnavailableEventGeneratedWhenClientIsKilled:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, IntegrationScene::DefaultViewportWidth, IntegrationScene::DefaultViewportHeight);
            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);
            testResultValue &= testFramework.waitForStreamSurfaceAvailabilityChange(waylandSurfaceIviId1, true);

            testFramework.killTestApplication();
            testFramework.waitUntilNumberOfCompositorConnections(0, true);
            testResultValue &= testFramework.waitForStreamSurfaceAvailabilityChange(waylandSurfaceIviId1, false);

            break;
        }

        case NoSurfaceEventGeneratedWhenBufferAttachedAndDetachedInSameLoop:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, IntegrationScene::DefaultViewportWidth, IntegrationScene::DefaultViewportHeight);
            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);

            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);
            testFramework.sendDetachBufferFromSurfaceToTestApplication(surfaceId);
            testFramework.waitForUnavailablilityOfContentOnStreamTexture(waylandSurfaceIviId1);

            testResultValue &= renderAndExpectNoStreamSurfaceAvailabilityChanged(testFramework, waylandSurfaceIviId1);

            testFramework.stopTestApplicationAndWaitUntilDisconnected();

            break;
        }

        case SurfaceAvailableAndUnavailableEventsGeneratedWhenBufferAttachedAndDetachedInDifferentLoops:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, IntegrationScene::DefaultViewportWidth, IntegrationScene::DefaultViewportHeight);
            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);

            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);
            testFramework.renderOneFrame(); //to process the event

            testFramework.sendDetachBufferFromSurfaceToTestApplication(surfaceId);
            testFramework.waitForUnavailablilityOfContentOnStreamTexture(waylandSurfaceIviId1);

            const BoolVector events = renderAndGetSurfaceAvailabilityChangeEvents(testFramework, waylandSurfaceIviId1);

            testResultValue &= (2u == events.size()
                                && true == events[0]
                                && false == events[1]
                                );

            testFramework.stopTestApplicationAndWaitUntilDisconnected();

            break;
        }

        case SurfaceAvailableEventsGeneratedTwiceWhenBufferReattached:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, IntegrationScene::DefaultViewportWidth, IntegrationScene::DefaultViewportHeight);
            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);

            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);
            testFramework.renderOneFrame(); //to process the event

            testFramework.sendDetachBufferFromSurfaceToTestApplication(surfaceId);
            testFramework.waitForUnavailablilityOfContentOnStreamTexture(waylandSurfaceIviId1);
            testFramework.renderOneFrame(); //to process the event

            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);

            const BoolVector events = renderAndGetSurfaceAvailabilityChangeEvents(testFramework, waylandSurfaceIviId1);
            testResultValue &= (3u == events.size()
                                && true == events[0]
                                && false == events[1]
                                && true == events[2]
                                );


            testFramework.stopTestApplicationAndWaitUntilDisconnected();

            break;
        }

        case SurfaceAvailableEventGeneratedWhenBufferAttached_NoSceneAvailable:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            testResultValue &= renderAndExpectNoStreamSurfaceAvailabilityChanged(testFramework, waylandSurfaceIviId1);

            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testResultValue &= renderAndExpectNoStreamSurfaceAvailabilityChanged(testFramework, waylandSurfaceIviId1);

            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testResultValue &= renderAndExpectNoStreamSurfaceAvailabilityChanged(testFramework, waylandSurfaceIviId1);

            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);
            testResultValue &= testFramework.waitForStreamSurfaceAvailabilityChange(waylandSurfaceIviId1, true);

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case SurfaceUnavailableEventGeneratedWhenBufferDetached_NoSceneAvailable:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);
            testResultValue &= testFramework.waitForStreamSurfaceAvailabilityChange(waylandSurfaceIviId1, true);

            testFramework.sendDetachBufferFromSurfaceToTestApplication(surfaceId);
            testFramework.waitForUnavailablilityOfContentOnStreamTexture(waylandSurfaceIviId1);
            testResultValue &= testFramework.waitForStreamSurfaceAvailabilityChange(waylandSurfaceIviId1, false);

            testFramework.stopTestApplicationAndWaitUntilDisconnected();

            break;
        }
        default:
            assert(false);
        }

        return testResultValue;
    }

    bool StreamTextureRendererEventTests::renderAndExpectNoStreamSurfaceAvailabilityChanged(EmbeddedCompositingTestsFramework& testFramework, WaylandIviSurfaceId streamSourceId)
    {
        return 0u == renderAndGetSurfaceAvailabilityChangeEvents(testFramework, streamSourceId).size();
    }

    BoolVector StreamTextureRendererEventTests::renderAndGetSurfaceAvailabilityChangeEvents(EmbeddedCompositingTestsFramework& testFramework, WaylandIviSurfaceId streamSourceId)
    {
        class StreamAvailabilityChangeEventHandler : public ramses::RendererSceneControlEventHandlerEmpty
        {
        public:
            explicit StreamAvailabilityChangeEventHandler(WaylandIviSurfaceId targetStreamSourceId)
                : m_targetStreamSourceId(targetStreamSourceId)
            {
            }

            void streamAvailabilityChanged(ramses::waylandIviSurfaceId_t eventStreamSourceId, bool state) override
            {
                if(eventStreamSourceId.getValue() == m_targetStreamSourceId.getValue())
                {
                    m_streamAvailabilityChangeEvents.push_back(state);
                }
            }

            [[nodiscard]] const BoolVector& getStreamAvailabilityChanged() const
            {
                return m_streamAvailabilityChangeEvents;
            }

        private:
            const WaylandIviSurfaceId m_targetStreamSourceId;
            BoolVector m_streamAvailabilityChangeEvents;
        };

        testFramework.renderOneFrame();

        ramses::RendererEventHandlerEmpty dummy;
        StreamAvailabilityChangeEventHandler handler(streamSourceId);
        testFramework.dispatchRendererEvents(dummy, handler);
        return handler.getStreamAvailabilityChanged();
    }
}
