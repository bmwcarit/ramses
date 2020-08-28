//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/VectorBinaryOutputStream.h"
#include "gtest/gtest.h"

namespace ramses_internal
{
    class AVectorBinaryOutputStream : public ::testing::Test
    {
    public:
        std::vector<Byte> vec;
    };

    TEST_F(AVectorBinaryOutputStream, canInitializeEmpty)
    {
        VectorBinaryOutputStream os(vec);
        EXPECT_TRUE(vec.empty());
        EXPECT_EQ(0u, os.asSpan().size());
    }

    TEST_F(AVectorBinaryOutputStream, canInitializePrefilled)
    {
        vec.assign({1, 2, 3});
        VectorBinaryOutputStream os(vec);
        EXPECT_EQ(3u, vec.size());
        EXPECT_EQ(0u, os.asSpan().size());
    }

    TEST_F(AVectorBinaryOutputStream, canWriteTo)
    {
        VectorBinaryOutputStream os(vec);
        Byte data[3] = {4, 3};
        os.write(data, 2);
        EXPECT_EQ(2u, vec.size());
        ASSERT_EQ(2u, os.asSpan().size());
        EXPECT_EQ(4u, os.asSpan()[0]);
        EXPECT_EQ(3u, os.asSpan()[1]);
        EXPECT_EQ(std::vector<Byte>({4, 3}), vec);
    }

    TEST_F(AVectorBinaryOutputStream, canWriteToPrefilled)
    {
        vec.assign({1, 2, 3});
        VectorBinaryOutputStream os(vec);
        Byte data[3] = {5, 4};
        os.write(data, 2);
        EXPECT_EQ(5u, vec.size());
        ASSERT_EQ(2u, os.asSpan().size());
        EXPECT_EQ(5u, os.asSpan()[0]);
        EXPECT_EQ(4u, os.asSpan()[1]);
        EXPECT_EQ(std::vector<Byte>({1, 2, 3, 5, 4}), vec);
    }
}
