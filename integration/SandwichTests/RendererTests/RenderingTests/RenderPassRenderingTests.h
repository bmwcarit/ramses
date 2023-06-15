//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERPASSRENDERINGTESTS_H
#define RAMSES_RENDERPASSRENDERINGTESTS_H

#include "IRendererTest.h"

class RenderPassRenderingTests : public IRendererTest
{
public:
    void setUpTestCases(RendererTestsFramework& testFramework) final;
    bool run(RendererTestsFramework& testFramework, const RenderingTestCase& testCase) final;

private:
    ramses::sceneId_t createAndSetupInitialRenderOnceScene(RendererTestsFramework& testFramework);

    bool runRenderOnceTest(RendererTestsFramework& testFramework);
    bool runRetriggerRenderOnceTest(RendererTestsFramework& testFramework);
    bool runRemapSceneWithRenderOnceTest(RendererTestsFramework& testFramework);

    enum
    {
        RenderPassTest_RenderOnce = 0,
        RenderPassTest_RetriggerRenderOnce,
        RenderPassTest_RemapSceneWithRenderOnce,
    };
};

#endif
