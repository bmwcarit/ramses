//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "MultiSceneStreamTextureTests.h"
#include "TestScenes/EmbeddedCompositorScene.h"
#include "internal/RendererLib/DisplayConfigData.h"
#include "internal/Core/Utils/LogMacros.h"
#include "ETriangleColor.h"
#include "impl/DisplayConfigImpl.h"

namespace ramses::internal
{
    void MultiSceneStreamTextureTests::setUpEmbeddedCompositingTestCases(EmbeddedCompositingTestsFramework& testFramework)
    {
        ramses::DisplayConfig displayConfig = RendererTestUtils::CreateTestDisplayConfig(0, true);
        displayConfig.setWindowRectangle(0, 0, DisplayWidth, DisplayHeight);
        displayConfig.setWaylandEmbeddedCompositingSocketName(EmbeddedCompositingTestsFramework::TestEmbeddedCompositingDisplayName.c_str());

        testFramework.createTestCase(CanUseTwoStreamTexturesWithSameSourceIdAndSameFallbackTextureFromTwoScenes, *this, "CanUseTwoStreamTexturesWithSameSourceIdAndSameFallbackTextureFromTwoScenes").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(CanUseTwoStreamTexturesWithSameSourceIdAndDifferentFallbackTextureFromTwoScenes, *this, "CanUseTwoStreamTexturesWithSameSourceIdAndDifferentFallbackTextureFromTwoScenes").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(CanUseTwoStreamTexturesWithDifferentSourceIdAndSameFallbackTextureFromTwoScenes, *this, "CanUseTwoStreamTexturesWithDifferentSourceIdAndSameFallbackTextureFromTwoScenes").m_displayConfigs.push_back(displayConfig);
    }

    bool MultiSceneStreamTextureTests::runEmbeddedCompositingTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase)
    {
        bool testResultValue = true;

        constexpr WaylandIviSurfaceId waylandSurfaceIviId1{57u};
        constexpr WaylandIviSurfaceId waylandSurfaceIviId2{788u};

        const auto streamBuffer1 = testFramework.createStreamBuffer(0u, waylandIviSurfaceId_t{ waylandSurfaceIviId1.getValue()});
        const auto streamBuffer2 = testFramework.createStreamBuffer(0u, waylandIviSurfaceId_t{ waylandSurfaceIviId2.getValue()});

        switch(testCase.m_id)
        {
        case CanUseTwoStreamTexturesWithSameSourceIdAndSameFallbackTextureFromTwoScenes:
        {
            const auto sceneId1 = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE_ON_THE_LEFT, DisplayWidth, DisplayHeight);
            const auto sceneId2 = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE_ON_THE_RIGHT_WITH_FALLBACK_FROM_LEFT_SCENE, DisplayWidth, DisplayHeight);

            testFramework.createBufferDataLink(streamBuffer1, sceneId1, EmbeddedCompositorScene::SamplerConsumerId1);
            testFramework.createBufferDataLink(streamBuffer1, sceneId2, EmbeddedCompositorScene::SamplerConsumerId1);

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_Fallback1_Right");

            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_RedTriangle_Right");
            break;
        }

        case CanUseTwoStreamTexturesWithSameSourceIdAndDifferentFallbackTextureFromTwoScenes:
        {
            const auto sceneId1 = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE_ON_THE_LEFT, DisplayWidth, DisplayHeight);
            const auto sceneId2 = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE_ON_THE_RIGHT, DisplayWidth, DisplayHeight);

            testFramework.createBufferDataLink(streamBuffer1, sceneId1, EmbeddedCompositorScene::SamplerConsumerId1);
            testFramework.createBufferDataLink(streamBuffer1, sceneId2, EmbeddedCompositorScene::SamplerConsumerId2);

            testFramework.startTestApplicationAndWaitUntilConnected();
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_Fallback2_Right");

            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_RedTriangle_Right");
            break;
        }
        case CanUseTwoStreamTexturesWithDifferentSourceIdAndSameFallbackTextureFromTwoScenes:
        {
            const auto sceneId1 = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE_ON_THE_LEFT, DisplayWidth, DisplayHeight);
            const auto sceneId2 = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE_ON_THE_RIGHT_WITH_SECOND_SOURCE_ID_AND_FALLBACK_FROM_LEFT_SCENE, DisplayWidth, DisplayHeight);

            testFramework.createBufferDataLink(streamBuffer1, sceneId1, EmbeddedCompositorScene::SamplerConsumerId1);
            testFramework.createBufferDataLink(streamBuffer2, sceneId2, EmbeddedCompositorScene::SamplerConsumerId2);

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_Fallback1_Right");

            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId1 = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId1, waylandSurfaceIviId1);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId1);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_Fallback1_Right");

            TestApplicationSurfaceId surfaceId2 = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId2, waylandSurfaceIviId2);
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId2);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId2);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_BlueTriangle_Right");
            break;
        }
        default:
            assert(false);
        }

        testFramework.destroyStreamBuffer(0u, streamBuffer1);
        testFramework.destroyStreamBuffer(0u, streamBuffer2);

        LOG_INFO(CONTEXT_RENDERER, "MultiSceneStreamTextureTests::runEmbeddedCompositingTestCase waiting until client test application has terminated ...");
        testFramework.stopTestApplicationAndWaitUntilDisconnected();
        return testResultValue;
    }
}
