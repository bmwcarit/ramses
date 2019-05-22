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
#include "ramses-client-api/Texture2DBuffer.h"
#include "ramses-client-api/StreamTexture.h"
#include "ramses-client-api/RenderBuffer.h"
#include "ramses-client-api/RenderTargetDescription.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "StreamTextureImpl.h"
#include "TextureSamplerImpl.h"
#include "RenderBufferImpl.h"
#include "Texture2DImpl.h"
#include "Texture3DImpl.h"
#include "TextureCubeImpl.h"
#include "Texture2DBufferImpl.h"

using namespace testing;

namespace ramses
{
    class TextureSamplerTest : public LocalTestClientWithScene, public ::testing::Test
    {
    };

    TEST_F(TextureSamplerTest, createSamplerForTexture2D)
    {
        const Texture2D& texture2D = createObject<Texture2D>("testTexture2D");
        const TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Linear, ETextureSamplingMethod_Linear, texture2D, 1u, "testSampler2D");

        ASSERT_NE(static_cast<TextureSampler*>(0), sampler);
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureSamplingMethod_Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod_Linear, sampler->getMagSamplingMethod());
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
        const TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Linear, ETextureSamplingMethod_Linear, texture2D, 16u, "testSampler2D");

        ASSERT_NE(static_cast<TextureSampler*>(0), sampler);
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureSamplingMethod_Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod_Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(16u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType_Texture2D, sampler->getTextureType());
    }

    TEST_F(TextureSamplerTest, createSamplerForTexture3D)
    {
        const Texture3D& texture3D = createObject<Texture3D>("testTexture3D");
        const TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureAddressMode_Mirror, ETextureSamplingMethod_Linear, ETextureSamplingMethod_Linear, texture3D, "testSampler3D");

        ASSERT_NE(static_cast<TextureSampler*>(0), sampler);
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureAddressMode_Mirror, sampler->getWrapRMode());
        EXPECT_EQ(ETextureSamplingMethod_Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod_Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(1u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType_Texture3D, sampler->getTextureType());
    }

    TEST_F(TextureSamplerTest, createSamplerForTextureCube)
    {
        const TextureCube& textureCube = createObject<TextureCube>("testTextureCube");
        const TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Linear, ETextureSamplingMethod_Linear, textureCube, 1u, "testSamplerCube");

        ASSERT_NE(static_cast<TextureSampler*>(0), sampler);
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureSamplingMethod_Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod_Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(1u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType_TextureCube, sampler->getTextureType());
    }

    TEST_F(TextureSamplerTest, createSamplerForTextureCubeWithAnisotropy)
    {
        const TextureCube& textureCube = createObject<TextureCube>("testTextureCube");
        const TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Linear, ETextureSamplingMethod_Linear, textureCube, 16u, "testSamplerCube");

        ASSERT_NE(static_cast<TextureSampler*>(0), sampler);
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureSamplingMethod_Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod_Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(16u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType_TextureCube, sampler->getTextureType());
    }

    TEST_F(TextureSamplerTest, createSamplerForRenderBuffer)
    {
        const RenderBuffer& renderBuffer = createObject<RenderBuffer>();
        TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Linear, ETextureSamplingMethod_Linear, renderBuffer, 16u);

        ASSERT_NE(static_cast<TextureSampler*>(0), sampler);
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureSamplingMethod_Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod_Linear, sampler->getMagSamplingMethod());
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
        const TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Linear, ETextureSamplingMethod_Linear, streamTexture);

        ASSERT_NE(static_cast<TextureSampler*>(0), sampler);
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureSamplingMethod_Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod_Linear, sampler->getMagSamplingMethod());
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
        const TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Linear, ETextureSamplingMethod_Linear, texture2D, 1u, "testSampler2D");
        ASSERT_TRUE(NULL != sampler);
        EXPECT_EQ(StatusOK, sampler->validate());

        EXPECT_EQ(StatusOK, client.destroy(texture2D));
        EXPECT_NE(StatusOK, sampler->validate());
    }

    TEST_F(TextureSamplerTest, reportsErrorWhenValidatedWithInvalidTexture3D)
    {
        const Texture3D& texture3D = createObject<Texture3D>("testTexture3D");
        const TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureAddressMode_Mirror, ETextureSamplingMethod_Linear, ETextureSamplingMethod_Linear, texture3D, "testSampler3D");
        ASSERT_TRUE(NULL != sampler);
        EXPECT_EQ(StatusOK, sampler->validate());

        EXPECT_EQ(StatusOK, client.destroy(texture3D));
        EXPECT_NE(StatusOK, sampler->validate());
    }

    TEST_F(TextureSamplerTest, reportsErrorWhenValidatedWithInvalidTextureCube)
    {
        const TextureCube& textureCube = createObject<TextureCube>("testTextureCube");
        const TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Linear, ETextureSamplingMethod_Linear, textureCube, 1u, "testSamplerCube");
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

        TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Linear, ETextureSamplingMethod_Linear, renderBuffer);
        ASSERT_TRUE(NULL != sampler);
        EXPECT_EQ(StatusOK, sampler->validate());

        EXPECT_EQ(StatusOK, m_scene.destroy(renderBuffer));
        EXPECT_NE(StatusOK, sampler->validate());
    }

    TEST_F(TextureSamplerTest, reportsErrorWhenValidatedWithInvalidStreamTexture)
    {
        StreamTexture& streamTexture = createObject<StreamTexture>();
        TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Linear, ETextureSamplingMethod_Linear, streamTexture);
        ASSERT_TRUE(NULL != sampler);
        EXPECT_EQ(StatusOK, sampler->validate());

        EXPECT_EQ(StatusOK, m_scene.destroy(streamTexture));
        EXPECT_NE(StatusOK, sampler->validate());
    }

    TEST_F(TextureSamplerTest, setTextureDataToTexture2D)
    {
        const Texture3D& texture3D = createObject<Texture3D>();
        TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Linear, ETextureSamplingMethod_Linear, texture3D);
        ASSERT_NE(nullptr, sampler);

        const Texture2D& texture2D = createObject<Texture2D>();
        EXPECT_EQ(StatusOK, sampler->setTextureData(texture2D));

        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapRMode());
        EXPECT_EQ(ETextureSamplingMethod_Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod_Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(1u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType_Texture2D, sampler->getTextureType());

        const ramses_internal::TextureSamplerHandle samplerHandle = sampler->impl.getTextureSamplerHandle();
        ASSERT_TRUE(m_internalScene.isTextureSamplerAllocated(samplerHandle));
        EXPECT_EQ(1u, m_internalScene.getTextureSamplerCount());
        EXPECT_EQ(ramses_internal::TextureSampler::ContentType::ClientTexture, m_internalScene.getTextureSampler(samplerHandle).contentType);
        EXPECT_EQ(texture2D.impl.getLowlevelResourceHash(), m_internalScene.getTextureSampler(samplerHandle).textureResource);
    }

    TEST_F(TextureSamplerTest, setTextureDataFromTexture3DToAnotherTexture3D)
    {
        const Texture3D& texture3Doriginal = createObject<Texture3D>();
        TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Linear, ETextureSamplingMethod_Linear, texture3Doriginal);
        ASSERT_NE(nullptr, sampler);

        const Texture3D& texture3D = createObject<Texture3D>();
        EXPECT_EQ(StatusOK, sampler->setTextureData(texture3D));

        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapRMode());
        EXPECT_EQ(ETextureSamplingMethod_Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod_Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(1u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType_Texture3D, sampler->getTextureType());

        const ramses_internal::TextureSamplerHandle samplerHandle = sampler->impl.getTextureSamplerHandle();
        ASSERT_TRUE(m_internalScene.isTextureSamplerAllocated(samplerHandle));
        EXPECT_EQ(1u, m_internalScene.getTextureSamplerCount());
        EXPECT_EQ(ramses_internal::TextureSampler::ContentType::ClientTexture, m_internalScene.getTextureSampler(samplerHandle).contentType);
        EXPECT_EQ(texture3D.impl.getLowlevelResourceHash(), m_internalScene.getTextureSampler(samplerHandle).textureResource);
    }

    TEST_F(TextureSamplerTest, failsToSetTextureDataFromTexture2DToTexture3D)
    {
        const Texture2D& texture2D = createObject<Texture2D>();
        TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Linear, ETextureSamplingMethod_Linear, texture2D, 1u);
        ASSERT_NE(nullptr, sampler);

        const Texture3D& texture3D = createObject<Texture3D>();
        EXPECT_NE(StatusOK, sampler->setTextureData(texture3D));
    }

    TEST_F(TextureSamplerTest, setTextureDataToTextureCube)
    {
        const Texture3D& texture3D = createObject<Texture3D>();
        TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Linear, ETextureSamplingMethod_Linear, texture3D);
        ASSERT_NE(nullptr, sampler);

        const TextureCube& textureCube = createObject<TextureCube>();
        EXPECT_EQ(StatusOK, sampler->setTextureData(textureCube));

        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapRMode());
        EXPECT_EQ(ETextureSamplingMethod_Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod_Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(1u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType_TextureCube, sampler->getTextureType());

        const ramses_internal::TextureSamplerHandle samplerHandle = sampler->impl.getTextureSamplerHandle();
        ASSERT_TRUE(m_internalScene.isTextureSamplerAllocated(samplerHandle));
        EXPECT_EQ(1u, m_internalScene.getTextureSamplerCount());
        EXPECT_EQ(ramses_internal::TextureSampler::ContentType::ClientTexture, m_internalScene.getTextureSampler(samplerHandle).contentType);
        EXPECT_EQ(textureCube.impl.getLowlevelResourceHash(), m_internalScene.getTextureSampler(samplerHandle).textureResource);
    }

    TEST_F(TextureSamplerTest, setTextureDataToTexture2DBuffer)
    {
        const Texture2D& texture2D = createObject<Texture2D>();
        TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Linear, ETextureSamplingMethod_Linear, texture2D, 1u);
        ASSERT_NE(nullptr, sampler);

        const Texture2DBuffer& textureBuffer = createObject<Texture2DBuffer>();
        EXPECT_EQ(StatusOK, sampler->setTextureData(textureBuffer));

        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureSamplingMethod_Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod_Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(1u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType_Texture2DBuffer, sampler->getTextureType());

        const ramses_internal::TextureSamplerHandle samplerHandle = sampler->impl.getTextureSamplerHandle();
        ASSERT_TRUE(m_internalScene.isTextureSamplerAllocated(samplerHandle));
        EXPECT_EQ(1u, m_internalScene.getTextureSamplerCount());
        EXPECT_EQ(ramses_internal::TextureSampler::ContentType::TextureBuffer, m_internalScene.getTextureSampler(samplerHandle).contentType);
        EXPECT_EQ(textureBuffer.impl.getTextureBufferHandle().asMemoryHandle(), m_internalScene.getTextureSampler(samplerHandle).contentHandle);
    }

    TEST_F(TextureSamplerTest, setTextureDataToRenderBuffer)
    {
        const Texture2D& texture2D = createObject<Texture2D>();
        TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Linear, ETextureSamplingMethod_Linear, texture2D, 1u);
        ASSERT_NE(nullptr, sampler);

        const RenderBuffer& buffer = createObject<RenderBuffer>();
        EXPECT_EQ(StatusOK, sampler->setTextureData(buffer));

        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureSamplingMethod_Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod_Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(1u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType_RenderBuffer, sampler->getTextureType());

        const ramses_internal::TextureSamplerHandle samplerHandle = sampler->impl.getTextureSamplerHandle();
        ASSERT_TRUE(m_internalScene.isTextureSamplerAllocated(samplerHandle));
        EXPECT_EQ(1u, m_internalScene.getTextureSamplerCount());
        EXPECT_EQ(ramses_internal::TextureSampler::ContentType::RenderBuffer, m_internalScene.getTextureSampler(samplerHandle).contentType);
        EXPECT_EQ(buffer.impl.getRenderBufferHandle().asMemoryHandle(), m_internalScene.getTextureSampler(samplerHandle).contentHandle);
    }

    TEST_F(TextureSamplerTest, setTextureDataToStreamTexture)
    {
        const Texture2D& texture2D = createObject<Texture2D>();
        TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Linear, ETextureSamplingMethod_Linear, texture2D, 1u);
        ASSERT_NE(nullptr, sampler);

        const StreamTexture& texture = createObject<StreamTexture>();
        EXPECT_EQ(StatusOK, sampler->setTextureData(texture));

        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureSamplingMethod_Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod_Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(1u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType_StreamTexture, sampler->getTextureType());

        const ramses_internal::TextureSamplerHandle samplerHandle = sampler->impl.getTextureSamplerHandle();
        ASSERT_TRUE(m_internalScene.isTextureSamplerAllocated(samplerHandle));
        EXPECT_EQ(1u, m_internalScene.getTextureSamplerCount());
        EXPECT_EQ(ramses_internal::TextureSampler::ContentType::StreamTexture, m_internalScene.getTextureSampler(samplerHandle).contentType);
        EXPECT_EQ(texture.impl.getHandle().asMemoryHandle(), m_internalScene.getTextureSampler(samplerHandle).contentHandle);
    }

    TEST_F(TextureSamplerTest, failsToSetTextureDataToSamplerMarkedAsConsumer)
    {
        const auto& texture2D = createObject<Texture2D>();
        TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Linear, ETextureSamplingMethod_Linear, texture2D);
        ASSERT_NE(nullptr, sampler);

        ASSERT_EQ(StatusOK, this->m_scene.createTextureConsumer(*sampler, 666u));

        const auto& texture2Dother = createObject<Texture2D>();
        EXPECT_NE(StatusOK, sampler->setTextureData(texture2Dother));
        const auto& textureCube = createObject<TextureCube>();
        EXPECT_NE(StatusOK, sampler->setTextureData(textureCube));
        const auto& textureBuffer = createObject<Texture2DBuffer>();
        EXPECT_NE(StatusOK, sampler->setTextureData(textureBuffer));
        const auto& renderBuffer = createObject<RenderBuffer>();
        EXPECT_NE(StatusOK, sampler->setTextureData(renderBuffer));
        const auto& streamTexture = createObject<StreamTexture>();
        EXPECT_NE(StatusOK, sampler->setTextureData(streamTexture));
    }

    TEST_F(TextureSamplerTest, failsToSetTextureDataFromOtherSceneThanSampler)
    {
        const auto& texture2D = createObject<Texture2D>();
        TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Linear, ETextureSamplingMethod_Linear, texture2D);
        ASSERT_NE(nullptr, sampler);

        RamsesClient otherClient("other", this->framework);
        CreationHelper creationHelper(otherClient.createScene(666u), nullptr, &otherClient);

        const auto& texture2Dother = *creationHelper.createObjectOfType<Texture2D>(nullptr);
        EXPECT_NE(StatusOK, sampler->setTextureData(texture2Dother));
        const auto& textureCube = *creationHelper.createObjectOfType<TextureCube>(nullptr);
        EXPECT_NE(StatusOK, sampler->setTextureData(textureCube));
        const auto& textureBuffer = *creationHelper.createObjectOfType<Texture2DBuffer>(nullptr);
        EXPECT_NE(StatusOK, sampler->setTextureData(textureBuffer));
        const auto& renderBuffer = *creationHelper.createObjectOfType<RenderBuffer>(nullptr);
        EXPECT_NE(StatusOK, sampler->setTextureData(renderBuffer));
        const auto& streamTexture = *creationHelper.createObjectOfType<StreamTexture>(nullptr);
        EXPECT_NE(StatusOK, sampler->setTextureData(streamTexture));
    }
}
