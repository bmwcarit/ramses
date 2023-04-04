//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "gtest/gtest.h"
#include "ramses-logic/EFeatureLevel.h"

namespace rlogic::internal
{
    static
        ::testing::internal::ValueArray<rlogic::EFeatureLevel, rlogic::EFeatureLevel, rlogic::EFeatureLevel, rlogic::EFeatureLevel, rlogic::EFeatureLevel>
        GetFeatureLevelTestValues()
    {
        return ::testing::Values(rlogic::EFeatureLevel_01, rlogic::EFeatureLevel_02, rlogic::EFeatureLevel_03, rlogic::EFeatureLevel_04, rlogic::EFeatureLevel_05);
    }
}
