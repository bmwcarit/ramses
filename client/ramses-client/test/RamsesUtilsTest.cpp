//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "ClientTestUtils.h"
#include "ramses-utils.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/MipLevelData.h"
#include "Texture2DImpl.h"
#include "Utils/File.h"
#include "ramses-client-api/TextureEnums.h"

using namespace testing;

namespace ramses
{
    class ARamsesUtilsTest : public ::testing::Test, public LocalTestClient
    {
    public:
        static void LoadFileToVector(const char* fname, std::vector<unsigned char>& data)
        {
            ramses_internal::File f(fname);
            ASSERT_TRUE(f.open(ramses_internal::File::Mode::ReadOnlyBinary));
            ramses_internal::UInt filesize = 0;
            ASSERT_TRUE(f.getSizeInBytes(filesize));
            data.resize(filesize);
            ramses_internal::UInt numread = 0;
            ASSERT_EQ(ramses_internal::EStatus::Ok, f.read(reinterpret_cast<char*>(data.data()), filesize, numread));
            ASSERT_EQ(filesize, numread);
        }
    };

    // texture from file: valid uses
    TEST_F(ARamsesUtilsTest, createTextureResourceFromPng)
    {
        Texture2D* texture = RamsesUtils::CreateTextureResourceFromPng("res/sampleTexture.png", client);
        ASSERT_TRUE(nullptr != texture);
        EXPECT_EQ(StatusOK, client.destroy(*texture));
    }

    TEST_F(ARamsesUtilsTest, createTextureResourceFromPng_withName)
    {
        Texture2D* texture = RamsesUtils::CreateTextureResourceFromPng("res/sampleTexture.png", client, {}, "mytesttexturename");
        ASSERT_TRUE(nullptr != texture);
        ASSERT_STREQ("mytesttexturename", texture->getName());
    }

    TEST_F(ARamsesUtilsTest, createTextureResourceFromPng_withSwizzle)
    {
        const TextureSwizzle swizzle {ramses::ETextureChannelColor::Blue, ramses::ETextureChannelColor::Alpha, ramses::ETextureChannelColor::Green, ramses::ETextureChannelColor::Red};
        Texture2D* texture = RamsesUtils::CreateTextureResourceFromPng("res/sampleTexture.png", client, swizzle);
        ASSERT_TRUE(nullptr != texture);
        EXPECT_EQ(swizzle.channelRed, texture->getTextureSwizzle().channelRed);
        EXPECT_EQ(swizzle.channelGreen, texture->getTextureSwizzle().channelGreen);
        EXPECT_EQ(swizzle.channelBlue, texture->getTextureSwizzle().channelBlue);
        EXPECT_EQ(swizzle.channelAlpha, texture->getTextureSwizzle().channelAlpha);
    }

    // texture from memory png: valid uses
    TEST_F(ARamsesUtilsTest, createTextureResourceFromPngBuffer)
    {
        std::vector<unsigned char> buffer;
        LoadFileToVector("res/sampleTexture.png", buffer);
        Texture2D* texture = RamsesUtils::CreateTextureResourceFromPngBuffer(buffer, client);
        ASSERT_TRUE(nullptr != texture);
        EXPECT_EQ(StatusOK, client.destroy(*texture));
    }

    TEST_F(ARamsesUtilsTest, createTextureResourceFromPngBuffer_withSwizzle)
    {
        const TextureSwizzle swizzle {ramses::ETextureChannelColor::Blue, ramses::ETextureChannelColor::Alpha, ramses::ETextureChannelColor::Green, ramses::ETextureChannelColor::Red};
        std::vector<unsigned char> buffer;
        LoadFileToVector("res/sampleTexture.png", buffer);
        Texture2D* texture = RamsesUtils::CreateTextureResourceFromPngBuffer(buffer, client, swizzle);
        ASSERT_TRUE(nullptr != texture);
        EXPECT_EQ(swizzle.channelRed, texture->getTextureSwizzle().channelRed);
        EXPECT_EQ(swizzle.channelGreen, texture->getTextureSwizzle().channelGreen);
        EXPECT_EQ(swizzle.channelBlue, texture->getTextureSwizzle().channelBlue);
        EXPECT_EQ(swizzle.channelAlpha, texture->getTextureSwizzle().channelAlpha);
        EXPECT_EQ(StatusOK, client.destroy(*texture));
    }

