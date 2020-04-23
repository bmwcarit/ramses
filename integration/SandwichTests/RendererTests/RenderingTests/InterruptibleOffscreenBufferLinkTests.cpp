//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "InterruptibleOffscreenBufferLinkTests.h"
#include "TestScenes/TextureLinkScene.h"
#include "TestScenes/MultipleTrianglesScene.h"
#include "RenderExecutor.h"

using namespace ramses_internal;

void InterruptibleOffscreenBufferLinkTests::setUpTestCases(RendererTestsFramework& testFramework)
{
    testFramework.createTestCaseWithDefaultDisplay(InterruptibleOffscreenBufferLinkTest_OneInterruptibleOBWithOneScene, *this, "InterruptibleOffscreenBufferLinkTest_OneInterruptibleOBWithOneScene")
        .m_displayConfigs.front().setClearColor(1.f, 0.f, 0.f, 1.f);
    testFramework.createTestCaseWithDefaultDisplay(InterruptibleOffscreenBufferLinkTest_OneInterruptibleOBWithTwoScenes, *this, "InterruptibleOffscreenBufferLinkTest_OneInterruptibleOBWithTwoScenes")
        .m_displayConfigs.front().setClearColor(1.f, 0.f, 0.f, 1.f);
    testFramework.createTestCaseWithDefaultDisplay(InterruptibleOffscreenBufferLinkTest_InterruptionDoesNotAffectFrameBufferScene, *this, "InterruptibleOffscreenBufferLinkTest_InterruptionDoesNotAffectFrameBufferScene")
        .m_displayConfigs.front().setClearColor(1.f, 0.f, 0.f, 1.f);
    testFramework.createTestCaseWithDefaultDisplay(InterruptibleOffscreenBufferLinkTest_TwoInterruptibleOBsEachWithOneScene, *this, "InterruptibleOffscreenBufferLinkTest_TwoInterruptibleOBsEachWithOneScene")
        .m_displayConfigs.front().setClearColor(1.f, 0.f, 0.f, 1.f);
    testFramework.createTestCaseWithDefaultDisplay(InterruptibleOffscreenBufferLinkTest_RerendersSceneIfItGetsModifiedWhileInterrupted, *this, "InterruptibleOffscreenBufferLinkTest_RerendersSceneIfItGetsModifiedWhileInterrupted")
        .m_displayConfigs.front().setClearColor(1.f, 0.f, 0.f, 1.f);
    testFramework.createTestCaseWithDefaultDisplay(InterruptibleOffscreenBufferLinkTest_RerendersInterruptedSceneIfItGetsModifiedWhileInterrupted, *this, "InterruptibleOffscreenBufferLinkTest_RerendersInterruptedSceneIfItGetsModifiedWhileInterrupted")
        .m_displayConfigs.front().setClearColor(1.f, 0.f, 0.f, 1.f);
    testFramework.createTestCaseWithDefaultDisplay(InterruptibleOffscreenBufferLinkTest_RerendersSceneIfItGetsModifiedWhileAnotherSceneIsBeingRendered, *this, "InterruptibleOffscreenBufferLinkTest_RerendersSceneIfItGetsModifiedWhileAnotherSceneIsBeingRendered")
        .m_displayConfigs.front().setClearColor(1.f, 0.f, 0.f, 1.f);
    testFramework.createTestCaseWithDefaultDisplay(InterruptibleOffscreenBufferLinkTest_RerendersInterruptedSceneIfItGetsModifiedWhileInterrupted_SameBuffer, *this, "InterruptibleOffscreenBufferLinkTest_RerendersInterruptedSceneIfItGetsModifiedWhileInterrupted_SameBuffer")
        .m_displayConfigs.front().setClearColor(1.f, 0.f, 0.f, 1.f);
    testFramework.createTestCaseWithDefaultDisplay(InterruptibleOffscreenBufferLinkTest_RerendersSceneIfItGetsModifiedWhileAnotherSceneIsBeingRendered_SameBuffer, *this, "InterruptibleOffscreenBufferLinkTest_RerendersSceneIfItGetsModifiedWhileAnotherSceneIsBeingRendered_SameBuffer")
        .m_displayConfigs.front().setClearColor(1.f, 0.f, 0.f, 1.f);
}

