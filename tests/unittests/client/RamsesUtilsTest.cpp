//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "ClientTestUtils.h"
#include "ramses/client/ramses-utils.h"
#include "ramses/client/Texture2D.h"
#include "ramses/client/MipLevelData.h"
#include "ramses/framework/TextureEnums.h"
#include "ramses/client/DataObject.h"
#include "impl/Texture2DImpl.h"
#include "internal/Core/Utils/File.h"
#include "internal/Core/Math3d/ProjectionParams.h"

#include <string_view>

using namespace testing;

namespace ramses::internal
{
    class ARamsesUtilsTest : public ::testing::Test, public LocalTestClientWithScene
    {
    public:
        static void LoadFileToVector(std::string_view fname, std::vector<unsigned char>& data)
        {
            ramses::internal::File f(fname);
            ASSERT_TRUE(f.open(ramses::internal::File::Mode::ReadOnlyBinary));
            size_t filesize = 0;
            ASSERT_TRUE(f.getSizeInBytes(filesize));
            data.resize(filesize);
            size_t numread = 0;
            ASSERT_EQ(ramses::internal::EStatus::Ok, f.read(data.data(), filesize, numread));
            ASSERT_EQ(filesize, numread);
        }
    };

    // texture from file: valid uses
    TEST_F(ARamsesUtilsTest, createTextureResourceFromPng)
    {
        Texture2D* texture = RamsesUtils::CreateTextureResourceFromPng("res/sampleTexture.png", m_scene);
        ASSERT_TRUE(nullptr != texture);
        EXPECT_TRUE(m_scene.destroy(*texture));
    }

    TEST_F(ARamsesUtilsTest, createTextureResourceFromPng_withName)
    {
        Texture2D* texture = RamsesUtils::CreateTextureResourceFromPng("res/sampleTexture.png", m_scene, {}, "mytesttexturename");
        ASSERT_TRUE(nullptr != texture);
        EXPECT_EQ("mytesttexturename", texture->getName());
    }

    TEST_F(ARamsesUtilsTest, createTextureResourceFromPng_withSwizzle)
    {
        const TextureSwizzle swizzle {ETextureChannelColor::Blue, ETextureChannelColor::Alpha, ETextureChannelColor::Green, ETextureChannelColor::Red};
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
        EXPECT_TRUE(m_scene.destroy(*texture));
    }

    TEST_F(ARamsesUtilsTest, createTextureResourceFromPngBuffer_withSwizzle)
    {
        const TextureSwizzle swizzle {ETextureChannelColor::Blue, ETextureChannelColor::Alpha, ETextureChannelColor::Green, ETextureChannelColor::Red};
        std::vector<unsigned char> buffer;
        LoadFileToVector("res/sampleTexture.png", buffer);
        Texture2D* texture = RamsesUtils::CreateTextureResourceFromPngBuffer(buffer, m_scene, swizzle);
        ASSERT_TRUE(nullptr != texture);
        EXPECT_EQ(swizzle.channelRed, texture->getTextureSwizzle().channelRed);
        EXPECT_EQ(swizzle.channelGreen, texture->getTextureSwizzle().channelGreen);
        EXPECT_EQ(swizzle.channelBlue, texture->getTextureSwizzle().channelBlue);
        EXPECT_EQ(swizzle.channelAlpha, texture->getTextureSwizzle().channelAlpha);
        EXPECT_TRUE(m_scene.destroy(*texture));
    }

    TEST_F(ARamsesUtilsTest, createTextureResourceFromPngBuffer_withName)
    {
        std::vector<unsigned char> buffer;
        LoadFileToVector("res/sampleTexture.png", buffer);
        Texture2D* texture = RamsesUtils::CreateTextureResourceFromPngBuffer(buffer, m_scene, {}, "mytesttexturename");
        ASSERT_TRUE(nullptr != texture);
        EXPECT_EQ("mytesttexturename", texture->getName());
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
        std::byte data[32u];
        for (uint8_t i = 0; i < 32u; i++)
        {
            data[i] = std::byte(i + 1u);
        }

        size_t mipMapCount = 0u;
        auto* mipData = RamsesUtils::GenerateMipMapsTexture2D(width, height, pixelSize, data, mipMapCount);
        EXPECT_TRUE(mipData);
        EXPECT_EQ(4u, mipMapCount);

        // mip sizes
        EXPECT_EQ(32u, (*mipData)[0].size());
        EXPECT_EQ(8u,  (*mipData)[1].size());
        EXPECT_EQ(2u,  (*mipData)[2].size());
        EXPECT_EQ(1u,  (*mipData)[3].size());

        // mip level 1
        EXPECT_EQ(std::byte{15u}, (*mipData)[0][14]);

        // mip level 2
        EXPECT_EQ(std::byte{3u},  (*mipData)[1][0]);
        EXPECT_EQ(std::byte{29u}, (*mipData)[1][7]);

        // mip level 3
        EXPECT_EQ(std::byte{8u},  (*mipData)[2][0]);
        EXPECT_EQ(std::byte{24u}, (*mipData)[2][1]);

        // mip level 4
        EXPECT_EQ(std::byte{16u}, (*mipData)[3][0]);

        RamsesUtils::DeleteGeneratedMipMaps(mipData);
        EXPECT_FALSE(mipData);
    }

