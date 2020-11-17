//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TESTEQUALHELPER_H
#define RAMSES_TESTEQUALHELPER_H

#include "gtest/gtest.h"

namespace ramses_internal
{

    template <typename MATRIXTYPE>
    inline
    void expectMatrixFloatEqual(const MATRIXTYPE& expected, const MATRIXTYPE& actual)
    {
        constexpr auto elementCount = sizeof(MATRIXTYPE) / sizeof(float);
        for (uint32_t i = 0u; i < elementCount; i++)
        {
            EXPECT_NEAR(expected.data[i], actual.data[i], 1.0e-6f);
        }
    }
}

#endif
