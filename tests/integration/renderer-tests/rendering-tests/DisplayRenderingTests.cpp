//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DisplayRenderingTests.h"
#include "TestScenes/MultipleTrianglesScene.h"
#include "TestScenes/RenderTargetScene.h"
#include "TestScenes/TextScene.h"
#include "RendererTestUtils.h"
#include "impl/DisplayConfigImpl.h"

namespace ramses::internal
{
    void DisplayRenderingTests::setUpTestCases(RendererTestsFramework& testFramework)
    {
        ramses::DisplayConfig displayConfig1 = RendererTestUtils::CreateTestDisplayConfig(0);
        displayConfig1.setWindowRectangle(0, 0, DisplayWidth, DisplayHeight);

        ramses::DisplayConfig displayConfig2 = RendererTestUtils::CreateTestDisplayConfig(1);
        displayConfig2.setWindowRectangle(0, 0, DisplayWidth, DisplayHeight);

        ramses::DisplayConfig displayConfigWithoutDepthAndStencil = RendererTestUtils::CreateTestDisplayConfig(3);
        displayConfigWithoutDepthAndStencil.setWindowRectangle(0, 0, ramses::internal::IntegrationScene::DefaultViewportWidth, ramses::internal::IntegrationScene::DefaultViewportHeight);
        displayConfigWithoutDepthAndStencil.setDepthStencilBufferType(ramses::EDepthBufferType::None);

        ramses::DisplayConfig displayConfigWithoutStencil = RendererTestUtils::CreateTestDisplayConfig(4);
        displayConfigWithoutStencil.setWindowRectangle(0, 0, ramses::internal::IntegrationScene::DefaultViewportWidth, ramses::internal::IntegrationScene::DefaultViewportHeight);
        displayConfigWithoutStencil.setDepthStencilBufferType(ramses::EDepthBufferType::Depth);

        ramses::DisplayConfig displayConfigWithoutAsyncEffectUpload = RendererTestUtils::CreateTestDisplayConfig(3);
        displayConfigWithoutAsyncEffectUpload.setWindowRectangle(0, 0, DisplayWidth, DisplayHeight);
        displayConfigWithoutAsyncEffectUpload.setAsyncEffectUploadEnabled(false);

        testFramework.createTestCase(DisplayRenderingTest_TwoScenes, *this, "DisplayRenderingTest_TwoScenes").m_displayConfigs.push_back(displayConfig1);
        testFramework.createTestCase(DisplayRenderingTest_UnpublishScene, *this, "DisplayRenderingTest_UnpublishScene").m_displayConfigs.push_back(displayConfig1);
        testFramework.createTestCase(DisplayRenderingTest_HideScene, *this, "DisplayRenderingTest_HideScene").m_displayConfigs.push_back(displayConfig1);
        testFramework.createTestCase(DisplayRenderingTest_SceneRenderOrder, *this, "DisplayRenderingTest_SceneRenderOrder").m_displayConfigs.push_back(displayConfig1);
        testFramework.createTestCase(DisplayRenderingTest_SceneRenderOrderInversed, *this, "DisplayRenderingTest_SceneRenderOrderInversed").m_displayConfigs.push_back(displayConfig1);

        // These tests are using default display config from framework which uses higher resolution than other display tests
        testFramework.createTestCaseWithDefaultDisplay(DisplayRenderingTest_Subimage, *this, "DisplayRenderingTest_Subimage");

        RenderingTestCase& testCaseRemap = testFramework.createTestCase(DisplayRenderingTest_RemapScene, *this, "DisplayRenderingTest_RemapScene");
        testCaseRemap.m_displayConfigs.push_back(displayConfig1);
        testCaseRemap.m_displayConfigs.push_back(displayConfig2);

        RenderingTestCase& testCaseSwap = testFramework.createTestCase(DisplayRenderingTest_SwapScenes, *this, "DisplayRenderingTest_SwapScenes");
        testCaseSwap.m_displayConfigs.push_back(displayConfig1);
        testCaseSwap.m_displayConfigs.push_back(displayConfig2);

        RenderingTestCase& testCaseRenderTarget = testFramework.createTestCase(DisplayRenderingTest_RemapSceneWithRenderTarget, *this, "DisplayRenderingTest_RemapSceneWithRenderTarget");
        testCaseRenderTarget.m_displayConfigs.push_back(displayConfig1);
        testCaseRenderTarget.m_displayConfigs.push_back(displayConfig2);

#if defined(RAMSES_TEXT_ENABLED)
        RenderingTestCase& testCaseText = testFramework.createTestCase(DisplayRenderingTest_RemapSceneWithText, *this, "DisplayRenderingTest_RemapSceneWithText");
        testCaseText.m_displayConfigs.push_back(displayConfig1);
        testCaseText.m_displayConfigs.push_back(displayConfig2);
#endif

        RenderingTestCase& testCaseRemapChanged = testFramework.createTestCase(DisplayRenderingTest_RemapSceneWithChangedContent, *this, "DisplayRenderingTest_RemapSceneWithChangedContent");
        testCaseRemapChanged.m_displayConfigs.push_back(displayConfig1);
        testCaseRemapChanged.m_displayConfigs.push_back(displayConfig2);

        testFramework.createTestCase(DisplayRenderingTest_ResubscribeScene, *this, "DisplayRenderingTest_ResubscribeScene").m_displayConfigs.push_back(displayConfig1);

        testFramework.createTestCase(DisplayRenderingTest_FramebufferWithoutDepthAndStencil, *this, "DisplayRenderingTest_FramebufferWithoutDepthAndStencil").m_displayConfigs.push_back(displayConfigWithoutDepthAndStencil);
        testFramework.createTestCase(DisplayRenderingTest_FramebufferWithoutStencil , *this, "DisplayRenderingTest_FramebufferWithoutStencil").m_displayConfigs.push_back(displayConfigWithoutStencil);

        testFramework.createTestCase(DisplayRenderingTest_AsyncEffectUploadDisabled, *this, "DisplayRenderingTest_AsyncEffectUploadDisabled").m_displayConfigs.push_back(displayConfigWithoutAsyncEffectUpload);
    }

