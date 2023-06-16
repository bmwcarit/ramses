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
#include "ramses-client-api/TextureEnums.h"
#include "ramses-client-api/DataObject.h"
#include "Texture2DImpl.h"
#include "Utils/File.h"
#include "Math3d/ProjectionParams.h"

#include <string_view>

using namespace testing;

namespace ramses
{
    class ARamsesUtilsTest : public ::testing::Test, public LocalTestClientWithScene
    {
    public:
        static void LoadFileToVector(std::string_view fname, std::vector<unsigned char>& data)
        {
            ramses_internal::File f(fname);
            ASSERT_TRUE(f.open(ramses_internal::File::Mode::ReadOnlyBinary));
            size_t filesize = 0;
            ASSERT_TRUE(f.getSizeInBytes(filesize));
            data.resize(filesize);
            size_t numread = 0;
            ASSERT_EQ(ramses_internal::EStatus::Ok, f.read(data.data(), filesize, numread));
            ASSERT_EQ(filesize, numread);
        }
    };

    // texture from file: valid uses
    TEST_F(ARamsesUtilsTest, createTextureResourceFromPng)
    {
        Texture2D* texture = RamsesUtils::CreateTextureResourceFromPng("res/sampleTexture.png", m_scene);
        ASSERT_TRUE(nullptr != texture);
        EXPECT_EQ(StatusOK, m_scene.destroy(*texture));
    }

    TEST_F(ARamsesUtilsTest, createTextureResourceFromPng_withName)
    {
        Texture2D* texture = RamsesUtils::CreateTextureResourceFromPng("res/sampleTexture.png", m_scene, {}, "mytesttexturename");
        ASSERT_TRUE(nullptr != texture);
        ASSERT_STREQ("mytesttexturename", texture->getName());
    }

    TEST_F(ARamsesUtilsTest, createTextureResourceFromPng_withSwizzle)
    {
        const TextureSwizzle swizzle {ramses::ETextureChannelColor::Blue, ramses::ETextureChannelColor::Alpha, ramses::ETextureChannelColor::Green, ramses::ETextureChannelColor::Red};
        Texture2D* texture = RamsesUtils::CreateTextureResourceFromPng("res/sampleTexture.png", m_scene, swizzle);
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
        Texture2D* texture = RamsesUtils::CreateTextureResourceFromPngBuffer(buffer, m_scene);
        ASSERT_TRUE(nullptr != texture);
        EXPECT_EQ(StatusOK, m_scene.destroy(*texture));
    }

    TEST_F(ARamsesUtilsTest, createTextureResourceFromPngBuffer_withSwizzle)
    {
        const TextureSwizzle swizzle {ramses::ETextureChannelColor::Blue, ramses::ETextureChannelColor::Alpha, ramses::ETextureChannelColor::Green, ramses::ETextureChannelColor::Red};
        std::vector<unsigned char> buffer;
        LoadFileToVector("res/sampleTexture.png", buffer);
        Texture2D* texture = RamsesUtils::CreateTextureResourceFromPngBuffer(buffer, m_scene, swizzle);
        ASSERT_TRUE(nullptr != texture);
        EXPECT_EQ(swizzle.channelRed, texture->getTextureSwizzle().channelRed);
        EXPECT_EQ(swizzle.channelGreen, texture->getTextureSwizzle().channelGreen);
        EXPECT_EQ(swizzle.channelBlue, texture->getTextureSwizzle().channelBlue);
        EXPECT_EQ(swizzle.channelAlpha, texture->getTextureSwizzle().channelAlpha);
        EXPECT_EQ(StatusOK, m_scene.destroy(*texture));
    }

    TEST_F(ARamsesUtilsTest, createTextureResourceFromPngBuffer_withName)
    {
        std::vector<unsigned char> buffer;
        LoadFileToVector("res/sampleTexture.png", buffer);
        Texture2D* texture = RamsesUtils::CreateTextureResourceFromPngBuffer(buffer, m_scene, {}, "mytesttexturename");
        ASSERT_TRUE(nullptr != texture);
        ASSERT_STREQ("mytesttexturename", texture->getName());
    }

    // texture from file: invalid uses
    TEST_F(ARamsesUtilsTest, createTextureResourceFromPng_invalidFileFormat)
    {
        Texture2D* texture = RamsesUtils::CreateTextureResourceFromPng("res/sampleTexture_invalid.png", m_scene);
        EXPECT_TRUE(nullptr == texture);
    }

