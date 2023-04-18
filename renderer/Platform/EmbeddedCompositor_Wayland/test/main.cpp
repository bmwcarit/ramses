//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "RendererTestUtils.h"
#include "PlatformAbstraction/PlatformConsole.h"
#include "Utils/ThreadLocalLog.h"

int main(int argc, char* argv[])
{
    testing::InitGoogleMock(&argc, argv);

    // set log prefix for all tests
    ramses_internal::ThreadLocalLog::SetPrefix(1);

    return RUN_ALL_TESTS();
}