    bool DisplayRenderingTests::run(RendererTestsFramework& testFramework, const RenderingTestCase& testCase)
    {
        switch (testCase.m_id)
        {
        case DisplayRenderingTest_AsyncEffectUploadDisabled:
        case DisplayRenderingTest_TwoScenes:
            return RunTwoScenesTest(testFramework);
        case DisplayRenderingTest_UnpublishScene:
            return RunUnpublishTest(testFramework);
        case DisplayRenderingTest_HideScene:
            return RunHideTest(testFramework);
        case DisplayRenderingTest_SceneRenderOrder:
            return RunSceneRenderOrderTest(testFramework);
        case DisplayRenderingTest_SceneRenderOrderInversed:
            return RunSceneRenderOrderInversedTest(testFramework);
        case DisplayRenderingTest_Subimage:
            return RunSubimageTest(testFramework);
        case DisplayRenderingTest_RemapScene:
            return RunRemapSceneTest(testFramework);
        case DisplayRenderingTest_SwapScenes:
            return RunSwapScenesTest(testFramework);
        case DisplayRenderingTest_RemapSceneWithRenderTarget:
            return RunRemapSceneWithRenderTargetTest(testFramework);
#if defined(RAMSES_TEXT_ENABLED)
        case DisplayRenderingTest_RemapSceneWithText:
            return RunRemapSceneWithTextTest(testFramework);
#endif
        case DisplayRenderingTest_RemapSceneWithChangedContent:
            return RunRemapSceneWithChangedContentTest(testFramework);
        case DisplayRenderingTest_ResubscribeScene:
            return RunResubscribeSceneTest(testFramework);
        case DisplayRenderingTest_FramebufferWithoutDepthAndStencil:
            return RunFramebufferWithoutDepthAndStencilTest(testFramework);
        case DisplayRenderingTest_FramebufferWithoutStencil:
            return RunFramebufferWithoutStencil(testFramework);
        default:
            assert(!"Invalid renderer test ID!");
            return false;
        }
    }

    bool DisplayRenderingTests::RunTwoScenesTest(RendererTestsFramework& testFramework)
    {
        const ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<ramses::internal::MultipleTrianglesScene>(ramses::internal::MultipleTrianglesScene::THREE_TRIANGLES,
            glm::vec3(2.0f, 0.0f, 5.0f), DisplayWidth, DisplayHeight);
        const ramses::sceneId_t otherId = testFramework.getScenesRegistry().createScene<ramses::internal::MultipleTrianglesScene>(ramses::internal::MultipleTrianglesScene::SUBTRACTIVE_BLENDING,
            glm::vec3(-2.5f, -0.5f, 5.0f), DisplayWidth, DisplayHeight);

        testFramework.publishAndFlushScene(sceneId);
        testFramework.publishAndFlushScene(otherId);
        testFramework.getSceneToRendered(sceneId);
        testFramework.getSceneToRendered(otherId);

        return testFramework.renderAndCompareScreenshot("ARendererDisplays_TwoScenes");
    }

