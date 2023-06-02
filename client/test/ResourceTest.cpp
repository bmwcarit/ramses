//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/ArrayResource.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Texture3D.h"
#include "ramses-client-api/TextureCube.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/TextureSwizzle.h"
#include "ramses-client-api/SceneObjectIterator.h"
#include "ramses-utils.h"

#include "Texture2DImpl.h"
#include "Texture3DImpl.h"
#include "TextureCubeImpl.h"
#include "ArrayResourceImpl.h"
#include "EffectImpl.h"
#include "RamsesObjectTestTypes.h"
#include "ClientTestUtils.h"
#include "Resource/IResource.h"
#include "PlatformAbstraction/PlatformMemory.h"
#include "Components/ManagedResource.h"
#include "RamsesClientImpl.h"
#include "UnsafeTestMemoryHelpers.h"

#include <thread>
#include <string_view>

namespace ramses
{
    using namespace testing;

    class AResourceTestClient : public LocalTestClientWithScene, public ::testing::Test
    {
    public:
        AResourceTestClient()
        {
            m_oldLogLevel = ramses_internal::CONTEXT_HLAPI_CLIENT.getLogLevel();
            ramses_internal::CONTEXT_HLAPI_CLIENT.setLogLevel(ramses_internal::ELogLevel::Trace);
        }
        ~AResourceTestClient() override {
            ramses_internal::CONTEXT_HLAPI_CLIENT.setLogLevel(m_oldLogLevel);
        }
        ramses_internal::ManagedResource getCreatedResource(const ramses_internal::ResourceContentHash& hash)
        {
            return client.m_impl.getClientApplication().getResource(hash);
        }
        ramses_internal::ELogLevel m_oldLogLevel;
    };

    //##############################################################
    //##############   Texture tests ###############################
    //##############################################################

