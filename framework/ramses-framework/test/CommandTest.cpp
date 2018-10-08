//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "CommandTest.h"

using namespace testing;
using namespace ramses_internal;


TEST(ACommand, CanBeCreated)
{
    const DoThisCommand dtc(1u);
    const DoNothingCommand dnc;

    EXPECT_EQ(E_TestCommandTypes_DoThis, dtc.commandType);
    EXPECT_EQ(E_TestCommandTypes_DoNothing, dnc.commandType);
}

TEST(ACommand, canConvertToItsType)
{
    const CustomCommand* c = new DoThisCommand(23u);
    ASSERT_TRUE(c);

    const DoThisCommand& dtc = c->convertTo< DoThisCommand >();
    EXPECT_EQ(23u, dtc.value);

    delete c;
}
