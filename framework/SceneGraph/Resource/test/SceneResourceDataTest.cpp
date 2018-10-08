//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"
#include "Resource/ResourceBase.h"
#include "PlatformAbstraction/PlatformMemory.h"

namespace
{
    static const ramses_internal::ResourceCacheFlag TestCacheFlag(15u);
}

namespace ramses_internal
{
    namespace
    {
        class DummyResource : public ResourceBase
        {
        public:
            DummyResource(uint32_t metadata = 0, const String& name = String())
                : ResourceBase(EResourceType_Invalid, TestCacheFlag, name)
                , m_metadata(metadata)
            {}

            virtual void serializeResourceMetadataToStream(IOutputStream& output) const override
            {
                output << m_metadata;
            }

        private:
            uint32_t m_metadata;
        };
    }

    TEST(SceneResourceData, returnsInvalidHashForEmptyResources)
    {
        DummyResource resA;
        ResourceContentHash hashValue = resA.getHash();
        EXPECT_EQ(ResourceContentHash::Invalid(), hashValue);
    }

    TEST(SceneResourceData, HasGivenHash)
    {
        DummyResource resA;
        SceneResourceData data(new MemoryBlob(8));
        ResourceContentHash someHash(1234568, 0);
        resA.setResourceData(data, someHash);
        ResourceContentHash hashValue = resA.getHash();
        EXPECT_EQ(someHash, hashValue);
    }

    TEST(SceneResourceData, HasValidHashWhenAskedToUpdateHash)
    {
        DummyResource resA;
        SceneResourceData data(new MemoryBlob(8));
        data->setDataToZero();
        resA.setResourceData(data);
        ResourceContentHash hashValue = resA.getHash();
        EXPECT_NE(ResourceContentHash::Invalid(), hashValue);
    }

    TEST(SceneResourceData, HashChangesWithContentChange)
    {
        DummyResource resA;
        SceneResourceData data(new MemoryBlob(8));
        data->setDataToZero();
        resA.setResourceData(data);
        const ResourceContentHash originalHashValue = resA.getHash();
        EXPECT_NE(ResourceContentHash::Invalid(), originalHashValue);

        SceneResourceData differentData(new MemoryBlob(6));
        differentData->setDataToZero();
        resA.setResourceData(differentData);
        const ResourceContentHash newHashValue = resA.getHash();
        EXPECT_NE(newHashValue, originalHashValue);
    }

    TEST(SceneResourceData, givesSameHashForSameContent)
    {
        const UInt8 resourceContent[8] = { "1234567" };
        DummyResource resA;
        SceneResourceData dataA(new MemoryBlob(resourceContent, 8));
        resA.setResourceData(dataA);
        DummyResource resB;
        SceneResourceData dataB(new MemoryBlob(resourceContent, 8));
        resB.setResourceData(dataB);
        EXPECT_EQ(resB.getHash(), resA.getHash());

        SceneResourceData differentDataA(new MemoryBlob(resourceContent, 6));
        resA.setResourceData(differentDataA);
        EXPECT_NE(resB.getHash(), resA.getHash());

        SceneResourceData differentDataB(new MemoryBlob(resourceContent, 6));
        resB.setResourceData(differentDataB);
        EXPECT_EQ(resB.getHash(), resA.getHash());
    }

    TEST(SceneResourceData, hashIsDifferentForSameContentButDifferentMetadata)
    {
        DummyResource resA(1);
        SceneResourceData dataA(new MemoryBlob(8));
        dataA->setDataToZero();
        resA.setResourceData(dataA);
        DummyResource resB(2);
        SceneResourceData dataB(new MemoryBlob(8));
        dataB->setDataToZero();
        resB.setResourceData(dataB);
        EXPECT_NE(resB.getHash(), resA.getHash());
    }