    TEST_F(ARamsesUtilsTest, createTextureResourceFromPngBuffer_withName)
    {
        std::vector<unsigned char> buffer;
        LoadFileToVector("res/sampleTexture.png", buffer);
        Texture2D* texture = RamsesUtils::CreateTextureResourceFromPngBuffer(buffer, client, {}, "mytesttexturename");
        ASSERT_TRUE(nullptr != texture);
        ASSERT_STREQ("mytesttexturename", texture->getName());
    }

    // texture from file: invalid uses
    TEST_F(ARamsesUtilsTest, createTextureResourceFromPng_invalidFileFormat)
    {
        Texture2D* texture = RamsesUtils::CreateTextureResourceFromPng("res/sampleTexture_invalid.png", client);
        EXPECT_TRUE(nullptr == texture);
    }

    TEST_F(ARamsesUtilsTest, createTextureResourceFromPng_nonexistantFile)
    {
        Texture2D* texture = RamsesUtils::CreateTextureResourceFromPng("res/this_file_should_not_exist_here_fdsgferg8u43g.png", client);
        EXPECT_TRUE(nullptr == texture);
    }

    TEST_F(ARamsesUtilsTest, createTextureResourceFromPng_NULLFile)
    {
        Texture2D* texture = RamsesUtils::CreateTextureResourceFromPng(nullptr, client);
        EXPECT_TRUE(nullptr == texture);
    }

    // texture from memory png: invalid uses
    TEST_F(ARamsesUtilsTest, createTextureResourceFromPngBuffer_EmptyBuffer)
    {
        std::vector<unsigned char> buffer;
        Texture2D* texture = RamsesUtils::CreateTextureResourceFromPngBuffer(buffer, client);
        EXPECT_TRUE(nullptr == texture);
    }

    TEST_F(ARamsesUtilsTest, createTextureResourceFromPngBuffer_InvalidBuffer)
    {
        std::vector<unsigned char> buffer(10, 1);
        Texture2D* texture = RamsesUtils::CreateTextureResourceFromPngBuffer(buffer, client);
        EXPECT_TRUE(nullptr == texture);
    }

    TEST_F(ARamsesUtilsTest, generateMipMapsForTexture2D)
    {
        const uint8_t pixelSize = 1u;
        const uint32_t width = 4u;
        const uint32_t height = 8u;
        uint8_t data[32u];
        for (uint8_t i = 0; i < 32u; i++)
        {
            data[i] = i + 1u;
        }

        uint32_t mipMapCount = 0u;
        MipLevelData* mipData = RamsesUtils::GenerateMipMapsTexture2D(width, height, pixelSize, data, mipMapCount);
        EXPECT_TRUE(mipData);
        EXPECT_EQ(4u, mipMapCount);

        // mip sizes
        EXPECT_EQ(32u, mipData[0].m_size);
        EXPECT_EQ(8u,  mipData[1].m_size);
        EXPECT_EQ(2u,  mipData[2].m_size);
        EXPECT_EQ(1u,  mipData[3].m_size);

        // original data has been copied
        EXPECT_NE(data, mipData[0].m_data);

        // mip level 1
        EXPECT_EQ(15u, mipData[0].m_data[14]);

        // mip level 2
        EXPECT_EQ(3u,  mipData[1].m_data[0]);
        EXPECT_EQ(29u, mipData[1].m_data[7]);

        // mip level 3
        EXPECT_EQ(8u,  mipData[2].m_data[0]);
        EXPECT_EQ(24u, mipData[2].m_data[1]);

        // mip level 4
        EXPECT_EQ(16u, mipData[3].m_data[0]);

        RamsesUtils::DeleteGeneratedMipMaps(mipData, mipMapCount);
        EXPECT_FALSE(mipData);
    }

