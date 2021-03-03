//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "OffscreenBufferLinkTests.h"
#include "TestScenes/TextureLinkScene.h"
#include "TestScenes/MultipleTrianglesScene.h"
#include "PlatformAbstraction/PlatformThread.h"

using namespace ramses_internal;

OffscreenBufferLinkTests::OffscreenBufferLinkTests(bool useInterruptibleBuffers)
    : m_interruptibleBuffers(useInterruptibleBuffers)
{
}

void OffscreenBufferLinkTests::setUpTestCases(RendererTestsFramework& testFramework)
{
    const String baseTestName = (m_interruptibleBuffers ? "InterruptibleOffscreenBufferLinkTest_" : "OffscreenBufferLinkTest_");
    testFramework.createTestCaseWithDefaultDisplay(OffscreenBufferLinkTest_ConsumerLinkedToEmptyBuffer, *this, baseTestName + "ConsumerLinkedToEmptyBuffer");
    testFramework.createTestCaseWithDefaultDisplay(OffscreenBufferLinkTest_ConsumersLinkedToBufferWithOneScene, *this, baseTestName + "ConsumersLinkedToBufferWithOneScene");
    testFramework.createTestCaseWithDefaultDisplay(OffscreenBufferLinkTest_OneOfTwoLinksRemoved, *this, baseTestName + "OneOfTwoLinksRemoved");
    testFramework.createTestCaseWithDefaultDisplay(OffscreenBufferLinkTest_ConsumerLinkedToBufferWithTwoScenes, *this, baseTestName + "ConsumerLinkedToBufferWithTwoScenes");
    testFramework.createTestCaseWithDefaultDisplay(OffscreenBufferLinkTest_ConsumerRelinkedToAnotherBuffer, *this, baseTestName + "ConsumerRelinkedToAnotherBuffer");
    testFramework.createTestCaseWithDefaultDisplay(OffscreenBufferLinkTest_ProviderBufferDestroyed, *this, baseTestName + "ProviderBufferDestroyed");
    testFramework.createTestCaseWithDefaultDisplay(OffscreenBufferLinkTest_SourceSceneHidden, *this, baseTestName + "SourceSceneHidden");
    testFramework.createTestCaseWithDefaultDisplay(OffscreenBufferLinkTest_SourceSceneAssignedToFBWhileShown, *this, baseTestName + "SourceSceneAssignedToFBWhileShown");
    testFramework.createTestCaseWithDefaultDisplay(OffscreenBufferLinkTest_OneOfTwoSourceScenesUnmapped, *this, baseTestName + "OneOfTwoSourceScenesUnmapped");
    testFramework.createTestCaseWithDefaultDisplay(OffscreenBufferLinkTest_ProviderSceneUsesDepthTest, *this, baseTestName + "ProviderSceneUsesDepthTest");
    testFramework.createTestCaseWithDefaultDisplay(OffscreenBufferLinkTest_ProviderSceneUsesStencilTest, *this, baseTestName + "ProviderSceneUsesStencilTest");
    testFramework.createTestCaseWithDefaultDisplay(OffscreenBufferLinkTest_SetClearColor, *this, baseTestName + "OffscreenBufferLinkTest_SetClearColor");
    if (!m_interruptibleBuffers)
    {
        testFramework.createTestCaseWithDefaultDisplay(OffscreenBufferLinkTest_ConsumerLinkedToMSAABuffer, *this, baseTestName + "OffscreenBufferLinkTest_MSAA");
        testFramework.createTestCaseWithDefaultDisplay(OffscreenBufferLinkTest_ConsumerUnlinkedMSAABuffer, *this, baseTestName + "OffscreenBufferLinkTest_MSAAUnlinked");
    }
}

