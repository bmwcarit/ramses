//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "IRendererTest.h"

namespace ramses::internal
{
    class DisplayRenderingTests : public IRendererTest
    {
    public:
        void setUpTestCases(RendererTestsFramework& testFramework) final;
        bool run(RendererTestsFramework& testFramework, const RenderingTestCase& testCase) final;

    private:
        static bool RunTwoScenesTest(RendererTestsFramework& testFramework);
        static bool RunUnpublishTest(RendererTestsFramework& testFramework);
        static bool RunHideTest(RendererTestsFramework& testFramework);
        static bool RunSceneRenderOrderTest(RendererTestsFramework& testFramework);
        static bool RunSceneRenderOrderInversedTest(RendererTestsFramework& testFramework);
        static bool RunSubimageTest(RendererTestsFramework& testFramework);
        static bool RunRemapSceneTest(RendererTestsFramework& testFramework);
        static bool RunSwapScenesTest(RendererTestsFramework& testFramework);
        static bool RunRemapSceneWithRenderTargetTest(RendererTestsFramework& testFramework);
        static bool RunRemapSceneWithTextTest(RendererTestsFramework& testFramework);
        static bool RunRemapSceneWithChangedContentTest(RendererTestsFramework& testFramework);
        static bool RunResubscribeSceneTest(RendererTestsFramework& testFramework);
        static bool RunFramebufferWithoutDepthAndStencilTest(RendererTestsFramework& testFramework);
        static bool RunFramebufferWithoutStencil(RendererTestsFramework& testFramework);

        enum
        {
            DisplayRenderingTest_TwoScenes = 0,
            DisplayRenderingTest_UnpublishScene,
            DisplayRenderingTest_HideScene,
            DisplayRenderingTest_SceneRenderOrder,
            DisplayRenderingTest_SceneRenderOrderInversed,
            DisplayRenderingTest_Subimage,
            DisplayRenderingTest_RemapScene,
            DisplayRenderingTest_SwapScenes,
            DisplayRenderingTest_RemapSceneWithRenderTarget,
#if defined(RAMSES_TEXT_ENABLED)
            DisplayRenderingTest_RemapSceneWithText,
#endif
            DisplayRenderingTest_RemapSceneWithChangedContent,
            DisplayRenderingTest_ResubscribeScene,
            DisplayRenderingTest_FramebufferWithoutDepthAndStencil,
            DisplayRenderingTest_FramebufferWithoutStencil,
            DisplayRenderingTest_AsyncEffectUploadDisabled
        };

        static constexpr uint32_t DisplayWidth = 128u;
        static constexpr uint32_t DisplayHeight = 64u;
    };
}
