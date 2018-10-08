//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
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

    TYPED_TEST(AScene, CreatesStreamTextureAndCheckItsValidity)
    {
        EXPECT_EQ(0u, this->m_scene.getStreamTextureCount());

        const uint32_t streamSource(12);
        const ResourceContentHash fallbackTextureHash(123, 0);
        StreamTextureHandle textureHandle = this->m_scene.allocateStreamTexture(streamSource, fallbackTextureHash);

        EXPECT_NE(StreamTextureHandle::Invalid(), textureHandle);
        EXPECT_TRUE(this->m_scene.isStreamTextureAllocated(textureHandle));
        EXPECT_EQ(1u, this->m_scene.getStreamTextureCount());
        EXPECT_EQ(fallbackTextureHash, this->m_scene.getStreamTexture(textureHandle).fallbackTexture);
        EXPECT_EQ(streamSource, this->m_scene.getStreamTexture(textureHandle).source);
    }

    TYPED_TEST(AScene, SetsStreamTextureForceFallbackImageSetting)
    {
        const uint32_t streamSource(12);
        const ResourceContentHash fallbackTextureHash(123, 0);
        StreamTextureHandle textureHandle = this->m_scene.allocateStreamTexture(streamSource, fallbackTextureHash);

        // by default no force
        EXPECT_FALSE(this->m_scene.getStreamTexture(textureHandle).forceFallbackTexture);

        this->m_scene.setForceFallbackImage(textureHandle, true);
        EXPECT_TRUE(this->m_scene.getStreamTexture(textureHandle).forceFallbackTexture);

        this->m_scene.setForceFallbackImage(textureHandle, false);
        EXPECT_FALSE(this->m_scene.getStreamTexture(textureHandle).forceFallbackTexture);
    }

    TYPED_TEST(AScene, DeleteStreamTextureAndCheckScene)
    {
        StreamTextureHandle textureHandle = this->m_scene.allocateStreamTexture(0, ResourceContentHash(123u, 0));

        EXPECT_NE(StreamTextureHandle::Invalid(), textureHandle);
        EXPECT_TRUE(this->m_scene.isStreamTextureAllocated(textureHandle));

        this->m_scene.releaseStreamTexture(textureHandle);

        EXPECT_FALSE(this->m_scene.isStreamTextureAllocated(textureHandle));
    }
}
