//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MULTISTREAMTEXTURETESTS_H
#define RAMSES_MULTISTREAMTEXTURETESTS_H

#include "IEmbeddedCompositingTest.h"

namespace ramses_internal
{
    class MultiStreamTextureTests : public IEmbeddedCompositingTest
    {
    public:
        virtual void setUpEmbeddedCompositingTestCases(EmbeddedCompositingTestsFramework& testFramework) final;
        virtual bool runEmbeddedCompositingTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase) final;

    private:
        enum
        {
            ShowDifferentFallbackTexturesOnStreamTexturesWithSameSourceId,
            ShowStreamTextureOnTwoStreamTexturesWithSameSourceId,
            ShowSameFallbackTextureOnStreamTexturesWithDifferentSourceId,
            ShowTwoStreamTextures,
            ClientRecreatesIVISurfaceWithDifferentId,
        };

        static constexpr uint32_t DisplayWidth = IntegrationScene::DefaultViewportWidth * 2;
        static constexpr uint32_t DisplayHeight = IntegrationScene::DefaultViewportHeight;
    };
}

#endif
