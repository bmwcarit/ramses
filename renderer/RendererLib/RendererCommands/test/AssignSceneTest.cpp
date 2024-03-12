//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "gtest/gtest.h"
#include "RendererCommands/AssignScene.h"
#include "RendererLib/RendererCommandBuffer.h"
#include "RendererCommandVisitorMock.h"

using namespace ramses_internal;
using namespace ::testing;

class ARamshAssignScene : public ::testing::Test
{
public:
    ARamshAssignScene()
        : m_cmd(m_rendererCommandBuffer)
    {
    }

protected:
    void expectCommand(const SceneId& scene, const DisplayHandle& display, const OffscreenBufferHandle& ob)
    {
        StrictMock<RendererCommandVisitorMock> cmdVisitor;
        EXPECT_CALL(cmdVisitor, setSceneMapping(scene, display));
        EXPECT_CALL(cmdVisitor, setSceneDisplayBufferAssignment(scene, ob, 0));
        cmdVisitor.visit(m_rendererCommandBuffer);
    }

    void expectCommand(const SceneId& scene, const DisplayHandle& display)
    {
        StrictMock<RendererCommandVisitorMock> cmdVisitor;
        EXPECT_CALL(cmdVisitor, setSceneMapping(scene, display));
        cmdVisitor.visit(m_rendererCommandBuffer);
    }

    void expectCommand(const SceneId& scene, const OffscreenBufferHandle& ob)
    {
        StrictMock<RendererCommandVisitorMock> cmdVisitor;
        EXPECT_CALL(cmdVisitor, setSceneDisplayBufferAssignment(scene, ob, 0));
        cmdVisitor.visit(m_rendererCommandBuffer);
    }

    RendererCommandBuffer m_rendererCommandBuffer;
    AssignScene m_cmd;
};

TEST_F(ARamshAssignScene, missingArguments)
{
    EXPECT_FALSE(m_cmd.executeInput(std::vector<std::string>{"assign"}));
    EXPECT_FALSE(m_cmd.executeInput(std::vector<std::string>{}));
}

TEST_F(ARamshAssignScene, invalidScene)
{
    EXPECT_FALSE(m_cmd.executeInput(std::vector<std::string>{"assign", "0"}));
}

TEST_F(ARamshAssignScene, missingTarget)
{
    EXPECT_FALSE(m_cmd.executeInput(std::vector<std::string>{"assign", "43300001"}));
}

TEST_F(ARamshAssignScene, assignToDisplay)
{
    EXPECT_TRUE(m_cmd.executeInput({"assign", "433000001", "-displayId", "0"}));
    expectCommand(SceneId(433000001), DisplayHandle(0));
}

TEST_F(ARamshAssignScene, assignToOffscreenBuffer)
{
    EXPECT_TRUE(m_cmd.executeInput({"assign", "433000001", "-ob", "7"}));
    expectCommand(SceneId(433000001), OffscreenBufferHandle(7));
}

TEST_F(ARamshAssignScene, assignToDisplayAndOffscreen)
{
    EXPECT_TRUE(m_cmd.executeInput({"assign", "433000001", "-ob", "7", "-displayId", "1"}));
    expectCommand(SceneId(433000001), DisplayHandle(1), OffscreenBufferHandle(7));
}

