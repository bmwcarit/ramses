//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_STREAMBUFFERTESTS_H
#define RAMSES_STREAMBUFFERTESTS_H

#include "IEmbeddedCompositingTest.h"

namespace ramses_internal
{
    class StreamBufferTests : public IEmbeddedCompositingTest
    {
    public:
        void setUpEmbeddedCompositingTestCases(EmbeddedCompositingTestsFramework& testFramework) final;
        bool runEmbeddedCompositingTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase) final;

    private:
        enum
        {
            StreamBufferLinkedToSceneWithTwoSamplers,
            StreamBufferLinkedToTwoScenesWithSampler,
            TwoStreamBuffersUsingSameSourceLinkedToSceneWithTwoSamplers,
            TwoStreamBuffersUsingDifferentSourceLinkedToSceneWithTwoSamplers,
            UnlinkStreamBuffer,
            DestroyStreamBuffer,
            StreamBufferBecomesUnavailable
        };

        static constexpr uint32_t DisplayWidth = IntegrationScene::DefaultViewportWidth * 2;
        static constexpr uint32_t DisplayHeight = IntegrationScene::DefaultViewportHeight;
    };
}

#endif
