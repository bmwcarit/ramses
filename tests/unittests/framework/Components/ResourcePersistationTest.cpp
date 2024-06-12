//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Core/Utils/BinaryInputStream.h"
#include "internal/Components/ResourcePersistation.h"
#include "internal/SceneGraph/Resource/TextureResource.h"
#include "internal/SceneGraph/Resource/ArrayResource.h"
#include "internal/SceneGraph/Resource/EffectResource.h"
#include "internal/Components/ManagedResource.h"
#include "internal/Components/IManagedResourceDeleterCallback.h"
#include "internal/Components/ResourceTableOfContents.h"
#include "internal/Components/ResourceDeleterCallingCallback.h"
#include "internal/Core/Utils/BinaryFileOutputStream.h"
#include "internal/Core/Utils/BinaryFileInputStream.h"
#include "internal/Core/Utils/BinaryOutputStream.h"
#include "ResourceMock.h"
#include "InputStreamMock.h"
#include "UnsafeTestMemoryHelpers.h"
#include "ResourceSerializationTestHelper.h"
#include <cstring>

using namespace testing;

namespace ramses::internal
{
    class AResourcePersistation : public ::testing::Test
    {
    protected:
        AResourcePersistation()
            : m_deleterMock(m_mock)
        {
        }

        static std::unique_ptr<IResource> ReadWriteResource(const ManagedResource& inResource, EFeatureLevel featureLevel = EFeatureLevel_Latest)
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
                loaded = ResourcePersistation::ReadOneResourceFromStream(stream, inResource->getHash(), featureLevel);
            }

            EXPECT_TRUE(loaded);

