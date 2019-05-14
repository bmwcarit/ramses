//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/LZ4CompressionUtils.h"
#include "PlatformAbstraction/PlatformMemory.h"
#include "Collections/Vector.h"
#include "gtest/gtest.h"
#include <memory>

namespace ramses_internal
{
    void checkCompressionDecompression(const std::vector<UInt8>& input)
    {
        const auto size = static_cast<UInt32>(input.size());
        for (auto level : { LZ4CompressionUtils::CompressionLevel::Fast,
                            LZ4CompressionUtils::CompressionLevel::High })
        {
            const UInt32 comressedBound = LZ4CompressionUtils::compressedSizeBound(static_cast<UInt32>(input.size()));
            UInt32 compressedSize = comressedBound;
            HeapArray<UInt8> comp(comressedBound);
            EXPECT_TRUE(LZ4CompressionUtils::compress(comp, compressedSize, input.data(), size, level));
            EXPECT_LE(compressedSize, comressedBound);

            HeapArray<UInt8> decomp(size);
            EXPECT_TRUE(LZ4CompressionUtils::decompress(decomp, comp.data(), compressedSize));
            EXPECT_EQ(0, PlatformMemory::Compare(decomp.data(), input.data(), size));
        }
    }

    TEST(LZ4CompressionUtilsTest, TestEmptyDataFrame)
    {
        checkCompressionDecompression(std::vector<UInt8>());
    }

    TEST(LZ4CompressionUtilsTest, TestSmallDataFrame)
    {
        std::vector<UInt8> small(6);
        PlatformMemory::Copy(small.data(), "foobar", small.size());
        checkCompressionDecompression(small);
    }

    TEST(LZ4CompressionUtilsTest, TestBigDataFrame)
    {
        const UInt32 size = 1024 * 128;
        std::vector<UInt8> big(size);
        for (auto i = 0u; i < size; ++i)
        {
            PlatformMemory::Set(big.data() + i, static_cast<UInt8>(10+i), sizeof(UInt8));
        }

        checkCompressionDecompression(big);
    }
}
