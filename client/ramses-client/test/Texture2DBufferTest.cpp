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
    };

    TEST_F(ATexture2DBuffer, IsAllocatedOnInternalSceneAfterCreation)
    {
        Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 3, 4, 2);
        const ramses_internal::TextureBufferHandle textureBufferHandle = textureBuffer.impl.getTextureBufferHandle();

        EXPECT_TRUE(textureBufferHandle.isValid());
        EXPECT_TRUE(this->m_scene.impl.getIScene().isTextureBufferAllocated(textureBufferHandle));
    }

    TEST_F(ATexture2DBuffer, CanGetTexelFormat)
    {
        Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 3, 4, 2);

        EXPECT_EQ(ETextureFormat::RGBA8, textureBuffer.getTexelFormat());
    }

    TEST_F(ATexture2DBuffer, PropagatesItsPropertiesToInternalScene)
    {
        Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 3, 4, 2);
        const ramses_internal::TextureBufferHandle textureBufferHandle = textureBuffer.impl.getTextureBufferHandle();
        const ramses_internal::TextureBuffer& internalTexBuffer = this->m_scene.impl.getIScene().getTextureBuffer(textureBufferHandle);

        EXPECT_EQ(ramses_internal::ETextureFormat::RGBA8, internalTexBuffer.textureFormat);
        ASSERT_EQ(2u, internalTexBuffer.mipMaps.size());
        EXPECT_EQ(3u, internalTexBuffer.mipMaps[0].width);
        EXPECT_EQ(4u, internalTexBuffer.mipMaps[0].height);
        EXPECT_EQ(1u, internalTexBuffer.mipMaps[1].width);
        EXPECT_EQ(2u, internalTexBuffer.mipMaps[1].height);
    }

    TEST_F(ATexture2DBuffer, CanBeCreatedSoThatTheLastMipLevelHasSize1x1)
    {
        // square
        EXPECT_NE(nullptr, m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 1, 1, 1));
        EXPECT_NE(nullptr, m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 4, 4, 3));
        // non-square
        EXPECT_NE(nullptr, m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 8, 4, 4));
        EXPECT_NE(nullptr, m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 4, 8, 4));
        // non-square with NPOT texture
        EXPECT_NE(nullptr, m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 6, 4, 3));
        EXPECT_NE(nullptr, m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 4, 6, 3));
    }

    TEST_F(ATexture2DBuffer, CanNotBeCreatedIfMipLevelsWouldResultInTwoConsecutiveMipsWithSize1x1)
    {
        // square
        EXPECT_EQ(nullptr, m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 1, 1, 2));
        EXPECT_EQ(nullptr, m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 4, 4, 4));
        // non-square
        EXPECT_EQ(nullptr, m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 8, 4, 5));
        EXPECT_EQ(nullptr, m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 8, 4, 5));
        // non-square with NPOT texture
        EXPECT_EQ(nullptr, m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 6, 4, 4));
        EXPECT_EQ(nullptr, m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 4, 6, 4));
    }

    TEST_F(ATexture2DBuffer, PropagatesDataUpdatesToInternalScene)
    {
        Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 3, 4, 2);
        const ramses_internal::TextureBufferHandle textureBufferHandle = textureBuffer.impl.getTextureBufferHandle();

        // update mipLevel = 0
        EXPECT_EQ(StatusOK, textureBuffer.updateData(0, 0, 0, 2, 2, std::array<uint32_t, 4>{ {12, 23, 34, 56} }.data()));
        const uint32_t* textureBufferDataMip0 = reinterpret_cast<const uint32_t*>(this->m_scene.impl.getIScene().getTextureBuffer(textureBufferHandle).mipMaps[0].data.data());

        EXPECT_EQ(12u, textureBufferDataMip0[0]);
        EXPECT_EQ(23u, textureBufferDataMip0[1]);
        EXPECT_EQ(34u, textureBufferDataMip0[3 * 1 + 0]);
        EXPECT_EQ(56u, textureBufferDataMip0[3 * 1 + 1]);

        // update mipLevel = 1
        EXPECT_EQ(StatusOK, textureBuffer.updateData(1, 0, 0, 1, 1, std::array<uint32_t, 1>{ {78} }.data()));
        const uint32_t* textureBufferDataMip1 = reinterpret_cast<const uint32_t*>(this->m_scene.impl.getIScene().getTextureBuffer(textureBufferHandle).mipMaps[1].data.data());

        EXPECT_EQ(78u, textureBufferDataMip1[0]);
    }

    TEST_F(ATexture2DBuffer, CanGetDataUpdates)
    {
        Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 3, 4, 2);

        // update mipLevel = 0
        EXPECT_EQ(StatusOK, textureBuffer.updateData(0, 0, 0, 2, 2, std::array<uint32_t, 4>{ {12, 23, 34, 56} }.data()));

        std::array<uint32_t, 12> mipLevel0RetrievedData;
        textureBuffer.getMipLevelData(0u, reinterpret_cast<char*>(mipLevel0RetrievedData.data()), textureBuffer.getMipLevelDataSizeInBytes(0u));

        EXPECT_EQ(12u, mipLevel0RetrievedData[0]);
        EXPECT_EQ(23u, mipLevel0RetrievedData[1]);
        EXPECT_EQ(34u, mipLevel0RetrievedData[3 * 1 + 0]);
        EXPECT_EQ(56u, mipLevel0RetrievedData[3 * 1 + 1]);

        // update mipLevel = 1
        EXPECT_EQ(StatusOK, textureBuffer.updateData(1, 0, 0, 1, 1, std::array<uint32_t, 1>{ {78} }.data()));

        std::array<uint32_t, 2> mipLevel1RetrievedData;
        textureBuffer.getMipLevelData(1u, reinterpret_cast<char*>(mipLevel1RetrievedData.data()), textureBuffer.getMipLevelDataSizeInBytes(1u));

        EXPECT_EQ(78u, mipLevel1RetrievedData[0]);
    }

    TEST_F(ATexture2DBuffer, GetsDataUpToBufferSize)
    {
        Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 3, 4, 2);

        EXPECT_EQ(StatusOK, textureBuffer.updateData(0, 0, 0, 2, 2, std::array<uint32_t, 4>{ {12, 23, 34, 56} }.data()));
        std::array<uint32_t, 2> mipLevel0RetrievedData;
        textureBuffer.getMipLevelData(0u, reinterpret_cast<char*>(&mipLevel0RetrievedData), sizeof(uint32_t) * 2u);
        EXPECT_EQ(12u, mipLevel0RetrievedData[0]);
        EXPECT_EQ(23u, mipLevel0RetrievedData[1]);

        EXPECT_EQ(StatusOK, textureBuffer.updateData(1, 0, 0, 1, 2, std::array<uint32_t, 2>{ {78, 87} }.data()));
        uint32_t mipLevel1RetrievedData;
        textureBuffer.getMipLevelData(1u, reinterpret_cast<char*>(&mipLevel1RetrievedData), sizeof(uint32_t));
        EXPECT_EQ(78u, mipLevel1RetrievedData);
    }

    TEST_F(ATexture2DBuffer, CanNotBeUpdatedWithEmptySubregion)
    {
        Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 3, 4, 2);

        EXPECT_NE(StatusOK, textureBuffer.updateData(0, 0, 0, 0, 0, std::array<uint32_t, 14>().data()));
        EXPECT_NE(StatusOK, textureBuffer.updateData(0, 2, 2, 0, 0, std::array<uint32_t, 14>().data()));
        EXPECT_NE(StatusOK, textureBuffer.updateData(1, 0, 0, 0, 0, std::array<uint32_t, 14>().data()));
        EXPECT_NE(StatusOK, textureBuffer.updateData(1, 1, 1, 0, 0, std::array<uint32_t, 14>().data()));
    }

    TEST_F(ATexture2DBuffer, CanNotBeUpdatedWithDataSizeBiggerThanMaximumSize)
    {
        Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 3, 4, 2);

        // offset exceeds total size
        EXPECT_NE(StatusOK, textureBuffer.updateData(0, 3, 0, 1, 1, std::array<uint32_t, 14>().data()));
        EXPECT_NE(StatusOK, textureBuffer.updateData(0, 0, 4, 1, 1, std::array<uint32_t, 14>().data()));

        // width/height exceed total size
        EXPECT_NE(StatusOK, textureBuffer.updateData(0, 0, 0, 3+1, 1, std::array<uint32_t, 14>().data()));
        EXPECT_NE(StatusOK, textureBuffer.updateData(0, 0, 0, 1, 4 + 1, std::array<uint32_t, 14>().data()));

        // offset + width/height exceed total size
        EXPECT_NE(StatusOK, textureBuffer.updateData(0, 1, 0, 3, 1, std::array<uint32_t, 14>().data()));
        EXPECT_NE(StatusOK, textureBuffer.updateData(0, 0, 1, 1, 4, std::array<uint32_t, 14>().data()));

        // second mipmap
        EXPECT_NE(StatusOK, textureBuffer.updateData(0, 1, 0, 3, 1, std::array<uint32_t, 14>().data()));
        EXPECT_NE(StatusOK, textureBuffer.updateData(0, 0, 1, 1, 4, std::array<uint32_t, 14>().data()));
    }

    TEST_F(ATexture2DBuffer, CanNotBeUpdatedForUnexistingMipMapLevel)
    {
        Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 3, 4, 2);
        EXPECT_NE(StatusOK, textureBuffer.updateData(3, 0, 0, 1, 1, std::array<uint32_t, 4>().data()));
    }

    TEST_F(ATexture2DBuffer, RetrievesMipMapCount)
    {
        EXPECT_EQ(1u, m_scene.createTexture2DBuffer(ETextureFormat::R8, 1, 1, 1)->getMipLevelCount());
        EXPECT_EQ(2u, m_scene.createTexture2DBuffer(ETextureFormat::R8, 8, 4, 2)->getMipLevelCount());
        EXPECT_EQ(4u, m_scene.createTexture2DBuffer(ETextureFormat::R8, 8, 4, 4)->getMipLevelCount());
    }

    TEST_F(ATexture2DBuffer, RetrievesMipMapSize)
    {
        const Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(ETextureFormat::R8, 8, 4, 4);
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
        const Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(ETextureFormat::R8, 8, 4, 4);
        uint32_t width = 0u;
        uint32_t height = 0u;
        EXPECT_NE(StatusOK, textureBuffer.getMipLevelSize(4, width, height));
    }

    TEST_F(ATexture2DBuffer, CanBeValidated)
    {
        Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 4, 4, 1);
        EXPECT_EQ(StatusOK, textureBuffer.updateData(0, 0, 0, 2, 2, std::array<uint32_t, 4>{ {12, 23, 34, 56} }.data()));
        m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Linear, ETextureSamplingMethod_Linear, textureBuffer);
        EXPECT_EQ(StatusOK, textureBuffer.validate());
    }

    TEST_F(ATexture2DBuffer, ReportsWarningIfNotUsedInSampler)
    {
        Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 4, 4, 1);
        EXPECT_EQ(StatusOK, textureBuffer.updateData(0, 0, 0, 2, 2, std::array<uint32_t, 4>{ {12, 23, 34, 56} }.data()));
        EXPECT_NE(StatusOK, textureBuffer.validate());
    }

    TEST_F(ATexture2DBuffer, ReportsWarningIfUsedInSamplerButNotInitialized)
    {
        Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 4, 4, 1);
        m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Linear, ETextureSamplingMethod_Linear, textureBuffer);
        EXPECT_NE(StatusOK, textureBuffer.validate());
    }
}
