//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"

#include "ramses-client-api/Node.h"
#include "internals/RotationUtils.h"

namespace rlogic::internal
{
    class TheRotationUtils : public ::testing::Test
    {
    protected:
    };

    TEST_F(TheRotationUtils, ConvertRotationTypeEnumsInBothDirections)
    {
        const std::vector<std::pair<ERotationType, ramses::ERotationConvention>> enumPairs =
        {
            {ERotationType::Euler_ZYX, ramses::ERotationConvention::Euler_ZYX},
            {ERotationType::Euler_YZX, ramses::ERotationConvention::Euler_YZX},
            {ERotationType::Euler_ZXY, ramses::ERotationConvention::Euler_ZXY},
            {ERotationType::Euler_XZY, ramses::ERotationConvention::Euler_XZY},
            {ERotationType::Euler_YXZ, ramses::ERotationConvention::Euler_YXZ},
            {ERotationType::Euler_XYZ, ramses::ERotationConvention::Euler_XYZ},
        };

        for (const auto& [logicEnum, ramsesEnum] : enumPairs)
        {
            std::optional<ramses::ERotationConvention> convertedRamsesEnum = RotationUtils::RotationTypeToRamsesRotationConvention(logicEnum);
            EXPECT_TRUE(convertedRamsesEnum);
            EXPECT_EQ(ramsesEnum, *convertedRamsesEnum);

            std::optional<ERotationType> convertedLogicEnum = RotationUtils::RamsesRotationConventionToRotationType(ramsesEnum);
            EXPECT_TRUE(convertedLogicEnum);
            EXPECT_EQ(logicEnum, *convertedLogicEnum);
        }

        EXPECT_FALSE(RotationUtils::RotationTypeToRamsesRotationConvention(ERotationType::Quaternion));
        EXPECT_FALSE(RotationUtils::RamsesRotationConventionToRotationType(ramses::ERotationConvention::Euler_XYX));
        EXPECT_FALSE(RotationUtils::RamsesRotationConventionToRotationType(ramses::ERotationConvention::Euler_XZX));
        EXPECT_FALSE(RotationUtils::RamsesRotationConventionToRotationType(ramses::ERotationConvention::Euler_YXY));
        EXPECT_FALSE(RotationUtils::RamsesRotationConventionToRotationType(ramses::ERotationConvention::Euler_YZY));
        EXPECT_FALSE(RotationUtils::RamsesRotationConventionToRotationType(ramses::ERotationConvention::Euler_ZXZ));
        EXPECT_FALSE(RotationUtils::RamsesRotationConventionToRotationType(ramses::ERotationConvention::Euler_ZYZ));
    }
}
