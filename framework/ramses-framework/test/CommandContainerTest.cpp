//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "CommandContainer.h"
#include "CommandTest.h"

using namespace testing;
using namespace ramses_internal;


class ACommandContainer : public ::testing::Test
{
protected:
    typedef CommandContainer< E_TestCommandTypes, CustomCommand > TestCommandContainer;
    TestCommandContainer commands;
};

TEST_F(ACommandContainer, isEmptyAfterCreation)
{
    EXPECT_EQ(0u, commands.getTotalCommandCount());
}

TEST_F(ACommandContainer, canAddAndGetCommand)
{
    DoThisCommand cmd(42u);
    commands.addCommand<DoThisCommand>(E_TestCommandTypes_DoThis, cmd);

    ASSERT_EQ(1u, commands.getTotalCommandCount());
    EXPECT_EQ(E_TestCommandTypes_DoThis, commands.getCommandType(0u));
    EXPECT_EQ(42u, commands.getCommandData<DoThisCommand>(0u).value);
}

TEST_F(ACommandContainer, canClearCommands)
{
    commands.addCommand<DoThisCommand>(E_TestCommandTypes_DoThis, DoThisCommand(13u));
    commands.addCommand<DoNothingCommand>(E_TestCommandTypes_DoNothing, DoNothingCommand());
    commands.clear();

    EXPECT_EQ(0u, commands.getTotalCommandCount());
}

TEST_F(ACommandContainer, canSwapContent)
{
    commands.addCommand<DoThisCommand>(E_TestCommandTypes_DoThis, DoThisCommand(13u));
    commands.addCommand<DoNothingCommand>(E_TestCommandTypes_DoNothing, DoNothingCommand());
    EXPECT_EQ(2u, commands.getTotalCommandCount());

    TestCommandContainer otherCommands;

    otherCommands.swap(commands);
    EXPECT_EQ(0u, commands.getTotalCommandCount());
    ASSERT_EQ(2u, otherCommands.getTotalCommandCount());
    EXPECT_EQ(E_TestCommandTypes_DoThis,  otherCommands.getCommandType(0));
    EXPECT_EQ(E_TestCommandTypes_DoNothing, otherCommands.getCommandType(1));
    EXPECT_EQ(13u,  otherCommands.getCommandData<DoThisCommand>(0).value);

}
