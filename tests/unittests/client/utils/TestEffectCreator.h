//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <gtest/gtest.h>
#include "ClientTestUtils.h"

namespace ramses::internal
{
    class TestEffectCreator : public LocalTestClientWithScene
    {
    public:
        explicit TestEffectCreator(ramses::EFeatureLevel featureLevel = EFeatureLevel_Latest, bool withGeometryShader = false)
            : LocalTestClientWithScene{ featureLevel }
        {
            effect = CreateEffect(m_scene, withGeometryShader, featureLevel);
            EXPECT_TRUE(effect != nullptr);
            appearance = this->m_scene.createAppearance(*effect);
            EXPECT_TRUE(appearance != nullptr);
        }

        Appearance& recreateAppearence()
        {
            assert(appearance);
            this->m_scene.destroy(*appearance);
            appearance = this->m_scene.createAppearance(*effect);
            assert(appearance);

            return *appearance;
        }

        ~TestEffectCreator() override
        {
            EXPECT_TRUE(this->m_scene.destroy(*appearance));
        }

        static Effect* CreateEffect(ramses::Scene& scene, bool withGeometryShader = false, EFeatureLevel featureLevel = EFeatureLevel_Latest);

        Effect* effect = nullptr;
        Appearance* appearance = nullptr;
    };

    // Helper class to provide effect creator in static fashion (to be shared across test cases) while supporting feature level.
    // Usage: derive test class from this and use m_sharedTestState.
    // Tip: use with RAMSES_INSTANTIATE_FEATURELEVEL_TEST_SUITE to instantiate templated test suite.
    class TestWithSharedEffectPerFeatureLevel : public ::testing::TestWithParam<ramses::EFeatureLevel>
    {
    public:
        explicit TestWithSharedEffectPerFeatureLevel(bool withGeometryShader = false)
            : m_withGeometryShader{ withGeometryShader }
        {
        }

        static TestEffectCreator& GetEffectCreator(ramses::EFeatureLevel featureLevel, bool withGeometryShader = false)
        {
            assert(featureLevel > 0 && featureLevel <= ramses::EFeatureLevel_Latest);
            const size_t idx = featureLevel - 1;
            if (!effectCreators[idx])
                effectCreators[idx] = std::make_unique<TestEffectCreator>(featureLevel, withGeometryShader);
            return *effectCreators[idx];
        }

        static void TearDownTestSuite()
        {
            for (auto& c : effectCreators)
                c.reset();
        }

    protected:
        bool m_withGeometryShader = false;
        TestEffectCreator& m_sharedTestState = GetEffectCreator(GetParam(), m_withGeometryShader);

    private:
        static std::array<std::unique_ptr<TestEffectCreator>, ramses::EFeatureLevel_Latest> effectCreators;
    };
}
