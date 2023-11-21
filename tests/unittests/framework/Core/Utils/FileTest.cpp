//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Core/Utils/File.h"
#include <gtest/gtest.h>

namespace ramses::internal
{
    class AFile : public ::testing::Test
    {
    public:
        static bool touchFile(const char* name)
        {
            File f(name);
            if (f.exists())
                return false;
            return f.open(File::Mode::WriteOverWriteOld);
        }

        void addForCleanup(std::initializer_list<const char*> lst)
        {
            m_cleanupFiles.insert(m_cleanupFiles.end(), lst.begin(), lst.end());
        }

        void TearDown() override
        {
            for (const auto& f : m_cleanupFiles)
                File(f).remove();
        }

    private:
        std::vector<std::string> m_cleanupFiles;
    };

    TEST_F(AFile, ConstructorTest)
    {
        addForCleanup({"foobar.txt", "test.txt"});

        File f1("foobar.txt");
        EXPECT_FALSE(f1.open(File::Mode::ReadOnly));
        EXPECT_FALSE(f1.isOpen());

        File f2("test.txt");
        EXPECT_TRUE(f2.open(File::Mode::WriteOverWriteOld));
        EXPECT_TRUE(f2.isOpen());
    }

    TEST_F(AFile, CantOpenNonExistingFile)
    {
        addForCleanup({"foobar.txt"});

        File f1("foobar.txt");
        EXPECT_FALSE(f1.open(File::Mode::ReadOnly));
        EXPECT_FALSE(f1.isOpen());
    }

    TEST_F(AFile, ReadableFileCanBeOpened)
    {
        addForCleanup({"test.txt"});

        File f2("test.txt");
        EXPECT_TRUE(f2.open(File::Mode::WriteOverWriteOld));
        EXPECT_TRUE(f2.isOpen());
    }

    TEST_F(AFile, WriteTest)
    {
        addForCleanup({"test.txt"});

        char buf1[15] = "This is a test";

        File f1("test.txt");
        EXPECT_TRUE(f1.open(File::Mode::WriteOverWriteOld));
        EXPECT_TRUE(f1.isOpen());

        // invalid params
        EXPECT_FALSE(f1.write(nullptr, 0));

        // write data
        EXPECT_TRUE(f1.write(buf1, sizeof(buf1) - 1));

        EXPECT_TRUE(f1.close());
        EXPECT_TRUE(f1.remove());
    }

    TEST_F(AFile, WriteSubstring)
    {
        addForCleanup({"test.txt"});

        char bufWrite[40] = "This is a substring. This is a postfix.";
        char bufRead[21];
        size_t substringSize = 20;

        File f1("test.txt");
        EXPECT_TRUE(f1.open(File::Mode::WriteOverWriteOld));
        EXPECT_TRUE(f1.isOpen());

        // write data
        EXPECT_TRUE(f1.write(bufWrite, substringSize));

        //reopen the file
        f1.close();
        EXPECT_TRUE(f1.open(File::Mode::ReadOnly));

        //read data back
        std::memset(bufRead, 0, sizeof(bufRead));
        size_t read = 0;
        EXPECT_EQ(EStatus::Ok, f1.read(bufRead, substringSize, read));
        EXPECT_EQ(substringSize, read);

        //extract substring
        EXPECT_STREQ("This is a substring.", bufRead);

        EXPECT_TRUE(f1.close());
        EXPECT_TRUE(f1.remove());
    }

    TEST_F(AFile, CantWriteToFileOpenedReadOnly)
    {
        addForCleanup({"test.txt"});

        char buf1[20] = "This is a test";
        char buf2[20];

        //create the file
        File f1("test.txt");
        EXPECT_TRUE(f1.open(File::Mode::WriteOverWriteOld));
        EXPECT_TRUE(f1.close());
        ASSERT_TRUE(f1.exists());

        // open file
        File readOnlyFile("test.txt");
        EXPECT_TRUE(readOnlyFile.open(File::Mode::ReadOnly));
        EXPECT_TRUE(readOnlyFile.isOpen());

        // file not opened for writing
        EXPECT_FALSE(readOnlyFile.write(buf2, strlen(buf1)));

        //clean up the file
        EXPECT_TRUE(readOnlyFile.close());
        EXPECT_TRUE(readOnlyFile.remove());
    }

