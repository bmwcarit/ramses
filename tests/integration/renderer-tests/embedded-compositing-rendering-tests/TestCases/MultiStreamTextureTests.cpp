//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "MultiStreamTextureTests.h"
#include "TestScenes/EmbeddedCompositorScene.h"
#include "internal/RendererLib/DisplayConfigData.h"
#include "internal/Core/Utils/LogMacros.h"
#include "impl/DisplayConfigImpl.h"

namespace ramses::internal
{
    void MultiStreamTextureTests::setUpEmbeddedCompositingTestCases(EmbeddedCompositingTestsFramework& testFramework)
    {
        ramses::DisplayConfig displayConfig = RendererTestUtils::CreateTestDisplayConfig(0, true);
        displayConfig.setWindowRectangle(0, 0, DisplayWidth, DisplayHeight);
        displayConfig.setWaylandEmbeddedCompositingSocketName(EmbeddedCompositingTestsFramework::TestEmbeddedCompositingDisplayName.c_str());

        testFramework.createTestCase(ShowDifferentFallbackTexturesOnStreamTexturesWithSameSourceId, *this, "ShowDifferentFallbackTexturesOnStreamTexturesWithSameSourceId").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(ShowStreamTextureOnTwoStreamTexturesWithSameSourceId, *this, "ShowStreamTextureOnTwoStreamTexturesWithSameSourceId").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(ShowSameFallbackTextureOnStreamTexturesWithDifferentSourceId, *this, "ShowSameFallbackTextureOnStreamTexturesWithDifferentSourceId").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(ShowTwoStreamTextures, *this, "ShowTwoStreamTextures").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(ClientRecreatesIVISurfaceWithDifferentId, *this, "ClientRecreatesIVISurfaceWithDifferentId").m_displayConfigs.push_back(displayConfig);
    }

    bool MultiStreamTextureTests::runEmbeddedCompositingTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase)
    {
        bool testResultValue = true;

        constexpr WaylandIviSurfaceId waylandSurfaceIviId1{574u};
        constexpr WaylandIviSurfaceId waylandSurfaceIviId2{7889u};
        constexpr WaylandIviSurfaceId waylandSurfaceIviId3{4709u};

        const auto streamBuffer1 = testFramework.createStreamBuffer(0u, waylandIviSurfaceId_t{ waylandSurfaceIviId1.getValue()});
        const auto streamBuffer2 = testFramework.createStreamBuffer(0u, waylandIviSurfaceId_t{ waylandSurfaceIviId2.getValue()});
        const auto streamBuffer3 = testFramework.createStreamBuffer(0u, waylandIviSurfaceId_t{ waylandSurfaceIviId3.getValue()});

        switch(testCase.m_id)
        {
        case ShowDifferentFallbackTexturesOnStreamTexturesWithSameSourceId:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::TWO_STREAM_TEXTURES_WITH_SAME_SOURCE_ID_AND_DIFFERENT_FALLBACK_TEXTURES, DisplayWidth, DisplayHeight);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId2);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId2);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_Fallback2_Right");
            break;
        }
        case ShowStreamTextureOnTwoStreamTexturesWithSameSourceId:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::TWO_STREAM_TEXTURES_WITH_SAME_SOURCE_ID_AND_DIFFERENT_FALLBACK_TEXTURES, DisplayWidth, DisplayHeight);

            //same SB to different consumers
            testFramework.createBufferDataLink(streamBuffer1, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);
            testFramework.createBufferDataLink(streamBuffer1, sceneId, EmbeddedCompositorScene::SamplerConsumerId2);

            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_RedTriangle_Right");
            break;
        }
        case ShowSameFallbackTextureOnStreamTexturesWithDifferentSourceId:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::TWO_STREAM_TEXTURES_WITH_DIFFERENT_SOURCE_ID_AND_SAME_FALLBACK_TEXTURE, DisplayWidth, DisplayHeight);

            //different SBs to different consumers
            testFramework.createBufferDataLink(streamBuffer1, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);
            testFramework.createBufferDataLink(streamBuffer2, sceneId, EmbeddedCompositorScene::SamplerConsumerId2);

            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId3);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId3);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_Fallback1_Right");
            break;
        }
        case ShowTwoStreamTextures:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::TWO_STREAM_TEXTURES_WITH_DIFFERENT_SOURCE_ID_AND_SAME_FALLBACK_TEXTURE, DisplayWidth, DisplayHeight);

            //different SBs to different consumers
            testFramework.createBufferDataLink(streamBuffer1, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);
            testFramework.createBufferDataLink(streamBuffer2, sceneId, EmbeddedCompositorScene::SamplerConsumerId2);

            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId1 = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId1, waylandSurfaceIviId1);
            TestApplicationSurfaceId surfaceId2 = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId2, waylandSurfaceIviId2);

            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId1);
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId2);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId2);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_BlueTriangle_Right");
            break;
        }
        case ClientRecreatesIVISurfaceWithDifferentId:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::TWO_STREAM_TEXTURES_WITH_DIFFERENT_SOURCE_ID_AND_SAME_FALLBACK_TEXTURE, DisplayWidth, DisplayHeight);

            //different SBs to different consumers
            testFramework.createBufferDataLink(streamBuffer1, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);
            testFramework.createBufferDataLink(streamBuffer2, sceneId, EmbeddedCompositorScene::SamplerConsumerId2);

            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_Fallback1_Right");

            testFramework.sendDestroyIVISurfaceToTestApplication(surfaceId);
            testFramework.waitForUnavailablilityOfContentOnStreamTexture(waylandSurfaceIviId1);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_Fallback1_Right");

            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId2);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId2);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_RedTriangle_Right");

            break;
        }

        default:
            assert(false);
        }

        testFramework.destroyStreamBuffer(0u, streamBuffer1);
        testFramework.destroyStreamBuffer(0u, streamBuffer2);
        testFramework.destroyStreamBuffer(0u, streamBuffer3);

        LOG_INFO(CONTEXT_RENDERER, "MultiStreamTextureTests::runEmbeddedCompositingTestCase waiting until client test application has terminated ...");
        testFramework.stopTestApplicationAndWaitUntilDisconnected();
        return testResultValue;
    }
}
