//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/renderer/IRendererEventHandler.h"
#include "ramses/renderer/IRendererSceneControlEventHandler.h"
#include "SharedMemoryBufferTests.h"
#include "TestScenes/EmbeddedCompositorScene.h"
#include "internal/Core/Utils/LogMacros.h"

#include <string>
#include <unordered_set>

namespace ramses::internal
{
    class StreamAvailabilityHandler : public ramses::RendererEventHandlerEmpty, public ramses::RendererSceneControlEventHandlerEmpty
    {
    public:
        void streamAvailabilityChanged(ramses::waylandIviSurfaceId_t streamId, bool available) override
        {
            ++callbacks;
            if (available)
            {
                LOG_INFO_P(CONTEXT_RENDERER, "stream available: {}", streamId.getValue());
                availableStreams.insert(streamId);
            }
            else
            {
                LOG_INFO_P(CONTEXT_RENDERER, "stream unavailable: {}", streamId.getValue());
                availableStreams.erase(streamId);
            }
        }

        std::unordered_set<ramses::waylandIviSurfaceId_t> availableStreams;
        int callbacks = 0;
    };

    void SharedMemoryBufferTests::setUpEmbeddedCompositingTestCases(EmbeddedCompositingTestsFramework& testFramework)
    {
        ramses::DisplayConfig displayConfig = RendererTestUtils::CreateTestDisplayConfig(0u, true);
        displayConfig.setWindowRectangle(0u, 0u, DisplayWidth, DisplayHeight);
        displayConfig.setWaylandEmbeddedCompositingSocketName(EmbeddedCompositingTestsFramework::TestEmbeddedCompositingDisplayName.c_str());

        testFramework.createTestCase(ShowSharedMemoryStreamTexture, *this, "ShowSharedMemoryStreamTexture").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(ShowFallbackTextureWhenBufferIsDetachedFromSurface, *this, "ShowFallbackTextureWhenBufferIsDetachedFromSurface").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(ShowFallbackTextureWhenBufferIsDetachedFromSurfaceAndLastFrameNotUsedForRendering, *this, "ShowFallbackTextureWhenBufferIsDetachedFromSurfaceAndLastFrameNotUsedForRendering").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(ClientAttachesAndDestroysBufferWithoutCommit, *this, "ClientAttachesAndDestroysBufferWithoutCommit").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(QuickBufferReAttach, *this, "QuickBufferReAttach").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(SwitchBetweenBufferTypes_ShmThenEgl, *this, "SwitchBetweenBufferTypes_ShmThenEgl").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(SwitchBetweenBufferTypes_EglThenShmThenEgl, *this, "SwitchBetweenBufferTypes_EglThenShmThenEgl").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(SwitchBetweenBufferTypes_EglThenShmThenDestroyShmThenEgl, *this, "SwitchBetweenBufferTypes_EglThenShmThenDestroyShmThenEgl").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(SwitchBetweenBufferTypes_ShmAttachedWithoutCommitThenEgl, *this, "SwitchBetweenBufferTypes_ShmAttachedWithoutCommitThenEgl").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(SwitchBetweenBufferTypes_ShmAttachedWithoutCommitThenDestroyedThenEgl, *this, "SwitchBetweenBufferTypes_ShmAttachedWithoutCommitThenDestroyedThenEgl").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(SwitchBetweenBufferTypes_ShmThenDestroyShmThenEgl, *this, "SwitchBetweenBufferTypes_ShmThenDestroyShmThenEgl").m_displayConfigs.push_back(displayConfig);
        testFramework.createTestCase(SwitchBetweenBufferTypes_ConfidenceTest, *this, "SwitchBetweenBufferTypes_ConfidenceTest").m_displayConfigs.push_back(displayConfig);

        ramses::DisplayConfig displayConfigForTwoStreams = RendererTestUtils::CreateTestDisplayConfig(0, true);
        displayConfigForTwoStreams.setWindowRectangle(0, 0, DisplayWidthTwoStreams, DisplayHeight);
        displayConfigForTwoStreams.setWaylandEmbeddedCompositingSocketName(EmbeddedCompositingTestsFramework::TestEmbeddedCompositingDisplayName.c_str());

        testFramework.createTestCase(ShowSameBufferOnTwoStreamTextures, *this, "ShowSameBufferOnTwoStreamTextures").m_displayConfigs.push_back(displayConfigForTwoStreams);
        testFramework.createTestCase(TestCorrectBufferRelease, *this, "TestCorrectBufferRelease").m_displayConfigs.push_back(displayConfigForTwoStreams);
        testFramework.createTestCase(SwitchBetweenBufferTypes_ConfidenceTest_TwoStreams, *this, "SwitchBetweenBufferTypes_ConfidenceTest_TwoStreams").m_displayConfigs.push_back(displayConfigForTwoStreams);
        testFramework.createTestCase(SwitchBetweenBufferTypes_ConfidenceTest_SwizzledFallbackTextures, *this, "SwitchBetweenBufferTypes_ConfidenceTest_WithSwizzledTexture").m_displayConfigs.push_back(displayConfigForTwoStreams);
    }

