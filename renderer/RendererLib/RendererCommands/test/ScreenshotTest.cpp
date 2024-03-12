//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "gtest/gtest.h"
#include "RendererCommands/Screenshot.h"
#include "RendererLib/RendererCommandBuffer.h"
#include "RendererCommandVisitorMock.h"

using namespace ramses_internal;
using namespace ::testing;

class AScreenshot : public ::testing::Test
{
public:
    AScreenshot()
        : m_defaultFilename("unnamed.png")
        , m_cmd(m_rendererCommandBuffer)
    {
    }

protected:
    void expectScreenshotCommand(const std::string& filename, const DisplayHandle& displayHandle = DisplayHandle(0u), bool autoSize = true, const OffscreenBufferHandle& obHandle = OffscreenBufferHandle::Invalid())
    {
        StrictMock<RendererCommandVisitorMock> cmdVisitor;
        EXPECT_CALL(cmdVisitor, handleReadPixels(displayHandle, obHandle, _, _, _, _, autoSize, String(filename)));
        cmdVisitor.visit(m_rendererCommandBuffer);
    }

    const std::string     m_defaultFilename;
    RendererCommandBuffer m_rendererCommandBuffer;
    Screenshot            m_cmd;
};

TEST_F(AScreenshot, executesScreenshotNoArgsDefined)
{
    EXPECT_TRUE(m_cmd.executeInput(std::vector<std::string>{}));
    expectScreenshotCommand(m_defaultFilename);
}

TEST_F(AScreenshot, executesScreenshotWithFilename)
{
    const std::string filename("someFilename.png");
    EXPECT_TRUE(m_cmd.executeInput({"-filename", filename}));
    expectScreenshotCommand(filename);
}

TEST_F(AScreenshot, executesScreenshotWithOffscreenBuffer)
{
    const std::string filename("someFilename.png");
    EXPECT_TRUE(m_cmd.executeInput({"-filename", filename, "-ob", "42"}));
    expectScreenshotCommand(filename, DisplayHandle(0), true, OffscreenBufferHandle(42));
}

TEST_F(AScreenshot, executesScreenshotWithDisplay)
{
    const DisplayHandle displayHandle(23u);
    EXPECT_TRUE(m_cmd.executeInput(std::vector<std::string>{"-displayId",  "23"}));
    expectScreenshotCommand(m_defaultFilename, displayHandle);
}

TEST_F(AScreenshot, brokenArgumentsAreNotExecuted)
{
    EXPECT_FALSE(m_cmd.executeInput(std::vector<std::string>{"foo"}));
    EXPECT_FALSE(m_cmd.executeInput(std::vector<std::string>{"-foo"}));
    EXPECT_FALSE(m_cmd.executeInput(std::vector<std::string>{"-filename", "firstfile.png", "secondfile.png"}));
    EXPECT_FALSE(m_cmd.executeInput(std::vector<std::string>{"-displayId", "0", "1", "2"}));
}

TEST_F(AScreenshot, emptyOptionsAreValid)
{
    EXPECT_TRUE(m_cmd.executeInput(std::vector<std::string>{"-filename", "-displayId"}));
    expectScreenshotCommand(m_defaultFilename);
}
