//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TESTUTILS_TESTRANDOM_H
#define RAMSES_TESTUTILS_TESTRANDOM_H

#include "PlatformAbstraction/PlatformTypes.h"
#include <random>

namespace ramses_internal
{
    class TestRandom
    {
    public:
        static size_t Get(size_t minVal, size_t maxVal)
        {
            if (minVal >= maxVal)
                return maxVal;
            std::uniform_int_distribution<size_t> dis(minVal, maxVal - 1);
            return dis(getGen());
        }
    private:
        static std::mt19937& getGen()
        {
            static std::mt19937 gen(std::random_device{}());
            return gen;
        }
    };
}

#endif
