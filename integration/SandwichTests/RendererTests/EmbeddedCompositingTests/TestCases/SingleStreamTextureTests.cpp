//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SingleStreamTextureTests.h"
#include "TestScenes/EmbeddedCompositorScene.h"
#include "Utils/LogMacros.h"
#include "RendererLib/RendererLogContext.h"
#include "RendererAPI/IEmbeddedCompositor.h"
#include <sstream>

namespace ramses_internal
{
    SingleStreamTextureTests::SingleStreamTextureTests()
    {
    }

    void SingleStreamTextureTests::setUpEmbeddedCompositingTestCases(EmbeddedCompositingTestsFramework& testFramework)
    {
        testFramework.createTestCaseWithDefaultDisplay(ShowFallbackTexture, *this, "ShowFallbackTexture");
        testFramework.createTestCaseWithDefaultDisplay(ShowStreamTexture, *this, "ShowStreamTexture");
        testFramework.createTestCaseWithDefaultDisplay(StreamTextureWithDifferentSizeFromFallbackTexture, *this, "StreamTextureWithDifferentSizeFromFallbackTexture");
        testFramework.createTestCaseWithDefaultDisplay(ShowFallbackTextureAfterSurfaceIsDestroyed, *this, "ShowFallbackTextureAfterSurfaceIsDestroyed");
        testFramework.createTestCaseWithDefaultDisplay(StreamTextureContentAvailableBeforeSceneCreated, *this, "StreamTextureCreatedAfterClientUpdate");
        testFramework.createTestCaseWithDefaultDisplay(TestCorrectNumberOfCommitedFrames, *this, "TestCorrectNumberOfCommitedFrames");
        testFramework.createTestCaseWithDefaultDisplay(ShowStreamTextureAfterChangingSurfaceSize, *this, "ShowStreamTextureAfterChangingSurfaceSize");
        testFramework.createTestCaseWithDefaultDisplay(ShowStreamTextureAfterRecreatingSurfaceWithDifferentSize, *this, "ShowStreamTextureAfterRecreatingSurfaceWithDifferentSize");
        testFramework.createTestCaseWithDefaultDisplay(ShowStreamTextureAfterRecreatingScene, *this, "ShowStreamTextureAfterRecreatingScene");
        testFramework.createTestCaseWithDefaultDisplay(ShowFallbackTextureWhenNoSurfaceUpdate, *this, "ShowFallbackTextureWhenNoSurfaceUpdate");
        testFramework.createTestCaseWithDefaultDisplay(EachFrameOfClientIsSynchronouslyDisplayedByRenderer, *this, "EachFrameOfClientIsSynchronouslyDisplayedByRenderer");
        testFramework.createTestCaseWithDefaultDisplay(SwapIntervalZeroDoesNotBlockClient, *this, "SwapIntervalZeroDoesNotBlockClient");
        testFramework.createTestCaseWithDefaultDisplay(SwapIntervalOneBlocksClient, *this, "SwapIntervalOneBlocksClient");
        testFramework.createTestCaseWithDefaultDisplay(ShowStreamTextureAfterClientRecreated, *this, "ShowStreamTextureAfterClientRecreated");
        testFramework.createTestCaseWithDefaultDisplay(ShowStreamTextureAfterSurfaceRecreated, *this, "ShowStreamTextureAfterSurfaceRecreated");
        testFramework.createTestCaseWithDefaultDisplay(ClientReceivesFrameCallback, *this, "ClientReceivesFrameCallback");
        testFramework.createTestCaseWithDefaultDisplay(ClientCanBindMultipleTimesToEmbeddedCompositor, *this, "ClientCanBindMultipleTimesToEmbeddedCompositor");
        testFramework.createTestCaseWithDefaultDisplay(ShowFallbackTextureWhenClientIsKilled, *this, "ShowFallbackTextureWhenClientIsKilled");
        testFramework.createTestCaseWithDefaultDisplay(ShowSharedMemoryStreamTexture, *this, "ShowSharedMemoryStreamTexture");
        testFramework.createTestCaseWithDefaultDisplay(ShowFallbackTextureWhenBufferIsDetachedFromSurface, *this, "ShowFallbackTextureWhenBufferIsDetachedFromSurface");
        testFramework.createTestCaseWithDefaultDisplay(ShowFallbackTextureWhenBufferIsDetachedFromSurfaceAndLastFrameNotUsedForRendering, *this, "ShowFallbackTextureWhenBufferIsDetachedFromSurfaceAndLastFrameNotUsedForRendering");
        testFramework.createTestCaseWithDefaultDisplay(ClientCreatesTwoSurfacesWithSameIVIIdAndUpdatesOnlyFirstSurface, *this, "ClientCreatesTwoSurfacesWithSameIVIIdAndUpdatesOnlyFirstSurface");
        testFramework.createTestCaseWithDefaultDisplay(ClientCreatesTwoSurfacesWithSameIVIIdAndUpdatesOnlySecondSurface, *this, "ClientCreatesTwoSurfacesWithSameIVIIdAndUpdatesOnlySecondSurface");
        testFramework.createTestCaseWithDefaultDisplay(ClientCreatesTwoSurfacesWithSameIVIIdAndUpdatesBothSurfaces, *this, "ClientCreatesTwoSurfacesWithSameIVIIdAndUpdatesBothSurfaces");
        testFramework.createTestCaseWithDefaultDisplay(ClientUsesShellSurface, *this, "ClientUsesShellSurface");
        testFramework.createTestCaseWithDefaultDisplay(ShowFallbackTextureWhenIVISurfaceDestroyed, *this, "ShowFallbackTextureWhenIVISurfaceDestroyed");
        testFramework.createTestCaseWithDefaultDisplay(ClientCanNotCreateTwoIVISurfacesWithSameIdForSameSurface, *this, "ClientCanNotCreateTwoIVISurfacesWithSameIdForSameSurface");
        testFramework.createTestCaseWithDefaultDisplay(ClientCanNotCreateTwoIVISurfacesWithDifferentIdsForSameSurface, *this, "ClientCanNotCreateTwoIVISurfacesWithDifferentIdsForSameSurface");
        testFramework.createTestCaseWithDefaultDisplay(ClientRecreatesIVISurfaceWithSameId, *this, "ClientRecreatesIVISurfaceWithSameId");
        testFramework.createTestCaseWithDefaultDisplay(ShowStreamTextureWhenIVISurfaceIsCreatedAfterUpdate, *this, "ShowStreamTextureWhenIVISurfaceIsCreatedAfterUpdate");
        testFramework.createTestCaseWithDefaultDisplay(ClientCanNotUseShellSurfaceWhenSurfaceHasBeenDeleted, *this, "ClientCanNotUseShellSurfaceWhenSurfaceHasBeenDeleted");
        testFramework.createTestCaseWithDefaultDisplay(ClientCanNotCreateTwoShellSurfacesForSameSurface, *this, "ClientCanNotCreateTwoShellSurfacesForSameSurface");
        testFramework.createTestCaseWithDefaultDisplay(SurfaceHasNoTitleWhenShellSurfaceDestroyed, *this, "SurfaceHasNoTitleWhenShellSurfaceDestroyed");
        testFramework.createTestCaseWithDefaultDisplay(ClientAttachesAndDestroysBufferWithoutCommit, *this, "ClientAttachesAndDestroysBufferWithoutCommit");
        testFramework.createTestCaseWithDefaultDisplay(CompositorLogsInfo, *this, "CompositorLogsInfo");
    }

