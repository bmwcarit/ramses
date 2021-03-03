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
#include "ramses-renderer-api/WarpingMeshData.h"
#include "DisplayConfigImpl.h"
#include "WarpingMeshDataImpl.h"

using namespace ramses_internal;

void DisplayRenderingTests::setUpTestCases(RendererTestsFramework& testFramework)
{
    ramses::DisplayConfig displayConfig1 = RendererTestUtils::CreateTestDisplayConfig(0);
    displayConfig1.setWindowRectangle(0, 0, DisplayWidth, DisplayHeight);

    ramses::DisplayConfig displayConfig2 = RendererTestUtils::CreateTestDisplayConfig(1);
    displayConfig2.setWindowRectangle(0, 0, DisplayWidth, DisplayHeight);

    ramses::DisplayConfig displayConfig3 = RendererTestUtils::CreateTestDisplayConfig(2);
    displayConfig3.setWindowRectangle(0, 0, DisplayWidth, DisplayHeight);
    displayConfig3.enableWarpingPostEffect();

    ramses::DisplayConfig displayConfigWithoutDepthAndStencil = RendererTestUtils::CreateTestDisplayConfig(3);
    displayConfigWithoutDepthAndStencil.setWindowRectangle(0, 0, ramses_internal::IntegrationScene::DefaultViewportWidth, ramses_internal::IntegrationScene::DefaultViewportHeight);
    ramses::DisplayConfig::setDepthStencilBufferType(displayConfigWithoutDepthAndStencil, ramses::EDepthBufferType_None);

    ramses::DisplayConfig displayConfigWithoutStencil = RendererTestUtils::CreateTestDisplayConfig(4);
    displayConfigWithoutStencil.setWindowRectangle(0, 0, ramses_internal::IntegrationScene::DefaultViewportWidth, ramses_internal::IntegrationScene::DefaultViewportHeight);
    ramses::DisplayConfig::setDepthStencilBufferType(displayConfigWithoutStencil, ramses::EDepthBufferType_Depth);

    testFramework.createTestCase(DisplayRenderingTest_TwoScenes, *this, "DisplayRenderingTest_TwoScenes").m_displayConfigs.push_back(displayConfig1);
    testFramework.createTestCase(DisplayRenderingTest_UnpublishScene, *this, "DisplayRenderingTest_UnpublishScene").m_displayConfigs.push_back(displayConfig1);
    testFramework.createTestCase(DisplayRenderingTest_HideScene, *this, "DisplayRenderingTest_HideScene").m_displayConfigs.push_back(displayConfig1);
    testFramework.createTestCase(DisplayRenderingTest_SceneRenderOrder, *this, "DisplayRenderingTest_SceneRenderOrder").m_displayConfigs.push_back(displayConfig1);
    testFramework.createTestCase(DisplayRenderingTest_SceneRenderOrderInversed, *this, "DisplayRenderingTest_SceneRenderOrderInversed").m_displayConfigs.push_back(displayConfig1);

    // These tests are using default display config from framework which uses higher resolution than other display tests
    RenderingTestCase& testCaseWarp = testFramework.createTestCaseWithDefaultDisplay(DisplayRenderingTest_Warping, *this, "DisplayRenderingTest_Warping");
    testCaseWarp.m_displayConfigs.front().enableWarpingPostEffect();
    RenderingTestCase& testCaseWarp2 = testFramework.createTestCaseWithDefaultDisplay(DisplayRenderingTest_UpdateWarping, *this, "DisplayRenderingTest_UpdateWarping");
    testCaseWarp2.m_displayConfigs.front().enableWarpingPostEffect();
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

    RenderingTestCase& testCaseText = testFramework.createTestCase(DisplayRenderingTest_RemapSceneWithText, *this, "DisplayRenderingTest_RemapSceneWithText");
    testCaseText.m_displayConfigs.push_back(displayConfig1);
    testCaseText.m_displayConfigs.push_back(displayConfig2);

    RenderingTestCase& testCaseRemapChanged = testFramework.createTestCase(DisplayRenderingTest_RemapSceneWithChangedContent, *this, "DisplayRenderingTest_RemapSceneWithChangedContent");
    testCaseRemapChanged.m_displayConfigs.push_back(displayConfig1);
    testCaseRemapChanged.m_displayConfigs.push_back(displayConfig2);

    RenderingTestCase& testCaseWarped = testFramework.createTestCase(DisplayRenderingTest_RemapSceneToWarpedDisplay, *this, "DisplayRenderingTest_RemapSceneToWarpedDisplay");
    testCaseWarped.m_displayConfigs.push_back(displayConfig1);
    testCaseWarped.m_displayConfigs.push_back(displayConfig3);

    testFramework.createTestCase(DisplayRenderingTest_ResubscribeScene, *this, "DisplayRenderingTest_ResubscribeScene").m_displayConfigs.push_back(displayConfig1);

    testFramework.createTestCase(DisplayRenderingTest_FramebufferWithoutDepthAndStencil, *this, "DisplayRenderingTest_FramebufferWithoutDepthAndStencil").m_displayConfigs.push_back(displayConfigWithoutDepthAndStencil);
    testFramework.createTestCase(DisplayRenderingTest_FramebufferWithoutStencil , *this, "DisplayRenderingTest_FramebufferWithoutStencil").m_displayConfigs.push_back(displayConfigWithoutStencil);
}

