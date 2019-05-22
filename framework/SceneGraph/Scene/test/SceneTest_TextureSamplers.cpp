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

    static const TextureSamplerStates SamplerStates = { EWrapMethod::Clamp, EWrapMethod::Repeat, EWrapMethod::RepeatMirrored, ESamplingMethod::Linear, ESamplingMethod::Nearest, 4u };

    TYPED_TEST(AScene, TextureSamplerCreated)
    {
        EXPECT_EQ(0u, this->m_scene.getTextureSamplerCount());

        const TextureSamplerHandle sampler = this->m_scene.allocateTextureSampler({ {}, ResourceContentHash(0,1) });

        EXPECT_EQ(1u, this->m_scene.getTextureSamplerCount());
        EXPECT_TRUE(this->m_scene.isTextureSamplerAllocated(sampler));
    }

    TYPED_TEST(AScene, TextureSamplerReleased)
    {
        const TextureSamplerHandle sampler = this->m_scene.allocateTextureSampler({ {}, ResourceContentHash(0,1) });
        this->m_scene.releaseTextureSampler(sampler);

        EXPECT_EQ(1u, this->m_scene.getTextureSamplerCount());
        EXPECT_FALSE(this->m_scene.isTextureSamplerAllocated(sampler));
    }

    TYPED_TEST(AScene, CalledContainsTextureSamplerOnNotExistingTextureSampler)
    {
        EXPECT_FALSE(this->m_scene.isTextureSamplerAllocated(TextureSamplerHandle(1)));
    }

    TYPED_TEST(AScene, AllocatedTextureSamplerHasGivenStates)
    {
        const TextureSamplerHandle sampler = this->m_scene.allocateTextureSampler({ SamplerStates, ResourceContentHash(0,1) });

        EXPECT_EQ(SamplerStates.m_addressModeU, this->m_scene.getTextureSampler(sampler).states.m_addressModeU);
        EXPECT_EQ(SamplerStates.m_addressModeV, this->m_scene.getTextureSampler(sampler).states.m_addressModeV);
        EXPECT_EQ(SamplerStates.m_addressModeR, this->m_scene.getTextureSampler(sampler).states.m_addressModeR);
        EXPECT_EQ(SamplerStates.m_minSamplingMode, this->m_scene.getTextureSampler(sampler).states.m_minSamplingMode);
        EXPECT_EQ(SamplerStates.m_magSamplingMode, this->m_scene.getTextureSampler(sampler).states.m_magSamplingMode);
        EXPECT_EQ(SamplerStates.m_anisotropyLevel, this->m_scene.getTextureSampler(sampler).states.m_anisotropyLevel);
    }

    TYPED_TEST(AScene, AllocatedTextureSamplerHasGivenReferencesToResources_TextureResrouce)
    {
        const ResourceContentHash resource(1u, 2u);
        const TextureSamplerHandle sampler = this->m_scene.allocateTextureSampler({ SamplerStates, resource });

        EXPECT_TRUE(this->m_scene.getTextureSampler(sampler).textureResource.isValid());
        EXPECT_EQ(InvalidMemoryHandle, this->m_scene.getTextureSampler(sampler).contentHandle);
        EXPECT_EQ(TextureSampler::ContentType::ClientTexture, this->m_scene.getTextureSampler(sampler).contentType);
    }

    TYPED_TEST(AScene, AllocatedTextureSamplerHasGivenReferencesToResources_RenderBuffer)
    {
        const RenderBufferHandle buffer(3u);
        const TextureSamplerHandle sampler = this->m_scene.allocateTextureSampler({ SamplerStates, buffer });

        EXPECT_FALSE(this->m_scene.getTextureSampler(sampler).textureResource.isValid());
        EXPECT_NE(InvalidMemoryHandle, this->m_scene.getTextureSampler(sampler).contentHandle);
        EXPECT_EQ(TextureSampler::ContentType::RenderBuffer, this->m_scene.getTextureSampler(sampler).contentType);
    }

    TYPED_TEST(AScene, AllocatedTextureSamplerHasGivenReferencesToResources_StreamTexture)
    {
        const StreamTextureHandle streamTex(this->m_scene.allocateStreamTexture(10u, ResourceContentHash(30u, 40u)));
        const TextureSamplerHandle sampler = this->m_scene.allocateTextureSampler({ SamplerStates, streamTex });

        EXPECT_FALSE(this->m_scene.getTextureSampler(sampler).textureResource.isValid());
        EXPECT_NE(InvalidMemoryHandle, this->m_scene.getTextureSampler(sampler).contentHandle);
        EXPECT_EQ(TextureSampler::ContentType::StreamTexture, this->m_scene.getTextureSampler(sampler).contentType);
    }

    TYPED_TEST(AScene, AllocatedTextureSamplerHasGivenReferencesToResources_TextureBuffer)
    {
        const TextureBufferHandle textureBuffer(1432u);
        const TextureSamplerHandle sampler = this->m_scene.allocateTextureSampler({ SamplerStates, textureBuffer });

        EXPECT_FALSE(this->m_scene.getTextureSampler(sampler).textureResource.isValid());
        EXPECT_NE(InvalidMemoryHandle, this->m_scene.getTextureSampler(sampler).contentHandle);
        EXPECT_EQ(TextureSampler::ContentType::TextureBuffer, this->m_scene.getTextureSampler(sampler).contentType);
    }
}