    TEST_F(ARamsesUtilsTest, createTextureResourceFromPng_nonexistantFile)
    {
        Texture2D* texture = RamsesUtils::CreateTextureResourceFromPng("res/this_file_should_not_exist_here_fdsgferg8u43g.png", m_scene);
        EXPECT_TRUE(nullptr == texture);
    }

    TEST_F(ARamsesUtilsTest, createTextureResourceFromPng_NULLFile)
    {
        Texture2D* texture = RamsesUtils::CreateTextureResourceFromPng(nullptr, m_scene);
        EXPECT_TRUE(nullptr == texture);
    }

    // texture from memory png: invalid uses
    TEST_F(ARamsesUtilsTest, createTextureResourceFromPngBuffer_EmptyBuffer)
    {
        std::vector<unsigned char> buffer;
        Texture2D* texture = RamsesUtils::CreateTextureResourceFromPngBuffer(buffer, m_scene);
        EXPECT_TRUE(nullptr == texture);
    }

    TEST_F(ARamsesUtilsTest, createTextureResourceFromPngBuffer_InvalidBuffer)
    {
        std::vector<unsigned char> buffer(10, 1);
        Texture2D* texture = RamsesUtils::CreateTextureResourceFromPngBuffer(buffer, m_scene);
        EXPECT_TRUE(nullptr == texture);
    }

    TEST_F(ARamsesUtilsTest, canSaveImageBufferToPng)
    {
        const std::string pngPath = "rgba8.png";
        const uint32_t width = 2u;
        const uint32_t height = 2u;
        const std::vector<uint8_t> rgba8Data =
        {
            0xff, 0x00, 0x00, 0xff,
            0x00, 0xff, 0x00, 0xff,
            0x00, 0x00, 0xff, 0xff,
            0xff, 0xff, 0xff, 0x7f
        };
        EXPECT_TRUE(RamsesUtils::SaveImageBufferToPng(pngPath, rgba8Data, width, height));

        Texture2D* texture = RamsesUtils::CreateTextureResourceFromPng(pngPath.c_str(), m_scene);

        EXPECT_TRUE(texture);
        EXPECT_EQ(width, texture->getWidth());
        EXPECT_EQ(height, texture->getHeight());
    }

    TEST_F(ARamsesUtilsTest, canSaveImageBufferToPngDifferentOverloadOfFunction)
    {
        const std::string pngPath = "rgba8.png";
        const uint32_t width = 2u;
        const uint32_t height = 2u;
        std::vector<uint8_t> rgba8Data =
        {
            0xff, 0x00, 0x00, 0xff,
            0x00, 0xff, 0x00, 0xff,
            0x00, 0x00, 0xff, 0xff,
            0xff, 0xff, 0xff, 0x7f
        };
        EXPECT_TRUE(RamsesUtils::SaveImageBufferToPng(pngPath, rgba8Data, width, height, false));

        Texture2D* texture = RamsesUtils::CreateTextureResourceFromPng(pngPath.c_str(), m_scene);

        EXPECT_TRUE(texture);
        EXPECT_EQ(width, texture->getWidth());
        EXPECT_EQ(height, texture->getHeight());
    }

    TEST_F(ARamsesUtilsTest, canSaveImageBufferToPngFlippedVertically)
    {
        const std::string pngPath = "rgba8_flippedVertically.png";
        const std::string expectedPngPath = "res/rgba8_expectedFlipped.png";
        const uint32_t width = 2u;
        const uint32_t height = 2u;
        std::vector<uint8_t> rgba8Data =
        {
            0xff, 0x12, 0xa1, 0xff,
            0xf0, 0x64, 0x26, 0xff,
            0x77, 0x8b, 0xff, 0xff,
            0xff, 0x97, 0xff, 0x7f
        };
        EXPECT_TRUE(RamsesUtils::SaveImageBufferToPng(pngPath, rgba8Data, width, height, true));

        Texture2D* createdTexture = RamsesUtils::CreateTextureResourceFromPng(pngPath.c_str(), m_scene);
        Texture2D* expectedTexture = RamsesUtils::CreateTextureResourceFromPng(expectedPngPath.c_str(), m_scene);

        EXPECT_TRUE(createdTexture);
        EXPECT_EQ(width, createdTexture->getWidth());
        EXPECT_EQ(height, createdTexture->getHeight());
        EXPECT_EQ(createdTexture->getResourceId(), expectedTexture->getResourceId());
    }