    bool DisplayRenderingTests::RunUnpublishTest(RendererTestsFramework& testFramework)
    {
        const ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<ramses::internal::MultipleTrianglesScene>(ramses::internal::MultipleTrianglesScene::THREE_TRIANGLES,
            glm::vec3(2.0f, 0.0f, 5.0f), DisplayWidth, DisplayHeight);
        const ramses::sceneId_t otherId = testFramework.getScenesRegistry().createScene<ramses::internal::MultipleTrianglesScene>(ramses::internal::MultipleTrianglesScene::SUBTRACTIVE_BLENDING,
            glm::vec3(-2.5f, -0.5f, 5.0f), DisplayWidth, DisplayHeight);

        testFramework.publishAndFlushScene(sceneId);
        testFramework.publishAndFlushScene(otherId);
        testFramework.getSceneToRendered(sceneId);
        testFramework.getSceneToRendered(otherId);

        bool testResult = testFramework.renderAndCompareScreenshot("ARendererDisplays_TwoScenes");

        testFramework.getScenesRegistry().getScene(otherId).unpublish();
        testResult &= testFramework.renderAndCompareScreenshot("ARendererDisplays_UnpublishedScene");

        return testResult;
    }

    bool DisplayRenderingTests::RunHideTest(RendererTestsFramework& testFramework)
    {
        const ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<ramses::internal::MultipleTrianglesScene>(ramses::internal::MultipleTrianglesScene::THREE_TRIANGLES,
            glm::vec3(2.0f, 0.0f, 5.0f), DisplayWidth, DisplayHeight);
        const ramses::sceneId_t otherId = testFramework.getScenesRegistry().createScene<ramses::internal::MultipleTrianglesScene>(ramses::internal::MultipleTrianglesScene::SUBTRACTIVE_BLENDING,
            glm::vec3(-2.5f, -0.5f, 5.0f), DisplayWidth, DisplayHeight);

        testFramework.publishAndFlushScene(sceneId);
        testFramework.publishAndFlushScene(otherId);
        testFramework.getSceneToRendered(sceneId);
        testFramework.getSceneToRendered(otherId);

        bool testResult = testFramework.renderAndCompareScreenshot("ARendererDisplays_TwoScenes");

        testFramework.getSceneToState(sceneId, ramses::RendererSceneState::Ready);
        testResult &= testFramework.renderAndCompareScreenshot("ARendererDisplays_HiddenScene");

        return testResult;
    }

    bool DisplayRenderingTests::RunSceneRenderOrderTest(RendererTestsFramework& testFramework)
    {
        const ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<ramses::internal::MultipleTrianglesScene>(ramses::internal::MultipleTrianglesScene::ALPHA_BLENDING,
            glm::vec3(0.5f, 0.5f, 5.0f), DisplayWidth, DisplayHeight);
        const ramses::sceneId_t otherId = testFramework.getScenesRegistry().createScene<ramses::internal::MultipleTrianglesScene>(ramses::internal::MultipleTrianglesScene::THREE_TRIANGLES,
            glm::vec3(-0.5f, 0.0f, 5.0f), DisplayWidth, DisplayHeight);

        testFramework.publishAndFlushScene(sceneId);
        testFramework.publishAndFlushScene(otherId);
        testFramework.getSceneToRendered(sceneId);
        testFramework.getSceneToRendered(otherId);
        testFramework.assignSceneToDisplayBuffer(sceneId, testFramework.getDisplayFramebufferId(0u), 1);
        testFramework.assignSceneToDisplayBuffer(otherId, testFramework.getDisplayFramebufferId(0u), 2);

        return testFramework.renderAndCompareScreenshot("ARendererDisplays_TwoScenesOrdered");
    }

