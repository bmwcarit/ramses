//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MULTISCENESTREAMTEXTURETESTS_H
#define RAMSES_MULTISCENESTREAMTEXTURETESTS_H

#include "IEmbeddedCompositingTest.h"

namespace ramses_internal
{
    class MultiSceneStreamTextureTests : public IEmbeddedCompositingTest
    {
    public:
        virtual void setUpEmbeddedCompositingTestCases(EmbeddedCompositingTestsFramework& testFramework) final;
        virtual bool runEmbeddedCompositingTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase) final;

    private:
        enum
        {
            CanUseTwoStreamTexturesWithSameSourceIdAndSameFallbackTextureFromTwoScenes,
            CanUseTwoStreamTexturesWithSameSourceIdAndDifferentFallbackTextureFromTwoScenes,
            CanUseTwoStreamTexturesWithDifferentSourceIdAndSameFallbackTextureFromTwoScenes,
        };

        static constexpr uint32_t DisplayWidth = IntegrationScene::DefaultViewportWidth * 2;
        static constexpr uint32_t DisplayHeight = IntegrationScene::DefaultViewportHeight;
    };
}

#endif
