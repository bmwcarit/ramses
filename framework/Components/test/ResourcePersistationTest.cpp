//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "Components/ResourcePersistation.h"
#include "Resource/TextureResource.h"
#include "Resource/ArrayResource.h"
#include "Resource/EffectResource.h"
#include "Components/ManagedResource.h"
#include "Components/IManagedResourceDeleterCallback.h"
#include "Components/ResourceTableOfContents.h"
#include "Components/ResourceDeleterCallingCallback.h"
#include "Utils/BinaryFileOutputStream.h"
#include "Utils/BinaryFileInputStream.h"
#include "ResourceMock.h"

using namespace testing;

namespace ramses_internal
{
    class AResourcePersistation : public ::testing::Test
    {
    protected:
        AResourcePersistation()
            : m_deleterMock(m_mock)
        {
        }

        IResource* readWriteResource(const ManagedResource& inResource)
        {
            IResource* loaded = nullptr;

            {
                File file("filename");
                BinaryFileOutputStream stream(file);
                ResourcePersistation::WriteOneResourceToStream(stream, inResource);
            }

            {
                File file("filename");
                BinaryFileInputStream stream(file);
                loaded = ResourcePersistation::ReadOneResourceFromStream(stream, inResource->getHash());
            }

            EXPECT_TRUE(nullptr != loaded);

            return loaded;
        }

        void checkRawResourceData(const IResource& createdResource, const IResource* loadedResource)
        {
            const ResourceBlob& referenceResourceData = createdResource.getResourceData();
            const Byte* referenceData = referenceResourceData.data();
            const Byte* loadedData = loadedResource->getResourceData().data();

            ASSERT_EQ(referenceResourceData.size(), loadedResource->getResourceData().size());
            EXPECT_EQ(0, PlatformMemory::Compare(referenceData, loadedData, referenceResourceData.size()));
        }

        template <typename ResourceType>
        const ResourceType* createLoadedResource(const IResource& res, const EResourceType resourceType)
        {
            ManagedResource managedRes{ &res, m_deleterMock };
            IResource* loadedResource = readWriteResource(managedRes);
            checkRawResourceData(res, loadedResource);

            EXPECT_EQ(resourceType, loadedResource->getTypeID());
            EXPECT_EQ(res.getCacheFlag(), loadedResource->getCacheFlag());

            return loadedResource->convertTo<ResourceType>();
        }

    private:
        NiceMock<ManagedResourceDeleterCallbackMock> m_mock;

    protected:
        ResourceDeleterCallingCallback m_deleterMock;
    };

    TEST_F(AResourcePersistation, WriteRead_TextureResource)
    {
        const TextureSwizzleArray swizzle{ETextureChannelColor::Blue, ETextureChannelColor::Red, ETextureChannelColor::Alpha, ETextureChannelColor::Green};
        const TextureMetaInfo texDesc(2u, 3u, 1u, ETextureFormat::RGB8, false, swizzle, { 1u, 2u });
        const ResourceCacheFlag flag(15u);
        TextureResource res(EResourceType_Texture3D, texDesc, flag, "resName");
        ResourceBlob pixels(std::accumulate(texDesc.m_dataSizes.cbegin(), texDesc.m_dataSizes.cend(), 0u));
        for (size_t i = 0; i < pixels.size(); ++i)
        {
            pixels.data()[i] = static_cast<uint8_t>(i);
        }
        ASSERT_EQ(pixels.size(), res.getResourceData().size());
        res.setResourceData(std::move(pixels));

        const TextureResource* loadedTextureResource = createLoadedResource<TextureResource>(res, EResourceType_Texture3D);

        ASSERT_EQ(res.getWidth(), loadedTextureResource->getWidth());
        ASSERT_EQ(res.getHeight(), loadedTextureResource->getHeight());
        ASSERT_EQ(res.getDepth(), loadedTextureResource->getDepth());
        ASSERT_EQ(ETextureFormat::RGB8, loadedTextureResource->getTextureFormat());
        ASSERT_EQ(ETextureChannelColor::Blue, loadedTextureResource->getTextureSwizzle()[0]);
        ASSERT_EQ(ETextureChannelColor::Red, loadedTextureResource->getTextureSwizzle()[1]);
        ASSERT_EQ(ETextureChannelColor::Alpha, loadedTextureResource->getTextureSwizzle()[2]);
        ASSERT_EQ(ETextureChannelColor::Green, loadedTextureResource->getTextureSwizzle()[3]);
        ASSERT_EQ(res.getGenerateMipChainFlag(), loadedTextureResource->getGenerateMipChainFlag());
        ASSERT_EQ(texDesc.m_dataSizes, loadedTextureResource->getMipDataSizes());
        ASSERT_EQ(flag, loadedTextureResource->getCacheFlag());
        EXPECT_EQ(String("resName"), loadedTextureResource->getName());

        delete loadedTextureResource;
    }