    TEST_F(ARamsesUtilsTest, doesNotGenerateMipMapsForTexture2DWithNonPow2Width)
    {
        const uint8_t pixelSize = 1u;
        const uint32_t width = 3u;
        const uint32_t height = 8u;
        const size_t dataSize = width * height * pixelSize;
        std::vector<std::byte> data(dataSize);
        for (uint8_t i = 0; i < dataSize; i++)
        {
            data[i] = std::byte(i + 1u);
        }

        size_t mipMapCount = 0u;
        auto mipData = RamsesUtils::GenerateMipMapsTexture2D(width, height, pixelSize, data.data(), mipMapCount);
        EXPECT_TRUE(mipData);
        ASSERT_EQ(1u, mipMapCount);

        EXPECT_EQ(data, (*mipData)[0]);

        RamsesUtils::DeleteGeneratedMipMaps(mipData);
        EXPECT_FALSE(mipData);
    }

    TEST_F(ARamsesUtilsTest, doesNotGenerateMipMapsForTexture2DWithNonPow2Height)
    {
        const uint8_t pixelSize = 1u;
        const uint32_t width = 4u;
        const uint32_t height = 7u;
        const size_t dataSize = width * height * pixelSize;
        std::vector<std::byte> data(dataSize);
        for (uint8_t i = 0; i < dataSize; i++)
        {
            data[i] = std::byte(i + 1u);
        }

        size_t mipMapCount = 0u;
        auto mipData = RamsesUtils::GenerateMipMapsTexture2D(width, height, pixelSize, data.data(), mipMapCount);
        EXPECT_TRUE(mipData);
        ASSERT_EQ(1u, mipMapCount);

        EXPECT_EQ(data, (*mipData)[0]);

        RamsesUtils::DeleteGeneratedMipMaps(mipData);
        EXPECT_FALSE(mipData);
    }

    TEST_F(ARamsesUtilsTest, generateMipMapsForTexture2DWithMultipleChannels)
    {
        const uint8_t pixelSize = 2u;
        const uint32_t width = 8u;
        const uint32_t height = 4u;
        std::byte data[64u];
        for (uint8_t i = 0; i < 32u; i++)
        {
            data[2*i] = std::byte(i + 1u);
            data[2*i + 1] = std::byte(i + 1u + 32u);
        }

        size_t mipMapCount = 0u;
        auto mipData = RamsesUtils::GenerateMipMapsTexture2D(width, height, pixelSize, data, mipMapCount);
        EXPECT_TRUE(mipData);
        EXPECT_EQ(4u, mipMapCount);

        // mip sizes
        EXPECT_EQ(64u, (*mipData)[0].size());
        EXPECT_EQ(16u, (*mipData)[1].size());
        EXPECT_EQ(4u,  (*mipData)[2].size());
        EXPECT_EQ(2u,  (*mipData)[3].size());

        // mip level 1
        EXPECT_EQ(std::byte{15u}, (*mipData)[0][28]);

        // mip level 2
        EXPECT_EQ(std::byte{5u},  (*mipData)[1][0]);
        EXPECT_EQ(std::byte{37u}, (*mipData)[1][1]);
        EXPECT_EQ(std::byte{27u}, (*mipData)[1][14]);
        EXPECT_EQ(std::byte{59u}, (*mipData)[1][15]);

        // mip level 3
        EXPECT_EQ(std::byte{14u}, (*mipData)[2][0]);
        EXPECT_EQ(std::byte{46u}, (*mipData)[2][1]);
        EXPECT_EQ(std::byte{18u}, (*mipData)[2][2]);
        EXPECT_EQ(std::byte{50u}, (*mipData)[2][3]);

        // mip level 4
        EXPECT_EQ(std::byte{16u}, (*mipData)[3][0]);
        EXPECT_EQ(std::byte{48u}, (*mipData)[3][1]);

        RamsesUtils::DeleteGeneratedMipMaps(mipData);
        EXPECT_FALSE(mipData);
    }

