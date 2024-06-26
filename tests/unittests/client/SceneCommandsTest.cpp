//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ClientTestUtils.h"
#include "internal/Core/Utils/File.h"
#include "ramses/client/UniformInput.h"

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

    class ASetProperty : public LocalTestClientWithScene, public ::testing::Test
    {
    protected:
        bool ramsh(const std::string& cmd)
        {
            auto ret = getFramework().executeRamshCommand(cmd);
            getScene().flush();
            return ret;
        }
        MeshNode& m_mesh = createValidMeshNode();
        Appearance* m_appearance = m_mesh.getAppearance();
    };

    TEST_F(ASetProperty, badParameters)
    {
        EXPECT_FALSE(ramsh("setprop"));
        EXPECT_FALSE(ramsh(fmt::format("setprop {}", m_scene.getSceneId())));
        EXPECT_FALSE(ramsh(fmt::format("setprop notAScene", m_scene.getSceneId())));
        EXPECT_FALSE(ramsh(fmt::format("setprop {} notanobject", m_scene.getSceneId())));
        EXPECT_FALSE(ramsh(fmt::format("setprop {} notanobject", m_scene.getSceneId())));
        EXPECT_FALSE(ramsh(fmt::format("setprop {} {}", m_scene.getSceneId(), m_mesh.getSceneObjectId().getValue())));
        EXPECT_FALSE(ramsh(fmt::format("setprop {} {} notaproperty", m_scene.getSceneId(), m_mesh.getSceneObjectId().getValue())));
        EXPECT_FALSE(ramsh(fmt::format("setprop {} {} vis 2 extraparam", m_scene.getSceneId(), m_mesh.getSceneObjectId().getValue())));

        EXPECT_FALSE(ramsh(fmt::format("setall {}", m_scene.getSceneId())));
        EXPECT_FALSE(ramsh(fmt::format("setall {} notAType", m_scene.getSceneId())));
        EXPECT_FALSE(ramsh(fmt::format("setall {} notAType prop value", m_scene.getSceneId())));
        EXPECT_FALSE(ramsh(fmt::format("setall {} mesh vis 2 extraparam", m_scene.getSceneId())));
    }

    TEST_F(ASetProperty, nodeVisible)
    {
        EXPECT_EQ(ramses::EVisibilityMode::Visible, m_mesh.getVisibility());
        EXPECT_TRUE(ramsh(fmt::format("setprop {} {} visible 1", m_scene.getSceneId(), m_mesh.getSceneObjectId().getValue())));
        EXPECT_EQ(ramses::EVisibilityMode::Invisible, m_mesh.getVisibility());
        EXPECT_TRUE(ramsh(fmt::format("setprop {} {} visible 2", m_scene.getSceneId(), m_mesh.getSceneObjectId().getValue())));
        EXPECT_EQ(ramses::EVisibilityMode::Visible, m_mesh.getVisibility());
        EXPECT_TRUE(ramsh(fmt::format("setprop {} {} visible 0", m_scene.getSceneId(), m_mesh.getSceneObjectId().getValue())));
        EXPECT_EQ(ramses::EVisibilityMode::Off, m_mesh.getVisibility());
        EXPECT_TRUE(ramsh(fmt::format("setprop {} {} visible invis", m_scene.getSceneId(), m_mesh.getSceneObjectId().getValue())));
        EXPECT_EQ(ramses::EVisibilityMode::Invisible, m_mesh.getVisibility());
        EXPECT_TRUE(ramsh(fmt::format("setprop {} {} visible visi", m_scene.getSceneId(), m_mesh.getSceneObjectId().getValue())));
        EXPECT_EQ(ramses::EVisibilityMode::Visible, m_mesh.getVisibility());
        EXPECT_TRUE(ramsh(fmt::format("setprop {} {} visible off", m_scene.getSceneId(), m_mesh.getSceneObjectId().getValue())));
        EXPECT_EQ(ramses::EVisibilityMode::Off, m_mesh.getVisibility());

        EXPECT_TRUE(ramsh(fmt::format("setall {} mesh visible vis", m_scene.getSceneId())));
        EXPECT_EQ(ramses::EVisibilityMode::Visible, m_mesh.getVisibility());
        EXPECT_TRUE(ramsh(fmt::format("setall {} meshnode visible 0", m_scene.getSceneId())));
        EXPECT_EQ(ramses::EVisibilityMode::Off, m_mesh.getVisibility());
    }

    TEST_F(ASetProperty, uniform)
    {
        auto uniform = m_appearance->getEffect().findUniformInput("u_FragColorR");
        float                value = -1.f;
        ASSERT_TRUE(uniform.has_value());
        EXPECT_TRUE(m_appearance->getInputValue(*uniform, value));
        EXPECT_FLOAT_EQ(0.f, value);
        EXPECT_TRUE(ramsh(fmt::format("setprop {} {} uniform.u_FragColorR 1", m_scene.getSceneId(), m_appearance->getSceneObjectId().getValue())));
        EXPECT_TRUE(m_appearance->getInputValue(*uniform, value));
        EXPECT_FLOAT_EQ(1.f, value);
        EXPECT_TRUE(ramsh(fmt::format("setall {} appear uniform.u_FragColorR 0.5", m_scene.getSceneId())));
        EXPECT_TRUE(m_appearance->getInputValue(*uniform, value));
        EXPECT_FLOAT_EQ(0.5f, value);
    }

    TEST_F(ASetProperty, depth_func)
    {
        EDepthFunc func = EDepthFunc::Disabled;
        EXPECT_TRUE(m_appearance->getDepthFunction(func));
        EXPECT_EQ(EDepthFunc::LessEqual, func);
        EXPECT_TRUE(ramsh(fmt::format("setprop {} {} depth.func always", m_scene.getSceneId(), m_appearance->getSceneObjectId().getValue())));
        EXPECT_TRUE(m_appearance->getDepthFunction(func));
        EXPECT_EQ(EDepthFunc::Always, func);
        EXPECT_TRUE(ramsh(fmt::format("setprop {} {} depth.func disabled", m_scene.getSceneId(), m_appearance->getSceneObjectId().getValue())));
        EXPECT_TRUE(m_appearance->getDepthFunction(func));
        EXPECT_EQ(EDepthFunc::Disabled, func);
        EXPECT_TRUE(ramsh(fmt::format("setprop {} {} depth.func less", m_scene.getSceneId(), m_appearance->getSceneObjectId().getValue())));
        EXPECT_TRUE(m_appearance->getDepthFunction(func));
        EXPECT_EQ(EDepthFunc::Less, func);
        EXPECT_TRUE(ramsh(fmt::format("setprop {} {} depth.func lessEqual", m_scene.getSceneId(), m_appearance->getSceneObjectId().getValue())));
        EXPECT_TRUE(m_appearance->getDepthFunction(func));
        EXPECT_EQ(EDepthFunc::LessEqual, func);
        EXPECT_TRUE(ramsh(fmt::format("setall {} appear depth.func always", m_scene.getSceneId())));
        EXPECT_TRUE(m_appearance->getDepthFunction(func));
        EXPECT_EQ(EDepthFunc::Always, func);
    }

    TEST_F(ASetProperty, depth_write)
    {
        EDepthWrite depthWrite = EDepthWrite::Disabled;
        EXPECT_TRUE(m_appearance->getDepthWriteMode(depthWrite));
        EXPECT_EQ(EDepthWrite::Enabled, depthWrite);
        EXPECT_TRUE(ramsh(fmt::format("setprop {} {} depth.write 0", m_scene.getSceneId(), m_appearance->getSceneObjectId().getValue())));
        EXPECT_TRUE(m_appearance->getDepthWriteMode(depthWrite));
        EXPECT_EQ(EDepthWrite::Disabled, depthWrite);
        EXPECT_TRUE(ramsh(fmt::format("setprop {} {} depth.write 1", m_scene.getSceneId(), m_appearance->getSceneObjectId().getValue())));
        EXPECT_TRUE(m_appearance->getDepthWriteMode(depthWrite));
        EXPECT_EQ(EDepthWrite::Enabled, depthWrite);
        EXPECT_TRUE(ramsh(fmt::format("setprop {} {} depth.write off", m_scene.getSceneId(), m_appearance->getSceneObjectId().getValue())));
        EXPECT_TRUE(m_appearance->getDepthWriteMode(depthWrite));
        EXPECT_EQ(EDepthWrite::Disabled, depthWrite);
        EXPECT_TRUE(ramsh(fmt::format("setprop {} {} depth.write on", m_scene.getSceneId(), m_appearance->getSceneObjectId().getValue())));
        EXPECT_TRUE(m_appearance->getDepthWriteMode(depthWrite));
        EXPECT_EQ(EDepthWrite::Enabled, depthWrite);
        EXPECT_TRUE(ramsh(fmt::format("setall {} app depth.write off", m_scene.getSceneId())));
        EXPECT_TRUE(m_appearance->getDepthWriteMode(depthWrite));
        EXPECT_EQ(EDepthWrite::Disabled, depthWrite);
    }

}
