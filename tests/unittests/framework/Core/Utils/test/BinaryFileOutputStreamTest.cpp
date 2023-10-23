//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "internal/Core/Utils/BinaryFileOutputStream.h"

#include <string>

namespace ramses::internal
{
    class BinaryFileOutputStreamTest : public testing::Test
    {
    public:
        BinaryFileOutputStreamTest();
        ~BinaryFileOutputStreamTest() override;

        File m_file;
        std::string m_testString;
    };

    BinaryFileOutputStreamTest::BinaryFileOutputStreamTest()
        : m_file("TestFile.bin")
    {
        BinaryFileOutputStream outputStream(m_file);

        m_testString = "Dies ist ein Text";

        outputStream << 10 << 20.0f << m_testString;
    }

    BinaryFileOutputStreamTest::~BinaryFileOutputStreamTest()
    {
        m_file.close();
        m_file.remove();
    }

    TEST_F(BinaryFileOutputStreamTest, ReadSomeData)
    {
        EXPECT_TRUE(m_file.open(File::Mode::ReadOnlyBinary));

        {
            size_t  numBytes = 0;
            int32_t inValue  = 0;
            EXPECT_EQ(EStatus::Ok, m_file.read(&inValue, sizeof(int32_t), numBytes));
            ASSERT_EQ(sizeof(int32_t), numBytes);
            EXPECT_EQ(10, inValue);
        }

        {
            size_t numBytes = 0;
            float  inValue  = 0.f;
            EXPECT_EQ(EStatus::Ok, m_file.read(&inValue, sizeof(float), numBytes));
            EXPECT_EQ(sizeof(float), numBytes);
        }

        {
            size_t numBytes   = 0;
            char   buffer[40] = {0};
            EXPECT_EQ(EStatus::Ok, m_file.read(buffer, sizeof(uint32_t), numBytes));
            EXPECT_EQ(EStatus::Ok, m_file.read(buffer, sizeof(char) * m_testString.size(), numBytes));
            buffer[m_testString.size()] = 0;
            EXPECT_EQ(sizeof(char) * m_testString.size(), numBytes);
            EXPECT_STREQ("Dies ist ein Text", buffer);
        }
    }

    TEST_F(BinaryFileOutputStreamTest, BadFileObj)
    {
        File file("some/non/existing/path");
        BinaryFileOutputStream outputStream(file);

        char data = '1';
        int size = 1;

        outputStream.write(&data, size);
    }
}