    TEST_F(ARamsesUtilsTest, generateMipMapsForTextureCube)
    {
        const uint8_t pixelSize = 1u;
        const uint32_t width = 4u;
        const uint32_t height = 4u;
        std::byte data[96u]; // = 4*4*6
        for (uint8_t i = 0; i < 96u; i++)
        {
            data[i] = std::byte(i + 1u);
        }

        size_t mipMapCount = 0u;
        auto mipData = RamsesUtils::GenerateMipMapsTextureCube(width, height, pixelSize, data, mipMapCount);
        EXPECT_TRUE(mipData);
        EXPECT_EQ(3u, mipMapCount);

        // mip sizes
        EXPECT_EQ(16u, (*mipData)[0].m_dataPX.size());
        EXPECT_EQ(4u,  (*mipData)[1].m_dataPX.size());
        EXPECT_EQ(1u,  (*mipData)[2].m_dataPX.size());

        // mip level 1
        EXPECT_EQ(std::byte{11u}, (*mipData)[0].m_dataPX[10]);
        EXPECT_EQ(std::byte{27u}, (*mipData)[0].m_dataNX[10]);
        EXPECT_EQ(std::byte{43u}, (*mipData)[0].m_dataPY[10]);
        EXPECT_EQ(std::byte{59u}, (*mipData)[0].m_dataNY[10]);
        EXPECT_EQ(std::byte{75u}, (*mipData)[0].m_dataPZ[10]);
        EXPECT_EQ(std::byte{91u}, (*mipData)[0].m_dataNZ[10]);

        // mip level 2
        EXPECT_EQ(std::byte{5u},  (*mipData)[1].m_dataPX[1]);
        EXPECT_EQ(std::byte{21u}, (*mipData)[1].m_dataNX[1]);
        EXPECT_EQ(std::byte{37u}, (*mipData)[1].m_dataPY[1]);
        EXPECT_EQ(std::byte{53u}, (*mipData)[1].m_dataNY[1]);
        EXPECT_EQ(std::byte{69u}, (*mipData)[1].m_dataPZ[1]);
        EXPECT_EQ(std::byte{85u}, (*mipData)[1].m_dataNZ[1]);

        // mip level 3
        EXPECT_EQ(std::byte{8u},  (*mipData)[2].m_dataPX[0]);
        EXPECT_EQ(std::byte{24u}, (*mipData)[2].m_dataNX[0]);
        EXPECT_EQ(std::byte{40u}, (*mipData)[2].m_dataPY[0]);
        EXPECT_EQ(std::byte{56u}, (*mipData)[2].m_dataNY[0]);
        EXPECT_EQ(std::byte{72u}, (*mipData)[2].m_dataPZ[0]);
        EXPECT_EQ(std::byte{88u}, (*mipData)[2].m_dataNZ[0]);

        RamsesUtils::DeleteGeneratedMipMaps(mipData);
        EXPECT_FALSE(mipData);
    }

    TEST_F(ARamsesUtilsTest, canSetFrustumOnDataObjects)
    {
        auto& do1 = *m_scene.createDataObject(ramses::EDataType::Vector4F);
        auto& do2 = *m_scene.createDataObject(ramses::EDataType::Vector2F);
        EXPECT_TRUE(RamsesUtils::SetPerspectiveCameraFrustumToDataObjects(33.f, 1.5f, 1.f, 2.f, do1, do2));

        const auto expectedParams = ramses::internal::ProjectionParams::Perspective(33.f, 1.5f, 1.f, 2.f);
        vec4f planes1;
        vec2f planes2;
        EXPECT_TRUE(do1.getValue(planes1));
        EXPECT_TRUE(do2.getValue(planes2));
        EXPECT_EQ(expectedParams.leftPlane, planes1[0]);
        EXPECT_EQ(expectedParams.rightPlane, planes1[1]);
        EXPECT_EQ(expectedParams.bottomPlane, planes1[2]);
        EXPECT_EQ(expectedParams.topPlane, planes1[3]);
        EXPECT_EQ(expectedParams.nearPlane, planes2[0]);
        EXPECT_EQ(expectedParams.farPlane, planes2[1]);
    }

    TEST_F(ARamsesUtilsTest, failsToSetInvalidFrustumOnDataObjects)
    {
        auto& do1 = *m_scene.createDataObject(ramses::EDataType::Vector4F);
        auto& do2 = *m_scene.createDataObject(ramses::EDataType::Vector2F);
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
        EXPECT_EQ(node.impl().getNodeHandle().asMemoryHandle(), RamsesUtils::GetNodeId(node).getValue());
    }
}
