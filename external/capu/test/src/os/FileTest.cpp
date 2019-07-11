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

#include <gtest/gtest.h>
#include "ramses-capu/Config.h"
#include "ramses-capu/os/File.h"
#include "ramses-capu/os/Memory.h"

TEST(File, ConstructorTest)
{
    ramses_capu::File f1("foobar.txt");
    f1.open(ramses_capu::READ_ONLY);
    EXPECT_FALSE(f1.isOpen());

    ramses_capu::File f2(std::string("test.txt"));
    f2.open(ramses_capu::READ_WRITE_OVERWRITE_OLD);
    EXPECT_TRUE(f2.isOpen());
}

TEST(File, CantOpenNonExistingFile)
{
    ramses_capu::File f1("foobar.txt");
    f1.open(ramses_capu::READ_ONLY);
    EXPECT_FALSE(f1.isOpen());
}

TEST(File, ReadableFileCanBeOpened)
{
    ramses_capu::File f2("test.txt");
    f2.open(ramses_capu::READ_WRITE_OVERWRITE_OLD);
    EXPECT_TRUE(f2.isOpen());
}

TEST(File, WriteTest)
{
    char buf1[15] = "This is a test";
    ramses_capu::status_t status;

    ramses_capu::File f1("test.txt");
    f1.open(ramses_capu::READ_WRITE_OVERWRITE_OLD);
    EXPECT_TRUE(f1.isOpen());

    // invalid params
    status = f1.write(NULL, 0);
    EXPECT_EQ(ramses_capu::CAPU_EINVAL, status);

    // write data
    status = f1.write(buf1, sizeof(buf1) - 1);
    EXPECT_EQ(ramses_capu::CAPU_OK, status);

    EXPECT_EQ(ramses_capu::CAPU_OK, f1.close());
    EXPECT_EQ(ramses_capu::CAPU_OK, f1.remove());
}

TEST(File, WriteSubstring)
{
    char bufWrite[40] = "This is a substring. This is a postfix.";
    char bufRead[21];
    ramses_capu::uint_t substringSize = 20;
    ramses_capu::status_t status;

    ramses_capu::File f1("test.txt");
    f1.open(ramses_capu::READ_WRITE_OVERWRITE_OLD);
    EXPECT_TRUE(f1.isOpen());

    // write data
    status = f1.write(bufWrite, substringSize);
    EXPECT_EQ(ramses_capu::CAPU_OK, status);

    //reopen the file
    f1.close();
    status = f1.open(ramses_capu::READ_ONLY);
    EXPECT_EQ(ramses_capu::CAPU_OK, status);

    //read data back
    memset(bufRead, 0, sizeof(bufRead));
    ramses_capu::uint_t read;
    status = f1.read(bufRead, substringSize, read);
    EXPECT_EQ(ramses_capu::CAPU_OK, status);
    EXPECT_EQ(substringSize, read);

    //extract substring
    char subString[21];
    ramses_capu::StringUtils::Strncpy(subString, sizeof(subString), bufWrite);

    EXPECT_STREQ(subString, bufRead);

    EXPECT_EQ(ramses_capu::CAPU_OK, f1.close());
    EXPECT_EQ(ramses_capu::CAPU_OK, f1.remove());
}

TEST(File, CantWriteToFileOpenedReadOnly)
{
    char buf1[20] = "This is a test";
    char buf2[20];

    ramses_capu::status_t status;

    //create the file
    ramses_capu::File f1("test.txt");
    f1.open(ramses_capu::READ_WRITE_OVERWRITE_OLD);
    f1.close();
    ASSERT_TRUE(f1.exists());

    // open file
    ramses_capu::File readOnlyFile("test.txt");
    readOnlyFile.open(ramses_capu::READ_ONLY);
    EXPECT_TRUE(readOnlyFile.isOpen());

    // file not opened for writing
    status = readOnlyFile.write(buf2, strlen(buf1));
    EXPECT_EQ(ramses_capu::CAPU_ERROR, status);

    //clean up the file
    EXPECT_EQ(ramses_capu::CAPU_OK, readOnlyFile.close());
    EXPECT_EQ(ramses_capu::CAPU_OK, readOnlyFile.remove());
}

