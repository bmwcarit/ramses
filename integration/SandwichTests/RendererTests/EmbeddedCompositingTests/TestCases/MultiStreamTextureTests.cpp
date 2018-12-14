//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "MultiStreamTextureTests.h"
#include "TestScenes/EmbeddedCompositorScene.h"
#include "RendererLib/DisplayConfig.h"
#include "DisplayConfigImpl.h"

namespace ramses_internal
{
    MultiStreamTextureTests::MultiStreamTextureTests()
    {
    }

    void MultiStreamTextureTests::setUpEmbeddedCompositingTestCases(EmbeddedCompositingTestsFramework& testFramework)
    {
        const UInt32 displayWidth = ramses_internal::IntegrationScene::DefaultDisplayWidth * 2;
        const UInt32 displayHeight = ramses_internal::IntegrationScene::DefaultDisplayHeight;
        const float aspectRatio = static_cast<float>(displayWidth) / displayHeight;

        ramses::DisplayConfig displayConfig = RendererTestUtils::CreateTestDisplayConfig(0, true);
        displayConfig.setWindowRectangle(0, 0, displayWidth, displayHeight);
        displayConfig.setPerspectiveProjection(19.f, aspectRatio, 0.1f, 1500.f);

        testFramework.createTestCase(ShowDifferentFallbackTexturesOnStreamTexturesWithSameSourceId, *this, "ShowDifferentFallbackTexturesOnStreamTexturesWithSameSourceId").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(ShowStreamTextureOnTwoStreamTexturesWithSameSourceId, *this, "ShowStreamTextureOnTwoStreamTexturesWithSameSourceId").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(ShowSameFallbackTextureOnStreamTexturesWithDifferentSourceId, *this, "ShowSameFallbackTextureOnStreamTexturesWithDifferentSourceId").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(ShowTwoStreamTextures, *this, "ShowTwoStreamTextures").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(ShowSameBufferOnTwoStreamTextures, *this, "ShowSameBufferOnTwoStreamTextures").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(TestCorrectBufferRelease, *this, "TestCorrectBufferRelease").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(ClientRecreatesIVISurfaceWithDifferentId, *this, "ClientRecreatesIVISurfaceWithDifferentId").m_displayConfigs.push_back(displayConfig);
    }

