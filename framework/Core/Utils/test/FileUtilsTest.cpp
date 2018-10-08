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
    TEST(FileUtilsTest, GetWorkingDirectory)
    {
        //depending on where the test is started the path can be different here
        //it makes most scene to check that a string with more than 0 characters
        //is returned.
        String workingDir = FileUtils::GetCurrentWorkingDirectory().getPath();
        EXPECT_TRUE(workingDir.getLength()>0);
    }

    TEST(FileUtilsTest, CreateDirectoriesIncorrectPathZero)
    {
        //no directory is created but an error code is returned.
        String workingDir = "";

        File directoryToCreate(workingDir);

        EStatus status = FileUtils::CreateDirectories(directoryToCreate);
        EXPECT_TRUE(status == EStatus_RAMSES_ERROR);
    }

    TEST(FileUtilsTest, CreateAndRemoveDirectory)
    {
        //create and delete are both tested here because
        //-a directory that is not deleted after the test (create without remove) is bad
        //-remove needs a directory that actually exists, no benefit of testing remove alone
        String workingDir = FileUtils::GetCurrentWorkingDirectory().getPath();
        workingDir += "/temporary1";

        File directory(workingDir);

        // first create the new directory
        EStatus status = FileUtils::CreateDirectories(directory);
        EXPECT_TRUE(status == EStatus_RAMSES_OK);

        //remove the newly created directory
        status = FileUtils::RemoveDirectory(directory);
        EXPECT_TRUE(status == EStatus_RAMSES_OK);
    }

    TEST(FileUtilsTest, CreateAlreadyExistingDirectory)
    {
        //if trying to create an already existing directory nothing happens
        //returned status is OK
        String workingDir = FileUtils::GetCurrentWorkingDirectory().getPath();
        workingDir += "/temporary2";

        File nameClash(workingDir);

        nameClash.createDirectory();

        EStatus status = FileUtils::CreateDirectories(nameClash);
        EXPECT_TRUE(status == EStatus_RAMSES_OK);

        status = FileUtils::RemoveDirectory(nameClash);
        EXPECT_TRUE(status == EStatus_RAMSES_OK);
    }

    TEST(FileUtilsTest, CreateDirectoryWithSameNameAsFile)
    {
        //if trying to create an directory with the same name as a file in the same location
        //an error will be returned
        String workingDir = FileUtils::GetCurrentWorkingDirectory().getPath();
        workingDir += "/temporary3";

        File nameClash(workingDir);

        nameClash.createFile();

        EStatus status = FileUtils::CreateDirectories(nameClash);
        EXPECT_TRUE(status == EStatus_RAMSES_ERROR);
    }

    TEST(FileUtilsTest, RecursiveDirectoryCreate)
    {
        //all subdirectories created in a single step
        //the check itself is recursive
        String workingDir = FileUtils::GetCurrentWorkingDirectory().getPath();
        File directories(workingDir + "/temporary4/sub/directory/another");

        EStatus status = FileUtils::CreateDirectories(directories);
        EXPECT_TRUE(status == EStatus_RAMSES_OK);

        String check(workingDir + "/temporary4");
        File checkDirectory(check);

        EXPECT_TRUE(checkDirectory.exists() && checkDirectory.isDirectory());

        check = workingDir + "/temporary4/sub/";
        checkDirectory = File(check);

        EXPECT_TRUE(checkDirectory.exists() && checkDirectory.isDirectory());

        check = workingDir + "/temporary4/sub/directory/";
        checkDirectory = File(check);

        EXPECT_TRUE(checkDirectory.exists() && checkDirectory.isDirectory());

        check = workingDir + "/temporary4/sub/directory/another/";
        checkDirectory = File(check);

        EXPECT_TRUE(checkDirectory.exists() && checkDirectory.isDirectory());
    }

    TEST(FileUtilsTest, RemoveDirectoryThatDoesNotExist)
    {
        String workingDir = FileUtils::GetCurrentWorkingDirectory().getPath();
        workingDir += "/temporary5";

        File directoryToRemove(workingDir);

        //remove the non-existing directory
        EStatus status = FileUtils::RemoveDirectory(directoryToRemove);
        EXPECT_TRUE(status == EStatus_RAMSES_ERROR);
    }

    TEST(FileUtilsTest, SetWorkingDirectoryNotExist)
    {
        //should lead to an error
        String workingDir = FileUtils::GetCurrentWorkingDirectory().getPath();
        EStatus status = FileUtils::SetCurrentWorkingDirectory(File(workingDir + "/temporary6"));
        EXPECT_TRUE(status == EStatus_RAMSES_ERROR);
    }

    TEST(FileUtilsTest, SetWorkingDirectoryExist)
    {
        String workingDir = FileUtils::GetCurrentWorkingDirectory().getPath();
        String newWorkingDir = workingDir +"/temporary7";

        File newWorkingDirectory(newWorkingDir);

        // first create the new directory but with a different name than before
        EStatus status = FileUtils::CreateDirectories(newWorkingDirectory);
        EXPECT_TRUE(status == EStatus_RAMSES_OK);

        status = FileUtils::SetCurrentWorkingDirectory(newWorkingDirectory);
        EXPECT_TRUE(status == EStatus_RAMSES_OK);

        String workingDirAfterSet = FileUtils::GetCurrentWorkingDirectory().getPath();

        // on windows newWorkingDir contains one slash and multiple backslashes
        // use slashes for newWorkingDir and workingDirAfterSet on all systems to ease the comparison
        workingDirAfterSet = workingDirAfterSet.replace("\\","/");
        newWorkingDir = newWorkingDir.replace("\\","/");
        EXPECT_EQ(workingDirAfterSet,newWorkingDir);

        //now back to old working directory, else it is possible that the text/read write tests fail
        File backToOldWorkingDirectory(workingDir);
        status = FileUtils::SetCurrentWorkingDirectory(backToOldWorkingDirectory);
        EXPECT_TRUE(status == EStatus_RAMSES_OK);

        status = FileUtils::RemoveDirectory(newWorkingDirectory);
        EXPECT_TRUE(status == EStatus_RAMSES_OK);
    }

    TEST(FileUtilsTest, ReadTextFromFile)
    {
        String textFile = FileUtils::GetCurrentWorkingDirectory().getPath();
        textFile += "/res/test-text-data.txt";
        File textDataFile(textFile);

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
        String textFile = FileUtils::GetCurrentWorkingDirectory().getPath();
        textFile += "/res/test-text-data-empty.txt";
        File textDataFile(textFile);

        EXPECT_TRUE(textDataFile.exists() == false);

        textDataFile.createFile();

        EXPECT_TRUE(textDataFile.exists() == true);

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
        String textFile = FileUtils::GetCurrentWorkingDirectory().getPath();
        textFile += "/res/test-text-data-does-not-exist.txt";
        File textDataFile(textFile);

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

        String textFile = FileUtils::GetCurrentWorkingDirectory().getPath();
        textFile += "/test-text-data-file-created.txt";
        File writeTextToFile(textFile);

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

        String textFile = FileUtils::GetCurrentWorkingDirectory().getPath();
        textFile += "/test-text-data-file-created.txt";
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