    bool SharedMemoryBufferTests::runEmbeddedCompositingTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase)
    {
        bool testResultValue = true;

        constexpr WaylandIviSurfaceId waylandSurfaceIviId1{517u};
        constexpr WaylandIviSurfaceId waylandSurfaceIviId2{718u};

        const auto streamBuffer1 = testFramework.createStreamBuffer(0u, waylandIviSurfaceId_t{ waylandSurfaceIviId1.getValue()});
        const auto streamBuffer2 = testFramework.createStreamBuffer(0u, waylandIviSurfaceId_t{ waylandSurfaceIviId2.getValue()});

        switch(testCase.m_id)
        {
        case ShowSharedMemoryStreamTexture:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, DisplayWidth, DisplayHeight);
            testFramework.createBufferDataLink(streamBuffer1, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);

            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithoutEGLContextToTestApplication(255, 255);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);

            if (testFramework.getNumberOfAllocatedSHMBufferFromTestApplication() != 0)
            {
                LOG_ERROR(CONTEXT_RENDERER, "SingleStreamTextureTests::runTest Number of allocated SHM buffers is not 0!");
                testResultValue = false;
            }

            const uint32_t cycles = 20;
            uint32_t frameCounter = 0;

            for (uint32_t i = 0; i < cycles; i++)
            {
                testResultValue &= RenderAndCheckOneSharedMemoryFrame(testFramework, surfaceId, ETriangleColor::Red, waylandSurfaceIviId1, frameCounter, "EC_ShMemRedTriangle");
                testResultValue &= RenderAndCheckOneSharedMemoryFrame(testFramework, surfaceId, ETriangleColor::Blue, waylandSurfaceIviId1, frameCounter, "EC_ShMemBlueTriangle");
                testResultValue &= RenderAndCheckOneSharedMemoryFrame(testFramework, surfaceId, ETriangleColor::White, waylandSurfaceIviId1, frameCounter, "EC_ShMemWhiteTriangle");
            }

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case QuickBufferReAttach:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, DisplayWidth, DisplayHeight);
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithoutEGLContextToTestApplication(255, 255);
            StreamAvailabilityHandler handler;
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);
            testFramework.renderOneFrame();
            testFramework.dispatchRendererEvents(handler, handler);
            if (handler.availableStreams.empty())
            {
                LOG_ERROR_P(CONTEXT_RENDERER, "Test surface {} is unavailable", surfaceId.getValue());
                testResultValue = false;
            }
            else
            {
                const auto iviSurface = *handler.availableStreams.begin();
                const auto callbacks = handler.callbacks;
                testFramework.sendReAttachBufferToTestApplication(surfaceId, 1);
                testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId1, 3);
                testFramework.renderOneFrame();
                testFramework.dispatchRendererEvents(handler, handler);
                if (handler.availableStreams.count(iviSurface) != 1 || handler.callbacks == callbacks)
                {
                    LOG_ERROR_P(CONTEXT_RENDERER, "Stream surface {} is unavailable after reattach", surfaceId.getValue());
                    testResultValue = false;
                }
            }
            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case ShowFallbackTextureWhenBufferIsDetachedFromSurface:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, DisplayWidth, DisplayHeight);
            testFramework.createBufferDataLink(streamBuffer1, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);

            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);
            testFramework.renderOneFrame();
            testFramework.sendDetachBufferFromSurfaceToTestApplication(surfaceId);
            testFramework.waitForUnavailablilityOfContentOnStreamTexture(waylandSurfaceIviId1);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1");
            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case ShowFallbackTextureWhenBufferIsDetachedFromSurfaceAndLastFrameNotUsedForRendering:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, DisplayWidth, DisplayHeight);
            testFramework.createBufferDataLink(streamBuffer1, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);

            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);
            testFramework.sendDetachBufferFromSurfaceToTestApplication(surfaceId);
            testFramework.waitForUnavailablilityOfContentOnStreamTexture(waylandSurfaceIviId1);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1");
            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case ClientAttachesAndDestroysBufferWithoutCommit:
        {
            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithoutEGLContextToTestApplication(384, 384);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);
            testFramework.sendAttachBufferToTestApplication(surfaceId);
            testFramework.waitForBufferAttachedToIviSurface(waylandSurfaceIviId1);
            testFramework.sendDestroyBuffersToTestApplication();
            testFramework.waitForNoBufferAttachedToIviSurface(waylandSurfaceIviId1);
            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case SwitchBetweenBufferTypes_ShmThenEgl:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, DisplayWidth, DisplayHeight);
            testFramework.createBufferDataLink(streamBuffer1, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);

            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);

            //render a frame to SHM buffer
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId1, 1u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemBlueTriangle");

            //render to EGL buffer
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId1, 2u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_BlueTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case SwitchBetweenBufferTypes_EglThenShmThenEgl:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, DisplayWidth, DisplayHeight);
            testFramework.createBufferDataLink(streamBuffer1, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);

            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);

            //render a frame to EGL buffer
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture");

            //render a frame to SHM buffer
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId1, 2u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemBlueTriangle");

            //render to EGL buffer again
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId1, 3u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_BlueTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case SwitchBetweenBufferTypes_EglThenShmThenDestroyShmThenEgl:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, DisplayWidth, DisplayHeight);
            testFramework.createBufferDataLink(streamBuffer1, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);

            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);

            //render a frame to EGL buffer
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture");

            //render a frame to SHM buffer
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId1, 2u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemBlueTriangle");

            //destroy SHM buffer
            testFramework.sendDestroyBuffersToTestApplication();
            testFramework.waitForNoBufferAttachedToIviSurface(waylandSurfaceIviId1);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1");

            //render to EGL buffer again
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_BlueTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case SwitchBetweenBufferTypes_ShmAttachedWithoutCommitThenEgl:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, DisplayWidth, DisplayHeight);
            testFramework.createBufferDataLink(streamBuffer1, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);

            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1");

            //attach SHM buffer
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendAttachBufferToTestApplication(surfaceId);
            testFramework.waitForBufferAttachedToIviSurface(waylandSurfaceIviId1);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1");

            //render to EGL buffer
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_BlueTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case SwitchBetweenBufferTypes_ShmAttachedWithoutCommitThenDestroyedThenEgl:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, DisplayWidth, DisplayHeight);
            testFramework.createBufferDataLink(streamBuffer1, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);

            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);

            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1");

            //attach SHM buffer
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendAttachBufferToTestApplication(surfaceId);
            testFramework.waitForBufferAttachedToIviSurface(waylandSurfaceIviId1);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1");

            //destroy SHM buffer
            testFramework.sendDestroyBuffersToTestApplication();
            testFramework.waitForNoBufferAttachedToIviSurface(waylandSurfaceIviId1);

            //render to EGL buffer
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_BlueTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case SwitchBetweenBufferTypes_ShmThenDestroyShmThenEgl:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, DisplayWidth, DisplayHeight);
            testFramework.createBufferDataLink(streamBuffer1, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);

            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);

            //render a frame to SHM buffer
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId1, 1u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemBlueTriangle");

            //destroy SHM buffer
            testFramework.sendDestroyBuffersToTestApplication();
            testFramework.waitForNoBufferAttachedToIviSurface(waylandSurfaceIviId1);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_FallbackTexture_1");

            //render to EGL buffer
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_BlueTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case SwitchBetweenBufferTypes_ConfidenceTest:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::SINGLE_STREAM_TEXTURE, DisplayWidth, DisplayHeight);
            testFramework.createBufferDataLink(streamBuffer1, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);

            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId, waylandSurfaceIviId1);

            //render a frame to SHM buffer
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId1, 1u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemBlueTriangle");

            //Detach SHM buffer
            testFramework.sendDetachBufferFromSurfaceToTestApplication(surfaceId);

            //render to EGL buffer
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId1, 3u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_BlueTriangleStreamTexture");

            //attach SHM buffer and commit to make the attach take effect
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Red);
            testFramework.sendAttachBufferToTestApplication(surfaceId, true);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId1, 4u);
            //blue triangle is rendered even though rendering color is set red, because only attach and commit
            //are executed by wayland client without actual change (rendering) into the shared memory buffer
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemBlueTriangle");

            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId1, 5u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemRedTriangle");

            //render to SHM buffer
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::DarkGrey);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId1, 6u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_GrayTriangleStreamTexture");

            //change color and render to EGL buffer
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Red);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId1, 7u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangleStreamTexture");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case SwitchBetweenBufferTypes_ConfidenceTest_TwoStreams:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::TWO_STREAM_TEXTURES_WITH_DIFFERENT_SOURCE_ID_AND_SAME_FALLBACK_TEXTURE, DisplayWidthTwoStreams, DisplayHeight);
            testFramework.createBufferDataLink(streamBuffer1, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);
            testFramework.createBufferDataLink(streamBuffer2, sceneId, EmbeddedCompositorScene::SamplerConsumerId2);

            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId1 = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1u);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId1, waylandSurfaceIviId1);
            const TestApplicationSurfaceId surfaceId2 = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1u);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId2, waylandSurfaceIviId2);

            //initially fallback on both surfaces
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_Fallback1_Right");

            //SHM (blue) on left surface and fallback on right surface
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId1);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId1, 1u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemBlueTriangle_Left_Fallback1_Right");

            //SHM (blue) on left surface and EGL (red) on right surface
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Red);
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId2);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId2, 1u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemBlueTriangle_Left_RedTriangle_Right");

            //update SHM (red) on left and keep EGL (red) on right (make sure update of shm does not lead to swizzle on EGL buf)
            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId1);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId1, 2u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemRedTriangle_Left_RedTriangle_Right");

            //SHM (red) on left surface and SHM (red) on right surface
            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId2);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId2, 2u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemRedTriangle_Left_ShMemRedTriangle_Right");

            //EGL (red) on left surface and SHM (red) on right surface
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId1);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId1, 3u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_ShMemRedTriangle_Right");

            //EGL (red) on left surface and EGL (red) on right surface
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId2);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId2, 3u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_RedTriangle_Right");

            //EGL (red) on left surface and fallback on right surface
            testFramework.sendDetachBufferFromSurfaceToTestApplication(surfaceId2);
            testFramework.waitForNoBufferAttachedToIviSurface(waylandSurfaceIviId2);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_Fallback1_Right");

            //SHM (blue) on left surface and fallback on right surface
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId1);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId1, 4u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemBlueTriangle_Left_Fallback1_Right");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();

            //eventually fallback on both surfaces
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_Fallback1_Right");
            break;
        }
        case SwitchBetweenBufferTypes_ConfidenceTest_SwizzledFallbackTextures:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::TWO_STREAM_TEXTURES_WITH_DIFFERENT_SOURCE_ID_AND_SWIZZLED_FALLBACK_TEXTURES, DisplayWidthTwoStreams, DisplayHeight);
            testFramework.createBufferDataLink(streamBuffer1, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);
            testFramework.createBufferDataLink(streamBuffer2, sceneId, EmbeddedCompositorScene::SamplerConsumerId2);

            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId1 = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1u);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId1, waylandSurfaceIviId1);
            const TestApplicationSurfaceId surfaceId2 = testFramework.sendCreateSurfaceWithEGLContextToTestApplication(384, 384, 1u);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId2, waylandSurfaceIviId2);

            //initially fallback on both surfaces
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_Fallback1_Swizzled_Right_Swizzled");

            //SHM (blue) on left surface and fallback on right surface
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId1);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemBlueTriangle_Left_Fallback1_Swizzled_Right");

            //SHM updated to red on left surface and fallback on right surface
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Red);
            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId1);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId1, 2u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemRedTriangle_Left_Fallback1_Swizzled_Right");

            //SHM (red) on left surface and attach without commit on right surface, so fallback is still visible
            testFramework.sendAttachBufferToTestApplication(surfaceId2, false);
            testFramework.waitForBufferAttachedToIviSurface(waylandSurfaceIviId2);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemRedTriangle_Left_Fallback1_Swizzled_Right");

            //EGL (red) on left surface and no change on right surface, so fallback still visible
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId1);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId1, 3u);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_Fallback1_Swizzled_Right");

            //EGL (red) on left surface and EGL (red) on right surface
            testFramework.sendRenderOneFrameToEGLBufferToTestApplication(surfaceId2);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId2);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_RedTriangle_Right");

            //Attach SHM without commit on left, so no change
            testFramework.sendAttachBufferToTestApplication(surfaceId1, false);
            testFramework.getNumberOfAllocatedSHMBufferFromTestApplication(); //make sure previous command was already executed
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_RedTriangle_Right");

            //Detach right surface, so EGL (red) on left surface and fallback on right surface
            testFramework.sendDetachBufferFromSurfaceToTestApplication(surfaceId2);
            testFramework.waitForNoBufferAttachedToIviSurface(waylandSurfaceIviId2);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_RedTriangle_Left_Fallback1_Swizzled_Right");

            testFramework.stopTestApplicationAndWaitUntilDisconnected();

            //eventually fallback on both surfaces
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_Fallback1_Swizzled_Right_Swizzled");
            break;
        }
        case ShowSameBufferOnTwoStreamTextures:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::TWO_STREAM_TEXTURES_WITH_DIFFERENT_SOURCE_ID_AND_SAME_FALLBACK_TEXTURE, DisplayWidthTwoStreams, DisplayHeight);
            testFramework.createBufferDataLink(streamBuffer1, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);
            testFramework.createBufferDataLink(streamBuffer1, sceneId, EmbeddedCompositorScene::SamplerConsumerId2);

            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId1 = testFramework.sendCreateSurfaceWithoutEGLContextToTestApplication(384, 384);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId1, waylandSurfaceIviId1);
            const TestApplicationSurfaceId surfaceId2 = testFramework.sendCreateSurfaceWithoutEGLContextToTestApplication(384, 384);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId2, waylandSurfaceIviId2);

            testFramework.sendRenderOneFrameToTwoSurfacesAndWaitOnFrameCallbackToTestApplication(surfaceId1, surfaceId2);

            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId1);
            testFramework.waitForContentOnStreamTexture(waylandSurfaceIviId2);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemRedTriangle_Left_ShMemRedTriangle_Right");
            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        case TestCorrectBufferRelease:
        {
            const auto sceneId = testFramework.createAndShowScene<EmbeddedCompositorScene>(EmbeddedCompositorScene::TWO_STREAM_TEXTURES_WITH_DIFFERENT_SOURCE_ID_AND_SAME_FALLBACK_TEXTURE, DisplayWidthTwoStreams, DisplayHeight);
            testFramework.createBufferDataLink(streamBuffer1, sceneId, EmbeddedCompositorScene::SamplerConsumerId1);
            testFramework.createBufferDataLink(streamBuffer2, sceneId, EmbeddedCompositorScene::SamplerConsumerId2);

            testFramework.startTestApplicationAndWaitUntilConnected();
            const TestApplicationSurfaceId surfaceId1 = testFramework.sendCreateSurfaceWithoutEGLContextToTestApplication(384, 384);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId1, waylandSurfaceIviId1);
            const TestApplicationSurfaceId surfaceId2 = testFramework.sendCreateSurfaceWithoutEGLContextToTestApplication(384, 384);
            testFramework.sendCreateIVISurfaceToTestApplication(surfaceId2, waylandSurfaceIviId2);

            // Current buffer assignment to the two surfaces and list of free buffers
            // Surface 1: <Buffer> | Surface 2: <Buffer> | Free: <Free Buffer>
            //             -       |             -       |        -

            // Render frame and attach to both surfaces:
            testFramework.sendRenderOneFrameToTwoSurfacesAndWaitOnFrameCallbackToTestApplication(surfaceId1, surfaceId2);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId1, 1);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId2, 1);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemRedTriangle_Left_ShMemRedTriangle_Right");
            //             0       |             0       |        -
            testResultValue &= CheckFreeBufferState(testFramework, "0"); // Buffer 0 not free

            // Render frame to first surface:
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Blue);
            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId1, true);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId1, 2);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemBlueTriangle_Left_ShMemRedTriangle_Right");
            //             1       |             0       |        -
            testResultValue &= CheckFreeBufferState(testFramework, "00"); // Buffer 0 and 1 not free

            // Render frame to second surface:
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::White);
            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId2, true);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId2, 2);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemBlueTriangle_Left_ShMemWhiteTriangle_Right");
            //             1       |             2       |        0
            testResultValue &= CheckFreeBufferState(testFramework, "100"); // Buffer 0 free, buffer 1 and 2 not free

            // Render frame to first surface:
            testFramework.sendSetTriangleColorToTestApplication(ETriangleColor::Grey);
            testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(surfaceId1, true);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId1, 3);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_ShMemGreyTriangle_Left_ShMemWhiteTriangle_Right");
            //             0       |             2       |        1
            testResultValue &= CheckFreeBufferState(testFramework, "010"); // Buffer 0 not free, buffer 1 free, and buffer 2 not free

            // Detach buffer from first surface:
            testFramework.sendDetachBufferFromSurfaceToTestApplication(surfaceId1);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId1, 4);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_WhiteTriangle_Right");
            //             -       |             2       |        0, 1
            testResultValue &= CheckFreeBufferState(testFramework, "110"); // Buffer 0 and 1 free, buffer 2 not free

            // Detach buffer from second surface:
            testFramework.sendDetachBufferFromSurfaceToTestApplication(surfaceId2);
            testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId2, 3);
            testResultValue &= testFramework.renderAndCompareScreenshot("EC_Fallback1_Left_Fallback1_Right");
            //             -       |               -       |      0, 1, 2
            testResultValue &= CheckFreeBufferState(testFramework, "111"); // Bufer 0, 1 and 2 free
            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }
        default:
            assert(false);
        }

        testFramework.destroyStreamBuffer(0u, streamBuffer1);
        testFramework.destroyStreamBuffer(0u, streamBuffer2);

        return testResultValue;
    }

    bool SharedMemoryBufferTests::RenderAndCheckOneSharedMemoryFrame(EmbeddedCompositingTestsFramework& testFramework,  TestApplicationSurfaceId testSurfaceId, ETriangleColor color, WaylandIviSurfaceId waylandSurfaceIviId, uint32_t& frameCount, const std::string& expectedImageName)
    {
        testFramework.sendSetTriangleColorToTestApplication(color);
        testFramework.sendRenderOneFrameToSharedMemoryBufferToTestApplication(testSurfaceId, true);
        testFramework.waitUntilNumberOfCommitedFramesForIviSurface(waylandSurfaceIviId, ++frameCount);
        if (!testFramework.renderAndCompareScreenshot(expectedImageName))
            return false;

        // Check, that for first frame one buffer is allocated, for second frame, two buffers are allocated, and that then
        // no more further buffers are needed. Checks if the Embedded Compositor releases the buffers.
        const uint32_t expectedNumberOfAllocatedSHMBuffer = std::min(frameCount, 2u);
        const uint32_t numberOfAllocatedSHMBuffer = testFramework.getNumberOfAllocatedSHMBufferFromTestApplication();

        if (numberOfAllocatedSHMBuffer != expectedNumberOfAllocatedSHMBuffer)
        {
            LOG_ERROR(CONTEXT_RENDERER, "SharedMemoryBufferTests::RenderAndCheckOneSharedMemoryFrame Number of allocated SHM buffers " << numberOfAllocatedSHMBuffer << " does not match expected value " << expectedNumberOfAllocatedSHMBuffer <<"!");
            return false;
        }
        return true;
    }

    bool SharedMemoryBufferTests::CheckFreeBufferState(EmbeddedCompositingTestsFramework& testFramework, std::string_view expectedBufferFreeState)
    {
        std::string bufferFreeState;
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
            LOG_ERROR(CONTEXT_RENDERER, "SharedMemoryBufferTests::CheckFreeBufferState Expected buffer free state: " << expectedBufferFreeState << " does not match actual buffer free state: " << bufferFreeState);
            return false;
        }
        return true;
    }
}
