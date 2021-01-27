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
    void expectScreenshotCommand(const String& filename, const DisplayHandle& displayHandle = DisplayHandle(0u), bool autoSize = true)
    {
        StrictMock<RendererCommandVisitorMock> cmdVisitor;
        EXPECT_CALL(cmdVisitor, handleReadPixels(displayHandle, OffscreenBufferHandle::Invalid(), _, _, _, _, autoSize, filename));
        cmdVisitor.visit(m_rendererCommandBuffer);
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
    const String filename("someFilename.png");
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
    argsWithMultipleFilenames.append("firstfile.png");
    argsWithMultipleFilenames.append("secondfile.png");

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
