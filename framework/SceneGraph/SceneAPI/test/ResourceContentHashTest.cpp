//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneAPI/ResourceContentHash.h"
#include "Utils/BinaryOutputStream.h"
#include "Utils/BinaryInputStream.h"
#include "Collections/HashSet.h"
#include "gtest/gtest.h"
#include <unordered_set>

namespace ramses_internal
{
    TEST(AResourceContentHash, canDefaultConstruct)
    {
        constexpr ResourceContentHash h1;
        EXPECT_EQ(0, h1.lowPart);
        EXPECT_EQ(0, h1.highPart);
    }

    TEST(AResourceContentHash, InvalidIsDefault)
    {
        constexpr ResourceContentHash h2(ResourceContentHash::Invalid());
        EXPECT_EQ(ResourceContentHash(), h2);
        EXPECT_EQ(0, h2.lowPart);
        EXPECT_EQ(0, h2.highPart);
    }

    TEST(AResourceContentHash, canValueConstruct)
    {
        constexpr ResourceContentHash h1{0, 0};
        EXPECT_EQ(0, h1.lowPart);
        EXPECT_EQ(0, h1.highPart);

        constexpr ResourceContentHash h2{123, 456};
        EXPECT_EQ(123, h2.lowPart);
        EXPECT_EQ(456, h2.highPart);

        constexpr ResourceContentHash h3(1234, 4567);
        EXPECT_EQ(1234, h3.lowPart);
        EXPECT_EQ(4567, h3.highPart);
    }

    TEST(AResourceContentHash, canCompare)
    {
        constexpr ResourceContentHash h1{1, 2};
        constexpr ResourceContentHash h2{1, 3};
        constexpr ResourceContentHash h3{2, 2};
        constexpr ResourceContentHash h4{1, 2};

        EXPECT_TRUE(h1 == h1);
        EXPECT_TRUE(h1 == h4);
        EXPECT_FALSE(h1 == h2);
        EXPECT_FALSE(h1 == h3);

        EXPECT_FALSE(h1 != h1);
        EXPECT_FALSE(h1 != h4);
        EXPECT_TRUE(h1 != h2);
        EXPECT_TRUE(h1 != h3);

        EXPECT_FALSE(h1 < h4);
        EXPECT_TRUE(h1 < h2);
        EXPECT_TRUE(h1 < h3);
    }

    TEST(AResourceContentHash, canSerializeDeserializeBinary)
    {
        constexpr ResourceContentHash o1{0xcafebabe, 0xdeadbeef};
        constexpr ResourceContentHash o2{0, 0};
        constexpr ResourceContentHash o3{0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};
        BinaryOutputStream out;
        out << o1 << o2 << o3;

        BinaryInputStream in(out.getData());
        ResourceContentHash i1{1, 1};
        ResourceContentHash i2{1, 1};
        ResourceContentHash i3{1, 1};
        in >> i1 >> i2 >> i3;

        EXPECT_EQ(o1, i1);
        EXPECT_EQ(o2, i2);
        EXPECT_EQ(o3, i3);
    }

    TEST(AResourceContentHash, canPrintToStream)
    {
        ResourceContentHash zero_hash(0, 0);
        ResourceContentHash small_hash(6342u, 0);
        ResourceContentHash big_hash(0x0123456789abcdef, 0xfedcba9876543210);

        EXPECT_EQ("0x00000000000000000000000000000000", StringOutputStream::ToString(zero_hash));
        EXPECT_EQ("0x000000000000000000000000000018C6", StringOutputStream::ToString(small_hash));
        EXPECT_EQ("0xFEDCBA98765432100123456789ABCDEF", StringOutputStream::ToString(big_hash));
    }

    TEST(AResourceContentHash, canBeUsedInHashContainers)
    {
        HashSet<ResourceContentHash> hs;
        hs.put(ResourceContentHash{1, 2});
        hs.put(ResourceContentHash{2, 3});
        hs.put(ResourceContentHash{1, 2});
        EXPECT_EQ(2u, hs.size());

        std::unordered_set<ResourceContentHash> us;
        us.insert(ResourceContentHash{1, 2});
        us.insert(ResourceContentHash{2, 3});
        us.insert(ResourceContentHash{1, 2});
        EXPECT_EQ(2u, us.size());
    }
}
