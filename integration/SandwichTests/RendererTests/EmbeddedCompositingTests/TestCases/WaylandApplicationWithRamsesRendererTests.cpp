//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "WaylandApplicationWithRamsesRendererTests.h"
#include "TestScenes/EmbeddedCompositorScene.h"
#include "TestScenes/TextureLinkScene.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    void WaylandApplicationWithRamsesRendererTests::setUpEmbeddedCompositingTestCases(EmbeddedCompositingTestsFramework& testFramework)
    {
        ramses::DisplayConfig displayConfig = RendererTestUtils::CreateTestDisplayConfig(0u, true);
        displayConfig.setWindowRectangle(0u, 0u, IntegrationScene::DefaultViewportWidth, IntegrationScene::DefaultViewportHeight);
        displayConfig.setWaylandEmbeddedCompositingSocketName(EmbeddedCompositingTestsFramework::TestEmbeddedCompositingDisplayName.c_str());

        testFramework.createTestCase(CanRunRamsesRendererWithinExistingWaylandApplication, *this, "CanRunRamsesRendererWithinExistingWaylandApplication").m_displayConfigs.push_back(displayConfig);
    }

    bool WaylandApplicationWithRamsesRendererTests::runEmbeddedCompositingTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase)
    {
        bool testResultValue = true;

        switch(testCase.m_id)
        {
        case CanRunRamsesRendererWithinExistingWaylandApplication:
        {
            constexpr WaylandIviSurfaceId waylandSurfaceIviId1{157u};
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, IntegrationScene::DefaultViewportWidth, IntegrationScene::DefaultViewportHeight);
            const auto streamBuffer = testFramework.createStreamBuffer(0u, ramses::waylandIviSurfaceId_t{ waylandSurfaceIviId1.getValue()});
            testFramework.createBufferDataLink(streamBuffer, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);

            //start test application
            testFramework.startTestApplicationAndWaitUntilConnected();
            //create surface and render a frame normally (using OpenGL, without RAMSES renderer)
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture");

            //start RAMSES renderer inside test application and run a rendering test in it
            testResultValue &= testFramework.sendStartRamsesRendererAndRunRenderingTest();

            testFramework.destroyStreamBuffer(0u, streamBuffer);
            break;
        }
        default:
            assert(false);
        }

        LOG_INFO(CONTEXT_RENDERER, "WaylandApplicationWithRamsesRendererTests::runEmbeddedCompositingTestCase waiting until client test application has terminated ...");
        testFramework.stopTestApplicationAndWaitUntilDisconnected();
        return testResultValue;
    }
}
