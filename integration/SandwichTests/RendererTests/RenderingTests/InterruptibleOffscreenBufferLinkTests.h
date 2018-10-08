//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_INTERRUPTIBLEOFFSCREENBUFFERLINKTESTS_H
#define RAMSES_INTERRUPTIBLEOFFSCREENBUFFERLINKTESTS_H

#include "IRendererTest.h"

class InterruptibleOffscreenBufferLinkTests : public IRendererTest
{
public:
    virtual void setUpTestCases(RendererTestsFramework& testFramework) override final;
    virtual bool run(RendererTestsFramework& testFramework, const RenderingTestCase& testCase) override final;

private:
    enum
    {
        InterruptibleOffscreenBufferLinkTest_OneInterruptibleOBWithOneScene,
        InterruptibleOffscreenBufferLinkTest_OneInterruptibleOBWithTwoScenes,
        InterruptibleOffscreenBufferLinkTest_InterruptionDoesNotAffectFrameBufferScene,
        InterruptibleOffscreenBufferLinkTest_TwoInterruptibleOBsEachWithOneScene,
        InterruptibleOffscreenBufferLinkTest_RerendersSceneIfItGetsModifiedWhileInterrupted,
        InterruptibleOffscreenBufferLinkTest_RerendersInterruptedSceneIfItGetsModifiedWhileInterrupted,
        InterruptibleOffscreenBufferLinkTest_RerendersSceneIfItGetsModifiedWhileAnotherSceneIsBeingRendered,
        InterruptibleOffscreenBufferLinkTest_RerendersInterruptedSceneIfItGetsModifiedWhileInterrupted_SameBuffer,
        InterruptibleOffscreenBufferLinkTest_RerendersSceneIfItGetsModifiedWhileAnotherSceneIsBeingRendered_SameBuffer,
    };

    bool renderAndCompareScreenshot(RendererTestsFramework& testFramework, const ramses_internal::String& expectedImage, uint32_t numFramesToRender = 1u);

    const ramses_internal::Vector3 m_cameraMid{ 0.f, 0.f, 8.f };
};

#endif
