//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "gtest/gtest.h"
#include "ramses-framework-api/EFeatureLevel.h"

namespace ramses::internal
{
    static
        ::testing::internal::ValueArray<ramses::EFeatureLevel>
        GetFeatureLevelTestValues()
    {
        return ::testing::Values(ramses::EFeatureLevel_01);
    }
}
