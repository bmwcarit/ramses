//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"
#include "Utils/BinaryFileInputStream.h"
#include "Math3d/Matrix44f.h"

namespace ramses_internal
{
    class BinaryFileInputStreamTest : public testing::Test
    {
    public:
        BinaryFileInputStreamTest()
            : m_file("TestInputFile.bin")
        {
            m_file.open(EFileMode_WriteNewBinary);

            int32_t intVal = 10;
            float floatVal = 20.f;
            String  stringVal = "Dies ist ein Text";
            uint32_t strlen = static_cast<uint32_t>(stringVal.getLength());

            m_file.write(reinterpret_cast<char*>(&intVal), sizeof(int32_t));
            m_file.write(reinterpret_cast<char*>(&floatVal), sizeof(float));
            m_file.write(reinterpret_cast<char*>(&strlen), sizeof(uint32_t));
            m_file.write(stringVal.c_str(), stringVal.getLength());

            m_file.close();
        }

        ~BinaryFileInputStreamTest()
        {
            m_file.close();
            m_file.remove();
        }

        File m_file;
    };

    TEST_F(BinaryFileInputStreamTest, ReadSomeData)
    {
        BinaryFileInputStream inputStream(m_file);

        int32_t intVal = 0;
        float floatVal = 0.f;
        String  stringVal = "";

        inputStream >> intVal;

        EXPECT_EQ(EStatus_RAMSES_OK, inputStream.getState());

        inputStream >> floatVal;

        EXPECT_EQ(EStatus_RAMSES_OK, inputStream.getState());

        inputStream >> stringVal;

        EXPECT_EQ(EStatus_RAMSES_OK, inputStream.getState());

        int32_t errorIntVal = 0;
        inputStream >> errorIntVal;

        EXPECT_EQ(EStatus_RAMSES_EOF, inputStream.getState());

        EXPECT_EQ(10, intVal);
        EXPECT_EQ(20.0f, floatVal);
        EXPECT_STREQ("Dies ist ein Text", stringVal.c_str());
    }

    TEST_F(BinaryFileInputStreamTest, BadFileObj)
    {
        File file("some/non/existing/path");
        BinaryFileInputStream inputStream(file);

        char data;
        int size = 1;

        inputStream.read(&data, size);

        EXPECT_NE(EStatus_RAMSES_OK, inputStream.getState());
    }

    TEST_F(BinaryFileInputStreamTest, Matrix44f)
    {
        Float data[16] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f,
            15.0f, 16.0f };

        {
            ramses_internal::File f("testFile.ram");
            f.open(EFileMode_WriteNewBinary);
            f.write(reinterpret_cast<const char*>(data), sizeof(Float) * 16);
            f.flush();
            f.close();
        }

        ramses_internal::File f("testFile.ram");
        BinaryFileInputStream s(f);

        Matrix44f m;

        s >> m;

        for (UInt32 i = 0; i < 16; i++)
        {
            EXPECT_EQ(Float(i + 1), m.data[i]);
        }

        f.remove();
    }

}
