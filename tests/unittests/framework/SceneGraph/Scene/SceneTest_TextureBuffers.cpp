//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "SceneTest.h"

using namespace testing;

namespace ramses::internal
{
    TYPED_TEST_SUITE(AScene, SceneTypes);

    TYPED_TEST(AScene, CreatesTextureBuffer)
    {
        EXPECT_EQ(0u, this->m_scene.getTextureBufferCount());

        const TextureBufferHandle textureBuffer = this->m_scene.allocateTextureBuffer(EPixelStorageFormat::R16F, { {1, 1} }, {});

        EXPECT_EQ(1u, this->m_scene.getTextureBufferCount());
        EXPECT_TRUE(this->m_scene.isTextureBufferAllocated(textureBuffer));
    }

    TYPED_TEST(AScene, ReleasesTextureBuffer)
    {
        const TextureBufferHandle textureBuffer = this->m_scene.allocateTextureBuffer(EPixelStorageFormat::R16F, { { 1, 1 } }, {});
        this->m_scene.releaseTextureBuffer(textureBuffer);

        EXPECT_FALSE(this->m_scene.isTextureBufferAllocated(textureBuffer));
    }

    TYPED_TEST(AScene, DoesNotContainTextureBufferWhichWasNotCreated)
    {
        EXPECT_FALSE(this->m_scene.isTextureBufferAllocated(TextureBufferHandle(1u)));
    }

    TYPED_TEST(AScene, StoresTextureBufferProperties)
    {
        const TextureBufferHandle handle = this->m_scene.allocateTextureBuffer(EPixelStorageFormat::R16F, { {2, 2}, { 1, 1 } }, {});
        const TextureBuffer& texBuffer = this->m_scene.getTextureBuffer(handle);
        EXPECT_EQ(EPixelStorageFormat::R16F, texBuffer.textureFormat);
        ASSERT_EQ(2u, texBuffer.mipMaps.size());
        EXPECT_EQ(2u, texBuffer.mipMaps[0].width);
        EXPECT_EQ(2u, texBuffer.mipMaps[0].height);
        EXPECT_EQ(1u, texBuffer.mipMaps[1].width);
        EXPECT_EQ(1u, texBuffer.mipMaps[1].height);
    }

    template <typename... Ts>
    std::array<std::byte, sizeof...(Ts)> make_byte_array(Ts&&... args) noexcept
    {
        return {std::byte(std::forward<Ts>(args))...};
    }

    TYPED_TEST(AScene, UpdatesTextureBufferDataPartially)
    {
        const TextureBufferHandle textureBuffer = this->m_scene.allocateTextureBuffer(EPixelStorageFormat::R8, { { 4, 4} }, {});
        const std::array<std::byte, 4> data = make_byte_array(1u, 2u, 3u, 4u);
        this->m_scene.updateTextureBuffer(textureBuffer, 0u, 0u, 0u, 2u, 2u, data.data());
        const TextureBuffer& texBuffer = this->m_scene.getTextureBuffer(textureBuffer);
        EXPECT_EQ(std::byte{1u}, texBuffer.mipMaps[0].data[0 * 4 + 0]);
        EXPECT_EQ(std::byte{2u}, texBuffer.mipMaps[0].data[0 * 4 + 1]);
        EXPECT_EQ(std::byte{3u}, texBuffer.mipMaps[0].data[1 * 4 + 0]);
        EXPECT_EQ(std::byte{4u}, texBuffer.mipMaps[0].data[1 * 4 + 1]);

        const std::array<std::byte, 4> data2 = make_byte_array(10u, 20u, 30u, 40u);
        this->m_scene.updateTextureBuffer(textureBuffer, 0u, 1u, 1u, 2u, 2u, data2.data());
        EXPECT_EQ(std::byte{1u},  texBuffer.mipMaps[0].data[0 * 4 + 0]); //stays same
        EXPECT_EQ(std::byte{2u},  texBuffer.mipMaps[0].data[0 * 4 + 1]); //stays same
        EXPECT_EQ(std::byte{3u},  texBuffer.mipMaps[0].data[1 * 4 + 0]); //stays same
        EXPECT_EQ(std::byte{10u}, texBuffer.mipMaps[0].data[1 * 4 + 1]);
        EXPECT_EQ(std::byte{20u}, texBuffer.mipMaps[0].data[1 * 4 + 2]);
        EXPECT_EQ(std::byte{30u}, texBuffer.mipMaps[0].data[2 * 4 + 1]);
        EXPECT_EQ(std::byte{40u}, texBuffer.mipMaps[0].data[2 * 4 + 2]);
    }

    TYPED_TEST(AScene, TextureBufferDoesNotHaveUsedRegionsUponCreation)
    {
        EXPECT_EQ(0u, this->m_scene.getTextureBufferCount());

        const TextureBufferHandle textureBuffer = this->m_scene.allocateTextureBuffer(EPixelStorageFormat::R16F, { {2, 2}, {1, 1} }, {});
        const TextureBuffer& texBuffer = this->m_scene.getTextureBuffer(textureBuffer);

        ASSERT_EQ(2u, texBuffer.mipMaps.size());
        EXPECT_EQ(Quad(), texBuffer.mipMaps[0].usedRegion);
        EXPECT_EQ(Quad(), texBuffer.mipMaps[1].usedRegion);
    }

    TYPED_TEST(AScene, UpdatesUsedRegionsWhenTextureBufferDataGetsUpdated)
    {
        const TextureBufferHandle textureBuffer = this->m_scene.allocateTextureBuffer(EPixelStorageFormat::R8, { { 4, 4}, {2, 2} }, {});
        const std::array<std::byte, 16> data = { std::byte{0u} };

        const TextureBuffer& texBuffer = this->m_scene.getTextureBuffer(textureBuffer);

        this->m_scene.updateTextureBuffer(textureBuffer, 0u, 0u, 0u, 2u, 2u, data.data());
        EXPECT_EQ(Quad(0, 0, 2, 2), texBuffer.mipMaps[0].usedRegion);
        EXPECT_EQ(Quad(), texBuffer.mipMaps[1].usedRegion); //stays same

        this->m_scene.updateTextureBuffer(textureBuffer, 0u, 0u, 1u, 2u, 2u, data.data());
        EXPECT_EQ(Quad(0, 0, 2, 3), texBuffer.mipMaps[0].usedRegion);
        EXPECT_EQ(Quad(), texBuffer.mipMaps[1].usedRegion); //stays same

        this->m_scene.updateTextureBuffer(textureBuffer, 1u, 0u, 1u, 1u, 1u, data.data());
        EXPECT_EQ(Quad(0, 0, 2, 3), texBuffer.mipMaps[0].usedRegion); //stays same
        EXPECT_EQ(Quad(0, 1, 1, 1), texBuffer.mipMaps[1].usedRegion);
    }
}
