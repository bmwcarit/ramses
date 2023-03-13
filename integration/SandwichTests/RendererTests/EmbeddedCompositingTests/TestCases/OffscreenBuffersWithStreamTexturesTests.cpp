//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "OffscreenBuffersWithStreamTexturesTests.h"
#include "TestScenes/EmbeddedCompositorScene.h"
#include "TestScenes/TextureLinkScene.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    void OffscreenBuffersWithStreamTexturesTests::setUpEmbeddedCompositingTestCases(EmbeddedCompositingTestsFramework& testFramework)
    {
        ramses::DisplayConfig displayConfig = RendererTestUtils::CreateTestDisplayConfig(0u, true);
        displayConfig.setWindowRectangle(0u, 0u, IntegrationScene::DefaultViewportWidth, IntegrationScene::DefaultViewportHeight);
        displayConfig.setWaylandEmbeddedCompositingSocketName(EmbeddedCompositingTestsFramework::TestEmbeddedCompositingDisplayName.c_str());

        testFramework.createTestCase(CanUseStreamTextureInASceneMappedToOffscreenBuffer, *this, "CanUseStreamTextureInASceneMappedToOffscreenBuffer").m_displayConfigs.push_back(displayConfig);
    }

    bool OffscreenBuffersWithStreamTexturesTests::runEmbeddedCompositingTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase)
    {
        Bool testResultValue = true;

        constexpr WaylandIviSurfaceId waylandSurfaceIviId1{409u};
        const auto streamBuffer = testFramework.createStreamBuffer(0u, ramses::waylandIviSurfaceId_t{ waylandSurfaceIviId1.getValue()});

        switch(testCase.m_id)
        {
        case CanUseStreamTextureInASceneMappedToOffscreenBuffer:
        {
            const auto sceneIdStreamConsumerOffscreenBufferProvider = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, 256u, 256u);
            const auto sceneIdOffscreenBufferConsumer = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_CONSUMER, IntegrationScene::DefaultViewportWidth, IntegrationScene::DefaultViewportHeight);

            testFramework.createBufferDataLink(streamBuffer, sceneIdStreamConsumerOffscreenBufferProvider, EmbeddedCompositorScene::SamplerConsumerId1);

            const auto offscreenBuffer = testFramework.createOffscreenBuffer(0, 256, 256, false);
            testFramework.assignSceneToDisplayBuffer(sceneIdStreamConsumerOffscreenBufferProvider, offscreenBuffer);
            testFramework.createBufferDataLink(offscreenBuffer, sceneIdOffscreenBufferConsumer, TextureLinkScene::DataConsumerId);

            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_CanUseStreamTextureInASceneMappedToOffscreenBuffer");
            break;
        }
        default:
            assert(false);
        }

        testFramework.destroyStreamBuffer(0u, streamBuffer);

        LOG_INFO(CONTEXT_RENDERER, "OffscreenBuffersWithStreamTexturesTests::runEmbeddedCompositingTestCase waiting until client test application has terminated ...");
        testFramework.stopTestApplicationAndWaitUntilDisconnected();
        return testResultValue;
    }
}
