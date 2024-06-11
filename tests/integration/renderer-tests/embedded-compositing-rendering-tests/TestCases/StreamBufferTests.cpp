//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "StreamBufferTests.h"
#include "TestScenes/EmbeddedCompositorScene.h"
#include "internal/RendererLib/DisplayConfigData.h"
#include "internal/Core/Utils/LogMacros.h"

namespace ramses::internal
{
    void StreamBufferTests::setUpEmbeddedCompositingTestCases(EmbeddedCompositingTestsFramework& testFramework)
    {
        ramses::DisplayConfig displayConfig = RendererTestUtils::CreateTestDisplayConfig(0, true);
        displayConfig.setWindowRectangle(0, 0, DisplayWidth, DisplayHeight);
        displayConfig.setWaylandEmbeddedCompositingSocketName(EmbeddedCompositingTestsFramework::TestEmbeddedCompositingDisplayName.c_str());

        testFramework.createTestCase(StreamBufferLinkedToSceneWithTwoSamplers, *this, "StreamBufferLinkedToSceneWithTwoSamplers").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(StreamBufferLinkedToTwoScenesWithSampler, *this, "StreamBufferLinkedToTwoScenesWithSampler").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(TwoStreamBuffersUsingSameSourceLinkedToSceneWithTwoSamplers, *this, "TwoStreamBuffersUsingSameSourceLinkedToSceneWithTwoSamplers").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(TwoStreamBuffersUsingDifferentSourceLinkedToSceneWithTwoSamplers, *this, "TwoStreamBuffersUsingDifferentSourceLinkedToSceneWithTwoSamplers").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(UnlinkStreamBuffer, *this, "UnlinkStreamBuffer").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(DestroyStreamBuffer, *this, "DestroyStreamBuffer").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(StreamBufferBecomesUnavailable, *this, "StreamBufferBecomesUnavailable").m_displayConfigs.push_back(displayConfig);
    }

