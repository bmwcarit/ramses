//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"
#include "Utils/FileUtils.h"

namespace ramses_internal
{
    TEST(FileUtilsTest, ReadTextFromFile)
    {
        File textDataFile("res/test-text-data.txt");

        EXPECT_TRUE(textDataFile.exists());

        String readText = FileUtils::ReadAllText(textDataFile);
        uint32_t textSize = static_cast<uint32_t>(readText.getLength());
        ASSERT_EQ(479u, textSize);

        char expected[] = {'/', ' ', 'h', ' ', '\n'};
        for(uint32_t i = 0 ; i < 5 ; i++)
        {
            EXPECT_EQ(readText[i * 100], expected[i]);
        }
    }

    TEST(FileUtilsTest, ReadFromEmptyFile)
    {
        //there is no returning error code, so an empty file cannot be distinguished
        //from a non-existing file. Both return a string of length 0
        File textDataFile("res/test-text-data-empty.txt");

        EXPECT_FALSE(textDataFile.exists());
        textDataFile.createFile();
        EXPECT_TRUE(textDataFile.exists());

        String readText = FileUtils::ReadAllText(textDataFile);
        uint32_t textSize = static_cast<uint32_t>(readText.getLength());
        EXPECT_EQ(0ul,textSize);

        //finally remove file after test
        textDataFile.remove();
    }

    TEST(FileUtilsTest, ReadFromNonExistingFile)
    {
        //there is no returning error code, so an empty file cannot be distinguished
        //from a non-existing file. Both return a string of length 0
        File textDataFile("res/test-text-data-does-not-exist.txt");

        EXPECT_TRUE(textDataFile.exists() == false);

        String readText = FileUtils::ReadAllText(textDataFile);
        uint32_t textSize = static_cast<uint32_t>(readText.getLength());
        EXPECT_EQ(0ul,textSize);

    }

    TEST(FileUtilsTest, WriteTextToFile)
    {
        //file does not exist and will be created
        const String writeText = String("No ius eloquentiam accommodare, perpetua explicari forensibus id nam,"
                                        "eos viris invenire scribentur in. Eos nobis periculis id, cu laudem "
                                        "dolorum intellegat mel, ei expetenda consectetuer quo. Quo an quem nemore."
                                        "Pri in semper dignissim, per ut purto invenire complectitur. "
                                        "Vivendo neglegentur pri an, nisl veri pro eu.\n");

        File writeTextToFile("test-text-data-file-created.txt");

        EStatus status = FileUtils::WriteAllText(writeTextToFile, writeText);
        EXPECT_TRUE(status == EStatus_RAMSES_OK);

        String readText = FileUtils::ReadAllText(writeTextToFile);
        uint32_t textSize = static_cast<uint32_t>(readText.getLength());
        EXPECT_EQ(318ul,textSize);

        //finally remove file after test
        writeTextToFile.remove();
    }

    TEST(FileUtilsTest, WriteTextOverwriteFile)
    {
        //expectation: no error
        const String writeText = String("No ius eloquentiam accommodare, perpetua explicari forensibus id nam,"
                                        "eos viris invenire scribentur in. Eos nobis periculis id, cu laudem "
                                        "dolorum intellegat mel, ei expetenda consectetuer quo. Quo an quem nemore."
                                        "Pri in semper dignissim, per ut purto invenire complectitur. "
                                        "Vivendo neglegentur pri an, nisl veri pro eu. In vino veritas.\n");

        String textFile = "test-text-data-file-created.txt";
        File writeTextToFile(textFile);

        EStatus status = FileUtils::WriteAllText(writeTextToFile, writeText);
        EXPECT_TRUE(status == EStatus_RAMSES_OK);

        status = FileUtils::WriteAllText(writeTextToFile, writeText);
        EXPECT_TRUE(status == EStatus_RAMSES_OK);

        String readText = FileUtils::ReadAllText(writeTextToFile);
        uint32_t textSize = static_cast<uint32_t>(readText.getLength());
        EXPECT_EQ(335ul,textSize);//overwritten file should only have a single instance of the above text

        //finally remove file after test
        writeTextToFile.remove();
    }
}
