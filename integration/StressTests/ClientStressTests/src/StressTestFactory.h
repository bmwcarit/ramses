//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_STRESSTESTFACTORY_H
#define RAMSES_STRESSTESTFACTORY_H

#include "StressTest.h"

class StressTest;


class StressTestFactory
{
public:
    static StressTestPtr CreateTest(uint32_t testIndex, int32_t argc, const char* argv[]);
    static uint32_t GetNumberOfTests();
    static const char* GetNameOfTest(uint32_t testIndex);
};

#endif