    bool MultiStreamTextureTests::runEmbeddedCompositingTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase)
    {
        Bool testResultValue = true;

        const StreamTextureSourceId streamTextureSourceId(EmbeddedCompositorScene::GetStreamTextureSourceId());
        const StreamTextureSourceId secondStreamTextureSourceId(EmbeddedCompositorScene::GetSecondStreamTextureSourceId());
        const StreamTextureSourceId thirdStreamTextureSourceId(EmbeddedCompositorScene::GetThirdStreamTextureSourceId());

        switch(testCase.m_id)
        {
        case ShowDifferentFallbackTexturesOnStreamTexturesWithSameSourceId:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::TWO_STREAM_TEXTURES_WITH_SAME_SOURCE_ID_AND_DIFFERENT_FALLBACK_TEXTURES);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, secondStreamTextureSourceId);
            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(secondStreamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_Fallback2_Right");
            break;
        }
        case ShowStreamTextureOnTwoStreamTexturesWithSameSourceId:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::TWO_STREAM_TEXTURES_WITH_SAME_SOURCE_ID_AND_DIFFERENT_FALLBACK_TEXTURES);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_RedTriangle_Right");
            break;
        }
        case ShowSameFallbackTextureOnStreamTexturesWithDifferentSourceId:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::TWO_STREAM_TEXTURES_WITH_DIFFERENT_SOURCE_ID_AND_SAME_FALLBACK_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, thirdStreamTextureSourceId);
            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(thirdStreamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_Fallback1_Right");
            break;
        }
        case ShowTwoStreamTextures:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::TWO_STREAM_TEXTURES_WITH_DIFFERENT_SOURCE_ID_AND_SAME_FALLBACK_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId1 = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId1, streamTextureSourceId);
            TestApplicationSurfaceId surfaceId2 = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId2, secondStreamTextureSourceId);

            testFramework.sendRenderOneFrameToTestApplication(surfaceId1);
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendRenderOneFrameToTestApplication(surfaceId2);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testFramework.waitForContentOnStreamTexture(secondStreamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_BlueTriangle_Right");
            break;
        }
        case ShowSameBufferOnTwoStreamTextures:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::TWO_STREAM_TEXTURES_WITH_DIFFERENT_SOURCE_ID_AND_SAME_FALLBACK_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId1 = testFramework.sendCreateSharedMemorySurfaceToTestApplication(384, 384);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId1, streamTextureSourceId);
            TestApplicationSurfaceId surfaceId2 = testFramework.sendCreateSharedMemorySurfaceToTestApplication(384, 384);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId2, secondStreamTextureSourceId);

            testFramework.sendRenderOneFrameToTwoSurfacesAndWaitOnFrameCallbackToTestApplication(surfaceId1, surfaceId2);

            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testFramework.waitForContentOnStreamTexture(secondStreamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMem_RedTriangle_Left_RedTriangle_Right");
            break;
        }

        case TestCorrectBufferRelease:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::TWO_STREAM_TEXTURES_WITH_DIFFERENT_SOURCE_ID_AND_SAME_FALLBACK_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId1 = testFramework.sendCreateSharedMemorySurfaceToTestApplication(384, 384);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId1, streamTextureSourceId);
            TestApplicationSurfaceId surfaceId2 = testFramework.sendCreateSharedMemorySurfaceToTestApplication(384, 384);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId2, secondStreamTextureSourceId);

            // Current buffer assignment to the two surfaces and list of free buffers
            // Surface 1: <Buffer> | Surface 2: <Buffer> | Free: <Free Buffer>
            //             -       |             -       |        -

            // Render frame and attach to both surfaces:
            testFramework.sendRenderOneFrameToTwoSurfacesAndWaitOnFrameCallbackToTestApplication(surfaceId1, surfaceId2);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 1);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(secondStreamTextureSourceId, 1);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMem_RedTriangle_Left_RedTriangle_Right");
            //             0       |             0       |        -
            testResultValue &= checkFreeBufferState(testFramework, "0"); // Buffer 0 not free

            // Render frame to first surface:
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendRenderOneFrameAndWaitOnFrameCallbackToTestApplication(surfaceId1);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 2);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMem_BlueTriangle_Left_RedTriangle_Right");
            //             1       |             0       |        -
            testResultValue &= checkFreeBufferState(testFramework, "00"); // Buffer 0 and 1 not free

            // Render frame to second surface:
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::White);
            testFramework.sendRenderOneFrameAndWaitOnFrameCallbackToTestApplication(surfaceId2);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(secondStreamTextureSourceId, 2);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMem_BlueTriangle_Left_WhiteTriangle_Right");
            //             1       |             2       |        0
            testResultValue &= checkFreeBufferState(testFramework, "100"); // Buffer 0 free, buffer 1 and 2 not free

            // Render frame to first surface:
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Grey);
            testFramework.sendRenderOneFrameAndWaitOnFrameCallbackToTestApplication(surfaceId1);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 3);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMem_GreyTriangle_Left_WhiteTriangle_Right");
            //             0       |             2       |        1
            testResultValue &= checkFreeBufferState(testFramework, "010"); // Buffer 0 not free, buffer 1 free, and buffer 2 not free

            // Detach buffer from first surface:
            testFramework.sendDetachBufferFromSurfaceToTestApplication(surfaceId1);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 4);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_WhiteTriangle_Right");
            //             -       |             2       |        0, 1
            testResultValue &= checkFreeBufferState(testFramework, "110"); // Buffer 0 and 1 free, buffer 2 not free

            // Detach buffer from second surface:
            testFramework.sendDetachBufferFromSurfaceToTestApplication(surfaceId2);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(secondStreamTextureSourceId, 3);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_Fallback1_Right");
            //             -       |               -       |      0, 1, 2
            testResultValue &= checkFreeBufferState(testFramework, "111"); // Bufer 0, 1 and 2 free

            break;
        }

        case ClientRecreatesIVISurfaceWithDifferentId:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::TWO_STREAM_TEXTURES_WITH_DIFFERENT_SOURCE_ID_AND_SAME_FALLBACK_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendRenderOneFrameToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_Fallback1_Right");

            testFramework.sendDestroyIVISurfaceToTestApplication(surfaceId);
            testFramework.waitForUnavailablilityOfContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_Fallback1_Right");

            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, secondStreamTextureSourceId);
            testFramework.waitForContentOnStreamTexture(secondStreamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_RedTriangle_Right");

            break;
        }

        default:
            assert(false);
        }

        LOG_INFO(CONTEXT_RENDERER, "MultiStreamTextureTests::runEmbeddedCompositingTestCase waiting until client test application has terminated ...");
        testFramework.stopTestApplicationAndWaitUntilDisconnected();
        return testResultValue;
    }

    bool MultiStreamTextureTests::checkFreeBufferState(EmbeddedCompositingTestsFramework& testFramework, const String& expectedBufferFreeState)
    {
        String bufferFreeState;
        uint32_t numberOfAllocatedBuffers = testFramework.getNumberOfAllocatedSHMBufferFromTestApplication();
        for (uint32_t i = 0; i < numberOfAllocatedBuffers; i++)
        {
            if (testFramework.getIsBufferFreeFromTestApplication(i))
            {
                bufferFreeState += "1";
            }
            else
            {
                bufferFreeState += "0";
            }
        }
        if (bufferFreeState != expectedBufferFreeState)
        {
            LOG_ERROR(CONTEXT_RENDERER, "MultiStreamTextureTests::checkFreeBufferState Expected buffer free state: " << expectedBufferFreeState << " does not match actual buffer free state: " << bufferFreeState);
            return false;
        }
        return true;
    }
}