bool OffscreenBufferLinkTests::run(RendererTestsFramework& testFramework, const RenderingTestCase& testCase)
{
    String expectedImageName("!!unknown!!");
    float expectedPixelError = RendererTestUtils::DefaultMaxAveragePercentPerPixel;
    switch (testCase.m_id)
    {
    case OffscreenBufferLinkTest_ConsumerLinkedToEmptyBuffer:
    {
        m_sceneIdConsumer = createAndShowScene<TextureLinkScene>(testFramework, TextureLinkScene::DATA_CONSUMER, m_cameraMid);

        const ramses::displayBufferId_t offscreenBuffer = testFramework.createOffscreenBuffer(0, OBWidth, OBHeight, m_interruptibleBuffers);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);

        expectedImageName = "OffscreenBufferLinkTest_Black";
        break;
    }
    case OffscreenBufferLinkTest_ConsumersLinkedToBufferWithOneScene:
    {
        m_sceneIdProvider = createAndShowScene<TextureLinkScene>(testFramework, TextureLinkScene::DATA_PROVIDER_LARGE, m_cameraMid, OBWidth, OBHeight);
        m_sceneIdConsumer = createAndShowScene<TextureLinkScene>(testFramework, TextureLinkScene::DATA_CONSUMER, m_cameraMid);

        const ramses::displayBufferId_t offscreenBuffer = testFramework.createOffscreenBuffer(0, OBWidth, OBHeight, m_interruptibleBuffers);
        testFramework.assignSceneToDisplayBuffer(m_sceneIdProvider, offscreenBuffer);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);
        if (!testFramework.renderAndCompareScreenshotOffscreenBuffer("OffscreenBufferLinkTest_Linked_OB_Content", 0u, offscreenBuffer, OBWidth, OBHeight))
            return false;

        expectedImageName = "OffscreenBufferLinkTest_Linked";
        break;
    }
    case OffscreenBufferLinkTest_ConsumerLinkedToMSAABuffer:
    {
        m_sceneIdProvider = createAndShowScene<MultipleTrianglesScene>(testFramework, MultipleTrianglesScene::TRIANGLES_REORDERED, Vector3(0.2f, 0.5f, -5.f), 16u, 16u);
        m_sceneIdConsumer = createAndShowScene<TextureLinkScene>(testFramework, TextureLinkScene::DATA_CONSUMER_MS, { 0.2f, 0.f, -3.f });

        const ramses::displayBufferId_t offscreenBuffer = testFramework.createOffscreenBuffer(0, 16u, 16u, m_interruptibleBuffers, 4u);
        testFramework.assignSceneToDisplayBuffer(m_sceneIdProvider, offscreenBuffer);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);

        expectedImageName = "OffscreenBufferLinkTest_MSAABuffer";
        break;
    }
    case OffscreenBufferLinkTest_ConsumerUnlinkedMSAABuffer:
    {
        m_sceneIdProvider = createAndShowScene<MultipleTrianglesScene>(testFramework, MultipleTrianglesScene::TRIANGLES_REORDERED, Vector3(0.2f, 0.5f, -5.f), 16u, 16u);
        m_sceneIdConsumer = createAndShowScene<TextureLinkScene>(testFramework, TextureLinkScene::DATA_CONSUMER_MS, { 0.2f, 0.f, -3.f });

        const ramses::displayBufferId_t offscreenBuffer = testFramework.createOffscreenBuffer(0, 16u, 16u, m_interruptibleBuffers, 4u);
        testFramework.assignSceneToDisplayBuffer(m_sceneIdProvider, offscreenBuffer);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);

        if (!renderAndCompareScreenshot(testFramework, "OffscreenBufferLinkTest_MSAABuffer", 0u))
        {
            return false;
        }

        testFramework.removeDataLink(m_sceneIdConsumer, TextureLinkScene::DataConsumerId);

        expectedImageName = "OffscreenBufferLinkTest_UnlinkedMSAABuffer";
        break;
    }
    case OffscreenBufferLinkTest_ConsumerLinkedToBufferWithTwoScenes:
    {
        m_sceneIdConsumer = createAndShowScene<TextureLinkScene>(testFramework, TextureLinkScene::DATA_CONSUMER, m_cameraMid);
        m_sceneIdProvider = createAndShowScene<TextureLinkScene>(testFramework, TextureLinkScene::DATA_PROVIDER_LARGE, m_cameraMid, OBWidth, OBHeight);
        m_sceneIdProvider2 = createAndShowScene<TextureLinkScene>(testFramework, TextureLinkScene::DATA_CONSUMER_AND_PROVIDER_LARGE, m_cameraLow, OBWidth, OBHeight);

        const ramses::displayBufferId_t offscreenBuffer = testFramework.createOffscreenBuffer(0, OBWidth, OBHeight, m_interruptibleBuffers);
        testFramework.assignSceneToDisplayBuffer(m_sceneIdProvider, offscreenBuffer);
        testFramework.assignSceneToDisplayBuffer(m_sceneIdProvider2, offscreenBuffer);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);
        if (!testFramework.renderAndCompareScreenshotOffscreenBuffer("OffscreenBufferLinkTest_LinkedTwoScenes_OB_Content", 0u, offscreenBuffer, OBWidth, OBHeight))
            return false;

        expectedImageName = "OffscreenBufferLinkTest_LinkedTwoScenes";
        break;
    }
    case OffscreenBufferLinkTest_ConsumerRelinkedToAnotherBuffer:
    {
        m_sceneIdConsumer = createAndShowScene<TextureLinkScene>(testFramework, TextureLinkScene::DATA_CONSUMER, m_cameraMid);
        m_sceneIdProvider = createAndShowScene<TextureLinkScene>(testFramework, TextureLinkScene::DATA_PROVIDER_LARGE, m_cameraMid, OBWidth, OBHeight);
        m_sceneIdProvider2 = createAndShowScene<TextureLinkScene>(testFramework, TextureLinkScene::DATA_CONSUMER_AND_PROVIDER_LARGE, m_cameraLow, OBWidth, OBHeight);

        const ramses::displayBufferId_t offscreenBuffer1 = testFramework.createOffscreenBuffer(0, OBWidth, OBHeight, m_interruptibleBuffers);
        const ramses::displayBufferId_t offscreenBuffer2 = testFramework.createOffscreenBuffer(0, OBWidth, OBHeight, m_interruptibleBuffers);
        testFramework.assignSceneToDisplayBuffer(m_sceneIdProvider, offscreenBuffer1);
        testFramework.assignSceneToDisplayBuffer(m_sceneIdProvider2, offscreenBuffer2);
        testFramework.createBufferDataLink(offscreenBuffer1, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);
        if (!testFramework.renderAndCompareScreenshotOffscreenBuffer("OffscreenBufferLinkTest_Linked_OB_Content", 0u, offscreenBuffer1, OBWidth, OBHeight))
            return false;
        if (!testFramework.renderAndCompareScreenshotOffscreenBuffer("OffscreenBufferLinkTest_Linked2_OB_Content", 0u, offscreenBuffer2, OBWidth, OBHeight))
            return false;

        if (!renderAndCompareScreenshot(testFramework, "OffscreenBufferLinkTest_Linked", 0u))
            return false;

        // relink to another buffer
        testFramework.createBufferDataLink(offscreenBuffer2, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);
        if (!testFramework.renderAndCompareScreenshotOffscreenBuffer("OffscreenBufferLinkTest_Linked2_OB_Content", 0u, offscreenBuffer2, OBWidth, OBHeight))
            return false;

        expectedImageName = "OffscreenBufferLinkTest_Linked2";
        break;
    }
    case OffscreenBufferLinkTest_OneOfTwoLinksRemoved:
    {
        m_sceneIdConsumer = createAndShowScene<TextureLinkScene>(testFramework, TextureLinkScene::DATA_CONSUMER, m_cameraHigh);
        m_sceneIdConsumer2 = createAndShowScene<TextureLinkScene>(testFramework, TextureLinkScene::DATA_CONSUMER, m_cameraMid);
        m_sceneIdProvider = createAndShowScene<TextureLinkScene>(testFramework, TextureLinkScene::DATA_PROVIDER_LARGE, m_cameraLow, OBWidth, OBHeight);

        const ramses::displayBufferId_t offscreenBuffer = testFramework.createOffscreenBuffer(0, OBWidth, OBHeight, m_interruptibleBuffers);
        testFramework.assignSceneToDisplayBuffer(m_sceneIdProvider, offscreenBuffer);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer2, TextureLinkScene::DataConsumerId);
        if (!testFramework.renderAndCompareScreenshotOffscreenBuffer("OffscreenBufferLinkTest_LinkedTwoConsumers_OB_Content", 0u, offscreenBuffer, OBWidth, OBHeight))
            return false;

        if (!renderAndCompareScreenshot(testFramework, "OffscreenBufferLinkTest_LinkedTwoConsumers", 0u))
        {
            return false;
        }

        testFramework.removeDataLink(m_sceneIdConsumer, TextureLinkScene::DataConsumerId);

        expectedImageName = "OffscreenBufferLinkTest_Unlinked";
        break;
    }
    case OffscreenBufferLinkTest_ProviderBufferDestroyed:
    {
        m_sceneIdProvider = createAndShowScene<TextureLinkScene>(testFramework, TextureLinkScene::DATA_PROVIDER_LARGE, m_cameraMid, OBWidth, OBHeight);
        m_sceneIdConsumer = createAndShowScene<TextureLinkScene>(testFramework, TextureLinkScene::DATA_CONSUMER, m_cameraMid);

        const ramses::displayBufferId_t offscreenBuffer = testFramework.createOffscreenBuffer(0, OBWidth, OBHeight, m_interruptibleBuffers);
        testFramework.assignSceneToDisplayBuffer(m_sceneIdProvider, offscreenBuffer);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);

        if (!renderAndCompareScreenshot(testFramework, "OffscreenBufferLinkTest_Linked", 0u))
        {
            return false;
        }

        testFramework.assignSceneToDisplayBuffer(m_sceneIdProvider, testFramework.getDisplayFramebufferId(0));
        testFramework.destroyOffscreenBuffer(0, offscreenBuffer);

        expectedImageName = "OffscreenBufferLinkTest_BufferDestroyed";
        break;
    }
    case OffscreenBufferLinkTest_SourceSceneHidden:
    {
        m_sceneIdProvider = createAndShowScene<TextureLinkScene>(testFramework, TextureLinkScene::DATA_PROVIDER_LARGE, m_cameraMid, OBWidth, OBHeight);
        m_sceneIdConsumer = createAndShowScene<TextureLinkScene>(testFramework, TextureLinkScene::DATA_CONSUMER, m_cameraMid);

        const ramses::displayBufferId_t offscreenBuffer = testFramework.createOffscreenBuffer(0, OBWidth, OBHeight, m_interruptibleBuffers);
        testFramework.assignSceneToDisplayBuffer(m_sceneIdProvider, offscreenBuffer);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);

        if (!renderAndCompareScreenshot(testFramework, "OffscreenBufferLinkTest_Linked", 0u))
        {
            return false;
        }

        testFramework.getSceneToState(m_sceneIdProvider, ramses::RendererSceneState::Ready);

        expectedImageName = "OffscreenBufferLinkTest_Black";
        break;
    }
    case OffscreenBufferLinkTest_SourceSceneAssignedToFBWhileShown:
    {
        m_sceneIdProvider = createAndShowScene<TextureLinkScene>(testFramework, TextureLinkScene::DATA_PROVIDER_LARGE, m_cameraMid, OBWidth, OBHeight);
        m_sceneIdConsumer = createAndShowScene<TextureLinkScene>(testFramework, TextureLinkScene::DATA_CONSUMER, m_cameraMid);

        const ramses::displayBufferId_t offscreenBuffer = testFramework.createOffscreenBuffer(0, OBWidth, OBHeight, m_interruptibleBuffers);
        testFramework.assignSceneToDisplayBuffer(m_sceneIdProvider, offscreenBuffer);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);

        if (!renderAndCompareScreenshot(testFramework, "OffscreenBufferLinkTest_Linked", 0u))
        {
            return false;
        }

        testFramework.assignSceneToDisplayBuffer(m_sceneIdProvider, testFramework.getDisplayFramebufferId(0));

        expectedImageName = "OffscreenBufferLinkTest_BlackOB";
        break;
    }
    case OffscreenBufferLinkTest_OneOfTwoSourceScenesUnmapped:
    {
        m_sceneIdConsumer = createAndShowScene<TextureLinkScene>(testFramework, TextureLinkScene::DATA_CONSUMER, m_cameraMid);
        m_sceneIdProvider = createAndShowScene<TextureLinkScene>(testFramework, TextureLinkScene::DATA_PROVIDER_LARGE, m_cameraMid, OBWidth, OBHeight);
        m_sceneIdProvider2 = createAndShowScene<TextureLinkScene>(testFramework, TextureLinkScene::DATA_CONSUMER_AND_PROVIDER_LARGE, m_cameraLow, OBWidth, OBHeight);

        const ramses::displayBufferId_t offscreenBuffer = testFramework.createOffscreenBuffer(0, OBWidth, OBHeight, m_interruptibleBuffers);
        testFramework.assignSceneToDisplayBuffer(m_sceneIdProvider, offscreenBuffer);
        testFramework.assignSceneToDisplayBuffer(m_sceneIdProvider2, offscreenBuffer);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);

        if (!renderAndCompareScreenshot(testFramework, "OffscreenBufferLinkTest_LinkedTwoScenes", 0u))
        {
            return false;
        }

        testFramework.getSceneToState(m_sceneIdProvider2, ramses::RendererSceneState::Available);

        // Scene unmapped -> buffer is still there, but is not cleared any more
        expectedImageName = "OffscreenBufferLinkTest_Linked";
        break;
    }
    case OffscreenBufferLinkTest_ProviderSceneUsesDepthTest:
    {
        m_sceneIdProvider = createAndShowScene<MultipleTrianglesScene>(testFramework, MultipleTrianglesScene::DEPTH_FUNC, Vector3(0.2f, 0.5f, -5.f), OBWidth, OBHeight);
        m_sceneIdConsumer = createAndShowScene<TextureLinkScene>(testFramework, TextureLinkScene::DATA_CONSUMER, Vector3(0.f, 0.f, -3.f));

        const ramses::displayBufferId_t offscreenBuffer = testFramework.createOffscreenBuffer(0, OBWidth, OBHeight, m_interruptibleBuffers, 0u, ramses::EDepthBufferType_Depth);
        testFramework.assignSceneToDisplayBuffer(m_sceneIdProvider, offscreenBuffer);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);

        expectedImageName = "OffscreenBufferLinkTest_DepthTest";
        break;
    }
    case OffscreenBufferLinkTest_ProviderSceneUsesStencilTest:
    {
        m_sceneIdProvider = createAndShowScene<MultipleTrianglesScene>(testFramework, MultipleTrianglesScene::STENCIL_TEST_1, Vector3(0.f, 0.5f, -5.f), OBWidth, OBHeight);
        m_sceneIdConsumer = createAndShowScene<TextureLinkScene>(testFramework, TextureLinkScene::DATA_CONSUMER, Vector3(0.f, 0.f, -4.f));

        const ramses::displayBufferId_t offscreenBuffer = testFramework.createOffscreenBuffer(0, OBWidth, OBHeight, m_interruptibleBuffers, 0u, ramses::EDepthBufferType_DepthStencil);
        testFramework.assignSceneToDisplayBuffer(m_sceneIdProvider, offscreenBuffer);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);

        expectedImageName = "OffscreenBufferLinkTest_StencilTest";
        break;
    }
    case OffscreenBufferLinkTest_SetClearColor:
    {
        m_sceneIdProvider = createAndShowScene<TextureLinkScene>(testFramework, TextureLinkScene::DATA_PROVIDER_LARGE, m_cameraMid, OBWidth, OBHeight);
        m_sceneIdConsumer = createAndShowScene<TextureLinkScene>(testFramework, TextureLinkScene::DATA_CONSUMER, m_cameraMid);

        const ramses::displayBufferId_t offscreenBuffer = testFramework.createOffscreenBuffer(0, OBWidth, OBHeight, m_interruptibleBuffers);
        testFramework.assignSceneToDisplayBuffer(m_sceneIdProvider, offscreenBuffer);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);

        if (!renderAndCompareScreenshot(testFramework, "OffscreenBufferLinkTest_Linked", 0u))
            return false;

        testFramework.setClearColor(0u, ramses::displayBufferId_t::Invalid(), { 0, 1, 0, 1 });
        testFramework.setClearColor(0u, offscreenBuffer, { 0, 0, 1, 1 });

        expectedImageName = "OffscreenBufferLinkTest_LinkedCustomClearColor";
        break;
    }
    default:
        assert(false && "undefined test case");
    }

    return renderAndCompareScreenshot(testFramework, expectedImageName, 0u, expectedPixelError);
}

