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
    testFramework.createTestCaseWithDefaultDisplay(OffscreenBufferLinkTest_ProviderBufferDestroyed, *this, baseTestName + "ProviderBufferDestroyed");
    testFramework.createTestCaseWithDefaultDisplay(OffscreenBufferLinkTest_SourceSceneHidden, *this, baseTestName + "SourceSceneHidden");
    testFramework.createTestCaseWithDefaultDisplay(OffscreenBufferLinkTest_SourceSceneHiddenAndUnmappedInOneFlush, *this, baseTestName + "SourceSceneHiddenAndUnmappedInOneFlush");
    testFramework.createTestCaseWithDefaultDisplay(OffscreenBufferLinkTest_SourceSceneAssignedToFBWhileShown, *this, baseTestName + "SourceSceneAssignedToFBWhileShown");
    testFramework.createTestCaseWithDefaultDisplay(OffscreenBufferLinkTest_OneOfTwoSourceScenesUnmapped, *this, baseTestName + "OneOfTwoSourceScenesUnmapped");
    testFramework.createTestCaseWithDefaultDisplay(OffscreenBufferLinkTest_ProviderSceneUsesDepthTest, *this, baseTestName + "ProviderSceneUsesDepthTest");
    testFramework.createTestCaseWithDefaultDisplay(OffscreenBufferLinkTest_ProviderSceneUsesStencilTest, *this, baseTestName + "ProviderSceneUsesStencilTest");
}