    bool DisplayRenderingTests::RunSceneRenderOrderInversedTest(RendererTestsFramework& testFramework)
    {
        const ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<ramses::internal::MultipleTrianglesScene>(ramses::internal::MultipleTrianglesScene::ALPHA_BLENDING,
            glm::vec3(0.5f, 0.5f, 5.0f), DisplayWidth, DisplayHeight);
        const ramses::sceneId_t otherId = testFramework.getScenesRegistry().createScene<ramses::internal::MultipleTrianglesScene>(ramses::internal::MultipleTrianglesScene::THREE_TRIANGLES,
            glm::vec3(-0.5f, 0.0f, 5.0f), DisplayWidth, DisplayHeight);

        testFramework.publishAndFlushScene(sceneId);
        testFramework.publishAndFlushScene(otherId);
        testFramework.getSceneToRendered(sceneId);
        testFramework.getSceneToRendered(otherId);
        testFramework.assignSceneToDisplayBuffer(sceneId, testFramework.getDisplayFramebufferId(0u), 2);
        testFramework.assignSceneToDisplayBuffer(otherId, testFramework.getDisplayFramebufferId(0u), 1);

        return testFramework.renderAndCompareScreenshot("ARendererDisplays_TwoScenesInverseOrdered");
    }

    bool DisplayRenderingTests::RunSubimageTest(RendererTestsFramework& testFramework)
    {
        const ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<ramses::internal::MultipleTrianglesScene>(ramses::internal::MultipleTrianglesScene::SUBIMAGES);
        testFramework.publishAndFlushScene(sceneId);
        testFramework.getSceneToRendered(sceneId);
        return testFramework.renderAndCompareScreenshotSubimage("MultipleTrianglesScene_Subimages_middle", 89u, 69u, 30u, 102u);
    }

    bool DisplayRenderingTests::RunRemapSceneTest(RendererTestsFramework& testFramework)
    {
        const ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<ramses::internal::MultipleTrianglesScene>(ramses::internal::MultipleTrianglesScene::THREE_TRIANGLES,
            glm::vec3(0.0f, 0.0f, 5.0f), DisplayWidth, DisplayHeight);
        testFramework.publishAndFlushScene(sceneId);
        testFramework.getSceneToRendered(sceneId, 0u);
        bool testResult = testFramework.renderAndCompareScreenshot("ARendererInstance_Three_Triangles", 0u);
        testResult &= testFramework.renderAndCompareScreenshot("ARendererDisplays_Black", 1u);

        testFramework.getSceneToState(sceneId, ramses::RendererSceneState::Available);
        testFramework.setSceneMapping(sceneId, 1u);
        testFramework.getSceneToState(sceneId, ramses::RendererSceneState::Rendered);

        testResult &= testFramework.renderAndCompareScreenshot("ARendererDisplays_Black", 0u);
        testResult &= testFramework.renderAndCompareScreenshot("ARendererInstance_Three_Triangles", 1u);

        return testResult;
    }

    bool DisplayRenderingTests::RunSwapScenesTest(RendererTestsFramework& testFramework)
    {
        const ramses::sceneId_t sceneId1 = testFramework.getScenesRegistry().createScene<ramses::internal::MultipleTrianglesScene>(ramses::internal::MultipleTrianglesScene::THREE_TRIANGLES,
            glm::vec3(0.0f, 0.0f, 5.0f), DisplayWidth, DisplayHeight);
        const ramses::sceneId_t sceneId2 = testFramework.getScenesRegistry().createScene<ramses::internal::MultipleTrianglesScene>(ramses::internal::MultipleTrianglesScene::TRIANGLES_REORDERED,
            glm::vec3(0.0f, 0.0f, 5.0f), DisplayWidth, DisplayHeight);

        testFramework.publishAndFlushScene(sceneId1);
        testFramework.publishAndFlushScene(sceneId2);
        testFramework.setSceneMapping(sceneId1, 0u);
        testFramework.setSceneMapping(sceneId2, 1u);
        testFramework.getSceneToState(sceneId1, ramses::RendererSceneState::Rendered);
        testFramework.getSceneToState(sceneId2, ramses::RendererSceneState::Rendered);

        bool testResult = testFramework.renderAndCompareScreenshot("ARendererInstance_Three_Triangles", 0u);
        testResult &= testFramework.renderAndCompareScreenshot("ARendererInstance_Triangles_reordered", 1u);

        testFramework.getSceneToState(sceneId1, ramses::RendererSceneState::Available);
        testFramework.getSceneToState(sceneId2, ramses::RendererSceneState::Available);
        testFramework.setSceneMapping(sceneId1, 1u);
        testFramework.setSceneMapping(sceneId2, 0u);
        testFramework.getSceneToState(sceneId1, ramses::RendererSceneState::Rendered);
        testFramework.getSceneToState(sceneId2, ramses::RendererSceneState::Rendered);

        testResult &= testFramework.renderAndCompareScreenshot("ARendererInstance_Triangles_reordered", 0u);
        testResult &= testFramework.renderAndCompareScreenshot("ARendererInstance_Three_Triangles", 1u);

        return testResult;
    }

