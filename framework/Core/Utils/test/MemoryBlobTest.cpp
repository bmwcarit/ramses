//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"
#include "Utils/MemoryBlob.h"
#include "Utils/CompressedMemoryBlob.h"
#include "PlatformAbstraction/PlatformMemory.h"

using namespace testing;

namespace ramses_internal
{
    TEST(MemoryBlobTest, PreallocatesWithGivenSize)
    {
        const MemoryBlob blob(99u);
        EXPECT_EQ(99u, blob.size());
    }

    TEST(MemoryBlobTest, InitializedWithData)
    {
        static const UInt8 data[] = { 1, 2, 3, 4, 5, 6 };

        const MemoryBlob blob(data, sizeof(data));
        EXPECT_EQ(sizeof(data), blob.size());
        EXPECT_EQ(0, PlatformMemory::Compare(data, blob.getRawData(), sizeof(data)));
        EXPECT_EQ(3, blob[2]);
    }

    TEST(MemoryBlobTest, ElementCanBeSetAndGet)
    {
        MemoryBlob blob(3u);
        blob[1] = 99;
        EXPECT_EQ(99, blob[1]);
        blob[1] = 11;
        EXPECT_EQ(11, blob[1]);
    }

    TEST(CompressedMemoryBlobTest, PreallocateWithGivenSize)
    {
        CompressedMemoryBlob compressedBlob(10u, 20u);
        EXPECT_EQ(compressedBlob.getDecompressedSize(), 20u);
        EXPECT_EQ(compressedBlob.size(), 10u);
    }

    TEST(CompressedMemoryBlobTest, InitializedWithData)
    {
        const UInt32 size = 1024;
        UInt8 data[size];
        PlatformMemory::Set(&data[0], 0xaa, size);

        MemoryBlob blob(&data[0], size);
        CompressedMemoryBlob compressedBlob(blob, LZ4CompressionUtils::CompressionLevel::Fast);

        EXPECT_EQ(compressedBlob.getDecompressedSize(), blob.size());
        EXPECT_LT(compressedBlob.size(), blob.size());

        CompressedMemoryBlob copyCompressedBlob(compressedBlob.getRawData(), compressedBlob.size(), compressedBlob.getDecompressedSize());

        EXPECT_EQ(compressedBlob.size(), copyCompressedBlob.size());
        EXPECT_EQ(compressedBlob.getDecompressedSize(), copyCompressedBlob.getDecompressedSize());
        EXPECT_EQ(PlatformMemory::Compare(compressedBlob.getRawData(), copyCompressedBlob.getRawData(), copyCompressedBlob.size()), 0);
    }

    TEST(CompressedMemoryBlobTest, CompressAndDecompressMemoryBlob)
    {
        for (auto compressionLevel : {  LZ4CompressionUtils::CompressionLevel::Fast,
                                        LZ4CompressionUtils::CompressionLevel::High })
        {
            const UInt32 size = 1024;
            UInt8 data[size];
            PlatformMemory::Set(&data[0], 0xaa, size);

            MemoryBlob blob(&data[0], size);
            CompressedMemoryBlob compressedBlob(blob, compressionLevel);

            EXPECT_EQ(compressedBlob.getDecompressedSize(), blob.size());
            EXPECT_LT(compressedBlob.size(), blob.size());

            MemoryBlob decompressedBlob(compressedBlob);

            EXPECT_EQ(decompressedBlob.size(), blob.size());
            EXPECT_EQ(PlatformMemory::Compare(decompressedBlob.getRawData(), blob.getRawData(), decompressedBlob.size()), 0);
        }
    }
}