    TEST_F(AFile, ReadTest)
    {
        addForCleanup({"test.txt"});

        char buf1[20] = "This is a test";
        char buf2[20];

        {
            // write data
            File f2("test.txt");
            EXPECT_TRUE(f2.open(File::Mode::WriteOverWriteOld));
            EXPECT_TRUE(f2.isOpen());
            EXPECT_TRUE(f2.write(buf1, strlen(buf1)));
        }
        {
            // read data
            File f3("test.txt");
            EXPECT_TRUE(f3.open(File::Mode::ReadOnly));
            EXPECT_TRUE(f3.isOpen());

            // invalid params
            size_t read = 0;
            EXPECT_EQ(EStatus::Error, f3.read(nullptr, 0, read));

            memset(buf2, 0, sizeof(buf2));
            EXPECT_EQ(EStatus::Ok, f3.read(buf2, strlen(buf1), read));
            EXPECT_EQ(static_cast<size_t>(strlen(buf1)), read);
        }
        {
            // read data
            File f4("test.txt");
            EXPECT_TRUE(f4.open(File::Mode::ReadOnly));
            EXPECT_TRUE(f4.isOpen());

            size_t read = 0;
            memset(buf2, 0, sizeof(buf2));
            EXPECT_EQ(EStatus::Ok, f4.read(buf2, strlen(buf1), read));
            EXPECT_EQ(static_cast<size_t>(strlen(buf1)), read);
        }
        {
            // read data Eof
            File f5("test.txt");
            EXPECT_TRUE(f5.open(File::Mode::ReadOnly));
            EXPECT_TRUE(f5.isOpen());

            size_t read = 0;
            memset(buf2, 0, sizeof(buf2));
            EXPECT_EQ(EStatus::Eof, f5.read(buf2, sizeof(buf2), read));
            EXPECT_EQ(static_cast<size_t>(strlen(buf1)), read);
            EXPECT_EQ(0, strcmp(buf1, buf2));
        }
    }

    TEST_F(AFile, ReadWriteBinaryTest)
    {
        addForCleanup({"test.txt"});
        ASSERT_TRUE(touchFile("test.txt"));

        char buf1[5];
        char buf2[5];
        buf1[0] = 4;
        buf1[1] = 2;
        buf1[2] = 6;
        buf1[3] = 6;
        buf1[4] = 6;

        // write data
        File f2("test.txt");
        EXPECT_TRUE(f2.open(File::Mode::WriteExistingBinary));
        EXPECT_TRUE(f2.isOpen());

        EXPECT_TRUE(f2.write(buf1, sizeof(buf1)));
        f2.close();

        // read data
        File f3("test.txt");
        EXPECT_TRUE(f3.open(File::Mode::WriteExistingBinary));
        size_t bytes = 0;
        EXPECT_EQ(EStatus::Ok, f3.read(buf2, sizeof(buf2), bytes));
        f3.close();

        EXPECT_EQ(sizeof(buf1), bytes);
        for (size_t i = 0; i < sizeof(buf1); i++)
        {
            EXPECT_EQ(buf1[i], buf2[i]);
        }
    }

    TEST_F(AFile, Seek)
    {
        addForCleanup({"seektest.txt"});

        char buf1[7] = "hello ";
        char buf2[6] = "world";
        char buf3[2] = "!";

        // write data
        File writeFile("seektest.txt");
        EXPECT_TRUE(writeFile.open(File::Mode::WriteNewBinary));

        EXPECT_TRUE(writeFile.seek(6, File::SeekOrigin::BeginningOfFile));
        EXPECT_TRUE(writeFile.write(buf2, sizeof(buf2) - 1));

        // seek back (negative) to beginning of file
        EXPECT_TRUE(writeFile.seek(-6-5, File::SeekOrigin::RelativeToCurrentPosition));
        EXPECT_TRUE(writeFile.write(buf1, sizeof(buf1)-1));

        // seek forwards to end of file
        EXPECT_TRUE(writeFile.seek(5, File::SeekOrigin::RelativeToCurrentPosition));
        EXPECT_TRUE(writeFile.write(buf3, 1));

        writeFile.close();

        // read data
        char readBuffer[12];
        File readFile("seektest.txt");
        EXPECT_TRUE(readFile.open(File::Mode::ReadOnlyBinary));
        size_t bytes = 0u;
        EXPECT_EQ(EStatus::Ok, readFile.read(readBuffer, sizeof(readBuffer), bytes));
        readFile.close();

        EXPECT_EQ(0, std::memcmp("hello world!", readBuffer, sizeof(readBuffer)));
    }