    bool DisplayRenderingTests::RunRemapSceneWithRenderTargetTest(RendererTestsFramework& testFramework)
    {
        const ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<ramses::internal::RenderTargetScene>(ramses::internal::RenderTargetScene::PERSPECTIVE_PROJECTION,
            glm::vec3(0.0f, 0.0f, 5.0f), DisplayWidth, DisplayHeight);
        testFramework.publishAndFlushScene(sceneId);
        testFramework.getSceneToRendered(sceneId, 0u);
        bool testResult = testFramework.renderAndCompareScreenshot("ARendererDisplays_RenderTarget", 0u);
        testResult &= testFramework.renderAndCompareScreenshot("ARendererDisplays_Black", 1u);

        testFramework.getSceneToState(sceneId, ramses::RendererSceneState::Available);
        testFramework.setSceneMapping(sceneId, 1u);
        testFramework.getSceneToState(sceneId, ramses::RendererSceneState::Rendered);

        testResult &= testFramework.renderAndCompareScreenshot("ARendererDisplays_Black", 0u);
        testResult &= testFramework.renderAndCompareScreenshot("ARendererDisplays_RenderTarget", 1u);

        return testResult;
    }

#if defined(RAMSES_TEXT_ENABLED)
    bool DisplayRenderingTests::RunRemapSceneWithTextTest(RendererTestsFramework& testFramework)
    {
        const ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<ramses::internal::TextScene>(ramses::internal::TextScene::EState_INITIAL_128_BY_64_VIEWPORT,
            glm::vec3(0.0f, 0.0f, 5.0f), DisplayWidth, DisplayHeight);
        testFramework.publishAndFlushScene(sceneId);
        testFramework.getSceneToRendered(sceneId);
        bool testResult = testFramework.renderAndCompareScreenshot("ARendererInstance_SimpleText", 0u);
        testResult &= testFramework.renderAndCompareScreenshot("ARendererDisplays_Black", 1u);

        testFramework.getSceneToState(sceneId, ramses::RendererSceneState::Available);
        testFramework.setSceneMapping(sceneId, 1u);
        testFramework.getSceneToState(sceneId, ramses::RendererSceneState::Rendered);
        testResult &= testFramework.renderAndCompareScreenshot("ARendererDisplays_Black", 0u);
        testResult &= testFramework.renderAndCompareScreenshot("ARendererInstance_SimpleText", 1u);

        return testResult;
    }
#endif

    bool DisplayRenderingTests::RunRemapSceneWithChangedContentTest(RendererTestsFramework& testFramework)
    {
        const ramses::sceneId_t sceneId1 = testFramework.getScenesRegistry().createScene<ramses::internal::MultipleTrianglesScene>(ramses::internal::MultipleTrianglesScene::THREE_TRIANGLES,
            glm::vec3(2.0f, 0.0f, 5.0f), DisplayWidth, DisplayHeight);
        const ramses::sceneId_t sceneId2 = testFramework.getScenesRegistry().createScene<ramses::internal::MultipleTrianglesScene>(ramses::internal::MultipleTrianglesScene::TRIANGLES_REORDERED,
            glm::vec3(-2.5f, -0.5f, 5.0f), DisplayWidth, DisplayHeight);

        testFramework.publishAndFlushScene(sceneId1);
        testFramework.publishAndFlushScene(sceneId2);
        testFramework.getSceneToRendered(sceneId1, 0u);
        testFramework.getSceneToRendered(sceneId2, 0u);

        bool testResult = testFramework.renderAndCompareScreenshot("ARendererDisplays_ModifiedScenes1", 0u);
        testResult &= testFramework.renderAndCompareScreenshot("ARendererDisplays_Black", 1u);

        // unmap scenes
        testFramework.getSceneToState(sceneId1, ramses::RendererSceneState::Available);
        testFramework.getSceneToState(sceneId2, ramses::RendererSceneState::Available);

        // modify scenes
        testFramework.getScenesRegistry().setSceneState<ramses::internal::MultipleTrianglesScene>(sceneId1, ramses::internal::MultipleTrianglesScene::TRIANGLES_REORDERED);
        testFramework.getScenesRegistry().setSceneState<ramses::internal::MultipleTrianglesScene>(sceneId2, ramses::internal::MultipleTrianglesScene::ADDITIVE_BLENDING);
        testFramework.getScenesRegistry().getScene(sceneId1).flush();
        testFramework.getScenesRegistry().getScene(sceneId2).flush();

        testResult &= testFramework.renderAndCompareScreenshot("ARendererDisplays_Black", 0u);
        testResult &= testFramework.renderAndCompareScreenshot("ARendererDisplays_Black", 1u);

        // map scenes to second display
        testFramework.setSceneMapping(sceneId1, 1u);
        testFramework.setSceneMapping(sceneId2, 1u);
        testFramework.getSceneToState(sceneId1, ramses::RendererSceneState::Rendered);
        testFramework.getSceneToState(sceneId2, ramses::RendererSceneState::Rendered);

        testResult &= testFramework.renderAndCompareScreenshot("ARendererDisplays_Black", 0u);
        testResult &= testFramework.renderAndCompareScreenshot("ARendererDisplays_ModifiedScenes2", 1u);

        return testResult;
    }

