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
    displayConfig1.setWindowRectangle(0, 0, 128u, 64u);
    displayConfig1.setPerspectiveProjection(19.f, 128.f / 64.f, 0.1f, 1500.f);

    ramses::DisplayConfig displayConfig2 = RendererTestUtils::CreateTestDisplayConfig(1);
    displayConfig2.setWindowRectangle(0, 0, 128u, 64u);
    displayConfig2.setPerspectiveProjection(19.f, 128.f / 64.f, 0.1f, 1500.f);

    ramses::DisplayConfig displayConfig3 = RendererTestUtils::CreateTestDisplayConfig(2);
    displayConfig3.setWindowRectangle(0, 0, 128u, 64u);
    displayConfig3.setPerspectiveProjection(19.f, 128.f / 64.f, 0.1f, 1500.f);
    displayConfig3.enableWarpingPostEffect();

    testFramework.createTestCase(DisplayTest_TwoScenes, *this, "DisplayRendererTest_TwoScenes").m_displayConfigs.push_back(displayConfig1);
    testFramework.createTestCase(DisplayTest_UnpublishScene, *this, "DisplayRendererTest_UnpublishScene").m_displayConfigs.push_back(displayConfig1);
    testFramework.createTestCase(DisplayTest_HideScene, *this, "DisplayRendererTest_HideScene").m_displayConfigs.push_back(displayConfig1);
    testFramework.createTestCase(DisplayTest_SceneRenderOrder, *this, "DisplayRendererTest_SceneRenderOrder").m_displayConfigs.push_back(displayConfig1);
    testFramework.createTestCase(DisplayTest_SceneRenderOrderInversed, *this, "DisplayRendererTest_SceneRenderOrderInversed").m_displayConfigs.push_back(displayConfig1);

    // These tests are using default display config from framework which uses higher resolution than other display tests
    RenderingTestCase& testCaseWarp = testFramework.createTestCaseWithDefaultDisplay(DisplayTest_Warping, *this, "DisplayRendererTest_Warping");
    testCaseWarp.m_displayConfigs.front().enableWarpingPostEffect();
    RenderingTestCase& testCaseWarp2 = testFramework.createTestCaseWithDefaultDisplay(DisplayTest_UpdateWarping, *this, "DisplayRendererTest_UpdateWarping");
    testCaseWarp2.m_displayConfigs.front().enableWarpingPostEffect();
    testFramework.createTestCaseWithDefaultDisplay(DisplayTest_Subimage, *this, "DisplayRendererTest_Subimage");

    RenderingTestCase& testCaseRemap = testFramework.createTestCase(DisplayTest_RemapScene, *this, "DisplayRendererTest_RemapScene");
    testCaseRemap.m_displayConfigs.push_back(displayConfig1);
    testCaseRemap.m_displayConfigs.push_back(displayConfig2);

    RenderingTestCase& testCaseSwap = testFramework.createTestCase(DisplayTest_SwapScenes, *this, "DisplayRendererTest_SwapScenes");
    testCaseSwap.m_displayConfigs.push_back(displayConfig1);
    testCaseSwap.m_displayConfigs.push_back(displayConfig2);

    RenderingTestCase& testCaseRenderTarget = testFramework.createTestCase(DisplayTest_RemapSceneWithRenderTarget, *this, "DisplayRendererTest_RemapSceneWithRenderTarget");
    testCaseRenderTarget.m_displayConfigs.push_back(displayConfig1);
    testCaseRenderTarget.m_displayConfigs.push_back(displayConfig2);

    RenderingTestCase& testCaseText = testFramework.createTestCase(DisplayTest_RemapSceneWithText, *this, "DisplayRendererTest_RemapSceneWithText");
    testCaseText.m_displayConfigs.push_back(displayConfig1);
    testCaseText.m_displayConfigs.push_back(displayConfig2);

    RenderingTestCase& testCaseRemapChanged = testFramework.createTestCase(DisplayTest_RemapSceneWithChangedContent, *this, "DisplayRendererTest_RemapSceneWithChangedContent");
    testCaseRemapChanged.m_displayConfigs.push_back(displayConfig1);
    testCaseRemapChanged.m_displayConfigs.push_back(displayConfig2);

    RenderingTestCase& testCaseWarped = testFramework.createTestCase(DisplayTest_RemapSceneToWarpedDisplay, *this, "DisplayRendererTest_RemapSceneToWarpedDisplay");
    testCaseWarped.m_displayConfigs.push_back(displayConfig1);
    testCaseWarped.m_displayConfigs.push_back(displayConfig3);

    testFramework.createTestCase(DisplayTest_ResubscribeScene, *this, "DisplayTest_ResubscribeScene").m_displayConfigs.push_back(displayConfig1);
}

