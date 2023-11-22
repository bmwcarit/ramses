//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "LogTestUtils.h"

#include "ramses/client/SaveFileConfig.h"
#include "impl/SaveFileConfigImpl.h"

#include "fmt/format.h"

namespace ramses::internal
{
    class ASaveFileConfig : public ::testing::Test
    {
    protected:
        static void setValues(SaveFileConfig& config)
        {
            config.setMetadataString("metadata");
            config.setExporterVersion(1u, 2u, 3u, 4u);
            config.setCompressionEnabled(true);
            config.setLuaSavingMode(ELuaSavingMode::SourceAndByteCode);
        }

        static void checkValues(const SaveFileConfig& config)
        {
            EXPECT_EQ("metadata", config.impl().getMetadataString());
            EXPECT_EQ(1u, config.impl().getExporterVersion().major);
            EXPECT_EQ(2u, config.impl().getExporterVersion().minor);
            EXPECT_EQ(3u, config.impl().getExporterVersion().patch);
            EXPECT_EQ(4u, config.impl().getExporterVersion().fileFormat);
            EXPECT_EQ(ELuaSavingMode::SourceAndByteCode, config.impl().getLuaSavingMode());
            EXPECT_EQ("'metadata' exporter:1.2.3.4 compress:true lua:2", fmt::to_string(config.impl()));
        }
    };

    TEST_F(ASaveFileConfig, HasDefaultValues)
    {
        SaveFileConfig config;
        EXPECT_TRUE(config.impl().getMetadataString().empty());
        EXPECT_EQ(0u, config.impl().getExporterVersion().major);
        EXPECT_EQ(0u, config.impl().getExporterVersion().minor);
        EXPECT_EQ(0u, config.impl().getExporterVersion().patch);
        EXPECT_EQ(0u, config.impl().getExporterVersion().fileFormat);
        EXPECT_EQ(ELuaSavingMode::SourceAndByteCode, config.impl().getLuaSavingMode());
        EXPECT_EQ("'' exporter:0.0.0.0 compress:false lua:2", fmt::to_string(config.impl()));
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
