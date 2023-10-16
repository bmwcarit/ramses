//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "internal/RendererLib/RamshCommands/SetSceneState.h"
#include "internal/RendererLib/RendererCommandBuffer.h"
#include "RendererCommandVisitorMock.h"

using namespace ::testing;

namespace ramses::internal
{
    class ARamshSetSceneState : public ::testing::Test
    {
    public:
        ARamshSetSceneState()
            : m_cmd(m_rendererCommandBuffer)
        {
        }

    protected:
        void expectCommand(const SceneId& scene, const RendererSceneState& state)
        {
            StrictMock<RendererCommandVisitorMock> cmdVisitor;
            EXPECT_CALL(cmdVisitor, setSceneState(scene, state));
            cmdVisitor.visit(m_rendererCommandBuffer);
        }

        RendererCommandBuffer m_rendererCommandBuffer;
        SetSceneState m_cmd;
    };

    TEST_F(ARamshSetSceneState, missingArguments)
    {
        EXPECT_FALSE(m_cmd.executeInput(std::vector<std::string>{"scenestate"}));
        EXPECT_FALSE(m_cmd.executeInput(std::vector<std::string>{}));
    }

    TEST_F(ARamshSetSceneState, unavailable)
    {
        EXPECT_TRUE(m_cmd.executeInput({"scenestate", "433000002", "0"}));
        expectCommand(SceneId(433000002), RendererSceneState::Unavailable);
    }

    TEST_F(ARamshSetSceneState, available)
    {
        EXPECT_TRUE(m_cmd.executeInput({"scenestate", "433000002", "1"}));
        expectCommand(SceneId(433000002), RendererSceneState::Available);
    }

    TEST_F(ARamshSetSceneState, ready)
    {
        EXPECT_TRUE(m_cmd.executeInput({"scenestate", "433000002", "2"}));
        expectCommand(SceneId(433000002), RendererSceneState::Ready);
    }

    TEST_F(ARamshSetSceneState, rendered)
    {
        EXPECT_TRUE(m_cmd.executeInput({"scenestate", "433000002", "3"}));
        expectCommand(SceneId(433000002), RendererSceneState::Rendered);
    }

    TEST_F(ARamshSetSceneState, invalid)
    {
        EXPECT_FALSE(m_cmd.executeInput({"scenestate", "433000002", "4"}));
    }
}
