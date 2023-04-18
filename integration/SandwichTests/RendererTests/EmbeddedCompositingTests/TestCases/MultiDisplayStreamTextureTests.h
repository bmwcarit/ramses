//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MULTIDISPLAYSTREAMTEXTURETESTS_H
#define RAMSES_MULTIDISPLAYSTREAMTEXTURETESTS_H

#include "IEmbeddedCompositingTest.h"

namespace ramses_internal
{
    class MultiDisplayStreamTextureTests : public IEmbeddedCompositingTest
    {
    public:
        void setUpEmbeddedCompositingTestCases(EmbeddedCompositingTestsFramework& testFramework) final;
        bool runEmbeddedCompositingTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase) final;

    private:
        enum
        {
            TwoDisplaysWithCompositingOnFirstDisplayOnly,
            TwoDisplaysWithCompositingOnSecondDisplayOnly,
            TwoDisplaysWithCompositingOnBothDisplays,
        };


        template <typename INTEGRATION_SCENE>
        ramses::sceneId_t createAndShowScene(EmbeddedCompositingTestsFramework& testFramework, uint32_t sceneState, uint32_t vpWidth, uint32_t vpHeight, uint32_t displayIdx)
        {
            const ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<INTEGRATION_SCENE>(sceneState, { 0, 0, 0 }, vpWidth, vpHeight);
            testFramework.publishAndFlushScene(sceneId);
            testFramework.getSceneToRendered(sceneId, displayIdx);
            return sceneId;
        }

        static constexpr uint32_t DisplayWidth = IntegrationScene::DefaultViewportWidth;
        static constexpr uint32_t DisplayHeight = IntegrationScene::DefaultViewportHeight;
    };
}

#endif
