//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include <array>

#include "ClientTestUtils.h"
#include "SceneImpl.h"
#include "ramses-utils.h"
#include "Scene/ClientScene.h"
#include "ramses-client-api/Texture2DBuffer.h"
#include "Texture2DBufferImpl.h"

using namespace testing;
using namespace ramses_internal;

namespace ramses
{
    class ATexture2DBuffer : public LocalTestClientWithScene, public testing::Test
    {
    protected:
        ATexture2DBuffer()
            : LocalTestClientWithScene()
        {
        }
    };

    TEST_F(ATexture2DBuffer, IsAllocatedOnInternalSceneAfterCreation)
    {
        Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(2, 3, 4, ETextureFormat_RGBA8);
        const ramses_internal::TextureBufferHandle textureBufferHandle = textureBuffer.impl.getTextureBufferHandle();

        EXPECT_TRUE(textureBufferHandle.isValid());
        EXPECT_TRUE(this->m_scene.impl.getIScene().isTextureBufferAllocated(textureBufferHandle));
    }

    TEST_F(ATexture2DBuffer, CanGetTexelFormat)
    {
        Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(2, 3, 4, ETextureFormat_RGBA8);

        EXPECT_EQ(ETextureFormat_RGBA8, textureBuffer.getTexelFormat());
    }

    TEST_F(ATexture2DBuffer, PropagatesItsPropertiesToInternalScene)
    {
        Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(2, 3, 4, ETextureFormat_RGBA8);
        const ramses_internal::TextureBufferHandle textureBufferHandle = textureBuffer.impl.getTextureBufferHandle();
        const ramses_internal::TextureBuffer& internalTexBuffer = this->m_scene.impl.getIScene().getTextureBuffer(textureBufferHandle);

        EXPECT_EQ(ramses_internal::ETextureFormat_RGBA8, internalTexBuffer.textureFormat);
        ASSERT_EQ(2u, internalTexBuffer.mipMaps.size());
        EXPECT_EQ(3u, internalTexBuffer.mipMaps[0].width);
        EXPECT_EQ(4u, internalTexBuffer.mipMaps[0].height);
        EXPECT_EQ(1u, internalTexBuffer.mipMaps[1].width);
        EXPECT_EQ(2u, internalTexBuffer.mipMaps[1].height);
    }

    TEST_F(ATexture2DBuffer, CanBeCreatedSoThatTheLastMipLevelHasSize1x1)
    {
        // square
        EXPECT_NE(nullptr, m_scene.createTexture2DBuffer(1, 1, 1, ETextureFormat_RGBA8));
        EXPECT_NE(nullptr, m_scene.createTexture2DBuffer(3, 4, 4, ETextureFormat_RGBA8));
        // non-square
        EXPECT_NE(nullptr, m_scene.createTexture2DBuffer(4, 8, 4, ETextureFormat_RGBA8));
        EXPECT_NE(nullptr, m_scene.createTexture2DBuffer(4, 4, 8, ETextureFormat_RGBA8));
        // non-square with NPOT texture
        EXPECT_NE(nullptr, m_scene.createTexture2DBuffer(3, 6, 4, ETextureFormat_RGBA8));
        EXPECT_NE(nullptr, m_scene.createTexture2DBuffer(3, 4, 6, ETextureFormat_RGBA8));
    }

    TEST_F(ATexture2DBuffer, CanNotBeCreatedIfMipLevelsWouldResultInTwoConsecutiveMipsWithSize1x1)
    {
        // square
        EXPECT_EQ(nullptr, m_scene.createTexture2DBuffer(2, 1, 1, ETextureFormat_RGBA8));
        EXPECT_EQ(nullptr, m_scene.createTexture2DBuffer(4, 4, 4, ETextureFormat_RGBA8));
        // non-square
        EXPECT_EQ(nullptr, m_scene.createTexture2DBuffer(5, 8, 4, ETextureFormat_RGBA8));
        EXPECT_EQ(nullptr, m_scene.createTexture2DBuffer(5, 8, 4, ETextureFormat_RGBA8));
        // non-square with NPOT texture
        EXPECT_EQ(nullptr, m_scene.createTexture2DBuffer(4, 6, 4, ETextureFormat_RGBA8));
        EXPECT_EQ(nullptr, m_scene.createTexture2DBuffer(4, 4, 6, ETextureFormat_RGBA8));
    }

