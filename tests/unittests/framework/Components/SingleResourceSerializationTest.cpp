//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Components/SingleResourceSerialization.h"
#include "gtest/gtest.h"
#include "internal/Core/Utils/BinaryInputStream.h"
#include "internal/Core/Utils/BinaryOutputStream.h"
#include "ResourceSerializationTestHelper.h"
#include <memory>

namespace ramses::internal
{
    namespace
    {
        std::unique_ptr<IResource> SerializeDeserializeCycle(const IResource& res, ResourceContentHash hash)
        {
            BinaryOutputStream outStream;
            SingleResourceSerialization::SerializeResource(outStream, res);

            BinaryInputStream inStream(outStream.getData());
            return std::unique_ptr<IResource>(SingleResourceSerialization::DeserializeResource(inStream, hash, EFeatureLevel_Latest));
        }
    }

    template <typename T>
    class ASingleResourceSerializationTyped : public ::testing::Test
    {
    };

    TYPED_TEST_SUITE(ASingleResourceSerializationTyped, ResourceSerializationTestHelper::Types);

    TYPED_TEST(ASingleResourceSerializationTyped, canSerializeDeserializeToSameResource)
    {
        std::unique_ptr<IResource> res(ResourceSerializationTestHelper::CreateTestResource<TypeParam>(100));
        EXPECT_NE(ResourceContentHash::Invalid(), res->getHash());

        std::unique_ptr<IResource> deserRes = SerializeDeserializeCycle(*res, res->getHash());

        ResourceSerializationTestHelper::CompareResourceValues(*res, *deserRes);
        ResourceSerializationTestHelper::CompareTypedResources(static_cast<const TypeParam&>(*res), static_cast<const TypeParam&>(*deserRes));
    }

    TYPED_TEST(ASingleResourceSerializationTyped, calculatesSameHashAfterDeserializationIfNoneGiven)
    {
        std::unique_ptr<IResource> res(ResourceSerializationTestHelper::CreateTestResource<TypeParam>(100));
        std::unique_ptr<IResource> deserRes = SerializeDeserializeCycle(*res, ResourceContentHash::Invalid());
        ResourceSerializationTestHelper::CompareResourceValues(*res, *deserRes);
    }

    TYPED_TEST(ASingleResourceSerializationTyped, canSerializeCompressedResourceAndResultsInSameResourceAfterDecompression)
    {
        std::unique_ptr<IResource> res(ResourceSerializationTestHelper::CreateTestResource<TypeParam>(3000));  // must be large enough to trigger compression
        res->compress(IResource::CompressionLevel::Realtime);
        EXPECT_TRUE(res->isCompressedAvailable());

        std::unique_ptr<IResource> deserRes = SerializeDeserializeCycle(*res, res->getHash());
        EXPECT_TRUE(deserRes->isCompressedAvailable());
        EXPECT_FALSE(deserRes->isDeCompressedAvailable());

        deserRes->decompress();
        ASSERT_TRUE(res->isDeCompressedAvailable());

        ResourceSerializationTestHelper::CompareResourceValues(*res, *deserRes);
        ResourceSerializationTestHelper::CompareTypedResources(static_cast<const TypeParam&>(*res), static_cast<const TypeParam&>(*deserRes));
    }
}