    bool SingleStreamTextureTests::runEmbeddedCompositingTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase)
    {
        Bool testResultValue = true;
        const StreamTextureSourceId streamTextureSourceId(EmbeddedCompositorScene::GetStreamTextureSourceId());
        const StreamTextureSourceId secondStreamTextureSourceId(EmbeddedCompositorScene::GetSecondStreamTextureSourceId());

        testFramework.setEnvironmentVariableWaylandDisplay();

        switch(testCase.m_id)
        {
        case ShowFallbackTexture:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, secondStreamTextureSourceId);
            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(secondStreamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case ShowStreamTexture:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case StreamTextureWithDifferentSizeFromFallbackTexture:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(3840, 10, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_BlueTriangleStreamTexture_SmallRes");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case ShowFallbackTextureAfterSurfaceIsDestroyed:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testFramework.sendDestroySurfaceToTestApplication(surfaceId);
            testFramework.waitForUnavailablilityOfContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case StreamTextureContentAvailableBeforeSceneCreated:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case TestCorrectNumberOfCommitedFrames:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testResultValue &= testFramework.waitUntilNumberOfCommitedFramesForIviSurface(WaylandIviSurfaceId(streamTextureSourceId.getValue()), 0);
            for (uint32_t frame = 1; frame <= 10; frame++)
            {
                testFramework.sendRenderOneFrameToTestApplication(surfaceId);
                testResultValue &= testFramework.waitUntilNumberOfCommitedFramesForIviSurface(WaylandIviSurfaceId(streamTextureSourceId.getValue()), frame);
                testFramework.renderOneFrame();
            }

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case ShowStreamTextureAfterChangingSurfaceSize:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE_WITH_TEXEL_FETCH);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testResultValue &= testFramework.waitUntilNumberOfCommitedFramesForIviSurface(WaylandIviSurfaceId(streamTextureSourceId.getValue()), 1);
            testFramework.renderOneFrame();

            testResultValue &= resizeSurfaceRenderAndCheckOneFrame(testFramework, surfaceId, 8, 8, streamTextureSourceId, 2, "EC_RedTriangleStreamTexture_SmallRes");
            testResultValue &= resizeSurfaceRenderAndCheckOneFrame(testFramework, surfaceId, 384, 384, streamTextureSourceId, 3, "EC_RedTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case ShowStreamTextureAfterRecreatingSurfaceWithDifferentSize:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE_WITH_TEXEL_FETCH);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);

            testResultValue &= recreateSurfaceRenderAndCheckOneFrame(testFramework, surfaceId, 8, 8, streamTextureSourceId, "EC_RedTriangleStreamTexture_SmallRes");
            testResultValue &= recreateSurfaceRenderAndCheckOneFrame(testFramework, surfaceId, 384, 384, streamTextureSourceId, "EC_RedTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case ShowStreamTextureAfterRecreatingScene:
        {
            ramses::sceneId_t sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testFramework.hideScene(sceneId);
            testFramework.unmapScene(sceneId);
            testFramework.getScenesRegistry().destroyScenes();
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case ShowFallbackTextureWhenNoSurfaceUpdate:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            testFramework.waitForSurfaceUnavailableForStreamTexture(streamTextureSourceId);
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.waitForSurfaceAvailableForStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case EachFrameOfClientIsSynchronouslyDisplayedByRenderer:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);

            const ETriangleColor colors[] = {
                ETriangleColor::Red,
                ETriangleColor::Blue,
                ETriangleColor::DarkGrey,
                ETriangleColor::White
            };

            const String screenshotFiles[] = {"EC_RedTriangleStreamTexture",
                                            "EC_BlueTriangleStreamTexture",
                                            "EC_GrayTriangleStreamTexture",
                                            "EC_WhiteTriangleStreamTexture"};

            testResultValue = renderFramesOnTestApplicationAndTakeScreenshots(testFramework, surfaceId, streamTextureSourceId, 30u, colors, screenshotFiles, 4u);

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case SwapIntervalZeroDoesNotBlockClient:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 0);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);

            const uint32_t frameCount = 20;
            for (uint32_t frame = 1; frame <= frameCount; frame++)
            {
                testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            }
            // waitUntilNumberOfCommitedFramesForIviSurface does no rendering -> would block without swapInterval 0
            testResultValue = testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, frameCount);

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case SwapIntervalOneBlocksClient:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);

            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.sendRenderOneFrameToTestApplication(surfaceId);

            // First frame does not block
            testResultValue = testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 1);

            // Second frame is blocked, must be timed out
            testResultValue &= !testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 2, 5000);

            // Render one frame and unblock test client, so that it can render the next frame
            testFramework.renderOneFrame();

            // Now, client is unblocked
            testResultValue &= testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 2);

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case ShowStreamTextureAfterClientRecreated:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testFramework.renderOneFrame();

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            testFramework.startTestApplicationAndWaitUntilConnected();

            TestApplicationSurfaceId surfaceIdNew = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceIdNew, streamTextureSourceId);
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendRenderOneFrameToTestApplication(surfaceIdNew);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);

            testResultValue = testFramework.renderAndCompareScreenshot("EC_BlueTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case ShowStreamTextureAfterSurfaceRecreated:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testFramework.renderOneFrame();

            testFramework.sendDestroySurfaceToTestApplication(surfaceId);
            testFramework.waitForUnavailablilityOfContentOnStreamTexture(streamTextureSourceId);

            TestApplicationSurfaceId surfaceIdNew = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceIdNew, streamTextureSourceId);
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendRenderOneFrameToTestApplication(surfaceIdNew);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);

            testResultValue = testFramework.renderAndCompareScreenshot("EC_BlueTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case ClientReceivesFrameCallback:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 0);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);

            testFramework.sendRenderOneFrameAndWaitOnFrameCallbackToTestApplication(surfaceId);
            testFramework.sendRenderOneFrameAndWaitOnFrameCallbackToTestApplication(surfaceId);

            // First frame does not block
            testResultValue &= testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 1);

            // Second frame is blocked, must be timed out
            testResultValue &= !testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 2, 5000);

            // Render one frame, which sends the frame callback and unblock test client, so that it can render the next frame
            testFramework.renderOneFrame();

            // Now, client is unblocked
            testResultValue &= testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 2);

            // Unblock waiting of second frame callback
            testFramework.renderOneFrame();

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case ClientCanBindMultipleTimesToEmbeddedCompositor:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);

            testFramework.sendAdditionalConnectToEmbeddedCompositorToTestApplication();
            testFramework.waitUntilNumberOfCompositorConnections(2);

            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case ShowFallbackTextureWhenClientIsKilled:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);

            testFramework.killTestApplication();
            testFramework.waitUntilNumberOfCompositorConnections(0);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1");
            break;
        }

        case ShowSharedMemoryStreamTexture:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSharedMemorySurfaceToTestApplication(255, 255);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);

            if (testFramework.getNumberOfAllocatedSHMBufferFromTestApplication() != 0)
            {
                LOG_ERROR(CONTEXT_RENDERER, "SingleStreamTextureTests::runTest Number of allocated SHM buffers is not 0!");
                testResultValue = false;
            }

            const uint32_t cycles = 20;
            uint32_t frameCounter = 0;

            for (uint32_t i = 0; i < cycles; i++)
            {
                testResultValue &= renderAndCheckOneSharedMemoryFrame(testFramework, surfaceId, ETriangleColor::Red, streamTextureSourceId, frameCounter, "EC_ShMemRedTriangle");
                testResultValue &= renderAndCheckOneSharedMemoryFrame(testFramework, surfaceId, ETriangleColor::Blue, streamTextureSourceId, frameCounter, "EC_ShMemBlueTriangle");
                testResultValue &= renderAndCheckOneSharedMemoryFrame(testFramework, surfaceId, ETriangleColor::White, streamTextureSourceId, frameCounter, "EC_ShMemWhiteTriangle");
            }

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case ShowFallbackTextureWhenBufferIsDetachedFromSurface:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testFramework.renderOneFrame();
            testFramework.sendDetachBufferFromSurfaceToTestApplication(surfaceId);
            testFramework.waitForUnavailablilityOfContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1");
            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case ShowFallbackTextureWhenBufferIsDetachedFromSurfaceAndLastFrameNotUsedForRendering:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testFramework.sendDetachBufferFromSurfaceToTestApplication(surfaceId);
            testFramework.waitForUnavailablilityOfContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1");
            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case ClientCreatesTwoSurfacesWithSameIVIIdAndUpdatesOnlyFirstSurface:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId1 = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId1, streamTextureSourceId);
            TestApplicationSurfaceId surfaceId2 = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId2, streamTextureSourceId);

            UNUSED(surfaceId2)

            // The embedded compositor sends a wl_resource_post_error to the client, for the second
            // ivi_application_surface_create. This terminates the wayland display connection of the client to the
            // embedded compositor.

            testFramework.sendRenderOneFrameToTestApplication(surfaceId1);
            testFramework.waitUntilNumberOfCompositorConnections(0, true);

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case ClientCreatesTwoSurfacesWithSameIVIIdAndUpdatesOnlySecondSurface:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId1 = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId1, streamTextureSourceId);
            TestApplicationSurfaceId surfaceId2 = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId2, streamTextureSourceId);

            UNUSED(surfaceId1)

            // There was a bug where the update of the second surface caused crash in the embedded compositor.
            // Both surfaces were registered with the same ivi-id. When the second surface has done an update, the first surface was found for compositing
            // upload to texture, but the buffer of this surface is a nullptr.
            //
            // Now, the embedded compositor sends a wl_resource_post_error to the client, for the second ivi_application_surface_create.
            // This terminates the wayland display connection of the client to the embedded compositor.

            testFramework.sendRenderOneFrameToTestApplication(surfaceId2);
            testFramework.waitUntilNumberOfCompositorConnections(0, true);

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case ClientCreatesTwoSurfacesWithSameIVIIdAndUpdatesBothSurfaces:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId1 = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId1, streamTextureSourceId);
            TestApplicationSurfaceId surfaceId2 = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId2, streamTextureSourceId);

            // The embedded compositor sends a wl_resource_post_error to the client, for the second
            // ivi_application_surface_create. This terminates the wayland display connection of the client to the
            // embedded compositor.

            testFramework.sendRenderOneFrameToTestApplication(surfaceId1);
            testFramework.sendRenderOneFrameToTestApplication(surfaceId2);
            testFramework.waitUntilNumberOfCompositorConnections(0, true);

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case ClientUsesShellSurface:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);

            // Rendering and wait is just done, to ensure that the surface is created in the EC.
            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 1);
            testFramework.renderOneFrame();

            if (testFramework.getTitleOfIviSurface(streamTextureSourceId) != "")
            {
                LOG_ERROR(CONTEXT_RENDERER,
                          "SingleStreamTextureTests::runEmbeddedCompositingTestCase Title is not empty!");
                testResultValue = false;
            }

            TestApplicationShellSurfaceId shellSurfaceId = testFramework.sendCreateShellSurfaceToTestApplication(surfaceId);
            testFramework.sendSetShellSurfaceTitleToTestApplication(shellSurfaceId, "TestWaylandApplication");
            testFramework.sendSetShellSurfaceDummyValuesToTestApplication(surfaceId, shellSurfaceId);

            // Rendering and wait is just done, to ensure that the shell surface title has also arrived in the EC.
            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 2);

            if (testFramework.getTitleOfIviSurface(streamTextureSourceId) != "TestWaylandApplication")
            {
                LOG_ERROR(CONTEXT_RENDERER, "SingleStreamTextureTests::runEmbeddedCompositingTestCase Title does not match!");
                testResultValue = false;
            }

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case ShowFallbackTextureWhenIVISurfaceDestroyed:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture");

            testFramework.sendDestroyIVISurfaceToTestApplication(surfaceId);
            testFramework.waitForUnavailablilityOfContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case ClientCanNotCreateTwoIVISurfacesWithSameIdForSameSurface:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);

            // The embedded compositor sends a wl_resource_post_error to the client, for the second
            // ivi_application_surface_create. This terminates the wayland display connection of the client to the
            // embedded compositor.

            testFramework.waitUntilNumberOfCompositorConnections(0);
            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case ClientCanNotCreateTwoIVISurfacesWithDifferentIdsForSameSurface:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, secondStreamTextureSourceId);

            // The embedded compositor sends a wl_resource_post_error to the client, for the second
            // ivi_application_surface_create. This terminates the wayland display connection of the client to the
            // embedded compositor.

            testFramework.waitUntilNumberOfCompositorConnections(0);
            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case ClientRecreatesIVISurfaceWithSameId:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture");

            testFramework.sendDestroyIVISurfaceToTestApplication(surfaceId);
            testFramework.waitForUnavailablilityOfContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1");

            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case ShowStreamTextureWhenIVISurfaceIsCreatedAfterUpdate:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId  = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendRenderOneFrameToTestApplication(surfaceId);

            testResultValue = testFramework.waitUntilNumberOfCommitedFramesForIviSurface(InvalidWaylandIviSurfaceId, 1);

            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);

            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case ClientCanNotUseShellSurfaceWhenSurfaceHasBeenDeleted:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId  = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            TestApplicationShellSurfaceId shellSurfaceId = testFramework.sendCreateShellSurfaceToTestApplication(surfaceId);
            testFramework.sendDestroySurfaceToTestApplication(surfaceId);
            testFramework.sendSetShellSurfaceTitleToTestApplication(shellSurfaceId, "TestWaylandApplication");
            TestApplicationSurfaceId surfaceId2  = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendSetShellSurfaceDummyValuesToTestApplication(surfaceId2, shellSurfaceId);
            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case ClientCanNotCreateTwoShellSurfacesForSameSurface:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateShellSurfaceToTestApplication(surfaceId);
            testFramework.sendCreateShellSurfaceToTestApplication(surfaceId);

            // The embedded compositor sends a wl_resource_post_error to the client, for the second
            // wl_shell_get_shell_surface. This terminates the wayland display connection of the client to the
            // embedded compositor.

            testFramework.waitUntilNumberOfCompositorConnections(0);
            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case SurfaceHasNoTitleWhenShellSurfaceDestroyed:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            TestApplicationShellSurfaceId shellSurfaceId =
                testFramework.sendCreateShellSurfaceToTestApplication(surfaceId);
            testFramework.sendSetShellSurfaceTitleToTestApplication(shellSurfaceId, "TestWaylandApplication");

            // Rendering and wait is just done, to ensure that the shell surface title has also arrived in the EC.
            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);

            if (testFramework.getTitleOfIviSurface(streamTextureSourceId) != "TestWaylandApplication")
            {
                LOG_ERROR(CONTEXT_RENDERER,
                          "SingleStreamTextureTests::runEmbeddedCompositingTestCase Title does not match!");
                testResultValue = false;
            }

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture");

            testFramework.sendDestroyShellSurfaceToTestApplication(shellSurfaceId);

            // Since the wl_shell_surface_interface has no destroy function (in contrast to ivi_surface_interface), the EC will not be informed,
            // that the wl_shell_surface is destroyed on client side, until the client has disconnected.
            // Otherwise, we could move the check here, if the title is now the empty string.
            // Possible, that in future Wayland versions, this will be changed and a destroy function is then also available
            // in wl_shell_surface_interface.

            // For future versions of Wayland, which have a destroy function in wl_shell_surface_interface:
            // Rendering and wait is just done, to ensure that the destroy shell surface has arrived in the EC
            // For the current version it is not needed, since we check for the empty string after the client has disconnected.
            // A version with the destroy function, could check here for the empty string.
            //
            // testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            // testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 2);

            testFramework.stopTestApplicationAndWaitUntilDisconnected();

            if (testFramework.getTitleOfIviSurface(streamTextureSourceId) != "")
            {
                LOG_ERROR(CONTEXT_RENDERER,
                          "SingleStreamTextureTests::runEmbeddedCompositingTestCase Title is not empty!");
                testResultValue = false;
            }
            break;
        }

        case ClientAttachesAndDestroysBufferWithoutCommit:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSharedMemorySurfaceToTestApplication(384, 384);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendAttachBufferToTestApplication(surfaceId);
            testFramework.waitForBufferAttachedToIviSurface(streamTextureSourceId);
            testFramework.sendDestroyBuffersToTestApplication();
            testFramework.waitForNoBufferAttachedToIviSurface(streamTextureSourceId);
            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case CompositorLogsInfo:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);

            TestApplicationShellSurfaceId shellSurfaceId = testFramework.sendCreateShellSurfaceToTestApplication(surfaceId);
            testFramework.sendSetShellSurfaceTitleToTestApplication(shellSurfaceId, "TestWaylandApplication");

            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);

            testFramework.waitForSurfaceAvailableForStreamTexture(streamTextureSourceId);

            RendererLogContext context(ERendererLogLevelFlag_Details);
            testFramework.logEmbeddedCompositor(context);

            const std::string outputString = context.getStream().c_str();

            std::stringstream expectedLogBuf;
            expectedLogBuf << "1 connected wayland client(s)\n"
                           << "  [ivi-surface-id: " << EmbeddedCompositorScene::GetStreamTextureSourceId().getValue() << "; title: \"TestWaylandApplication\"]\n";

            if (outputString != expectedLogBuf.str())
            {
                LOG_ERROR(CONTEXT_RENDERER, "SingleStreamTextureTests::runEmbeddedCompositingTestCase  Wrong log output: " << outputString.c_str());
                testResultValue = false;
            }

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        default:
            assert(false);
        }

        return testResultValue;
    }

    bool SingleStreamTextureTests::recreateSurfaceRenderAndCheckOneFrame(EmbeddedCompositingTestsFramework& testFramework, TestApplicationSurfaceId& testSurfaceIdOut, UInt32 surfaceWidth, UInt32 surfaceHeight, StreamTextureSourceId streamTextureSourceId, const ramses_internal::String& expectedImageName)
    {
        testFramework.sendDestroySurfaceToTestApplication(testSurfaceIdOut);
        testFramework.waitForUnavailablilityOfContentOnStreamTexture(streamTextureSourceId);
        testSurfaceIdOut = testFramework.sendCreateSurfaceToTestApplication(surfaceWidth, surfaceHeight, 1);
        testFramework.sendCreateIVISurfaceToTestApplication(testSurfaceIdOut, streamTextureSourceId);
        testFramework.sendRenderOneFrameToTestApplication(testSurfaceIdOut);
        testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
        return testFramework.renderAndCompareScreenshot(expectedImageName);
    }

    bool SingleStreamTextureTests::resizeSurfaceRenderAndCheckOneFrame(EmbeddedCompositingTestsFramework& testFramework, TestApplicationSurfaceId testSurfaceId, UInt32 surfaceWidth, UInt32 surfaceHeight, StreamTextureSourceId streamTextureSourceId, UInt32 frameCount, const ramses_internal::String& expectedImageName)
    {
        testFramework.sendSetSurfaceSizeToTestApplicaton(testSurfaceId, surfaceWidth, surfaceHeight);
        testFramework.sendRenderOneFrameToTestApplication(testSurfaceId);
        const Bool frameCountReached = testFramework.waitUntilNumberOfCommitedFramesForIviSurface(WaylandIviSurfaceId(streamTextureSourceId.getValue()), frameCount);
        return frameCountReached && testFramework.renderAndCompareScreenshot(expectedImageName);
    }

    bool SingleStreamTextureTests::renderFramesOnTestApplicationAndTakeScreenshots(EmbeddedCompositingTestsFramework& testFramework, TestApplicationSurfaceId testSurfaceId, StreamTextureSourceId streamTextureSourceId, const UInt32 frameCountToRender, const ETriangleColor triangleColors[], const String screenshotFiles[], const UInt32 triangleColorCount)
    {
        for (UInt32 frameIndex = 0u; frameIndex < frameCountToRender; ++frameIndex)
        {
            const ETriangleColor color = triangleColors[frameIndex % triangleColorCount];
            testFramework.sendSetTriangleColorToTestApplication(color);
            testFramework.sendRenderOneFrameToTestApplication(testSurfaceId);
        }

        Bool testResult = true;

        for (UInt32 frameIndex = 0u; frameIndex < frameCountToRender; ++frameIndex)
        {
            testResult &= testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, frameIndex + 1);

            LOG_INFO(CONTEXT_RENDERER, "SingleStreamTextureTests::renderFramesOnTestApplicationAndTakeScreenshots(): taking screenshot for frame with index: " << frameIndex);

            const String& screenshotName = screenshotFiles[frameIndex % triangleColorCount];
            if(!testFramework.renderAndCompareScreenshot(screenshotName))
            {
                LOG_ERROR(CONTEXT_RENDERER, "SingleStreamTextureTests::renderFramesOnTestApplicationAndTakeScreenshots(): test failed for screenshot for frame with index: " << frameIndex);
                testResult = false;
            }
        }

        return testResult;
    }

    bool SingleStreamTextureTests::renderAndCheckOneSharedMemoryFrame(EmbeddedCompositingTestsFramework& testFramework,  TestApplicationSurfaceId testSurfaceId, ETriangleColor color, StreamTextureSourceId streamTextureSourceId, UInt32& frameCount, const String& expectedImageName)
    {
        testFramework.sendSetTriangleColorToTestApplication(color);
        testFramework.sendRenderOneFrameAndWaitOnFrameCallbackToTestApplication(testSurfaceId);
        testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, ++frameCount);
        if (!testFramework.renderAndCompareScreenshot(expectedImageName))
        {
            return false;
        }

        // Check, that for first frame one buffer is allocated, for second frame, two buffers are allocated, and that then
        // no more further buffers are needed. Checks if the Embedded Compositor releases the buffers.
        const UInt32 expectedNumberOfAllocatedSHMBuffer = min(frameCount, 2u);
        const UInt32 numberOfAllocatedSHMBuffer = testFramework.getNumberOfAllocatedSHMBufferFromTestApplication();

        if (numberOfAllocatedSHMBuffer != expectedNumberOfAllocatedSHMBuffer)
        {
            LOG_ERROR(CONTEXT_RENDERER, "SingleStreamTextureTests::renderAndCheckOneSharedMemoryFrame Number of allocated SHM buffers " << numberOfAllocatedSHMBuffer << " does not match expected value " << expectedNumberOfAllocatedSHMBuffer <<"!");
            return false;
        }
        return true;
    }
}
