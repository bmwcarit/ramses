//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "internal/Core/Utils/BinaryFileInputStream.h"

namespace ramses::internal
{
    class BinaryFileInputStreamTest : public testing::Test
    {
    public:
        BinaryFileInputStreamTest()
            : m_file("TestInputFile.bin")
        {
            EXPECT_TRUE(m_file.open(File::Mode::WriteNewBinary));

            int32_t intVal = 10;
            float floatVal = 20.f;
            std::string  stringVal = "Dies ist ein Text";
            auto strlen = static_cast<uint32_t>(stringVal.size());

            EXPECT_TRUE(m_file.write(&intVal, sizeof(int32_t)));
            EXPECT_TRUE(m_file.write(&floatVal, sizeof(float)));
            EXPECT_TRUE(m_file.write(&strlen, sizeof(uint32_t)));
            EXPECT_TRUE(m_file.write(stringVal.c_str(), stringVal.size()));

            m_file.close();
        }

        ~BinaryFileInputStreamTest() override
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
        std::string stringVal;

        inputStream >> intVal;

        EXPECT_EQ(EStatus::Ok, inputStream.getState());

        inputStream >> floatVal;

        EXPECT_EQ(EStatus::Ok, inputStream.getState());

        inputStream >> stringVal;

        EXPECT_EQ(EStatus::Ok, inputStream.getState());

        int32_t errorIntVal = 0;
        inputStream >> errorIntVal;

        EXPECT_EQ(EStatus::Eof, inputStream.getState());

        EXPECT_EQ(10, intVal);
        EXPECT_EQ(20.0f, floatVal);
        EXPECT_STREQ("Dies ist ein Text", stringVal.c_str());
    }

    TEST_F(BinaryFileInputStreamTest, BadFileObj)
    {
        File file("some/non/existing/path");
        BinaryFileInputStream inputStream(file);

        char data = 0;
        int size = 1;

        inputStream.read(&data, size);

        EXPECT_NE(EStatus::Ok, inputStream.getState());
    }
}