            return loaded;
        }

        template <typename ResourceType>
        std::unique_ptr<const ResourceType> createLoadedResource(const IResource& res, const EResourceType resourceType, EFeatureLevel featureLevel = EFeatureLevel_Latest)
        {
            ManagedResource managedRes{ &res, m_deleterMock };
            std::unique_ptr<IResource> loadedResource = ReadWriteResource(managedRes, featureLevel);
            EXPECT_EQ(res.getResourceData().span(), loadedResource->getResourceData().span());
            EXPECT_EQ(resourceType, loadedResource->getTypeID());

            // save and load resource second time - this can reveal discrepencies between deserialize and serialize code (in this order)
            ManagedResource managedRes2{ loadedResource.get(), m_deleterMock };
            std::unique_ptr<IResource> loadedResource2 = ReadWriteResource(managedRes2, featureLevel);
            EXPECT_EQ(res.getResourceData().span(), loadedResource2->getResourceData().span());
            EXPECT_EQ(resourceType, loadedResource2->getTypeID());

            return std::unique_ptr<ResourceType>(static_cast<ResourceType*>(loadedResource2.release()));
        }

        static EffectResource CreateTestEffect(EFeatureLevel featureLevel)
        {
            const std::string customVertexShader(R"SHADER(
                #version 310 es
                layout(location=11) uniform vec2 u_myVec2;
                layout(location=5) uniform sampler2D u_mySampler1;
                layout(std140,binding=1) uniform MyUbo_t
                {
                    vec2 u_myVec2;
                } myUbo;
                layout(location=23) uniform vec3 u_myVec3;
                layout(location=7) uniform sampler2D u_mySampler2;

                void main(void)
                {
                    gl_Position = texture(u_mySampler1, u_myVec2)+ texture(u_mySampler2, myUbo.u_myVec2) * u_myVec3.x;
                }
                )SHADER");
            const std::string customFragmentShader(R"SHADER(
                #version 310 es
                out highp vec4 fragColor;
                void main(void)
                {
                    fragColor = vec4(1.0);
                }
                )SHADER");

            EffectInputInformationVector uniformInputs;
            uniformInputs.push_back(EffectInputInformation("u_myVec2", 1, EDataType::Vector2F, EFixedSemantics::Invalid));
            uniformInputs.push_back(EffectInputInformation("u_mySampler1", 1, EDataType::TextureSampler2D, EFixedSemantics::Invalid));
            uniformInputs.push_back(EffectInputInformation("myUbo", 1, EDataType::UniformBuffer, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }));
            uniformInputs.push_back(EffectInputInformation("myUbo.u_myVec2", 1, EDataType::Vector2F, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 8u }, UniformBufferFieldOffset{ 0u }));
            uniformInputs.push_back(EffectInputInformation("u_myVec3", 1, EDataType::Vector3F, EFixedSemantics::Invalid));
            uniformInputs.push_back(EffectInputInformation("u_mySampler2", 1, EDataType::TextureSampler2D, EFixedSemantics::Invalid));

            SPIRVShaders dummySpirvShaders{
                SPIRVShaderBlob{ 1u, 2u, 3u, 4u },
                SPIRVShaderBlob{ 5u, 6u, 7u, 8u, 9u },
                SPIRVShaderBlob{ 10u, 11u, 12u} };

            return EffectResource(
                customVertexShader,
                customFragmentShader,
                "geometryFoo",
                featureLevel >= EFeatureLevel_02 ? dummySpirvShaders : SPIRVShaders{},
                EDrawMode::Lines,
                uniformInputs,
                {},
                "test effect", featureLevel);
        }

    private:
        NiceMock<ManagedResourceDeleterCallbackMock> m_mock;

    protected:
        ResourceDeleterCallingCallback m_deleterMock;
    };

    TEST_F(AResourcePersistation, WriteRead_TextureResource)
    {
        const TextureSwizzleArray swizzle{ETextureChannelColor::Blue, ETextureChannelColor::Red, ETextureChannelColor::Alpha, ETextureChannelColor::Green};
        const TextureMetaInfo texDesc(2u, 3u, 1u, EPixelStorageFormat::RGB8, false, swizzle, { 1u, 2u });
        TextureResource res(EResourceType::Texture3D, texDesc, "resName");
        ResourceBlob pixels(std::accumulate(texDesc.m_dataSizes.cbegin(), texDesc.m_dataSizes.cend(), 0u));
        for (size_t i = 0; i < pixels.size(); ++i)
        {
            pixels.data()[i] = std::byte{static_cast<uint8_t>(i)};
        }
        ASSERT_EQ(pixels.size(), res.getResourceData().size());
        res.setResourceData(std::move(pixels));

        auto loadedTextureResource = createLoadedResource<TextureResource>(res, EResourceType::Texture3D);

        ASSERT_EQ(res.getWidth(), loadedTextureResource->getWidth());
        ASSERT_EQ(res.getHeight(), loadedTextureResource->getHeight());
        ASSERT_EQ(res.getDepth(), loadedTextureResource->getDepth());
        ASSERT_EQ(EPixelStorageFormat::RGB8, loadedTextureResource->getTextureFormat());
        ASSERT_EQ(ETextureChannelColor::Blue, loadedTextureResource->getTextureSwizzle()[0]);
        ASSERT_EQ(ETextureChannelColor::Red, loadedTextureResource->getTextureSwizzle()[1]);
        ASSERT_EQ(ETextureChannelColor::Alpha, loadedTextureResource->getTextureSwizzle()[2]);
        ASSERT_EQ(ETextureChannelColor::Green, loadedTextureResource->getTextureSwizzle()[3]);
        ASSERT_EQ(res.getGenerateMipChainFlag(), loadedTextureResource->getGenerateMipChainFlag());
        ASSERT_EQ(texDesc.m_dataSizes, loadedTextureResource->getMipDataSizes());
        EXPECT_EQ(std::string("resName"), loadedTextureResource->getName());
    }

    TEST_F(AResourcePersistation, WriteRead_TextureResourceCube)
    {
        const TextureMetaInfo texDesc(2u, 1u, 1u, EPixelStorageFormat::RGB8, false, {}, { 1u, 2u });
        TextureResource res(EResourceType::TextureCube, texDesc, "resName");
        ResourceBlob pixels(6u * std::accumulate(texDesc.m_dataSizes.cbegin(), texDesc.m_dataSizes.cend(), 0u));
        for (size_t i = 0; i < pixels.size(); ++i)
        {
            pixels.data()[i] = std::byte{static_cast<uint8_t>(i)};
        }
        ASSERT_EQ(pixels.size(), res.getResourceData().size());
        res.setResourceData(std::move(pixels));

        auto loadedTextureResource = createLoadedResource<TextureResource>(res, EResourceType::TextureCube);

        ASSERT_EQ(res.getWidth(), loadedTextureResource->getWidth());
        ASSERT_EQ(EPixelStorageFormat::RGB8, loadedTextureResource->getTextureFormat());
        ASSERT_EQ(res.getGenerateMipChainFlag(), loadedTextureResource->getGenerateMipChainFlag());
        ASSERT_EQ(texDesc.m_dataSizes, loadedTextureResource->getMipDataSizes());
        EXPECT_EQ(std::string("resName"), loadedTextureResource->getName());
    }

    TEST_F(AResourcePersistation, WriteRead_VertexArrayResource)
    {
        const uint32_t cnt = 200;

        ArrayResource res(EResourceType::VertexArray, cnt, EDataType::Vector2F, nullptr, "resName");
        ResourceBlob vertices(cnt * EnumToSize(EDataType::Vector2F));
        for (size_t i = 0; i < 2*cnt; ++i)
        {
            UnsafeTestMemoryHelpers::WriteToMemoryBlob(static_cast<float>(i) * .1f, vertices.data(), i);
        }
        res.setResourceData(std::move(vertices));

        auto loadedVertexArrayResource = createLoadedResource<ArrayResource>(res, EResourceType::VertexArray);

        ASSERT_EQ(res.getElementCount(), loadedVertexArrayResource->getElementCount());
        ASSERT_EQ(EDataType::Vector2F, loadedVertexArrayResource->getElementType());
        EXPECT_EQ(std::string("resName"), loadedVertexArrayResource->getName());
    }

    TEST_F(AResourcePersistation, WriteRead_Index16ArrayResource)
    {
        const uint32_t cnt = 220;

        ArrayResource res(EResourceType::IndexArray, cnt, EDataType::UInt16, nullptr, "resName");
        ResourceBlob indices(cnt * EnumToSize(EDataType::UInt16));
        for (size_t i = 0; i < cnt; ++i)
        {
            UnsafeTestMemoryHelpers::WriteToMemoryBlob(static_cast<uint16_t>(i), indices.data(), i);
        }
        res.setResourceData(std::move(indices));

        auto loadedIndexArrayResource = createLoadedResource<ArrayResource>(res, EResourceType::IndexArray);

        ASSERT_EQ(res.getElementCount(), loadedIndexArrayResource->getElementCount());
        ASSERT_EQ(EDataType::UInt16, loadedIndexArrayResource->getElementType());
        EXPECT_EQ(std::string("resName"), loadedIndexArrayResource->getName());
    }

    TEST_F(AResourcePersistation, WriteRead_Index32ArrayResource)
    {
        const uint32_t cnt = 220;

        ArrayResource res(EResourceType::IndexArray, cnt, EDataType::UInt32, nullptr, "resName");
        ResourceBlob indices(cnt * EnumToSize(EDataType::UInt32));
        for (size_t i = 0; i < cnt; ++i)
        {
            UnsafeTestMemoryHelpers::WriteToMemoryBlob(static_cast<uint32_t>(i), indices.data(), i);
        }
        res.setResourceData(std::move(indices));

        auto loadedIndexArrayResource = createLoadedResource<ArrayResource>(res, EResourceType::IndexArray);

        ASSERT_EQ(res.getElementCount(), loadedIndexArrayResource->getElementCount());
        ASSERT_EQ(EDataType::UInt32, loadedIndexArrayResource->getElementType());
        EXPECT_EQ(std::string("resName"), loadedIndexArrayResource->getName());
    }

    TEST_F(AResourcePersistation, WriteRead_EffectResource)
    {
        const auto effectResource = CreateTestEffect(EFeatureLevel_Latest);

        auto loadedEffectResource = createLoadedResource<EffectResource>(effectResource, EResourceType::Effect);
        ResourceSerializationTestHelper::CompareTypedResources<EffectResource>(effectResource, *loadedEffectResource);
    }

    TEST_F(AResourcePersistation, WriteRead_EffectResource_FL01)
    {
        static_assert(EFeatureLevel_Latest != EFeatureLevel_01, "Remove test when feature levels flattened");
        const auto effectResource = CreateTestEffect(EFeatureLevel_01);

        auto loadedEffectResource = createLoadedResource<EffectResource>(effectResource, EResourceType::Effect, EFeatureLevel_01);

        EXPECT_STREQ(effectResource.getVertexShader(), loadedEffectResource->getVertexShader());
        EXPECT_STREQ(effectResource.getFragmentShader(), loadedEffectResource->getFragmentShader());
        EXPECT_STREQ(effectResource.getGeometryShader(), loadedEffectResource->getGeometryShader());
        EXPECT_EQ(std::string("test effect"), loadedEffectResource->getName());
        EXPECT_EQ(EDrawMode::Lines, loadedEffectResource->getGeometryShaderInputType());

        EXPECT_EQ(0u, loadedEffectResource->getVertexShaderSPIRVSize());
        EXPECT_EQ(0u, loadedEffectResource->getFragmentShaderSPIRVSize());
        EXPECT_EQ(0u, loadedEffectResource->getGeometryShaderSPIRVSize());
    }

    TEST(ResourcePersistation, sandwich_writeThreeResources_ReadOneBackBasedTableOfContentsInformation)
    {
        NiceMock<ManagedResourceDeleterCallbackMock> managedResourceDeleter;
        ResourceDeleterCallingCallback dummyManagedResourceCallback(managedResourceDeleter);

        float dataA[9];
        for (uint32_t i = 0u; i < 9; ++i)
        {
            dataA[i] = static_cast<float>(i);
        }
        ArrayResource res(EResourceType::VertexArray, 3, EDataType::Vector3F, dataA, "res1");
        ManagedResource managedRes{ &res, dummyManagedResourceCallback };
        const ResourceContentHash hash = managedRes->getHash();

        float dataB[18];
        for (uint32_t i = 0u; i < 18; ++i)
        {
            dataB[i] = static_cast<float>(i);
        }
        ArrayResource res2(EResourceType::VertexArray, 6, EDataType::Vector3F, dataB, "res2");
        ManagedResource managedRes2{ &res2, dummyManagedResourceCallback };
        const ResourceContentHash hash2 = managedRes2->getHash();

        EffectResource res3("foo", "bar", "qux", {}, EDrawMode::Lines, EffectInputInformationVector(), EffectInputInformationVector(), "Some effect with a name", EFeatureLevel_Latest);
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
            auto loadedResource = ResourcePersistation::RetrieveResourceFromStream(instream, loadedTOC.getEntryForHash(hash), EFeatureLevel_Latest);
            ASSERT_TRUE(UnsafeTestMemoryHelpers::CompareMemoryBlobToSpan(dataA, sizeof(dataA), loadedResource->getResourceData().span()));
            EXPECT_EQ(std::string("res1"), loadedResource->getName());
        }

        {
            ASSERT_TRUE(loadedTOC.containsResource(hash2));
            auto loadedResource = ResourcePersistation::RetrieveResourceFromStream(instream, loadedTOC.getEntryForHash(hash2), EFeatureLevel_Latest);
            ASSERT_TRUE(UnsafeTestMemoryHelpers::CompareMemoryBlobToSpan(dataB, sizeof(dataB), loadedResource->getResourceData().span()));
            EXPECT_EQ(std::string("res2"), loadedResource->getName());
        }

        {
            ASSERT_TRUE(loadedTOC.containsResource(hash3));
            auto loadedResource = ResourcePersistation::RetrieveResourceFromStream(instream, loadedTOC.getEntryForHash(hash3), EFeatureLevel_Latest);
            EXPECT_STREQ(res3.getVertexShader(), loadedResource->convertTo<EffectResource>()->getVertexShader());
            EXPECT_STREQ(res3.getFragmentShader(), loadedResource->convertTo<EffectResource>()->getFragmentShader());
            EXPECT_EQ(std::string("Some effect with a name"), loadedResource->getName());
        }
    }

    static std::pair<std::vector<std::byte>, ResourceFileEntry> getDummyResourceData()
    {
        BinaryOutputStream outStream;
        EffectResource res("foo", "bar", "qux", {}, EDrawMode::Lines, EffectInputInformationVector(), EffectInputInformationVector(), "Some effect with a name", EFeatureLevel_Latest);
        NiceMock<ManagedResourceDeleterCallbackMock> managedResourceDeleter;
        ResourceDeleterCallingCallback dummyManagedResourceCallback(managedResourceDeleter);
        ManagedResource managedRes{ &res, dummyManagedResourceCallback };
        ResourcePersistation::WriteOneResourceToStream(outStream, managedRes);
        auto data = outStream.release();

        const auto dataSize = static_cast<uint32_t>(data.size());
        ResourceFileEntry entry{0, dataSize,
                                ResourceInfo(EResourceType::Effect, res.getHash(), dataSize, 0)};
        return {data, entry};
    }

    TEST(ResourcePersistation, retrieveResourceFromStreamCanLoadDummyData)
    {
        const auto dummyResource = getDummyResourceData();
        InputStreamMock stream;
        BinaryInputStream resStream(dummyResource.first.data());
        EXPECT_CALL(stream, seek(_, _)).WillRepeatedly([&](int64_t offset, auto origin) {
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
        EXPECT_TRUE(ResourcePersistation::RetrieveResourceFromStream(stream, dummyResource.second, EFeatureLevel_Latest));
    }

    TEST(ResourcePersistation, retrieveResourceFromStreamCanHandleSeekErrors)
    {
        const auto dummyResource = getDummyResourceData();
        InputStreamMock stream;
        BinaryInputStream resStream(dummyResource.first.data());
        EXPECT_CALL(stream, seek(_, _)).WillRepeatedly(Return(EStatus::Error));
        EXPECT_FALSE(ResourcePersistation::RetrieveResourceFromStream(stream, dummyResource.second, EFeatureLevel_Latest));
    }

    TEST(ResourcePersistation, retrieveResourceFromStreamCanHandleGetPosErrors)
    {
        const auto dummyResource = getDummyResourceData();
        InputStreamMock stream;
        BinaryInputStream resStream(dummyResource.first.data());
        EXPECT_CALL(stream, seek(_, _)).WillRepeatedly([&](int64_t offset, auto origin) {
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
        EXPECT_FALSE(ResourcePersistation::RetrieveResourceFromStream(stream, dummyResource.second, EFeatureLevel_Latest));
    }

    TEST(ResourcePersistation, retrieveResourceFromStreamCanHandleGetPosWrongData)
    {
        const auto dummyResource = getDummyResourceData();
        InputStreamMock stream;
        BinaryInputStream resStream(dummyResource.first.data());
        EXPECT_CALL(stream, seek(_, _)).WillRepeatedly([&](int64_t offset, auto origin) {
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
        EXPECT_FALSE(ResourcePersistation::RetrieveResourceFromStream(stream, dummyResource.second, EFeatureLevel_Latest));
    }

    TEST(ResourcePersistation, retrieveResourceFromStreamCanHandleGetStateErrors)
    {
        const auto dummyResource = getDummyResourceData();
        InputStreamMock stream;
        BinaryInputStream resStream(dummyResource.first.data());
        EXPECT_CALL(stream, seek(_, _)).WillRepeatedly([&](int64_t offset, auto origin) {
            return resStream.seek(offset, origin);
        });
        EXPECT_CALL(stream, getState()).WillRepeatedly(Return(EStatus::Error));
        EXPECT_CALL(stream, read(_, _)).WillRepeatedly([&](void* data, size_t size) -> IInputStream& {
            resStream.read(data, size);
            return stream;
        });
        EXPECT_FALSE(ResourcePersistation::RetrieveResourceFromStream(stream, dummyResource.second, EFeatureLevel_Latest));
    }

    TEST(ResourcePersistation, retrieveResourceFromStreamCanHandleZeroReader)
    {
        const auto dummyResource = getDummyResourceData();
        InputStreamMock stream;
        BinaryInputStream resStream(dummyResource.first.data());
        EXPECT_CALL(stream, seek(_, _)).WillRepeatedly([&](int64_t offset, auto origin) {
            return resStream.seek(offset, origin);
        });
        EXPECT_CALL(stream, read(_, _)).WillRepeatedly([&](void* data, size_t size) -> IInputStream& {
            resStream.read(data, size);
            std::memset(data, 0, size);
            return stream;
        });
        EXPECT_FALSE(ResourcePersistation::RetrieveResourceFromStream(stream, dummyResource.second, EFeatureLevel_Latest));
    }
}
