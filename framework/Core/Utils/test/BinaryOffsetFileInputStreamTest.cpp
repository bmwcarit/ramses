//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "Utils/BinaryOffsetFileInputStream.h"
#include "WindowsInvalidParameterCheckSuppression.h"
#include "Collections/IInputStream.h"
#include "Utils/File.h"
#include "gtest/gtest.h"
#include <sys/stat.h>
#include <fcntl.h>

namespace ramses_internal
{
    class ABinaryOffsetFileInputStream : public ::testing::Test
    {
    public:
        void TearDown() override
        {
            close(fd);
            File(testFileName).remove();
        }

        void writeFile(std::initializer_list<Byte> data)
        {
            File f(testFileName);
            ASSERT_TRUE(f.open(File::Mode::WriteNewBinary));
            ASSERT_TRUE(f.write(data.begin(), data.size()));
        }

        void openFile(int flags = O_RDONLY)
        {
            assert(fd == -1);
            fd = open(testFileName, flags);
            ASSERT_NE(-1, fd);
        }

        std::vector<Byte> readData(BinaryOffsetFileInputStream& is, size_t size)
        {
            std::vector<Byte> data(size);
            is.read(data.data(), size);
            return data;
        }

        WindowsInvalidParameterCheckSuppression suppress;
        const char* testFileName = "testfile.bin";
        int fd = -1;
    };

    TEST_F(ABinaryOffsetFileInputStream, readRegularFileInOneBlock)
    {
        writeFile({3, 2, 1});
        openFile();
        BinaryOffsetFileInputStream is(fd, 0, 3);
        EXPECT_EQ(std::vector<Byte>({3, 2, 1}), readData(is, 3));
        EXPECT_EQ(EStatus::Ok, is.getState());
    }

    TEST_F(ABinaryOffsetFileInputStream, readRegularFileInChunks)
    {
        writeFile({3, 2, 1});
        openFile();
        BinaryOffsetFileInputStream is(fd, 0, 3);
        EXPECT_EQ(std::vector<Byte>({3}), readData(is, 1));
        EXPECT_EQ(std::vector<Byte>({2}), readData(is, 1));
        EXPECT_EQ(std::vector<Byte>({1}), readData(is, 1));
        EXPECT_EQ(EStatus::Ok, is.getState());
    }

    TEST_F(ABinaryOffsetFileInputStream, readWithOffset)
    {
        writeFile({0, 0, 3, 2, 1, 0, 0});
        openFile();
        BinaryOffsetFileInputStream is(fd, 2, 3);
        EXPECT_EQ(std::vector<Byte>({3, 2, 1}), readData(is, 3));
        EXPECT_EQ(EStatus::Ok, is.getState());
    }

    TEST_F(ABinaryOffsetFileInputStream, openingInvalidFileDescriptorFails)
    {
        BinaryOffsetFileInputStream is(-1, 0, 3);
        EXPECT_EQ(EStatus::Error, is.getState());
    }

    TEST_F(ABinaryOffsetFileInputStream, openingWithWrongPermissionFails)
    {
        writeFile({3, 2, 1});
        openFile(O_WRONLY);
        BinaryOffsetFileInputStream is(-1, 0, 3);
        EXPECT_EQ(EStatus::Error, is.getState());
    }

    TEST_F(ABinaryOffsetFileInputStream, readOutsideRangeFails)
    {
        writeFile({0, 0, 3, 2, 1, 0, 0});
        openFile();
        BinaryOffsetFileInputStream is(fd, 2, 3);
        readData(is, 4);
        EXPECT_EQ(EStatus::Eof, is.getState());
    }

    TEST_F(ABinaryOffsetFileInputStream, readOverEofFails)
    {
        writeFile({0, 3, 2, 1});
        openFile();
        BinaryOffsetFileInputStream is(fd, 1, 4);
        readData(is, 4);
        EXPECT_EQ(EStatus::Eof, is.getState());
    }

