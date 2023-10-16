//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RenderPassRenderingTests.h"
#include "TestScenes/RenderPassOnceScene.h"
#include "RendererTestUtils.h"

namespace ramses::internal
{
    void RenderPassRenderingTests::setUpTestCases(RendererTestsFramework& testFramework)
    {
        testFramework.createTestCaseWithDefaultDisplay(RenderPassTest_RenderOnce, *this, "RenderPassTest_RenderOnce");
        testFramework.createTestCaseWithDefaultDisplay(RenderPassTest_RetriggerRenderOnce, *this, "RenderPassTest_RetriggerRenderOnce");
        testFramework.createTestCaseWithDefaultDisplay(RenderPassTest_RemapSceneWithRenderOnce, *this, "RenderPassTest_RemapSceneWithRenderOnce");
    }

    bool RenderPassRenderingTests::run(RendererTestsFramework& testFramework, const RenderingTestCase& testCase)
    {
        switch (testCase.m_id)
        {
        case RenderPassTest_RenderOnce:
            return RunRenderOnceTest(testFramework);
        case RenderPassTest_RetriggerRenderOnce:
            return RunRetriggerRenderOnceTest(testFramework);
        case RenderPassTest_RemapSceneWithRenderOnce:
            return RunRemapSceneWithRenderOnceTest(testFramework);
        default:
            assert(!"Invalid renderer test ID!");
            return false;
        }
    }

    ramses::sceneId_t RenderPassRenderingTests::CreateAndSetupInitialRenderOnceScene(RendererTestsFramework& testFramework)
    {
        const ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<RenderPassOnceScene>(RenderPassOnceScene::INITIAL_RENDER_ONCE);

        testFramework.publishAndFlushScene(sceneId);
        testFramework.getSceneToRendered(sceneId);

        return sceneId;
    }

    bool RenderPassRenderingTests::RunRenderOnceTest(RendererTestsFramework& testFramework)
    {
        const ramses::sceneId_t sceneId = CreateAndSetupInitialRenderOnceScene(testFramework);

        // take 2 screenshots to make sure final image stays even with render once pass
        if (!testFramework.renderAndCompareScreenshot("RenderPassOnce_Initial") ||
            !testFramework.renderAndCompareScreenshot("RenderPassOnce_Initial"))
        {
            return false;
        }

        // change clear color but do not retrigger render once pass, expect same result
        testFramework.getScenesRegistry().setSceneState<RenderPassOnceScene>(sceneId, RenderPassOnceScene::CHANGE_CLEAR_COLOR);
        testFramework.getScenesRegistry().getScene(sceneId).flush();

        return testFramework.renderAndCompareScreenshot("RenderPassOnce_Initial");
    }

    bool RenderPassRenderingTests::RunRetriggerRenderOnceTest(RendererTestsFramework& testFramework)
    {
        const ramses::sceneId_t sceneId = CreateAndSetupInitialRenderOnceScene(testFramework);

        if (!testFramework.renderAndCompareScreenshot("RenderPassOnce_Initial"))
        {
            return false;
        }

        // change clear color but do not retrigger render once pass, expect same result
        testFramework.getScenesRegistry().setSceneState<RenderPassOnceScene>(sceneId, RenderPassOnceScene::CHANGE_CLEAR_COLOR);
        testFramework.getScenesRegistry().getScene(sceneId).flush();

        if (!testFramework.renderAndCompareScreenshot("RenderPassOnce_Initial"))
        {
            return false;
        }

        // retrigger render once pass and expect new result
        testFramework.getScenesRegistry().setSceneState<RenderPassOnceScene>(sceneId, RenderPassOnceScene::RETRIGGER_PASS);
        testFramework.getScenesRegistry().getScene(sceneId).flush();

        // take 2 screenshots to make sure final image stays even with render once pass
        return testFramework.renderAndCompareScreenshot("RenderPassOnce_Retriggered")
            && testFramework.renderAndCompareScreenshot("RenderPassOnce_Retriggered");
    }

    bool RenderPassRenderingTests::RunRemapSceneWithRenderOnceTest(RendererTestsFramework& testFramework)
    {
        const ramses::sceneId_t sceneId = CreateAndSetupInitialRenderOnceScene(testFramework);

        if (!testFramework.renderAndCompareScreenshot("RenderPassOnce_Initial"))
        {
            return false;
        }

        testFramework.getSceneToState(sceneId, ramses::RendererSceneState::Available);
        if (!testFramework.renderAndCompareScreenshot("OffscreenBufferLinkTest_Black"))
        {
            return false;
        }

        testFramework.getSceneToState(sceneId, ramses::RendererSceneState::Rendered);

        return testFramework.renderAndCompareScreenshot("RenderPassOnce_Initial");
    }
}
