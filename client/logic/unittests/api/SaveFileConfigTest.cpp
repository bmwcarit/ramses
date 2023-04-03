//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "LogTestUtils.h"

#include "ramses-logic/SaveFileConfig.h"
#include "impl/SaveFileConfigImpl.h"

#include "fmt/format.h"

namespace rlogic::internal
{
    class ASaveFileConfig : public ::testing::Test
    {
    protected:
        static void setValues(SaveFileConfig& config)
        {
            config.setMetadataString("metadata");
            config.setExporterVersion(1u, 2u, 3u, 4u);
            config.setValidationEnabled(false);
            config.setLuaSavingMode(ELuaSavingMode::SourceAndByteCode);
        }

        static void checkValues(const SaveFileConfig& config)
        {
            EXPECT_EQ("metadata", config.m_impl->getMetadataString());
            EXPECT_EQ(1u, config.m_impl->getExporterMajorVersion());
            EXPECT_EQ(2u, config.m_impl->getExporterMinorVersion());
            EXPECT_EQ(3u, config.m_impl->getExporterPatchVersion());
            EXPECT_EQ(4u, config.m_impl->getExporterFileFormatVersion());
            EXPECT_FALSE(config.m_impl->getValidationEnabled());
            EXPECT_EQ(ELuaSavingMode::SourceAndByteCode, config.m_impl->getLuaSavingMode());
        }
    };

    TEST_F(ASaveFileConfig, HasDefaultValues)
    {
        SaveFileConfig config;
        EXPECT_TRUE(config.m_impl->getMetadataString().empty());
        EXPECT_EQ(0u, config.m_impl->getExporterMajorVersion());
        EXPECT_EQ(0u, config.m_impl->getExporterMinorVersion());
        EXPECT_EQ(0u, config.m_impl->getExporterPatchVersion());
        EXPECT_EQ(0u, config.m_impl->getExporterFileFormatVersion());
        EXPECT_TRUE(config.m_impl->getValidationEnabled());
        EXPECT_EQ(ELuaSavingMode::SourceAndByteCode, config.m_impl->getLuaSavingMode());
    }

    TEST_F(ASaveFileConfig, IsCopied)
    {
        SaveFileConfig config;
        setValues(config);
        SaveFileConfig configCopy(config);
        checkValues(configCopy);
    }

    TEST_F(ASaveFileConfig, IsCopyAssigned)
    {
        SaveFileConfig config;
        setValues(config);
        SaveFileConfig configCopy;
        configCopy = config;
        checkValues(configCopy);
    }

    TEST_F(ASaveFileConfig, IsMoved)
    {
        SaveFileConfig config;
        setValues(config);
        SaveFileConfig movedConfig(std::move(config));
        checkValues(movedConfig);
    }

    TEST_F(ASaveFileConfig, IsMoveAssigned)
    {
        SaveFileConfig config;
        setValues(config);
        SaveFileConfig movedAssigned;
        movedAssigned = std::move(config);
        checkValues(movedAssigned);
    }
}
