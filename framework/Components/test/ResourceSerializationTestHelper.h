//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCESERIALIZATIONTESTHELPER_H
#define RAMSES_RESOURCESERIALIZATIONTESTHELPER_H

#include "Resource/TextureResource.h"
#include "Resource/ArrayResource.h"
#include "Resource/EffectResource.h"
#include "gtest/gtest.h"
#include "Collections/Vector.h"
#include "Components/ManagedResource.h"
#include "Components/ResourceStreamSerialization.h"
#include "TestRandom.h"

namespace ramses_internal
{
    class ResourceSerializationTestHelper
    {
    public:
        template <typename T>
        static IResource* CreateTestResource(UInt32 blobSize);

        template <typename T>
        static void CompareTypedResources(const T& a, const T& b);

        static void CompareResourceValues(const IResource& a, const IResource& b);

        static void SetResourceDataRandom(IResource& res, UInt32 blobSize);

        using Types = ::testing::Types<TextureResource, ArrayResource, EffectResource>;

        static std::vector<std::vector<Byte>> ConvertResourcesToResourceDataVector(const ManagedResourceVector& resources, UInt32 chunkSize);
    };

    inline std::vector<std::vector<Byte>> ResourceSerializationTestHelper::ConvertResourcesToResourceDataVector(const ManagedResourceVector& resources, UInt32 chunkSize)
    {
        std::vector<std::vector<Byte>> resData;
        std::vector<Byte> buffer;
        auto preparePacketFun = [&](UInt32 neededSize) -> std::pair<Byte*, UInt32> {
            buffer.resize(std::min(chunkSize, neededSize));
            return std::make_pair(buffer.data(), static_cast<UInt32>(buffer.size()));
        };
        auto finishedPacketFun = [&](UInt32 usedSize) {
            assert(usedSize <= buffer.size());
            buffer.resize(usedSize);
            resData.push_back(buffer);
        };

        ResourceStreamSerializer serializer;
        serializer.serialize(preparePacketFun, finishedPacketFun, resources);
        return resData;
    }

    inline void ResourceSerializationTestHelper::CompareResourceValues(const IResource& a, const IResource& b)
    {
        ASSERT_EQ(a.getTypeID(), b.getTypeID());
        EXPECT_EQ(a.getHash(), b.getHash());
        EXPECT_EQ(a.getCacheFlag(), b.getCacheFlag());
        EXPECT_EQ(a.getName(), b.getName());

        ASSERT_TRUE(a.isDeCompressedAvailable());
        ASSERT_TRUE(b.isDeCompressedAvailable());
        ASSERT_TRUE(a.getResourceData().get() != nullptr);
        ASSERT_TRUE(b.getResourceData().get() != nullptr);
        ASSERT_EQ(a.getResourceData()->size(), b.getResourceData()->size());
        EXPECT_EQ(0, PlatformMemory::Compare(a.getResourceData()->getRawData(), b.getResourceData()->getRawData(), a.getResourceData()->size()));
    }

    inline void ResourceSerializationTestHelper::SetResourceDataRandom(IResource& res, UInt32 blobSize)
    {
        const uint8_t seed = static_cast<UInt8>(TestRandom::Get(0, 256));
        SceneResourceData data(new MemoryBlob(blobSize));
        for (UInt32 i = 0; i < data->size(); ++i)
        {
            (*data)[i] = static_cast<uint8_t>(i + seed);
        }
        res.setResourceData(data);
    }

    // TextureResource
    template <>
    inline IResource* ResourceSerializationTestHelper::CreateTestResource<TextureResource>(UInt32 blobSize)
    {
        const TextureMetaInfo texDesc = { 16u, 17u, 1u, ETextureFormat_RGBA16, false, { 18u, 19u} };
        TextureResource* resource = new TextureResource(EResourceType_Texture3D, texDesc, ResourceCacheFlag(15u), "resName");
        SetResourceDataRandom(*resource, blobSize);
        return resource;
    }

    template <>
    inline void ResourceSerializationTestHelper::CompareTypedResources(const TextureResource& a, const TextureResource& b)
    {
        EXPECT_EQ(a.getWidth(), b.getWidth());
        EXPECT_EQ(a.getHeight(), b.getHeight());
        EXPECT_EQ(a.getDepth(), b.getDepth());
        EXPECT_EQ(a.getMipDataSizes(), b.getMipDataSizes());
        EXPECT_EQ(a.getTextureFormat(), b.getTextureFormat());
        EXPECT_EQ(a.getGenerateMipChainFlag(), b.getGenerateMipChainFlag());
    }

    // ArrayResource
    template <>
    inline IResource* ResourceSerializationTestHelper::CreateTestResource<ArrayResource>(UInt32 blobSize)
    {
        ArrayResource* resource = new ArrayResource(EResourceType_VertexArray, 3, EDataType_Vector3F, 0u, ResourceCacheFlag(15u), "resName");
        SetResourceDataRandom(*resource, blobSize);
        return resource;
    }

    template <>
    inline void ResourceSerializationTestHelper::CompareTypedResources(const ArrayResource& a, const ArrayResource& b)
    {
        EXPECT_EQ(a.getElementType(), b.getElementType());
        EXPECT_EQ(a.getElementCount(), b.getElementCount());
    }

    // EffectResource
    template <>
    inline IResource* ResourceSerializationTestHelper::CreateTestResource<EffectResource>(UInt32 blobSize)
    {
        String vert;
        String frag;
        vert.resize(blobSize / 2);
        frag.resize(blobSize / 2);
        for (UInt32 i = 0; i < blobSize / 2; ++i)
        {
            vert[i] = 'a';
            frag[i] = 'b';
        }
        EffectResource* resource = new EffectResource(vert, frag, EffectInputInformationVector(), EffectInputInformationVector(), "effect name", ResourceCacheFlag(1u));
        return resource;
    }

    template <>
    inline void ResourceSerializationTestHelper::CompareTypedResources(const EffectResource& a, const EffectResource& b)
    {
        EXPECT_STREQ(a.getVertexShader(), b.getVertexShader());
        EXPECT_STREQ(a.getFragmentShader(), b.getFragmentShader());
    }
}

#endif
