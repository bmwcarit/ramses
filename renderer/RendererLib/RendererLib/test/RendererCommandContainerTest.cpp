//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "gtest/gtest.h"
#include "RendererLib/RendererCommandContainer.h"

using namespace testing;
using namespace ramses_internal;

class ARendererCommandContainer : public ::testing::Test
{
protected:
    RendererCommandContainer commands;
};

TEST_F(ARendererCommandContainer, isEmptyAfterCreation)
{
    EXPECT_EQ(0u, commands.getTotalCommandCount());
}

TEST_F(ARendererCommandContainer, canAddAndGetCommand)
{
    SceneInfo info(SceneId(123u));
    SceneInfoCommand cmd;
    cmd.sceneInformation = info;
    commands.addCommand<SceneInfoCommand>(ERendererCommand_PublishedScene, cmd);

    ASSERT_EQ(1u, commands.getTotalCommandCount());
    EXPECT_EQ(ERendererCommand_PublishedScene, commands.getCommandType(0u));
    EXPECT_EQ(info, commands.getCommandData<SceneInfoCommand>(0u).sceneInformation);
}

TEST_F(ARendererCommandContainer, canClearCommands)
{
    commands.addCommand<SceneInfoCommand>(ERendererCommand_PublishedScene, SceneInfoCommand());
    commands.addCommand<SceneMappingCommand>(ERendererCommand_PublishedScene, SceneMappingCommand());
    commands.clear();

    EXPECT_EQ(0u, commands.getTotalCommandCount());
}
