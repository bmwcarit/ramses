//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "Resource/ResourceBase.h"
#include "gtest/gtest.h"
#include <numeric>

namespace ramses_internal
{
    namespace
    {
        class TestResource : public ResourceBase
        {
        public:
            explicit TestResource(EResourceType typeID, ResourceCacheFlag cacheFlag, const String& name)
                : ResourceBase(typeID, cacheFlag, name)
            {}

            virtual void serializeResourceMetadataToStream(IOutputStream&) const override {}
        };

        class ResourceCompression : public ::testing::TestWithParam<IResource::CompressionLevel>
        {
        };

        class DummyResource : public ResourceBase
        {
        public:
            DummyResource(uint32_t metadata = 0, const String& name = String())
                : ResourceBase(EResourceType_Invalid, ResourceCacheFlag(15u), name)
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

    INSTANTIATE_TEST_SUITE_P(AResourceTest,
                            ResourceCompression,
                            ::testing::Values(IResource::CompressionLevel::REALTIME,
                                              IResource::CompressionLevel::OFFLINE));

    TEST_P(ResourceCompression, compressUncompressGivesInitialDataForSmallSizes)
    {
        for (UInt32 dataSize = 1001; dataSize < 2002; ++dataSize)
        {
            SCOPED_TRACE(dataSize);

            TestResource res(EResourceType_Invalid, ResourceCacheFlag(0), String());
            ResourceBlob data(dataSize);
            for (UInt32 idx = 0; idx < dataSize; ++idx)
            {
                data.data()[idx] = static_cast<UInt8>(idx+1);
            }
            res.setResourceData(ResourceBlob(data.size(), data.data()));  // copy data
            res.compress(GetParam());

            TestResource resFromCompressed(EResourceType_Invalid, ResourceCacheFlag(0), String());
            resFromCompressed.setCompressedResourceData(CompressedResouceBlob(res.getCompressedResourceData().size(), res.getCompressedResourceData().data()),
                                                        res.getDecompressedDataSize(), res.getHash());
            resFromCompressed.decompress();

            ASSERT_EQ(data.size(), resFromCompressed.getResourceData().size());
            EXPECT_EQ(0, PlatformMemory::Compare(data.data(), resFromCompressed.getResourceData().data(), data.size()));
        }
    }

    TEST_P(ResourceCompression, noCompressionForSmallSizes)
    {
        for (UInt32 dataSize = 1; dataSize < 1001; ++dataSize)
        {
            SCOPED_TRACE(dataSize);
            TestResource res(EResourceType_Invalid, ResourceCacheFlag(0), String());
            res.setResourceData(ResourceBlob(dataSize));
            res.compress(GetParam());
            EXPECT_FALSE(res.isCompressedAvailable());
        }
    }

    class AResource : public ::testing::Test
    {
    public:
        AResource()
            : zeroBlobA(2048)
            , zeroBlobB(2048)
            , compressedBlob(10)
        {
            zeroBlobA.setZero();
            zeroBlobB.setZero();
            compressedBlob.setZero();
        }

        ResourceBlob zeroBlobA;
        ResourceBlob zeroBlobB;
        CompressedResouceBlob compressedBlob;
    };

    TEST_F(AResource, noCompressionForCompressionLevelNone)
    {
        for (UInt32 dataSize = 1; dataSize < 2000; ++dataSize)
        {
            SCOPED_TRACE(dataSize);
            TestResource res(EResourceType_Invalid, ResourceCacheFlag(0), String());
            res.setResourceData(ResourceBlob(dataSize));
            res.compress(IResource::CompressionLevel::NONE);
            EXPECT_FALSE(res.isCompressedAvailable());
        }
    }

    TEST_F(AResource, canGetEmptyName)
    {
        TestResource emptyNameRes(EResourceType_Invalid, ResourceCacheFlag(0), String());
        EXPECT_EQ(String(), emptyNameRes.getName());
    }

    TEST_F(AResource, canGetNonEmptyName)
    {
        TestResource nonEmptyNameRes(EResourceType_Invalid, ResourceCacheFlag(0), String("foobar"));
        EXPECT_EQ(String("foobar"), nonEmptyNameRes.getName());
    }