    TEST_F(ARamsesUtilsTest, generateMipMapsForTexture2DWithMultipleChannels)
    {
        const uint8_t pixelSize = 2u;
        const uint32_t width = 8u;
        const uint32_t height = 4u;
        uint8_t data[64u];
        for (uint8_t i = 0; i < 32u; i++)
        {
            data[2*i] = i + 1u;
            data[2*i + 1] = i + 1u + 32u;
        }

        uint32_t mipMapCount = 0u;
        MipLevelData* mipData = RamsesUtils::GenerateMipMapsTexture2D(width, height, pixelSize, data, mipMapCount);
        EXPECT_TRUE(mipData);
        EXPECT_EQ(4u, mipMapCount);

        // mip sizes
        EXPECT_EQ(64u, mipData[0].m_size);
        EXPECT_EQ(16u, mipData[1].m_size);
        EXPECT_EQ(4u,  mipData[2].m_size);
        EXPECT_EQ(2u,  mipData[3].m_size);

        // original data has been copied
        EXPECT_NE(data, mipData[0].m_data);

        // mip level 1
        EXPECT_EQ(15u, mipData[0].m_data[28]);

        // mip level 2
        EXPECT_EQ(5u,  mipData[1].m_data[0]);
        EXPECT_EQ(37u, mipData[1].m_data[1]);
        EXPECT_EQ(27u, mipData[1].m_data[14]);
        EXPECT_EQ(59u, mipData[1].m_data[15]);

        // mip level 3
        EXPECT_EQ(14u, mipData[2].m_data[0]);
        EXPECT_EQ(46u, mipData[2].m_data[1]);
        EXPECT_EQ(18u, mipData[2].m_data[2]);
        EXPECT_EQ(50u, mipData[2].m_data[3]);

        // mip level 4
        EXPECT_EQ(16u, mipData[3].m_data[0]);
        EXPECT_EQ(48u, mipData[3].m_data[1]);

        RamsesUtils::DeleteGeneratedMipMaps(mipData, mipMapCount);
        EXPECT_FALSE(mipData);
    }

    TEST_F(ARamsesUtilsTest, generateMipMapsForTextureCube)
    {
        const uint8_t pixelSize = 1u;
        const uint32_t width = 4u;
        const uint32_t height = 4u;
        uint8_t data[96u]; // = 4*4*6
        for (uint8_t i = 0; i < 96u; i++)
        {
            data[i] = i + 1u;
        }

        uint32_t mipMapCount = 0u;
        CubeMipLevelData* mipData = RamsesUtils::GenerateMipMapsTextureCube(width, height, pixelSize, data, mipMapCount);
        EXPECT_TRUE(mipData);
        EXPECT_EQ(3u, mipMapCount);

        // mip sizes
        EXPECT_EQ(16u, mipData[0].m_faceDataSize);
        EXPECT_EQ(4u,  mipData[1].m_faceDataSize);
        EXPECT_EQ(1u,  mipData[2].m_faceDataSize);

        // original data has been copied
        EXPECT_NE(&data[0],  mipData[0].m_dataPX);
        EXPECT_NE(&data[16], mipData[0].m_dataNX);
        EXPECT_NE(&data[32], mipData[0].m_dataPY);
        EXPECT_NE(&data[48], mipData[0].m_dataNY);
        EXPECT_NE(&data[64], mipData[0].m_dataPZ);
        EXPECT_NE(&data[80], mipData[0].m_dataNZ);

        // mip level 1
        EXPECT_EQ(11u, mipData[0].m_dataPX[10]);
        EXPECT_EQ(27u, mipData[0].m_dataNX[10]);
        EXPECT_EQ(43u, mipData[0].m_dataPY[10]);
        EXPECT_EQ(59u, mipData[0].m_dataNY[10]);
        EXPECT_EQ(75u, mipData[0].m_dataPZ[10]);
        EXPECT_EQ(91u, mipData[0].m_dataNZ[10]);

        // mip level 2
        EXPECT_EQ(5u,  mipData[1].m_dataPX[1]);
        EXPECT_EQ(21u, mipData[1].m_dataNX[1]);
        EXPECT_EQ(37u, mipData[1].m_dataPY[1]);
        EXPECT_EQ(53u, mipData[1].m_dataNY[1]);
        EXPECT_EQ(69u, mipData[1].m_dataPZ[1]);
        EXPECT_EQ(85u, mipData[1].m_dataNZ[1]);

        // mip level 3
        EXPECT_EQ(8u,  mipData[2].m_dataPX[0]);
        EXPECT_EQ(24u, mipData[2].m_dataNX[0]);
        EXPECT_EQ(40u, mipData[2].m_dataPY[0]);
        EXPECT_EQ(56u, mipData[2].m_dataNY[0]);
        EXPECT_EQ(72u, mipData[2].m_dataPZ[0]);
        EXPECT_EQ(88u, mipData[2].m_dataNZ[0]);

        RamsesUtils::DeleteGeneratedMipMaps(mipData, mipMapCount);
        EXPECT_FALSE(mipData);
    }
}