TEST(File, ReadTest)
{
    char buf1[20] = "This is a test";
    char buf2[20];

    ramses_capu::status_t status;
    ramses_capu::uint_t read = 0;

    {
        // write data
        ramses_capu::File f2("test.txt");
        f2.open(ramses_capu::READ_WRITE_OVERWRITE_OLD);
        EXPECT_TRUE(f2.isOpen());

        status = f2.write(buf1, strlen(buf1));
        EXPECT_EQ(ramses_capu::CAPU_OK, status);
    }
    {
        // read data
        ramses_capu::File f3("test.txt");
        f3.open(ramses_capu::READ_ONLY);
        EXPECT_TRUE(f3.isOpen());

        // invalid params
        status = f3.read(NULL, 0, read);
        EXPECT_EQ(ramses_capu::CAPU_EINVAL, status);

        read = 0;
        memset(buf2, 0, sizeof(buf2));
        status = f3.read(buf2, strlen(buf1), read);
        EXPECT_EQ(ramses_capu::CAPU_OK, status);
        EXPECT_EQ(static_cast<ramses_capu::uint_t>(strlen(buf1)), read);
    }
    {
        // read data
        ramses_capu::File f4("test.txt");
        f4.open(ramses_capu::READ_ONLY);
        EXPECT_TRUE(f4.isOpen());

        memset(buf2, 0, sizeof(buf2));
        status = f4.read(buf2, strlen(buf1), read);
        EXPECT_EQ(ramses_capu::CAPU_OK, status);
        EXPECT_EQ(static_cast<ramses_capu::uint_t>(strlen(buf1)), read);
    }
    {
        // read data Eof
        ramses_capu::File f5("test.txt");
        f5.open(ramses_capu::READ_ONLY);
        EXPECT_TRUE(f5.isOpen());

        read = 0;
        memset(buf2, 0, sizeof(buf2));
        status = f5.read(buf2, sizeof(buf2), read);
        EXPECT_EQ(ramses_capu::CAPU_EOF, status);
        EXPECT_EQ(static_cast<ramses_capu::uint_t>(strlen(buf1)), read);
        EXPECT_EQ(0, strcmp(buf1, buf2));
    }
}

TEST(File, ReadWriteBinaryTest)
{
    char buf1[5];
    char buf2[5];
    buf1[0] = 4;
    buf1[1] = 2;
    buf1[2] = 6;
    buf1[3] = 6;
    buf1[4] = 6;
    ramses_capu::status_t status;

    // write data
    ramses_capu::File f2("test.txt");
    f2.open(ramses_capu::READ_WRITE_EXISTING_BINARY);
    EXPECT_TRUE(f2.isOpen());

    status = f2.write(buf1, sizeof(buf1));
    EXPECT_EQ(ramses_capu::CAPU_OK, status);
    f2.close();

    // read data
    ramses_capu::File f3("test.txt");
    f3.open(ramses_capu::READ_WRITE_EXISTING_BINARY);
    ramses_capu::uint_t bytes;
    status = f3.read(buf2, sizeof(buf2), bytes);
    EXPECT_EQ(ramses_capu::CAPU_OK, status);
    f3.close();

    EXPECT_EQ(sizeof(buf1), bytes);
    for (uint32_t i = 0; i < sizeof(buf1); i++)
    {
        EXPECT_EQ(buf1[i], buf2[i]);
    }
}

TEST(File, Seek)
{
    char buf1[7] = "hello ";
    char buf2[6] = "world";
    char buf3[2] = "!";
    ramses_capu::status_t status;

    // write data
    ramses_capu::File writeFile("seektest.txt");
    writeFile.open(ramses_capu::WRITE_NEW_BINARY);

    status = writeFile.seek(6, ramses_capu::FROM_BEGINNING);
    EXPECT_EQ(ramses_capu::CAPU_OK, status);
    status = writeFile.write(buf2, sizeof(buf2) - 1);
    EXPECT_EQ(ramses_capu::CAPU_OK, status);
    // seek back (negative) to beginning of file
    status = writeFile.seek(-6-5, ramses_capu::FROM_CURRENT_POSITION);
    EXPECT_EQ(ramses_capu::CAPU_OK, status);
    status = writeFile.write(buf1, sizeof(buf1)-1);
    EXPECT_EQ(ramses_capu::CAPU_OK, status);
    // seek forwards to end of file
    status = writeFile.seek(5, ramses_capu::FROM_CURRENT_POSITION);
    status = writeFile.write(buf3, 1);
    EXPECT_EQ(ramses_capu::CAPU_OK, status);
    writeFile.close();

    // read data
    char readBuffer[12];
    ramses_capu::File readFile("seektest.txt");
    readFile.open(ramses_capu::READ_ONLY_BINARY);
    ramses_capu::uint_t bytes;
    status = readFile.read(readBuffer, sizeof(readBuffer), bytes);
    EXPECT_EQ(ramses_capu::CAPU_OK, status);
    readFile.close();

    EXPECT_EQ(0, ramses_capu::Memory::Compare("hello world!", readBuffer, sizeof(readBuffer)));
}

