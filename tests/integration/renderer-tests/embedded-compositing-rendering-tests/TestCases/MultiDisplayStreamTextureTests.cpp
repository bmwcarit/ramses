//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "MultiDisplayStreamTextureTests.h"
#include "TestScenes/EmbeddedCompositorScene.h"
#include "internal/RendererLib/DisplayConfig.h"
#include "ETriangleColor.h"
#include "impl/DisplayConfigImpl.h"

namespace ramses::internal
{
    void MultiDisplayStreamTextureTests::setUpEmbeddedCompositingTestCases(EmbeddedCompositingTestsFramework& testFramework)
    {
        testFramework.createTestCaseWithoutRenderer(TwoDisplaysWithCompositingOnFirstDisplayOnly, *this, "TwoDisplaysWithCompositingOnFirstDisplayOnly");
        testFramework.createTestCaseWithoutRenderer(TwoDisplaysWithCompositingOnSecondDisplayOnly, *this, "TwoDisplaysWithCompositingOnSecondDisplayOnly");
        testFramework.createTestCaseWithoutRenderer(TwoDisplaysWithCompositingOnBothDisplays, *this, "TwoDisplaysWithCompositingOnBothDisplays");
    }

    bool MultiDisplayStreamTextureTests::runEmbeddedCompositingTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase)
    {
        const ramses::RendererConfig rendererConfig = RendererTestUtils::CreateTestRendererConfig();

        ramses::DisplayConfig displayConfigWithEC = RendererTestUtils::CreateTestDisplayConfig(0, true);
        displayConfigWithEC.setWindowRectangle(0, 0, DisplayWidth, DisplayHeight);
        displayConfigWithEC.setWaylandEmbeddedCompositingSocketName(EmbeddedCompositingTestsFramework::TestEmbeddedCompositingDisplayName.c_str());

        ramses::DisplayConfig displayConfigWithoutEC = RendererTestUtils::CreateTestDisplayConfig(1, true);
        displayConfigWithoutEC.setWindowRectangle(DisplayWidth, 0, DisplayWidth, DisplayHeight);
        displayConfigWithoutEC.setWaylandEmbeddedCompositingSocketName("");
        displayConfigWithoutEC.setWaylandEmbeddedCompositingSocketGroup("");

        ramses::DisplayConfig displayConfigWithOtherEC = RendererTestUtils::CreateTestDisplayConfig(2, true);
        displayConfigWithOtherEC.setWindowRectangle(2 * DisplayWidth, 0, DisplayWidth, DisplayHeight);
        displayConfigWithOtherEC.setWaylandEmbeddedCompositingSocketName(EmbeddedCompositingTestsFramework::TestAlternateEmbeddedCompositingDisplayName.c_str());

        ramses::DisplayConfig otherDisplayConfigWithoutEC = RendererTestUtils::CreateTestDisplayConfig(3, true);
        otherDisplayConfigWithoutEC.setWindowRectangle(3 * DisplayWidth, 0, DisplayWidth, DisplayHeight);
        otherDisplayConfigWithoutEC.setWaylandEmbeddedCompositingSocketName("");
        otherDisplayConfigWithoutEC.setWaylandEmbeddedCompositingSocketGroup("");

        bool testResultValue = true;

        constexpr WaylandIviSurfaceId waylandSurfaceIviId1{157u};

        switch(testCase.m_id)
        {
        case TwoDisplaysWithCompositingOnFirstDisplayOnly:
        {
            testFramework.initializeRenderer(rendererConfig);
            testFramework.createDisplay(displayConfigWithEC);
            testFramework.createDisplay(displayConfigWithoutEC);

            const auto sceneId1 = createAndShowScene<EmbeddedCompositorScene>(testFramework, EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, DisplayWidth, DisplayHeight, 0u);
            const auto sceneId2 = createAndShowScene<EmbeddedCompositorScene>(testFramework, EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, DisplayWidth, DisplayHeight, 1u);

            const auto streamBufferDisp1 = testFramework.createStreamBuffer(0u, waylandIviSurfaceId_t{ waylandSurfaceIviId1.getValue()});
            const auto streamBufferDisp2 = testFramework.createStreamBuffer(1u, waylandIviSurfaceId_t{ waylandSurfaceIviId1.getValue()});
            testFramework.createBufferDataLink(streamBufferDisp1, sceneId1, EmbeddedCompositorScene::SamplerConsumerId1);
            testFramework.createBufferDataLink(streamBufferDisp2, sceneId2, EmbeddedCompositorScene::SamplerConsumerId1);

            //fallback on both displays
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1", 0u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1", 1u);

            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture"   , 0u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1"          , 1u);

            testFramework.destroyStreamBuffer(0u, streamBufferDisp1);
            testFramework.destroyStreamBuffer(1u, streamBufferDisp2);

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case TwoDisplaysWithCompositingOnSecondDisplayOnly:
        {
            testFramework.initializeRenderer(rendererConfig);
            testFramework.createDisplay(displayConfigWithoutEC);
            testFramework.createDisplay(displayConfigWithEC);

            const auto sceneId1 = createAndShowScene<EmbeddedCompositorScene>(testFramework, EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, DisplayWidth, DisplayHeight, 0u);
            const auto sceneId2 = createAndShowScene<EmbeddedCompositorScene>(testFramework, EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, DisplayWidth, DisplayHeight, 1u);

            const auto streamBufferDisp1 = testFramework.createStreamBuffer(0u, waylandIviSurfaceId_t{ waylandSurfaceIviId1.getValue()});
            const auto streamBufferDisp2 = testFramework.createStreamBuffer(1u, waylandIviSurfaceId_t{ waylandSurfaceIviId1.getValue()});
            testFramework.createBufferDataLink(streamBufferDisp1, sceneId1, EmbeddedCompositorScene::SamplerConsumerId1);
            testFramework.createBufferDataLink(streamBufferDisp2, sceneId2, EmbeddedCompositorScene::SamplerConsumerId1);

            //fallback on both displays
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1", 0u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1", 1u);

            testFramework.startTestApplicationAndWaitUntilConnected(EConnectionMode::DisplayName, 1u);
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1, 1u);

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1"          , 0u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture"   , 1u);

            testFramework.destroyStreamBuffer(0u, streamBufferDisp1);
            testFramework.destroyStreamBuffer(1u, streamBufferDisp2);

            testFramework.stopTestApplicationAndWaitUntilDisconnected(1u);
            break;
        }
        case TwoDisplaysWithCompositingOnBothDisplays:
        {
            testFramework.initializeRenderer(rendererConfig);
            testFramework.createDisplay(displayConfigWithEC);
            testFramework.createDisplay(displayConfigWithOtherEC);

            const auto sceneId1 = createAndShowScene<EmbeddedCompositorScene>(testFramework, EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, DisplayWidth, DisplayHeight, 0u);
            const auto sceneId2 = createAndShowScene<EmbeddedCompositorScene>(testFramework, EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, DisplayWidth, DisplayHeight, 1u);

            const auto streamBufferDisp1 = testFramework.createStreamBuffer(0u, waylandIviSurfaceId_t{ waylandSurfaceIviId1.getValue()});
            const auto streamBufferDisp2 = testFramework.createStreamBuffer(1u, waylandIviSurfaceId_t{ waylandSurfaceIviId1.getValue()});
            testFramework.createBufferDataLink(streamBufferDisp1, sceneId1, EmbeddedCompositorScene::SamplerConsumerId1);
            testFramework.createBufferDataLink(streamBufferDisp2, sceneId2, EmbeddedCompositorScene::SamplerConsumerId1);

            //fallback on both displays
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1", 0u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1", 1u);

            //start test app that gets connected to 1st display
            testFramework.startTestApplicationAndWaitUntilConnected(EConnectionMode::DisplayName, 0u, 0u);

            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1, 0u);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1, 0u);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId, false, 0u);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1, 0u);

            //composited texture only on 1st display
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture"   , 0u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1"          , 1u);

            //start 2nd test app that gets connected to 2nd display
            testFramework.startTestApplicationAndWaitUntilConnected(EConnectionMode::AlternateDisplayName, 1u, 1u);

            const TestApplicationSurfaceId surfaceId2 = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1, 1u);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId2, waylandSurfaceIviId1, 1u);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId2, false, 1u);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1, 1u);

            //composited texture on both displays
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture"   , 0u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture"   , 1u);

            //stop 1st test app
            testFramework.stopTestApplicationAndWaitUntilDisconnected(0u, 0u);

            //composited texture only on 2nd display
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1"          , 0u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture"   , 1u);

            //stop 2nd test app
            testFramework.stopTestApplicationAndWaitUntilDisconnected(1u, 1u);

            //fallback on both displays
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1", 0u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1", 1u);

            testFramework.destroyStreamBuffer(0u, streamBufferDisp1);
            testFramework.destroyStreamBuffer(1u, streamBufferDisp2);

            break;
        }

        default:
            assert(false);
        }


        return testResultValue;
    }
}
