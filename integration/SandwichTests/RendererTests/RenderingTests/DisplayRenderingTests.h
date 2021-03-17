//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DISPLAYRENDERINGTESTS_H
#define RAMSES_DISPLAYRENDERINGTESTS_H

#include "IRendererTest.h"

class DisplayRenderingTests : public IRendererTest
{
public:
    virtual void setUpTestCases(RendererTestsFramework& testFramework) final;
    virtual bool run(RendererTestsFramework& testFramework, const RenderingTestCase& testCase) final;

private:
    bool runTwoScenesTest(RendererTestsFramework& testFramework);
    bool runUnpublishTest(RendererTestsFramework& testFramework);
    bool runHideTest(RendererTestsFramework& testFramework);
    bool runSceneRenderOrderTest(RendererTestsFramework& testFramework);
    bool runSceneRenderOrderInversedTest(RendererTestsFramework& testFramework);
    bool runWarpingTest(RendererTestsFramework& testFramework);
    bool runUpdateWarpingTest(RendererTestsFramework& testFramework);
    bool runSubimageTest(RendererTestsFramework& testFramework);
    bool runRemapSceneTest(RendererTestsFramework& testFramework);
    bool runSwapScenesTest(RendererTestsFramework& testFramework);
    bool runRemapSceneWithRenderTargetTest(RendererTestsFramework& testFramework);
    bool runRemapSceneWithTextTest(RendererTestsFramework& testFramework);
    bool runRemapSceneToWarpedDisplayTest(RendererTestsFramework& testFramework);
    bool runRemapSceneWithChangedContentTest(RendererTestsFramework& testFramework);
    bool runResubscribeSceneTest(RendererTestsFramework& testFramework);
    bool runFramebufferWithoutDepthAndStencilTest(RendererTestsFramework& testFramework);
    bool runFramebufferWithoutStencil(RendererTestsFramework& testFramework);

    enum
    {
        DisplayRenderingTest_TwoScenes = 0,
        DisplayRenderingTest_UnpublishScene,
        DisplayRenderingTest_HideScene,
        DisplayRenderingTest_SceneRenderOrder,
        DisplayRenderingTest_SceneRenderOrderInversed,
        DisplayRenderingTest_Warping,
        DisplayRenderingTest_UpdateWarping,
        DisplayRenderingTest_Subimage,
        DisplayRenderingTest_RemapScene,
        DisplayRenderingTest_SwapScenes,
        DisplayRenderingTest_RemapSceneWithRenderTarget,
        DisplayRenderingTest_RemapSceneWithText,
        DisplayRenderingTest_RemapSceneWithChangedContent,
        DisplayRenderingTest_RemapSceneToWarpedDisplay,
        DisplayRenderingTest_ResubscribeScene,
        DisplayRenderingTest_FramebufferWithoutDepthAndStencil,
        DisplayRenderingTest_FramebufferWithoutStencil,
        DisplayRenderingTest_AsyncEffectUploadDisabled
    };

    static constexpr uint32_t DisplayWidth = 128u;
    static constexpr uint32_t DisplayHeight = 64u;
};

#endif
