//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "IRendererTest.h"

namespace ramses::internal
{
    class RenderPassRenderingTests : public IRendererTest
    {
    public:
        void setUpTestCases(RendererTestsFramework& testFramework) final;
        bool run(RendererTestsFramework& testFramework, const RenderingTestCase& testCase) final;

    private:
        static ramses::sceneId_t CreateAndSetupInitialRenderOnceScene(RendererTestsFramework& testFramework);

        static bool RunRenderOnceTest(RendererTestsFramework& testFramework);
        static bool RunRetriggerRenderOnceTest(RendererTestsFramework& testFramework);
        static bool RunRemapSceneWithRenderOnceTest(RendererTestsFramework& testFramework);

        enum
        {
            RenderPassTest_RenderOnce = 0,
            RenderPassTest_RetriggerRenderOnce,
            RenderPassTest_RemapSceneWithRenderOnce,
        };
    };
}
