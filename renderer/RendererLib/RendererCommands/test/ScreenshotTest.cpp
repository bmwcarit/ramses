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

using namespace ramses_internal;
using namespace ::testing;

class AScreenshot : public ::testing::Test
{
public:
    AScreenshot()
        : m_defaultFilename("unnamed.bmp")
        , m_cmd(m_rendererCommandBuffer)
    {
        m_rendererCommandBuffer.clear();
    }

protected:
    void expectScreenshotCommand(const String& filename, const DisplayHandle& displayHandle = DisplayHandle(0u), bool autoSize = true)
    {
        const RendererCommandContainer& commands = m_rendererCommandBuffer.getCommands();
        EXPECT_EQ(1u, commands.getTotalCommandCount());
        EXPECT_EQ(ERendererCommand_ReadPixels, commands.getCommandType(0u));
        const ReadPixelsCommand& command = commands.getCommandData<ReadPixelsCommand>(0u);
        EXPECT_EQ(filename, command.filename);
        EXPECT_EQ(command.displayHandle, displayHandle);
        EXPECT_EQ(autoSize, command.fullScreen);
    }

    const String          m_defaultFilename;
    RendererCommandBuffer m_rendererCommandBuffer;
    Screenshot            m_cmd;
};

TEST_F(AScreenshot, executesScreenshotNoArgsDefined)
{
    RamshInput args;

    EXPECT_TRUE(m_cmd.executeInput(args));
    expectScreenshotCommand(m_defaultFilename);
}

TEST_F(AScreenshot, executesScreenshotWithFilename)
{
    const String filename("someFilename.bmp");
    RamshInput args;
    args.append("-filename");
    args.append(filename);

    EXPECT_TRUE(m_cmd.executeInput(args));
    expectScreenshotCommand(filename);
}

TEST_F(AScreenshot, executesScreenshotWithDisplay)
{
    const DisplayHandle displayHandle(23u);

    RamshInput args;
    args.append("-displayId");
    args.append("23");

    EXPECT_TRUE(m_cmd.executeInput(args));
    expectScreenshotCommand(m_defaultFilename, displayHandle);
}

TEST_F(AScreenshot, brokenArgumentsAreNotExecuted)
{
    RamshInput argsWithoutOption;
    argsWithoutOption.append("foo");

    RamshInput argsWithMultipleFilenames;
    argsWithMultipleFilenames.append("-filename");
    argsWithMultipleFilenames.append("firstfile.bmp");
    argsWithMultipleFilenames.append("secondfile.bmp");

    RamshInput argsWithMultipleDisplayIDs;
    argsWithMultipleDisplayIDs.append("-displayId");
    argsWithMultipleDisplayIDs.append("0");
    argsWithMultipleDisplayIDs.append("1");
    argsWithMultipleDisplayIDs.append("2");

    EXPECT_FALSE(m_cmd.executeInput(argsWithoutOption));
    EXPECT_FALSE(m_cmd.executeInput(argsWithMultipleFilenames));
    EXPECT_FALSE(m_cmd.executeInput(argsWithMultipleDisplayIDs));
}

TEST_F(AScreenshot, emptyOptionsAreValid)
{
    RamshInput args;
    args.append("-filename");
    args.append("-displayId");

    EXPECT_TRUE(m_cmd.executeInput(args));
    expectScreenshotCommand(m_defaultFilename);
}
