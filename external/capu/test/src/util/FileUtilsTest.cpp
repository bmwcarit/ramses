/*
 * Copyright (C) 2012 BMW Car IT GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "gmock/gmock.h"
#include "ramses-capu/util/FileUtils.h"
#include "ramses-capu/Config.h"
#include <vector>

TEST(FileUtilsTest, TestRemoveDirectory)
{
    // setup
    ramses_capu::File f1("foobarfolder");
    f1.createDirectory();
    ramses_capu::File f2(f1, "foobarfolder2");
    f2.createDirectory();
    ramses_capu::File f3(f2, "foobar.txt");
    f3.createFile();

    EXPECT_EQ(ramses_capu::CAPU_ERROR, f1.remove()); // will not work because folder is not empty

    ramses_capu::status_t retVal = ramses_capu::FileUtils::removeDirectory(f1);
    EXPECT_EQ(ramses_capu::CAPU_OK, retVal);
    EXPECT_FALSE(f1.exists());
}

TEST(FileUtilsTest, CreateAndRemoveDirectories)
{
    // setup
    ramses_capu::File temp1234("temp1/temp2/temp3/temp4");
    ramses_capu::File temp123("temp1/temp2/temp3");
    // execute
    EXPECT_FALSE(temp1234.isDirectory());
    EXPECT_FALSE(temp1234.exists());
    ramses_capu::FileUtils::createDirectories(temp1234);
    EXPECT_TRUE(temp1234.exists());
    EXPECT_TRUE(temp1234.isDirectory());
    temp1234.remove();
    EXPECT_FALSE(temp1234.exists());

    // cleanup

    temp123.remove();
    ramses_capu::File temp12("temp1/temp2");
    temp12.remove();
    ramses_capu::File temp1("temp1");
    temp1.remove();
}


TEST(FileUtilsTest, RecursiveDirectoryDelete)
{
    // setup
    ramses_capu::File temp1234("temp1/temp2/temp3/temp4");
    ramses_capu::File temp123("temp1/temp2/temp3");
    // execute
    EXPECT_FALSE(temp1234.isDirectory());
    EXPECT_FALSE(temp1234.exists());
    ramses_capu::FileUtils::createDirectories(temp1234);
    EXPECT_TRUE(temp1234.exists());
    EXPECT_TRUE(temp1234.isDirectory());
    ramses_capu::FileUtils::removeDirectory(temp123);
    EXPECT_FALSE(temp1234.exists());
    EXPECT_FALSE(temp123.exists());
    // cleanup

    temp123.remove();
    ramses_capu::File temp12("temp1/temp2");
    temp12.remove();
    ramses_capu::File temp1("temp1");
    temp1.remove();
}

TEST(FileUtilsTest, CreateDirectoriesOnFile)
{
    ramses_capu::File temp("temp.txt");
    temp.createFile();
    EXPECT_EQ(ramses_capu::CAPU_ERROR, ramses_capu::FileUtils::createDirectories(temp));
    temp.remove();
}

TEST(FileUtilsTest, CreateDirectoriesOnExistingFolder)
{
    ramses_capu::File temp("temp");
    temp.createDirectory();
    EXPECT_EQ(ramses_capu::CAPU_OK, ramses_capu::FileUtils::createDirectories(temp));
    temp.remove();
}

TEST(FileUtilsTest, ReadWriteAllText)
{
    ramses_capu::File temp("temp.txt");
    EXPECT_FALSE(temp.exists());

    ramses_capu::String content("this is some text\nwith multi lines and\ngerman umlauts öäüß !§$%&/()\r\nas well as some different line breaks and \\ escapes.");
    ramses_capu::FileUtils::writeAllText(temp, content);
    EXPECT_TRUE(temp.exists());

    ramses_capu::String fromFile = ramses_capu::FileUtils::readAllText(temp);

    EXPECT_STREQ(content.c_str(), fromFile.c_str());

    ramses_capu::String content2("this is a shorter text");
    ramses_capu::FileUtils::writeAllText(temp, content2);
    EXPECT_TRUE(temp.exists());

    ramses_capu::String fromFile2 = ramses_capu::FileUtils::readAllText(temp);
    EXPECT_STREQ(content2.c_str(), fromFile2.c_str());

    temp.remove();
    EXPECT_FALSE(temp.exists());
}

TEST(FileUtilsTest, ReadWriteAllBytes)
{
    ramses_capu::File temp("readWriteBytesTest.dat");
    EXPECT_FALSE(temp.exists());

    ramses_capu::Byte largeContent[8000];

    for (uint32_t i = 0; i < sizeof(largeContent); i++)
    {
        largeContent[i] = i % 255;
    }

    ramses_capu::FileUtils::writeAllBytes(temp, largeContent, sizeof(largeContent));
    EXPECT_TRUE(temp.exists());

    std::vector<ramses_capu::Byte> readBuffer;
    EXPECT_EQ(ramses_capu::CAPU_OK, ramses_capu::FileUtils::readAllBytes(temp, readBuffer));
    EXPECT_EQ(readBuffer.size(), sizeof(largeContent));
    EXPECT_EQ(0, ramses_capu::Memory::Compare(largeContent, readBuffer.data(), readBuffer.size()));

    // Keep the file and overwrite with a smaller set of data
    const ramses_capu::Byte smallContent[] = {0xAA, 0xBB, 0x0, 0xCC, 0xFF, 0x0, 0x05 };
    ramses_capu::FileUtils::writeAllBytes(temp, smallContent, sizeof(smallContent));
    readBuffer.clear(); // Re-use the same buffer
    EXPECT_EQ(ramses_capu::CAPU_OK, ramses_capu::FileUtils::readAllBytes(temp, readBuffer));
    EXPECT_EQ(readBuffer.size(), sizeof(smallContent));
    EXPECT_EQ(0, ramses_capu::Memory::Compare(smallContent, readBuffer.data(), readBuffer.size()));

    temp.remove();
    EXPECT_FALSE(temp.exists());
}

TEST(FileUtilsTest, GetCurrentWorkingDirectory)
{
    ramses_capu::File cwd = ramses_capu::FileUtils::getCurrentWorkingDirectory();

    EXPECT_STRNE(cwd.getPath().c_str(), "");
    EXPECT_TRUE(cwd.exists());

    ramses_capu::File secondCwd = ramses_capu::FileUtils::getCurrentWorkingDirectory();
    EXPECT_STREQ(cwd.getPath().c_str(), secondCwd.getPath().c_str());
}

TEST(FileUtilsTest, SetCurrentWorkingDirectory)
{
    ramses_capu::File oldCwd = ramses_capu::FileUtils::getCurrentWorkingDirectory();

    ramses_capu::File newCwd("..");
    EXPECT_EQ(ramses_capu::CAPU_OK, ramses_capu::FileUtils::setCurrentWorkingDirectory(newCwd));

    newCwd = ramses_capu::FileUtils::getCurrentWorkingDirectory();
    EXPECT_STRNE(oldCwd.getPath().c_str(), newCwd.getPath().c_str());

    EXPECT_TRUE(oldCwd.getPath().startsWith(newCwd.getPath()));

    bool status = false;
    EXPECT_EQ(newCwd.getPath(), oldCwd.getParentFile(status).getPath());
    EXPECT_TRUE(status);

    EXPECT_EQ(ramses_capu::CAPU_OK, ramses_capu::FileUtils::setCurrentWorkingDirectory(oldCwd));

    ramses_capu::File newOldCwd = ramses_capu::FileUtils::getCurrentWorkingDirectory();
    EXPECT_STREQ(oldCwd.getPath().c_str(), newOldCwd.getPath().c_str());
}
