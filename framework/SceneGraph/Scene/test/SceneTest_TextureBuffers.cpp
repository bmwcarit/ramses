//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "SceneTest.h"

using namespace testing;

namespace ramses_internal
{
    TYPED_TEST_CASE(AScene, SceneTypes);

    TYPED_TEST(AScene, CreatesTextureBuffer)
    {
        EXPECT_EQ(0u, this->m_scene.getTextureBufferCount());

        const TextureBufferHandle textureBuffer = this->m_scene.allocateTextureBuffer(ETextureFormat_R16F, { {1, 1} });

        EXPECT_EQ(1u, this->m_scene.getTextureBufferCount());
        EXPECT_TRUE(this->m_scene.isTextureBufferAllocated(textureBuffer));
    }

    TYPED_TEST(AScene, ReleasesTextureBuffer)
    {
        const TextureBufferHandle textureBuffer = this->m_scene.allocateTextureBuffer(ETextureFormat_R16F, { { 1, 1 } });
        this->m_scene.releaseTextureBuffer(textureBuffer);

        EXPECT_FALSE(this->m_scene.isTextureBufferAllocated(textureBuffer));
    }

    TYPED_TEST(AScene, DoesNotContainTextureBufferWhichWasNotCreated)
    {
        EXPECT_FALSE(this->m_scene.isTextureBufferAllocated(TextureBufferHandle(1u)));
    }

    TYPED_TEST(AScene, StoresTextureBufferProperties)
    {
        const TextureBufferHandle handle = this->m_scene.allocateTextureBuffer(ETextureFormat_R16F, { {2, 2}, { 1, 1 } });
        const TextureBuffer& texBuffer = this->m_scene.getTextureBuffer(handle);
        EXPECT_EQ(ETextureFormat_R16F, texBuffer.textureFormat);
        EXPECT_EQ(2u, texBuffer.mipMapDimensions.size());
        EXPECT_EQ(2u, texBuffer.mipMapDimensions[0].width);
        EXPECT_EQ(2u, texBuffer.mipMapDimensions[0].height);
        EXPECT_EQ(1u, texBuffer.mipMapDimensions[1].width);
        EXPECT_EQ(1u, texBuffer.mipMapDimensions[1].height);
    }

    TYPED_TEST(AScene, UpdatesTextureBufferDataPartially)
    {
        const TextureBufferHandle textureBuffer = this->m_scene.allocateTextureBuffer(ETextureFormat_R8, { { 4, 4} });
        const Byte data[] = {1u, 2u, 3u, 4u};
        this->m_scene.updateTextureBuffer(textureBuffer, 0u, 0u, 0u, 2u, 2u, data);
        const TextureBuffer& texBuffer = this->m_scene.getTextureBuffer(textureBuffer);
        EXPECT_EQ(1u, texBuffer.mipMapData[0][0 * 4 + 0]);
        EXPECT_EQ(2u, texBuffer.mipMapData[0][0 * 4 + 1]);
        EXPECT_EQ(3u, texBuffer.mipMapData[0][1 * 4 + 0]);
        EXPECT_EQ(4u, texBuffer.mipMapData[0][1 * 4 + 1]);

        const Byte data2[] = { 10u, 20u, 30u, 40u };
        this->m_scene.updateTextureBuffer(textureBuffer, 0u, 1u, 1u, 2u, 2u, data2);
        EXPECT_EQ(1u,  texBuffer.mipMapData[0][0 * 4 + 0]); //stays same
        EXPECT_EQ(2u,  texBuffer.mipMapData[0][0 * 4 + 1]); //stays same
        EXPECT_EQ(3u,  texBuffer.mipMapData[0][1 * 4 + 0]); //stays same
        EXPECT_EQ(10u, texBuffer.mipMapData[0][1 * 4 + 1]);
        EXPECT_EQ(20u, texBuffer.mipMapData[0][1 * 4 + 2]);
        EXPECT_EQ(30u, texBuffer.mipMapData[0][2 * 4 + 1]);
        EXPECT_EQ(40u, texBuffer.mipMapData[0][2 * 4 + 2]);
    }
}
