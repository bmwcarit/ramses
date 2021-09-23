//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "MultiDisplayStreamTextureTests.h"
#include "TestScenes/EmbeddedCompositorScene.h"
#include "RendererLib/DisplayConfig.h"
#include "ETriangleColor.h"
#include "DisplayConfigImpl.h"

namespace ramses_internal
{
    void MultiDisplayStreamTextureTests::setUpEmbeddedCompositingTestCases(EmbeddedCompositingTestsFramework& testFramework)
    {
        testFramework.createTestCaseWithoutRenderer(TwoDisplaysWithCompositingOnFirstDisplayOnly, *this, "TwoDisplaysWithCompositingOnFirstDisplayOnly");
        testFramework.createTestCaseWithoutRenderer(TwoDisplaysWithCompositingOnSecondDisplayOnly, *this, "TwoDisplaysWithCompositingOnSecondDisplayOnly");
        testFramework.createTestCaseWithoutRenderer(TwoDisplaysWithCompositingOnBothDisplays, *this, "TwoDisplaysWithCompositingOnBothDisplays");
        testFramework.createTestCaseWithoutRenderer(SingleDisplayWithCompositing_SetOnRendererConfig, *this, "SingleDisplayWithCompositing_SetOnRendererConfig");
        testFramework.createTestCaseWithoutRenderer(TwoDisplaysWithCompositingOnFirstDisplayOnly_SetOnRendererConfig, *this, "TwoDisplaysWithCompositingOnFirstDisplayOnly_SetOnRendererConfig");
    }