TEST(File, getCurrentPositionWithinFile)
{
    char buffer[12] = "hello world";
    const ramses_capu::uint_t numberOfBytesToWrite = sizeof(buffer) - 1;

    // write data
    ramses_capu::File file("getCurrentFilePosition.txt");
    file.open(ramses_capu::WRITE_NEW_BINARY);

    file.seek(6, ramses_capu::FROM_BEGINNING);
    ramses_capu::uint_t filePosition = 0;
    EXPECT_EQ(ramses_capu::CAPU_OK, file.getCurrentPosition(filePosition));
    EXPECT_EQ(6u, filePosition);

    file.write(buffer, numberOfBytesToWrite);
    EXPECT_EQ(ramses_capu::CAPU_OK, file.getCurrentPosition(filePosition));
    EXPECT_EQ(6u+numberOfBytesToWrite, filePosition);
    file.close();
}

TEST(File, OpenBinaryFileCanBeOpenedForReadAgain)
{
    ramses_capu::File f1("test.txt");
    f1.open(ramses_capu::READ_ONLY_BINARY);
    EXPECT_TRUE(f1.isOpen());

    ramses_capu::File f2("test.txt");
    f2.open(ramses_capu::READ_ONLY_BINARY);
    EXPECT_TRUE(f2.isOpen());
}

TEST(File, TestCreateAndDelete)
{
    ramses_capu::File file("something");

    EXPECT_EQ(ramses_capu::CAPU_OK, file.createFile());
    EXPECT_TRUE(file.exists());

    EXPECT_EQ(ramses_capu::CAPU_OK, file.remove());
    EXPECT_FALSE(file.exists());
}

TEST(File, TestExists)
{
    ramses_capu::File file(".");
    EXPECT_TRUE(file.exists());
}

TEST(File, TestGetFilename)
{
    ramses_capu::File f1("filename");
    EXPECT_STREQ("filename", f1.getFileName().c_str());

    ramses_capu::File f2("parent/filename");
    EXPECT_STREQ("filename", f2.getFileName().c_str());

    ramses_capu::File f3("filename.ext");
    EXPECT_STREQ("filename.ext", f3.getFileName().c_str());

    ramses_capu::File f4("/parent/filename.ext");
    EXPECT_STREQ("filename.ext", f4.getFileName().c_str());
}
TEST(File, TestGetExtension)
{
    ramses_capu::File file("msdia80.dll");
    EXPECT_STREQ("dll", file.getExtension().c_str());

    ramses_capu::File file2("myVacation.jpg");
    EXPECT_STREQ("jpg", file2.getExtension().c_str());

    ramses_capu::File file3("noEnding");
    EXPECT_STREQ("", file3.getExtension().c_str());
}

TEST(File, TestGetPath)
{
    ramses_capu::File f1("./relative");
    EXPECT_STREQ("./relative", f1.getPath().c_str());

    ramses_capu::File f2("parent/filename");
    EXPECT_STREQ("parent/filename", f2.getPath().c_str());

    ramses_capu::File f3("filename.ext");
    EXPECT_STREQ("filename.ext", f3.getPath().c_str());

    ramses_capu::File f4("/absolute/filename.ext");
    EXPECT_STREQ("/absolute/filename.ext", f4.getPath().c_str());
}

TEST(File, TestGetFileSize)
{
    char buf1[15] = "This is a test";
    ramses_capu::status_t status;
    // make a file with a size>0
    ramses_capu::File file("sizetest.txt");
    file.open(ramses_capu::READ_WRITE_OVERWRITE_OLD);
    EXPECT_TRUE(file.isOpen());

    status = file.write(buf1, strlen(buf1));
    EXPECT_EQ(ramses_capu::CAPU_OK, status);
    file.close();
    ramses_capu::uint_t byteSize;
    status = file.getSizeInBytes(byteSize);
    EXPECT_EQ(ramses_capu::CAPU_OK, status);
    EXPECT_EQ(14u, byteSize);
}

TEST(File, TestIsDirectory)
{
    ramses_capu::File tempFile("temp.txt");
    tempFile.createFile();
    EXPECT_FALSE(tempFile.isDirectory());

    ramses_capu::File file2(".");
    EXPECT_TRUE(file2.isDirectory());

    ramses_capu::File file3("..");
    EXPECT_TRUE(file3.isDirectory());

    tempFile.remove();
}

TEST(File, CreateAndRemoveDirectory)
{
    ramses_capu::File temp("temp");

    EXPECT_FALSE(temp.isDirectory());
    EXPECT_FALSE(temp.exists());
    temp.createDirectory();
    EXPECT_TRUE(temp.exists());
    EXPECT_TRUE(temp.isDirectory());

    temp.remove();
    EXPECT_FALSE(temp.isDirectory());
    EXPECT_FALSE(temp.exists());
}
