//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"
#include "Resource/ResourceBase.h"

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
    }

    INSTANTIATE_TEST_CASE_P(AResourceTest,
                            ResourceCompression,
                            ::testing::Values(IResource::CompressionLevel::REALTIME,
                                              IResource::CompressionLevel::OFFLINE));

    TEST_P(ResourceCompression, compressUncompressGivesInitialDataForSmallSizes)
    {
        for (UInt32 dataSize = 1001; dataSize < 2002; ++dataSize)
        {
            SCOPED_TRACE(dataSize);

            TestResource res(EResourceType_Invalid, ResourceCacheFlag(0), String());
            SceneResourceData data = SceneResourceData(new MemoryBlob(dataSize));
            UInt8* ar = data->getRawData();
            for (UInt32 idx = 0; idx < dataSize; ++idx)
            {
                ar[idx] = static_cast<UInt8>(idx+1);
            }
            res.setResourceData(data);
            res.compress(GetParam());

            TestResource resFromCompressed(EResourceType_Invalid, ResourceCacheFlag(0), String());
            resFromCompressed.setCompressedResourceData(res.getCompressedResourceData(), res.getHash());
            resFromCompressed.decompress();

            ASSERT_EQ(data->size(), resFromCompressed.getResourceData()->size());
            EXPECT_EQ(0, PlatformMemory::Compare(data->getRawData(), resFromCompressed.getResourceData()->getRawData(), data->size()));
        }
    }

    TEST_P(ResourceCompression, noCompressionForSmallSizes)
    {
        for (UInt32 dataSize = 0; dataSize < 1001; ++dataSize)
        {
            SCOPED_TRACE(dataSize);
            TestResource res(EResourceType_Invalid, ResourceCacheFlag(0), String());
            res.setResourceData(SceneResourceData(new MemoryBlob(dataSize)));
            res.compress(GetParam());
            EXPECT_FALSE(res.isCompressedAvailable());
        }
    }

    TEST(AResourceTest, noCompressionForCompressionLevelNone)
    {
        for (UInt32 dataSize = 0; dataSize < 2000; ++dataSize)
        {
            SCOPED_TRACE(dataSize);
            TestResource res(EResourceType_Invalid, ResourceCacheFlag(0), String());
            res.setResourceData(SceneResourceData(new MemoryBlob(dataSize)));
            res.compress(IResource::CompressionLevel::NONE);
            EXPECT_FALSE(res.isCompressedAvailable());
        }
    }

    TEST(AResourceTest, canGetEmptyName)
    {
        TestResource emptyNameRes(EResourceType_Invalid, ResourceCacheFlag(0), String());
        EXPECT_EQ(String(), emptyNameRes.getName());
    }

    TEST(AResourceTest, canGetNonEmptyName)
    {
        TestResource nonEmptyNameRes(EResourceType_Invalid, ResourceCacheFlag(0), String("foobar"));
        EXPECT_EQ(String("foobar"), nonEmptyNameRes.getName());
    }

    TEST(AResourceTest, givesSameHashForDifferentNames)
    {
        TestResource noNameRes(EResourceType_Invalid, ResourceCacheFlag(0), "");
        TestResource namedRes(EResourceType_Invalid, ResourceCacheFlag(0), "some name");
        TestResource otherNamedRes(EResourceType_Invalid, ResourceCacheFlag(0), "other name");

        EXPECT_EQ(noNameRes.getHash(), namedRes.getHash());
        EXPECT_EQ(noNameRes.getHash(), otherNamedRes.getHash());
    }
}