    TEST_F(ABinaryOffsetFileInputStream, canSeekWithinRange)
    {
        writeFile({0, 1, 2, 3, 0});
        openFile();
        BinaryOffsetFileInputStream is(fd, 1, 3);

        EXPECT_EQ(EStatus::Ok, is.seek(0, IInputStream::Seek::FromBeginning));
        EXPECT_EQ(std::vector<Byte>({1}), readData(is, 1));

        EXPECT_EQ(EStatus::Ok, is.seek(0, IInputStream::Seek::Relative));
        EXPECT_EQ(std::vector<Byte>({2}), readData(is, 1));

        EXPECT_EQ(EStatus::Ok, is.seek(2, IInputStream::Seek::FromBeginning));
        EXPECT_EQ(std::vector<Byte>({3}), readData(is, 1));

        EXPECT_EQ(EStatus::Ok, is.seek(-1, IInputStream::Seek::Relative));
        EXPECT_EQ(std::vector<Byte>({3}), readData(is, 1));

        EXPECT_EQ(EStatus::Ok, is.seek(-3, IInputStream::Seek::Relative));
        EXPECT_EQ(std::vector<Byte>({1}), readData(is, 1));

        EXPECT_EQ(EStatus::Ok, is.getState());
    }

    TEST_F(ABinaryOffsetFileInputStream, seekOutsideRangeFails)
    {
        writeFile({0, 1, 2, 3, 0});
        openFile();
        BinaryOffsetFileInputStream is(fd, 1, 3);

        size_t pos = 0;

        EXPECT_EQ(EStatus::Error, is.seek(-1, IInputStream::Seek::Relative));
        EXPECT_EQ(EStatus::Error, is.seek(4, IInputStream::Seek::Relative));
        EXPECT_EQ(EStatus::Error, is.seek(4, IInputStream::Seek::FromBeginning));

        EXPECT_EQ(EStatus::Ok, is.getPos(pos));
        EXPECT_EQ(0u, pos);

        EXPECT_EQ(EStatus::Ok, is.seek(3, IInputStream::Seek::FromBeginning));
        EXPECT_EQ(EStatus::Error, is.seek(-4, IInputStream::Seek::Relative));
        EXPECT_EQ(EStatus::Error, is.seek(1, IInputStream::Seek::Relative));

        EXPECT_EQ(EStatus::Ok, is.getPos(pos));
        EXPECT_EQ(3u, pos);

        EXPECT_EQ(EStatus::Ok, is.getState());
    }

    TEST_F(ABinaryOffsetFileInputStream, canGetPosRegularFile)
    {
        writeFile({1, 2, 3});
        openFile();
        BinaryOffsetFileInputStream is(fd, 0, 3);

        size_t pos = 0;
        EXPECT_EQ(EStatus::Ok, is.getPos(pos));
        EXPECT_EQ(0u, pos);

        EXPECT_EQ(EStatus::Ok, is.seek(2, IInputStream::Seek::FromBeginning));
        EXPECT_EQ(EStatus::Ok, is.getPos(pos));
        EXPECT_EQ(2u, pos);

        EXPECT_EQ(EStatus::Ok, is.seek(-1, IInputStream::Seek::Relative));
        EXPECT_EQ(EStatus::Ok, is.getPos(pos));
        EXPECT_EQ(1u, pos);

        EXPECT_EQ(std::vector<Byte>({2}), readData(is, 1));
        EXPECT_EQ(EStatus::Ok, is.getPos(pos));
        EXPECT_EQ(2u, pos);
    }

    TEST_F(ABinaryOffsetFileInputStream, destructorClosesFileDescriptor)
    {
        {
            writeFile({1, 2, 3});
            openFile();
            BinaryOffsetFileInputStream is(fd, 0, 3);
        }
        EXPECT_EQ(-1, ::close(fd));
    }
}