template <typename INTEGRATION_SCENE>
ramses::sceneId_t OffscreenBufferLinkTests::createAndShowScene(RendererTestsFramework& testFramework, uint32_t sceneState, const Vector3& camPos, uint32_t vpWidth, uint32_t vpHeight)
{
    const auto sceneId = testFramework.getScenesRegistry().createScene<INTEGRATION_SCENE>(sceneState, camPos, vpWidth, vpHeight);
    testFramework.publishAndFlushScene(sceneId);
    testFramework.getSceneToRendered(sceneId);

    return sceneId;
}

bool OffscreenBufferLinkTests::renderAndCompareScreenshot(RendererTestsFramework& testFramework, const ramses_internal::String& expectedImageName, uint32_t testDisplayIdx, float expectedPixelError)
{
    // Any changes to interruptible offscreen buffers affect the framebuffer (with consumer) only the next frame due to being executed always as last in each frame.
    // Adding simply another doOneLoop here might not always work on wayland platforms, where a frame is not guaranteed to be rendered (see DisplayController::canRenderNewFrame).
    // There is no way to find out if a frame was rendered and therefore the solution here is to execute reading of pixels twice before checking result
    // (argument flag passed in call below), the read pixels command guarantees that a frame was rendered.
    // Generally it still cannot be guaranteed if interruptible OB is really finished (not interrupted) but we assume no interruptions due to no rendering time limits used.

    return testFramework.renderAndCompareScreenshot(expectedImageName, testDisplayIdx, expectedPixelError, m_interruptibleBuffers);
}