    TEST_F(ARamsesUtilsTest, failsSavingImageBufferToPngGivenInvalidFilePath)
    {
        const std::string invalidFilePath = "nonexisting_dir/file.png";
        const uint32_t width = 2u;
        const uint32_t height = 2u;
        const std::vector<uint8_t> rgba8Data =
        {
            0xff, 0x12, 0xa1, 0xff,
            0xf0, 0x64, 0x26, 0xff,
            0x77, 0x8b, 0xff, 0xff,
            0xff, 0x97, 0xff, 0x7f
        };
        EXPECT_FALSE(RamsesUtils::SaveImageBufferToPng(invalidFilePath, rgba8Data, width, height));
    }

    TEST_F(ARamsesUtilsTest, failsSavingImageBufferToPngGivenInvalidFilePathDifferentFunctionOverload)
    {
        const std::string invalidFilePath = "nonexisting_dir/file.png";
        const uint32_t width = 2u;
        const uint32_t height = 2u;
        std::vector<uint8_t> rgba8Data =
        {
            0xff, 0x12, 0xa1, 0xff,
            0xf0, 0x64, 0x26, 0xff,
            0x77, 0x8b, 0xff, 0xff,
            0xff, 0x97, 0xff, 0x7f
        };
        EXPECT_FALSE(RamsesUtils::SaveImageBufferToPng(invalidFilePath, rgba8Data, width, height, false));
    }

    TEST_F(ARamsesUtilsTest, failsSavingImageBufferToPngGivenWidthOrHeightZero)
    {
        const std::vector<uint8_t> texureData =
        {
            0xff, 0x00, 0x00, 0xff,
            0x00, 0xff, 0x00, 0xff,
            0x00, 0x00, 0xff, 0xff,
            0xff, 0xff, 0xff, 0x7f
        };
        const uint32_t width = 4u;
        const uint32_t zeroWidth = 0u;
        const uint32_t height = 1u;
        const uint32_t zeroHeight = 0u;
        EXPECT_FALSE(RamsesUtils::SaveImageBufferToPng("invalidTexture.png", texureData, zeroWidth, height));
        EXPECT_FALSE(RamsesUtils::SaveImageBufferToPng("invalidTexture.png", texureData, width, zeroHeight));
    }

    TEST_F(ARamsesUtilsTest, failsSavingImageBufferToPngGivenWidthOrHeightZeroDifferentFunctionOverload)
    {
        std::vector<uint8_t> texureData =
        {
            0xff, 0x00, 0x00, 0xff,
            0x00, 0xff, 0x00, 0xff,
            0x00, 0x00, 0xff, 0xff,
            0xff, 0xff, 0xff, 0x7f
        };
        const uint32_t width = 4u;
        const uint32_t zeroWidth = 0u;
        const uint32_t height = 1u;
        const uint32_t zeroHeight = 0u;
        EXPECT_FALSE(RamsesUtils::SaveImageBufferToPng("invalidTexture.png", texureData, zeroWidth, height, false));
        EXPECT_FALSE(RamsesUtils::SaveImageBufferToPng("invalidTexture.png", texureData, width, zeroHeight, false));
    }

    TEST_F(ARamsesUtilsTest, failsSavingImageBufferToPngGivenImageSizeNotMatchingBufferSize)
    {
        const std::vector<uint8_t> texureData =
        {
            0xff, 0x00, 0x00, 0xff,
            0x00, 0xff, 0x00, 0xff
        };
        const uint32_t wrongWidth = 5u;
        const uint32_t wrongHeight = 3u;
        EXPECT_FALSE(RamsesUtils::SaveImageBufferToPng("invalidTexture.png", texureData, wrongWidth, wrongHeight));
    }

    TEST_F(ARamsesUtilsTest, failsSavingImageBufferToPngGivenImageSizeNotMatchingBufferSizeDifferentFunctionOverload)
    {
        std::vector<uint8_t> texureData =
        {
            0xff, 0x00, 0x00, 0xff,
            0x00, 0xff, 0x00, 0xff
        };
        const uint32_t wrongWidth = 5u;
        const uint32_t wrongHeight = 3u;
        EXPECT_FALSE(RamsesUtils::SaveImageBufferToPng("invalidTexture.png", texureData, wrongWidth, wrongHeight, false));
    }