    TEST_F(ATexture2DBuffer, PropagatesDataUpdatesToInternalScene)
    {
        Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(2, 3, 4, ETextureFormat_RGBA8);
        const ramses_internal::TextureBufferHandle textureBufferHandle = textureBuffer.impl.getTextureBufferHandle();

        // update mipLevel = 0
        EXPECT_EQ(StatusOK, textureBuffer.setData(reinterpret_cast<const char*>(std::array<uint32_t, 4>{ {12, 23, 34, 56} }.data()), 0, 0, 0, 2, 2));
        const uint32_t* textureBufferDataMip0 = reinterpret_cast<const uint32_t*>(this->m_scene.impl.getIScene().getTextureBuffer(textureBufferHandle).mipMaps[0].data.data());

        EXPECT_EQ(12u, textureBufferDataMip0[0]);
        EXPECT_EQ(23u, textureBufferDataMip0[1]);
        EXPECT_EQ(34u, textureBufferDataMip0[3 * 1 + 0]);
        EXPECT_EQ(56u, textureBufferDataMip0[3 * 1 + 1]);

        // update mipLevel = 1
        EXPECT_EQ(StatusOK, textureBuffer.setData(reinterpret_cast<const char*>(std::array<uint32_t, 1>{ {78} }.data()), 1, 0, 0, 1, 1));
        const uint32_t* textureBufferDataMip1 = reinterpret_cast<const uint32_t*>(this->m_scene.impl.getIScene().getTextureBuffer(textureBufferHandle).mipMaps[1].data.data());

        EXPECT_EQ(78u, textureBufferDataMip1[0]);
    }

    TEST_F(ATexture2DBuffer, CanGetDataUpdates)
    {
        Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(2, 3, 4, ETextureFormat_RGBA8);

        // update mipLevel = 0
        EXPECT_EQ(StatusOK, textureBuffer.setData(reinterpret_cast<const char*>(std::array<uint32_t, 4>{ {12, 23, 34, 56} }.data()), 0, 0, 0, 2, 2));

        std::array<uint32_t, 12> mipLevel0RetrievedData;
        textureBuffer.getMipLevelData(0u, reinterpret_cast<char*>(mipLevel0RetrievedData.data()), textureBuffer.getMipLevelDataSizeInBytes(0u));

        EXPECT_EQ(12u, mipLevel0RetrievedData[0]);
        EXPECT_EQ(23u, mipLevel0RetrievedData[1]);
        EXPECT_EQ(34u, mipLevel0RetrievedData[3 * 1 + 0]);
        EXPECT_EQ(56u, mipLevel0RetrievedData[3 * 1 + 1]);

        // update mipLevel = 1
        EXPECT_EQ(StatusOK, textureBuffer.setData(reinterpret_cast<const char*>(std::array<uint32_t, 1>{ {78} }.data()), 1, 0, 0, 1, 1));

        std::array<uint32_t, 2> mipLevel1RetrievedData;
        textureBuffer.getMipLevelData(1u, reinterpret_cast<char*>(mipLevel1RetrievedData.data()), textureBuffer.getMipLevelDataSizeInBytes(1u));

        EXPECT_EQ(78u, mipLevel1RetrievedData[0]);
    }

    TEST_F(ATexture2DBuffer, GetsDataUpToBufferSize)
    {
        Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(2, 3, 4, ETextureFormat_RGBA8);

        EXPECT_EQ(StatusOK, textureBuffer.setData(reinterpret_cast<const char*>(std::array<uint32_t, 4>{ {12, 23, 34, 56} }.data()), 0, 0, 0, 2, 2));
        std::array<uint32_t, 2> mipLevel0RetrievedData;
        textureBuffer.getMipLevelData(0u, reinterpret_cast<char*>(&mipLevel0RetrievedData), sizeof(uint32_t) * 2u);
        EXPECT_EQ(12u, mipLevel0RetrievedData[0]);
        EXPECT_EQ(23u, mipLevel0RetrievedData[1]);

        EXPECT_EQ(StatusOK, textureBuffer.setData(reinterpret_cast<const char*>(std::array<uint32_t, 2>{ {78, 87} }.data()), 1, 0, 0, 1, 2));
        uint32_t mipLevel1RetrievedData;
        textureBuffer.getMipLevelData(1u, reinterpret_cast<char*>(&mipLevel1RetrievedData), sizeof(uint32_t));
        EXPECT_EQ(78u, mipLevel1RetrievedData);
    }