bool DisplayRenderingTests::run(RendererTestsFramework& testFramework, const RenderingTestCase& testCase)
{
    switch (testCase.m_id)
    {
    case DisplayRenderingTest_TwoScenes:
        return runTwoScenesTest(testFramework);
    case DisplayRenderingTest_UnpublishScene:
        return runUnpublishTest(testFramework);
    case DisplayRenderingTest_HideScene:
        return runHideTest(testFramework);
    case DisplayRenderingTest_SceneRenderOrder:
        return runSceneRenderOrderTest(testFramework);
    case DisplayRenderingTest_SceneRenderOrderInversed:
        return runSceneRenderOrderInversedTest(testFramework);
    case DisplayRenderingTest_Warping:
        return runWarpingTest(testFramework);
    case DisplayRenderingTest_UpdateWarping:
        return runUpdateWarpingTest(testFramework);
    case DisplayRenderingTest_Subimage:
        return runSubimageTest(testFramework);
    case DisplayRenderingTest_RemapScene:
        return runRemapSceneTest(testFramework);
    case DisplayRenderingTest_SwapScenes:
        return runSwapScenesTest(testFramework);
    case DisplayRenderingTest_RemapSceneWithRenderTarget:
        return runRemapSceneWithRenderTargetTest(testFramework);
    case DisplayRenderingTest_RemapSceneWithText:
        return runRemapSceneWithTextTest(testFramework);
    case DisplayRenderingTest_RemapSceneWithChangedContent:
        return runRemapSceneWithChangedContentTest(testFramework);
    case DisplayRenderingTest_RemapSceneToWarpedDisplay:
        return runRemapSceneToWarpedDisplayTest(testFramework);
    case DisplayRenderingTest_ResubscribeScene:
        return runResubscribeSceneTest(testFramework);
    case DisplayRenderingTest_FramebufferWithoutDepthAndStencil:
        return runFramebufferWithoutDepthAndStencilTest(testFramework);
    case DisplayRenderingTest_FramebufferWithoutStencil:
        return runFramebufferWithoutStencil(testFramework);
    default:
        assert(!"Invalid renderer test ID!");
        return false;
    }
}

bool DisplayRenderingTests::runTwoScenesTest(RendererTestsFramework& testFramework)
{
    const ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES,
        ramses_internal::Vector3(2.0f, 0.0f, 5.0f), DisplayWidth, DisplayHeight);
    const ramses::sceneId_t otherId = testFramework.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::SUBTRACTIVE_BLENDING,
        ramses_internal::Vector3(-2.5f, -0.5f, 5.0f), DisplayWidth, DisplayHeight);

    testFramework.publishAndFlushScene(sceneId);
    testFramework.publishAndFlushScene(otherId);
    testFramework.getSceneToRendered(sceneId);
    testFramework.getSceneToRendered(otherId);

    return testFramework.renderAndCompareScreenshot("ARendererDisplays_TwoScenes");
}