    bool DisplayRenderingTests::RunResubscribeSceneTest(RendererTestsFramework& testFramework)
    {
        const ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<ramses::internal::MultipleTrianglesScene>(ramses::internal::MultipleTrianglesScene::THREE_TRIANGLES,
            glm::vec3(0.0f, 0.0f, 5.0f), DisplayWidth, DisplayHeight);
        testFramework.publishAndFlushScene(sceneId);
        testFramework.getSceneToRendered(sceneId);
        bool testResult = testFramework.renderAndCompareScreenshot("ARendererInstance_Three_Triangles", 0u);

        testFramework.getSceneToState(sceneId, ramses::RendererSceneState::Available);
        testResult &= testFramework.renderAndCompareScreenshot("ARendererDisplays_Black", 0u);

        testFramework.getSceneToState(sceneId, ramses::RendererSceneState::Rendered);
        testResult &= testFramework.renderAndCompareScreenshot("ARendererInstance_Three_Triangles", 0u);

        return testResult;
    }

    bool DisplayRenderingTests::RunFramebufferWithoutDepthAndStencilTest(RendererTestsFramework& testFramework)
    {
        ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<ramses::internal::MultipleTrianglesScene>(ramses::internal::MultipleTrianglesScene::DEPTH_FUNC);
        testFramework.publishAndFlushScene(sceneId);
        testFramework.getSceneToRendered(sceneId);
        bool testResult = testFramework.renderAndCompareScreenshot("MultipleTrianglesScene_DepthFunc_NoDepthBuffer");

        testFramework.getScenesRegistry().getScene(sceneId).unpublish();

        sceneId = testFramework.getScenesRegistry().createScene<ramses::internal::MultipleTrianglesScene>(ramses::internal::MultipleTrianglesScene::STENCIL_TEST_1);
        testFramework.publishAndFlushScene(sceneId);
        testFramework.getSceneToRendered(sceneId);
        testResult &= testFramework.renderAndCompareScreenshot("MultipleTrianglesScene_StencilTest_1_NoDepthOrStencilBuffer");

        return testResult;
    }

    bool DisplayRenderingTests::RunFramebufferWithoutStencil(RendererTestsFramework& testFramework)
    {
        ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<ramses::internal::MultipleTrianglesScene>(ramses::internal::MultipleTrianglesScene::DEPTH_FUNC);
        testFramework.publishAndFlushScene(sceneId);
        testFramework.getSceneToRendered(sceneId);
        bool testResult = testFramework.renderAndCompareScreenshot("MultipleTrianglesScene_DepthFunc");

        testFramework.getScenesRegistry().getScene(sceneId).unpublish();

        sceneId = testFramework.getScenesRegistry().createScene<ramses::internal::MultipleTrianglesScene>(ramses::internal::MultipleTrianglesScene::STENCIL_TEST_1);
        testFramework.publishAndFlushScene(sceneId);
        testFramework.getSceneToRendered(sceneId);
        testResult &= testFramework.renderAndCompareScreenshot("MultipleTrianglesScene_StencilTest_1_NoStencilBuffer");

        return testResult;
    }
}