    TEST_F(ATexture2DBuffer, CanNotBeUpdatedWithEmptySubregion)
    {
        Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(2, 3, 4, ETextureFormat_RGBA8);

        EXPECT_NE(StatusOK, textureBuffer.setData(reinterpret_cast<const char*>(std::array<uint32_t, 14>().data()), 0, 0, 0, 0, 0));
        EXPECT_NE(StatusOK, textureBuffer.setData(reinterpret_cast<const char*>(std::array<uint32_t, 14>().data()), 0, 2, 2, 0, 0));
        EXPECT_NE(StatusOK, textureBuffer.setData(reinterpret_cast<const char*>(std::array<uint32_t, 14>().data()), 1, 0, 0, 0, 0));
        EXPECT_NE(StatusOK, textureBuffer.setData(reinterpret_cast<const char*>(std::array<uint32_t, 14>().data()), 1, 1, 1, 0, 0));
    }

    TEST_F(ATexture2DBuffer, CanNotBeUpdatedWithDataSizeBiggerThanMaximumSize)
    {
        Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(2, 3, 4, ETextureFormat_RGBA8);

        // offset exceeds total size
        EXPECT_NE(StatusOK, textureBuffer.setData(reinterpret_cast<const char*>(std::array<uint32_t, 14>().data()), 0, 3, 0, 1, 1));
        EXPECT_NE(StatusOK, textureBuffer.setData(reinterpret_cast<const char*>(std::array<uint32_t, 14>().data()), 0, 0, 4, 1, 1));

        // width/height exceed total size
        EXPECT_NE(StatusOK, textureBuffer.setData(reinterpret_cast<const char*>(std::array<uint32_t, 14>().data()), 0, 0, 0, 3+1, 1));
        EXPECT_NE(StatusOK, textureBuffer.setData(reinterpret_cast<const char*>(std::array<uint32_t, 14>().data()), 0, 0, 0, 1, 4 + 1));

        // offset + width/height exceed total size
        EXPECT_NE(StatusOK, textureBuffer.setData(reinterpret_cast<const char*>(std::array<uint32_t, 14>().data()), 0, 1, 0, 3, 1));
        EXPECT_NE(StatusOK, textureBuffer.setData(reinterpret_cast<const char*>(std::array<uint32_t, 14>().data()), 0, 0, 1, 1, 4));

        // second mipmap
        EXPECT_NE(StatusOK, textureBuffer.setData(reinterpret_cast<const char*>(std::array<uint32_t, 14>().data()), 0, 1, 0, 3, 1));
        EXPECT_NE(StatusOK, textureBuffer.setData(reinterpret_cast<const char*>(std::array<uint32_t, 14>().data()), 0, 0, 1, 1, 4));
    }

    TEST_F(ATexture2DBuffer, CanNotBeUpdatedForUnexistingMipMapLevel)
    {
        Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(2, 3, 4, ETextureFormat_RGBA8);
        EXPECT_NE(StatusOK, textureBuffer.setData(reinterpret_cast<const char*>(std::array<uint32_t, 4>().data()), 3, 0, 0, 1, 1));
    }

    TEST_F(ATexture2DBuffer, RetrievesMipMapCount)
    {
        EXPECT_EQ(1u, m_scene.createTexture2DBuffer(1, 1, 1, ETextureFormat_R8)->getMipLevelCount());
        EXPECT_EQ(2u, m_scene.createTexture2DBuffer(2, 8, 4, ETextureFormat_R8)->getMipLevelCount());
        EXPECT_EQ(4u, m_scene.createTexture2DBuffer(4, 8, 4, ETextureFormat_R8)->getMipLevelCount());
    }

    TEST_F(ATexture2DBuffer, RetrievesMipMapSize)
    {
        const Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(4, 8, 4, ETextureFormat_R8);
        uint32_t width = 0u;
        uint32_t height = 0u;
        EXPECT_EQ(StatusOK, textureBuffer.getMipLevelSize(0, width, height));
        EXPECT_EQ(8u, width);
        EXPECT_EQ(4u, height);
        EXPECT_EQ(StatusOK, textureBuffer.getMipLevelSize(1, width, height));
        EXPECT_EQ(4u, width);
        EXPECT_EQ(2u, height);
        EXPECT_EQ(StatusOK, textureBuffer.getMipLevelSize(2, width, height));
        EXPECT_EQ(2u, width);
        EXPECT_EQ(1u, height);
        EXPECT_EQ(StatusOK, textureBuffer.getMipLevelSize(3, width, height));
        EXPECT_EQ(1u, width);
        EXPECT_EQ(1u, height);
    }

    TEST_F(ATexture2DBuffer, FailsToRetrieveMipMapSizeForInvalidMipLevel)
    {
        const Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(4, 8, 4, ETextureFormat_R8);
        uint32_t width = 0u;
        uint32_t height = 0u;
        EXPECT_NE(StatusOK, textureBuffer.getMipLevelSize(4, width, height));
    }
}
