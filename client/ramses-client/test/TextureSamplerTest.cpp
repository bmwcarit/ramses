//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "ClientTestUtils.h"
#include "CreationHelper.h"
#include "ramses-client-api/TextureSampler.h"
#include "ramses-client-api/RenderBuffer.h"
#include "ramses-client-api/RenderTarget.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Texture3D.h"
#include "ramses-client-api/TextureCube.h"
#include "ramses-client-api/StreamTexture.h"
#include "ramses-client-api/RenderBuffer.h"
#include "ramses-client-api/RenderTargetDescription.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "StreamTextureImpl.h"
#include "TextureSamplerImpl.h"
#include "RenderBufferImpl.h"
#include "Texture2DImpl.h"

using namespace testing;

namespace ramses
{
    class TextureSamplerTest : public LocalTestClientWithScene, public ::testing::Test
    {
    public:
        TextureSamplerTest()
        {
        }
    };

    TEST_F(TextureSamplerTest, createSamplerForTexture2D)
    {
        const Texture2D& texture2D = createObject<Texture2D>("testTexture2D");
        const TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Trilinear, texture2D, 1u, "testSampler2D");

        ASSERT_NE(static_cast<TextureSampler*>(0), sampler);
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureSamplingMethod_Trilinear, sampler->getSamplingMethod());
        EXPECT_EQ(1u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType_Texture2D, sampler->getTextureType());

        const ramses_internal::TextureSamplerHandle samplerHandle = sampler->impl.getTextureSamplerHandle();
        const auto texHash = texture2D.impl.getLowlevelResourceHash();
        ASSERT_TRUE(m_internalScene.isTextureSamplerAllocated(samplerHandle));
        EXPECT_EQ(ramses_internal::TextureSampler::ContentType::ClientTexture, m_internalScene.getTextureSampler(samplerHandle).contentType);
        EXPECT_EQ(texHash, m_internalScene.getTextureSampler(samplerHandle).textureResource);
    }

    TEST_F(TextureSamplerTest, createSamplerForTexture2DWithAnisotropy)
    {
        const Texture2D& texture2D = createObject<Texture2D>("testTexture2D");
        const TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Trilinear, texture2D, 16u, "testSampler2D");

        ASSERT_NE(static_cast<TextureSampler*>(0), sampler);
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureSamplingMethod_Trilinear, sampler->getSamplingMethod());
        EXPECT_EQ(16u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType_Texture2D, sampler->getTextureType());
    }

    TEST_F(TextureSamplerTest, createSamplerForTexture3D)
    {
        const Texture3D& texture3D = createObject<Texture3D>("testTexture3D");
        const TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureAddressMode_Mirror, ETextureSamplingMethod_Trilinear, texture3D, "testSampler3D");

        ASSERT_NE(static_cast<TextureSampler*>(0), sampler);
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureAddressMode_Mirror, sampler->getWrapRMode());
        EXPECT_EQ(ETextureSamplingMethod_Trilinear, sampler->getSamplingMethod());
        EXPECT_EQ(1u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType_Texture3D, sampler->getTextureType());
    }

    TEST_F(TextureSamplerTest, createSamplerForTextureCube)
    {
        const TextureCube& textureCube = createObject<TextureCube>("testTextureCube");
        const TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Trilinear, textureCube, 1u, "testSamplerCube");

        ASSERT_NE(static_cast<TextureSampler*>(0), sampler);
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureSamplingMethod_Trilinear, sampler->getSamplingMethod());
        EXPECT_EQ(1u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType_TextureCube, sampler->getTextureType());
    }

    TEST_F(TextureSamplerTest, createSamplerForTextureCubeWithAnisotropy)
    {
        const TextureCube& textureCube = createObject<TextureCube>("testTextureCube");
        const TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Trilinear, textureCube, 16u, "testSamplerCube");

        ASSERT_NE(static_cast<TextureSampler*>(0), sampler);
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureSamplingMethod_Trilinear, sampler->getSamplingMethod());
        EXPECT_EQ(16u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType_TextureCube, sampler->getTextureType());
    }

    TEST_F(TextureSamplerTest, createSamplerForRenderBuffer)
    {
        const RenderBuffer& renderBuffer = createObject<RenderBuffer>();
        TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Trilinear, renderBuffer, 16u);

        ASSERT_NE(static_cast<TextureSampler*>(0), sampler);
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureSamplingMethod_Trilinear, sampler->getSamplingMethod());
        EXPECT_EQ(16u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType_RenderBuffer, sampler->getTextureType());

        const ramses_internal::TextureSamplerHandle samplerHandle = sampler->impl.getTextureSamplerHandle();
        const ramses_internal::RenderBufferHandle renderBufferHandle = renderBuffer.impl.getRenderBufferHandle();
        ASSERT_TRUE(m_internalScene.isTextureSamplerAllocated(samplerHandle));
        EXPECT_EQ(ramses_internal::TextureSampler::ContentType::RenderBuffer, m_internalScene.getTextureSampler(samplerHandle).contentType);
        EXPECT_EQ(renderBufferHandle.asMemoryHandle(), m_internalScene.getTextureSampler(samplerHandle).contentHandle);
    }

    TEST_F(TextureSamplerTest, createSamplerForStreamTexture)
    {
        const StreamTexture& streamTexture = createObject<StreamTexture>();
        const TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Trilinear, streamTexture);

        ASSERT_NE(static_cast<TextureSampler*>(0), sampler);
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureSamplingMethod_Trilinear, sampler->getSamplingMethod());
        EXPECT_EQ(1u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType_StreamTexture, sampler->getTextureType());


        const ramses_internal::TextureSamplerHandle samplerHandle = sampler->impl.getTextureSamplerHandle();
        const ramses_internal::StreamTextureHandle streamTextureHandle = streamTexture.impl.getHandle();

        ASSERT_TRUE(m_internalScene.isTextureSamplerAllocated(samplerHandle));
        EXPECT_EQ(ramses_internal::TextureSampler::ContentType::StreamTexture, m_internalScene.getTextureSampler(samplerHandle).contentType);
        EXPECT_EQ(streamTextureHandle.asMemoryHandle(), m_internalScene.getTextureSampler(samplerHandle).contentHandle);
    }

    TEST_F(TextureSamplerTest, reportsErrorWhenValidatedWithInvalidTexture2D)
    {
        const Texture2D& texture2D = createObject<Texture2D>("testTexture2D");
        const TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Trilinear, texture2D, 1u, "testSampler2D");
        ASSERT_TRUE(NULL != sampler);
        EXPECT_EQ(StatusOK, sampler->validate());

        EXPECT_EQ(StatusOK, client.destroy(texture2D));
        EXPECT_NE(StatusOK, sampler->validate());
    }

    TEST_F(TextureSamplerTest, reportsErrorWhenValidatedWithInvalidTexture3D)
    {
        const Texture3D& texture3D = createObject<Texture3D>("testTexture3D");
        const TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureAddressMode_Mirror, ETextureSamplingMethod_Trilinear, texture3D, "testSampler3D");
        ASSERT_TRUE(NULL != sampler);
        EXPECT_EQ(StatusOK, sampler->validate());

        EXPECT_EQ(StatusOK, client.destroy(texture3D));
        EXPECT_NE(StatusOK, sampler->validate());
    }

    TEST_F(TextureSamplerTest, reportsErrorWhenValidatedWithInvalidTextureCube)
    {
        const TextureCube& textureCube = createObject<TextureCube>("testTextureCube");
        const TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Trilinear, textureCube, 1u, "testSamplerCube");
        ASSERT_TRUE(NULL != sampler);
        EXPECT_EQ(StatusOK, sampler->validate());

        EXPECT_EQ(StatusOK, client.destroy(textureCube));
        EXPECT_NE(StatusOK, sampler->validate());
    }

    TEST_F(TextureSamplerTest, reportsErrorWhenValidatedWithInvalidRenderBuffer)
    {
        RenderBuffer& renderBuffer = createObject<RenderBuffer>();
        RenderTargetDescription rtDesc;
        rtDesc.addRenderBuffer(renderBuffer);
        RenderTarget* rt = this->m_scene.createRenderTarget(rtDesc);
        RenderPass& renderPass = createObject<RenderPass>();
        OrthographicCamera* orthoCam = this->m_scene.createOrthographicCamera("camera");
        orthoCam->setFrustum(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f);
        orthoCam->setViewport(0, 0, 100, 200);
        renderPass.setCamera(*orthoCam);
        renderPass.setRenderTarget(rt);

        TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Trilinear, renderBuffer);
        ASSERT_TRUE(NULL != sampler);
        EXPECT_EQ(StatusOK, sampler->validate());

        EXPECT_EQ(StatusOK, m_scene.destroy(renderBuffer));
        EXPECT_NE(StatusOK, sampler->validate());
    }

    TEST_F(TextureSamplerTest, reportsErrorWhenValidatedWithInvalidStreamTexture)
    {
        StreamTexture& streamTexture = createObject<StreamTexture>();
        TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Trilinear, streamTexture);
        ASSERT_TRUE(NULL != sampler);
        EXPECT_EQ(StatusOK, sampler->validate());

        EXPECT_EQ(StatusOK, m_scene.destroy(streamTexture));
        EXPECT_NE(StatusOK, sampler->validate());
    }
}
