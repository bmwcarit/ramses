//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SharedMemoryBufferTests.h"
#include "TestScenes/EmbeddedCompositorScene.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    void SharedMemoryBufferTests::setUpEmbeddedCompositingTestCases(EmbeddedCompositingTestsFramework& testFramework)
    {
        testFramework.createTestCaseWithDefaultDisplay(ShowSharedMemoryStreamTexture, *this, "ShowSharedMemoryStreamTexture");
        testFramework.createTestCaseWithDefaultDisplay(ShowFallbackTextureWhenBufferIsDetachedFromSurface, *this, "ShowFallbackTextureWhenBufferIsDetachedFromSurface");
        testFramework.createTestCaseWithDefaultDisplay(ShowFallbackTextureWhenBufferIsDetachedFromSurfaceAndLastFrameNotUsedForRendering, *this, "ShowFallbackTextureWhenBufferIsDetachedFromSurfaceAndLastFrameNotUsedForRendering");
        testFramework.createTestCaseWithDefaultDisplay(ClientAttachesAndDestroysBufferWithoutCommit, *this, "ClientAttachesAndDestroysBufferWithoutCommit");
        testFramework.createTestCaseWithDefaultDisplay(SwitchBetweenBufferTypes_ShmThenEgl, *this, "SwitchBetweenBufferTypes_ShmThenEgl");
        testFramework.createTestCaseWithDefaultDisplay(SwitchBetweenBufferTypes_EglThenShmThenEgl, *this, "SwitchBetweenBufferTypes_EglThenShmThenEgl");
        testFramework.createTestCaseWithDefaultDisplay(SwitchBetweenBufferTypes_EglThenShmThenDestroyShmThenEgl, *this, "SwitchBetweenBufferTypes_EglThenShmThenDestroyShmThenEgl");
        testFramework.createTestCaseWithDefaultDisplay(SwitchBetweenBufferTypes_ShmAttachedWithoutCommitThenEgl, *this, "SwitchBetweenBufferTypes_ShmAttachedWithoutCommitThenEgl");
        testFramework.createTestCaseWithDefaultDisplay(SwitchBetweenBufferTypes_ShmAttachedWithoutCommitThenDestroyedThenEgl, *this, "SwitchBetweenBufferTypes_ShmAttachedWithoutCommitThenDestroyedThenEgl");
        testFramework.createTestCaseWithDefaultDisplay(SwitchBetweenBufferTypes_ShmThenDestroyShmThenEgl, *this, "SwitchBetweenBufferTypes_ShmThenDestroyShmThenEgl");
        testFramework.createTestCaseWithDefaultDisplay(SwitchBetweenBufferTypes_ConfidenceTest, *this, "SwitchBetweenBufferTypes_ConfidenceTest");

        const UInt32 displayWidthForTwoStreams = ramses_internal::IntegrationScene::DefaultDisplayWidth * 2;
        const UInt32 displayHeight = ramses_internal::IntegrationScene::DefaultDisplayHeight;
        const float aspectRatioForTwoStreams = static_cast<float>(displayWidthForTwoStreams) / displayHeight;
        ramses::DisplayConfig displayConfigForTwoStreams = RendererTestUtils::CreateTestDisplayConfig(0, true);
        displayConfigForTwoStreams.setWindowRectangle(0, 0, displayWidthForTwoStreams, displayHeight);
        displayConfigForTwoStreams.setPerspectiveProjection(19.f, aspectRatioForTwoStreams, 0.1f, 1500.f);

        testFramework.createTestCase(ShowSameBufferOnTwoStreamTextures, *this, "ShowSameBufferOnTwoStreamTextures").m_displayConfigs.push_back(displayConfigForTwoStreams);
        testFramework.createTestCase(TestCorrectBufferRelease, *this, "TestCorrectBufferRelease").m_displayConfigs.push_back(displayConfigForTwoStreams);
        testFramework.createTestCase(SwitchBetweenBufferTypes_ConfidenceTest_TwoStreams, *this, "SwitchBetweenBufferTypes_ConfidenceTest_TwoStreams").m_displayConfigs.push_back(displayConfigForTwoStreams);
        testFramework.createTestCase(SwitchBetweenBufferTypes_ConfidenceTest_SwizzledFallbackTextures, *this, "SwitchBetweenBufferTypes_ConfidenceTest_WithSwizzledTexture").m_displayConfigs.push_back(displayConfigForTwoStreams);
    }

    bool SharedMemoryBufferTests::runEmbeddedCompositingTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase)
    {
        Bool testResultValue = true;
        const StreamTextureSourceId streamTextureSourceId(EmbeddedCompositorScene::GetStreamTextureSourceId());
        const StreamTextureSourceId secondStreamTextureSourceId(EmbeddedCompositorScene::GetSecondStreamTextureSourceId());

        testFramework.setEnvironmentVariableWaylandDisplay();

        switch(testCase.m_id)
        {
        case ShowSharedMemoryStreamTexture:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithoutEGLContextToTestApplication(255, 255);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);

            if (testFramework.getNumberOfAllocatedSHMBufferFromTestApplication() != 0)
            {
                LOG_ERROR(CONTEXT_RENDERER, "SingleStreamTextureTests::runTest Number of allocated SHM buffers is not 0!");
                testResultValue = false;
            }

            const uint32_t cycles = 20;
            uint32_t frameCounter = 0;

            for (uint32_t i = 0; i < cycles; i++)
            {
                testResultValue &= renderAndCheckOneSharedMemoryFrame(testFramework, surfaceId, ETriangleColor::Red, streamTextureSourceId, frameCounter, "EC_ShMemRedTriangle");
                testResultValue &= renderAndCheckOneSharedMemoryFrame(testFramework, surfaceId, ETriangleColor::Blue, streamTextureSourceId, frameCounter, "EC_ShMemBlueTriangle");
                testResultValue &= renderAndCheckOneSharedMemoryFrame(testFramework, surfaceId, ETriangleColor::White, streamTextureSourceId, frameCounter, "EC_ShMemWhiteTriangle");
            }

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case ShowFallbackTextureWhenBufferIsDetachedFromSurface:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testFramework.renderOneFrame();
            testFramework.sendDetachBufferFromSurfaceToTestApplication(surfaceId);
            testFramework.waitForUnavailablilityOfContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1");
            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case ShowFallbackTextureWhenBufferIsDetachedFromSurfaceAndLastFrameNotUsedForRendering:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testFramework.sendDetachBufferFromSurfaceToTestApplication(surfaceId);
            testFramework.waitForUnavailablilityOfContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1");
            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case ClientAttachesAndDestroysBufferWithoutCommit:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithoutEGLContextToTestApplication(384, 384);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);
            testFramework.sendAttachBufferToTestApplication(surfaceId);
            testFramework.waitForBufferAttachedToIviSurface(streamTextureSourceId);
            testFramework.sendDestroyBuffersToTestApplication();
            testFramework.waitForNoBufferAttachedToIviSurface(streamTextureSourceId);
            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case SwitchBetweenBufferTypes_ShmThenEgl:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);

            //render a frame to SHM buffer
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 1u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemBlueTriangle");

            //render to EGL buffer
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 2u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_BlueTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case SwitchBetweenBufferTypes_EglThenShmThenEgl:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);

            //render a frame to EGL buffer
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture");

            //render a frame to SHM buffer
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 2u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemBlueTriangle");

            //render to EGL buffer again
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 3u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_BlueTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case SwitchBetweenBufferTypes_EglThenShmThenDestroyShmThenEgl:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);

            //render a frame to EGL buffer
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture");

            //render a frame to SHM buffer
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 2u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemBlueTriangle");

            //destroy SHM buffer
            testFramework.sendDestroyBuffersToTestApplication();
            testFramework.waitForNoBufferAttachedToIviSurface(streamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1");

            //render to EGL buffer again
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_BlueTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case SwitchBetweenBufferTypes_ShmAttachedWithoutCommitThenEgl:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1");

            //attach SHM buffer
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendAttachBufferToTestApplication(surfaceId);
            testFramework.waitForBufferAttachedToIviSurface(streamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1");

            //render to EGL buffer
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_BlueTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case SwitchBetweenBufferTypes_ShmAttachedWithoutCommitThenDestroyedThenEgl:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1");

            //attach SHM buffer
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendAttachBufferToTestApplication(surfaceId);
            testFramework.waitForBufferAttachedToIviSurface(streamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1");

            //destroy SHM buffer
            testFramework.sendDestroyBuffersToTestApplication();
            testFramework.waitForNoBufferAttachedToIviSurface(streamTextureSourceId);

            //render to EGL buffer
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_BlueTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case SwitchBetweenBufferTypes_ShmThenDestroyShmThenEgl:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);

            //render a frame to SHM buffer
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 1u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemBlueTriangle");

            //destroy SHM buffer
            testFramework.sendDestroyBuffersToTestApplication();
            testFramework.waitForNoBufferAttachedToIviSurface(streamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1");

            //render to EGL buffer
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_BlueTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case SwitchBetweenBufferTypes_ConfidenceTest:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, streamTextureSourceId);

            //render a frame to SHM buffer
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 1u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemBlueTriangle");

            //Detach SHM buffer
            testFramework.sendDetachBufferFromSurfaceToTestApplication(surfaceId);

            //render to EGL buffer
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 3u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_BlueTriangleStreamTexture");

            //attach SHM buffer and commit to make the attach take effect
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Red);
            testFramework.sendAttachBufferToTestApplication(surfaceId, true);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 4u);
            //blue triangle is rendered even though rendering color is set red, because only attach and commit
            //are executed by wayland client without actual change (rendering) into the shared memory buffer
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemBlueTriangle");

            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 5u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemRedTriangle");

            //render to SHM buffer
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::DarkGrey);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 6u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_GrayTriangleStreamTexture");

            //change color and render to EGL buffer
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Red);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 7u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case SwitchBetweenBufferTypes_ConfidenceTest_TwoStreams:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::TWO_STREAM_TEXTURES_WITH_DIFFERENT_SOURCE_ID_AND_SAME_FALLBACK_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId1 = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1u);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId1, streamTextureSourceId);
            TestApplicationSurfaceId surfaceId2 = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1u);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId2, secondStreamTextureSourceId);

            //initially fallback on both surfaces
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_Fallback1_Right");

            //SHM (blue) on left surface and fallback on right surface
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId1);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 1u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemBlueTriangle_Left_Fallback1_Right");

            //SHM (blue) on left surface and EGL (red) on right surface
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Red);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId2);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(secondStreamTextureSourceId, 1u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemBlueTriangle_Left_RedTriangle_Right");

            //update SHM (red) on left and keep EGL (red) on right (make sure update of shm does not lead to swizzle on EGL buf)
            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId1);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 2u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemRedTriangle_Left_RedTriangle_Right");

            //SHM (red) on left surface and SHM (red) on right surface
            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId2);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(secondStreamTextureSourceId, 2u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemRedTriangle_Left_ShMemRedTriangle_Right");

            //EGL (red) on left surface and SHM (red) on right surface
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId1);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 3u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_ShMemRedTriangle_Right");

            //EGL (red) on left surface and EGL (red) on right surface
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId2);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(secondStreamTextureSourceId, 3u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_RedTriangle_Right");

            //EGL (red) on left surface and fallback on right surface
            testFramework.sendDetachBufferFromSurfaceToTestApplication(surfaceId2);
            testFramework.waitForNoBufferAttachedToIviSurface(secondStreamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_Fallback1_Right");

            //SHM (blue) on left surface and fallback on right surface
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId1);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 4u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemBlueTriangle_Left_Fallback1_Right");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();

            //eventually fallback on both surfaces
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_Fallback1_Right");
            break;
        }
        case SwitchBetweenBufferTypes_ConfidenceTest_SwizzledFallbackTextures:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::TWO_STREAM_TEXTURES_WITH_DIFFERENT_SOURCE_ID_AND_SWIZZLED_FALLBACK_TEXTURES);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId1 = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1u);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId1, streamTextureSourceId);
            TestApplicationSurfaceId surfaceId2 = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1u);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId2, secondStreamTextureSourceId);

            //initially fallback on both surfaces
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_Fallback1_Swizzled_Right_Swizzled");

            //SHM (blue) on left surface and fallback on right surface
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId1);
            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemBlueTriangle_Left_Fallback1_Swizzled_Right");

            //SHM updated to red on left surface and fallback on right surface
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Red);
            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId1);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 2u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemRedTriangle_Left_Fallback1_Swizzled_Right");

            //SHM (red) on left surface and attach without commit on right surface, so fallback is still visible
            testFramework.sendAttachBufferToTestApplication(surfaceId2, false);
            testFramework.waitForBufferAttachedToIviSurface(secondStreamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemRedTriangle_Left_Fallback1_Swizzled_Right");

            //EGL (red) on left surface and no change on right surface, so fallback still visible
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId1);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 3u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_Fallback1_Swizzled_Right");

            //EGL (red) on left surface and EGL (red) on right surface
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId2);
            testFramework.waitForContentOnStreamTexture(secondStreamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_RedTriangle_Right");

            //Attach SHM without commit on left, so no change
            testFramework.sendAttachBufferToTestApplication(surfaceId1, false);
            testFramework.getNumberOfAllocatedSHMBufferFromTestApplication(); //make sure previous command was already executed
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_RedTriangle_Right");

            //Detach right surface, so EGL (red) on left surface and fallback on right surface
            testFramework.sendDetachBufferFromSurfaceToTestApplication(surfaceId2);
            testFramework.waitForNoBufferAttachedToIviSurface(secondStreamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_Fallback1_Swizzled_Right");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();

            //eventually fallback on both surfaces
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_Fallback1_Swizzled_Right_Swizzled");
            break;
        }
        case ShowSameBufferOnTwoStreamTextures:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::TWO_STREAM_TEXTURES_WITH_DIFFERENT_SOURCE_ID_AND_SAME_FALLBACK_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId1 = testFramework.sendCreateSurfaceWithoutEGLContextToTestApplication(384, 384);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId1, streamTextureSourceId);
            TestApplicationSurfaceId surfaceId2 = testFramework.sendCreateSurfaceWithoutEGLContextToTestApplication(384, 384);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId2, secondStreamTextureSourceId);

            testFramework.sendRenderOneFrameToTwoSurfacesAndWaitOnFrameCallbackToTestApplication(surfaceId1, surfaceId2);

            testFramework.waitForContentOnStreamTexture(streamTextureSourceId);
            testFramework.waitForContentOnStreamTexture(secondStreamTextureSourceId);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemRedTriangle_Left_ShMemRedTriangle_Right");
            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case TestCorrectBufferRelease:
        {
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::TWO_STREAM_TEXTURES_WITH_DIFFERENT_SOURCE_ID_AND_SAME_FALLBACK_TEXTURE);
            testFramework.startTestApplicationAndWaitUntilConnected();
            TestApplicationSurfaceId surfaceId1 = testFramework.sendCreateSurfaceWithoutEGLContextToTestApplication(384, 384);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId1, streamTextureSourceId);
            TestApplicationSurfaceId surfaceId2 = testFramework.sendCreateSurfaceWithoutEGLContextToTestApplication(384, 384);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId2, secondStreamTextureSourceId);

            // Current buffer assignment to the two surfaces and list of free buffers
            // Surface 1: <Buffer> | Surface 2: <Buffer> | Free: <Free Buffer>
            //             -       |             -       |        -

            // Render frame and attach to both surfaces:
            testFramework.sendRenderOneFrameToTwoSurfacesAndWaitOnFrameCallbackToTestApplication(surfaceId1, surfaceId2);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 1);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(secondStreamTextureSourceId, 1);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemRedTriangle_Left_ShMemRedTriangle_Right");
            //             0       |             0       |        -
            testResultValue &= checkFreeBufferState(testFramework, "0"); // Buffer 0 not free

            // Render frame to first surface:
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId1, true);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 2);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemBlueTriangle_Left_ShMemRedTriangle_Right");
            //             1       |             0       |        -
            testResultValue &= checkFreeBufferState(testFramework, "00"); // Buffer 0 and 1 not free

            // Render frame to second surface:
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::White);
            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId2, true);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(secondStreamTextureSourceId, 2);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemBlueTriangle_Left_ShMemWhiteTriangle_Right");
            //             1       |             2       |        0
            testResultValue &= checkFreeBufferState(testFramework, "100"); // Buffer 0 free, buffer 1 and 2 not free

            // Render frame to first surface:
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Grey);
            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId1, true);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, 3);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemGreyTriangle_Left_ShMemWhiteTriangle_Right");
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
            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        default:
            assert(false);
        }

        return testResultValue;
    }

    bool SharedMemoryBufferTests::renderAndCheckOneSharedMemoryFrame(EmbeddedCompositingTestsFramework& testFramework,  TestApplicationSurfaceId testSurfaceId, ETriangleColor color, StreamTextureSourceId streamTextureSourceId, UInt32& frameCount, const String& expectedImageName)
    {
        testFramework.sendSetTriangleColorToTestApplication(color);
        testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(testSurfaceId, true);
        testFramework.waitUntilNumberOfCommitedFramesForIviSurface(streamTextureSourceId, ++frameCount);
        if (!testFramework.renderAndCompareScreenshot(expectedImageName))
            return false;

        // Check, that for first frame one buffer is allocated, for second frame, two buffers are allocated, and that then
        // no more further buffers are needed. Checks if the Embedded Compositor releases the buffers.
        const UInt32 expectedNumberOfAllocatedSHMBuffer = std::min(frameCount, 2u);
        const UInt32 numberOfAllocatedSHMBuffer = testFramework.getNumberOfAllocatedSHMBufferFromTestApplication();

        if (numberOfAllocatedSHMBuffer != expectedNumberOfAllocatedSHMBuffer)
        {
            LOG_ERROR(CONTEXT_RENDERER, "SharedMemoryBufferTests::renderAndCheckOneSharedMemoryFrame Number of allocated SHM buffers " << numberOfAllocatedSHMBuffer << " does not match expected value " << expectedNumberOfAllocatedSHMBuffer <<"!");
            return false;
        }
        return true;
    }

    bool SharedMemoryBufferTests::checkFreeBufferState(EmbeddedCompositingTestsFramework& testFramework, const String& expectedBufferFreeState)
    {
        String bufferFreeState;
        uint32_t numberOfAllocatedBuffers = testFramework.getNumberOfAllocatedSHMBufferFromTestApplication();

        for (uint32_t i = 0; i < numberOfAllocatedBuffers; i++)
        {
            if (testFramework.getIsBufferFreeFromTestApplication(i))
                bufferFreeState += "1";
            else
                bufferFreeState += "0";
        }

        if (bufferFreeState != expectedBufferFreeState)
        {
            LOG_ERROR(CONTEXT_RENDERER, "SharedMemoryBufferTests::checkFreeBufferState Expected buffer free state: " << expectedBufferFreeState << " does not match actual buffer free state: " << bufferFreeState);
            return false;
        }
        return true;
    }
}
