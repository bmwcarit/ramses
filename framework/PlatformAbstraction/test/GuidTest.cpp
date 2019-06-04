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

    TEST(GuidTest, TestGuidIsV4)
    {
        Guid id1(true);
        String str = id1.toString();
        EXPECT_EQ('4', str.c_str()[14]);
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
        EXPECT_TRUE(id1.isInvalid());
    }

    TEST(GuidTest, TestParseToString1)
    {
        Guid id1("D92BB305-D8B2-4B60-A3A3-A3CFDA57678F");
        EXPECT_EQ(String("D92BB305-D8B2-4B60-A3A3-A3CFDA57678F"), id1.toString());
    }

    TEST(GuidTest, TestParseToString2)
    {
        Guid id1("BA28BA67-E777-4014-B8DD-6B928BDF57B1");
        EXPECT_EQ(String("BA28BA67-E777-4014-B8DD-6B928BDF57B1"), id1.toString());
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
        EXPECT_EQ(String("BA28BA67-E777-4014-B8DD-6B928BDF57B1"), id2.toString());
    }

    TEST(GuidTest, TestParse1)
    {
        Guid id1("BA28BA67-E777-4014-B8DD-6B928BDF57B1");
        Guid id2(true);
        EXPECT_FALSE(id1 == id2);
        id2 = Guid(id1.toString());
        EXPECT_TRUE(id1 == id2);
        EXPECT_EQ(String("BA28BA67-E777-4014-B8DD-6B928BDF57B1"), id2.toString());
    }

    TEST(GuidTest, TestParse2)
    {
        Guid id1(true);
        EXPECT_FALSE(id1.isInvalid());
        id1 = Guid("some strange guid string");
        EXPECT_TRUE(id1.isInvalid());
        EXPECT_EQ(String("00000000-0000-0000-0000-000000000000"), id1.toString());
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
        EXPECT_STREQ("A20AEFB2-7F63-49C6-A6CE-06905B0B1D28", g1.toString().c_str());
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
    }
}