    TEST_F(AResourceTestClient, createTextureAndDestroyManually)
    {
        const uint8_t data[4 * 10 * 12] = {};
        MipLevelData mipLevelData(sizeof(data), data);
        Texture2D* texture = m_scene.createTexture2D(ramses::ETextureFormat::RGBA8, 10, 12, 1, &mipLevelData, false, {}, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(nullptr != texture);
        EXPECT_EQ(StatusOK, m_scene.destroy(*texture));
    }

    TEST_F(AResourceTestClient, createTextureWithMipMaps)
    {
        const uint8_t data[2 * 2 * 4] = {};
        std::vector<MipLevelData> mipLevelData;
        mipLevelData.push_back(MipLevelData(2 * 2 * 4, data));
        mipLevelData.push_back(MipLevelData(1 * 1 * 4, data));
        Texture2D* texture = m_scene.createTexture2D(ramses::ETextureFormat::RGBA8, 2, 2, 2, &mipLevelData[0], false, {}, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(nullptr != texture);
    }

    TEST_F(AResourceTestClient, createTextureAndCheckWidthHeight)
    {
        const uint8_t data[4 * 10 * 12] = {};
        MipLevelData mipLevelData(sizeof(data), data);
        Texture2D* texture = m_scene.createTexture2D(ramses::ETextureFormat::RGBA8, 10, 12, 1, &mipLevelData, false, {}, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(nullptr != texture);
        EXPECT_EQ(10u, texture->getWidth());
        EXPECT_EQ(12u, texture->getHeight());
    }

    TEST_F(AResourceTestClient, createTextureAndCheckFormat)
    {
        const uint8_t data[3 * 10 * 12] = {};
        MipLevelData mipLevelData(sizeof(data), data);
        Texture2D* texture = m_scene.createTexture2D(ramses::ETextureFormat::RGB8, 10, 12, 1, &mipLevelData, false, {}, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(nullptr != texture);
        EXPECT_EQ(ETextureFormat::RGB8, texture->getTextureFormat());
    }

    TEST_F(AResourceTestClient, createsTextureWithDefaultSwizzle)
    {
        const uint8_t data[3 * 10 * 12] = {};
        MipLevelData mipLevelData(sizeof(data), data);
        TextureSwizzle swizzle;
        Texture2D* texture = m_scene.createTexture2D(ramses::ETextureFormat::RGB8, 10, 12, 1, &mipLevelData, false, swizzle, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(nullptr != texture);
        EXPECT_EQ(ETextureChannelColor::Red, swizzle.channelRed);
        EXPECT_EQ(ETextureChannelColor::Green, swizzle.channelGreen);
        EXPECT_EQ(ETextureChannelColor::Blue, swizzle.channelBlue);
        EXPECT_EQ(ETextureChannelColor::Alpha, swizzle.channelAlpha);
    }

    TEST_F(AResourceTestClient, createsTextureWithNonDefaultSwizzle)
    {
        TextureSwizzle swizzle = { ETextureChannelColor::Blue, ETextureChannelColor::Alpha, ETextureChannelColor::Red, ETextureChannelColor::Green };
        const uint8_t data[3 * 10 * 12] = {};
        MipLevelData mipLevelData(sizeof(data), data);
        Texture2D* texture = m_scene.createTexture2D(ramses::ETextureFormat::RGB8, 10, 12, 1, &mipLevelData, false, swizzle, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(nullptr != texture);
        EXPECT_EQ(swizzle.channelRed, texture->getTextureSwizzle().channelRed);
        EXPECT_EQ(swizzle.channelGreen, texture->getTextureSwizzle().channelGreen);
        EXPECT_EQ(swizzle.channelBlue, texture->getTextureSwizzle().channelBlue);
        EXPECT_EQ(swizzle.channelAlpha, texture->getTextureSwizzle().channelAlpha);
    }

    TEST_F(AResourceTestClient, createTextureWithProvidedMipsButNotFullChain)
    {
        const uint8_t data[4 * 4 * 4] = {};

        std::vector<MipLevelData> mipLevelData;
        mipLevelData.push_back(MipLevelData(sizeof(data), data));
        mipLevelData.push_back(MipLevelData(2 * 2 * 4, data));

        Texture2D* texture = m_scene.createTexture2D(ramses::ETextureFormat::RGBA8, 4u, 4u, 2, &mipLevelData[0], false, {}, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(nullptr != texture);
    }

    TEST_F(AResourceTestClient, createTextureWithMoreMipsThanExpected)
    {
        const uint8_t data[1] = {};

        std::vector<MipLevelData> mipLevelData;
        mipLevelData.push_back(MipLevelData(sizeof(data), data));
        mipLevelData.push_back(MipLevelData(sizeof(data), data));

        Texture2D* texture = m_scene.createTexture2D(ramses::ETextureFormat::RGBA8, 1u, 1u, 2, &mipLevelData[0], false, {}, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, createTextureWithNullMipMapData)
    {
        const uint8_t data[4 * 4 * 4] = {};

        std::vector<MipLevelData> mipLevelData;
        mipLevelData.push_back(MipLevelData(sizeof(data), nullptr));

        Texture2D* texture = m_scene.createTexture2D(ramses::ETextureFormat::RGBA8, 4u, 4u, 1, &mipLevelData[0], false, {}, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, createTextureWithZeroSizeMipMapData)
    {
        const uint8_t data[4 * 4 * 4] = {};

        std::vector<MipLevelData> mipLevelData;
        mipLevelData.push_back(MipLevelData(0u, data));

        Texture2D* texture = m_scene.createTexture2D(ramses::ETextureFormat::RGBA8, 4u, 4u, 1, &mipLevelData[0], false, {}, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, createTextureWithNullOrZeroSizeLowerMipMapData)
    {
        const uint8_t data[2 * 2 * 4] = {};
        {
            std::vector<MipLevelData> mipLevelData;
            mipLevelData.push_back(MipLevelData(sizeof(data), data));
            mipLevelData.push_back(MipLevelData(0u, data));
            Texture2D* texture = m_scene.createTexture2D(ramses::ETextureFormat::RGBA8, 2u, 2u, 2, &mipLevelData[0], false, {}, ramses::ResourceCacheFlag_DoNotCache, "name");
            EXPECT_EQ(nullptr, texture);
        }
        {
            std::vector<MipLevelData> mipLevelData;
            mipLevelData.push_back(MipLevelData(sizeof(data), data));
            mipLevelData.push_back(MipLevelData(sizeof(data), nullptr));
            Texture2D* texture = m_scene.createTexture2D(ramses::ETextureFormat::RGBA8, 2u, 2u, 2, &mipLevelData[0], false, {}, ramses::ResourceCacheFlag_DoNotCache, "name");
            EXPECT_EQ(nullptr, texture);
        }
    }

    TEST_F(AResourceTestClient, createTextureWithWrongSizeOfLowerMipMapData)
    {
        const uint8_t data[2 * 2 * 4] = {};
        std::vector<MipLevelData> mipLevelData;
        mipLevelData.push_back(MipLevelData(2 * 2 * 4, data));
        mipLevelData.push_back(MipLevelData(1 * 1 * 2, data));
        Texture2D* texture = m_scene.createTexture2D(ramses::ETextureFormat::RGBA8, 2u, 2u, 2, &mipLevelData[0], false, {}, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, createTextureOfZeroSize)
    {
        const uint8_t data[4 * 10 * 12] = {};
        MipLevelData mipLevelData(sizeof(data), data);
        Texture2D* texture = m_scene.createTexture2D(ramses::ETextureFormat::RGBA8, 0, 0, 1, &mipLevelData, false, {}, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, createTextureWithNoMipData)
    {
        Texture2D* texture = m_scene.createTexture2D(ramses::ETextureFormat::RGBA8, 1, 1, 1, nullptr, false, {}, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, createTextureWithoutName)
    {
        const uint8_t data[4 * 10 * 12] = {};
        MipLevelData mipLevelData(sizeof(data), data);
        Texture2D* texture = m_scene.createTexture2D(ramses::ETextureFormat::RGBA8, 10, 12, 1, &mipLevelData, false, {}, ramses::ResourceCacheFlag_DoNotCache, {});
        ASSERT_TRUE(nullptr != texture);
        EXPECT_STREQ("", texture->getName());
    }

    TEST_F(AResourceTestClient, createTextureCheckHashIsValid)
    {
        const uint8_t data[4 * 10 * 12] = {};
        MipLevelData mipLevelData(sizeof(data), data);
        Texture2D* texture = m_scene.createTexture2D(ramses::ETextureFormat::RGBA8, 10, 12, 1, &mipLevelData, false, {}, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(nullptr != texture);

        const ramses_internal::ResourceContentHash hash = texture->m_impl.getLowlevelResourceHash();
        EXPECT_TRUE(hash.isValid());
    }

    TEST_F(AResourceTestClient, createTextureCheckHashIsUnique)
    {
        uint8_t data[4 * 10 * 12] = {};
        data[20] = 48;
        MipLevelData mipLevelData(sizeof(data), data);
        Texture2D* texture = m_scene.createTexture2D(ramses::ETextureFormat::RGBA8, 10, 12, 1, &mipLevelData, false, {}, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(nullptr != texture);

        const ramses_internal::ResourceContentHash hash = texture->m_impl.getLowlevelResourceHash();
        uint8_t data2[4 * 10 * 12] = {};
        data[20] = 42;
        MipLevelData mipLevelData2(sizeof(data2), data2);
        Texture2D* texture2 = m_scene.createTexture2D(ramses::ETextureFormat::RGBA8, 10, 12, 1, &mipLevelData2, false, {}, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(nullptr != texture2);

        const ramses_internal::ResourceContentHash hash2 = texture2->m_impl.getLowlevelResourceHash();
        EXPECT_TRUE(hash != hash2);
    }

    TEST_F(AResourceTestClient, createTextureRGBA_AndCheckTexels)
    {
        const uint8_t data[1 * 2 * 4] = { 1, 2, 3, 4, 5, 6, 7, 8 };
        MipLevelData mipLevelData(sizeof(data), data);
        Texture2D* texture = m_scene.createTexture2D(ramses::ETextureFormat::RGBA8, 2, 1, 1, &mipLevelData, false, {}, ramses::ResourceCacheFlag_DoNotCache, {});
        ASSERT_TRUE(nullptr != texture);
        ramses_internal::ManagedResource res = getCreatedResource(texture->m_impl.getLowlevelResourceHash());

        ASSERT_EQ(sizeof(data), res->getDecompressedDataSize());
        EXPECT_EQ(data, res->getResourceData().span());
    }

    TEST_F(AResourceTestClient, createTextureRGB_AndCheckTexels)
    {
        const uint8_t data[1 * 2 * 3] = { 1, 2, 3, 4, 5, 6 };
        MipLevelData mipLevelData(sizeof(data), data);
        Texture2D* texture = m_scene.createTexture2D(ramses::ETextureFormat::RGB8, 2, 1, 1, &mipLevelData, false, {}, ramses::ResourceCacheFlag_DoNotCache, {});
        ASSERT_TRUE(nullptr != texture);

        ramses_internal::ManagedResource res = getCreatedResource(texture->m_impl.getLowlevelResourceHash());

        ASSERT_EQ(sizeof(data), res->getDecompressedDataSize());
        EXPECT_EQ(data, res->getResourceData().span());
    }

    TEST_F(AResourceTestClient, createTextureRGBWithMips_AndCheckTexels)
    {
        const uint8_t data0[2 * 2 * 3] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
        const uint8_t data1[1 * 1 * 3] = { 13, 14, 15 };
        const MipLevelData mipLevelData[2] = { { sizeof(data0), data0 },{ sizeof(data1), data1 } };
        const Texture2D* texture = m_scene.createTexture2D(ramses::ETextureFormat::RGB8, 2, 2, 2, mipLevelData, false, {}, ramses::ResourceCacheFlag_DoNotCache, {});
        ASSERT_TRUE(nullptr != texture);

        const ramses_internal::ManagedResource res = getCreatedResource(texture->m_impl.getLowlevelResourceHash());

        ASSERT_EQ(sizeof(data0) + sizeof(data1), res->getDecompressedDataSize());
        EXPECT_EQ(data0, res->getResourceData().span().subspan(0, sizeof(data0)));
        EXPECT_EQ(data1, res->getResourceData().span().subspan(sizeof(data0)));
    }

    //##############################################################
    //##############    Cube Texture tests #########################
    //##############################################################

    TEST_F(AResourceTestClient, createCubeTextureAndDestroyManually)
    {
        const uint8_t data[4 * 10 * 10] = {};
        CubeMipLevelData mipLevelData(sizeof(data), data, data, data, data, data, data);
        TextureCube* texture = m_scene.createTextureCube(ramses::ETextureFormat::RGBA8, 10, 1, &mipLevelData, false);
        ASSERT_TRUE(nullptr != texture);
        EXPECT_EQ(StatusOK, m_scene.destroy(*texture));
    }

    TEST_F(AResourceTestClient, createCubeTextureWithMipMaps)
    {
        const uint8_t data[2 * 2 * 4] = {};
        std::vector<CubeMipLevelData> mipLevelData;
        mipLevelData.push_back(CubeMipLevelData(2 * 2 * 4, data, data, data, data, data, data));
        mipLevelData.push_back(CubeMipLevelData(1 * 1 * 4, data, data, data, data, data, data));
        TextureCube* texture = m_scene.createTextureCube(ramses::ETextureFormat::RGBA8, 2, 2, &mipLevelData[0], false, {}, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(nullptr != texture);
    }

    TEST_F(AResourceTestClient, createCubeTextureAndCheckWidthHeight)
    {
        const uint8_t data[4 * 10 * 10] = {};
        CubeMipLevelData mipLevelData(sizeof(data), data, data, data, data, data, data);
        TextureCube* texture = m_scene.createTextureCube(ramses::ETextureFormat::RGBA8, 10, 1, &mipLevelData, false, {}, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(nullptr != texture);
        EXPECT_EQ(10u, texture->getSize());
    }

    TEST_F(AResourceTestClient, createCubeTextureAndCheckFormat)
    {
        const uint8_t data[3 * 10 * 10] = {};
        CubeMipLevelData mipLevelData(sizeof(data), data, data, data, data, data, data);
        TextureCube* texture = m_scene.createTextureCube(ramses::ETextureFormat::RGB8, 10, 1, &mipLevelData, false, {}, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(nullptr != texture);
        EXPECT_EQ(ETextureFormat::RGB8, texture->getTextureFormat());
    }

    TEST_F(AResourceTestClient, createsCubeTextureWithDefaultSwizzle)
    {
        TextureSwizzle swizzle;
        const uint8_t data[4 * 10 * 10] = {};
        const CubeMipLevelData mipLevelData(sizeof(data), data, data, data, data, data, data);
        TextureCube* texture = m_scene.createTextureCube(ramses::ETextureFormat::RGBA8, 10, 1, &mipLevelData, false, swizzle, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(nullptr != texture);
        EXPECT_EQ(ETextureChannelColor::Red, swizzle.channelRed);
        EXPECT_EQ(ETextureChannelColor::Green, swizzle.channelGreen);
        EXPECT_EQ(ETextureChannelColor::Blue, swizzle.channelBlue);
        EXPECT_EQ(ETextureChannelColor::Alpha, swizzle.channelAlpha);
    }

    TEST_F(AResourceTestClient, createsCubeTextureWithNonDefaultSwizzle)
    {
        TextureSwizzle swizzle = { ETextureChannelColor::Blue, ETextureChannelColor::Alpha, ETextureChannelColor::Red, ETextureChannelColor::Green };
        const uint8_t data[4 * 10 * 10] = {};
        const CubeMipLevelData mipLevelData(sizeof(data), data, data, data, data, data, data);
        TextureCube* texture = m_scene.createTextureCube(ramses::ETextureFormat::RGBA8, 10, 1, &mipLevelData, false, swizzle, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(nullptr != texture);
        EXPECT_EQ(swizzle.channelRed, texture->getTextureSwizzle().channelRed);
        EXPECT_EQ(swizzle.channelGreen, texture->getTextureSwizzle().channelGreen);
        EXPECT_EQ(swizzle.channelBlue, texture->getTextureSwizzle().channelBlue);
        EXPECT_EQ(swizzle.channelAlpha, texture->getTextureSwizzle().channelAlpha);
    }

    TEST_F(AResourceTestClient, createCubeTextureOfZeroSize)
    {
        const uint8_t data[4 * 10 * 10] = {};
        CubeMipLevelData mipLevelData(sizeof(data), data, data, data, data, data, data);
        TextureCube* texture = m_scene.createTextureCube(ramses::ETextureFormat::RGBA8, 0, 1, &mipLevelData, false, {}, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, createCubeTextureOfZeroDataSize)
    {
        const uint8_t data[4 * 10 * 10] = {};
        CubeMipLevelData mipLevelData(0, data, data, data, data, data, data);
        TextureCube* texture = m_scene.createTextureCube(ramses::ETextureFormat::RGBA8, 10, 1, &mipLevelData, false, {}, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, createCubeTextureWithNoMipData)
    {
        TextureCube* texture = m_scene.createTextureCube(ramses::ETextureFormat::RGBA8, 10, 1, nullptr, false, {}, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, createCubeTextureWithoutName)
    {
        const uint8_t data[4 * 10 * 10] = {};
        CubeMipLevelData mipLevelData(sizeof(data), data, data, data, data, data, data);
        TextureCube* texture = m_scene.createTextureCube(ramses::ETextureFormat::RGBA8, 10, 1, &mipLevelData, false, {}, ramses::ResourceCacheFlag_DoNotCache, {});
        ASSERT_TRUE(nullptr != texture);
        EXPECT_STREQ("", texture->getName());
    }

    TEST_F(AResourceTestClient, createCubeTextureRGBA_AndCheckTexels)
    {
        const uint8_t data[2 * 2 * 4] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
        CubeMipLevelData mipLevelData(sizeof(data), data, data, data, data, data, data);
        TextureCube* texture = m_scene.createTextureCube(ramses::ETextureFormat::RGBA8, 2, 1, &mipLevelData, false);
        ASSERT_TRUE(nullptr != texture);
        ramses_internal::ManagedResource res = getCreatedResource(texture->m_impl.getLowlevelResourceHash());

        ASSERT_EQ(sizeof(data) * 6u, res->getDecompressedDataSize());
        for (uint32_t i = 0u; i < 6u; ++i)
            EXPECT_EQ(data, res->getResourceData().span().subspan(i*sizeof(data), sizeof(data)));
    }

    TEST_F(AResourceTestClient, createCubeTextureRGB_AndCheckTexels)
    {
        const uint8_t data[2 * 2 * 3] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
        CubeMipLevelData mipLevelData(sizeof(data), data, data, data, data, data, data);
        TextureCube* texture = m_scene.createTextureCube(ramses::ETextureFormat::RGB8, 2, 1, &mipLevelData, false);
        ASSERT_TRUE(nullptr != texture);
        ramses_internal::ManagedResource res = getCreatedResource(texture->m_impl.getLowlevelResourceHash());

        ASSERT_EQ(sizeof(data) * 6u, res->getDecompressedDataSize());
        for (uint32_t i = 0u; i < 6u; ++i)
            EXPECT_EQ(data, res->getResourceData().span().subspan(i*sizeof(data), sizeof(data)));
    }

    TEST_F(AResourceTestClient, createCubeTextureRGBWithPerFaceDataAndMips_AndCheckTexels)
    {
        const uint8_t data0px[2 * 2 * 3] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
        const uint8_t data1px[1 * 1 * 3] = { 13, 14, 15 };
        const uint8_t data0nx[2 * 2 * 3] = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120 };
        const uint8_t data1nx[1 * 1 * 3] = { 130, 140, 150 };
        const uint8_t data0py[2 * 2 * 3] = { 11, 21, 31, 41, 51, 61, 71, 81, 91, 101, 111, 121 };
        const uint8_t data1py[1 * 1 * 3] = { 131, 141, 151 };
        const uint8_t data0ny[2 * 2 * 3] = { 12, 22, 32, 42, 52, 62, 72, 82, 92, 102, 112, 122 };
        const uint8_t data1ny[1 * 1 * 3] = { 132, 142, 152 };
        const uint8_t data0pz[2 * 2 * 3] = { 13, 23, 33, 43, 53, 63, 73, 83, 93, 103, 113, 123 };
        const uint8_t data1pz[1 * 1 * 3] = { 133, 143, 153 };
        const uint8_t data0nz[2 * 2 * 3] = { 14, 24, 34, 44, 54, 64, 74, 84, 94, 104, 114, 124 };
        const uint8_t data1nz[1 * 1 * 3] = { 134, 144, 154 };

        const CubeMipLevelData mipLevelData[2] =
        {
            { sizeof(data0px), data0px, data0nx, data0py, data0ny, data0pz, data0nz },
            { sizeof(data1px), data1px, data1nx, data1py, data1ny, data1pz, data1nz }
        };
        const TextureCube* texture = m_scene.createTextureCube(ramses::ETextureFormat::RGB8, 2u, 2, mipLevelData, false, {}, ramses::ResourceCacheFlag_DoNotCache, {});
        ASSERT_TRUE(nullptr != texture);

        const ramses_internal::ManagedResource res = getCreatedResource(texture->m_impl.getLowlevelResourceHash());

        ASSERT_EQ(6u * (sizeof(data0px) + sizeof(data1px)), res->getDecompressedDataSize());

        // mips are bundled together per face:
        // - facePX mip0, facePX mip1 .. face PX mipN, face NX mips, face PY .. face NZ
        size_t dataStart = 0;
        EXPECT_EQ(data0px, res->getResourceData().span().subspan(dataStart, sizeof(data0px)));
        dataStart += sizeof(data0px);
        EXPECT_EQ(data1px, res->getResourceData().span().subspan(dataStart, sizeof(data1px)));
        dataStart += sizeof(data1px);

        EXPECT_EQ(data0nx, res->getResourceData().span().subspan(dataStart, sizeof(data0nx)));
        dataStart += sizeof(data0nx);
        EXPECT_EQ(data1nx, res->getResourceData().span().subspan(dataStart, sizeof(data1nx)));
        dataStart += sizeof(data1nx);

        EXPECT_EQ(data0py, res->getResourceData().span().subspan(dataStart, sizeof(data0py)));
        dataStart += sizeof(data0py);
        EXPECT_EQ(data1py, res->getResourceData().span().subspan(dataStart, sizeof(data1py)));
        dataStart += sizeof(data1py);

        EXPECT_EQ(data0ny, res->getResourceData().span().subspan(dataStart, sizeof(data0ny)));
        dataStart += sizeof(data0ny);
        EXPECT_EQ(data1ny, res->getResourceData().span().subspan(dataStart, sizeof(data1ny)));
        dataStart += sizeof(data1ny);

        EXPECT_EQ(data0pz, res->getResourceData().span().subspan(dataStart, sizeof(data0pz)));
        dataStart += sizeof(data0pz);
        EXPECT_EQ(data1pz, res->getResourceData().span().subspan(dataStart, sizeof(data1pz)));
        dataStart += sizeof(data1pz);

        EXPECT_EQ(data0nz, res->getResourceData().span().subspan(dataStart, sizeof(data0nz)));
        dataStart += sizeof(data0nz);
        EXPECT_EQ(data1nz, res->getResourceData().span().subspan(dataStart, sizeof(data1nz)));
    }

    TEST_F(AResourceTestClient, createTextureCubeWithProvidedMipsButNotFullChain)
    {
        const uint8_t data[4 * 4 * 4] = {};

        std::vector<CubeMipLevelData> mipLevelData;
        mipLevelData.push_back(CubeMipLevelData(sizeof(data), data, data, data, data, data, data));
        mipLevelData.push_back(CubeMipLevelData(2 * 2 * 4, data, data, data, data, data, data));

        TextureCube* texture = m_scene.createTextureCube(ramses::ETextureFormat::RGBA8, 4u, 2, &mipLevelData[0], false, {}, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_NE(nullptr, texture);
    }

    TEST_F(AResourceTestClient, createTextureCubeWithMoreMipsThanExpected)
    {
        const uint8_t data[1] = {};

        std::vector<CubeMipLevelData> mipLevelData;
        mipLevelData.push_back(CubeMipLevelData(sizeof(data), data, data, data, data, data, data));
        mipLevelData.push_back(CubeMipLevelData(sizeof(data), data, data, data, data, data, data));

        TextureCube* texture = m_scene.createTextureCube(ramses::ETextureFormat::RGBA8, 1u, 2, &mipLevelData[0], false, {}, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, createTextureCubeWithNullMipMapData)
    {
        const uint8_t data[4 * 4 * 4] = {};

        std::vector<CubeMipLevelData> mipLevelData;
        mipLevelData.push_back(CubeMipLevelData(sizeof(data), data, data, nullptr, data, data, data));

        TextureCube* texture = m_scene.createTextureCube(ramses::ETextureFormat::RGBA8, 4u, 1, &mipLevelData[0], false, {}, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, createTextureCubeWithZeroSizeMipMapData)
    {
        const uint8_t data[4 * 4 * 4] = {};

        std::vector<CubeMipLevelData> mipLevelData;
        mipLevelData.push_back(CubeMipLevelData(0u, data, data, data, data, data, data));

        TextureCube* texture = m_scene.createTextureCube(ramses::ETextureFormat::RGBA8, 4u, 1, &mipLevelData[0], false, {}, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, createTextureCubeWithNullOrZeroSizeLowerMipMapData)
    {
        const uint8_t data[2 * 2 * 4] = {};
        {
            std::vector<CubeMipLevelData> mipLevelData;
            mipLevelData.push_back(CubeMipLevelData(sizeof(data), data, data, data, data, data, data));
            mipLevelData.push_back(CubeMipLevelData(0u, data, data, data, data, data, data));
            TextureCube* texture = m_scene.createTextureCube(ramses::ETextureFormat::RGBA8, 2u, 2, &mipLevelData[0], false, {}, ramses::ResourceCacheFlag_DoNotCache, "name");
            EXPECT_EQ(nullptr, texture);
        }
        {
            std::vector<CubeMipLevelData> mipLevelData;
            mipLevelData.push_back(CubeMipLevelData(sizeof(data), data, data, data, data, data, data));
            mipLevelData.push_back(CubeMipLevelData(sizeof(data), data, data, data, data, nullptr, data));
            TextureCube* texture = m_scene.createTextureCube(ramses::ETextureFormat::RGBA8, 2u, 2, &mipLevelData[0], false, {}, ramses::ResourceCacheFlag_DoNotCache, "name");
            EXPECT_EQ(nullptr, texture);
        }
    }

    TEST_F(AResourceTestClient, createTextureCubeWithWrongSizeOfLowerMipMapData)
    {
        const uint8_t data[2 * 2 * 4] = {};
        std::vector<CubeMipLevelData> mipLevelData;
        mipLevelData.push_back(CubeMipLevelData(2 * 2 * 4, data, data, data, data, data, data));
        mipLevelData.push_back(CubeMipLevelData(1 * 1 * 2, data, data, data, data, data, data));
        TextureCube* texture = m_scene.createTextureCube(ramses::ETextureFormat::RGBA8, 2u, 2, &mipLevelData[0], false, {}, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(nullptr, texture);
    }

    //##############################################################
    //##############    3D texture tests ###########################
    //##############################################################

    TEST_F(AResourceTestClient, create3DTextureAndDestroyManually)
    {
        const uint8_t data[4 * 10 * 12 * 14] = {};
        MipLevelData mipLevelData(sizeof(data), data);
        Texture3D* texture = m_scene.createTexture3D(ramses::ETextureFormat::RGBA8, 10, 12, 14, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(nullptr != texture);
        EXPECT_EQ(StatusOK, m_scene.destroy(*texture));
    }

    TEST_F(AResourceTestClient, create3DTextureWithMipMaps)
    {
        const uint8_t data[2 * 2 * 2 * 4] = {};
        std::vector<MipLevelData> mipLevelData;
        mipLevelData.push_back(MipLevelData(2 * 2 * 2 * 4, data));
        mipLevelData.push_back(MipLevelData(1 * 1 * 1 * 4, data));
        Texture3D* texture = m_scene.createTexture3D(ramses::ETextureFormat::RGBA8, 2u, 2u, 2u, 2, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(nullptr != texture);
    }

    TEST_F(AResourceTestClient, create3DTextureAndCheckWidthHeightDepth)
    {
        const uint8_t data[4 * 10 * 12 * 14] = {};
        MipLevelData mipLevelData(sizeof(data), data);
        Texture3D* texture = m_scene.createTexture3D(ramses::ETextureFormat::RGBA8, 10, 12, 14, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(nullptr != texture);
        EXPECT_EQ(10u, texture->getWidth());
        EXPECT_EQ(12u, texture->getHeight());
        EXPECT_EQ(14u, texture->getDepth());
    }

    TEST_F(AResourceTestClient, create3DTextureAndCheckFormat)
    {
        const uint8_t data[3 * 10 * 12 * 14] = {};
        MipLevelData mipLevelData(sizeof(data), data);
        Texture3D* texture = m_scene.createTexture3D(ramses::ETextureFormat::RGB8, 10, 12, 14, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(nullptr != texture);
        EXPECT_EQ(ETextureFormat::RGB8, texture->getTextureFormat());
    }

    TEST_F(AResourceTestClient, create3DTextureOfZeroSize)
    {
        const uint8_t data[4 * 10 * 12 * 14] = {};
        MipLevelData mipLevelData(sizeof(data), data);
        Texture3D* texture = m_scene.createTexture3D(ramses::ETextureFormat::RGBA8, 0, 0, 0, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, create3DTextureWithNoMipData)
    {
        Texture3D* texture = m_scene.createTexture3D(ramses::ETextureFormat::RGBA8, 1, 1, 1, 1, nullptr, false, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, create3DTextureWithoutName)
    {
        const uint8_t data[4 * 10 * 12 * 14] = {};
        MipLevelData mipLevelData(sizeof(data), data);
        Texture3D* texture = m_scene.createTexture3D(ramses::ETextureFormat::RGBA8, 10, 12, 14, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, {});
        ASSERT_TRUE(nullptr != texture);
        EXPECT_STREQ("", texture->getName());
    }

    TEST_F(AResourceTestClient, create3DTextureCheckHashIsValid)
    {
        const uint8_t data[4 * 10 * 12 * 14] = {};
        MipLevelData mipLevelData(sizeof(data), data);
        Texture3D* texture = m_scene.createTexture3D(ramses::ETextureFormat::RGBA8, 10, 12, 14, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(nullptr != texture);

        const ramses_internal::ResourceContentHash hash = texture->m_impl.getLowlevelResourceHash();
        EXPECT_TRUE(hash.isValid());
    }

    TEST_F(AResourceTestClient, create3DTextureCheckHashIsUnique)
    {
        uint8_t data[4 * 10 * 12 * 14] = {};
        data[20] = 48;
        MipLevelData mipLevelData1(sizeof(data), data);
        Texture3D* texture = m_scene.createTexture3D(ramses::ETextureFormat::RGBA8, 10, 12, 14, 1, &mipLevelData1, false, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(nullptr != texture);

        const ramses_internal::ResourceContentHash hash = texture->m_impl.getLowlevelResourceHash();
        uint8_t data2[4 * 10 * 12 * 14] = {};
        data[20] = 42;
        MipLevelData mipLevelData2(sizeof(data2), data2);
        Texture3D* texture2 = m_scene.createTexture3D(ramses::ETextureFormat::RGBA8, 10, 12, 14, 1, &mipLevelData2, false, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(nullptr != texture2);

        const ramses_internal::ResourceContentHash hash2 = texture2->m_impl.getLowlevelResourceHash();
        EXPECT_TRUE(hash != hash2);
    }

    TEST_F(AResourceTestClient, create3DTextureRGBA_AndCheckTexels)
    {
        const uint8_t data[1 * 2 * 2 * 4] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
        MipLevelData mipLevelData(sizeof(data), data);
        Texture3D* texture = m_scene.createTexture3D(ramses::ETextureFormat::RGBA8, 2, 1, 2, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, {});

        ASSERT_TRUE(nullptr != texture);
        ramses_internal::ManagedResource res = getCreatedResource(texture->m_impl.getLowlevelResourceHash());

        ASSERT_EQ(sizeof(data), res->getDecompressedDataSize());
        EXPECT_EQ(data, res->getResourceData().span());
    }

    TEST_F(AResourceTestClient, create3DTextureRGB_AndCheckTexels)
    {
        const uint8_t data[1 * 2 * 2 * 3] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
        MipLevelData mipLevelData(sizeof(data), data);
        Texture3D* texture = m_scene.createTexture3D(ramses::ETextureFormat::RGB8, 2, 1, 2, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, {});

        ASSERT_TRUE(nullptr != texture);
        ramses_internal::ManagedResource res = getCreatedResource(texture->m_impl.getLowlevelResourceHash());

        ASSERT_EQ(sizeof(data), res->getDecompressedDataSize());
        EXPECT_EQ(data, res->getResourceData().span());
    }

    TEST_F(AResourceTestClient, create3DTextureRGBWithMips_AndCheckTexels)
    {
        const uint8_t data0[2 * 2 * 2 * 3] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120 };
        const uint8_t data1[1 * 1 * 3] = { 13, 14, 15 };
        const MipLevelData mipLevelData[2] = { { sizeof(data0), data0 },{ sizeof(data1), data1 } };
        const Texture3D* texture = m_scene.createTexture3D(ramses::ETextureFormat::RGB8, 2u, 2u, 2u, 2, mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, {});
        ASSERT_TRUE(nullptr != texture);

        const ramses_internal::ManagedResource res = getCreatedResource(texture->m_impl.getLowlevelResourceHash());

        ASSERT_EQ(sizeof(data0) + sizeof(data1), res->getDecompressedDataSize());
        EXPECT_EQ(data0, res->getResourceData().span().subspan(0, sizeof(data0)));
        EXPECT_EQ(data1, res->getResourceData().span().subspan(sizeof(data0)));
    }

    TEST_F(AResourceTestClient, createTexture3DWithProvidedMipsButNotFullChain)
    {
        const uint8_t data[4 * 4 * 4 * 4] = {};

        std::vector<MipLevelData> mipLevelData;
        mipLevelData.push_back(MipLevelData(sizeof(data), data));
        mipLevelData.push_back(MipLevelData(2 * 2 * 2 * 4, data));

        Texture3D* texture = m_scene.createTexture3D(ramses::ETextureFormat::RGBA8, 4u, 4u, 4u, 2, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_NE(nullptr, texture);
    }

    TEST_F(AResourceTestClient, createTexture3DWithMoreMipsThanExpected)
    {
        const uint8_t data[1] = {};

        std::vector<MipLevelData> mipLevelData;
        mipLevelData.push_back(MipLevelData(sizeof(data), data));
        mipLevelData.push_back(MipLevelData(sizeof(data), data));

        Texture3D* texture = m_scene.createTexture3D(ramses::ETextureFormat::RGBA8, 1u, 1u, 1u, 2, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, create3DTextureWithNullMipMapData)
    {
        const uint8_t data[2 * 2 * 2 * 4] = {};

        std::vector<MipLevelData> mipLevelData;
        mipLevelData.push_back(MipLevelData(sizeof(data), nullptr));

        Texture3D* texture = m_scene.createTexture3D(ramses::ETextureFormat::RGBA8, 2u, 2u, 2u, 1, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, create3DTextureWithZeroSizeMipMapData)
    {
        const uint8_t data[2 * 2 * 2 * 4] = {};

        std::vector<MipLevelData> mipLevelData;
        mipLevelData.push_back(MipLevelData(0, data));

        Texture3D* texture = m_scene.createTexture3D(ramses::ETextureFormat::RGBA8, 2u, 2u, 2u, 1, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, create3DTextureWithNullOrZeroSizeLowerMipMapData)
    {
        const uint8_t data[2 * 2 * 2 * 4] = {};
        {
            std::vector<MipLevelData> mipLevelData;
            mipLevelData.push_back(MipLevelData(sizeof(data), data));
            mipLevelData.push_back(MipLevelData(0u, data));
            Texture3D* texture = m_scene.createTexture3D(ramses::ETextureFormat::RGBA8, 2u, 2u, 2u, 2, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
            EXPECT_EQ(nullptr, texture);
        }
        {
            std::vector<MipLevelData> mipLevelData;
            mipLevelData.push_back(MipLevelData(sizeof(data), data));
            mipLevelData.push_back(MipLevelData(sizeof(data), nullptr));
            Texture3D* texture = m_scene.createTexture3D(ramses::ETextureFormat::RGBA8, 2u, 2u, 2u, 2, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
            EXPECT_EQ(nullptr, texture);
        }
    }

    TEST_F(AResourceTestClient, create3DTextureWithWrongSizeOfLowerMipMapData)
    {
        const uint8_t data[2 * 2 * 2 * 4] = {};
        std::vector<MipLevelData> mipLevelData;
        mipLevelData.push_back(MipLevelData(2 * 2 * 2 * 4, data));
        mipLevelData.push_back(MipLevelData(1 * 1 * 1 * 2, data));
        Texture3D* texture = m_scene.createTexture3D(ramses::ETextureFormat::RGBA8, 2u, 2u, 2u, 2, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, create3DTextureWithCompressedFormatCanBeCreatedWithArbitraryNonZeroDataSize)
    {
        const uint8_t data[2 * 2 * 2 * 4] = {};
        std::vector<MipLevelData> mipLevelData;
        mipLevelData.push_back(MipLevelData(1, data));
        mipLevelData.push_back(MipLevelData(1, data));
        Texture3D* texture = m_scene.createTexture3D(ramses::ETextureFormat::ASTC_RGBA_4x4, 2u, 2u, 2u, 2, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_TRUE(nullptr != texture);
    }

    //##############################################################
    //##############    Array resource tests #######################
    //##############################################################

    TEST_F(AResourceTestClient, creatingResourcesWithSameDataInternallyRefersToSameResourceHash)
    {
        const vec2f data[2] = { vec2f{1.f,2.f}, vec2f{3.f,4.f} };
        auto a = m_scene.createArrayResource(2, data);
        auto b = m_scene.createArrayResource(2, data);

        ASSERT_EQ(a->m_impl.getLowlevelResourceHash(), b->m_impl.getLowlevelResourceHash());
    }

    TEST_F(AResourceTestClient, creatingResourcesWithSameDataInternallySharesData)
    {
        const vec2f data[2] = { vec2f{1.f,2.f}, vec2f{3.f,4.f} };
        auto a = m_scene.createArrayResource(2, data);
        auto b = m_scene.createArrayResource(2, data);
        ramses_internal::ManagedResource resourceA = client.m_impl.getResource(a->m_impl.getLowlevelResourceHash());
        ramses_internal::ManagedResource resourceB = client.m_impl.getResource(b->m_impl.getLowlevelResourceHash());
        ASSERT_EQ(resourceA, resourceB);
    }

    TEST_F(AResourceTestClient, creatingResourcesWithSameDataContentInternallySharesData)
    {
        const vec2f data1[2] = { vec2f{1.f,2.f}, vec2f{3.f,4.f} };
        const vec2f data2[2] = { vec2f{1.f,2.f}, vec2f{3.f,4.f} };
        auto a = m_scene.createArrayResource(2, data1);
        auto b = m_scene.createArrayResource(2, data2);
        ramses_internal::ManagedResource resourceA = client.m_impl.getResource(a->m_impl.getLowlevelResourceHash());
        ramses_internal::ManagedResource resourceB = client.m_impl.getResource(b->m_impl.getLowlevelResourceHash());
        ASSERT_EQ(resourceA, resourceB);
    }

    TEST_F(AResourceTestClient, destroyingDuplicateResourceDoesNotDeleteData)
    {
        const vec2f data1[2] = { vec2f{1.f,2.f}, vec2f{3.f,4.f} };
        const vec2f data2[2] = { vec2f{1.f,2.f}, vec2f{3.f,4.f} };
        auto a = m_scene.createArrayResource(2, data1);
        auto b = m_scene.createArrayResource(2, data2);

        m_scene.destroy(*b);
        ramses_internal::ManagedResource aRes = client.m_impl.getResource(a->m_impl.getLowlevelResourceHash());
        EXPECT_TRUE(ramses_internal::UnsafeTestMemoryHelpers::CompareMemoryBlobToSpan(&data1, sizeof(data1), aRes->getResourceData().span()));
    }

    TEST_F(AResourceTestClient, createFloatArray)
    {
        const float data[2] = {};
        const auto a = m_scene.createArrayResource(2, data);
        ASSERT_TRUE(nullptr != a);
        EXPECT_EQ(a->getNumberOfElements(), 2u);
        EXPECT_EQ(a->getDataType(), EDataType::Float);
    }

    TEST_F(AResourceTestClient, createFloatArrayHashIsValid)
    {
        const float data[2] = {};
        auto a = m_scene.createArrayResource(2, data);
        ASSERT_TRUE(nullptr != a);

        const ramses_internal::ResourceContentHash hash = a->m_impl.getLowlevelResourceHash();
        EXPECT_TRUE(hash.isValid());
    }

    TEST_F(AResourceTestClient, createFloatArrayHashIsUnique)
    {
        float data[2] = {};
        data[0] = 4;
        auto a = m_scene.createArrayResource(2, data);

        const ramses_internal::ResourceContentHash hash = a->m_impl.getLowlevelResourceHash();

        float data2[2] = {};
        data2[0] = 42.0f;
        auto a2 = m_scene.createArrayResource(2, data2);

        const ramses_internal::ResourceContentHash hash2 = a2->m_impl.getLowlevelResourceHash();
        EXPECT_TRUE(hash != hash2);
    }

    TEST_F(AResourceTestClient, createEmptyFloatArrayFails)
    {
        auto a = m_scene.createArrayResource<float>(0, nullptr);
        EXPECT_TRUE(nullptr == a);
    }

    TEST_F(AResourceTestClient, createVector2fArray)
    {
        const vec2f data[2] = { vec2f{1.f,2.f}, vec2f{3.f,4.f} };
        const auto a = m_scene.createArrayResource(2, data);
        ASSERT_TRUE(nullptr != a);
        EXPECT_EQ(a->getNumberOfElements(), 2u);
        EXPECT_EQ(a->getDataType(), EDataType::Vector2F);
    }

    TEST_F(AResourceTestClient, createVector2fArrayHashIsValid)
    {
        const vec2f data[2] = { vec2f{1.f,2.f}, vec2f{3.f,4.f} };
        const auto a = m_scene.createArrayResource(2, data);
        ASSERT_TRUE(nullptr != a);

        const ramses_internal::ResourceContentHash hash = a->m_impl.getLowlevelResourceHash();
        EXPECT_TRUE(hash.isValid());
    }

    TEST_F(AResourceTestClient, createVector2fArrayHashIsUnique)
    {
        vec2f data[2] = { vec2f{1.f,2.f}, vec2f{3.f,4.f} };
        data[0][0] = 4;
        auto a = m_scene.createArrayResource(2, data);

        const ramses_internal::ResourceContentHash hash = a->m_impl.getLowlevelResourceHash();

        vec2f data2[2] = { vec2f{1.f,2.f}, vec2f{3.f,4.f} };
        data2[0][0] = 42.0f;
        auto a2 = m_scene.createArrayResource(2, data2);

        const ramses_internal::ResourceContentHash hash2 = a2->m_impl.getLowlevelResourceHash();
        EXPECT_TRUE(hash != hash2);
    }

    TEST_F(AResourceTestClient, createEmptyVector2fArrayFails)
    {
        auto a = m_scene.createArrayResource<vec2f>(0, nullptr);
        EXPECT_TRUE(nullptr == a);
    }

    TEST_F(AResourceTestClient, createVector3fArray)
    {
        const vec3f data[2] = { vec3f{1.f,2.f,3.f}, vec3f{3.f,4.f,5.f} };
        auto a = m_scene.createArrayResource(2, data);
        ASSERT_TRUE(nullptr != a);
        EXPECT_EQ(a->getNumberOfElements(), 2u);
        EXPECT_EQ(a->getDataType(), EDataType::Vector3F);
    }

    TEST_F(AResourceTestClient, createVector3fArrayHashIsValid)
    {
        const vec3f data[2] = { vec3f{1.f,2.f,3.f}, vec3f{3.f,4.f,5.f} };
        auto a = m_scene.createArrayResource(2, data);
        ASSERT_TRUE(nullptr != a);

        const ramses_internal::ResourceContentHash hash = a->m_impl.getLowlevelResourceHash();
        EXPECT_TRUE(hash.isValid());
    }

    TEST_F(AResourceTestClient, createVector3fArrayHashIsUnique)
    {
        vec3f data[2] = { vec3f{1.f,2.f,3.f}, vec3f{3.f,4.f,5.f} };
        data[0][0] = 4.0f;
        auto a = m_scene.createArrayResource(2, data);

        const ramses_internal::ResourceContentHash hash = a->m_impl.getLowlevelResourceHash();

        vec3f data2[2] = { vec3f{1.f,2.f,3.f}, vec3f{3.f,4.f,5.f} };
        data2[0][0] = 44.0f;
        auto a2 = m_scene.createArrayResource(2, data2);

        const ramses_internal::ResourceContentHash hash2 = a2->m_impl.getLowlevelResourceHash();
        EXPECT_TRUE(hash != hash2);
    }

    TEST_F(AResourceTestClient, createEmptyVector3fArrayFails)
    {
        auto a = m_scene.createArrayResource<vec3f>(0, nullptr);
        EXPECT_TRUE(nullptr == a);
    }

    TEST_F(AResourceTestClient, createVector4fArray)
    {
        const vec4f data[2] = { vec4f{1.f,2.f,3.f,4.f}, vec4f{3.f,4.f,5.f,6.f} };
        auto a = m_scene.createArrayResource(2, data);
        ASSERT_TRUE(nullptr != a);
        EXPECT_EQ(a->getNumberOfElements(), 2u);
        EXPECT_EQ(a->getDataType(), EDataType::Vector4F);
    }

    TEST_F(AResourceTestClient, createVector4fArrayHashIsValid)
    {
        const vec4f data[2] = { vec4f{1.f,2.f,3.f,4.f}, vec4f{3.f,4.f,5.f,6.f} };
        auto a = m_scene.createArrayResource(2, data);
        ASSERT_TRUE(nullptr != a);

        const ramses_internal::ResourceContentHash hash = a->m_impl.getLowlevelResourceHash();
        EXPECT_TRUE(hash.isValid());
    }

    TEST_F(AResourceTestClient, createVector4fArrayHashIsUnique)
    {
        vec4f data[2] = { vec4f{1.f,2.f,3.f,4.f}, vec4f{3.f,4.f,5.f,6.f} };
        data[0][0] = 4.0f;
        auto a = m_scene.createArrayResource(2, data);

        const ramses_internal::ResourceContentHash hash = a->m_impl.getLowlevelResourceHash();

        vec4f data2[2] = { vec4f{1.f,2.f,3.f,4.f}, vec4f{3.f,4.f,5.f,6.f} };
        data2[0][0] = 42.0f;
        auto a2 = m_scene.createArrayResource(2, data2);

        const ramses_internal::ResourceContentHash hash2 = a2->m_impl.getLowlevelResourceHash();
        EXPECT_TRUE(hash != hash2);
    }

    TEST_F(AResourceTestClient, createEmptyVector4fArrayFails)
    {
        auto a = m_scene.createArrayResource<vec4f>(0, nullptr);
        EXPECT_TRUE(nullptr == a);
    }

    TEST_F(AResourceTestClient, createUInt16ArrayResource)
    {
        const uint16_t data[2] = {};
        const ArrayResource *a = m_scene.createArrayResource(2, data);
        ASSERT_TRUE(nullptr != a);
        EXPECT_EQ(a->getNumberOfElements(), 2u);
        EXPECT_EQ(a->getDataType(), EDataType::UInt16);
    }

    TEST_F(AResourceTestClient, createUInt16ArrayResourceHashIsValid)
    {
        const uint16_t data[2] = {};
        const ArrayResource* a = m_scene.createArrayResource(2, data);
        ASSERT_TRUE(nullptr != a);

        const ramses_internal::ResourceContentHash hash = a->m_impl.getLowlevelResourceHash();
        EXPECT_TRUE(hash.isValid());
    }

    TEST_F(AResourceTestClient, createUInt16ArrayHashIsUnique)
    {
        uint16_t data[2] = {};
        data[0] = 4;
        auto a = m_scene.createArrayResource(2, data);

        const ramses_internal::ResourceContentHash hash = a->m_impl.getLowlevelResourceHash();

        uint16_t data2[2] = {};
        data2[0] = 42;
        auto a2 = m_scene.createArrayResource(2, data2);

        const ramses_internal::ResourceContentHash hash2 = a2->m_impl.getLowlevelResourceHash();
        EXPECT_TRUE(hash != hash2);
    }

    TEST_F(AResourceTestClient, createEmptyUInt16ArrayFails)
    {
        auto a = m_scene.createArrayResource<uint16_t>(2, nullptr);
        EXPECT_TRUE(nullptr == a);
    }

    TEST_F(AResourceTestClient, createUInt32Array)
    {
        const uint32_t data[2] = {};
        auto a = m_scene.createArrayResource(2, data);
        ASSERT_TRUE(nullptr != a);
        EXPECT_EQ(a->getNumberOfElements(), 2u);
        EXPECT_EQ(a->getDataType(), EDataType::UInt32);
    }

    TEST_F(AResourceTestClient, createUInt32ArrayHashIsValid)
    {
        const uint32_t data[2] = {};
        auto a = m_scene.createArrayResource(2, data);
        ASSERT_TRUE(nullptr != a);

        const ramses_internal::ResourceContentHash hash = a->m_impl.getLowlevelResourceHash();
        EXPECT_TRUE(hash.isValid());
    }

    TEST_F(AResourceTestClient, createUInt32ArrayHashIsUnique)
    {
        uint32_t data[2] = {};
        data[0] = 4;
        auto a = m_scene.createArrayResource(2, data);

        const ramses_internal::ResourceContentHash hash = a->m_impl.getLowlevelResourceHash();

        uint32_t data2[2] = {};
        data2[0] = 42;
        auto a2 = m_scene.createArrayResource(2, data2);

        const ramses_internal::ResourceContentHash hash2 = a2->m_impl.getLowlevelResourceHash();
        EXPECT_TRUE(hash != hash2);
    }

    TEST_F(AResourceTestClient, createEmptyUInt32ArrayFails)
    {
        auto a = m_scene.createArrayResource<uint32_t>(0, nullptr);
        EXPECT_TRUE(nullptr == a);
    }


    TEST_F(AResourceTestClient, createByteBlobArray)
    {
        const ramses::Byte data[2] = {};
        const auto a = m_scene.createArrayResource(2, data);
        ASSERT_TRUE(nullptr != a);
        EXPECT_EQ(a->getNumberOfElements(), 2u);
        EXPECT_EQ(a->getDataType(), EDataType::ByteBlob);
    }

    TEST_F(AResourceTestClient, createByteBlobArrayHashIsValid)
    {
        const ramses::Byte data[2] = {};
        const auto a = m_scene.createArrayResource(2, data);
        ASSERT_TRUE(nullptr != a);

        const ramses_internal::ResourceContentHash hash = a->m_impl.getLowlevelResourceHash();
        EXPECT_TRUE(hash.isValid());
    }

    TEST_F(AResourceTestClient, createByteBlobArrayHashIsUnique)
    {
        ramses::Byte data[2] = {};
        data[0] = 4;
        auto a = m_scene.createArrayResource(sizeof(data), data);

        const ramses_internal::ResourceContentHash hash = a->m_impl.getLowlevelResourceHash();

        ramses::Byte data2[2] = {};
        data2[0] = 5;
        auto a2 = m_scene.createArrayResource(2, data2);

        const ramses_internal::ResourceContentHash hash2 = a2->m_impl.getLowlevelResourceHash();
        EXPECT_TRUE(hash != hash2);
    }

    TEST_F(AResourceTestClient, createEmptyByteBlobArrayFails)
    {
        auto a = m_scene.createArrayResource<ramses::Byte>(0, nullptr);
        EXPECT_TRUE(nullptr == a);
    }

    TEST_F(AResourceTestClient, createAndDestroyEffect)
    {
        EffectDescription effectDesc;
        EXPECT_EQ(StatusOK, effectDesc.setVertexShaderFromFile("res/ramses-client-test_shader.vert"));
        EXPECT_EQ(StatusOK, effectDesc.setFragmentShaderFromFile("res/ramses-client-test_minimalShader.frag"));

        auto effect = m_scene.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "effect");
        ASSERT_TRUE(effect != nullptr);
    }

    TEST_F(AResourceTestClient, effectCreatedTwiceWithSameHashCanBeCreatedAndDestroyed)
    {
        EffectDescription effectDesc;
        EXPECT_EQ(StatusOK, effectDesc.setVertexShaderFromFile("res/ramses-client-test_shader.vert"));
        EXPECT_EQ(StatusOK, effectDesc.setFragmentShaderFromFile("res/ramses-client-test_minimalShader.frag"));

        auto effect1 = m_scene.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "effect1");
        ASSERT_TRUE(effect1 != nullptr);
        auto effect2 = m_scene.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "effect2");
        ASSERT_TRUE(effect2 != nullptr);

        EXPECT_EQ(StatusOK, m_scene.destroy(*effect1));
        EXPECT_EQ(StatusOK, m_scene.destroy(*effect2));
    }

    TEST_F(AResourceTestClient, effectCreatedTwiceWithDifferentNameProducesSameHash)
    {
        EffectDescription effectDesc;
        EXPECT_EQ(StatusOK, effectDesc.setVertexShaderFromFile("res/ramses-client-test_shader.vert"));
        EXPECT_EQ(StatusOK, effectDesc.setFragmentShaderFromFile("res/ramses-client-test_minimalShader.frag"));
        auto effect1 = m_scene.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "effect1");
        ASSERT_TRUE(effect1 != nullptr);
        auto effect2 = m_scene.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "effect2");
        ASSERT_TRUE(effect2 != nullptr);
        EXPECT_EQ(effect1->m_impl.getLowlevelResourceHash(), effect2->m_impl.getLowlevelResourceHash());
        m_scene.destroy(*effect1);
        m_scene.destroy(*effect2);
    }

    template <typename ResourceType>
    class ResourceTest : public LocalTestClientWithScene, public testing::Test
    {
    public:
        ResourceType& createResource(std::string_view name)
        {
            return (this->template createObject<ResourceType>(name));
        }
    };

    TYPED_TEST_SUITE(ResourceTest, ResourceTypes);

    TYPED_TEST(ResourceTest, canBeConvertedToResource)
    {
        RamsesObject& obj = this->createResource("resource");
        EXPECT_TRUE(RamsesUtils::TryConvert<Resource>(obj) != nullptr);
        const RamsesObject& constObj = obj;
        EXPECT_TRUE(RamsesUtils::TryConvert<Resource>(constObj) != nullptr);
    }

    TYPED_TEST(ResourceTest, sameResourcesWithDifferentNamesShareSameHash)
    {
        const std::string name_A("name_A");
        const std::string name_B("name_B");
        Resource* resource_A = static_cast<Resource*>(&this->createResource(name_A));
        Resource* resource_B = static_cast<Resource*>(&this->createResource(name_B));

        RamsesObject* foundObject_A = this->getScene().findObjectByName(name_A);
        RamsesObject* foundObject_B = this->getScene().findObjectByName(name_B);

        ASSERT_TRUE(resource_A);
        ASSERT_TRUE(resource_B);
        EXPECT_EQ(resource_A->m_impl.getLowlevelResourceHash(), resource_B->m_impl.getLowlevelResourceHash());

        ASSERT_TRUE(foundObject_A);
        ASSERT_TRUE(foundObject_B);
        EXPECT_EQ(foundObject_A, static_cast<RamsesObject*>(resource_A));
        EXPECT_EQ(foundObject_B, static_cast<RamsesObject*>(resource_B));
        EXPECT_NE(resource_A, resource_B);
    }

    TYPED_TEST(ResourceTest, statisticCounterIsUpdated)
    {
        EXPECT_EQ(0u, this->getFramework().m_impl.getStatisticCollection().statResourcesCreated.getCounterValue());
        EXPECT_EQ(0u, this->getFramework().m_impl.getStatisticCollection().statResourcesDestroyed.getCounterValue());
        EXPECT_EQ(0u, this->getFramework().m_impl.getStatisticCollection().statResourcesNumber.getCounterValue());

        RamsesObject& obj = this->createResource("resource");
        Resource* res = RamsesUtils::TryConvert<Resource>(obj);
        EXPECT_EQ(1u, this->getFramework().m_impl.getStatisticCollection().statResourcesCreated.getCounterValue());

        this->getFramework().m_impl.getStatisticCollection().nextTimeInterval(); //statResourcesNumber is updated by nextTimeInterval()
        EXPECT_EQ(1u, this->getFramework().m_impl.getStatisticCollection().statResourcesNumber.getCounterValue());

        this->getScene().destroy(*res);
        EXPECT_EQ(1u, this->getFramework().m_impl.getStatisticCollection().statResourcesDestroyed.getCounterValue());

        this->getFramework().m_impl.getStatisticCollection().nextTimeInterval();
        EXPECT_EQ(0u, this->getFramework().m_impl.getStatisticCollection().statResourcesNumber.getCounterValue());
    }

    TYPED_TEST(ResourceTest, canCreateResourceSetNameAndAfterwardsDestroyResourceCleanly)
    {
        TypeParam& obj = this->createResource("resource");
        obj.setName("otherName");
        EXPECT_EQ(StatusOK, this->m_scene.destroy(obj));
        SceneObjectIterator iterator(this->m_scene, ERamsesObjectType::Resource);
        EXPECT_EQ(iterator.getNext(), nullptr);
        EXPECT_EQ(StatusOK, this->m_scene.saveToFile("someFileName.ramses", false));
    }

    TYPED_TEST(ResourceTest, canCreateSameResourceTwiceAndDeleteBothAgain)
    {
        TypeParam& obj1 = this->createResource("resource");
        TypeParam& obj2 = this->createResource("resource");
        EXPECT_EQ(StatusOK, this->m_scene.destroy(obj1));
        EXPECT_EQ(StatusOK, this->m_scene.destroy(obj2));
    }

    TYPED_TEST(ResourceTest, canFindResourceByItsNewIdAfterRenaming)
    {
        TypeParam& obj = this->createResource("resource");
        const auto id = obj.getResourceId();
        EXPECT_EQ(this->m_scene.getResource(id), &obj);
        obj.setName("otherName");
        EXPECT_EQ(this->m_scene.getResource(id), nullptr);
        EXPECT_EQ(this->m_scene.findObjectByName("otherName"), &obj);
        EXPECT_EQ(this->m_scene.getResource(obj.getResourceId()), &obj);
    }

    TYPED_TEST(ResourceTest, canGetResourceByIdAsLongAsAtLeastOneExists)
    {
        const auto id = this->createResource("resource").getResourceId();
        this->createResource("resource");
        const auto obj1 = this->m_scene.getResource(id);
        EXPECT_EQ(StatusOK, this->m_scene.destroy(*obj1));
        const auto obj2 = this->m_scene.getResource(id);
        EXPECT_NE(obj1, obj2);
        EXPECT_EQ(StatusOK, this->m_scene.destroy(*obj2));
        EXPECT_EQ(this->m_scene.getResource(id), nullptr);
    }
}
