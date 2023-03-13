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
#include "PlatformAbstraction/PlatformMath.h"
#include "wayland-server-protocol.h"
#include <sstream>

namespace ramses_internal
{
    SingleStreamTextureTests::SingleStreamTextureTests()
    {
    }

    void SingleStreamTextureTests::setUpEmbeddedCompositingTestCases(EmbeddedCompositingTestsFramework& testFramework)
    {
        ramses::DisplayConfig displayConfig = RendererTestUtils::CreateTestDisplayConfig(0u, true);
        displayConfig.setWindowRectangle(0u, 0u, IntegrationScene::DefaultViewportWidth, IntegrationScene::DefaultViewportHeight);
        displayConfig.setWaylandEmbeddedCompositingSocketName(EmbeddedCompositingTestsFramework::TestEmbeddedCompositingDisplayName.c_str());

        testFramework.createTestCase(ShowFallbackTexture, *this, "ShowFallbackTexture").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(ShowStreamTexture, *this, "ShowStreamTexture").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(StreamTextureWithDifferentSizeFromFallbackTexture, *this, "StreamTextureWithDifferentSizeFromFallbackTexture").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(ShowStreamTextureWithTexCoordsOffset, *this, "ShowStreamTextureWithTexCoordsOffset").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(ShowFallbackTextureAfterSurfaceIsDestroyed, *this, "ShowFallbackTextureAfterSurfaceIsDestroyed").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(StreamTextureContentAvailableBeforeSceneCreated, *this, "StreamTextureCreatedAfterClientUpdate").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(TestCorrectNumberOfCommitedFrames, *this, "TestCorrectNumberOfCommitedFrames").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(ShowStreamTextureAfterChangingSurfaceSize, *this, "ShowStreamTextureAfterChangingSurfaceSize").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(ShowStreamTextureAfterRecreatingSurfaceWithDifferentSize, *this, "ShowStreamTextureAfterRecreatingSurfaceWithDifferentSize").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(ShowStreamTextureAfterRecreatingScene, *this, "ShowStreamTextureAfterRecreatingScene").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(ShowFallbackTextureWhenNoSurfaceUpdate, *this, "ShowFallbackTextureWhenNoSurfaceUpdate").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(EachFrameOfClientIsSynchronouslyDisplayedByRenderer, *this, "EachFrameOfClientIsSynchronouslyDisplayedByRenderer").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(SwapIntervalZeroDoesNotBlockClient, *this, "SwapIntervalZeroDoesNotBlockClient").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(SwapIntervalOneBlocksClient, *this, "SwapIntervalOneBlocksClient").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(ShowStreamTextureAfterClientRecreated, *this, "ShowStreamTextureAfterClientRecreated").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(ShowStreamTextureAfterSurfaceRecreated, *this, "ShowStreamTextureAfterSurfaceRecreated").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(ClientReceivesFrameCallback, *this, "ClientReceivesFrameCallback").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(ClientCanBindMultipleTimesToEmbeddedCompositor, *this, "ClientCanBindMultipleTimesToEmbeddedCompositor").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(ShowFallbackTextureWhenClientIsKilled, *this, "ShowFallbackTextureWhenClientIsKilled").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(ClientCreatesTwoSurfacesWithSameIVIIdSendsErrorToClient, *this, "ClientCreatesTwoSurfacesWithSameIVIIdDoesNotCrashRenderer").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(ClientUsesShellSurface, *this, "ClientUsesShellSurface").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(ShowFallbackTextureWhenIVISurfaceDestroyed, *this, "ShowFallbackTextureWhenIVISurfaceDestroyed").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(ClientCanNotCreateTwoIVISurfacesWithSameIdForSameSurface, *this, "ClientCanNotCreateTwoIVISurfacesWithSameIdForSameSurface").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(ClientCanNotCreateTwoIVISurfacesWithDifferentIdsForSameSurface, *this, "ClientCanNotCreateTwoIVISurfacesWithDifferentIdsForSameSurface").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(ClientRecreatesIVISurfaceWithSameId, *this, "ClientRecreatesIVISurfaceWithSameId").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(ShowStreamTextureWhenIVISurfaceIsCreatedAfterUpdate, *this, "ShowStreamTextureWhenIVISurfaceIsCreatedAfterUpdate").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(ClientCanNotUseShellSurfaceWhenSurfaceHasBeenDeleted, *this, "ClientCanNotUseShellSurfaceWhenSurfaceHasBeenDeleted").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(ClientCanNotCreateTwoShellSurfacesForSameSurface, *this, "ClientCanNotCreateTwoShellSurfacesForSameSurface").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(SurfaceHasNoTitleWhenShellSurfaceDestroyed, *this, "SurfaceHasNoTitleWhenShellSurfaceDestroyed").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(CompositorLogsInfo, *this, "CompositorLogsInfo").m_displayConfigs.push_back(displayConfig);
    }