bool OffscreenBufferLinkTests::run(RendererTestsFramework& testFramework, const RenderingTestCase& testCase)
{
    String expectedImageName("!!unknown!!");
    float expectedPixelError = RendererTestUtils::DefaultMaxAveragePercentPerPixel;
    switch (testCase.m_id)
    {
    case OffscreenBufferLinkTest_ConsumerLinkedToEmptyBuffer:
    {
        m_sceneIdConsumer = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_CONSUMER, m_cameraMid);

        const ramses::offscreenBufferId_t offscreenBuffer = testFramework.createOffscreenBuffer(0, 256, 128, m_interruptibleBuffers);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);

        expectedImageName = "OffscreenBufferLinkTest_Black";
        break;
    }
    case OffscreenBufferLinkTest_ConsumersLinkedToBufferWithOneScene:
    {
        m_sceneIdProvider = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_PROVIDER_LARGE, m_cameraMid, -1);
        m_sceneIdConsumer = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_CONSUMER, m_cameraMid, 1);

        const ramses::offscreenBufferId_t offscreenBuffer = testFramework.createOffscreenBuffer(0, 256, 128, m_interruptibleBuffers);
        testFramework.assignSceneToOffscreenBuffer(m_sceneIdProvider, offscreenBuffer);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);

        expectedImageName = "OffscreenBufferLinkTest_Linked";
        break;
    }
    case OffscreenBufferLinkTest_ConsumerLinkedToBufferWithTwoScenes:
    {
        m_sceneIdConsumer = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_CONSUMER, m_cameraMid, 1);
        m_sceneIdProvider = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_PROVIDER_LARGE, m_cameraMid, -1);
        m_sceneIdProvider2 = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_CONSUMER_AND_PROVIDER_LARGE, m_cameraLow, -1);

        const ramses::offscreenBufferId_t offscreenBuffer = testFramework.createOffscreenBuffer(0, 256, 128, m_interruptibleBuffers);
        testFramework.assignSceneToOffscreenBuffer(m_sceneIdProvider, offscreenBuffer);
        testFramework.assignSceneToOffscreenBuffer(m_sceneIdProvider2, offscreenBuffer);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);

        expectedImageName = "OffscreenBufferLinkTest_LinkedTwoScenes";
        break;
    }
    case OffscreenBufferLinkTest_OneOfTwoLinksRemoved:
    {
        m_sceneIdConsumer = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_CONSUMER, m_cameraHigh, 1);
        m_sceneIdConsumer2 = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_CONSUMER, m_cameraMid, 1);
        m_sceneIdProvider = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_PROVIDER_LARGE, m_cameraLow, -1);

        const ramses::offscreenBufferId_t offscreenBuffer = testFramework.createOffscreenBuffer(0, 256, 128, m_interruptibleBuffers);
        testFramework.assignSceneToOffscreenBuffer(m_sceneIdProvider, offscreenBuffer);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer2, TextureLinkScene::DataConsumerId);

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
        m_sceneIdProvider = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_PROVIDER_LARGE, m_cameraMid, -1);
        m_sceneIdConsumer = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_CONSUMER, m_cameraMid, 1);

        const ramses::offscreenBufferId_t offscreenBuffer = testFramework.createOffscreenBuffer(0, 256, 128, m_interruptibleBuffers);
        testFramework.assignSceneToOffscreenBuffer(m_sceneIdProvider, offscreenBuffer);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);

        if (!renderAndCompareScreenshot(testFramework, "OffscreenBufferLinkTest_Linked", 0u))
        {
            return false;
        }

        testFramework.assignSceneToFramebuffer(m_sceneIdProvider);
        testFramework.destroyOffscreenBuffer(0, offscreenBuffer);

        expectedImageName = "OffscreenBufferLinkTest_BufferDestroyed";
        break;
    }
    case OffscreenBufferLinkTest_SourceSceneHidden:
    {
        m_sceneIdProvider = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_PROVIDER_LARGE, m_cameraMid, -1);
        m_sceneIdConsumer = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_CONSUMER, m_cameraMid, 1);

        const ramses::offscreenBufferId_t offscreenBuffer = testFramework.createOffscreenBuffer(0, 256, 128, m_interruptibleBuffers);
        testFramework.assignSceneToOffscreenBuffer(m_sceneIdProvider, offscreenBuffer);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);

        if (!renderAndCompareScreenshot(testFramework, "OffscreenBufferLinkTest_Linked", 0u))
        {
            return false;
        }

        testFramework.hideScene(m_sceneIdProvider);

        expectedImageName = "OffscreenBufferLinkTest_Black";
        break;
    }
    case OffscreenBufferLinkTest_SourceSceneHiddenAndUnmappedInOneFlush:
    {
        m_sceneIdProvider = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_PROVIDER_LARGE, m_cameraMid, -1);
        m_sceneIdConsumer = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_CONSUMER, m_cameraMid, 1);

        const ramses::offscreenBufferId_t offscreenBuffer = testFramework.createOffscreenBuffer(0, 256, 128, m_interruptibleBuffers);
        testFramework.assignSceneToOffscreenBuffer(m_sceneIdProvider, offscreenBuffer);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);

        if (!renderAndCompareScreenshot(testFramework, "OffscreenBufferLinkTest_Linked", 0u))
        {
            return false;
        }

        testFramework.hideAndUnmap(m_sceneIdProvider);

        expectedImageName = "OffscreenBufferLinkTest_Black";
        break;
    }
    case OffscreenBufferLinkTest_SourceSceneAssignedToFBWhileShown:
    {
        m_sceneIdProvider = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_PROVIDER_LARGE, m_cameraMid, -1);
        m_sceneIdConsumer = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_CONSUMER, m_cameraMid, 1);

        const ramses::offscreenBufferId_t offscreenBuffer = testFramework.createOffscreenBuffer(0, 256, 128, m_interruptibleBuffers);
        testFramework.assignSceneToOffscreenBuffer(m_sceneIdProvider, offscreenBuffer);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);

        if (!renderAndCompareScreenshot(testFramework, "OffscreenBufferLinkTest_Linked", 0u))
        {
            return false;
        }

        testFramework.assignSceneToFramebuffer(m_sceneIdProvider);

        expectedImageName = "OffscreenBufferLinkTest_BlackOB";
        break;
    }
    case OffscreenBufferLinkTest_OneOfTwoSourceScenesUnmapped:
    {
        m_sceneIdConsumer = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_CONSUMER, m_cameraMid, 1);
        m_sceneIdProvider = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_PROVIDER_LARGE, m_cameraMid, -1);
        m_sceneIdProvider2 = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_CONSUMER_AND_PROVIDER_LARGE, m_cameraLow, -1);

        const ramses::offscreenBufferId_t offscreenBuffer = testFramework.createOffscreenBuffer(0, 256, 128, m_interruptibleBuffers);
        testFramework.assignSceneToOffscreenBuffer(m_sceneIdProvider, offscreenBuffer);
        testFramework.assignSceneToOffscreenBuffer(m_sceneIdProvider2, offscreenBuffer);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);

        if (!renderAndCompareScreenshot(testFramework, "OffscreenBufferLinkTest_LinkedTwoScenes", 0u))
        {
            return false;
        }

        testFramework.hideScene(m_sceneIdProvider2);
        testFramework.unmapScene(m_sceneIdProvider2);

        // Scene unmapped -> buffer is still there, but is not cleared any more
        expectedImageName = "OffscreenBufferLinkTest_Linked";
        break;
    }
    case OffscreenBufferLinkTest_ProviderSceneUsesDepthTest:
    {
        m_sceneIdProvider = testFramework.createAndShowScene<MultipleTrianglesScene>(MultipleTrianglesScene::DEPTH_FUNC, Vector3(0.2f, 0.5f, -5.f), -1);
        m_sceneIdConsumer = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_CONSUMER, Vector3(0.f, 0.f, -3.f), 1);

        const ramses::offscreenBufferId_t offscreenBuffer = testFramework.createOffscreenBuffer(0, 256, 256, m_interruptibleBuffers);
        testFramework.assignSceneToOffscreenBuffer(m_sceneIdProvider, offscreenBuffer);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);

        expectedImageName = "OffscreenBufferLinkTest_DepthTest";
        break;
    }
    case OffscreenBufferLinkTest_ProviderSceneUsesStencilTest:
    {
        m_sceneIdProvider = testFramework.createAndShowScene<MultipleTrianglesScene>(MultipleTrianglesScene::STENCIL_TEST_1, Vector3(0.f, 0.5f, -5.f), -1);
        m_sceneIdConsumer = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_CONSUMER, Vector3(0.f, 0.f, -4.f), 1);

        const ramses::offscreenBufferId_t offscreenBuffer = testFramework.createOffscreenBuffer(0, 256, 256, m_interruptibleBuffers);
        testFramework.assignSceneToOffscreenBuffer(m_sceneIdProvider, offscreenBuffer);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);

        expectedImageName = "OffscreenBufferLinkTest_StencilTest";
        break;
    }
    default:
        assert(false && "undefined test case");
    }

    return renderAndCompareScreenshot(testFramework, expectedImageName, 0u, expectedPixelError);
}

bool OffscreenBufferLinkTests::renderAndCompareScreenshot(RendererTestsFramework& testFramework, const ramses_internal::String& expectedImageName, uint32_t testDisplayIdx, float expectedPixelError)
{
    if (m_interruptibleBuffers)
    {
        // Changes to interruptible offscreen buffers are delayed by 1 frame, render one frame before taking screenshot.
        // On wayland platforms a frame is not guaranteed to be rendered (see DisplayController::canRenderNewFrame),
        // there is also no way to query if frame was rendered, this is just an experimental sleep value that should be
        // high enough in most cases for wayland to consume previous frame and therefor render the next one.
        // TODO vaclav: delay events affecting rendered result (show,hide,assign,link) till all displays rendered at least 1 (better 2 in case of delayed effect of int.OBs) frame since execution
        PlatformThread::Sleep(50);
        testFramework.flushRendererAndDoOneLoop();
    }
    return testFramework.renderAndCompareScreenshot(expectedImageName, testDisplayIdx, expectedPixelError);
}
