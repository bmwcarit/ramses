//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"
#include "Collections/Guid.h"
#include "Utils/BinaryOutputStream.h"
#include "Utils/BinaryInputStream.h"


namespace ramses_internal
{
    TEST(GuidTest, TestNewAreNotEqual)
    {
        Guid id1(true);
        Guid id2(true);
        EXPECT_FALSE(id1 == id2);
        EXPECT_TRUE(id1 != id2);
    }

    TEST(GuidTest, TestParseAreEqual1)
    {
        Guid id1(true);
        Guid id2(id1.toString());
        EXPECT_TRUE(id1 == id2);
        EXPECT_FALSE(id1 != id2);
    }

    TEST(GuidTest, TestParseInvalidGuid1)
    {
        Guid id1("THIS_IS_NO_GUID");
        EXPECT_TRUE(id1.isInvalid());
    }

    TEST(GuidTest, TestParseInvalidGuid2)
    {
        Guid id1("");
        EXPECT_TRUE(id1.isInvalid());
    }

    TEST(GuidTest, TestParseInvalidGuid3)
    {
        Guid id1("D92BB305XD8B2X4B60XA3A3XA3CFDA57678F");
        Guid id2("4B600A3A3-A3CFDA57678F");
        EXPECT_TRUE(id1.isInvalid());
        EXPECT_TRUE(id2.isInvalid());
    }

    TEST(GuidTest, TestParseToString1)
    {
        Guid id1("D92BB305-D8B2-4B60-A3A3-A3CFDA57678F");
        Guid id2("A3A3-A3CFDA57678F");
        EXPECT_EQ(String("A3A3-A3CFDA57678F"), id1.toString());
        EXPECT_EQ(String("A3A3-A3CFDA57678F"), id2.toString());

        EXPECT_EQ(0u, id1.getGuidData().Data1);
        EXPECT_EQ(0u, id1.getGuidData().Data2);
        EXPECT_EQ(0u, id1.getGuidData().Data3);

        EXPECT_EQ(0u, id2.getGuidData().Data1);
        EXPECT_EQ(0u, id2.getGuidData().Data2);
        EXPECT_EQ(0u, id2.getGuidData().Data3);
    }

    TEST(GuidTest, TestParseToString2)
    {
        Guid id1("BA28BA67-E777-4014-B8DD-6B928BDF57B1");
        Guid id2("B8DD-6B928BDF57B1");
        EXPECT_EQ(String("B8DD-6B928BDF57B1"), id1.toString());
        EXPECT_EQ(String("B8DD-6B928BDF57B1"), id2.toString());
    }

    TEST(GuidTest, TestCopyConstructor)
    {
        Guid id1("BA28BA67-E777-4014-B8DD-6B928BDF57B1");
        Guid id2(id1);
        EXPECT_TRUE(id1 == id2);
    }

    TEST(GuidTest, TestAssignmentOperator1)
    {
        Guid id1("BA28BA67-E777-4014-B8DD-6B928BDF57B1");
        Guid id2(true);
        EXPECT_FALSE(id1 == id2);
        id2 = id1;
        EXPECT_TRUE(id1 == id2);
        EXPECT_EQ(String("B8DD-6B928BDF57B1"), id2.toString());
    }

    TEST(GuidTest, TestParse1)
    {
        Guid id1("BA28BA67-E777-4014-B8DD-6B928BDF57B1");
        Guid id2(true);
        EXPECT_FALSE(id1 == id2);
        id2 = Guid(id1.toString());
        EXPECT_TRUE(id1 == id2);
        EXPECT_EQ(String("B8DD-6B928BDF57B1"), id2.toString());
    }

    TEST(GuidTest, TestParse2)
    {
        Guid id1(true);
        EXPECT_FALSE(id1.isInvalid());
        id1 = Guid("some strange guid string");
        EXPECT_TRUE(id1.isInvalid());
        EXPECT_EQ(String("0000-000000000000"), id1.toString());
    }

    TEST(GuidTest, CopyConstructor)
    {
        Guid id1(true);
        Guid id2(id1);

        EXPECT_STREQ(id1.toString().c_str(), id2.toString().c_str());
    }

    TEST(GuidTest, ExpectUuidStructPacked)
    {
        EXPECT_EQ(16u, sizeof(generic_uuid_t));
    }

    TEST(GuidTest, HashGuid)
    {
        Guid guid(true);
        guid.toString();
        Guid guid2(guid);

        EXPECT_EQ(ramses_capu::HashValue(guid), ramses_capu::HashValue(guid2));
        EXPECT_EQ(ramses_capu::Hash<Guid>()(guid), ramses_capu::Hash<Guid>()(guid2));
    }

    TEST(GuidTest, HashersForGuidEqual)
    {
        Guid guid(true);
        Guid guid2(true);

        EXPECT_EQ(std::hash<Guid>()(guid), ramses_capu::Hash<Guid>()(guid));
        EXPECT_EQ(std::hash<Guid>()(guid2), ramses_capu::Hash<Guid>()(guid2));
    }

    TEST(GuidTest, createsRandom)
    {
        Guid g1(true);
        Guid g2(true);
        EXPECT_NE(g1, g2);
    }

