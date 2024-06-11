//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "gtest/gtest.h"
#include "ramses/framework/EFeatureLevel.h"

namespace ramses::internal
{
    // List of test values for all supported feature levels.
    // Usage: derive test class from ::testing::TestWithParam<ramses::EFeatureLevel>
    //        and use RAMSES_INSTANTIATE_FEATURELEVEL_TEST_SUITE below to instantiate them
    [[nodiscard]] inline ::testing::internal::ValueArray<ramses::EFeatureLevel, ramses::EFeatureLevel>
        GetFeatureLevelTestValues()
    {
        static_assert(ramses::EFeatureLevel_Latest == ramses::EFeatureLevel_02, "Update this list!");
        return ::testing::Values(ramses::EFeatureLevel_01, ramses::EFeatureLevel_02);
    }

    // List of test values for feature level templated tests but containing only the latest feature level.
    // This is meant to speed up test runtime for tests which are templated for feature level testing
    // but there is no need to test all of them because feature levels do not affect their tested classes' behavior.
    [[nodiscard]] inline ::testing::internal::ValueArray<ramses::EFeatureLevel>
        GetLatestFeatureLevelOnlyTestValues()
    {
        return ::testing::Values(ramses::EFeatureLevel_Latest);
    }

#define RAMSES_INSTANTIATE_FEATURELEVEL_TEST_SUITE(_testName) \
    INSTANTIATE_TEST_SUITE_P( \
        _testName ## Tests, \
        _testName, \
        ramses::internal::GetFeatureLevelTestValues()); \
        static_assert(ramses::EFeatureLevel_Latest == ramses::EFeatureLevel_02, "Re-evaluate which tests need to be instantiated for all feature levels");

#define RAMSES_INSTANTIATE_LATEST_FEATURELEVEL_ONLY_TEST_SUITE(_testName) \
    INSTANTIATE_TEST_SUITE_P( \
        _testName ## Tests, \
        _testName, \
        ramses::internal::GetLatestFeatureLevelOnlyTestValues()); \
        static_assert(ramses::EFeatureLevel_Latest == ramses::EFeatureLevel_02, "Re-evaluate which tests need to be instantiated for all feature levels");
}
