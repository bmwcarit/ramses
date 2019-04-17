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
#include "gtest/internal/gtest-internal.h"

namespace ramses_internal
{
    // compares two matrices based on google test's float comparison
    template <typename MATRIXTYPE>
    inline
        ::testing::AssertionResult matrixFloatEquals(const MATRIXTYPE& expected, const MATRIXTYPE& actual)
    {
        ::testing::AssertionResult result = ::testing::AssertionFailure();
        Bool isOK = true;
        static const UInt32 NumElements = sizeof(MATRIXTYPE) / sizeof(Float);
        for (UInt32 index = 0; index < NumElements; index++)
        {
            const ::testing::internal::FloatingPoint<Float> lhs(expected.data[index]), rhs(actual.data[index]);
            if (!lhs.AlmostEquals(rhs))
            {
                result << " " << expected.data[index] << " != " << actual.data[index] << " @ " << index << ", ";
                isOK = false;
            }
        }
        if (!isOK)
        {
            return result;
        }
        return ::testing::AssertionSuccess();
    }
}

#endif