    bool SingleStreamTextureTests::runEmbeddedCompositingTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase)
    {
        bool testResultValue = true;
        constexpr WaylandIviSurfaceId waylandSurfaceIviId1{574u};
        constexpr WaylandIviSurfaceId waylandSurfaceIviId2{7889u};

        const auto streamBuffer = testFramework.createStreamBuffer(0u, ramses::waylandIviSurfaceId_t{ waylandSurfaceIviId1.getValue()});

        switch(testCase.m_id)
        {
        case ShowFallbackTexture:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.createBufferDataLink(streamBuffer, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);

            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId2);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId2);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case ShowStreamTexture:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.createBufferDataLink(streamBuffer, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);

            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case StreamTextureWithDifferentSizeFromFallbackTexture:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.createBufferDataLink(streamBuffer, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);

            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(3840, 10, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_BlueTriangleStreamTexture_SmallRes");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case ShowStreamTextureWithTexCoordsOffset:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE_WITH_TEXCOORDS_OFFSET);
            testFramework.createBufferDataLink(streamBuffer, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_WithTexCoordsOffset");
            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(3840, 10, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_BlueTriangleStreamTexture_WithTexCoordsOffset");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case ShowFallbackTextureAfterSurfaceIsDestroyed:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.createBufferDataLink(streamBuffer, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);

            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);
            testFramework.sendDestroySurfaceToTestApplication(surfaceId);
            testFramework.waitForSurfaceUnavailableForStreamTexture(waylandSurfaceIviId1);

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case StreamTextureContentAvailableBeforeSceneCreated:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.createBufferDataLink(streamBuffer, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);

            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case TestCorrectNumberOfCommitedFrames:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testResultValue &= testFramework.waitUntilNumberOfCommitedFramesForIviSurface(WaylandIviSurfaceId(waylandSurfaceIviId1.getValue()), 0);
            for (uint32_t frame = 1; frame <= 10; frame++)
            {
                testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
                testResultValue &= testFramework.waitUntilNumberOfCommitedFramesForIviSurface(WaylandIviSurfaceId(waylandSurfaceIviId1.getValue()), frame);
                testFramework.renderOneFrame();
            }

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case ShowStreamTextureAfterChangingSurfaceSize:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE_WITH_TEXEL_FETCH);
            testFramework.createBufferDataLink(streamBuffer, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);

            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testResultValue &= testFramework.waitUntilNumberOfCommitedFramesForIviSurface(WaylandIviSurfaceId(waylandSurfaceIviId1.getValue()), 1);
            testFramework.renderOneFrame();

            testResultValue &= resizeSurfaceRenderAndCheckOneFrame(testFramework, surfaceId, 8, 8, waylandSurfaceIviId1, 2, "EC_RedTriangleStreamTexture_SmallRes");
            testResultValue &= resizeSurfaceRenderAndCheckOneFrame(testFramework, surfaceId, 384, 384, waylandSurfaceIviId1, 3, "EC_RedTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case ShowStreamTextureAfterRecreatingSurfaceWithDifferentSize:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE_WITH_TEXEL_FETCH);
            testFramework.createBufferDataLink(streamBuffer, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);

            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);

            testResultValue &= recreateSurfaceRenderAndCheckOneFrame(testFramework, surfaceId, 8, 8, waylandSurfaceIviId1, "EC_RedTriangleStreamTexture_SmallRes");
            testResultValue &= recreateSurfaceRenderAndCheckOneFrame(testFramework, surfaceId, 384, 384, waylandSurfaceIviId1, "EC_RedTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case ShowStreamTextureAfterRecreatingScene:
        {
            const auto sceneId1 = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.createBufferDataLink(streamBuffer, sceneId1, EmbeddedCompositorScene::SamplerConsumerId1);

            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);

            testFramework.getScenesRegistry().destroyScenes();
            const auto sceneId2= testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.createBufferDataLink(streamBuffer, sceneId2, EmbeddedCompositorScene::SamplerConsumerId1);

            testFramework.createBufferDataLink(streamBuffer, sceneId2, EmbeddedCompositorScene::SamplerConsumerId1);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case ShowFallbackTextureWhenNoSurfaceUpdate:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.createBufferDataLink(streamBuffer, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);

            testFramework.startTestApplicationAndWaitUntilConnected();
            testFramework.waitForSurfaceUnavailableForStreamTexture(waylandSurfaceIviId1);
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.waitForSurfaceAvailableForStreamTexture(waylandSurfaceIviId1);

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case EachFrameOfClientIsSynchronouslyDisplayedByRenderer:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.createBufferDataLink(streamBuffer, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);

            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);

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

            testResultValue = renderFramesOnTestApplicationAndTakeScreenshots(testFramework, surfaceId, waylandSurfaceIviId1, 30u, colors, screenshotFiles, 4u);

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case SwapIntervalZeroDoesNotBlockClient:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 0);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);

            const uint32_t frameCount = 20;
            for (uint32_t frame = 1; frame <= frameCount; frame++)
            {
                testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            }
            // waitUntilNumberOfCommitedFramesForIviSurface does no rendering -> would block without swapInterval 0
            testResultValue = testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId1, frameCount);

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case SwapIntervalOneBlocksClient:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);

            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);

            // First frame does not block
            testResultValue = testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId1, 1);

            // Second frame is blocked, must be timed out
            testResultValue &= !testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId1, 2, 5000);

            // Render 1st frame and unblock test client, so that it can render the next frame
            testFramework.renderOneFrame();

            // Now, client is unblocked
            testResultValue &= testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId1, 2);

            // Render 2nd frame to unblock test client, so that it can be destroyed without blocking
            testFramework.renderOneFrame();

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case ShowStreamTextureAfterClientRecreated:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.createBufferDataLink(streamBuffer, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);

            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);

            testFramework.renderOneFrame();

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            testFramework.startTestApplicationAndWaitUntilConnected();

            TestApplicationSurfaceId surfaceIdNew = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceIdNew, waylandSurfaceIviId1);
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceIdNew);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);

            testFramework.createBufferDataLink(streamBuffer, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);

            testResultValue = testFramework.renderAndCompareScreenshot("EC_BlueTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case ShowStreamTextureAfterSurfaceRecreated:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();

            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.createBufferDataLink(streamBuffer, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);

            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);

            testFramework.renderOneFrame();

            testFramework.sendDestroySurfaceToTestApplication(surfaceId);
            testFramework.waitForUnavailablilityOfContentOnStreamTexture(waylandSurfaceIviId1);

            TestApplicationSurfaceId surfaceIdNew = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceIdNew, waylandSurfaceIviId1);
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceIdNew);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);

            testResultValue = testFramework.renderAndCompareScreenshot("EC_BlueTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case ClientReceivesFrameCallback:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 0);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);

            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId, true);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId, true);

            // First frame does not block
            testResultValue &= testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId1, 1);

            // Second frame is blocked, must be timed out
            testResultValue &= !testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId1, 2, 5000);

            // Render one frame, which sends the frame callback and unblock test client, so that it can render the next frame
            testFramework.renderOneFrame();

            // Now, client is unblocked
            testResultValue &= testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId1, 2);

            // Unblock waiting of second frame callback
            testFramework.renderOneFrame();

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case ClientCanBindMultipleTimesToEmbeddedCompositor:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.createBufferDataLink(streamBuffer, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);

            testFramework.sendAdditionalConnectToEmbeddedCompositorToTestApplication();
            testFramework.waitUntilNumberOfCompositorConnections(2);

            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case ShowFallbackTextureWhenClientIsKilled:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.createBufferDataLink(streamBuffer, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);

            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);

            testFramework.killTestApplication();
            testFramework.waitUntilNumberOfCompositorConnections(0);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1");
            break;
        }

        case ClientCreatesTwoSurfacesWithSameIVIIdSendsErrorToClient:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId1 = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId1, waylandSurfaceIviId1);
            TestApplicationSurfaceId surfaceId2 = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId2, waylandSurfaceIviId1);

            // The embedded compositor sends a wl_resource_post_error to the client, for the second
            // ivi_application_surface_create. This terminates the wayland display connection of the client to the
            // embedded compositor.
            testFramework.waitUntilNumberOfCompositorConnections(0, true);

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case ClientUsesShellSurface:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.createBufferDataLink(streamBuffer, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);

            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);

            // Rendering and wait is just done, to ensure that the surface is created in the EC.
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId1, 1);
            testFramework.renderOneFrame();

            if (testFramework.getTitleOfIviSurface(waylandSurfaceIviId1) != "")
            {
                LOG_ERROR(CONTEXT_RENDERER,
                          "SingleStreamTextureTests::runEmbeddedCompositingTestCase Title is not empty!");
                testResultValue = false;
            }

            TestApplicationShellSurfaceId shellSurfaceId = testFramework.sendCreateShellSurfaceToTestApplication(surfaceId);
            testFramework.sendSetShellSurfaceTitleToTestApplication(shellSurfaceId, "TestWaylandApplication");
            testFramework.sendSetShellSurfaceDummyValuesToTestApplication(surfaceId, shellSurfaceId);

            // Rendering and wait is just done, to ensure that the shell surface title has also arrived in the EC.
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId1, 2);

            if (testFramework.getTitleOfIviSurface(waylandSurfaceIviId1) != "TestWaylandApplication")
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
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.createBufferDataLink(streamBuffer, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);

            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture");

            testFramework.sendDestroyIVISurfaceToTestApplication(surfaceId);
            testFramework.waitForUnavailablilityOfContentOnStreamTexture(waylandSurfaceIviId1);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case ClientCanNotCreateTwoIVISurfacesWithSameIdForSameSurface:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);

            // The embedded compositor sends a wl_resource_post_error to the client, for the second
            // ivi_application_surface_create. This terminates the wayland display connection of the client to the
            // embedded compositor.

            testFramework.waitUntilNumberOfCompositorConnections(0);
            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case ClientCanNotCreateTwoIVISurfacesWithDifferentIdsForSameSurface:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId2);

            // The embedded compositor sends a wl_resource_post_error to the client, for the second
            // ivi_application_surface_create. This terminates the wayland display connection of the client to the
            // embedded compositor.

            testFramework.waitUntilNumberOfCompositorConnections(0);
            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case ClientRecreatesIVISurfaceWithSameId:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.createBufferDataLink(streamBuffer, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);

            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture");

            testFramework.sendDestroyIVISurfaceToTestApplication(surfaceId);
            testFramework.waitForUnavailablilityOfContentOnStreamTexture(waylandSurfaceIviId1);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1");

            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);

            testFramework.createBufferDataLink(streamBuffer, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case ShowStreamTextureWhenIVISurfaceIsCreatedAfterUpdate:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.createBufferDataLink(streamBuffer, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);

            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId  = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);

            testResultValue = testFramework.waitUntilNumberOfCommitedFramesForIviSurface(WaylandIviSurfaceId::Invalid(), 1);

            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);

            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case ClientCanNotUseShellSurfaceWhenSurfaceHasBeenDeleted:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId  = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            TestApplicationShellSurfaceId shellSurfaceId = testFramework.sendCreateShellSurfaceToTestApplication(surfaceId);
            testFramework.sendDestroySurfaceToTestApplication(surfaceId);
            testFramework.sendSetShellSurfaceTitleToTestApplication(shellSurfaceId, "TestWaylandApplication");
            TestApplicationSurfaceId surfaceId2  = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendSetShellSurfaceDummyValuesToTestApplication(surfaceId2, shellSurfaceId);
            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case ClientCanNotCreateTwoShellSurfacesForSameSurface:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
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
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.createBufferDataLink(streamBuffer, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);

            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            TestApplicationShellSurfaceId shellSurfaceId =
                testFramework.sendCreateShellSurfaceToTestApplication(surfaceId);
            testFramework.sendSetShellSurfaceTitleToTestApplication(shellSurfaceId, "TestWaylandApplication");

            // Rendering and wait is just done, to ensure that the shell surface title has also arrived in the EC.
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);

            if (testFramework.getTitleOfIviSurface(waylandSurfaceIviId1) != "TestWaylandApplication")
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
            // testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId1, 2);

            testFramework.stopTestApplicationAndWaitUntilDisconnected();

            if (testFramework.getTitleOfIviSurface(waylandSurfaceIviId1) != "")
            {
                LOG_ERROR(CONTEXT_RENDERER,
                          "SingleStreamTextureTests::runEmbeddedCompositingTestCase Title is not empty!");
                testResultValue = false;
            }
            break;
        }

        case CompositorLogsInfo:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);

            TestApplicationShellSurfaceId shellSurfaceId = testFramework.sendCreateShellSurfaceToTestApplication(surfaceId);
            testFramework.sendSetShellSurfaceTitleToTestApplication(shellSurfaceId, "TestWaylandApplication");

            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);

            testFramework.waitForSurfaceAvailableForStreamTexture(waylandSurfaceIviId1);

            RendererLogContext context(ERendererLogLevelFlag_Details);
            testFramework.logEmbeddedCompositor(context);

            const std::string outputString = context.getStream().c_str();

            std::stringstream expectedLogBuf;
            expectedLogBuf << "1 connected wayland client(s)\n"
                           << "  [ivi-surface-id: " << waylandSurfaceIviId1.getValue() << "; title: \"TestWaylandApplication\"]\n";

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

        testFramework.destroyStreamBuffer(0u, streamBuffer);

        return testResultValue;
    }

    bool SingleStreamTextureTests::recreateSurfaceRenderAndCheckOneFrame(EmbeddedCompositingTestsFramework& testFramework,
                                                                         TestApplicationSurfaceId& testSurfaceIdOut,
                                                                         UInt32 surfaceWidth,
                                                                         UInt32 surfaceHeight,
                                                                         WaylandIviSurfaceId waylandSurfaceIviId,
                                                                         const ramses_internal::String& expectedImageName)
    {
        testFramework.sendDestroySurfaceToTestApplication(testSurfaceIdOut);
        testFramework.waitForSurfaceUnavailableForStreamTexture(waylandSurfaceIviId);
        testSurfaceIdOut = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(surfaceWidth, surfaceHeight, 1);
        testFramework.sendCreateIVISurfaceToTestApplication(testSurfaceIdOut, waylandSurfaceIviId);
        testFramework.sendRenderOneFrameToEGLBufferToTestApplication(testSurfaceIdOut);
        testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId);

        //testFramework.createBufferDataLink(streamBuffer, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);

        return testFramework.renderAndCompareScreenshot(expectedImageName);
    }

    bool SingleStreamTextureTests::resizeSurfaceRenderAndCheckOneFrame(EmbeddedCompositingTestsFramework& testFramework,
                                                                       TestApplicationSurfaceId testSurfaceId,
                                                                       UInt32 surfaceWidth,
                                                                       UInt32 surfaceHeight,
                                                                       WaylandIviSurfaceId waylandSurfaceIviId,
                                                                       UInt32 frameCount,
                                                                       const ramses_internal::String& expectedImageName)
    {
        testFramework.sendSetSurfaceSizeToTestApplicaton(testSurfaceId, surfaceWidth, surfaceHeight);
        testFramework.sendRenderOneFrameToEGLBufferToTestApplication(testSurfaceId);
        const Bool frameCountReached = testFramework.waitUntilNumberOfCommitedFramesForIviSurface(WaylandIviSurfaceId(waylandSurfaceIviId.getValue()), frameCount);
        return frameCountReached && testFramework.renderAndCompareScreenshot(expectedImageName);
    }

    bool SingleStreamTextureTests::renderFramesOnTestApplicationAndTakeScreenshots(EmbeddedCompositingTestsFramework& testFramework,
                                                                                   TestApplicationSurfaceId testSurfaceId,
                                                                                   WaylandIviSurfaceId waylandSurfaceIviId,
                                                                                   const UInt32 frameCountToRender,
                                                                                   const ETriangleColor triangleColors[],
                                                                                   const String screenshotFiles[],
                                                                                   const UInt32 triangleColorCount)
    {
        for (UInt32 frameIndex = 0u; frameIndex < frameCountToRender; ++frameIndex)
        {
            const ETriangleColor color = triangleColors[frameIndex % triangleColorCount];
            testFramework.sendSetTriangleColorToTestApplication(color);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(testSurfaceId);
        }

        Bool testResult = true;

        for (UInt32 frameIndex = 0u; frameIndex < frameCountToRender; ++frameIndex)
        {
            testResult &= testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId, frameIndex + 1);

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
}
