//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <string>
#include "gmock/gmock.h"
#include "gtest/gtest.h"

GTEST_API_ int main(int argc, char** argv)
{
    std::cout << "Running main() from ramses-framework-test\n";

    // Since Google Mock depends on Google Test, InitGoogleMock() is
    // also responsible for initializing Google Test.  Therefore there's
    // no need for calling testing::InitGoogleTest() separately.
    testing::InitGoogleMock(&argc, argv);
    int testResult = RUN_ALL_TESTS();

    return testResult;
}