    TEST(GuidTest, copyConstructor)
    {
        Guid g1(true);
        Guid g2(g1);
        EXPECT_EQ(g1, g2);
    }

    TEST(GuidTest, createFromString)
    {
        Guid g1(String("A20AEFB2-7F63-49C6-A6CE-06905B0B1D28"));
        EXPECT_STREQ("A6CE-06905B0B1D28", g1.toString().c_str());
    }

    TEST(GuidTest, canCreateAlwaysSameInvalidGuid)
    {
        Guid g1 = Guid(false);
        Guid g2 = Guid(false);
        EXPECT_EQ(g1, g2);
    }

    TEST(GuidTest, InvalidGuidIsInvalidOthersNot)
    {
        Guid g1 = Guid(false);
        Guid g2(true);

        EXPECT_TRUE(g1.isInvalid());
        EXPECT_FALSE(g2.isInvalid());

        EXPECT_FALSE(g1.isValid());
        EXPECT_TRUE(g2.isValid());
    }

    TEST(GuidTest, writeToStringOutputStream_full)
    {
        StringOutputStream outputStream;
        outputStream << Guid("38ABDEF8-DBB9-42B7-B834-F8C1E76FBDA0");
        EXPECT_STREQ("B834-F8C1E76FBDA0", outputStream.c_str());
        EXPECT_EQ(17U, outputStream.size());
    }

    TEST(GuidTest, writeToStringOutputStream_leading_zeros)
    {
        StringOutputStream outputStream;
        outputStream << Guid("0000-0000006FBDA0");
        EXPECT_STREQ("0000-0000006FBDA0", outputStream.c_str());
        EXPECT_EQ(17U, outputStream.size());
    }

    TEST(GuidTest, writeToStringOutputStream_zero)
    {
        StringOutputStream outputStream;
        outputStream << Guid(false);
        EXPECT_STREQ("0000", outputStream.c_str());
        EXPECT_EQ(4U, outputStream.size());
    }

    TEST(GuidTest, writeToStringOutputStream_specialNumber)
    {
        StringOutputStream outputStream;
        outputStream << Guid("0000-000000000004");
        EXPECT_STREQ("0004", outputStream.c_str());
        EXPECT_EQ(4U, outputStream.size());
    }

    TEST(GuidTest, get64)
    {
        Guid id1("D92BB305-D8B2-4B60-A3A3-A3CFDA57678F");
        EXPECT_EQ(0xA3A3A3CFDA57678Fu, id1.getLow64());

        Guid id2(false);
        EXPECT_EQ(0x0u, id2.getLow64());

        Guid id3("00000000-0000-0000-0000-000000000012");
        EXPECT_EQ(0x12u, id3.getLow64());

        Guid id4("01234567-89AB-CDEF-1234-56789ABCDEFB");
        EXPECT_EQ(0x123456789ABCDEFBu, id4.getLow64());
    }

    TEST(GuidTest, initFrom64)
    {
        Guid id1(0xD92BB305D8B24B60u, 0xA3A3A3CFDA57678Fu);
        EXPECT_EQ(0xA3A3A3CFDA57678Fu, id1.getLow64());

        Guid id2(0, 0);
        EXPECT_EQ(0x0u, id2.getLow64());

        Guid id3(0x0u, 0x12u);
        EXPECT_EQ(0x12u, id3.getLow64());

        Guid id4(0x0123456789ABCDEFu, 0x123456789ABCDEFBu);
        EXPECT_EQ(0x123456789ABCDEFBu, id4.getLow64());
    }

    TEST(GuidTest, writeReadToBinaryStream)
    {
        Guid idOut(0x0123456789ABCDEFu, 0x123456789ABCDEFBu);
        BinaryOutputStream out;
        out << idOut;
        ASSERT_EQ(2*sizeof(uint64_t), out.getSize());

        Guid idIn;
        BinaryInputStream in(out.getData());
        in >> idIn;
        EXPECT_EQ(idOut, idIn);
        EXPECT_EQ(0u, idIn.getGuidData().Data1);
        EXPECT_EQ(0u, idIn.getGuidData().Data2);
        EXPECT_EQ(0u, idIn.getGuidData().Data3);
    }

    TEST(GuidTest, readBinaryStreamWithUpperBytesSet)
    {
        BinaryOutputStream out;
        out << static_cast<uint64_t>(0xAAAAAAAAAAAAAAAAu)
            << static_cast<uint8_t>(0x12)
            << static_cast<uint8_t>(0x34)
            << static_cast<uint8_t>(0x56)
            << static_cast<uint8_t>(0x78)
            << static_cast<uint8_t>(0x9A)
            << static_cast<uint8_t>(0xBC)
            << static_cast<uint8_t>(0xDE)
            << static_cast<uint8_t>(0xFB);
        ASSERT_EQ(2*sizeof(uint64_t), out.getSize());

        Guid idIn;
        BinaryInputStream in(out.getData());
        in >> idIn;
        EXPECT_EQ(0x123456789ABCDEFBu, idIn.getLow64());
        EXPECT_EQ(0u, idIn.getGuidData().Data1);
        EXPECT_EQ(0u, idIn.getGuidData().Data2);
        EXPECT_EQ(0u, idIn.getGuidData().Data3);
    }

}
