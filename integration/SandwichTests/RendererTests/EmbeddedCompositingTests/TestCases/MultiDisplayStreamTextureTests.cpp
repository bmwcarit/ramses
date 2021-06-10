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
        ramses::DisplayConfig displayConfig1 = RendererTestUtils::CreateTestDisplayConfig(0, true);
        displayConfig1.setWindowRectangle(0, 0, DisplayWidth, DisplayHeight);

        ramses::DisplayConfig displayConfig2 = RendererTestUtils::CreateTestDisplayConfig(1, true);
        displayConfig2.setWindowRectangle(DisplayWidth, 0, DisplayWidth, DisplayHeight);

        auto& testCaseTwoDisplaysWithCompositingOnFirstDisplayOnly = testFramework.createTestCase(TwoDisplaysWithCompositingOnFirstDisplayOnly, *this, "TwoDisplaysWithCompositingOnFirstDisplayOnly");
        testCaseTwoDisplaysWithCompositingOnFirstDisplayOnly.m_displayConfigs.push_back(displayConfig1);
        testCaseTwoDisplaysWithCompositingOnFirstDisplayOnly.m_displayConfigs.push_back(displayConfig2);
    }

    bool MultiDisplayStreamTextureTests::runEmbeddedCompositingTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase)
    {
        Bool testResultValue = true;

        const WaylandIviSurfaceId streamTextureSourceId(EmbeddedCompositorScene::GetStreamTextureSourceId());

        testFramework.setEnvironmentVariableWaylandDisplay();

        switch(testCase.m_id)
        {
        case TwoDisplaysWithCompositingOnFirstDisplayOnly:
        {
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
            break;
        }
        default:
            assert(false);
        }

        LOG_INFO(CONTEXT_RENDERER, "MultiDisplayStreamTextureTests::runEmbeddedCompositingTestCase waiting until client test application has terminated ...");
        testFramework.stopTestApplicationAndWaitUntilDisconnected();
        return testResultValue;
    }
}