    TEST_F(ARamsesUtilsTest, failsSavingImageBufferToPngWidthHeightExceedingLimit)
    {
        const std::string pngPath = "rgba8.png";
        const uint32_t width = 134217727u;
        const uint32_t height = 135217727u;
        const std::vector<uint8_t> rgba8Data =
        {
            0xff, 0x00, 0x00, 0xff,
            0x00, 0xff, 0x00, 0xff,
            0x00, 0x00, 0xff, 0xff,
            0xff, 0xff, 0xff, 0x7f
        };
        EXPECT_FALSE(RamsesUtils::SaveImageBufferToPng(pngPath, rgba8Data, width, height));
    }

    TEST_F(ARamsesUtilsTest, failsSavingImageBufferToPngWidthHeightExceedingLimitDifferentFunctionOverload)
    {
        const std::string pngPath = "rgba8.png";
        const uint32_t width = 134217727u;
        const uint32_t height = 135217727u;
        std::vector<uint8_t> rgba8Data =
        {
            0xff, 0x00, 0x00, 0xff,
            0x00, 0xff, 0x00, 0xff,
            0x00, 0x00, 0xff, 0xff,
            0xff, 0xff, 0xff, 0x7f
        };
        EXPECT_FALSE(RamsesUtils::SaveImageBufferToPng(pngPath, rgba8Data, width, height, false));
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

        size_t mipMapCount = 0u;
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

        size_t mipMapCount = 0u;
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

        size_t mipMapCount = 0u;
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

    TEST_F(ARamsesUtilsTest, canSetFrustumOnDataObjects)
    {
        auto& do1 = *m_scene.createDataObject(EDataType::Vector4F);
        auto& do2 = *m_scene.createDataObject(EDataType::Vector2F);
        EXPECT_TRUE(RamsesUtils::SetPerspectiveCameraFrustumToDataObjects(33.f, 1.5f, 1.f, 2.f, do1, do2));

        const auto expectedParams = ramses_internal::ProjectionParams::Perspective(33.f, 1.5f, 1.f, 2.f);
        vec4f planes1;
        vec2f planes2;
        EXPECT_EQ(StatusOK, do1.getValue(planes1));
        EXPECT_EQ(StatusOK, do2.getValue(planes2));
        EXPECT_EQ(expectedParams.leftPlane, planes1[0]);
        EXPECT_EQ(expectedParams.rightPlane, planes1[1]);
        EXPECT_EQ(expectedParams.bottomPlane, planes1[2]);
        EXPECT_EQ(expectedParams.topPlane, planes1[3]);
        EXPECT_EQ(expectedParams.nearPlane, planes2[0]);
        EXPECT_EQ(expectedParams.farPlane, planes2[1]);
    }

    TEST_F(ARamsesUtilsTest, failsToSetInvalidFrustumOnDataObjects)
    {
        auto& do1 = *m_scene.createDataObject(EDataType::Vector4F);
        auto& do2 = *m_scene.createDataObject(EDataType::Vector2F);
        EXPECT_FALSE(RamsesUtils::SetPerspectiveCameraFrustumToDataObjects(0.f, 1.5f, 1.f, 2.f, do1, do2));
        EXPECT_FALSE(RamsesUtils::SetPerspectiveCameraFrustumToDataObjects(-33.f, 1.5f, 1.f, 2.f, do1, do2));
        EXPECT_FALSE(RamsesUtils::SetPerspectiveCameraFrustumToDataObjects(33.f, 0.f, 1.f, 2.f, do1, do2));
        EXPECT_FALSE(RamsesUtils::SetPerspectiveCameraFrustumToDataObjects(33.f, -1.5f, 1.f, 2.f, do1, do2));
        EXPECT_FALSE(RamsesUtils::SetPerspectiveCameraFrustumToDataObjects(33.f, 1.5f, 0.f, 2.f, do1, do2));
        EXPECT_FALSE(RamsesUtils::SetPerspectiveCameraFrustumToDataObjects(33.f, 1.5f, 2.f, 1.f, do1, do2));
    }

    TEST_F(ARamsesUtilsTest, getNodeHandleReturnsMemoryHandle)
    {
        MeshNode& node = this->createValidMeshNode();
        EXPECT_EQ(node.m_impl.getNodeHandle().asMemoryHandle(), RamsesUtils::GetNodeId(node).getValue());
    }
}
