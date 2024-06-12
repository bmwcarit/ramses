//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Communication/TransportCommon/SceneUpdateSerializationHelper.h"
#include "internal/SceneGraph/Scene/SceneActionCollection.h"
#include "ResourceSerializationTestHelper.h"
#include "gtest/gtest.h"

namespace ramses::internal
{
    class ASceneActionSerialization : public ::testing::Test
    {
    public:
        SceneActionCollection SerializeDeserialize(const SceneActionCollection& actions)
        {
            const absl::Span<const std::byte> desc = SceneActionSerialization::SerializeDescription(actions, workingMem);
            const absl::Span<const std::byte> data = SceneActionSerialization::SerializeData(actions);
            return SceneActionSerialization::Deserialize(desc, data);
        }

        std::vector<std::byte> workingMem;
    };

    TEST_F(ASceneActionSerialization, canSerializeDeserializeEmptyCollection)
    {
        SceneActionCollection in;
        EXPECT_EQ(in, SerializeDeserialize(in));
    }

    TEST_F(ASceneActionSerialization, canSerializeDeserializeCollectionWithData)
    {
        SceneActionCollection in;
        in.beginWriteSceneAction(ESceneActionId::TestAction);
        in.write(static_cast<uint32_t>(123));
        in.write(static_cast<uint32_t>(456));
        in.beginWriteSceneAction(ESceneActionId::AllocateNode);
        in.write("foobar");

        EXPECT_EQ(in, SerializeDeserialize(in));
    }


    class AResourceSerialization : public ::testing::Test
    {
    public:
        std::unique_ptr<IResource> SerializeDeserialize(const IResource& resource)
        {
            const absl::Span<const std::byte> desc = ResourceSerialization::SerializeDescription(resource, workingMem);
            const absl::Span<const std::byte> data = ResourceSerialization::SerializeData(resource);
            return ResourceSerialization::Deserialize(desc, data, EFeatureLevel_Latest);
        }

        std::vector<std::byte> workingMem;
    };

    template <typename T>
    class AResourceSerializationTyped : public AResourceSerialization
    {
    };

    TYPED_TEST_SUITE(AResourceSerializationTyped, ResourceSerializationTestHelper::Types);

    TYPED_TEST(AResourceSerializationTyped, canSerializeDeserializeToSameResource)
    {
        std::unique_ptr<IResource> res(ResourceSerializationTestHelper::CreateTestResource<TypeParam>(100));
        EXPECT_NE(ResourceContentHash::Invalid(), res->getHash());

        std::unique_ptr<IResource> deserRes(this->SerializeDeserialize(*res));
        ASSERT_TRUE(deserRes);

        ResourceSerializationTestHelper::CompareResourceValues(*res, *deserRes);
        ResourceSerializationTestHelper::CompareTypedResources(static_cast<const TypeParam&>(*res), static_cast<const TypeParam&>(*deserRes));
    }

    TEST_F(AResourceSerialization, canSerializeDeserializeWithoutBlob)
    {
        const std::unique_ptr<IResource> res(ResourceSerializationTestHelper::CreateTestResource<ArrayResource>(0));
        std::unique_ptr<IResource> deserRes(this->SerializeDeserialize(*res));
        ASSERT_TRUE(deserRes);

        EXPECT_EQ(0u, deserRes->getCompressedDataSize());
        EXPECT_EQ(0u, deserRes->getDecompressedDataSize());

        ResourceSerializationTestHelper::CompareResourceValues(*res, *deserRes);
        ResourceSerializationTestHelper::CompareTypedResources(static_cast<const ArrayResource&>(*res), static_cast<const ArrayResource&>(*deserRes));
    }

    TEST_F(AResourceSerialization, canSerializeDeserializeCompressedResource)
    {
        const std::unique_ptr<IResource> res(ResourceSerializationTestHelper::CreateTestResource<ArrayResource>(10000));
        res->compress(IResource::CompressionLevel::Realtime);
        ASSERT_TRUE(res->isCompressedAvailable());

        std::unique_ptr<IResource> deserRes(this->SerializeDeserialize(*res));
        ASSERT_TRUE(deserRes);
        EXPECT_TRUE(deserRes->isCompressedAvailable());

        deserRes->decompress();
        ResourceSerializationTestHelper::CompareResourceValues(*res, *deserRes);
        ResourceSerializationTestHelper::CompareTypedResources(static_cast<const ArrayResource&>(*res), static_cast<const ArrayResource&>(*deserRes));
    }

    TEST_F(AResourceSerialization, deserializeFailsWithDataSizeMatch)
    {
        const std::unique_ptr<IResource> res(ResourceSerializationTestHelper::CreateTestResource<ArrayResource>(1000));
        const absl::Span<const std::byte> desc = ResourceSerialization::SerializeDescription(*res, workingMem);
        const absl::Span<const std::byte> data = ResourceSerialization::SerializeData(*res);

        ASSERT_GT(data.size(), 0u);
        std::unique_ptr<IResource> deserRes(ResourceSerialization::Deserialize(desc, data.subspan(1), EFeatureLevel_Latest));
        ASSERT_FALSE(deserRes);
    }
}
