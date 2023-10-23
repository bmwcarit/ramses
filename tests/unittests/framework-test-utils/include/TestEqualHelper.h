//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "gtest/gtest.h"
#include "glm/mat2x2.hpp"

namespace ramses::internal
{
    template <int C, int R>
    inline
    void expectMatrixFloatEqual(const glm::mat<C, R, float, glm::defaultp>& expected, const glm::mat<C, R, float, glm::defaultp>& actual)
    {
        for (int i = 0; i < C; i++)
        {
            for (int k = 0; k < R; ++k)
            {
                EXPECT_NEAR(expected[i][k], actual[i][k], 1.0e-6f);
            }
        }
    }
}
