//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/SceneGraph/Resource/LZ4CompressionUtils.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"
#include "gtest/gtest.h"
#include <numeric>

namespace ramses::internal
{
    void checkCompressionDecompression(const std::vector<std::byte>& input)
    {
        for (auto level : { LZ4CompressionUtils::CompressionLevel::Fast,
                            LZ4CompressionUtils::CompressionLevel::High })
        {
            ResourceBlob inBlob(input.size(), input.data());
            CompressedResourceBlob compBlob = LZ4CompressionUtils::compress(inBlob, level);
            EXPECT_GT(compBlob.size(), 0u);
            ASSERT_TRUE(compBlob.data() != nullptr);

            ResourceBlob outBlob = LZ4CompressionUtils::decompress(compBlob, static_cast<uint32_t>(input.size()));
            ASSERT_EQ(inBlob.size(), outBlob.size());
            EXPECT_EQ(input, outBlob.span());
        }
    }

    TEST(LZ4CompressionUtilsTest, TestEmptyCompressionFast)
    {
        CompressedResourceBlob res = LZ4CompressionUtils::compress(ResourceBlob(), LZ4CompressionUtils::CompressionLevel::Fast);
        EXPECT_EQ(0u, res.size());
        EXPECT_TRUE(res.data() == nullptr);
    }

    TEST(LZ4CompressionUtilsTest, TestEmptyCompressionHigh)
    {
        CompressedResourceBlob res = LZ4CompressionUtils::compress(ResourceBlob(), LZ4CompressionUtils::CompressionLevel::High);
        EXPECT_EQ(0u, res.size());
        EXPECT_TRUE(res.data() == nullptr);
    }

    TEST(LZ4CompressionUtilsTest, TestEmptyDecompression)
    {
        ResourceBlob res = LZ4CompressionUtils::decompress(CompressedResourceBlob(), 10);
        EXPECT_EQ(0u, res.size());
        EXPECT_TRUE(res.data() == nullptr);
    }

    TEST(LZ4CompressionUtilsTest, TestDecompressionZeroUncompressedSize)
    {
        ResourceBlob res = LZ4CompressionUtils::decompress(CompressedResourceBlob(10), 0);
        EXPECT_EQ(0u, res.size());
        EXPECT_TRUE(res.data() == nullptr);
    }

    TEST(LZ4CompressionUtilsTest, TestSmallData)
    {
        std::vector<std::byte> small = {std::byte{10}, std::byte{20}, std::byte{30}, std::byte{40}, std::byte{50}, std::byte{60}};
        checkCompressionDecompression(small);
    }

    TEST(LZ4CompressionUtilsTest, TestBigData)
    {
        std::vector<std::byte> big(1024 * 128);
        std::generate(big.begin(), big.end(), [](){ static uint8_t i{4}; return std::byte(++i); });
        checkCompressionDecompression(big);
    }
}
