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
#include "impl/SceneImpl.h"
#include "ramses/client/ramses-utils.h"
#include "internal/SceneGraph/Scene/ClientScene.h"
#include "ramses/client/Texture2DBuffer.h"
#include "impl/Texture2DBufferImpl.h"
#include "UnsafeTestMemoryHelpers.h"

using namespace testing;
using namespace ramses::internal;

namespace ramses::internal
{
    class ATexture2DBuffer : public LocalTestClientWithScene, public testing::Test
    {
    };

    TEST_F(ATexture2DBuffer, IsAllocatedOnInternalSceneAfterCreation)
    {
        Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 3, 4, 2);
        const ramses::internal::TextureBufferHandle textureBufferHandle = textureBuffer.impl().getTextureBufferHandle();

        EXPECT_TRUE(textureBufferHandle.isValid());
        EXPECT_TRUE(this->m_scene.impl().getIScene().isTextureBufferAllocated(textureBufferHandle));
    }

    TEST_F(ATexture2DBuffer, CanGetTexelFormat)
    {
        Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 3, 4, 2);

        EXPECT_EQ(ETextureFormat::RGBA8, textureBuffer.getTexelFormat());
    }

    TEST_F(ATexture2DBuffer, PropagatesItsPropertiesToInternalScene)
    {
        Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 3, 4, 2);
        const ramses::internal::TextureBufferHandle textureBufferHandle = textureBuffer.impl().getTextureBufferHandle();
        const ramses::internal::TextureBuffer& internalTexBuffer = this->m_scene.impl().getIScene().getTextureBuffer(textureBufferHandle);

        EXPECT_EQ(ramses::internal::EPixelStorageFormat::RGBA8, internalTexBuffer.textureFormat);
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
        const ramses::internal::TextureBufferHandle textureBufferHandle = textureBuffer.impl().getTextureBufferHandle();

        // update mipLevel = 0
        EXPECT_TRUE(textureBuffer.updateData(0, 0, 0, 2, 2, UnsafeTestMemoryHelpers::ConvertToBytes({12, 23, 34, 56})));
        const std::byte* textureBufferDataMip0 = this->m_scene.impl().getIScene().getTextureBuffer(textureBufferHandle).mipMaps[0].data.data();

        EXPECT_EQ(12u, UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint32_t>(textureBufferDataMip0, 0));
        EXPECT_EQ(23u, UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint32_t>(textureBufferDataMip0, 1));
        EXPECT_EQ(34u, UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint32_t>(textureBufferDataMip0, 3 * 1 + 0));
        EXPECT_EQ(56u, UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint32_t>(textureBufferDataMip0, 3 * 1 + 1));

        // update mipLevel = 1
        EXPECT_TRUE(textureBuffer.updateData(1, 0, 0, 1, 1, UnsafeTestMemoryHelpers::ConvertToBytes({78})));
        const std::byte* textureBufferDataMip1 = this->m_scene.impl().getIScene().getTextureBuffer(textureBufferHandle).mipMaps[1].data.data();
        EXPECT_EQ(78u, UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint32_t>(textureBufferDataMip1, 0));
    }

    TEST_F(ATexture2DBuffer, CanGetDataUpdates)
    {
        Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 3, 4, 2);

        // update mipLevel = 0
        EXPECT_TRUE(textureBuffer.updateData(0, 0, 0, 2, 2, UnsafeTestMemoryHelpers::ConvertToBytes({12, 23, 34, 56})));

        std::array<uint32_t, 12> mipLevel0RetrievedData{};
        textureBuffer.getMipLevelData(0u, mipLevel0RetrievedData.data(), textureBuffer.getMipLevelDataSizeInBytes(0u));

        EXPECT_EQ(12u, mipLevel0RetrievedData[0]);
        EXPECT_EQ(23u, mipLevel0RetrievedData[1]);
        EXPECT_EQ(34u, mipLevel0RetrievedData[3 * 1 + 0]);
        EXPECT_EQ(56u, mipLevel0RetrievedData[3 * 1 + 1]);

        // update mipLevel = 1
        EXPECT_TRUE(textureBuffer.updateData(1, 0, 0, 1, 1, UnsafeTestMemoryHelpers::ConvertToBytes({78})));

        std::array<uint32_t, 2> mipLevel1RetrievedData{};
        textureBuffer.getMipLevelData(1u, mipLevel1RetrievedData.data(), textureBuffer.getMipLevelDataSizeInBytes(1u));

        EXPECT_EQ(78u, mipLevel1RetrievedData[0]);
    }

    TEST_F(ATexture2DBuffer, GetsDataUpToBufferSize)
    {
        Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 3, 4, 2);

        EXPECT_TRUE(textureBuffer.updateData(0, 0, 0, 2, 2, UnsafeTestMemoryHelpers::ConvertToBytes({12, 23, 34, 56})));
        std::array<uint32_t, 2> mipLevel0RetrievedData{};
        textureBuffer.getMipLevelData(0u, &mipLevel0RetrievedData, sizeof(uint32_t) * 2u);
        EXPECT_EQ(12u, mipLevel0RetrievedData[0]);
        EXPECT_EQ(23u, mipLevel0RetrievedData[1]);

        EXPECT_TRUE(textureBuffer.updateData(1, 0, 0, 1, 2, UnsafeTestMemoryHelpers::ConvertToBytes({78, 87})));
        uint32_t mipLevel1RetrievedData{};
        textureBuffer.getMipLevelData(1u, &mipLevel1RetrievedData, sizeof(uint32_t));
        EXPECT_EQ(78u, mipLevel1RetrievedData);
    }

    TEST_F(ATexture2DBuffer, CanNotBeUpdatedWithEmptySubregion)
    {
        Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 3, 4, 2);

        EXPECT_FALSE(textureBuffer.updateData(0, 0, 0, 0, 0, std::array<std::byte, 64>().data()));
        EXPECT_FALSE(textureBuffer.updateData(0, 2, 2, 0, 0, std::array<std::byte, 64>().data()));
        EXPECT_FALSE(textureBuffer.updateData(1, 0, 0, 0, 0, std::array<std::byte, 64>().data()));
        EXPECT_FALSE(textureBuffer.updateData(1, 1, 1, 0, 0, std::array<std::byte, 64>().data()));
    }

    TEST_F(ATexture2DBuffer, CanNotBeUpdatedWithDataSizeBiggerThanMaximumSize)
    {
        Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 3, 4, 2);

        // offset exceeds total size
        EXPECT_FALSE(textureBuffer.updateData(0, 3, 0, 1, 1, std::array<std::byte, 64>().data()));
        EXPECT_FALSE(textureBuffer.updateData(0, 0, 4, 1, 1, std::array<std::byte, 64>().data()));

        // width/height exceed total size
        EXPECT_FALSE(textureBuffer.updateData(0, 0, 0, 3+1, 1, std::array<std::byte, 64>().data()));
        EXPECT_FALSE(textureBuffer.updateData(0, 0, 0, 1, 4 + 1, std::array<std::byte, 64>().data()));

        // offset + width/height exceed total size
        EXPECT_FALSE(textureBuffer.updateData(0, 1, 0, 3, 1, std::array<std::byte, 64>().data()));
        EXPECT_FALSE(textureBuffer.updateData(0, 0, 1, 1, 4, std::array<std::byte, 64>().data()));

        // second mipmap
        EXPECT_FALSE(textureBuffer.updateData(0, 1, 0, 3, 1, std::array<std::byte, 64>().data()));
        EXPECT_FALSE(textureBuffer.updateData(0, 0, 1, 1, 4, std::array<std::byte, 64>().data()));
    }

    TEST_F(ATexture2DBuffer, CanNotBeUpdatedForUnexistingMipMapLevel)
    {
        Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 3, 4, 2);
        EXPECT_FALSE(textureBuffer.updateData(3, 0, 0, 1, 1, std::array<std::byte, 64>().data()));
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
        EXPECT_TRUE(textureBuffer.getMipLevelSize(0, width, height));
        EXPECT_EQ(8u, width);
        EXPECT_EQ(4u, height);
        EXPECT_TRUE(textureBuffer.getMipLevelSize(1, width, height));
        EXPECT_EQ(4u, width);
        EXPECT_EQ(2u, height);
        EXPECT_TRUE(textureBuffer.getMipLevelSize(2, width, height));
        EXPECT_EQ(2u, width);
        EXPECT_EQ(1u, height);
        EXPECT_TRUE(textureBuffer.getMipLevelSize(3, width, height));
        EXPECT_EQ(1u, width);
        EXPECT_EQ(1u, height);
    }

    TEST_F(ATexture2DBuffer, FailsToRetrieveMipMapSizeForInvalidMipLevel)
    {
        const Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(ETextureFormat::R8, 8, 4, 4);
        uint32_t width = 0u;
        uint32_t height = 0u;
        EXPECT_FALSE(textureBuffer.getMipLevelSize(4, width, height));
    }

    TEST_F(ATexture2DBuffer, CanBeValidated)
    {
        Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 4, 4, 1);
        EXPECT_TRUE(textureBuffer.updateData(0, 0, 0, 2, 2, UnsafeTestMemoryHelpers::ConvertToBytes({12, 23, 34, 56})));
        m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, textureBuffer);
        ValidationReport report;
        textureBuffer.validate(report);
        EXPECT_FALSE(report.hasIssue());
    }

    TEST_F(ATexture2DBuffer, ReportsWarningIfNotUsedInSampler)
    {
        Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 4, 4, 1);
        EXPECT_TRUE(textureBuffer.updateData(0, 0, 0, 2, 2, UnsafeTestMemoryHelpers::ConvertToBytes({12, 23, 34, 56})));
        ValidationReport report;
        textureBuffer.validate(report);
        EXPECT_TRUE(report.hasIssue());
    }

    TEST_F(ATexture2DBuffer, ReportsWarningIfUsedInSamplerButNotInitialized)
    {
        Texture2DBuffer& textureBuffer = *m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 4, 4, 1);
        m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, textureBuffer);
        ValidationReport report;
        textureBuffer.validate(report);
        EXPECT_TRUE(report.hasIssue());
    }
}