    TEST_F(AResource, givesSameHashForDifferentNames)
    {
        TestResource noNameRes(EResourceType_Invalid, ResourceCacheFlag(0), "");
        TestResource namedRes(EResourceType_Invalid, ResourceCacheFlag(0), "some name");
        TestResource otherNamedRes(EResourceType_Invalid, ResourceCacheFlag(0), "other name");

        EXPECT_EQ(noNameRes.getHash(), namedRes.getHash());
        EXPECT_EQ(noNameRes.getHash(), otherNamedRes.getHash());
    }

    TEST_F(AResource, canGetType)
    {
        TestResource res(EResourceType_Effect, ResourceCacheFlag(0), String());
        EXPECT_EQ(EResourceType_Effect, res.getTypeID());
    }

    TEST_F(AResource, canGetCacheFlag)
    {
        TestResource res(EResourceType_Invalid, ResourceCacheFlag(11), String());
        EXPECT_EQ(ResourceCacheFlag(11), res.getCacheFlag());
    }

    TEST_F(AResource, returnsInvalidHashForEmptyResources)
    {
        DummyResource res;
        EXPECT_EQ(ResourceContentHash::Invalid(), res.getHash());
    }

    TEST_F(AResource, hasGivenHashWhenExplicitlySet)
    {
        DummyResource res;
        ResourceContentHash someHash(1234568, 0);
        res.setResourceData(std::move(zeroBlobA), someHash);
        EXPECT_EQ(someHash, res.getHash());
    }

    TEST_F(AResource, hasGivenHashWhenExplicitlySetForCompressed)
    {
        DummyResource res;
        ResourceContentHash someHash(1234568, 0);
        res.setCompressedResourceData(std::move(compressedBlob), 1, someHash);
        EXPECT_EQ(someHash, res.getHash());
    }

    TEST_F(AResource, calculatesValidHashWhenNoneSet)
    {
        DummyResource res;
        res.setResourceData(std::move(zeroBlobA));
        EXPECT_NE(ResourceContentHash::Invalid(), res.getHash());
    }


    TEST_F(AResource, hashChangesWhenContentChanges)
    {
        DummyResource res;
        res.setResourceData(std::move(zeroBlobA));
        const ResourceContentHash hash = res.getHash();
        zeroBlobB.data()[0] = 1;
        res.setResourceData(std::move(zeroBlobB));
        EXPECT_NE(hash, res.getHash());
    }

    TEST_F(AResource, givesSameHashForSameContent)
    {
        DummyResource resA;
        resA.setResourceData(std::move(zeroBlobA));
        DummyResource resB;
        resB.setResourceData(std::move(zeroBlobB));

        EXPECT_EQ(resA.getHash(), resB.getHash());
    }

    TEST_F(AResource, hashIsDifferentForSameContentButDifferentMetadata)
    {
        DummyResource resA(1);
        resA.setResourceData(std::move(zeroBlobA));

        DummyResource resB(2);
        resB.setResourceData(std::move(zeroBlobB));

        EXPECT_NE(resA.getHash(), resB.getHash());
    }

    TEST_F(AResource, contentSameAfterCompressDecompress)
    {
        DummyResource resA;
        ResourceBlob blob(4096);
        std::iota(blob.data(), blob.data() + blob.size(), static_cast<uint8_t>(10));
        resA.setResourceData(std::move(blob));
        resA.compress(IResource::CompressionLevel::REALTIME);
        ASSERT_TRUE(resA.isCompressedAvailable());
        ASSERT_TRUE(resA.isDeCompressedAvailable());

        const CompressedResouceBlob& compBlobA = resA.getCompressedResourceData();
        DummyResource resB;
        resB.setCompressedResourceData(CompressedResouceBlob(compBlobA.size(), compBlobA.data()), resA.getDecompressedDataSize(), resA.getHash());
        EXPECT_FALSE(resB.isDeCompressedAvailable());
        resB.decompress();
        ASSERT_TRUE(resB.isDeCompressedAvailable());
        EXPECT_TRUE(resB.isCompressedAvailable());

        ASSERT_EQ(resA.getDecompressedDataSize(), resB.getDecompressedDataSize());
        EXPECT_EQ(0, std::memcmp(resA.getResourceData().data(), resB.getResourceData().data(), resA.getDecompressedDataSize()));
    }

    TEST_F(AResource, canCompressDecompressSameResource)
    {
        DummyResource resA(1);
        resA.setResourceData(std::move(zeroBlobA));
        resA.compress(IResource::CompressionLevel::REALTIME);
        resA.decompress();
        EXPECT_TRUE(resA.isCompressedAvailable());
        EXPECT_TRUE(resA.isDeCompressedAvailable());
    }
}
