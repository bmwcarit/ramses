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
    StreamTextureRendererEventTests::StreamTextureRendererEventTests()
    {
    }

    void StreamTextureRendererEventTests::setUpEmbeddedCompositingTestCases(EmbeddedCompositingTestsFramework& testFramework)
    {
        testFramework.createTestCaseWithDefaultDisplay(SurfaceAvailableEventGeneratedWhenBufferAttached, *this, "SurfaceAvailableEventGeneratedWhenBufferAttached");
        testFramework.createTestCaseWithDefaultDisplay(SurfaceUnavailableEventGeneratedWhenBufferDetached, *this, "SurfaceUnavailableEventGeneratedWhenBufferDetached");
        testFramework.createTestCaseWithDefaultDisplay(SurfaceUnavailableEventGeneratedWhenSurfaceDestroyed, *this, "SurfaceUnavailableEventGeneratedWhenSurfaceDestroyed");
        testFramework.createTestCaseWithDefaultDisplay(SurfaceUnavailableEventGeneratedWhenIviSurfaceDestroyed, *this, "SurfaceUnavailableEventGeneratedWhenIviSurfaceDestroyed");
        testFramework.createTestCaseWithDefaultDisplay(SurfaceUnavailableEventGeneratedWhenClientIsKilled, *this, "SurfaceUnavailableEventGeneratedWhenClientIsKilled");
        testFramework.createTestCaseWithDefaultDisplay(NoSurfaceEventGeneratedWhenBufferAttachedAndDetachedInSameLoop, *this, "NoSurfaceEventGeneratedWhenBufferAttachedAndDetachedInSameLoop");
        testFramework.createTestCaseWithDefaultDisplay(SurfaceAvailableAndUnavailableEventsGeneratedWhenBufferAttachedAndDetachedInDifferentLoops, *this, "SurfaceAvailableAndUnavailableEventsGeneratedWhenBufferAttachedAndDetachedInDifferentLoops");
        testFramework.createTestCaseWithDefaultDisplay(SurfaceAvailableEventsGeneratedTwiceWhenBufferReattached, *this, "SurfaceAvailableEventsGeneratedTwiceWhenBufferReattached");
        testFramework.createTestCaseWithDefaultDisplay(SurfaceAvailableEventGeneratedWhenBufferAttached_NoSceneAvailable, *this, "SurfaceAvailableEventGeneratedWhenBufferAttached_NoSceneAvailable");
        testFramework.createTestCaseWithDefaultDisplay(SurfaceUnavailableEventGeneratedWhenBufferDetached_NoSceneAvailable, *this, "SurfaceUnavailableEventGeneratedWhenBufferDetached_NoSceneAvailable");
    }

    bool StreamTextureRendererEventTests::runEmbeddedCompositingTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase)
    {
        const bool testResult = runTestCase(testFramework, testCase);

        //get rid of any renderer events still lingering from running the test case
        renderAndGetSurfaceAvailabilityChangeEvents(testFramework, StreamTextureSourceId(0u));

        return testResult;
    }

    bool StreamTextureRendererEventTests::runTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase)
    {
        Bool testResultValue = true;
        const StreamTextureSourceId streamTextureSourceId(EmbeddedCompositorScene::GetStreamTextureSourceId());

        testFramework.setEnvironmentVariableWaylandDisplay();

        switch(testCase.m_id)
        {
        case SurfaceAvailableEventGeneratedWhenBufferAttached:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            testResultValue &= renderAndExpectNoStreamSurfaceAvailabilityChanged(testFramework, streamTextureSourceId);

            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testResultValue &= renderAndExpectNoStreamSurfaceAvailabilityChanged(testFramework, streamTextureSourceId);

            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testResultValue &= renderAndExpectNoStreamSurfaceAvailabilityChanged(testFramework, streamTextureSourceId);

            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.waitForStreamSurfaceAvailabilityChange(streamTextureSourceId, true);

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case SurfaceUnavailableEventGeneratedWhenBufferDetached:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.waitForStreamSurfaceAvailabilityChange(streamTextureSourceId, true);

            testFramework.sendDetachBufferFromSurfaceToTestApplication(surfaceId);
            testFramework.waitForUnavailablilityOfContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.waitForStreamSurfaceAvailabilityChange(streamTextureSourceId, false);

            testFramework.stopTestApplicationAndWaitUntilDisconnected();

            break;
        }

        case SurfaceUnavailableEventGeneratedWhenSurfaceDestroyed:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.waitForStreamSurfaceAvailabilityChange(streamTextureSourceId, true);

            testFramework.sendDestroySurfaceToTestApplication(surfaceId);
            testFramework.waitForUnavailablilityOfContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.waitForStreamSurfaceAvailabilityChange(streamTextureSourceId, false);

            testFramework.stopTestApplicationAndWaitUntilDisconnected();

            break;
        }

        case SurfaceUnavailableEventGeneratedWhenIviSurfaceDestroyed:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.waitForStreamSurfaceAvailabilityChange(streamTextureSourceId, true);

            testFramework.sendDestroyIVISurfaceToTestApplication(surfaceId);
            testFramework.waitForUnavailablilityOfContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.waitForStreamSurfaceAvailabilityChange(streamTextureSourceId, false);

            testFramework.sendDestroySurfaceToTestApplication(surfaceId);
            testFramework.stopTestApplicationAndWaitUntilDisconnected();

            break;
        }

        case SurfaceUnavailableEventGeneratedWhenClientIsKilled:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.waitForStreamSurfaceAvailabilityChange(streamTextureSourceId, true);

            testFramework.killTestApplication();
            testFramework.waitUntilNumberOfCompositorConnections(0, true);
            testResultValue &= testFramework.waitForStreamSurfaceAvailabilityChange(streamTextureSourceId, false);

            break;
        }

        case NoSurfaceEventGeneratedWhenBufferAttachedAndDetachedInSameLoop:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);

            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testFramework.sendDetachBufferFromSurfaceToTestApplication(surfaceId);
            testFramework.waitForUnavailablilityOfContentOnStreamTexture(streamTextureSourceId);

            testResultValue &= renderAndExpectNoStreamSurfaceAvailabilityChanged(testFramework, streamTextureSourceId);

            testFramework.stopTestApplicationAndWaitUntilDisconnected();

            break;
        }

        case SurfaceAvailableAndUnavailableEventsGeneratedWhenBufferAttachedAndDetachedInDifferentLoops:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);

            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testFramework.renderOneFrame(); //to process the event

            testFramework.sendDetachBufferFromSurfaceToTestApplication(surfaceId);
            testFramework.waitForUnavailablilityOfContentOnStreamTexture(streamTextureSourceId);

            const BoolVector events = renderAndGetSurfaceAvailabilityChangeEvents(testFramework, streamTextureSourceId);

            testResultValue &= (2u == events.size()
                                && true == events[0]
                                && false == events[1]
                                );

            testFramework.stopTestApplicationAndWaitUntilDisconnected();

            break;
        }

        case SurfaceAvailableEventsGeneratedTwiceWhenBufferReattached:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);

            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testFramework.renderOneFrame(); //to process the event

            testFramework.sendDetachBufferFromSurfaceToTestApplication(surfaceId);
            testFramework.waitForUnavailablilityOfContentOnStreamTexture(streamTextureSourceId);
            testFramework.renderOneFrame(); //to process the event

            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);

            const BoolVector events = renderAndGetSurfaceAvailabilityChangeEvents(testFramework, streamTextureSourceId);
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
            testResultValue &= renderAndExpectNoStreamSurfaceAvailabilityChanged(testFramework, streamTextureSourceId);

            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testResultValue &= renderAndExpectNoStreamSurfaceAvailabilityChanged(testFramework, streamTextureSourceId);

            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testResultValue &= renderAndExpectNoStreamSurfaceAvailabilityChanged(testFramework, streamTextureSourceId);

            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.waitForStreamSurfaceAvailabilityChange(streamTextureSourceId, true);

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case SurfaceUnavailableEventGeneratedWhenBufferDetached_NoSceneAvailable:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.waitForStreamSurfaceAvailabilityChange(streamTextureSourceId, true);

            testFramework.sendDetachBufferFromSurfaceToTestApplication(surfaceId);
            testFramework.waitForUnavailablilityOfContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.waitForStreamSurfaceAvailabilityChange(streamTextureSourceId, false);

            testFramework.stopTestApplicationAndWaitUntilDisconnected();

            break;
        }
        default:
            assert(false);
        }

        return testResultValue;
    }

    bool StreamTextureRendererEventTests::renderAndExpectNoStreamSurfaceAvailabilityChanged(EmbeddedCompositingTestsFramework& testFramework, StreamTextureSourceId streamSourceId)
    {
        return 0u == renderAndGetSurfaceAvailabilityChangeEvents(testFramework, streamSourceId).size();
    }

    BoolVector StreamTextureRendererEventTests::renderAndGetSurfaceAvailabilityChangeEvents(EmbeddedCompositingTestsFramework& testFramework, StreamTextureSourceId streamSourceId)
    {
        class StreamAvailabilityChangeEventHandler : public ramses::RendererSceneControlEventHandlerEmpty
        {
        public:
            explicit StreamAvailabilityChangeEventHandler(StreamTextureSourceId targetStreamSourceId)
                : m_targetStreamSourceId(targetStreamSourceId)
            {
            }

            virtual void streamAvailabilityChanged(ramses::waylandIviSurfaceId_t eventStreamSourceId, bool state) override
            {
                if(eventStreamSourceId.getValue() == m_targetStreamSourceId.getValue())
                {
                    m_streamAvailabilityChangeEvents.push_back(state);
                }
            }

            const BoolVector& getStreamAvailabilityChanged() const
            {
                return m_streamAvailabilityChangeEvents;
            }

        private:
            const StreamTextureSourceId m_targetStreamSourceId;
            BoolVector m_streamAvailabilityChangeEvents;
        };

        testFramework.renderOneFrame();

        ramses::RendererEventHandlerEmpty dummy;
        StreamAvailabilityChangeEventHandler handler(streamSourceId);
        testFramework.dispatchRendererEvents(dummy, handler);
        return handler.getStreamAvailabilityChanged();
    }
}