    TEST_F(AResourcePersistation, WriteRead_TextureResourceCube)
    {
        const TextureMetaInfo texDesc(2u, 1u, 1u, ETextureFormat::RGB8, false, {}, { 1u, 2u });
        const ResourceCacheFlag flag(15u);
        TextureResource res(EResourceType_TextureCube, texDesc, flag, "resName");
        ResourceBlob pixels(6u * std::accumulate(texDesc.m_dataSizes.cbegin(), texDesc.m_dataSizes.cend(), 0u));
        for (size_t i = 0; i < pixels.size(); ++i)
        {
            pixels.data()[i] = static_cast<uint8_t>(i);
        }
        ASSERT_EQ(pixels.size(), res.getResourceData().size());
        res.setResourceData(std::move(pixels));

        const TextureResource* loadedTextureResource = createLoadedResource<TextureResource>(res, EResourceType_TextureCube);

        ASSERT_EQ(res.getWidth(), loadedTextureResource->getWidth());
        ASSERT_EQ(ETextureFormat::RGB8, loadedTextureResource->getTextureFormat());
        ASSERT_EQ(res.getGenerateMipChainFlag(), loadedTextureResource->getGenerateMipChainFlag());
        ASSERT_EQ(texDesc.m_dataSizes, loadedTextureResource->getMipDataSizes());
        ASSERT_EQ(flag, loadedTextureResource->getCacheFlag());
        EXPECT_EQ(String("resName"), loadedTextureResource->getName());

        delete loadedTextureResource;
    }

    TEST_F(AResourcePersistation, WriteRead_VertexArrayResource)
    {
        const UInt32 cnt = 200;
        const ResourceCacheFlag flag(15u);

        ArrayResource res(EResourceType_VertexArray, cnt, EDataType::Vector2F, nullptr, flag, "resName");
        ResourceBlob vertices(cnt * EnumToSize(EDataType::Vector2F));
        for (UInt i = 0; i < 2*cnt; ++i)
        {
            reinterpret_cast<Float*>(vertices.data())[i] = i*.1f;
        }
        res.setResourceData(std::move(vertices));

        const ArrayResource* loadedVertexArrayResource = createLoadedResource<ArrayResource>(res, EResourceType_VertexArray);

        ASSERT_EQ(res.getElementCount(), loadedVertexArrayResource->getElementCount());
        ASSERT_EQ(EDataType::Vector2F, loadedVertexArrayResource->getElementType());
        ASSERT_EQ(flag, loadedVertexArrayResource->getCacheFlag());
        EXPECT_EQ(String("resName"), loadedVertexArrayResource->getName());

        delete loadedVertexArrayResource;
    }

    TEST_F(AResourcePersistation, WriteRead_Index16ArrayResource)
    {
        const UInt32 cnt = 220;
        const ResourceCacheFlag flag(15u);

        ArrayResource res(EResourceType_IndexArray, cnt, EDataType::UInt16, nullptr, flag, "resName");
        ResourceBlob indices(cnt * EnumToSize(EDataType::UInt16));
        for (UInt i = 0; i < cnt; ++i)
        {
            reinterpret_cast<UInt16*>(indices.data())[i] = static_cast<UInt16>(i);
        }
        res.setResourceData(std::move(indices));

        const ArrayResource* loadedIndexArrayResource = createLoadedResource<ArrayResource>(res, EResourceType_IndexArray);

        ASSERT_EQ(res.getElementCount(), loadedIndexArrayResource->getElementCount());
        ASSERT_EQ(EDataType::UInt16, loadedIndexArrayResource->getElementType());
        ASSERT_EQ(flag, loadedIndexArrayResource->getCacheFlag());
        EXPECT_EQ(String("resName"), loadedIndexArrayResource->getName());

        delete loadedIndexArrayResource;
    }

    TEST_F(AResourcePersistation, WriteRead_Index32ArrayResource)
    {
        const UInt32 cnt = 220;
        const ResourceCacheFlag flag(15u);

        ArrayResource res(EResourceType_IndexArray, cnt, EDataType::UInt32, nullptr, flag, "resName");
        ResourceBlob indices(cnt * EnumToSize(EDataType::UInt32));
        for (UInt i = 0; i < cnt; ++i)
        {
            reinterpret_cast<UInt32*>(indices.data())[i] = static_cast<UInt32>(i);
        }
        res.setResourceData(std::move(indices));

        const ArrayResource* loadedIndexArrayResource = createLoadedResource<ArrayResource>(res, EResourceType_IndexArray);

        ASSERT_EQ(res.getElementCount(), loadedIndexArrayResource->getElementCount());
        ASSERT_EQ(EDataType::UInt32, loadedIndexArrayResource->getElementType());
        ASSERT_EQ(flag, loadedIndexArrayResource->getCacheFlag());
        EXPECT_EQ(String("resName"), loadedIndexArrayResource->getName());

        delete loadedIndexArrayResource;
    }

