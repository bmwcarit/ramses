//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Core/Utils/VectorBinaryOutputStream.h"
#include "gtest/gtest.h"

namespace ramses::internal
{
    template <typename... Ts>
    std::vector<std::byte> make_byte_vector(Ts&&... args) noexcept
    {
        return {std::byte(std::forward<Ts>(args))...};
    }

    class AVectorBinaryOutputStream : public ::testing::Test
    {
    public:
        std::vector<std::byte> vec;
    };

    TEST_F(AVectorBinaryOutputStream, canInitializeEmpty)
    {
        VectorBinaryOutputStream os(vec);
        EXPECT_TRUE(vec.empty());
        EXPECT_EQ(0u, os.asSpan().size());

        size_t position = std::numeric_limits<size_t>::max();
        EXPECT_EQ(EStatus::Ok, os.getPos(position));
        EXPECT_EQ(0u, position);
    }

    TEST_F(AVectorBinaryOutputStream, canInitializePrefilled)
    {
        vec = make_byte_vector(1, 2, 3);
        VectorBinaryOutputStream os(vec);
        EXPECT_EQ(3u, vec.size());
        EXPECT_EQ(0u, os.asSpan().size());

        size_t position = std::numeric_limits<size_t>::max();
        EXPECT_EQ(EStatus::Ok, os.getPos(position));
        EXPECT_EQ(0u, position);
    }

    TEST_F(AVectorBinaryOutputStream, canWriteTo)
    {
        VectorBinaryOutputStream os(vec);
        std::byte data[3] = {std::byte{4}, std::byte{3}};
        os.write(data, 2);
        EXPECT_EQ(2u, vec.size());
        ASSERT_EQ(2u, os.asSpan().size());
        EXPECT_EQ(std::byte{4u}, os.asSpan()[0]);
        EXPECT_EQ(std::byte{3u}, os.asSpan()[1]);
        EXPECT_EQ(make_byte_vector(4, 3), vec);

        size_t position = std::numeric_limits<size_t>::max();
        EXPECT_EQ(EStatus::Ok, os.getPos(position));
        EXPECT_EQ(2u, position);
    }

    TEST_F(AVectorBinaryOutputStream, canWriteToPrefilled)
    {
        vec = make_byte_vector(1, 2, 3);
        VectorBinaryOutputStream os(vec);
        std::byte data[3] = {std::byte{5}, std::byte{4}};
        os.write(data, 2);
        EXPECT_EQ(5u, vec.size());
        ASSERT_EQ(2u, os.asSpan().size());
        EXPECT_EQ(std::byte{5u}, os.asSpan()[0]);
        EXPECT_EQ(std::byte{4u}, os.asSpan()[1]);
        EXPECT_EQ(make_byte_vector(1, 2, 3, 5, 4), vec);

        size_t position = std::numeric_limits<size_t>::max();
        EXPECT_EQ(EStatus::Ok, os.getPos(position));
        EXPECT_EQ(2u, position);
    }
}