    TEST(SceneResourceData, initUncompressedThenCompressAndDecompressContent)
    {
        DummyResource resA;

        const UInt dataSize = 1024;
        SceneResourceData data = SceneResourceData(new MemoryBlob(1024));
        PlatformMemory::Set(data->getRawData(), 0xaa, dataSize);

        resA.setResourceData(data);
        EXPECT_EQ(resA.isCompressedAvailable(), false);

        // shared pointer references same data?
        EXPECT_EQ(resA.getResourceData(), data);

        resA.compress(IResource::CompressionLevel::REALTIME);
        EXPECT_EQ(resA.isCompressedAvailable(), true);

        // compressed size smaller than uncompressed size?
        EXPECT_LT(resA.getCompressedResourceData()->size(), data->size());

        resA.decompress();
        EXPECT_EQ(resA.isCompressedAvailable(), true);
        EXPECT_EQ(resA.isDeCompressedAvailable(), true);

        // data still the same after compression/decompression?
        EXPECT_EQ(resA.getResourceData()->size(), data->size());
        EXPECT_EQ(PlatformMemory::Compare(resA.getResourceData()->getRawData(), data->getRawData(), data->size()), 0);
    }

    TEST(SceneResourceData, initCompressedThenDecompressAndCompressContent)
    {
        DummyResource resA;

        const UInt dataSize = 1024;
        SceneResourceData data = SceneResourceData(new MemoryBlob(1024));
        PlatformMemory::Set(data->getRawData(), 0xaa, dataSize);

        CompressedSceneResourceData compressedData = CompressedSceneResourceData(
            new CompressedMemoryBlob(*data.get(), LZ4CompressionUtils::CompressionLevel::Fast));

        resA.setCompressedResourceData(compressedData, ramses_internal::ResourceContentHash::Invalid());
        EXPECT_EQ(resA.isCompressedAvailable(), true);
        EXPECT_EQ(resA.isDeCompressedAvailable(), false);

        // shared pointer references same data?
        EXPECT_EQ(resA.getCompressedResourceData(), compressedData);

        resA.decompress();
        // now both are available
        EXPECT_EQ(resA.isCompressedAvailable(), true);
        EXPECT_EQ(resA.isDeCompressedAvailable(), true);

        // decompressed size bigger than compressed size and
        // data the same as original uncompressed content?
        EXPECT_GT(resA.getResourceData()->size(), compressedData->size());
        EXPECT_EQ(resA.getResourceData()->size(), data->size());
        EXPECT_EQ(PlatformMemory::Compare(resA.getResourceData()->getRawData(), data->getRawData(), data->size()), 0);

        resA.compress(IResource::CompressionLevel::REALTIME);
        EXPECT_EQ(resA.isCompressedAvailable(), true);

        // compressed size smaller than decompressed size and
        // data the same as original compressed content?
        EXPECT_LT(resA.getCompressedResourceData()->size(), data->size());
        EXPECT_EQ(resA.getCompressedResourceData()->size(), compressedData->size());
        EXPECT_EQ(PlatformMemory::Compare(resA.getCompressedResourceData()->getRawData(), compressedData->getRawData(), compressedData->size()), 0);
    }

    TEST(SceneResourceData, hashForUncompressedAndCompressedIsSame)
    {
        DummyResource resA(1);
        SceneResourceData dataA(new MemoryBlob(100*1000));
        dataA->setDataToZero();
        resA.setResourceData(dataA);
        ResourceContentHash hashUncompressed = resA.getHash();

        DummyResource resB(1);
        SceneResourceData dataB(new MemoryBlob(100*1000));
        dataB->setDataToZero();
        resB.setResourceData(dataB);
        resB.compress(IResource::CompressionLevel::REALTIME);
        ResourceContentHash hashCompressed = resB.getHash();

        EXPECT_EQ(hashUncompressed, hashCompressed);
    }

    TEST(SceneResourceData, storesCacheFlag)
    {
        DummyResource resWithCacheFlag;
        EXPECT_EQ(resWithCacheFlag.getCacheFlag(), TestCacheFlag);
    }
}
