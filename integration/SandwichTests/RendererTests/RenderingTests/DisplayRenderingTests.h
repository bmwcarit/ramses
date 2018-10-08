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
    bool runStereoTest(RendererTestsFramework& testFramework);
    bool runSubimageTest(RendererTestsFramework& testFramework);
    bool runRemapSceneTest(RendererTestsFramework& testFramework);
    bool runSwapScenesTest(RendererTestsFramework& testFramework);
    bool runRemapSceneWithRenderTargetTest(RendererTestsFramework& testFramework);
    bool runRemapSceneWithTextTest(RendererTestsFramework& testFramework);
    bool runRemapSceneToWarpedDisplayTest(RendererTestsFramework& testFramework);
    bool runRemapSceneWithChangedContentTest(RendererTestsFramework& testFramework);
    bool runResubscribeSceneTest(RendererTestsFramework& testFramework);

    enum
    {
        DisplayTest_TwoScenes = 0,
        DisplayTest_UnpublishScene,
        DisplayTest_HideScene,
        DisplayTest_SceneRenderOrder,
        DisplayTest_SceneRenderOrderInversed,
        DisplayTest_Warping,
        DisplayTest_UpdateWarping,
        DisplayTest_Stereo,
        DisplayTest_Subimage,
        DisplayTest_RemapScene,
        DisplayTest_SwapScenes,
        DisplayTest_RemapSceneWithRenderTarget,
        DisplayTest_RemapSceneWithText,
        DisplayTest_RemapSceneWithChangedContent,
        DisplayTest_RemapSceneToWarpedDisplay,
        DisplayTest_ResubscribeScene
    };
};

#endif
