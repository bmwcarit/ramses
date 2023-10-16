//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#include "gtest/gtest.h"
#include "internal/SceneGraph/Resource/TextureResource.h"

namespace ramses::internal
{
    class TextureResourceTest : public ::testing::Test
    {
    public:
        TextureResourceTest()
            : m_mipLevelSizes({ 12u, 3u })
        {
            m_texDesc.m_width = 1u;
            m_texDesc.m_height = 2u;
            m_texDesc.m_depth = 3u;
            m_texDesc.m_format = EPixelStorageFormat::RGB8;
            m_texDesc.m_swizzle = {ETextureChannelColor::Green, ETextureChannelColor::Blue, ETextureChannelColor::Red, ETextureChannelColor::Alpha};
            m_texDesc.m_dataSizes = m_mipLevelSizes;
            m_texDesc.m_generateMipChain = false;
        }

        TextureMetaInfo m_texDesc;
        const MipDataSizeVector m_mipLevelSizes;
    };

    TEST_F(TextureResourceTest, textureResourceStoresMetaInfo)
    {
        TextureResource texRes(EResourceType::Texture3D, this->m_texDesc, {});

        // check properties
        EXPECT_EQ(texRes.getWidth(), this->m_texDesc.m_width);
        EXPECT_EQ(texRes.getHeight(), this->m_texDesc.m_height);
        EXPECT_EQ(texRes.getDepth(), this->m_texDesc.m_depth);
        EXPECT_EQ(texRes.getTextureFormat(), this->m_texDesc.m_format);
        EXPECT_EQ(texRes.getTextureSwizzle(), this->m_texDesc.m_swizzle);
        EXPECT_EQ(texRes.getTypeID(), EResourceType::Texture3D);
        EXPECT_EQ(texRes.getMipDataSizes(), this->m_mipLevelSizes);
        EXPECT_EQ(texRes.getResourceData().size(), std::accumulate(this->m_mipLevelSizes.begin(), this->m_mipLevelSizes.end(), 0u));
        EXPECT_EQ(texRes.getGenerateMipChainFlag(), this->m_texDesc.m_generateMipChain);
    }

    TEST_F(TextureResourceTest, textureResourceStoresMetaInfoWithCorrectSizeForCubeTexture)
    {
        TextureResource texRes(EResourceType::TextureCube, this->m_texDesc, {});

        // check properties
        EXPECT_EQ(texRes.getWidth(), this->m_texDesc.m_width);
        EXPECT_EQ(texRes.getHeight(), this->m_texDesc.m_height);
        EXPECT_EQ(texRes.getDepth(), this->m_texDesc.m_depth);
        EXPECT_EQ(texRes.getTextureFormat(), this->m_texDesc.m_format);
        EXPECT_EQ(texRes.getTextureSwizzle(), this->m_texDesc.m_swizzle);
        EXPECT_EQ(texRes.getTypeID(), EResourceType::TextureCube);
        EXPECT_EQ(texRes.getMipDataSizes(), this->m_mipLevelSizes);
        EXPECT_EQ(texRes.getResourceData().size(), 6u * std::accumulate(this->m_mipLevelSizes.begin(), this->m_mipLevelSizes.end(), 0u));
        EXPECT_EQ(texRes.getGenerateMipChainFlag(), this->m_texDesc.m_generateMipChain);
    }
}