    bool MultiDisplayStreamTextureTests::runEmbeddedCompositingTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase)
    {
        const ramses::RendererConfig rendererConfig = RendererTestUtils::CreateTestRendererConfig();

        ramses::DisplayConfig displayConfigWithEC = RendererTestUtils::CreateTestDisplayConfig(0, true);
        displayConfigWithEC.setWindowRectangle(0, 0, DisplayWidth, DisplayHeight);
        displayConfigWithEC.setWaylandEmbeddedCompositingSocketName(EmbeddedCompositingTestsFramework::TestEmbeddedCompositingDisplayName.c_str());
        displayConfigWithEC.setWaylandEmbeddedCompositingSocketGroup(testFramework.getEmbeddedCompositingSocketGroupName().c_str());

        ramses::DisplayConfig displayConfigWithoutEC = RendererTestUtils::CreateTestDisplayConfig(1, true);
        displayConfigWithoutEC.setWindowRectangle(DisplayWidth, 0, DisplayWidth, DisplayHeight);
        displayConfigWithoutEC.setWaylandEmbeddedCompositingSocketName("");
        displayConfigWithoutEC.setWaylandEmbeddedCompositingSocketGroup("");

        ramses::DisplayConfig displayConfigWithOtherEC = RendererTestUtils::CreateTestDisplayConfig(2, true);
        displayConfigWithOtherEC.setWindowRectangle(2 * DisplayWidth, 0, DisplayWidth, DisplayHeight);
        displayConfigWithOtherEC.setWaylandEmbeddedCompositingSocketName(EmbeddedCompositingTestsFramework::TestAlternateEmbeddedCompositingDisplayName.c_str());
        displayConfigWithOtherEC.setWaylandEmbeddedCompositingSocketGroup(testFramework.getEmbeddedCompositingSocketGroupName().c_str());

        //configs needed for EC tests with renderer config
        ramses::RendererConfig rendererConfigWithEC = RendererTestUtils::CreateTestRendererConfig();
        //hack to take/steal EC socket config set on cmd line args from display config
        rendererConfigWithEC.setWaylandEmbeddedCompositingSocketName(displayConfigWithEC.impl.getWaylandEmbeddedCompositingSocketName());
        rendererConfigWithEC.setWaylandEmbeddedCompositingSocketGroup(testFramework.getEmbeddedCompositingSocketGroupName().c_str());

        ramses::DisplayConfig otherDisplayConfigWithoutEC = RendererTestUtils::CreateTestDisplayConfig(3, true);
        otherDisplayConfigWithoutEC.setWindowRectangle(3 * DisplayWidth, 0, DisplayWidth, DisplayHeight);
        otherDisplayConfigWithoutEC.setWaylandEmbeddedCompositingSocketName("");
        otherDisplayConfigWithoutEC.setWaylandEmbeddedCompositingSocketGroup("");

        Bool testResultValue = true;

        const WaylandIviSurfaceId streamTextureSourceId(EmbeddedCompositorScene::GetStreamTextureSourceId());

        switch(testCase.m_id)
        {
        case TwoDisplaysWithCompositingOnFirstDisplayOnly:
        {
            testFramework.initializeRenderer(rendererConfig);
            testFramework.createDisplay(displayConfigWithEC);
            testFramework.createDisplay(displayConfigWithoutEC);

            createAndShowScene<EmbeddedCompositorScene>(testFramework, EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, DisplayWidth, DisplayHeight, 0u);
            createAndShowScene<EmbeddedCompositorScene>(testFramework, EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, DisplayWidth, DisplayHeight, 1u);

            //fallback on both displays
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1", 0u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1", 1u);

            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture"   , 0u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1"          , 1u);

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case TwoDisplaysWithCompositingOnSecondDisplayOnly:
        {
            testFramework.initializeRenderer(rendererConfig);
            testFramework.createDisplay(displayConfigWithoutEC);
            testFramework.createDisplay(displayConfigWithEC);

            createAndShowScene<EmbeddedCompositorScene>(testFramework, EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, DisplayWidth, DisplayHeight, 0u);
            createAndShowScene<EmbeddedCompositorScene>(testFramework, EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, DisplayWidth, DisplayHeight, 1u);

            //fallback on both displays
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1", 0u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1", 1u);

            testFramework.startTestApplicationAndWaitUntilConnected(EConnectionMode::DisplayName, 1u);
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId, 1u);

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1"          , 0u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture"   , 1u);

            testFramework.stopTestApplicationAndWaitUntilDisconnected(1u);
            break;
        }
        case TwoDisplaysWithCompositingOnBothDisplays:
        {
            testFramework.initializeRenderer(rendererConfig);
            testFramework.createDisplay(displayConfigWithEC);
            testFramework.createDisplay(displayConfigWithOtherEC);

            createAndShowScene<EmbeddedCompositorScene>(testFramework, EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, DisplayWidth, DisplayHeight, 0u);
            createAndShowScene<EmbeddedCompositorScene>(testFramework, EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, DisplayWidth, DisplayHeight, 1u);

            //fallback on both displays
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1", 0u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1", 1u);

            //start test app that gets connected to 1st display
            testFramework.startTestApplicationAndWaitUntilConnected(EConnectionMode::DisplayName, 0u, 0u);

            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1, 0u);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId, 0u);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId, false, 0u);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId, 0u);

            //composited texture only on 1st display
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture"   , 0u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1"          , 1u);

            //start 2nd test app that gets connected to 2nd display
            testFramework.startTestApplicationAndWaitUntilConnected(EConnectionMode::AlternateDisplayName, 1u, 1u);

            const TestApplicationSurfaceId surfaceId2 = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1, 1u);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId2, streamTextureSourceId, 1u);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId2, false, 1u);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId, 1u);

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

            break;
        }
        case SingleDisplayWithCompositing_SetOnRendererConfig:
        {
            testFramework.initializeRenderer(rendererConfigWithEC);
            // Use display config without EC. Nevertheless, EC gets created (because EC config is set on renderer config)
            testFramework.createDisplay(displayConfigWithoutEC);

            createAndShowScene<EmbeddedCompositorScene>(testFramework, EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, DisplayWidth, DisplayHeight, 0u);

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1", 0u);

            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture"   , 0u);

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case TwoDisplaysWithCompositingOnFirstDisplayOnly_SetOnRendererConfig:
        {
            testFramework.initializeRenderer(rendererConfigWithEC);
            // Use display config without EC. Nevertheless, EC gets created on 1st display ONLY (because EC config is set on renderer config)
            testFramework.createDisplay(displayConfigWithoutEC);
            testFramework.createDisplay(otherDisplayConfigWithoutEC);

            createAndShowScene<EmbeddedCompositorScene>(testFramework, EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, DisplayWidth, DisplayHeight, 0u);
            createAndShowScene<EmbeddedCompositorScene>(testFramework, EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, DisplayWidth, DisplayHeight, 1u);

            //fallback on both displays
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1", 0u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1", 1u);

            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture"   , 0u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1"          , 1u);

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        default:
            assert(false);
        }

        LOG_INFO(CONTEXT_RENDERER, "MultiDisplayStreamTextureTests::runEmbeddedCompositingTestCase waiting until client test application has terminated ...");
        testFramework.destroyDisplays();
        testFramework.destroyRenderer();

        return testResultValue;
    }
}
