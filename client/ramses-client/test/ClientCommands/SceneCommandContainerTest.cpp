//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "ClientCommands/SceneCommandContainer.h"
#include "ClientCommands/SceneCommandTypes.h"

using namespace testing;

namespace ramses_internal
{
    TEST(SceneCommandContainerTest, ContainerIsInitiallyEmpty)
    {
        SceneCommandContainer container;
        EXPECT_EQ(0u, container.getTotalCommandCount());
    }


    TEST(SceneCommandContainerTest, CanAddAndGetCommands)
    {
        const String name1("cmd1");
        const String name2("cmd2");

        ForceFallbackCommand cmd1;
        cmd1.streamTextureName = name1;
        cmd1.forceFallback     = false;

        ForceFallbackCommand cmd2;
        cmd2.streamTextureName = name2;
        cmd2.forceFallback     = true;

        SceneCommandContainer container;

        container.addCommand(ESceneCommand_ForceFallbackImage, cmd1);
        container.addCommand(ESceneCommand_ForceFallbackImage, cmd2);

        ASSERT_EQ(2u, container.getTotalCommandCount());
        EXPECT_EQ(ESceneCommand_ForceFallbackImage ,container.getCommandType(0));
        EXPECT_EQ(name1 ,container.getCommandData<ForceFallbackCommand>(0).streamTextureName);
        EXPECT_FALSE(container.getCommandData<ForceFallbackCommand>(0).forceFallback);
        EXPECT_EQ(ESceneCommand_ForceFallbackImage ,container.getCommandType(1));
        EXPECT_EQ(name2 ,container.getCommandData<ForceFallbackCommand>(1).streamTextureName);
        EXPECT_TRUE(container.getCommandData<ForceFallbackCommand>(1).forceFallback);
    }

    TEST(SceneCommandContainerTest, CanSwapContainer)
    {
        const String name1("cmd1");
        const String name2("cmd2");

        ForceFallbackCommand cmd1;
        cmd1.streamTextureName = name1;
        cmd1.forceFallback     = false;

        ForceFallbackCommand cmd2;
        cmd2.streamTextureName = name2;
        cmd2.forceFallback     = true;

        SceneCommandContainer container;
        SceneCommandContainer otherContainer;

        container.addCommand(ESceneCommand_ForceFallbackImage, cmd1);
        container.addCommand(ESceneCommand_ForceFallbackImage, cmd2);

        otherContainer.swap(container);
        EXPECT_EQ(0u, container.getTotalCommandCount());
        ASSERT_EQ(2u, otherContainer.getTotalCommandCount());
        EXPECT_EQ(ESceneCommand_ForceFallbackImage ,otherContainer.getCommandType(0));
        EXPECT_EQ(name1 ,otherContainer.getCommandData<ForceFallbackCommand>(0).streamTextureName);
        EXPECT_FALSE(otherContainer.getCommandData<ForceFallbackCommand>(0).forceFallback);
        EXPECT_EQ(ESceneCommand_ForceFallbackImage ,otherContainer.getCommandType(1));
        EXPECT_EQ(name2 ,otherContainer.getCommandData<ForceFallbackCommand>(1).streamTextureName);
        EXPECT_TRUE(otherContainer.getCommandData<ForceFallbackCommand>(1).forceFallback);
    }
}
