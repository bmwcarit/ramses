//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "internal/RendererLib/RamshCommands/SetClearColor.h"
#include "internal/RendererLib/RendererCommandBuffer.h"
#include "RendererCommandVisitorMock.h"

using namespace ::testing;

namespace ramses::internal
{
    class ASetClearColor : public ::testing::Test
    {
    public:
        ASetClearColor()
            : m_cmd(m_rendererCommandBuffer)
        {
        }

    protected:
        void expectCommand(const DisplayHandle& displayHandle, const OffscreenBufferHandle& obHandle, const glm::vec4& color)
        {
            StrictMock<RendererCommandVisitorMock> cmdVisitor;
            EXPECT_CALL(cmdVisitor, handleSetClearColor(displayHandle, obHandle, color));
            cmdVisitor.visit(m_rendererCommandBuffer);
        }

        RendererCommandBuffer m_rendererCommandBuffer;
        SetClearColor m_cmd;
    };

    TEST_F(ASetClearColor, missingArguments)
    {
        EXPECT_FALSE(m_cmd.executeInput(std::vector<std::string>{"clc"}));
        EXPECT_FALSE(m_cmd.executeInput(std::vector<std::string>{}));
    }

    TEST_F(ASetClearColor, tooManyArguments)
    {
        EXPECT_FALSE(m_cmd.executeInput({"clc", "0", "0", "0", "0", "0", "0"}));
    }

    TEST_F(ASetClearColor, clearDisplayColor)
    {
        EXPECT_TRUE(m_cmd.executeInput({"clc", "42", "1", "0", "1", "1"}));
        expectCommand(DisplayHandle(42), OffscreenBufferHandle::Invalid(), {1.f, 0.f, 1.f, 1.f});
    }

    TEST_F(ASetClearColor, clearOffscreenBuffer)
    {
        EXPECT_TRUE(m_cmd.executeInput({"clc", "42", "0.1", "0.2", "0.3", "1", "-ob", "3"}));
        expectCommand(DisplayHandle(42), OffscreenBufferHandle(3), {0.1f, 0.2f, 0.3f, 1.f});
    }
}