    TEST_F(AFile, getPosWithinFile)
    {
        addForCleanup({"getCurrentFilePosition.txt"});

        char buffer[12] = "hello world";
        const size_t numberOfBytesToWrite = sizeof(buffer) - 1;

        // write data
        File file("getCurrentFilePosition.txt");
        EXPECT_TRUE(file.open(File::Mode::WriteNewBinary));

        EXPECT_TRUE(file.seek(6, File::SeekOrigin::BeginningOfFile));
        size_t filePosition = 0;
        EXPECT_TRUE(file.getPos(filePosition));
        EXPECT_EQ(6u, filePosition);

        EXPECT_TRUE(file.write(buffer, numberOfBytesToWrite));
        EXPECT_TRUE(file.getPos(filePosition));
        EXPECT_EQ(6u+numberOfBytesToWrite, filePosition);
        file.close();
    }

    TEST_F(AFile, OpenBinaryFileCanBeOpenedForReadAgain)
    {
        addForCleanup({"test.txt"});
        ASSERT_TRUE(touchFile("test.txt"));

        File f1("test.txt");
        EXPECT_TRUE(f1.open(File::Mode::ReadOnlyBinary));
        EXPECT_TRUE(f1.isOpen());

        File f2("test.txt");
        EXPECT_TRUE(f2.open(File::Mode::ReadOnlyBinary));
        EXPECT_TRUE(f2.isOpen());
    }

    TEST_F(AFile, TestCreateAndDelete)
    {
        addForCleanup({"something"});

        File file("something");

        EXPECT_TRUE(file.createFile());
        EXPECT_TRUE(file.exists());

        EXPECT_TRUE(file.remove());
        EXPECT_FALSE(file.exists());
    }

    TEST_F(AFile, TestExists)
    {
        File file(".");
        EXPECT_TRUE(file.exists());
    }

    TEST_F(AFile, TestGetFilename)
    {
        File f1("filename");
        EXPECT_STREQ("filename", f1.getFileName().c_str());

        File f2("parent/filename");
        EXPECT_STREQ("filename", f2.getFileName().c_str());

        File f3("filename.ext");
        EXPECT_STREQ("filename.ext", f3.getFileName().c_str());

        File f4("/parent/filename.ext");
        EXPECT_STREQ("filename.ext", f4.getFileName().c_str());
    }

    TEST_F(AFile, TestGetExtension)
    {
        File file("msdia80.dll");
        EXPECT_STREQ("dll", file.getExtension().c_str());

        File file2("myVacation.jpg");
        EXPECT_STREQ("jpg", file2.getExtension().c_str());

        File file3("noEnding");
        EXPECT_STREQ("", file3.getExtension().c_str());
    }

    TEST_F(AFile, TestGetPath)
    {
        File f1("./relative");
        EXPECT_STREQ("./relative", f1.getPath().c_str());

        File f2("parent/filename");
        EXPECT_STREQ("parent/filename", f2.getPath().c_str());

        File f3("filename.ext");
        EXPECT_STREQ("filename.ext", f3.getPath().c_str());

        File f4("/absolute/filename.ext");
        EXPECT_STREQ("/absolute/filename.ext", f4.getPath().c_str());
    }

    TEST_F(AFile, TestGetFileSize)
    {
        addForCleanup({"sizetest.txt"});

        char buf1[15] = "This is a test";

        // make a file with a size>0
        File file("sizetest.txt");
        EXPECT_TRUE(file.open(File::Mode::WriteOverWriteOld));
        EXPECT_TRUE(file.isOpen());

        EXPECT_TRUE(file.write(buf1, strlen(buf1)));
        file.close();
        size_t byteSize = 0u;
        EXPECT_TRUE(file.getSizeInBytes(byteSize));
        EXPECT_EQ(14u, byteSize);
    }

    TEST_F(AFile, TestIsDirectory)
    {
        addForCleanup({"temp.txt"});

        File tempFile("temp.txt");
        tempFile.createFile();
        EXPECT_FALSE(tempFile.isDirectory());

        File file2(".");
        EXPECT_TRUE(file2.isDirectory());

        File file3("..");
        EXPECT_TRUE(file3.isDirectory());

        tempFile.remove();
    }

    TEST_F(AFile, CreateAndRemoveDirectory)
    {
        addForCleanup({"temp"});

        File temp("temp");

        EXPECT_FALSE(temp.isDirectory());
        EXPECT_FALSE(temp.exists());
        temp.createDirectory();
        EXPECT_TRUE(temp.exists());
        EXPECT_TRUE(temp.isDirectory());

        temp.remove();
        EXPECT_FALSE(temp.isDirectory());
        EXPECT_FALSE(temp.exists());
    }
}
