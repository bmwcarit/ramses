//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "FilePathsConfig.h"
#include "gtest/gtest.h"

namespace
{
    void expectLoadFilePathesConfig(const char* configFile, bool succeed)
    {
        FilePathsConfig config;
        EXPECT_EQ(succeed, config.loadFromFile(configFile));
    }
}

TEST(AFilePathsConfig, canLoadCorrectlyWhenAllLinesAreValid)
{
    FilePathsConfig config;
    EXPECT_TRUE(config.loadFromFile("res/ramses-resource-tools-test.filepathesconfig"));

    const FilePaths& filePaths = config.getFilePaths();
    EXPECT_TRUE(filePaths.contains("res/ramses-resource-tools-test1.res"));
    EXPECT_TRUE(filePaths.contains("res/ramses-resource-tools-test2.res"));
    EXPECT_TRUE(filePaths.contains("res/ramses-resource-tools-test3.res"));
    EXPECT_TRUE(filePaths.contains("res/ramses-resource-tools-test4.res"));
}

TEST(AFilePathsConfig, isFineToLoadValidFiles)
{
    expectLoadFilePathesConfig("res/ramses-resource-tools-empty.filepathesconfig", true);
    expectLoadFilePathesConfig("res/ramses-resource-tools-valid-with-additional-spaces-and-empty-lines.filepathesconfig", true);
}

TEST(AFilePathsConfig, reportsErrorWhenLoadANonExistConfig)
{
    expectLoadFilePathesConfig("res/ramses-resource-tools-nonexist.filepathesconfig", false);
}

TEST(AFilePathsConfig, reportsErrorWhenLoadAConfigContainingInvalidFilePath)
{
    expectLoadFilePathesConfig("res/ramses-resource-tools-invalid-wrong-num-tokens.filepathesconfig", false);
    expectLoadFilePathesConfig("res/ramses-resource-tools-invalid-non-exist-filepath.filepathesconfig", false);
    expectLoadFilePathesConfig("res/ramses-resource-tools-invalid-duplicate-filepath.filepathesconfig", false);
}
