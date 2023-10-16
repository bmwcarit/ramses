//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/Resource/TextureResource.h"
#include "internal/SceneGraph/Resource/ArrayResource.h"
#include "internal/SceneGraph/Resource/EffectResource.h"
#include "gtest/gtest.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"
#include "internal/Components/ManagedResource.h"
#include "TestRandom.h"

namespace ramses::internal
{
    class ResourceSerializationTestHelper
    {
    public:
        template <typename T>
        static IResource* CreateTestResource(uint32_t blobSize);

        template <typename T>
        static void CompareTypedResources(const T& a, const T& b);

        static void CompareResourceValues(const IResource& a, const IResource& b);

        static void SetResourceDataRandom(IResource& res, uint32_t blobSize);

        using Types = ::testing::Types<TextureResource, ArrayResource, EffectResource>;
    };

    inline void ResourceSerializationTestHelper::CompareResourceValues(const IResource& a, const IResource& b)
    {
        ASSERT_EQ(a.getTypeID(), b.getTypeID());
        EXPECT_EQ(a.getHash(), b.getHash());
        EXPECT_EQ(a.getName(), b.getName());

        ASSERT_EQ(a.isDeCompressedAvailable(), b.isDeCompressedAvailable());
        if (a.isDeCompressedAvailable())
        {
            ASSERT_TRUE(a.getResourceData().data());
        }
        if (b.isDeCompressedAvailable())
        {
            ASSERT_TRUE(b.getResourceData().data());
        }
        if (a.isDeCompressedAvailable())
        {
            ASSERT_EQ(a.getResourceData().size(), b.getResourceData().size());
            EXPECT_EQ(0, PlatformMemory::Compare(a.getResourceData().data(), b.getResourceData().data(), a.getResourceData().size()));
        }
    }

    inline void ResourceSerializationTestHelper::SetResourceDataRandom(IResource& res, uint32_t blobSize)
    {
        if (blobSize == 0u)
            return;
        const auto seed = static_cast<uint8_t>(TestRandom::Get(0, 256));
        ResourceBlob data(blobSize);
        for (size_t i = 0; i < data.size(); ++i)
        {
            data.data()[i] = static_cast<std::byte>(i + seed);
        }
        res.setResourceData(std::move(data));
    }

    // TextureResource
    template <>
    inline IResource* ResourceSerializationTestHelper::CreateTestResource<TextureResource>(uint32_t blobSize)
    {
        const TextureMetaInfo texDesc{ 16u, 17u, 1u, EPixelStorageFormat::RGBA16F, false, {}, { 18u, 19u} };
        auto* resource = new TextureResource(EResourceType::Texture3D, texDesc, "resName");
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
    inline IResource* ResourceSerializationTestHelper::CreateTestResource<ArrayResource>(uint32_t blobSize)
    {
        auto* resource = new ArrayResource(EResourceType::VertexArray, 0, EDataType::Vector3F, nullptr, "resName");
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
    inline IResource* ResourceSerializationTestHelper::CreateTestResource<EffectResource>(uint32_t blobSize)
    {
        std::string vert;
        std::string frag;
        std::string geom;
        vert.resize(blobSize / 3);
        frag.resize(blobSize / 3);
        geom.resize(blobSize / 3);
        for (uint32_t i = 0; i < blobSize / 3; ++i)
        {
            vert[i] = 'a';
            frag[i] = 'b';
            geom[i] = 'c';
        }
        auto* resource = new EffectResource(vert, frag, geom, EDrawMode::Points, EffectInputInformationVector(), EffectInputInformationVector(), "effect name");
        return resource;
    }

    template <>
    inline void ResourceSerializationTestHelper::CompareTypedResources(const EffectResource& a, const EffectResource& b)
    {
        EXPECT_STREQ(a.getVertexShader(), b.getVertexShader());
        EXPECT_STREQ(a.getFragmentShader(), b.getFragmentShader());
        EXPECT_STREQ(a.getGeometryShader(), b.getGeometryShader());
        EXPECT_EQ(a.getGeometryShaderInputType(), b.getGeometryShaderInputType());
    }
}