    bool StreamBufferTests::runEmbeddedCompositingTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase)
    {
        bool testResultValue = true;

        constexpr WaylandIviSurfaceId streamBufferSourceId{ 486u };
        constexpr WaylandIviSurfaceId streamBufferSourceId2{ 487u };

        switch (testCase.m_id)
        {
        case StreamBufferLinkedToSceneWithTwoSamplers:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::TWO_STREAM_TEXTURES_WITH_SAME_SOURCE_ID_AND_DIFFERENT_FALLBACK_TEXTURES, DisplayWidth, DisplayHeight);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_Fallback2_Right");

            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamBufferSourceId);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamBufferSourceId);

            const auto streamBuffer = testFramework.createStreamBuffer(0u, waylandIviSurfaceId_t{ streamBufferSourceId.getValue() });
            testFramework.createBufferDataLink(streamBuffer, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);
            testFramework.createBufferDataLink(streamBuffer, sceneId, EmbeddedCompositorScene::SamplerConsumerId2);

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_RedTriangle_Right");
            break;
        }
        case StreamBufferLinkedToTwoScenesWithSampler:
        {
            const auto sceneId1 = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE_ON_THE_LEFT, DisplayWidth, DisplayHeight);
            const auto sceneId2 = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE_ON_THE_RIGHT_WITH_FALLBACK_FROM_LEFT_SCENE, DisplayWidth, DisplayHeight);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_Fallback1_Right");

            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamBufferSourceId);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamBufferSourceId);

            const auto streamBuffer = testFramework.createStreamBuffer(0u, waylandIviSurfaceId_t{ streamBufferSourceId.getValue() });
            testFramework.createBufferDataLink(streamBuffer, sceneId1, EmbeddedCompositorScene::SamplerConsumerId1);
            testFramework.createBufferDataLink(streamBuffer, sceneId2, EmbeddedCompositorScene::SamplerConsumerId1);

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_RedTriangle_Right");
            break;
        }
        case TwoStreamBuffersUsingSameSourceLinkedToSceneWithTwoSamplers:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::TWO_STREAM_TEXTURES_WITH_SAME_SOURCE_ID_AND_DIFFERENT_FALLBACK_TEXTURES, DisplayWidth, DisplayHeight);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_Fallback2_Right");

            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamBufferSourceId);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamBufferSourceId);

            const auto streamBuffer1 = testFramework.createStreamBuffer(0u, waylandIviSurfaceId_t{ streamBufferSourceId.getValue() });
            const auto streamBuffer2 = testFramework.createStreamBuffer(0u, waylandIviSurfaceId_t{ streamBufferSourceId.getValue() });
            testFramework.createBufferDataLink(streamBuffer1, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);
            testFramework.createBufferDataLink(streamBuffer2, sceneId, EmbeddedCompositorScene::SamplerConsumerId2);

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_RedTriangle_Right");
            break;
        }
        case TwoStreamBuffersUsingDifferentSourceLinkedToSceneWithTwoSamplers:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::TWO_STREAM_TEXTURES_WITH_SAME_SOURCE_ID_AND_DIFFERENT_FALLBACK_TEXTURES, DisplayWidth, DisplayHeight);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_Fallback2_Right");

            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId1 = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId1, streamBufferSourceId);
            const TestApplicationSurfaceId surfaceId2 = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId2, streamBufferSourceId2);

            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId1);
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId2);
            testFramework.waitForContentOnStreamTexture(streamBufferSourceId);
            testFramework.waitForContentOnStreamTexture(streamBufferSourceId2);

            const auto streamBuffer1 = testFramework.createStreamBuffer(0u, waylandIviSurfaceId_t{ streamBufferSourceId.getValue() });
            const auto streamBuffer2 = testFramework.createStreamBuffer(0u, waylandIviSurfaceId_t{ streamBufferSourceId2.getValue() });
            testFramework.createBufferDataLink(streamBuffer1, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);
            testFramework.createBufferDataLink(streamBuffer2, sceneId, EmbeddedCompositorScene::SamplerConsumerId2);

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_BlueTriangle_Right");
            break;
        }
        case UnlinkStreamBuffer:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::TWO_STREAM_TEXTURES_WITH_SAME_SOURCE_ID_AND_DIFFERENT_FALLBACK_TEXTURES, DisplayWidth, DisplayHeight);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_Fallback2_Right");

            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamBufferSourceId);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamBufferSourceId);

            const auto streamBuffer = testFramework.createStreamBuffer(0u, waylandIviSurfaceId_t{ streamBufferSourceId.getValue() });
            testFramework.createBufferDataLink(streamBuffer, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);
            testFramework.createBufferDataLink(streamBuffer, sceneId, EmbeddedCompositorScene::SamplerConsumerId2);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_RedTriangle_Right");

            testFramework.removeDataLink(sceneId, EmbeddedCompositorScene::SamplerConsumerId1);
            testFramework.removeDataLink(sceneId, EmbeddedCompositorScene::SamplerConsumerId2);

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_Fallback2_Right");
            break;
        }
        case DestroyStreamBuffer:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::TWO_STREAM_TEXTURES_WITH_SAME_SOURCE_ID_AND_DIFFERENT_FALLBACK_TEXTURES, DisplayWidth, DisplayHeight);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_Fallback2_Right");

            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamBufferSourceId);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamBufferSourceId);

            const auto streamBuffer = testFramework.createStreamBuffer(0u, waylandIviSurfaceId_t{ streamBufferSourceId.getValue() });
            testFramework.createBufferDataLink(streamBuffer, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);
            testFramework.createBufferDataLink(streamBuffer, sceneId, EmbeddedCompositorScene::SamplerConsumerId2);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_RedTriangle_Right");

            testFramework.destroyStreamBuffer(0u, streamBuffer);

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_Fallback2_Right");
            break;
        }
        case StreamBufferBecomesUnavailable:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::TWO_STREAM_TEXTURES_WITH_SAME_SOURCE_ID_AND_DIFFERENT_FALLBACK_TEXTURES, DisplayWidth, DisplayHeight);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_Fallback2_Right");

            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamBufferSourceId);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamBufferSourceId);

            const auto streamBuffer = testFramework.createStreamBuffer(0u, waylandIviSurfaceId_t{ streamBufferSourceId.getValue() });
            testFramework.createBufferDataLink(streamBuffer, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);
            testFramework.createBufferDataLink(streamBuffer, sceneId, EmbeddedCompositorScene::SamplerConsumerId2);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_RedTriangle_Right");

            testFramework.sendDestroyIVISurfaceToTestApplication(surfaceId);
            testFramework.waitForUnavailablilityOfContentOnStreamTexture(streamBufferSourceId);

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_Fallback2_Right");
            break;
        }
        default:
            assert(false);
        }

        LOG_INFO(CONTEXT_RENDERER, "StreamBufferTests::runEmbeddedCompositingTestCase waiting until client test application has terminated ...");
        testFramework.stopTestApplicationAndWaitUntilDisconnected();

        return testResultValue;
    }
}