bool DisplayRenderingTests::runUnpublishTest(RendererTestsFramework& testFramework)
{
    const ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES,
        ramses_internal::Vector3(2.0f, 0.0f, 5.0f), DisplayWidth, DisplayHeight);
    const ramses::sceneId_t otherId = testFramework.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::SUBTRACTIVE_BLENDING,
        ramses_internal::Vector3(-2.5f, -0.5f, 5.0f), DisplayWidth, DisplayHeight);

    testFramework.publishAndFlushScene(sceneId);
    testFramework.publishAndFlushScene(otherId);
    testFramework.getSceneToRendered(sceneId);
    testFramework.getSceneToRendered(otherId);

    bool testResult = testFramework.renderAndCompareScreenshot("ARendererDisplays_TwoScenes");

    testFramework.getScenesRegistry().getScene(otherId).unpublish();
    testResult &= testFramework.renderAndCompareScreenshot("ARendererDisplays_UnpublishedScene");

    return testResult;
}

bool DisplayRenderingTests::runHideTest(RendererTestsFramework& testFramework)
{
    const ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES,
        ramses_internal::Vector3(2.0f, 0.0f, 5.0f), DisplayWidth, DisplayHeight);
    const ramses::sceneId_t otherId = testFramework.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::SUBTRACTIVE_BLENDING,
        ramses_internal::Vector3(-2.5f, -0.5f, 5.0f), DisplayWidth, DisplayHeight);

    testFramework.publishAndFlushScene(sceneId);
    testFramework.publishAndFlushScene(otherId);
    testFramework.getSceneToRendered(sceneId);
    testFramework.getSceneToRendered(otherId);

    bool testResult = testFramework.renderAndCompareScreenshot("ARendererDisplays_TwoScenes");

    testFramework.getSceneToState(sceneId, ramses::RendererSceneState::Ready);
    testResult &= testFramework.renderAndCompareScreenshot("ARendererDisplays_HiddenScene");

    return testResult;
}

bool DisplayRenderingTests::runSceneRenderOrderTest(RendererTestsFramework& testFramework)
{
    const ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::ALPHA_BLENDING,
        ramses_internal::Vector3(0.5f, 0.5f, 5.0f), DisplayWidth, DisplayHeight);
    const ramses::sceneId_t otherId = testFramework.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES,
        ramses_internal::Vector3(-0.5f, 0.0f, 5.0f), DisplayWidth, DisplayHeight);

    testFramework.publishAndFlushScene(sceneId);
    testFramework.publishAndFlushScene(otherId);
    testFramework.getSceneToRendered(sceneId);
    testFramework.getSceneToRendered(otherId);
    testFramework.assignSceneToDisplayBuffer(sceneId, testFramework.getDisplayFramebufferId(0u), 1);
    testFramework.assignSceneToDisplayBuffer(otherId, testFramework.getDisplayFramebufferId(0u), 2);

    return testFramework.renderAndCompareScreenshot("ARendererDisplays_TwoScenesOrdered");
}

bool DisplayRenderingTests::runSceneRenderOrderInversedTest(RendererTestsFramework& testFramework)
{
    const ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::ALPHA_BLENDING,
        ramses_internal::Vector3(0.5f, 0.5f, 5.0f), DisplayWidth, DisplayHeight);
    const ramses::sceneId_t otherId = testFramework.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES,
        ramses_internal::Vector3(-0.5f, 0.0f, 5.0f), DisplayWidth, DisplayHeight);

    testFramework.publishAndFlushScene(sceneId);
    testFramework.publishAndFlushScene(otherId);
    testFramework.getSceneToRendered(sceneId);
    testFramework.getSceneToRendered(otherId);
    testFramework.assignSceneToDisplayBuffer(sceneId, testFramework.getDisplayFramebufferId(0u), 2);
    testFramework.assignSceneToDisplayBuffer(otherId, testFramework.getDisplayFramebufferId(0u), 1);

    return testFramework.renderAndCompareScreenshot("ARendererDisplays_TwoScenesInverseOrdered");
}

bool DisplayRenderingTests::runWarpingTest(RendererTestsFramework& testFramework)
{
    const ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<ramses_internal::RenderTargetScene>(ramses_internal::RenderTargetScene::ORTHOGRAPHIC_PROJECTION,
        ramses_internal::Vector3(0.0f, 0.2f, 2.0f));
    testFramework.publishAndFlushScene(sceneId);
    testFramework.getSceneToRendered(sceneId);
    return testFramework.renderAndCompareScreenshot("RenderTargetScene_Warping");
}

