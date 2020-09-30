//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "MultiSceneStreamTextureTests.h"
#include "TestScenes/EmbeddedCompositorScene.h"
#include "RendererLib/DisplayConfig.h"
#include "ETriangleColor.h"
#include "DisplayConfigImpl.h"

namespace ramses_internal
{
    MultiSceneStreamTextureTests::MultiSceneStreamTextureTests()
    {
    }

    void MultiSceneStreamTextureTests::setUpEmbeddedCompositingTestCases(EmbeddedCompositingTestsFramework& testFramework)
    {
        const UInt32 displayWidth = ramses_internal::IntegrationScene::DefaultDisplayWidth * 2;
        const UInt32 displayHeight = ramses_internal::IntegrationScene::DefaultDisplayHeight;
        const float aspectRatio = static_cast<float>(displayWidth) / displayHeight;

        ramses::DisplayConfig displayConfig = RendererTestUtils::CreateTestDisplayConfig(0, true);
        displayConfig.setWindowRectangle(0, 0, displayWidth, displayHeight);
        displayConfig.setPerspectiveProjection(19.f, aspectRatio, 0.1f, 1500.f);

        testFramework.createTestCase(CanUseTwoStreamTexturesWithSameSourceIdAndSameFallbackTextureFromTwoScenes, *this, "CanUseTwoStreamTexturesWithSameSourceIdAndSameFallbackTextureFromTwoScenes").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(CanUseTwoStreamTexturesWithSameSourceIdAndDifferentFallbackTextureFromTwoScenes, *this, "CanUseTwoStreamTexturesWithSameSourceIdAndDifferentFallbackTextureFromTwoScenes").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(CanUseTwoStreamTexturesWithDifferentSourceIdAndSameFallbackTextureFromTwoScenes, *this, "CanUseTwoStreamTexturesWithDifferentSourceIdAndSameFallbackTextureFromTwoScenes").m_displayConfigs.push_back(displayConfig);
    }

    bool MultiSceneStreamTextureTests::runEmbeddedCompositingTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase)
    {
        Bool testResultValue = true;

        const StreamTextureSourceId streamTextureSourceId(EmbeddedCompositorScene::GetStreamTextureSourceId());
        const StreamTextureSourceId secondStreamTextureSourceId(EmbeddedCompositorScene::GetSecondStreamTextureSourceId());

        testFramework.setEnvironmentVariableWaylandDisplay();

        switch(testCase.m_id)
        {
        case CanUseTwoStreamTexturesWithSameSourceIdAndSameFallbackTextureFromTwoScenes:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE_ON_THE_LEFT);
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE_ON_THE_RIGHT_WITH_FALLBACK_FROM_LEFT_SCENE);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_Fallback1_Right");

            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_RedTriangle_Right");
            break;
        }

        case CanUseTwoStreamTexturesWithSameSourceIdAndDifferentFallbackTextureFromTwoScenes:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE_ON_THE_LEFT);
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE_ON_THE_RIGHT);
            testFramework.startTestApplicationAndWaitUntilConnected();
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_Fallback2_Right");

            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_RedTriangle_Right");
            break;
        }
        case CanUseTwoStreamTexturesWithDifferentSourceIdAndSameFallbackTextureFromTwoScenes:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE_ON_THE_LEFT);
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE_ON_THE_RIGHT_WITH_SECOND_SOURCE_ID_AND_FALLBACK_FROM_LEFT_SCENE);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_Fallback1_Right");

            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId1 = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId1, streamTextureSourceId);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId1);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_Fallback1_Right");

            TestApplicationSurfaceId surfaceId2 = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId2, secondStreamTextureSourceId);
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId2);
            testFramework.waitForContentOnStreamTexture(secondStreamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_BlueTriangle_Right");
            break;
        }
        default:
            assert(false);
        }

        LOG_INFO(CONTEXT_RENDERER, "MultiSceneStreamTextureTests::runEmbeddedCompositingTestCase waiting until client test application has terminated ...");
        testFramework.stopTestApplicationAndWaitUntilDisconnected();
        return testResultValue;
    }
}
