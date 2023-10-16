//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ClientTestUtils.h"
#include "internal/Core/Utils/File.h"

#include <gtest/gtest.h>

namespace ramses::internal
{
    using ramses::internal::File;

    class DumpSceneToFileForTests : public LocalTestClientWithScene, public ::testing::Test
    {
    protected:

        void SetUp() override
        {
            auto file = getSceneDumpFile();
            if (file.exists())
            {
                ASSERT_TRUE(file.remove());
            }
        }

        File getSceneDumpFile() const
        {
            return File(fmt::format("{}{}.ramses", testing::TempDir(), m_scene.getSceneId()).c_str());
        }
    };

    TEST_F(DumpSceneToFileForTests, badParameters)
    {
        EXPECT_FALSE(getFramework().executeRamshCommand("dumpSceneToFile"));
        EXPECT_FALSE(getFramework().executeRamshCommand(fmt::format("dumpSceneToFile {}", m_scene.getSceneId())));
        EXPECT_FALSE(getFramework().executeRamshCommand(fmt::format("dumpSceneToFile notAScene filename", m_scene.getSceneId())));
    }

    TEST_F(DumpSceneToFileForTests, dumpSceneWithNextFlush)
    {
        const auto filename = fmt::format("{}{}", testing::TempDir(), m_scene.getSceneId()); // .ramses is added internally
        EXPECT_TRUE(getFramework().executeRamshCommand(fmt::format("dumpSceneToFile {} {}", m_scene.getSceneId(), filename)));
        EXPECT_FALSE(getSceneDumpFile().exists());
        m_scene.flush();
        EXPECT_TRUE(getSceneDumpFile().exists());
    }

    TEST_F(DumpSceneToFileForTests, dumpSceneAndFlush)
    {
        const auto filename = fmt::format("{}{}", testing::TempDir(), m_scene.getSceneId()); // .ramses is added internally
        EXPECT_FALSE(getSceneDumpFile().exists());
        EXPECT_TRUE(getFramework().executeRamshCommand(fmt::format("dumpSceneToFile -flush {} {}", m_scene.getSceneId(), filename)));
        EXPECT_TRUE(getSceneDumpFile().exists());
    }

    TEST_F(DumpSceneToFileForTests, dumpSceneAndFlushInvalidScene)
    {
        const auto filename = fmt::format("{}{}", testing::TempDir(), m_scene.getSceneId()); // .ramses is added internally
        EXPECT_FALSE(getSceneDumpFile().exists());
        EXPECT_FALSE(getFramework().executeRamshCommand(fmt::format("dumpSceneToFile -flush {} {}", m_scene.getSceneId().getValue() + 1, filename)));
        EXPECT_FALSE(getSceneDumpFile().exists());
    }

    TEST_F(DumpSceneToFileForTests, dumpSceneToDltNoFile)
    {
        EXPECT_TRUE(getFramework().executeRamshCommand(fmt::format("dumpScene -sendViaDLT -flush {}", m_scene.getSceneId())));
        EXPECT_FALSE(getSceneDumpFile().exists());
    }

    TEST_F(DumpSceneToFileForTests, dumpSceneToDltStoreFile)
    {
        EXPECT_FALSE(getSceneDumpFile().exists());
        const auto filename = fmt::format("{}{}", testing::TempDir(), m_scene.getSceneId()); // .ramses is added internally
        EXPECT_TRUE(getFramework().executeRamshCommand(fmt::format("dumpScene -sendViaDLT -flush {} {}", m_scene.getSceneId(), filename)));
        EXPECT_TRUE(getSceneDumpFile().exists());
    }
}
