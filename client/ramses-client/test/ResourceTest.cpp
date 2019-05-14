//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/FloatArray.h"
#include "ramses-client-api/Vector2fArray.h"
#include "ramses-client-api/Vector3fArray.h"
#include "ramses-client-api/Vector4fArray.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Texture3D.h"
#include "ramses-client-api/TextureCube.h"
#include "ramses-client-api/UInt16Array.h"
#include "ramses-client-api/UInt32Array.h"
#include "ramses-client-api/EffectDescription.h"
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
#include "ramses-client-api/ResourceFileDescription.h"

#include <thread>

namespace ramses
{
    using namespace testing;

    class AResourceTestClient : public LocalTestClient, public ::testing::Test
    {
    public:
        AResourceTestClient()
        {
            m_oldLogLevel = ramses_internal::CONTEXT_HLAPI_CLIENT.getLogLevel();
            ramses_internal::CONTEXT_HLAPI_CLIENT.setLogLevel(ramses_internal::ELogLevel::Trace);
        }
        ~AResourceTestClient() {
            ramses_internal::CONTEXT_HLAPI_CLIENT.setLogLevel(m_oldLogLevel);
        }
        ramses_internal::ManagedResource getCreatedResource(const ramses_internal::ResourceContentHash& hash)
        {
            return client.impl.getClientApplication().getResource(hash);
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
        Texture2D* texture = client.createTexture2D(10, 12, ramses::ETextureFormat_RGBA8, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(NULL != texture);
        EXPECT_EQ(StatusOK, client.destroy(*texture));
    }

    TEST_F(AResourceTestClient, createTextureWithMipMaps)
    {
        const uint8_t data[2 * 2 * 4] = {};
        std::vector<MipLevelData> mipLevelData;
        mipLevelData.push_back(MipLevelData(2 * 2 * 4, data));
        mipLevelData.push_back(MipLevelData(1 * 1 * 4, data));
        Texture2D* texture = client.createTexture2D(2, 2, ramses::ETextureFormat_RGBA8, 2, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(NULL != texture);
    }

    TEST_F(AResourceTestClient, createTextureAndCheckWidthHeight)
    {
        const uint8_t data[4 * 10 * 12] = {};
        MipLevelData mipLevelData(sizeof(data), data);
        Texture2D* texture = client.createTexture2D(10, 12, ramses::ETextureFormat_RGBA8, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(NULL != texture);
        EXPECT_EQ(10u, texture->getWidth());
        EXPECT_EQ(12u, texture->getHeight());
    }

    TEST_F(AResourceTestClient, createTextureAndCheckFormat)
    {
        const uint8_t data[3 * 10 * 12] = {};
        MipLevelData mipLevelData(sizeof(data), data);
        Texture2D* texture = client.createTexture2D(10, 12, ramses::ETextureFormat_RGB8, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(NULL != texture);
        EXPECT_EQ(ETextureFormat_RGB8, texture->getTextureFormat());
    }

    TEST_F(AResourceTestClient, createTextureWithProvidedMipsButNotFullChain)
    {
        const uint8_t data[4 * 4 * 4] = {};

        std::vector<MipLevelData> mipLevelData;
        mipLevelData.push_back(MipLevelData(sizeof(data), data));
        mipLevelData.push_back(MipLevelData(2 * 2 * 4, data));

        Texture2D* texture = client.createTexture2D(4u, 4u, ramses::ETextureFormat_RGBA8, 2, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(NULL != texture);
    }

    TEST_F(AResourceTestClient, createTextureWithMoreMipsThanExpected)
    {
        const uint8_t data[1] = {};

        std::vector<MipLevelData> mipLevelData;
        mipLevelData.push_back(MipLevelData(sizeof(data), data));
        mipLevelData.push_back(MipLevelData(sizeof(data), data));

        Texture2D* texture = client.createTexture2D(1u, 1u, ramses::ETextureFormat_RGBA8, 2, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(NULL, texture);
    }

    TEST_F(AResourceTestClient, createTextureWithNullMipMapData)
    {
        const uint8_t data[4 * 4 * 4] = {};

        std::vector<MipLevelData> mipLevelData;
        mipLevelData.push_back(MipLevelData(sizeof(data), nullptr));

        Texture2D* texture = client.createTexture2D(4u, 4u, ramses::ETextureFormat_RGBA8, 1, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(NULL, texture);
    }

    TEST_F(AResourceTestClient, createTextureWithZeroSizeMipMapData)
    {
        const uint8_t data[4 * 4 * 4] = {};

        std::vector<MipLevelData> mipLevelData;
        mipLevelData.push_back(MipLevelData(0u, data));

        Texture2D* texture = client.createTexture2D(4u, 4u, ramses::ETextureFormat_RGBA8, 1, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(NULL, texture);
    }

    TEST_F(AResourceTestClient, createTextureWithNullOrZeroSizeLowerMipMapData)
    {
        const uint8_t data[2 * 2 * 4] = {};
        {
            std::vector<MipLevelData> mipLevelData;
            mipLevelData.push_back(MipLevelData(sizeof(data), data));
            mipLevelData.push_back(MipLevelData(0u, data));
            Texture2D* texture = client.createTexture2D(2u, 2u, ramses::ETextureFormat_RGBA8, 2, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
            EXPECT_EQ(NULL, texture);
        }
        {
            std::vector<MipLevelData> mipLevelData;
            mipLevelData.push_back(MipLevelData(sizeof(data), data));
            mipLevelData.push_back(MipLevelData(sizeof(data), nullptr));
            Texture2D* texture = client.createTexture2D(2u, 2u, ramses::ETextureFormat_RGBA8, 2, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
            EXPECT_EQ(NULL, texture);
        }
    }

    TEST_F(AResourceTestClient, createTextureWithWrongSizeOfLowerMipMapData)
    {
        const uint8_t data[2 * 2 * 4] = {};
        std::vector<MipLevelData> mipLevelData;
        mipLevelData.push_back(MipLevelData(2 * 2 * 4, data));
        mipLevelData.push_back(MipLevelData(1 * 1 * 2, data));
        Texture2D* texture = client.createTexture2D(2u, 2u, ramses::ETextureFormat_RGBA8, 2, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(NULL, texture);
    }

    TEST_F(AResourceTestClient, createTextureOfZeroSize)
    {
        const uint8_t data[4 * 10 * 12] = {};
        MipLevelData mipLevelData(sizeof(data), data);
        Texture2D* texture = client.createTexture2D(0, 0, ramses::ETextureFormat_RGBA8, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(NULL, texture);
    }

    TEST_F(AResourceTestClient, createTextureWithNoMipData)
    {
        Texture2D* texture = client.createTexture2D(1, 1, ramses::ETextureFormat_RGBA8, 1, nullptr, false, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(NULL, texture);
    }

    TEST_F(AResourceTestClient, createTextureWithoutName)
    {
        const uint8_t data[4 * 10 * 12] = {};
        MipLevelData mipLevelData(sizeof(data), data);
        Texture2D* texture = client.createTexture2D(10, 12, ramses::ETextureFormat_RGBA8, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, NULL);
        ASSERT_TRUE(NULL != texture);
        EXPECT_STREQ("", texture->getName());
    }

    TEST_F(AResourceTestClient, createTextureCheckHashIsValid)
    {
        const uint8_t data[4 * 10 * 12] = {};
        MipLevelData mipLevelData(sizeof(data), data);
        Texture2D* texture = client.createTexture2D(10, 12, ramses::ETextureFormat_RGBA8, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(NULL != texture);

        const ramses_internal::ResourceContentHash hash = texture->impl.getLowlevelResourceHash();
        EXPECT_TRUE(hash.isValid());
    }

    TEST_F(AResourceTestClient, createTextureCheckHashIsUnique)
    {
        uint8_t data[4 * 10 * 12] = {};
        data[20] = 48;
        MipLevelData mipLevelData(sizeof(data), data);
        Texture2D* texture = client.createTexture2D(10, 12, ramses::ETextureFormat_RGBA8, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(NULL != texture);

        const ramses_internal::ResourceContentHash hash = texture->impl.getLowlevelResourceHash();
        uint8_t data2[4 * 10 * 12] = {};
        data[20] = 42;
        MipLevelData mipLevelData2(sizeof(data2), data2);
        Texture2D* texture2 = client.createTexture2D(10, 12, ramses::ETextureFormat_RGBA8, 1, &mipLevelData2, false, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(NULL != texture2);

        const ramses_internal::ResourceContentHash hash2 = texture2->impl.getLowlevelResourceHash();
        EXPECT_TRUE(hash != hash2);
    }

    TEST_F(AResourceTestClient, createTextureRGBA_AndCheckTexels)
    {
        const uint8_t data[1 * 2 * 4] = { 1, 2, 3, 4, 5, 6, 7, 8 };
        MipLevelData mipLevelData(sizeof(data), data);
        Texture2D* texture = client.createTexture2D(2, 1, ramses::ETextureFormat_RGBA8, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, NULL);
        ASSERT_TRUE(NULL != texture);
        ramses_internal::ManagedResource res = getCreatedResource(texture->impl.getLowlevelResourceHash());

        ASSERT_EQ(sizeof(data), res.getResourceObject()->getDecompressedDataSize());
        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(data, res.getResourceObject()->getResourceData().get()->getRawData(), sizeof(data)));
    }

    TEST_F(AResourceTestClient, createTextureRGB_AndCheckTexels)
    {
        const uint8_t data[1 * 2 * 3] = { 1, 2, 3, 4, 5, 6 };
        MipLevelData mipLevelData(sizeof(data), data);
        Texture2D* texture = client.createTexture2D(2, 1, ramses::ETextureFormat_RGB8, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, NULL);
        ASSERT_TRUE(NULL != texture);

        ramses_internal::ManagedResource res = getCreatedResource(texture->impl.getLowlevelResourceHash());

        ASSERT_EQ(sizeof(data), res.getResourceObject()->getDecompressedDataSize());
        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(data, res.getResourceObject()->getResourceData().get()->getRawData(), sizeof(data)));
    }

    TEST_F(AResourceTestClient, createTextureRGBWithMips_AndCheckTexels)
    {
        const uint8_t data0[2 * 2 * 3] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
        const uint8_t data1[1 * 1 * 3] = { 13, 14, 15 };
        const MipLevelData mipLevelData[2] = { { sizeof(data0), data0 },{ sizeof(data1), data1 } };
        const Texture2D* texture = client.createTexture2D(2, 2, ramses::ETextureFormat_RGB8, 2, mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, NULL);
        ASSERT_TRUE(NULL != texture);

        const ramses_internal::ManagedResource res = getCreatedResource(texture->impl.getLowlevelResourceHash());

        ASSERT_EQ(sizeof(data0) + sizeof(data1), res.getResourceObject()->getDecompressedDataSize());
        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(data0, res.getResourceObject()->getResourceData().get()->getRawData(), sizeof(data0)));
        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(data1, res.getResourceObject()->getResourceData().get()->getRawData() + sizeof(data0), sizeof(data1)));
    }

    //##############################################################
    //##############    Cube Texture tests #########################
    //##############################################################

    TEST_F(AResourceTestClient, createCubeTextureAndDestroyManually)
    {
        const uint8_t data[4 * 10 * 10] = {};
        CubeMipLevelData mipLevelData(sizeof(data), data, data, data, data, data, data);
        TextureCube* texture = client.createTextureCube(10, ramses::ETextureFormat_RGBA8, 1, &mipLevelData, false);
        ASSERT_TRUE(NULL != texture);
        EXPECT_EQ(StatusOK, client.destroy(*texture));
    }

    TEST_F(AResourceTestClient, createCubeTextureWithMipMaps)
    {
        const uint8_t data[2 * 2 * 4] = {};
        std::vector<CubeMipLevelData> mipLevelData;
        mipLevelData.push_back(CubeMipLevelData(2 * 2 * 4, data, data, data, data, data, data));
        mipLevelData.push_back(CubeMipLevelData(1 * 1 * 4, data, data, data, data, data, data));
        TextureCube* texture = client.createTextureCube(2, ramses::ETextureFormat_RGBA8, 2, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(NULL != texture);
    }

    TEST_F(AResourceTestClient, createCubeTextureAndCheckWidthHeight)
    {
        const uint8_t data[4 * 10 * 10] = {};
        CubeMipLevelData mipLevelData(sizeof(data), data, data, data, data, data, data);
        TextureCube* texture = client.createTextureCube(10, ramses::ETextureFormat_RGBA8, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(NULL != texture);
        EXPECT_EQ(10u, texture->getSize());
    }

    TEST_F(AResourceTestClient, createCubeTextureAndCheckFormat)
    {
        const uint8_t data[3 * 10 * 10] = {};
        CubeMipLevelData mipLevelData(sizeof(data), data, data, data, data, data, data);
        TextureCube* texture = client.createTextureCube(10, ramses::ETextureFormat_RGB8, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(NULL != texture);
        EXPECT_EQ(ETextureFormat_RGB8, texture->getTextureFormat());
    }

    TEST_F(AResourceTestClient, createCubeTextureOfZeroSize)
    {
        const uint8_t data[4 * 10 * 10] = {};
        CubeMipLevelData mipLevelData(sizeof(data), data, data, data, data, data, data);
        TextureCube* texture = client.createTextureCube(0, ramses::ETextureFormat_RGBA8, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(NULL, texture);
    }

    TEST_F(AResourceTestClient, createCubeTextureOfZeroDataSize)
    {
        const uint8_t data[4 * 10 * 10] = {};
        CubeMipLevelData mipLevelData(0, data, data, data, data, data, data);
        TextureCube* texture = client.createTextureCube(10, ramses::ETextureFormat_RGBA8, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(NULL, texture);
    }

    TEST_F(AResourceTestClient, createCubeTextureWithNoMipData)
    {
        TextureCube* texture = client.createTextureCube(10, ramses::ETextureFormat_RGBA8, 1, nullptr, false, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(NULL, texture);
    }

    TEST_F(AResourceTestClient, createCubeTextureWithoutName)
    {
        const uint8_t data[4 * 10 * 10] = {};
        CubeMipLevelData mipLevelData(sizeof(data), data, data, data, data, data, data);
        TextureCube* texture = client.createTextureCube(10, ramses::ETextureFormat_RGBA8, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, NULL);
        ASSERT_TRUE(NULL != texture);
        EXPECT_STREQ("", texture->getName());
    }

    TEST_F(AResourceTestClient, createCubeTextureRGBA_AndCheckTexels)
    {
        const uint8_t data[2 * 2 * 4] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
        CubeMipLevelData mipLevelData(sizeof(data), data, data, data, data, data, data);
        TextureCube* texture = client.createTextureCube(2, ramses::ETextureFormat_RGBA8, 1, &mipLevelData, false);
        ASSERT_TRUE(NULL != texture);
        ramses_internal::ManagedResource res = getCreatedResource(texture->impl.getLowlevelResourceHash());

        ASSERT_EQ(sizeof(data) * 6u, res.getResourceObject()->getDecompressedDataSize());
        auto resData = res.getResourceObject()->getResourceData().get()->getRawData();
        for (uint32_t i = 0u; i < 6u; ++i)
        {
            EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(data, resData, sizeof(data)));
            resData += sizeof(data);
        }
    }

    TEST_F(AResourceTestClient, createCubeTextureRGB_AndCheckTexels)
    {
        const uint8_t data[2 * 2 * 3] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
        CubeMipLevelData mipLevelData(sizeof(data), data, data, data, data, data, data);
        TextureCube* texture = client.createTextureCube(2, ramses::ETextureFormat_RGB8, 1, &mipLevelData, false);
        ASSERT_TRUE(NULL != texture);
        ramses_internal::ManagedResource res = getCreatedResource(texture->impl.getLowlevelResourceHash());

        ASSERT_EQ(sizeof(data) * 6u, res.getResourceObject()->getDecompressedDataSize());
        auto resData = res.getResourceObject()->getResourceData().get()->getRawData();
        for (uint32_t i = 0u; i < 6u; ++i)
        {
            EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(data, resData, sizeof(data)));
            resData += sizeof(data);
        }
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
        const TextureCube* texture = client.createTextureCube(2u, ramses::ETextureFormat_RGB8, 2, mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, NULL);
        ASSERT_TRUE(NULL != texture);

        const ramses_internal::ManagedResource res = getCreatedResource(texture->impl.getLowlevelResourceHash());

        ASSERT_EQ(6u * (sizeof(data0px) + sizeof(data1px)), res.getResourceObject()->getDecompressedDataSize());

        // mips are bundled together per face:
        // - facePX mip0, facePX mip1 .. face PX mipN, face NX mips, face PY .. face NZ
        auto resData = res.getResourceObject()->getResourceData().get()->getRawData();
        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(data0px, resData, sizeof(data0px)));
        resData += sizeof(data0px);
        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(data1px, resData, sizeof(data1px)));
        resData += sizeof(data1px);

        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(data0nx, resData, sizeof(data0nx)));
        resData += sizeof(data0nx);
        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(data1nx, resData, sizeof(data1nx)));
        resData += sizeof(data1nx);

        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(data0py, resData, sizeof(data0py)));
        resData += sizeof(data0py);
        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(data1py, resData, sizeof(data1py)));
        resData += sizeof(data1py);

        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(data0ny, resData, sizeof(data0ny)));
        resData += sizeof(data0ny);
        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(data1ny, resData, sizeof(data1ny)));
        resData += sizeof(data1ny);

        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(data0pz, resData, sizeof(data0pz)));
        resData += sizeof(data0pz);
        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(data1pz, resData, sizeof(data1pz)));
        resData += sizeof(data1pz);

        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(data0nz, resData, sizeof(data0nz)));
        resData += sizeof(data0nz);
        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(data1nz, resData, sizeof(data1nz)));
    }

    TEST_F(AResourceTestClient, createTextureCubeWithProvidedMipsButNotFullChain)
    {
        const uint8_t data[4 * 4 * 4] = {};

        std::vector<CubeMipLevelData> mipLevelData;
        mipLevelData.push_back(CubeMipLevelData(sizeof(data), data, data, data, data, data, data));
        mipLevelData.push_back(CubeMipLevelData(2 * 2 * 4, data, data, data, data, data, data));

        TextureCube* texture = client.createTextureCube(4u, ramses::ETextureFormat_RGBA8, 2, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_NE(nullptr, texture);
    }

    TEST_F(AResourceTestClient, createTextureCubeWithMoreMipsThanExpected)
    {
        const uint8_t data[1] = {};

        std::vector<CubeMipLevelData> mipLevelData;
        mipLevelData.push_back(CubeMipLevelData(sizeof(data), data, data, data, data, data, data));
        mipLevelData.push_back(CubeMipLevelData(sizeof(data), data, data, data, data, data, data));

        TextureCube* texture = client.createTextureCube(1u, ramses::ETextureFormat_RGBA8, 2, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(NULL, texture);
    }

    TEST_F(AResourceTestClient, createTextureCubeWithNullMipMapData)
    {
        const uint8_t data[4 * 4 * 4] = {};

        std::vector<CubeMipLevelData> mipLevelData;
        mipLevelData.push_back(CubeMipLevelData(sizeof(data), data, data, nullptr, data, data, data));

        TextureCube* texture = client.createTextureCube(4u, ramses::ETextureFormat_RGBA8, 1, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(NULL, texture);
    }

    TEST_F(AResourceTestClient, createTextureCubeWithZeroSizeMipMapData)
    {
        const uint8_t data[4 * 4 * 4] = {};

        std::vector<CubeMipLevelData> mipLevelData;
        mipLevelData.push_back(CubeMipLevelData(0u, data, data, data, data, data, data));

        TextureCube* texture = client.createTextureCube(4u, ramses::ETextureFormat_RGBA8, 1, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(NULL, texture);
    }

    TEST_F(AResourceTestClient, createTextureCubeWithNullOrZeroSizeLowerMipMapData)
    {
        const uint8_t data[2 * 2 * 4] = {};
        {
            std::vector<CubeMipLevelData> mipLevelData;
            mipLevelData.push_back(CubeMipLevelData(sizeof(data), data, data, data, data, data, data));
            mipLevelData.push_back(CubeMipLevelData(0u, data, data, data, data, data, data));
            TextureCube* texture = client.createTextureCube(2u, ramses::ETextureFormat_RGBA8, 2, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
            EXPECT_EQ(NULL, texture);
        }
        {
            std::vector<CubeMipLevelData> mipLevelData;
            mipLevelData.push_back(CubeMipLevelData(sizeof(data), data, data, data, data, data, data));
            mipLevelData.push_back(CubeMipLevelData(sizeof(data), data, data, data, data, nullptr, data));
            TextureCube* texture = client.createTextureCube(2u, ramses::ETextureFormat_RGBA8, 2, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
            EXPECT_EQ(NULL, texture);
        }
    }

    TEST_F(AResourceTestClient, createTextureCubeWithWrongSizeOfLowerMipMapData)
    {
        const uint8_t data[2 * 2 * 4] = {};
        std::vector<CubeMipLevelData> mipLevelData;
        mipLevelData.push_back(CubeMipLevelData(2 * 2 * 4, data, data, data, data, data, data));
        mipLevelData.push_back(CubeMipLevelData(1 * 1 * 2, data, data, data, data, data, data));
        TextureCube* texture = client.createTextureCube(2u, ramses::ETextureFormat_RGBA8, 2, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(NULL, texture);
    }

    //##############################################################
    //##############    3D texture tests ###########################
    //##############################################################

    TEST_F(AResourceTestClient, create3DTextureAndDestroyManually)
    {
        const uint8_t data[4 * 10 * 12 * 14] = {};
        MipLevelData mipLevelData(sizeof(data), data);
        Texture3D* texture = client.createTexture3D(10, 12, 14, ramses::ETextureFormat_RGBA8, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(NULL != texture);
        EXPECT_EQ(StatusOK, client.destroy(*texture));
    }

    TEST_F(AResourceTestClient, create3DTextureWithMipMaps)
    {
        const uint8_t data[2 * 2 * 2 * 4] = {};
        std::vector<MipLevelData> mipLevelData;
        mipLevelData.push_back(MipLevelData(2 * 2 * 2 * 4, data));
        mipLevelData.push_back(MipLevelData(1 * 1 * 1 * 4, data));
        Texture3D* texture = client.createTexture3D(2u, 2u, 2u, ramses::ETextureFormat_RGBA8, 2, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(NULL != texture);
    }

    TEST_F(AResourceTestClient, create3DTextureAndCheckWidthHeightDepth)
    {
        const uint8_t data[4 * 10 * 12 * 14] = {};
        MipLevelData mipLevelData(sizeof(data), data);
        Texture3D* texture = client.createTexture3D(10, 12, 14, ramses::ETextureFormat_RGBA8, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(NULL != texture);
        EXPECT_EQ(10u, texture->getWidth());
        EXPECT_EQ(12u, texture->getHeight());
        EXPECT_EQ(14u, texture->getDepth());
    }

    TEST_F(AResourceTestClient, create3DTextureAndCheckFormat)
    {
        const uint8_t data[3 * 10 * 12 * 14] = {};
        MipLevelData mipLevelData(sizeof(data), data);
        Texture3D* texture = client.createTexture3D(10, 12, 14, ramses::ETextureFormat_RGB8, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(NULL != texture);
        EXPECT_EQ(ETextureFormat_RGB8, texture->getTextureFormat());
    }

    TEST_F(AResourceTestClient, create3DTextureOfZeroSize)
    {
        const uint8_t data[4 * 10 * 12 * 14] = {};
        MipLevelData mipLevelData(sizeof(data), data);
        Texture3D* texture = client.createTexture3D(0, 0, 0, ramses::ETextureFormat_RGBA8, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(NULL, texture);
    }

    TEST_F(AResourceTestClient, create3DTextureWithNoMipData)
    {
        Texture3D* texture = client.createTexture3D(1, 1, 1, ramses::ETextureFormat_RGBA8, 1, nullptr, false, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(NULL, texture);
    }

    TEST_F(AResourceTestClient, create3DTextureWithoutName)
    {
        const uint8_t data[4 * 10 * 12 * 14] = {};
        MipLevelData mipLevelData(sizeof(data), data);
        Texture3D* texture = client.createTexture3D(10, 12, 14, ramses::ETextureFormat_RGBA8, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, NULL);
        ASSERT_TRUE(NULL != texture);
        EXPECT_STREQ("", texture->getName());
    }

    TEST_F(AResourceTestClient, create3DTextureCheckHashIsValid)
    {
        const uint8_t data[4 * 10 * 12 * 14] = {};
        MipLevelData mipLevelData(sizeof(data), data);
        Texture3D* texture = client.createTexture3D(10, 12, 14, ramses::ETextureFormat_RGBA8, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(NULL != texture);

        const ramses_internal::ResourceContentHash hash = texture->impl.getLowlevelResourceHash();
        EXPECT_TRUE(hash.isValid());
    }

    TEST_F(AResourceTestClient, create3DTextureCheckHashIsUnique)
    {
        uint8_t data[4 * 10 * 12 * 14] = {};
        data[20] = 48;
        MipLevelData mipLevelData1(sizeof(data), data);
        Texture3D* texture = client.createTexture3D(10, 12, 14, ramses::ETextureFormat_RGBA8, 1, &mipLevelData1, false, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(NULL != texture);

        const ramses_internal::ResourceContentHash hash = texture->impl.getLowlevelResourceHash();
        uint8_t data2[4 * 10 * 12 * 14] = {};
        data[20] = 42;
        MipLevelData mipLevelData2(sizeof(data2), data2);
        Texture3D* texture2 = client.createTexture3D(10, 12, 14, ramses::ETextureFormat_RGBA8, 1, &mipLevelData2, false, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(NULL != texture2);

        const ramses_internal::ResourceContentHash hash2 = texture2->impl.getLowlevelResourceHash();
        EXPECT_TRUE(hash != hash2);
    }

    TEST_F(AResourceTestClient, create3DTextureRGBA_AndCheckTexels)
    {
        const uint8_t data[1 * 2 * 2 * 4] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
        MipLevelData mipLevelData(sizeof(data), data);
        Texture3D* texture = client.createTexture3D(2, 1, 2, ramses::ETextureFormat_RGBA8, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, NULL);

        ASSERT_TRUE(NULL != texture);
        ramses_internal::ManagedResource res = getCreatedResource(texture->impl.getLowlevelResourceHash());

        ASSERT_EQ(sizeof(data), res.getResourceObject()->getDecompressedDataSize());
        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(data, res.getResourceObject()->getResourceData().get()->getRawData(), sizeof(data)));
    }

    TEST_F(AResourceTestClient, create3DTextureRGB_AndCheckTexels)
    {
        const uint8_t data[1 * 2 * 2 * 3] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
        MipLevelData mipLevelData(sizeof(data), data);
        Texture3D* texture = client.createTexture3D(2, 1, 2, ramses::ETextureFormat_RGB8, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, NULL);

        ASSERT_TRUE(NULL != texture);
        ramses_internal::ManagedResource res = getCreatedResource(texture->impl.getLowlevelResourceHash());

        ASSERT_EQ(sizeof(data), res.getResourceObject()->getDecompressedDataSize());
        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(data, res.getResourceObject()->getResourceData().get()->getRawData(), sizeof(data)));
    }

    TEST_F(AResourceTestClient, create3DTextureRGBWithMips_AndCheckTexels)
    {
        const uint8_t data0[2 * 2 * 2 * 3] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120 };
        const uint8_t data1[1 * 1 * 3] = { 13, 14, 15 };
        const MipLevelData mipLevelData[2] = { { sizeof(data0), data0 },{ sizeof(data1), data1 } };
        const Texture3D* texture = client.createTexture3D(2u, 2u, 2u, ramses::ETextureFormat_RGB8, 2, mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, NULL);
        ASSERT_TRUE(NULL != texture);

        const ramses_internal::ManagedResource res = getCreatedResource(texture->impl.getLowlevelResourceHash());

        ASSERT_EQ(sizeof(data0) + sizeof(data1), res.getResourceObject()->getDecompressedDataSize());
        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(data0, res.getResourceObject()->getResourceData().get()->getRawData(), sizeof(data0)));
        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(data1, res.getResourceObject()->getResourceData().get()->getRawData() + sizeof(data0), sizeof(data1)));
    }

    TEST_F(AResourceTestClient, createTexture3DWithProvidedMipsButNotFullChain)
    {
        const uint8_t data[4 * 4 * 4 * 4] = {};

        std::vector<MipLevelData> mipLevelData;
        mipLevelData.push_back(MipLevelData(sizeof(data), data));
        mipLevelData.push_back(MipLevelData(2 * 2 * 2 * 4, data));

        Texture3D* texture = client.createTexture3D(4u, 4u, 4u, ramses::ETextureFormat_RGBA8, 2, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_NE(nullptr, texture);
    }

    TEST_F(AResourceTestClient, createTexture3DWithMoreMipsThanExpected)
    {
        const uint8_t data[1] = {};

        std::vector<MipLevelData> mipLevelData;
        mipLevelData.push_back(MipLevelData(sizeof(data), data));
        mipLevelData.push_back(MipLevelData(sizeof(data), data));

        Texture3D* texture = client.createTexture3D(1u, 1u, 1u, ramses::ETextureFormat_RGBA8, 2, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(NULL, texture);
    }

    TEST_F(AResourceTestClient, create3DTextureWithNullMipMapData)
    {
        const uint8_t data[2 * 2 * 2 * 4] = {};

        std::vector<MipLevelData> mipLevelData;
        mipLevelData.push_back(MipLevelData(sizeof(data), nullptr));

        Texture3D* texture = client.createTexture3D(2u, 2u, 2u, ramses::ETextureFormat_RGBA8, 1, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(NULL, texture);
    }

    TEST_F(AResourceTestClient, create3DTextureWithZeroSizeMipMapData)
    {
        const uint8_t data[2 * 2 * 2 * 4] = {};

        std::vector<MipLevelData> mipLevelData;
        mipLevelData.push_back(MipLevelData(0, data));

        Texture3D* texture = client.createTexture3D(2u, 2u, 2u, ramses::ETextureFormat_RGBA8, 1, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(NULL, texture);
    }

    TEST_F(AResourceTestClient, create3DTextureWithNullOrZeroSizeLowerMipMapData)
    {
        const uint8_t data[2 * 2 * 2 * 4] = {};
        {
            std::vector<MipLevelData> mipLevelData;
            mipLevelData.push_back(MipLevelData(sizeof(data), data));
            mipLevelData.push_back(MipLevelData(0u, data));
            Texture3D* texture = client.createTexture3D(2u, 2u, 2u, ramses::ETextureFormat_RGBA8, 2, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
            EXPECT_EQ(NULL, texture);
        }
        {
            std::vector<MipLevelData> mipLevelData;
            mipLevelData.push_back(MipLevelData(sizeof(data), data));
            mipLevelData.push_back(MipLevelData(sizeof(data), nullptr));
            Texture3D* texture = client.createTexture3D(2u, 2u, 2u, ramses::ETextureFormat_RGBA8, 2, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
            EXPECT_EQ(NULL, texture);
        }
    }

    TEST_F(AResourceTestClient, create3DTextureWithWrongSizeOfLowerMipMapData)
    {
        const uint8_t data[2 * 2 * 2 * 4] = {};
        std::vector<MipLevelData> mipLevelData;
        mipLevelData.push_back(MipLevelData(2 * 2 * 2 * 4, data));
        mipLevelData.push_back(MipLevelData(1 * 1 * 1 * 2, data));
        Texture3D* texture = client.createTexture3D(2u, 2u, 2u, ramses::ETextureFormat_RGBA8, 2, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_EQ(NULL, texture);
    }

    TEST_F(AResourceTestClient, create3DTextureWithCompressedFormatCanBeCreatedWithArbitraryNonZeroDataSize)
    {
        const uint8_t data[2 * 2 * 2 * 4] = {};
        std::vector<MipLevelData> mipLevelData;
        mipLevelData.push_back(MipLevelData(1, data));
        mipLevelData.push_back(MipLevelData(1, data));
        Texture3D* texture = client.createTexture3D(2u, 2u, 2u, ramses::ETextureFormat_ASTC_RGBA_4x4, 2, &mipLevelData[0], false, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_TRUE(nullptr != texture);
    }

    //##############################################################
    //##############    Array resource tests #######################
    //##############################################################

    TEST_F(AResourceTestClient, creatingResourcesWithSameDataInternallyRefersToSameResourceHash)
    {
        const float data[2 * 2] = {1,2,3,4};
        const Vector2fArray *a = client.createConstVector2fArray(2, data);
        const Vector2fArray *b = client.createConstVector2fArray(2, data);

        ASSERT_EQ(a->impl.getLowlevelResourceHash(), b->impl.getLowlevelResourceHash());
    }

    TEST_F(AResourceTestClient, creatingResourcesWithSameDataInternallySharesData)
    {
        const float data[2 * 2] = { 1,2,3,4 };
        const Vector2fArray *a = client.createConstVector2fArray(2, data);
        const Vector2fArray *b = client.createConstVector2fArray(2, data);
        ramses_internal::ManagedResource resourceA = client.impl.getResource(a->impl.getLowlevelResourceHash());
        ramses_internal::ManagedResource resourceB = client.impl.getResource(b->impl.getLowlevelResourceHash());
        ASSERT_EQ(resourceA.getResourceObject(), resourceB.getResourceObject());
    }

    TEST_F(AResourceTestClient, creatingResourcesWithSameDataContentInternallySharesData)
    {
        const float data[2 * 2] = { 1,2,3,4 };
        const float data2[2 * 2] = { 1,2,3,4 };
        const Vector2fArray *a = client.createConstVector2fArray(2, data);
        const Vector2fArray *b = client.createConstVector2fArray(2, data2);
        ramses_internal::ManagedResource resourceA = client.impl.getResource(a->impl.getLowlevelResourceHash());
        ramses_internal::ManagedResource resourceB = client.impl.getResource(b->impl.getLowlevelResourceHash());
        ASSERT_EQ(resourceA.getResourceObject(), resourceB.getResourceObject());
    }

    TEST_F(AResourceTestClient, destroyingDuplicateResourceDoesNotDeleteData)
    {
        const float data[2 * 2] = { 1, 2, 3, 4 };
        const float data2[2 * 2] = { 1, 2, 3, 4 };
        const Vector2fArray *a = client.createConstVector2fArray(2, data);
        const Vector2fArray *b = client.createConstVector2fArray(2, data2);

        client.destroy(*b);
        ramses_internal::ManagedResource aRes = client.impl.getResource(a->impl.getLowlevelResourceHash());
        void* rawResourceData = aRes.getResourceObject()->getResourceData().get()->getRawData();
        ASSERT_EQ(0, ramses_internal::PlatformMemory::Compare(data, rawResourceData, sizeof(float)* 4));
    }

    TEST_F(AResourceTestClient, createVector2fArray)
    {
        const float data[2 * 2] = {};
        const Vector2fArray *a = client.createConstVector2fArray(2, data);
        ASSERT_TRUE(NULL != a);
    }

    TEST_F(AResourceTestClient, createVector2fArrayHashIsValid)
    {
        const float data[2 * 2] = {};
        const Vector2fArray *a = client.createConstVector2fArray(2, data);
        ASSERT_TRUE(NULL != a);

        const ramses_internal::ResourceContentHash hash = a->impl.getLowlevelResourceHash();
        EXPECT_TRUE(hash.isValid());
    }

    TEST_F(AResourceTestClient, createVector2fArrayHashIsUnique)
    {
        float data[2 * 2] = {};
        data[0] = 4;
        const Vector2fArray *a = client.createConstVector2fArray(2, data);

        const ramses_internal::ResourceContentHash hash = a->impl.getLowlevelResourceHash();

        float data2[2 * 2] = {};
        data2[0] = 42.0f;
        const Vector2fArray *a2 = client.createConstVector2fArray(2, data2);

        const ramses_internal::ResourceContentHash hash2 = a2->impl.getLowlevelResourceHash();
        EXPECT_TRUE(hash != hash2);
    }

    TEST_F(AResourceTestClient, createEmptyVector2fArrayFails)
    {
        const Vector2fArray *a = client.createConstVector2fArray(0, NULL);
        EXPECT_TRUE(NULL == a);
    }

    TEST_F(AResourceTestClient, createVector3fArray)
    {
        const float data[2 * 3] = {};
        const Vector3fArray *a = client.createConstVector3fArray(2, data);
        ASSERT_TRUE(NULL != a);
    }

    TEST_F(AResourceTestClient, createVector3fArrayHashIsValid)
    {
        const float data[2 * 3] = {};
        const Vector3fArray *a = client.createConstVector3fArray(2, data);
        ASSERT_TRUE(NULL != a);

        const ramses_internal::ResourceContentHash hash = a->impl.getLowlevelResourceHash();
        EXPECT_TRUE(hash.isValid());
    }

    TEST_F(AResourceTestClient, createVector3fArrayHashIsUnique)
    {
        float data[2 * 3] = {};
        data[0] = 4.0f;
        const Vector3fArray *a = client.createConstVector3fArray(2, data);

        const ramses_internal::ResourceContentHash hash = a->impl.getLowlevelResourceHash();

        float data2[2 * 3] = {};
        data2[0] = 44.0f;
        const Vector3fArray *a2 = client.createConstVector3fArray(2, data2);

        const ramses_internal::ResourceContentHash hash2 = a2->impl.getLowlevelResourceHash();
        EXPECT_TRUE(hash != hash2);
    }

    TEST_F(AResourceTestClient, createEmptyVector3fArrayFails)
    {
        const Vector3fArray *a = client.createConstVector3fArray(0, NULL);
        EXPECT_TRUE(NULL == a);
    }

    TEST_F(AResourceTestClient, createVector4fArray)
    {
        const float data[2 * 4] = {};
        const Vector4fArray *a = client.createConstVector4fArray(2, data);
        ASSERT_TRUE(NULL != a);
    }

    TEST_F(AResourceTestClient, createVector4fArrayHashIsValid)
    {
        const float data[2 * 4] = {};
        const Vector4fArray *a = client.createConstVector4fArray(2, data);
        ASSERT_TRUE(NULL != a);

        const ramses_internal::ResourceContentHash hash = a->impl.getLowlevelResourceHash();
        EXPECT_TRUE(hash.isValid());
    }

    TEST_F(AResourceTestClient, createVector4fArrayHashIsUnique)
    {
        float data[2 * 4] = {};
        data[0] = 4.0f;
        const Vector4fArray *a = client.createConstVector4fArray(2, data);

        const ramses_internal::ResourceContentHash hash = a->impl.getLowlevelResourceHash();

        float data2[2 * 4] = {};
        data2[0] = 42.0f;
        const Vector4fArray *a2 = client.createConstVector4fArray(2, data2);

        const ramses_internal::ResourceContentHash hash2 = a2->impl.getLowlevelResourceHash();
        EXPECT_TRUE(hash != hash2);
    }

    TEST_F(AResourceTestClient, createEmptyVector4fArrayFails)
    {
        const Vector4fArray *a = client.createConstVector4fArray(0, NULL);
        EXPECT_TRUE(NULL == a);
    }

    TEST_F(AResourceTestClient, createUInt16Array)
    {
        const uint16_t data[2] = {};
        const UInt16Array *a = client.createConstUInt16Array(2, data);
        ASSERT_TRUE(NULL != a);
    }

    TEST_F(AResourceTestClient, createUInt16ArrayHashIsValid)
    {
        const uint16_t data[2] = {};
        const UInt16Array *a = client.createConstUInt16Array(2, data);
        ASSERT_TRUE(NULL != a);

        const ramses_internal::ResourceContentHash hash = a->impl.getLowlevelResourceHash();
        EXPECT_TRUE(hash.isValid());
    }

    TEST_F(AResourceTestClient, createUInt16ArrayHashIsUnique)
    {
        uint16_t data[2] = {};
        data[0] = 4;
        const UInt16Array *a = client.createConstUInt16Array(2, data);

        const ramses_internal::ResourceContentHash hash = a->impl.getLowlevelResourceHash();

        uint16_t data2[2] = {};
        data2[0] = 42;
        const UInt16Array *a2 = client.createConstUInt16Array(2, data2);

        const ramses_internal::ResourceContentHash hash2 = a2->impl.getLowlevelResourceHash();
        EXPECT_TRUE(hash != hash2);
    }

    TEST_F(AResourceTestClient, createEmptyUInt16ArrayFails)
    {
        const UInt16Array *a = client.createConstUInt16Array(0, NULL);
        EXPECT_TRUE(NULL == a);
    }

    TEST_F(AResourceTestClient, createUInt32Array)
    {
        const uint32_t data[2] = {};
        const UInt32Array *a = client.createConstUInt32Array(2, data);
        ASSERT_TRUE(NULL != a);
    }

    TEST_F(AResourceTestClient, createUInt32ArrayHashIsValid)
    {
        const uint32_t data[2] = {};
        const UInt32Array *a = client.createConstUInt32Array(2, data);
        ASSERT_TRUE(NULL != a);

        const ramses_internal::ResourceContentHash hash = a->impl.getLowlevelResourceHash();
        EXPECT_TRUE(hash.isValid());
    }

    TEST_F(AResourceTestClient, createUInt32ArrayHashIsUnique)
    {
        uint32_t data[2] = {};
        data[0] = 4;
        const UInt32Array *a = client.createConstUInt32Array(2, data);

        const ramses_internal::ResourceContentHash hash = a->impl.getLowlevelResourceHash();

        uint32_t data2[2] = {};
        data2[0] = 42;
        const UInt32Array *a2 = client.createConstUInt32Array(2, data2);

        const ramses_internal::ResourceContentHash hash2 = a2->impl.getLowlevelResourceHash();
        EXPECT_TRUE(hash != hash2);
    }

    TEST_F(AResourceTestClient, createEmptyUInt32ArrayFails)
    {
        const UInt32Array *a = client.createConstUInt32Array(0, NULL);
        EXPECT_TRUE(NULL == a);
    }





    TEST_F(AResourceTestClient, createAndDestroyEffect)
    {
        EffectDescription effectDesc;
        EXPECT_EQ(StatusOK, effectDesc.setVertexShaderFromFile("res/ramses-client-test_shader.vert"));
        EXPECT_EQ(StatusOK, effectDesc.setFragmentShaderFromFile("res/ramses-client-test_minimalShader.frag"));

        const Effect* const effect = client.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "effect");
        ASSERT_TRUE(effect != NULL);
    }

    TEST_F(AResourceTestClient, effectCreatedTwiceWithSameHashCanBeCreatedAndDestroyed)
    {
        EffectDescription effectDesc;
        EXPECT_EQ(StatusOK, effectDesc.setVertexShaderFromFile("res/ramses-client-test_shader.vert"));
        EXPECT_EQ(StatusOK, effectDesc.setFragmentShaderFromFile("res/ramses-client-test_minimalShader.frag"));

        const Effect* const effect1 = client.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "effect1");
        ASSERT_TRUE(effect1 != NULL);
        const Effect* const effect2 = client.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "effect2");
        ASSERT_TRUE(effect2 != NULL);

        EXPECT_EQ(StatusOK, client.destroy(*effect1));
        EXPECT_EQ(StatusOK, client.destroy(*effect2));
    }

    TEST_F(AResourceTestClient, effectCreatedTwiceWithDifferentNameProducesSameHash)
    {
        EffectDescription effectDesc;
        EXPECT_EQ(StatusOK, effectDesc.setVertexShaderFromFile("res/ramses-client-test_shader.vert"));
        EXPECT_EQ(StatusOK, effectDesc.setFragmentShaderFromFile("res/ramses-client-test_minimalShader.frag"));
        const Effect* const effect1 = client.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "effect1");
        ASSERT_TRUE(effect1 != NULL);
        const Effect* const effect2 = client.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "effect2");
        ASSERT_TRUE(effect2 != NULL);
        EXPECT_EQ(effect1->impl.getLowlevelResourceHash(), effect2->impl.getLowlevelResourceHash());
        client.destroy(*effect1);
        client.destroy(*effect2);
    }

    template <typename ResourceType>
    class ResourceTest : public LocalTestClientWithScene, public testing::Test
    {
    public:
        ResourceType& createResource(const char* name)
        {
            return (this->template createObject<ResourceType>(name));
        }
    };

    TYPED_TEST_CASE(ResourceTest, ResourceTypes);

    TYPED_TEST(ResourceTest, canBeConvertedToResource)
    {
        RamsesObject& obj = this->createResource("resource");
        EXPECT_TRUE(RamsesUtils::TryConvert<Resource>(obj) != NULL);
        const RamsesObject& constObj = obj;
        EXPECT_TRUE(RamsesUtils::TryConvert<Resource>(constObj) != NULL);
    }

    TYPED_TEST(ResourceTest, sameResourcesWithDifferentNamesShareSameHash)
    {
        const ramses_internal::String name_A("name_A");
        const ramses_internal::String name_B("name_B");
        Resource* resource_A = static_cast<Resource*>(&this->createResource(name_A.c_str()));
        Resource* resource_B = static_cast<Resource*>(&this->createResource(name_B.c_str()));

        RamsesObject* foundObject_A = this->getClient().findObjectByName(name_A.c_str());
        RamsesObject* foundObject_B = this->getClient().findObjectByName(name_B.c_str());

        ASSERT_TRUE(resource_A);
        ASSERT_TRUE(resource_B);
        EXPECT_EQ(resource_A->impl.getLowlevelResourceHash(), resource_B->impl.getLowlevelResourceHash());

        ASSERT_TRUE(foundObject_A);
        ASSERT_TRUE(foundObject_B);
        EXPECT_EQ(foundObject_A, static_cast<RamsesObject*>(resource_A));
        EXPECT_EQ(foundObject_B, static_cast<RamsesObject*>(resource_B));
        EXPECT_NE(resource_A, resource_B);
    }

    TYPED_TEST(ResourceTest, statisticCounterIsUpdated)
    {
        EXPECT_EQ(0u, this->getFramework().impl.getStatisticCollection().statResourcesCreated.getCounterValue());
        EXPECT_EQ(0u, this->getFramework().impl.getStatisticCollection().statResourcesDestroyed.getCounterValue());
        EXPECT_EQ(0u, this->getFramework().impl.getStatisticCollection().statResourcesNumber.getCounterValue());

        RamsesObject& obj = this->createResource("resource");
        Resource* res = RamsesUtils::TryConvert<Resource>(obj);
        EXPECT_EQ(1u, this->getFramework().impl.getStatisticCollection().statResourcesCreated.getCounterValue());

        this->getFramework().impl.getStatisticCollection().nextTimeInterval(); //statResourcesNumber is updated by nextTimeInterval()
        EXPECT_EQ(1u, this->getFramework().impl.getStatisticCollection().statResourcesNumber.getCounterValue());

        this->getClient().destroy(*res);
        EXPECT_EQ(1u, this->getFramework().impl.getStatisticCollection().statResourcesDestroyed.getCounterValue());

        this->getFramework().impl.getStatisticCollection().nextTimeInterval();
        EXPECT_EQ(0u, this->getFramework().impl.getStatisticCollection().statResourcesNumber.getCounterValue());
    }

    TEST_F(AResourceTestClient, statisticCounterIsUpdatedWhenLoadingFromFile)
    {
        const float data[2 * 2] = { 1, 2, 3, 4 };
        const Vector2fArray* res = client.createConstVector2fArray(2, data);

        ResourceFileDescription fileDescription("testfile.ramres");
        fileDescription.add(res);
        status_t status = this->getClient().saveResources(fileDescription, false);
        EXPECT_EQ(StatusOK, status);

        client.destroy(*res);
        EXPECT_EQ(1u, this->getFramework().impl.getStatisticCollection().statResourcesCreated.getCounterValue());
        EXPECT_EQ(1u, this->getFramework().impl.getStatisticCollection().statResourcesDestroyed.getCounterValue());

        this->getFramework().impl.getStatisticCollection().nextTimeInterval();
        EXPECT_EQ(0u, this->getFramework().impl.getStatisticCollection().statResourcesNumber.getCounterValue());

        EXPECT_EQ(0u, this->getFramework().impl.getStatisticCollection().statResourcesCreated.getCounterValue());
        EXPECT_EQ(0u, this->getFramework().impl.getStatisticCollection().statResourcesDestroyed.getCounterValue());


        ResourceFileDescription fileDescription2("testfile.ramres");
        status = client.loadResources(fileDescription2);
        EXPECT_EQ(StatusOK, status);
        EXPECT_EQ(1u, this->getFramework().impl.getStatisticCollection().statResourcesCreated.getCounterValue());
        EXPECT_EQ(0u, this->getFramework().impl.getStatisticCollection().statResourcesDestroyed.getCounterValue());

        this->getFramework().impl.getStatisticCollection().nextTimeInterval();
        EXPECT_EQ(1u, this->getFramework().impl.getStatisticCollection().statResourcesNumber.getCounterValue());
    }


    TEST_F(AResourceTestClient, destroyedResourceKeptInCache)
    {
        const float data[2 * 2] = { 1, 2, 3, 4 };
        const Vector2fArray* res = client.createConstVector2fArray(2, data);
        ramses_internal::ResourceContentHash resHash = res->impl.getLowlevelResourceHash(); //store hash as resource object gets destroyed
        const ramses_internal::IResource* internalRes = client.impl.getFramework().getResourceComponent().getResource(resHash).getResourceObject();

        client.destroy(*res);
        {
            //updateClientResourceCache is not called so resource still has to be alive
            ramses_internal::ManagedResource resourceFromResourceComponent = client.impl.getFramework().getResourceComponent().getResource(resHash);
            EXPECT_EQ(resourceFromResourceComponent.getResourceObject(), internalRes);
        }

        client.impl.setClientResourceCacheTimeout(std::chrono::milliseconds{ 1u });
        const std::chrono::milliseconds sleepDuration{ 10u };
        const auto timeBeforeSleep = std::chrono::steady_clock::now();
        // make sure timeout is over because on some platforms (e.g. the NUCs) are unstable with small time periods of sleep
        do
        {
            std::this_thread::sleep_for(sleepDuration);
        } while (std::chrono::steady_clock::now() - timeBeforeSleep < sleepDuration);
        //trigger updateClientResourceCache by scene flush
        Scene* dummyScene = client.createScene(123);
        dummyScene->flush();

        {
            ramses_internal::ManagedResource resourceFromResourceComponent = client.impl.getFramework().getResourceComponent().getResource(resHash);
            EXPECT_EQ(resourceFromResourceComponent.getResourceObject(), nullptr);
        }
    }
}
