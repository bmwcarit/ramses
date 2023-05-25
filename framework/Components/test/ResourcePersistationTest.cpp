//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/BinaryInputStream.h"
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
#include "Utils/BinaryOutputStream.h"
#include "ResourceMock.h"
#include "InputStreamMock.h"
#include "UnsafeTestMemoryHelpers.h"
#include <cstring>

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

        std::unique_ptr<IResource> readWriteResource(const ManagedResource& inResource)
        {
            std::unique_ptr<IResource> loaded;

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

            EXPECT_TRUE(loaded);

            return loaded;
        }

        template <typename ResourceType>
        std::unique_ptr<const ResourceType> createLoadedResource(const IResource& res, const EResourceType resourceType)
        {
            ManagedResource managedRes{ &res, m_deleterMock };
            std::unique_ptr<IResource> loadedResource = readWriteResource(managedRes);
            EXPECT_EQ(res.getResourceData().span(), loadedResource->getResourceData().span());

            EXPECT_EQ(resourceType, loadedResource->getTypeID());
            EXPECT_EQ(res.getCacheFlag(), loadedResource->getCacheFlag());

            return std::unique_ptr<ResourceType>(static_cast<ResourceType*>(loadedResource.release()));
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

        auto loadedTextureResource = createLoadedResource<TextureResource>(res, EResourceType_Texture3D);

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
        EXPECT_EQ(std::string("resName"), loadedTextureResource->getName());
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

        auto loadedTextureResource = createLoadedResource<TextureResource>(res, EResourceType_TextureCube);

        ASSERT_EQ(res.getWidth(), loadedTextureResource->getWidth());
        ASSERT_EQ(ETextureFormat::RGB8, loadedTextureResource->getTextureFormat());
        ASSERT_EQ(res.getGenerateMipChainFlag(), loadedTextureResource->getGenerateMipChainFlag());
        ASSERT_EQ(texDesc.m_dataSizes, loadedTextureResource->getMipDataSizes());
        ASSERT_EQ(flag, loadedTextureResource->getCacheFlag());
        EXPECT_EQ(std::string("resName"), loadedTextureResource->getName());
    }

    TEST_F(AResourcePersistation, WriteRead_VertexArrayResource)
    {
        const UInt32 cnt = 200;
        const ResourceCacheFlag flag(15u);

        ArrayResource res(EResourceType_VertexArray, cnt, EDataType::Vector2F, nullptr, flag, "resName");
        ResourceBlob vertices(cnt * EnumToSize(EDataType::Vector2F));
        for (UInt i = 0; i < 2*cnt; ++i)
        {
            UnsafeTestMemoryHelpers::WriteToMemoryBlob(i*.1f, vertices.data(), i);
        }
        res.setResourceData(std::move(vertices));

        auto loadedVertexArrayResource = createLoadedResource<ArrayResource>(res, EResourceType_VertexArray);

        ASSERT_EQ(res.getElementCount(), loadedVertexArrayResource->getElementCount());
        ASSERT_EQ(EDataType::Vector2F, loadedVertexArrayResource->getElementType());
        ASSERT_EQ(flag, loadedVertexArrayResource->getCacheFlag());
        EXPECT_EQ(std::string("resName"), loadedVertexArrayResource->getName());
    }

    TEST_F(AResourcePersistation, WriteRead_Index16ArrayResource)
    {
        const UInt32 cnt = 220;
        const ResourceCacheFlag flag(15u);

        ArrayResource res(EResourceType_IndexArray, cnt, EDataType::UInt16, nullptr, flag, "resName");
        ResourceBlob indices(cnt * EnumToSize(EDataType::UInt16));
        for (UInt i = 0; i < cnt; ++i)
        {
            UnsafeTestMemoryHelpers::WriteToMemoryBlob(static_cast<UInt16>(i), indices.data(), i);
        }
        res.setResourceData(std::move(indices));

        auto loadedIndexArrayResource = createLoadedResource<ArrayResource>(res, EResourceType_IndexArray);

        ASSERT_EQ(res.getElementCount(), loadedIndexArrayResource->getElementCount());
        ASSERT_EQ(EDataType::UInt16, loadedIndexArrayResource->getElementType());
        ASSERT_EQ(flag, loadedIndexArrayResource->getCacheFlag());
        EXPECT_EQ(std::string("resName"), loadedIndexArrayResource->getName());
    }

    TEST_F(AResourcePersistation, WriteRead_Index32ArrayResource)
    {
        const UInt32 cnt = 220;
        const ResourceCacheFlag flag(15u);

        ArrayResource res(EResourceType_IndexArray, cnt, EDataType::UInt32, nullptr, flag, "resName");
        ResourceBlob indices(cnt * EnumToSize(EDataType::UInt32));
        for (UInt i = 0; i < cnt; ++i)
        {
            UnsafeTestMemoryHelpers::WriteToMemoryBlob(static_cast<UInt32>(i), indices.data(), i);
        }
        res.setResourceData(std::move(indices));

        auto loadedIndexArrayResource = createLoadedResource<ArrayResource>(res, EResourceType_IndexArray);

        ASSERT_EQ(res.getElementCount(), loadedIndexArrayResource->getElementCount());
        ASSERT_EQ(EDataType::UInt32, loadedIndexArrayResource->getElementType());
        ASSERT_EQ(flag, loadedIndexArrayResource->getCacheFlag());
        EXPECT_EQ(std::string("resName"), loadedIndexArrayResource->getName());
    }

    TEST_F(AResourcePersistation, WriteRead_EffectResource)
    {
        const ResourceCacheFlag flag(15u);
        EffectResource effectResource("vertexBla", "fragmentFoo", "geometryFoo", EDrawMode::Lines, EffectInputInformationVector(), EffectInputInformationVector(), "effect name", flag);

        auto loadedEffectResource = createLoadedResource<EffectResource>(effectResource, EResourceType_Effect);

        EXPECT_STREQ(effectResource.getVertexShader(), loadedEffectResource->getVertexShader());
        EXPECT_STREQ(effectResource.getFragmentShader(), loadedEffectResource->getFragmentShader());
        EXPECT_STREQ(effectResource.getGeometryShader(), loadedEffectResource->getGeometryShader());
        ASSERT_EQ(flag, loadedEffectResource->getCacheFlag());
        EXPECT_EQ(std::string("effect name"), loadedEffectResource->getName());
        EXPECT_EQ(EDrawMode::Lines, loadedEffectResource->getGeometryShaderInputType());
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
            dataA[i] = static_cast<float>(i);
        }
        ArrayResource res(EResourceType_VertexArray, 3, EDataType::Vector3F, dataA, flag1, "res1");
        ManagedResource managedRes{ &res, dummyManagedResourceCallback };
        const ResourceContentHash hash = managedRes->getHash();

        float dataB[18];
        for (uint32_t i = 0u; i < 18; ++i)
        {
            dataB[i] = static_cast<float>(i);
        }
        ArrayResource res2(EResourceType_VertexArray, 6, EDataType::Vector3F, dataB, flag2, "res2");
        ManagedResource managedRes2{ &res2, dummyManagedResourceCallback };
        const ResourceContentHash hash2 = managedRes2->getHash();

        EffectResource res3("foo", "bar", "qux", EDrawMode::Lines, EffectInputInformationVector(), EffectInputInformationVector(), "Some effect with a name", flag3);
        ManagedResource managedRes3{ &res3, dummyManagedResourceCallback };
        const ResourceContentHash hash3 = managedRes3->getHash();

        ManagedResourceVector resources;
        resources.push_back(managedRes);
        resources.push_back(managedRes2);
        resources.push_back(managedRes3);

        File tempFile("onDemandResourceFile");
        BinaryFileOutputStream out(tempFile);
        ResourcePersistation::WriteNamedResourcesWithTOCToStream(out, resources, false);
        tempFile.close();

        ResourceTableOfContents loadedTOC;
        BinaryFileInputStream instream(tempFile);
        loadedTOC.readTOCPosAndTOCFromStream(instream);

        {
            ASSERT_TRUE(loadedTOC.containsResource(hash));
            auto loadedResource = ResourcePersistation::RetrieveResourceFromStream(instream, loadedTOC.getEntryForHash(hash));
            ASSERT_TRUE(UnsafeTestMemoryHelpers::CompareMemoryBlobToSpan(dataA, sizeof(dataA), loadedResource->getResourceData().span()));
            ASSERT_EQ(flag1, loadedResource->getCacheFlag());
            EXPECT_EQ(std::string("res1"), loadedResource->getName());
        }

        {
            ASSERT_TRUE(loadedTOC.containsResource(hash2));
            auto loadedResource = ResourcePersistation::RetrieveResourceFromStream(instream, loadedTOC.getEntryForHash(hash2));
            ASSERT_TRUE(UnsafeTestMemoryHelpers::CompareMemoryBlobToSpan(dataB, sizeof(dataB), loadedResource->getResourceData().span()));
            ASSERT_EQ(flag2, loadedResource->getCacheFlag());
            EXPECT_EQ(std::string("res2"), loadedResource->getName());
        }

        {
            ASSERT_TRUE(loadedTOC.containsResource(hash3));
            auto loadedResource = ResourcePersistation::RetrieveResourceFromStream(instream, loadedTOC.getEntryForHash(hash3));
            EXPECT_STREQ(res3.getVertexShader(), loadedResource->convertTo<EffectResource>()->getVertexShader());
            EXPECT_STREQ(res3.getFragmentShader(), loadedResource->convertTo<EffectResource>()->getFragmentShader());
            ASSERT_EQ(flag3, loadedResource->getCacheFlag());
            EXPECT_EQ(std::string("Some effect with a name"), loadedResource->getName());
        }
    }

    static std::pair<std::vector<Byte>, ResourceFileEntry> getDummyResourceData()
    {
        BinaryOutputStream outStream;
        EffectResource res("foo", "bar", "qux", EDrawMode::Lines, EffectInputInformationVector(), EffectInputInformationVector(), "Some effect with a name", ResourceCacheFlag(0));
        NiceMock<ManagedResourceDeleterCallbackMock> managedResourceDeleter;
        ResourceDeleterCallingCallback dummyManagedResourceCallback(managedResourceDeleter);
        ManagedResource managedRes{ &res, dummyManagedResourceCallback };
        ResourcePersistation::WriteOneResourceToStream(outStream, managedRes);
        auto data = outStream.release();

        const uint32_t dataSize = static_cast<uint32_t>(data.size());
        ResourceFileEntry entry{0, dataSize,
                                ResourceInfo(EResourceType_Effect, res.getHash(), dataSize, 0)};
        return {data, entry};
    }

    TEST(ResourcePersistation, retrieveResourceFromStreamCanLoadDummyData)
    {
        const auto dummyResource = getDummyResourceData();
        InputStreamMock stream;
        BinaryInputStream resStream(dummyResource.first.data());
        EXPECT_CALL(stream, seek(_, _)).WillRepeatedly([&](Int offset, auto origin) {
            return resStream.seek(offset, origin);
        });
        EXPECT_CALL(stream, getState()).WillRepeatedly([&]() {
            return resStream.getState();
        });
        EXPECT_CALL(stream, getPos(_)).WillRepeatedly([&](size_t& pos) {
            return resStream.getPos(pos);
        });
        EXPECT_CALL(stream, read(_, _)).WillRepeatedly([&](void* data, size_t size) -> IInputStream& {
            resStream.read(data, size);
            return stream;
        });
        EXPECT_TRUE(ResourcePersistation::RetrieveResourceFromStream(stream, dummyResource.second));
    }

    TEST(ResourcePersistation, retrieveResourceFromStreamCanHandleSeekErrors)
    {
        const auto dummyResource = getDummyResourceData();
        InputStreamMock stream;
        BinaryInputStream resStream(dummyResource.first.data());
        EXPECT_CALL(stream, seek(_, _)).WillRepeatedly(Return(EStatus::Error));
        EXPECT_FALSE(ResourcePersistation::RetrieveResourceFromStream(stream, dummyResource.second));
    }

    TEST(ResourcePersistation, retrieveResourceFromStreamCanHandleGetPosErrors)
    {
        const auto dummyResource = getDummyResourceData();
        InputStreamMock stream;
        BinaryInputStream resStream(dummyResource.first.data());
        EXPECT_CALL(stream, seek(_, _)).WillRepeatedly([&](Int offset, auto origin) {
            return resStream.seek(offset, origin);
        });
        EXPECT_CALL(stream, getState()).WillRepeatedly([&]() {
            return resStream.getState();
        });
        EXPECT_CALL(stream, getPos(_)).WillRepeatedly(Return(EStatus::Error));
        EXPECT_CALL(stream, read(_, _)).WillRepeatedly([&](void* data, size_t size) -> IInputStream& {
            resStream.read(data, size);
            return stream;
        });
        EXPECT_FALSE(ResourcePersistation::RetrieveResourceFromStream(stream, dummyResource.second));
    }

    TEST(ResourcePersistation, retrieveResourceFromStreamCanHandleGetPosWrongData)
    {
        const auto dummyResource = getDummyResourceData();
        InputStreamMock stream;
        BinaryInputStream resStream(dummyResource.first.data());
        EXPECT_CALL(stream, seek(_, _)).WillRepeatedly([&](Int offset, auto origin) {
            return resStream.seek(offset, origin);
        });
        EXPECT_CALL(stream, getState()).WillRepeatedly([&]() {
            return resStream.getState();
        });
        EXPECT_CALL(stream, getPos(_)).WillRepeatedly([&](size_t& pos) {
            const auto res = resStream.getPos(pos);
            ++pos;
            return res;
        });
        EXPECT_CALL(stream, read(_, _)).WillRepeatedly([&](void* data, size_t size) -> IInputStream& {
            resStream.read(data, size);
            return stream;
        });
        EXPECT_FALSE(ResourcePersistation::RetrieveResourceFromStream(stream, dummyResource.second));
    }

    TEST(ResourcePersistation, retrieveResourceFromStreamCanHandleGetStateErrors)
    {
        const auto dummyResource = getDummyResourceData();
        InputStreamMock stream;
        BinaryInputStream resStream(dummyResource.first.data());
        EXPECT_CALL(stream, seek(_, _)).WillRepeatedly([&](Int offset, auto origin) {
            return resStream.seek(offset, origin);
        });
        EXPECT_CALL(stream, getState()).WillRepeatedly(Return(EStatus::Error));
        EXPECT_CALL(stream, read(_, _)).WillRepeatedly([&](void* data, size_t size) -> IInputStream& {
            resStream.read(data, size);
            return stream;
        });
        EXPECT_FALSE(ResourcePersistation::RetrieveResourceFromStream(stream, dummyResource.second));
    }

    TEST(ResourcePersistation, retrieveResourceFromStreamCanHandleZeroReader)
    {
        const auto dummyResource = getDummyResourceData();
        InputStreamMock stream;
        BinaryInputStream resStream(dummyResource.first.data());
        EXPECT_CALL(stream, seek(_, _)).WillRepeatedly([&](Int offset, auto origin) {
            return resStream.seek(offset, origin);
        });
        EXPECT_CALL(stream, read(_, _)).WillRepeatedly([&](void* data, size_t size) -> IInputStream& {
            resStream.read(data, size);
            std::memset(data, 0, size);
            return stream;
        });
        EXPECT_FALSE(ResourcePersistation::RetrieveResourceFromStream(stream, dummyResource.second));
    }
}