bool DisplayRenderingTests::run(RendererTestsFramework& testFramework, const RenderingTestCase& testCase)
{
    switch (testCase.m_id)
    {
    case DisplayTest_TwoScenes:
        return runTwoScenesTest(testFramework);
    case DisplayTest_UnpublishScene:
        return runUnpublishTest(testFramework);
    case DisplayTest_HideScene:
        return runHideTest(testFramework);
    case DisplayTest_SceneRenderOrder:
        return runSceneRenderOrderTest(testFramework);
    case DisplayTest_SceneRenderOrderInversed:
        return runSceneRenderOrderInversedTest(testFramework);
    case DisplayTest_Warping:
        return runWarpingTest(testFramework);
    case DisplayTest_UpdateWarping:
        return runUpdateWarpingTest(testFramework);
    case DisplayTest_Subimage:
        return runSubimageTest(testFramework);
    case DisplayTest_RemapScene:
        return runRemapSceneTest(testFramework);
    case DisplayTest_SwapScenes:
        return runSwapScenesTest(testFramework);
    case DisplayTest_RemapSceneWithRenderTarget:
        return runRemapSceneWithRenderTargetTest(testFramework);
    case DisplayTest_RemapSceneWithText:
        return runRemapSceneWithTextTest(testFramework);
    case DisplayTest_RemapSceneWithChangedContent:
        return runRemapSceneWithChangedContentTest(testFramework);
    case DisplayTest_RemapSceneToWarpedDisplay:
        return runRemapSceneToWarpedDisplayTest(testFramework);
    case DisplayTest_ResubscribeScene:
        return runResubscribeSceneTest(testFramework);
    default:
        assert(!"Invalid renderer test ID!");
        return false;
    }
}

bool DisplayRenderingTests::runTwoScenesTest(RendererTestsFramework& testFramework)
{
    const ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES,
        ramses_internal::Vector3(2.0f, 0.0f, 5.0f));
    const ramses::sceneId_t otherId = testFramework.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::SUBTRACTIVE_BLENDING,
        ramses_internal::Vector3(-2.5f, -0.5f, 5.0f));

    testFramework.publishAndFlushScene(sceneId);
    testFramework.publishAndFlushScene(otherId);
    testFramework.getSceneToRendered(sceneId);
    testFramework.getSceneToRendered(otherId);

    return testFramework.renderAndCompareScreenshot("ARendererDisplays_TwoScenes");
}

bool DisplayRenderingTests::runUnpublishTest(RendererTestsFramework& testFramework)
{
    const ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES,
        ramses_internal::Vector3(2.0f, 0.0f, 5.0f));
    const ramses::sceneId_t otherId = testFramework.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::SUBTRACTIVE_BLENDING,
        ramses_internal::Vector3(-2.5f, -0.5f, 5.0f));

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
        ramses_internal::Vector3(2.0f, 0.0f, 5.0f));
    const ramses::sceneId_t otherId = testFramework.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::SUBTRACTIVE_BLENDING,
        ramses_internal::Vector3(-2.5f, -0.5f, 5.0f));

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
        ramses_internal::Vector3(0.5f, 0.5f, 5.0f));
    const ramses::sceneId_t otherId = testFramework.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES,
        ramses_internal::Vector3(-0.5f, 0.0f, 5.0f));

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
        ramses_internal::Vector3(0.5f, 0.5f, 5.0f));
    const ramses::sceneId_t otherId = testFramework.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES,
        ramses_internal::Vector3(-0.5f, 0.0f, 5.0f));

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
        ramses_internal::Vector3(0.0f, 0.0f, 5.0f));
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
        ramses_internal::Vector3(0.0f, 0.0f, 5.0f));
    const ramses::sceneId_t sceneId2 = testFramework.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::TRIANGLES_REORDERED,
        ramses_internal::Vector3(0.0f, 0.0f, 5.0f));

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
        ramses_internal::Vector3(0.0f, 0.0f, 5.0f));
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
        ramses_internal::Vector3(0.0f, 0.0f, 5.0f));
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
        ramses_internal::Vector3(0.0f, 0.0f, 5.0f));
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
        ramses_internal::Vector3(2.0f, 0.0f, 5.0f));
    const ramses::sceneId_t sceneId2 = testFramework.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::TRIANGLES_REORDERED,
        ramses_internal::Vector3(-2.5f, -0.5f, 5.0f));

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
        ramses_internal::Vector3(0.0f, 0.0f, 5.0f));
    testFramework.publishAndFlushScene(sceneId);
    testFramework.getSceneToRendered(sceneId);
    bool testResult = testFramework.renderAndCompareScreenshot("ARendererInstance_Three_Triangles", 0u);

    testFramework.getSceneToState(sceneId, ramses::RendererSceneState::Unavailable);
    testResult &= testFramework.renderAndCompareScreenshot("ARendererDisplays_Black", 0u);

    testFramework.getSceneToState(sceneId, ramses::RendererSceneState::Rendered);
    testResult &= testFramework.renderAndCompareScreenshot("ARendererInstance_Three_Triangles", 0u);

    return testResult;
}
