//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "TestingScene.h"
#include "SceneTest.h"

using namespace testing;

namespace ramses::internal
{
    template <typename SCENE>
    class ATestingScene : public testing::Test
    {
    };

    TYPED_TEST_SUITE(ATestingScene, SceneTypes);

    TYPED_TEST(ATestingScene, generateAndCheckContent)
    {
        // no easy way in GTest to test combinations of template parameters, so using simply for loop here
        for (EFeatureLevel featureLevel = EFeatureLevel_01; featureLevel <= EFeatureLevel_Latest; featureLevel = EFeatureLevel{ featureLevel + 1 })
        {
            TypeParam scene;
            TestingScene testingScene{ scene, featureLevel };
            testingScene.VerifyContent(scene);
        }
    }
}
