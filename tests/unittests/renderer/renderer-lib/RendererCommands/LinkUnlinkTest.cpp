//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "internal/RendererLib/RamshCommands/LinkUnlink.h"
#include "internal/RendererLib/RendererCommandBuffer.h"
#include "RendererCommandVisitorMock.h"

using namespace ::testing;

namespace ramses::internal
{
    class ARamshLinkBuffer : public ::testing::Test
    {
    public:
        ARamshLinkBuffer()
            : m_cmd(m_rendererCommandBuffer)
        {
        }

    protected:
        void expectCommand(const OffscreenBufferHandle& obHandle, const SceneId& scene, const DataSlotId& slot)
        {
            StrictMock<RendererCommandVisitorMock> cmdVisitor;
            EXPECT_CALL(cmdVisitor, handleBufferToSceneDataLinkRequest(obHandle, scene, slot));
            cmdVisitor.visit(m_rendererCommandBuffer);
        }

        RendererCommandBuffer m_rendererCommandBuffer;
        LinkBuffer m_cmd;
    };

    TEST_F(ARamshLinkBuffer, missingArguments)
    {
        EXPECT_FALSE(m_cmd.executeInput(std::vector<std::string>{"link"}));
        EXPECT_FALSE(m_cmd.executeInput(std::vector<std::string>{}));
    }

    TEST_F(ARamshLinkBuffer, linkOffscreenBuffer)
    {
        EXPECT_TRUE(m_cmd.executeInput({"link", "433000001", "26589712", "42"}));
        expectCommand(OffscreenBufferHandle(42), SceneId(433000001), DataSlotId(26589712));
    }

    class ARamshUnlinkBuffer : public ::testing::Test
    {
    public:
        ARamshUnlinkBuffer()
            : m_cmd(m_rendererCommandBuffer)
        {
        }

    protected:
        void expectCommand(const SceneId& scene, const DataSlotId& slot)
        {
            StrictMock<RendererCommandVisitorMock> cmdVisitor;
            EXPECT_CALL(cmdVisitor, handleDataUnlinkRequest(scene, slot));
            cmdVisitor.visit(m_rendererCommandBuffer);
        }

        RendererCommandBuffer m_rendererCommandBuffer;
        UnlinkBuffer m_cmd;
    };

    TEST_F(ARamshUnlinkBuffer, missingArguments)
    {
        EXPECT_FALSE(m_cmd.executeInput(std::vector<std::string>{"unlink"}));
        EXPECT_FALSE(m_cmd.executeInput(std::vector<std::string>{}));
    }

    TEST_F(ARamshUnlinkBuffer, unlinkOffscreenBuffer)
    {
        EXPECT_TRUE(m_cmd.executeInput({"unlink", "433000001", "26589712"}));
        expectCommand(SceneId(433000001), DataSlotId(26589712));
    }
}
