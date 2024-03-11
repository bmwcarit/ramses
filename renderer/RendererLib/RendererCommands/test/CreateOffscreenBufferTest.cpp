//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "gtest/gtest.h"
#include "RendererCommands/CreateOffscreenBuffer.h"
#include "RendererLib/RendererCommandBuffer.h"
#include "RendererCommandVisitorMock.h"

using namespace ramses_internal;
using namespace ::testing;

class ARamshCreateOffscreenBuffer : public ::testing::Test
{
public:
    ARamshCreateOffscreenBuffer()
        : m_cmd(m_rendererCommandBuffer)
    {
    }

protected:
    void expectCommand(const DisplayHandle& display, const OffscreenBufferHandle& buffer, uint32_t width, uint32_t height)
    {
        StrictMock<RendererCommandVisitorMock> cmdVisitor;
        const uint32_t                         samples = 0;
        const bool                             interruptible = false;
        EXPECT_CALL(cmdVisitor, handleBufferCreateRequest(buffer, display, width, height, samples, interruptible, ERenderBufferType_DepthStencilBuffer));
        cmdVisitor.visit(m_rendererCommandBuffer);
    }

    RendererCommandBuffer m_rendererCommandBuffer;
    CreateOffscreenBuffer m_cmd;
};

TEST_F(ARamshCreateOffscreenBuffer, missingArguments)
{
    EXPECT_FALSE(m_cmd.executeInput(std::vector<std::string>{"unlink"}));
    EXPECT_FALSE(m_cmd.executeInput(std::vector<std::string>{}));
}

TEST_F(ARamshCreateOffscreenBuffer, create)
{
    EXPECT_TRUE(m_cmd.executeInput({"obCreate", "0", "7", "400", "240"}));
    expectCommand(DisplayHandle(0), OffscreenBufferHandle(7), 400, 240);
}

TEST_F(ARamshCreateOffscreenBuffer, sizeLimit)
{
    EXPECT_TRUE(m_cmd.executeInput({"obCreate", "0", "7", "4096", "4096"}));
    expectCommand(DisplayHandle(0), OffscreenBufferHandle(7), 4096, 4096);
}

TEST_F(ARamshCreateOffscreenBuffer, invalidDisplay)
{
    EXPECT_FALSE(m_cmd.executeInput({"obCreate", "foo", "7", "400", "240"}));
}

TEST_F(ARamshCreateOffscreenBuffer, invalidOffscreenBuffer)
{
    EXPECT_FALSE(m_cmd.executeInput({"obCreate", "0", "bar", "400", "240"}));
}

TEST_F(ARamshCreateOffscreenBuffer, invalidWidth)
{
    EXPECT_FALSE(m_cmd.executeInput({"obCreate", "0", "7", "4097", "240"}));
}

TEST_F(ARamshCreateOffscreenBuffer, zeroWidth)
{
    EXPECT_FALSE(m_cmd.executeInput({"obCreate", "0", "7", "0", "240"}));
}

TEST_F(ARamshCreateOffscreenBuffer, invalidHeight)
{
    EXPECT_FALSE(m_cmd.executeInput({"obCreate", "0", "7", "1025", "4097"}));
}

TEST_F(ARamshCreateOffscreenBuffer, zeroHeight)
{
    EXPECT_FALSE(m_cmd.executeInput({"obCreate", "0", "7", "1025", "0"}));
}