bool InterruptibleOffscreenBufferLinkTests::run(RendererTestsFramework& testFramework, const RenderingTestCase& testCase)
{
    testFramework.setFrameTimerLimits(std::numeric_limits<uint64_t>::max(), 0u);
    // Override the number of meshes rendered between the time budget checks, for the test we need to interrupt at every single renderable
    ramses_internal::RenderExecutor::NumRenderablesToRenderInBetweenTimeBudgetChecks = 1u;

    bool testResultValue = true;
    switch (testCase.m_id)
    {
    case InterruptibleOffscreenBufferLinkTest_OneInterruptibleOBWithOneScene:
    {
        const ramses::sceneId_t m_sceneIdProvider = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_PROVIDER_LARGE, m_cameraMid);
        const ramses::sceneId_t m_sceneIdConsumer = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_CONSUMER, m_cameraMid);

        const ramses::displayBufferId_t offscreenBuffer = testFramework.createOffscreenBuffer(0, 256, 128, true);
        testFramework.assignSceneToDisplayBuffer(m_sceneIdProvider, offscreenBuffer);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);

        // provider scene has two meshes, there is interruption after each, that means 3 frames to finish the rendering
        testResultValue &= renderAndCompareScreenshot(testFramework, "OffscreenBufferLinkTest_LinkedInterrupted", 3u);
        // once more frame to render FB with the finished OB contents
        testResultValue &= testFramework.renderAndCompareScreenshot("OffscreenBufferLinkTest_LinkedFinished");
        break;
    }
    case InterruptibleOffscreenBufferLinkTest_OneInterruptibleOBWithTwoScenes:
    {
        const ramses::sceneId_t m_sceneIdProvider = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_PROVIDER, m_cameraMid - Vector3{ 1.f, 0.f, 0.f });
        const ramses::sceneId_t m_sceneIdProvider2 = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_PROVIDER, m_cameraMid + Vector3{ 1.f, 0.f, 0.f });
        const ramses::sceneId_t m_sceneIdConsumer = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_CONSUMER, m_cameraMid);

        const ramses::displayBufferId_t offscreenBuffer = testFramework.createOffscreenBuffer(0, 256, 128, true);
        testFramework.assignSceneToDisplayBuffer(m_sceneIdProvider, offscreenBuffer);
        testFramework.assignSceneToDisplayBuffer(m_sceneIdProvider2, offscreenBuffer);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);

        // each provider scene has two meshes, there is interruption after each mesh, that means 5 frames to finish the rendering
        testResultValue &= renderAndCompareScreenshot(testFramework, "OffscreenBufferLinkTest_LinkedInterrupted", 5u);
        // once more frame to render FB with the finished OB contents
        testResultValue &= testFramework.renderAndCompareScreenshot("InterruptibleOffscreenBufferLinkTest_OneOBWithTwoScenes");
        break;
    }
    case InterruptibleOffscreenBufferLinkTest_InterruptionDoesNotAffectFrameBufferScene:
    {
        const ramses::sceneId_t trianglesSceneId = testFramework.createAndShowScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES);
        const ramses::sceneId_t m_sceneIdProvider = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_PROVIDER_LARGE, m_cameraMid);
        const ramses::sceneId_t m_sceneIdConsumer = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_CONSUMER, m_cameraMid + Vector3{ 0.f, 2.f, 0.f });

        const ramses::displayBufferId_t offscreenBuffer = testFramework.createOffscreenBuffer(0, 256, 128, true);
        testFramework.assignSceneToDisplayBuffer(m_sceneIdProvider, offscreenBuffer);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);

        testResultValue &= testFramework.renderAndCompareScreenshot("InterruptibleOffscreenBufferLinkTest_FBState0");

        testFramework.getScenesRegistry().setSceneState<MultipleTrianglesScene>(trianglesSceneId, MultipleTrianglesScene::ALPHA_BLENDING);
        testFramework.getScenesRegistry().getScene(trianglesSceneId).flush();
        testResultValue &= testFramework.renderAndCompareScreenshot("InterruptibleOffscreenBufferLinkTest_FBState1");

        testFramework.getScenesRegistry().setSceneState<MultipleTrianglesScene>(trianglesSceneId, MultipleTrianglesScene::COLOR_MASK);
        testFramework.getScenesRegistry().getScene(trianglesSceneId).flush();
        testResultValue &= testFramework.renderAndCompareScreenshot("InterruptibleOffscreenBufferLinkTest_FBState2");

        testFramework.getScenesRegistry().setSceneState<MultipleTrianglesScene>(trianglesSceneId, MultipleTrianglesScene::TRIANGLES_REORDERED);
        testFramework.getScenesRegistry().getScene(trianglesSceneId).flush();
        testResultValue &= testFramework.renderAndCompareScreenshot("InterruptibleOffscreenBufferLinkTest_FBState3Linked");
        break;
    }
    case InterruptibleOffscreenBufferLinkTest_TwoInterruptibleOBsEachWithOneScene:
    {
        const ramses::sceneId_t m_sceneIdProvider = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_PROVIDER_LARGE, m_cameraMid);
        const ramses::sceneId_t m_sceneIdProvider2 = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_PROVIDER_LARGE, m_cameraMid);
        const ramses::sceneId_t m_sceneIdConsumer = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_CONSUMER, m_cameraMid);
        const ramses::sceneId_t m_sceneIdConsumer2 = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_CONSUMER, m_cameraMid + Vector3{ 0.f, 2.f, 0.f });

        const ramses::displayBufferId_t offscreenBuffer = testFramework.createOffscreenBuffer(0, 256, 128, true);
        testFramework.assignSceneToDisplayBuffer(m_sceneIdProvider, offscreenBuffer);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);

        const ramses::displayBufferId_t offscreenBuffer2 = testFramework.createOffscreenBuffer(0, 256, 128, true);
        testFramework.assignSceneToDisplayBuffer(m_sceneIdProvider2, offscreenBuffer2);
        testFramework.createBufferDataLink(offscreenBuffer2, m_sceneIdConsumer2, TextureLinkScene::DataConsumerId);

        // each provider scene has two meshes, there is interruption after each mesh, that means 3 frames to finish the 1st provider
        testResultValue &= renderAndCompareScreenshot(testFramework, "OffscreenBufferLinkTest_LinkedInterrupted2", 3u);
        // in the 3rd frame 1st provider is finished and 2nd provider has one mesh rendered

        // 1st provider will be visible next frame, another 2 frames needed to finish 2nd provider
        testResultValue &= renderAndCompareScreenshot(testFramework, "OffscreenBufferLinkTest_LinkedFinished2", 2u);

        // 2nd provider will be visible next frame
        testResultValue &= testFramework.renderAndCompareScreenshot("InterruptibleOffscreenBufferLinkTest_TwoOBsEachWithOneScene");

        break;
    }
    case InterruptibleOffscreenBufferLinkTest_RerendersSceneIfItGetsModifiedWhileInterrupted:
    {
        const ramses::sceneId_t m_sceneIdProvider = testFramework.createAndShowScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES);
        const ramses::sceneId_t m_sceneIdConsumer = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_CONSUMER, m_cameraMid);

        const ramses::displayBufferId_t offscreenBuffer = testFramework.createOffscreenBuffer(0, 256, 128, true);
        testFramework.assignSceneToDisplayBuffer(m_sceneIdProvider, offscreenBuffer);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);

        // render 1 mesh and interrupt
        testResultValue &= testFramework.renderAndCompareScreenshot("OffscreenBufferLinkTest_LinkedInterrupted");

        // change scene state
        testFramework.getScenesRegistry().setSceneState<MultipleTrianglesScene>(m_sceneIdProvider, MultipleTrianglesScene::TRIANGLES_REORDERED);
        testFramework.getScenesRegistry().getScene(m_sceneIdProvider).flush();

        // render 3 more frames to finish initial scene state (and new state started to be rendered into OB's 'backbuffer')
        testResultValue &= renderAndCompareScreenshot(testFramework, "OffscreenBufferLinkTest_LinkedInterrupted", 3u);
        // next frame initial scene state is shown in FB and the new state is finished after 4 more frames
        testResultValue &= renderAndCompareScreenshot(testFramework, "InterruptibleOffscreenBufferLinkTest_ThreeTriangles", 4u);

        // next frame new scene state is shown in FB
        testResultValue &= testFramework.renderAndCompareScreenshot("InterruptibleOffscreenBufferLinkTest_TrianglesReordered");
        break;
    }

    case InterruptibleOffscreenBufferLinkTest_RerendersInterruptedSceneIfItGetsModifiedWhileInterrupted:
    {
        const ramses::sceneId_t m_sceneIdProvider = testFramework.createAndShowScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES);
        const ramses::sceneId_t m_sceneIdProvider2 = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_PROVIDER_LARGE, m_cameraMid);
        const ramses::sceneId_t m_sceneIdConsumer = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_CONSUMER, m_cameraMid);
        const ramses::sceneId_t m_sceneIdConsumer2 = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_CONSUMER, m_cameraMid + Vector3{ 0.f, 2.f, 0.f });

        const ramses::displayBufferId_t offscreenBuffer = testFramework.createOffscreenBuffer(0, 256, 128, true);
        testFramework.assignSceneToDisplayBuffer(m_sceneIdProvider, offscreenBuffer);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);

        const ramses::displayBufferId_t offscreenBuffer2 = testFramework.createOffscreenBuffer(0, 256, 128, true);
        testFramework.assignSceneToDisplayBuffer(m_sceneIdProvider2, offscreenBuffer2);
        testFramework.createBufferDataLink(offscreenBuffer2, m_sceneIdConsumer2, TextureLinkScene::DataConsumerId);

        // render 1 mesh and interrupt
        testResultValue &= testFramework.renderAndCompareScreenshot("OffscreenBufferLinkTest_LinkedInterrupted2");

        // change scene state
        testFramework.getScenesRegistry().setSceneState<MultipleTrianglesScene>(m_sceneIdProvider, MultipleTrianglesScene::TRIANGLES_REORDERED);
        testFramework.getScenesRegistry().getScene(m_sceneIdProvider).flush();

        // render 3 more frames to finish initial provider1 state (and provider2 started to be rendered)
        testResultValue &= renderAndCompareScreenshot(testFramework, "OffscreenBufferLinkTest_LinkedInterrupted2", 3u);
        // next frame initial provider1 state is shown in FB and provider2 is finished after 2 more frames
        testResultValue &= renderAndCompareScreenshot(testFramework, "InterruptibleOffscreenBufferLinkTest_ThreeTriangles2", 2u);
        // next frame provider2 is shown in FB and new state provider1 renders and finishes in 4 frames
        testResultValue &= renderAndCompareScreenshot(testFramework, "InterruptibleOffscreenBufferLinkTest_ThreeTriangles3", 4u);
        // new state provider1 is shown in FB
        testResultValue &= testFramework.renderAndCompareScreenshot("InterruptibleOffscreenBufferLinkTest_TrianglesReordered2");
        break;
    }
    case InterruptibleOffscreenBufferLinkTest_RerendersSceneIfItGetsModifiedWhileAnotherSceneIsBeingRendered:
    {
        const ramses::sceneId_t m_sceneIdProvider = testFramework.createAndShowScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES);
        const ramses::sceneId_t m_sceneIdProvider2 = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_PROVIDER_LARGE, m_cameraMid);
        const ramses::sceneId_t m_sceneIdConsumer = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_CONSUMER, m_cameraMid);
        const ramses::sceneId_t m_sceneIdConsumer2 = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_CONSUMER, m_cameraMid + Vector3{ 0.f, 2.f, 0.f });

        const ramses::displayBufferId_t offscreenBuffer = testFramework.createOffscreenBuffer(0, 256, 128, true);
        testFramework.assignSceneToDisplayBuffer(m_sceneIdProvider, offscreenBuffer);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);

        const ramses::displayBufferId_t offscreenBuffer2 = testFramework.createOffscreenBuffer(0, 256, 128, true);
        testFramework.assignSceneToDisplayBuffer(m_sceneIdProvider2, offscreenBuffer2);
        testFramework.createBufferDataLink(offscreenBuffer2, m_sceneIdConsumer2, TextureLinkScene::DataConsumerId);

        testResultValue &= renderAndCompareScreenshot(testFramework, "OffscreenBufferLinkTest_LinkedInterrupted2", 4u);
        //initial state of 1st provider scene gets visible (provider scene has 3 meshes, so get visible after 5 loops)
        testResultValue &= testFramework.renderAndCompareScreenshot("InterruptibleOffscreenBufferLinkTest_ThreeTriangles2");

        //modify state of 1st provider (now 2nd provider is being rendered)
        testFramework.getScenesRegistry().setSceneState<MultipleTrianglesScene>(m_sceneIdProvider, MultipleTrianglesScene::TRIANGLES_REORDERED);
        testFramework.getScenesRegistry().getScene(m_sceneIdProvider).flush();

        testResultValue &= testFramework.renderAndCompareScreenshot("InterruptibleOffscreenBufferLinkTest_ThreeTriangles2");
        //2nd provider gets visible (has 2 meshes)
        testResultValue &= renderAndCompareScreenshot(testFramework, "InterruptibleOffscreenBufferLinkTest_ThreeTriangles3", 4u);
        //modification of 1st provider gets visible
        testResultValue &= testFramework.renderAndCompareScreenshot("InterruptibleOffscreenBufferLinkTest_TrianglesReordered2");
        break;
    }
    case InterruptibleOffscreenBufferLinkTest_RerendersInterruptedSceneIfItGetsModifiedWhileInterrupted_SameBuffer:
    {
        const ramses::sceneId_t m_sceneIdProvider = testFramework.createAndShowScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES);
        const ramses::sceneId_t m_sceneIdProvider2 = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_PROVIDER_LARGE, m_cameraMid + Vector3{5.0f, 0.0f, 0.0f});
        const ramses::sceneId_t m_sceneIdConsumer = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_CONSUMER, m_cameraMid);

        const ramses::displayBufferId_t offscreenBuffer = testFramework.createOffscreenBuffer(0, 256, 128, true);
        testFramework.assignSceneToDisplayBuffer(m_sceneIdProvider, offscreenBuffer, 0);
        testFramework.assignSceneToDisplayBuffer(m_sceneIdProvider2, offscreenBuffer, -1);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);

        //render once then update provider
        testResultValue &= testFramework.renderAndCompareScreenshot("OffscreenBufferLinkTest_LinkedInterrupted");

        testFramework.getScenesRegistry().setSceneState<MultipleTrianglesScene>(m_sceneIdProvider, MultipleTrianglesScene::TRIANGLES_REORDERED);
        testFramework.getScenesRegistry().getScene(m_sceneIdProvider).flush();

        testResultValue &= renderAndCompareScreenshot(testFramework, "OffscreenBufferLinkTest_LinkedInterrupted", 5u);
        //both provider get visible (have 5 meshes together, need 7 loops)
        testResultValue &= renderAndCompareScreenshot(testFramework, "InterruptibleOffscreenBufferLinkTest_ThreeTriangles_SameOB", 6u);
        //modification of 1st provider gets visible
        testResultValue &= testFramework.renderAndCompareScreenshot("InterruptibleOffscreenBufferLinkTest_TrianglesReordered_SameOB");
        break;
    }
    case InterruptibleOffscreenBufferLinkTest_RerendersSceneIfItGetsModifiedWhileAnotherSceneIsBeingRendered_SameBuffer:
    {
        const ramses::sceneId_t m_sceneIdProvider = testFramework.createAndShowScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES);
        const ramses::sceneId_t m_sceneIdProvider2 = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_PROVIDER_LARGE, m_cameraMid + Vector3{ 5.0f, 0.0f, 0.0f });
        const ramses::sceneId_t m_sceneIdConsumer = testFramework.createAndShowScene<TextureLinkScene>(TextureLinkScene::DATA_CONSUMER, m_cameraMid);

        const ramses::displayBufferId_t offscreenBuffer = testFramework.createOffscreenBuffer(0, 256, 128, true);
        testFramework.assignSceneToDisplayBuffer(m_sceneIdProvider, offscreenBuffer, 0);
        testFramework.assignSceneToDisplayBuffer(m_sceneIdProvider2, offscreenBuffer, -1);
        testFramework.createBufferDataLink(offscreenBuffer, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);

        testResultValue &= renderAndCompareScreenshot(testFramework, "OffscreenBufferLinkTest_LinkedInterrupted", 4u);

        //modify state of 1st provider (now 2nd provider is being rendered)
        testFramework.getScenesRegistry().setSceneState<MultipleTrianglesScene>(m_sceneIdProvider, MultipleTrianglesScene::TRIANGLES_REORDERED);
        testFramework.getScenesRegistry().getScene(m_sceneIdProvider).flush();

        testResultValue &= renderAndCompareScreenshot(testFramework, "OffscreenBufferLinkTest_LinkedInterrupted", 2u);
        //2nd provider gets visible (has 2 meshes)
        testResultValue &= renderAndCompareScreenshot(testFramework, "InterruptibleOffscreenBufferLinkTest_ThreeTriangles_SameOB", 6u);
        //modification of 1st provider gets visible
        testResultValue &= testFramework.renderAndCompareScreenshot("InterruptibleOffscreenBufferLinkTest_TrianglesReordered_SameOB");
        break;
    }

    default:
        assert(false && "undefined test case");
    }

    // Restore the default value for time budget checks
    ramses_internal::RenderExecutor::NumRenderablesToRenderInBetweenTimeBudgetChecks = ramses_internal::RenderExecutor::DefaultNumRenderablesToRenderInBetweenTimeBudgetChecks;

    return testResultValue;
}

bool InterruptibleOffscreenBufferLinkTests::renderAndCompareScreenshot(RendererTestsFramework& testFramework, const ramses_internal::String& expectedImage, uint32_t numFramesToRender)
{
    bool result = true;

    for (uint32_t i = 0u; i < numFramesToRender; ++i)
        result &= testFramework.renderAndCompareScreenshot(expectedImage);

    return result;
}