bool DisplayRenderingTests::runUpdateWarpingTest(RendererTestsFramework& testFramework)
{
    const ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<ramses_internal::RenderTargetScene>(ramses_internal::RenderTargetScene::ORTHOGRAPHIC_PROJECTION,
        ramses_internal::Vector3(0.0f, 0.2f, 2.0f));
    testFramework.publishAndFlushScene(sceneId);
    testFramework.getSceneToRendered(sceneId);
    if (!testFramework.renderAndCompareScreenshot("RenderTargetScene_Warping"))
        return false;

    const ramses::WarpingMeshData& defaultWarpMesh = RendererTestUtils::CreateTestWarpingMesh();
    const ramses_internal::WarpingMeshData& defaultWarpData = defaultWarpMesh.impl.getWarpingMeshData();
    const auto& texCoords = defaultWarpData.getTextureCoordinates();
    std::vector<float> updatedTexCoords(texCoords.size() * 2);
    auto it = updatedTexCoords.begin();
    for (const auto& texCoord : texCoords)
    {
        *it = texCoord.x * 0.75f;
        ++it;
        *it = texCoord.y * 0.75f;
        ++it;
    }
    const ramses::WarpingMeshData updatedWarpMesh(
        static_cast<UInt32>(defaultWarpData.getIndices().size()), defaultWarpData.getIndices().data(),
        static_cast<UInt32>(defaultWarpData.getVertexPositions().size()), static_cast<const float*>(&defaultWarpData.getVertexPositions().front().x), updatedTexCoords.data());

    testFramework.setWarpingMeshData(updatedWarpMesh);
    if (!testFramework.renderAndCompareScreenshot("RenderTargetScene_Warping2"))
        return false;

    testFramework.setWarpingMeshData(defaultWarpMesh);
    return testFramework.renderAndCompareScreenshot("RenderTargetScene_Warping");
}

bool DisplayRenderingTests::runSubimageTest(RendererTestsFramework& testFramework)
{
    const ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::SUBIMAGES);
    testFramework.publishAndFlushScene(sceneId);
    testFramework.getSceneToRendered(sceneId);
    return testFramework.renderAndCompareScreenshotSubimage("MultipleTrianglesScene_Subimages_middle", 89u, 69u, 30u, 102u);
}

bool DisplayRenderingTests::runRemapSceneTest(RendererTestsFramework& testFramework)
{
    const ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES,
        ramses_internal::Vector3(0.0f, 0.0f, 5.0f), DisplayWidth, DisplayHeight);
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

bool DisplayRenderingTests::runSwapScenesTest(RendererTestsFramework& testFramework)
{
    const ramses::sceneId_t sceneId1 = testFramework.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES,
        ramses_internal::Vector3(0.0f, 0.0f, 5.0f), DisplayWidth, DisplayHeight);
    const ramses::sceneId_t sceneId2 = testFramework.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::TRIANGLES_REORDERED,
        ramses_internal::Vector3(0.0f, 0.0f, 5.0f), DisplayWidth, DisplayHeight);

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

