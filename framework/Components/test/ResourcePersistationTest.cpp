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
#include "Utils/BinaryFileOutputStream.h"
#include "Utils/BinaryFileInputStream.h"
#include "ResourceMock.h"
#include "Components/ResourceTableOfContents.h"

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
            IResource* loaded = 0;

            {
                File file("filename");
                BinaryFileOutputStream stream(file);
                ResourcePersistation::WriteOneResourceToStream(stream, inResource);
            }

            {
                File file("filename");
                BinaryFileInputStream stream(file);
                loaded = ResourcePersistation::ReadOneResourceFromStream(stream, inResource.getResourceObject()->getHash());
            }

            EXPECT_TRUE(0 != loaded);

            return loaded;
        }

        void checkRawResourceData(const IResource& createdResource, const IResource* loadedResource)
        {
            const SceneResourceData& referenceResourceData = createdResource.getResourceData();
            void* referenceData = referenceResourceData->getRawData();
            void* loadedData = loadedResource->getResourceData()->getRawData();

            EXPECT_EQ(referenceResourceData->size(), loadedResource->getResourceData()->size());
            EXPECT_EQ(0, PlatformMemory::Compare(referenceData, loadedData, referenceResourceData->size()));
        }

        template <typename ResourceType>
        const ResourceType* createLoadedResource(const IResource& res, const EResourceType resourceType)
        {
            ManagedResource managedRes(res, m_deleterMock);
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
        const TextureMetaInfo texDesc(2u, 3u, 1u, ETextureFormat_RGB8, false, { 1u, 2u });
        const ResourceCacheFlag flag(15u);
        TextureResource res(EResourceType_Texture3D, texDesc, flag, "resName");
        SceneResourceData pixels(new MemoryBlob(std::accumulate(texDesc.m_dataSizes.cbegin(), texDesc.m_dataSizes.cend(), 0u)));
        for (UInt8 i = 0; i < pixels->size(); ++i)
        {
            (*pixels)[i] = i;
        }
        ASSERT_EQ(pixels->size(), res.getResourceData()->size());
        res.setResourceData(pixels);

        const TextureResource* loadedTextureResource = createLoadedResource<TextureResource>(res, EResourceType_Texture3D);

        ASSERT_EQ(res.getWidth(), loadedTextureResource->getWidth());
        ASSERT_EQ(res.getHeight(), loadedTextureResource->getHeight());
        ASSERT_EQ(res.getDepth(), loadedTextureResource->getDepth());
        ASSERT_EQ(ETextureFormat_RGB8, loadedTextureResource->getTextureFormat());
        ASSERT_EQ(res.getGenerateMipChainFlag(), loadedTextureResource->getGenerateMipChainFlag());
        ASSERT_EQ(texDesc.m_dataSizes, loadedTextureResource->getMipDataSizes());
        ASSERT_EQ(flag, loadedTextureResource->getCacheFlag());
        EXPECT_EQ(String("resName"), loadedTextureResource->getName());

        delete loadedTextureResource;
    }

    TEST_F(AResourcePersistation, WriteRead_TextureResourceCube)
    {
        const TextureMetaInfo texDesc(2u, 1u, 1u, ETextureFormat_RGB8, false, { 1u, 2u });
        const ResourceCacheFlag flag(15u);
        TextureResource res(EResourceType_TextureCube, texDesc, flag, "resName");
        SceneResourceData pixels(new MemoryBlob(6u * std::accumulate(texDesc.m_dataSizes.cbegin(), texDesc.m_dataSizes.cend(), 0u)));
        for (UInt8 i = 0; i < pixels->size(); ++i)
        {
            (*pixels)[i] = i;
        }
        ASSERT_EQ(pixels->size(), res.getResourceData()->size());
        res.setResourceData(pixels);

        const TextureResource* loadedTextureResource = createLoadedResource<TextureResource>(res, EResourceType_TextureCube);

        ASSERT_EQ(res.getWidth(), loadedTextureResource->getWidth());
        ASSERT_EQ(ETextureFormat_RGB8, loadedTextureResource->getTextureFormat());
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

        ArrayResource res(EResourceType_VertexArray, cnt, EDataType_Vector2F, NULL, flag, "resName");
        SceneResourceData vertices(new MemoryBlob(cnt * EnumToSize(EDataType_Vector2F)));
        for (UInt i = 0; i < 2*cnt; ++i)
        {
            reinterpret_cast<Float*>(vertices->getRawData())[i] = i*.1f;
        }
        res.setResourceData(vertices);

        const ArrayResource* loadedVertexArrayResource = createLoadedResource<ArrayResource>(res, EResourceType_VertexArray);

        ASSERT_EQ(res.getElementCount(), loadedVertexArrayResource->getElementCount());
        ASSERT_EQ(EDataType_Vector2F, loadedVertexArrayResource->getElementType());
        ASSERT_EQ(flag, loadedVertexArrayResource->getCacheFlag());
        EXPECT_EQ(String("resName"), loadedVertexArrayResource->getName());

        delete loadedVertexArrayResource;
    }

    TEST_F(AResourcePersistation, WriteRead_Index16ArrayResource)
    {
        const UInt32 cnt = 220;
        const ResourceCacheFlag flag(15u);

        ArrayResource res(EResourceType_IndexArray, cnt, EDataType_UInt16, NULL, flag, "resName");
        SceneResourceData indices(new MemoryBlob(cnt * EnumToSize(EDataType_UInt16)));
        for (UInt i = 0; i < cnt; ++i)
        {
            reinterpret_cast<UInt16*>(indices->getRawData())[i] = static_cast<UInt16>(i);
        }
        res.setResourceData(indices);

        const ArrayResource* loadedIndexArrayResource = createLoadedResource<ArrayResource>(res, EResourceType_IndexArray);

        ASSERT_EQ(res.getElementCount(), loadedIndexArrayResource->getElementCount());
        ASSERT_EQ(EDataType_UInt16, loadedIndexArrayResource->getElementType());
        ASSERT_EQ(flag, loadedIndexArrayResource->getCacheFlag());
        EXPECT_EQ(String("resName"), loadedIndexArrayResource->getName());

        delete loadedIndexArrayResource;
    }

    TEST_F(AResourcePersistation, WriteRead_Index32ArrayResource)
    {
        const UInt32 cnt = 220;
        const ResourceCacheFlag flag(15u);

        ArrayResource res(EResourceType_IndexArray, cnt, EDataType_UInt32, NULL, flag, "resName");
        SceneResourceData indices(new MemoryBlob(cnt * EnumToSize(EDataType_UInt32)));
        for (UInt i = 0; i < cnt; ++i)
        {
            reinterpret_cast<UInt32*>(indices->getRawData())[i] = static_cast<UInt32>(i);
        }
        res.setResourceData(indices);

        const ArrayResource* loadedIndexArrayResource = createLoadedResource<ArrayResource>(res, EResourceType_IndexArray);

        ASSERT_EQ(res.getElementCount(), loadedIndexArrayResource->getElementCount());
        ASSERT_EQ(EDataType_UInt32, loadedIndexArrayResource->getElementType());
        ASSERT_EQ(flag, loadedIndexArrayResource->getCacheFlag());
        EXPECT_EQ(String("resName"), loadedIndexArrayResource->getName());

        delete loadedIndexArrayResource;
    }

    TEST_F(AResourcePersistation, WriteRead_EffectResource)
    {
        const ResourceCacheFlag flag(15u);
        EffectResource effectResource("vertexBla", "fragmentFoo", EffectInputInformationVector(), EffectInputInformationVector(), "effect name", flag);

        const EffectResource* loadedEffectResource = createLoadedResource<EffectResource>(effectResource, EResourceType_Effect);

        EXPECT_STREQ(effectResource.getVertexShader(), loadedEffectResource->getVertexShader());
        EXPECT_STREQ(effectResource.getFragmentShader(), loadedEffectResource->getFragmentShader());
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
        ArrayResource res(EResourceType_VertexArray, 3, EDataType_Vector3F, reinterpret_cast<const Byte*>(dataA), flag1, "res1");
        ManagedResource managedRes(res, dummyManagedResourceCallback);
        const ResourceContentHash hash = managedRes.getResourceObject()->getHash();

        float dataB[18];
        for (uint32_t i = 0u; i < 18; ++i)
        {
            dataB[i] = static_cast<Float>(i);
        }
        ArrayResource res2(EResourceType_VertexArray, 6, EDataType_Vector3F, reinterpret_cast<const Byte*>(dataB), flag2, "res2");
        ManagedResource managedRes2(res2, dummyManagedResourceCallback);
        const ResourceContentHash hash2 = managedRes2.getResourceObject()->getHash();

        EffectResource res3("foo", "bar", EffectInputInformationVector(), EffectInputInformationVector(), "Some effect with a name", flag3);
        ManagedResource managedRes3(res3, dummyManagedResourceCallback);
        const ResourceContentHash hash3 = managedRes3.getResourceObject()->getHash();

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
        ASSERT_EQ(0, PlatformMemory::Compare(dataA, loadedResource->getResourceData()->getRawData(), 36));
        ASSERT_EQ(flag1, loadedResource->getCacheFlag());
        EXPECT_EQ(String("res1"), loadedResource->getName());
        delete loadedResource;

        ASSERT_TRUE(loadedTOC.containsResource(hash2));
        loadedResource = ResourcePersistation::RetrieveResourceFromStream(instream, loadedTOC.getEntryForHash(hash2));
        ASSERT_EQ(0, PlatformMemory::Compare(dataB, loadedResource->getResourceData()->getRawData(), 72));
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
