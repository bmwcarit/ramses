//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RamsesResourcePackerArguments.h"
#include "gtest/gtest.h"

TEST(ARamsesResourcePackerArguments, canLoadCorrectlyWhenAllParametersAreProvidedWithShortName)
{
    const char* argv[] = { "program.exe", "-ir", "res/ramses-resource-tools-test.filepathesconfig", "-or", "res/ramses-resource-tools-output.res", NULL };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesResourcePackerArguments arguments;
    ASSERT_TRUE(arguments.loadArguments(argc, argv));

    const FilePathsConfig& inputs = arguments.getInputResourceFiles();
    EXPECT_TRUE(inputs.getFilePaths().hasElement("res/ramses-resource-tools-test1.res"));
    EXPECT_TRUE(inputs.getFilePaths().hasElement("res/ramses-resource-tools-test2.res"));
    EXPECT_TRUE(inputs.getFilePaths().hasElement("res/ramses-resource-tools-test3.res"));
    EXPECT_TRUE(inputs.getFilePaths().hasElement("res/ramses-resource-tools-test4.res"));
}

TEST(ARamsesResourcePackerArguments, canLoadCorrectlyWhenAllParametersAreProvidedWithFullName)
{
    const char* argv[] = { "program.exe", "--in-resource-files-config", "res/ramses-resource-tools-test.filepathesconfig", "--out-resource-file", "res/ramses-resource-tools-output.res", NULL };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesResourcePackerArguments arguments;
    ASSERT_TRUE(arguments.loadArguments(argc, argv));

    const FilePathsConfig& inputs = arguments.getInputResourceFiles();
    EXPECT_TRUE(inputs.getFilePaths().hasElement("res/ramses-resource-tools-test1.res"));
    EXPECT_TRUE(inputs.getFilePaths().hasElement("res/ramses-resource-tools-test2.res"));
    EXPECT_TRUE(inputs.getFilePaths().hasElement("res/ramses-resource-tools-test3.res"));
    EXPECT_TRUE(inputs.getFilePaths().hasElement("res/ramses-resource-tools-test4.res"));
}

TEST(ARamsesResourcePackerArguments, reportsErrorWhenInputResourceFilesConfigIsNotAvailable)
{
    const char* argv[] = { "program.exe", "--out-resource-file", "res/ramses-resource-tools-output.res", NULL };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesResourcePackerArguments arguments;
    EXPECT_FALSE(arguments.loadArguments(argc, argv));
}

TEST(ARamsesResourcePackerArguments, reportsErrorWhenOutputResourceFileIsNotAvailable)
{
    const char* argv[] = { "program.exe", "--in-resource-files-config", "res/ramses-resource-tools-test.filepathesconfig", NULL };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesResourcePackerArguments arguments;
    EXPECT_FALSE(arguments.loadArguments(argc, argv));
}

TEST(ARamsesResourcePackerArguments, reportsErrorWhenInputResourceFilesConfigDoesNotExist)
{
    const char* argv[] = { "program.exe", "-ir", "res/ramses-resource-tools-nonexist.filepathesconfig", "-or", "res/ramses-resource-tools-output.res", NULL };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesResourcePackerArguments arguments;
    EXPECT_FALSE(arguments.loadArguments(argc, argv));
}

TEST(ARamsesResourcePackerArguments, reportsErrorWhenInputResourceFilesConfigIsNotValid)
{
    const char* argv[] = { "program.exe", "-ir", "res/ramses-resource-tools-invalid-duplicate-filepath.filepathesconfig", "-or", "res/output.res", NULL };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesResourcePackerArguments arguments;
    EXPECT_FALSE(arguments.loadArguments(argc, argv));
}