    TEST_F(AResourcePersistation, WriteRead_EffectResource)
    {
        const ResourceCacheFlag flag(15u);
        EffectResource effectResource("vertexBla", "fragmentFoo", "geometryFoo", EffectInputInformationVector(), EffectInputInformationVector(), "effect name", flag);

        const EffectResource* loadedEffectResource = createLoadedResource<EffectResource>(effectResource, EResourceType_Effect);

        EXPECT_STREQ(effectResource.getVertexShader(), loadedEffectResource->getVertexShader());
        EXPECT_STREQ(effectResource.getFragmentShader(), loadedEffectResource->getFragmentShader());
        EXPECT_STREQ(effectResource.getGeometryShader(), loadedEffectResource->getGeometryShader());
        ASSERT_EQ(flag, loadedEffectResource->getCacheFlag());
        EXPECT_EQ(String("effect name"), loadedEffectResource->getName());

        delete loadedEffectResource;
    }

    TEST(ResourcePersistation, sandwich_writeThreeResources_ReadOneBackBasedTableOfContentsInformation)
    {
        NiceMock<ManagedResourceDeleterCallbackMock> managedResourceDeleter;
        ResourceDeleterCallingCallback dummyManagedResourceCallback(managedResourceDeleter);

        const ResourceCacheFlag flag1(15u);
        const ResourceCacheFlag flag2(16u);
        const ResourceCacheFlag flag3(17u);

        float dataA[9];
        for (uint32_t i = 0u; i < 9; ++i)
        {
            dataA[i] = static_cast<Float>(i);
        }
        ArrayResource res(EResourceType_VertexArray, 3, EDataType::Vector3F, dataA, flag1, "res1");
        ManagedResource managedRes{ &res, dummyManagedResourceCallback };
        const ResourceContentHash hash = managedRes->getHash();

        float dataB[18];
        for (uint32_t i = 0u; i < 18; ++i)
        {
            dataB[i] = static_cast<Float>(i);
        }
        ArrayResource res2(EResourceType_VertexArray, 6, EDataType::Vector3F, dataB, flag2, "res2");
        ManagedResource managedRes2{ &res2, dummyManagedResourceCallback };
        const ResourceContentHash hash2 = managedRes2->getHash();

        EffectResource res3("foo", "bar", "qux", EffectInputInformationVector(), EffectInputInformationVector(), "Some effect with a name", flag3);
        ManagedResource managedRes3{ &res3, dummyManagedResourceCallback };
        const ResourceContentHash hash3 = managedRes3->getHash();

        ManagedResourceVector resources;
        resources.push_back(managedRes);
        resources.push_back(managedRes2);
        resources.push_back(managedRes3);

        const String filename("onDemandResourceFile");
        File tempFile(filename);
        BinaryFileOutputStream out(tempFile);
        ResourcePersistation::WriteNamedResourcesWithTOCToStream(out, resources, false);
        tempFile.close();

        ResourceTableOfContents loadedTOC;
        BinaryFileInputStream instream(tempFile);
        loadedTOC.readTOCPosAndTOCFromStream(instream);

        ASSERT_TRUE(loadedTOC.containsResource(hash));
        IResource* loadedResource = ResourcePersistation::RetrieveResourceFromStream(instream, loadedTOC.getEntryForHash(hash));
        ASSERT_EQ(0, PlatformMemory::Compare(dataA, loadedResource->getResourceData().data(), 36));
        ASSERT_EQ(flag1, loadedResource->getCacheFlag());
        EXPECT_EQ(String("res1"), loadedResource->getName());
        delete loadedResource;

        ASSERT_TRUE(loadedTOC.containsResource(hash2));
        loadedResource = ResourcePersistation::RetrieveResourceFromStream(instream, loadedTOC.getEntryForHash(hash2));
        ASSERT_EQ(0, PlatformMemory::Compare(dataB, loadedResource->getResourceData().data(), 72));
        ASSERT_EQ(flag2, loadedResource->getCacheFlag());
        EXPECT_EQ(String("res2"), loadedResource->getName());
        delete loadedResource;

        ASSERT_TRUE(loadedTOC.containsResource(hash3));
        loadedResource = ResourcePersistation::RetrieveResourceFromStream(instream, loadedTOC.getEntryForHash(hash3));
        EXPECT_STREQ(res3.getVertexShader(), loadedResource->convertTo<EffectResource>()->getVertexShader());
        EXPECT_STREQ(res3.getFragmentShader(), loadedResource->convertTo<EffectResource>()->getFragmentShader());
        ASSERT_EQ(flag3, loadedResource->getCacheFlag());
        EXPECT_EQ(String("Some effect with a name"), loadedResource->getName());
        delete loadedResource;
    }
}