bool DisplayRenderingTests::runRemapSceneWithRenderTargetTest(RendererTestsFramework& testFramework)
{
    const ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<ramses_internal::RenderTargetScene>(ramses_internal::RenderTargetScene::PERSPECTIVE_PROJECTION,
        ramses_internal::Vector3(0.0f, 0.0f, 5.0f), DisplayWidth, DisplayHeight);
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

bool DisplayRenderingTests::runRemapSceneWithTextTest(RendererTestsFramework& testFramework)
{
    const ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<ramses_internal::TextScene>(ramses_internal::TextScene::EState_INITIAL_128_BY_64_VIEWPORT,
        ramses_internal::Vector3(0.0f, 0.0f, 5.0f), DisplayWidth, DisplayHeight);
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

bool DisplayRenderingTests::runRemapSceneToWarpedDisplayTest(RendererTestsFramework& testFramework)
{
    const ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<ramses_internal::RenderTargetScene>(ramses_internal::RenderTargetScene::PERSPECTIVE_PROJECTION,
        ramses_internal::Vector3(0.0f, 0.0f, 5.0f), DisplayWidth, DisplayHeight);
    testFramework.publishAndFlushScene(sceneId);
    testFramework.getSceneToRendered(sceneId);
    bool testResult = testFramework.renderAndCompareScreenshot("ARendererDisplays_RenderTarget", 0u);
    testResult &= testFramework.renderAndCompareScreenshot("ARendererDisplays_Black", 1u);

    testFramework.getSceneToState(sceneId, ramses::RendererSceneState::Available);
    testFramework.setSceneMapping(sceneId, 1u);
    testFramework.getSceneToState(sceneId, ramses::RendererSceneState::Rendered);
    testResult &= testFramework.renderAndCompareScreenshot("ARendererDisplays_Black", 0u);
    testResult &= testFramework.renderAndCompareScreenshot("ARendererDisplays_Warped", 1u);

    return testResult;
}

bool DisplayRenderingTests::runRemapSceneWithChangedContentTest(RendererTestsFramework& testFramework)
{
    const ramses::sceneId_t sceneId1 = testFramework.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES,
        ramses_internal::Vector3(2.0f, 0.0f, 5.0f), DisplayWidth, DisplayHeight);
    const ramses::sceneId_t sceneId2 = testFramework.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::TRIANGLES_REORDERED,
        ramses_internal::Vector3(-2.5f, -0.5f, 5.0f), DisplayWidth, DisplayHeight);

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
    testFramework.getScenesRegistry().setSceneState<ramses_internal::MultipleTrianglesScene>(sceneId1, ramses_internal::MultipleTrianglesScene::TRIANGLES_REORDERED);
    testFramework.getScenesRegistry().setSceneState<ramses_internal::MultipleTrianglesScene>(sceneId2, ramses_internal::MultipleTrianglesScene::ADDITIVE_BLENDING);
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

bool DisplayRenderingTests::runResubscribeSceneTest(RendererTestsFramework& testFramework)
{
    const ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES,
        ramses_internal::Vector3(0.0f, 0.0f, 5.0f), DisplayWidth, DisplayHeight);
    testFramework.publishAndFlushScene(sceneId);
    testFramework.getSceneToRendered(sceneId);
    bool testResult = testFramework.renderAndCompareScreenshot("ARendererInstance_Three_Triangles", 0u);

    testFramework.getSceneToState(sceneId, ramses::RendererSceneState::Available);
    testResult &= testFramework.renderAndCompareScreenshot("ARendererDisplays_Black", 0u);

    testFramework.getSceneToState(sceneId, ramses::RendererSceneState::Rendered);
    testResult &= testFramework.renderAndCompareScreenshot("ARendererInstance_Three_Triangles", 0u);

    return testResult;
}

bool DisplayRenderingTests::runFramebufferWithoutDepthAndStencilTest(RendererTestsFramework& testFramework)
{
    ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::DEPTH_FUNC);
    testFramework.publishAndFlushScene(sceneId);
    testFramework.getSceneToRendered(sceneId);
    bool testResult = testFramework.renderAndCompareScreenshot("MultipleTrianglesScene_DepthFunc_NoDepthBuffer");

    testFramework.getScenesRegistry().getScene(sceneId).unpublish();

    sceneId = testFramework.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::STENCIL_TEST_1);
    testFramework.publishAndFlushScene(sceneId);
    testFramework.getSceneToRendered(sceneId);
    testResult &= testFramework.renderAndCompareScreenshot("MultipleTrianglesScene_StencilTest_1_NoDepthOrStencilBuffer");

    return testResult;
}

bool DisplayRenderingTests::runFramebufferWithoutStencil(RendererTestsFramework& testFramework)
{
    ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::DEPTH_FUNC);
    testFramework.publishAndFlushScene(sceneId);
    testFramework.getSceneToRendered(sceneId);
    bool testResult = testFramework.renderAndCompareScreenshot("MultipleTrianglesScene_DepthFunc");

    testFramework.getScenesRegistry().getScene(sceneId).unpublish();

    sceneId = testFramework.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::STENCIL_TEST_1);
    testFramework.publishAndFlushScene(sceneId);
    testFramework.getSceneToRendered(sceneId);
    testResult &= testFramework.renderAndCompareScreenshot("MultipleTrianglesScene_StencilTest_1_NoStencilBuffer");

    return testResult;
}
