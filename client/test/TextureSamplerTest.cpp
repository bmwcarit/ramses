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
#include "ramses-client-api/TextureSamplerMS.h"
#include "ramses-client-api/TextureSamplerExternal.h"
#include "ramses-client-api/RenderBuffer.h"
#include "ramses-client-api/RenderTarget.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Texture3D.h"
#include "ramses-client-api/TextureCube.h"
#include "ramses-client-api/Texture2DBuffer.h"
#include "ramses-client-api/RenderBuffer.h"
#include "ramses-client-api/RenderTargetDescription.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/OrthographicCamera.h"
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
        const TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, texture2D, 1u, "testSampler2D");

        ASSERT_NE(static_cast<TextureSampler*>(nullptr), sampler);
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(1u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType::Texture2D, sampler->getTextureType());

        const ramses_internal::TextureSamplerHandle samplerHandle = sampler->m_impl.getTextureSamplerHandle();
        const auto texHash = texture2D.m_impl.getLowlevelResourceHash();
        ASSERT_TRUE(m_internalScene.isTextureSamplerAllocated(samplerHandle));
        EXPECT_EQ(ramses_internal::TextureSampler::ContentType::ClientTexture, m_internalScene.getTextureSampler(samplerHandle).contentType);
        EXPECT_EQ(texHash, m_internalScene.getTextureSampler(samplerHandle).textureResource);
    }

    TEST_F(TextureSamplerTest, createSamplerForTexture2DWithAnisotropy)
    {
        const Texture2D& texture2D = createObject<Texture2D>("testTexture2D");
        const TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, texture2D, 16u, "testSampler2D");

        ASSERT_NE(static_cast<TextureSampler*>(nullptr), sampler);
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(16u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType::Texture2D, sampler->getTextureType());
    }

    TEST_F(TextureSamplerTest, createSamplerForTexture3D)
    {
        const Texture3D& texture3D = createObject<Texture3D>("testTexture3D");
        const TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureAddressMode::Mirror, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, texture3D, "testSampler3D");

        ASSERT_NE(static_cast<TextureSampler*>(nullptr), sampler);
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureAddressMode::Mirror, sampler->getWrapRMode());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(1u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType::Texture3D, sampler->getTextureType());
    }

    TEST_F(TextureSamplerTest, createSamplerForTextureCube)
    {
        const TextureCube& textureCube = createObject<TextureCube>("testTextureCube");
        const TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, textureCube, 1u, "testSamplerCube");

        ASSERT_NE(static_cast<TextureSampler*>(nullptr), sampler);
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(1u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType::TextureCube, sampler->getTextureType());
    }

    TEST_F(TextureSamplerTest, createSamplerForTextureCubeWithAnisotropy)
    {
        const TextureCube& textureCube = createObject<TextureCube>("testTextureCube");
        const TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, textureCube, 16u, "testSamplerCube");

        ASSERT_NE(static_cast<TextureSampler*>(nullptr), sampler);
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(16u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType::TextureCube, sampler->getTextureType());
    }

    TEST_F(TextureSamplerTest, createSamplerForRenderBuffer)
    {
        const RenderBuffer& renderBuffer = createObject<RenderBuffer>();
        TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, renderBuffer, 16u);

        ASSERT_NE(static_cast<TextureSampler*>(nullptr), sampler);
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(16u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType::RenderBuffer, sampler->getTextureType());

        const ramses_internal::TextureSamplerHandle samplerHandle = sampler->m_impl.getTextureSamplerHandle();
        const ramses_internal::RenderBufferHandle renderBufferHandle = renderBuffer.m_impl.getRenderBufferHandle();
        ASSERT_TRUE(m_internalScene.isTextureSamplerAllocated(samplerHandle));
        EXPECT_EQ(ramses_internal::TextureSampler::ContentType::RenderBuffer, m_internalScene.getTextureSampler(samplerHandle).contentType);
        EXPECT_EQ(renderBufferHandle.asMemoryHandle(), m_internalScene.getTextureSampler(samplerHandle).contentHandle);
    }

    TEST_F(TextureSamplerTest, createSamplerMS)
    {
        const RenderBuffer& renderBuffer = createObject<RenderBuffer>();
        TextureSamplerMS* sampler = this->m_scene.createTextureSamplerMS(renderBuffer, "renderBuffer");

        ASSERT_NE(static_cast<TextureSamplerMS*>(nullptr), sampler);

        const ramses_internal::TextureSamplerHandle samplerHandle = sampler->m_impl.getTextureSamplerHandle();
        const ramses_internal::RenderBufferHandle renderBufferHandle = renderBuffer.m_impl.getRenderBufferHandle();
        ASSERT_TRUE(m_internalScene.isTextureSamplerAllocated(samplerHandle));
        EXPECT_EQ(ramses_internal::TextureSampler::ContentType::RenderBufferMS, m_internalScene.getTextureSampler(samplerHandle).contentType);
        EXPECT_EQ(renderBufferHandle.asMemoryHandle(), m_internalScene.getTextureSampler(samplerHandle).contentHandle);
    }

    TEST_F(TextureSamplerTest, createSamplerForTextureExternal)
    {
        const TextureSamplerExternal* sampler = this->m_scene.createTextureSamplerExternal(ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, "testSampler2DExternal");

        ASSERT_NE(static_cast<TextureSamplerExternal*>(nullptr), sampler);
        const ramses_internal::TextureSamplerHandle samplerHandle = sampler->m_impl.getTextureSamplerHandle();

        ASSERT_TRUE(m_internalScene.isTextureSamplerAllocated(samplerHandle));
        const auto& samplerInternal = m_internalScene.getTextureSampler(samplerHandle);

        EXPECT_EQ(ramses_internal::TextureSampler::ContentType::ExternalTexture, samplerInternal.contentType);
        EXPECT_EQ(ramses_internal::InvalidMemoryHandle, samplerInternal.contentHandle);
        EXPECT_FALSE(samplerInternal.textureResource.isValid());

        EXPECT_EQ(ramses_internal::EWrapMethod::Clamp, samplerInternal.states.m_addressModeU);
        EXPECT_EQ(ramses_internal::EWrapMethod::Clamp, samplerInternal.states.m_addressModeV);
        EXPECT_EQ(ramses_internal::EWrapMethod::Clamp, samplerInternal.states.m_addressModeR);
        EXPECT_EQ(ramses_internal::ESamplingMethod::Linear, samplerInternal.states.m_minSamplingMode);
        EXPECT_EQ(ramses_internal::ESamplingMethod::Linear, samplerInternal.states.m_magSamplingMode);
        EXPECT_EQ(1u, samplerInternal.states.m_anisotropyLevel);
    }

    TEST_F(TextureSamplerTest, doesNotCreateSamplerForTextureExternalWithNotAllowedFilteringMethod)
    {
        EXPECT_EQ(nullptr, this->m_scene.createTextureSamplerExternal(ETextureSamplingMethod::Linear_MipMapLinear, ETextureSamplingMethod::Linear, "testSampler2DExternal"));
        EXPECT_EQ(nullptr, this->m_scene.createTextureSamplerExternal(ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear_MipMapNearest, "testSampler2DExternal"));
    }

    TEST_F(TextureSamplerTest, canCreateSamplerConsumersWithSameConsumerIdButFailValidation)
    {
        auto sampler1 = m_scene.createTextureSamplerExternal(ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear);
        const dataConsumerId_t consumerId{ 123u };
        EXPECT_EQ(StatusOK, m_scene.createTextureConsumer(*sampler1, consumerId));
        EXPECT_EQ(StatusOK, m_scene.validate());

        const auto sampler2 = m_scene.createTextureSamplerExternal(ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear);
        EXPECT_EQ(StatusOK, m_scene.createTextureConsumer(*sampler2, consumerId));
        EXPECT_NE(StatusOK, m_scene.validate());
        const std::string report = m_scene.getValidationReport(EValidationSeverity::Error);
        EXPECT_THAT(report, HasSubstr("Duplicate texture consumer ID '123'"));

        // validates fine again after duplicate removed
        EXPECT_EQ(StatusOK, m_scene.destroy(*sampler1));
        EXPECT_EQ(StatusOK, m_scene.validate());
    }

    TEST_F(TextureSamplerTest, cannotCreateSamplerForRenderBufferWithSamples)
    {
        const RenderBuffer& renderBuffer = *m_scene.createRenderBuffer(4u, 4u, ERenderBufferType::Color, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite, 4u);
        TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, renderBuffer, 16u);

        EXPECT_EQ(nullptr, sampler);
    }

    TEST_F(TextureSamplerTest, cannotCreateSamplerMSForWriteOnlyRenderBuffer)
    {
        const RenderBuffer& renderBuffer = *m_scene.createRenderBuffer(4u, 4u, ERenderBufferType::Color, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::WriteOnly, 4u);
        TextureSamplerMS* sampler = this->m_scene.createTextureSamplerMS(renderBuffer, "renderBuffer");

        EXPECT_EQ(nullptr, sampler);
    }

    TEST_F(TextureSamplerTest, reportsErrorWhenValidatedWithInvalidTexture2D)
    {
        Texture2D& texture2D = createObject<Texture2D>("testTexture2D");
        const TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, texture2D, 1u, "testSampler2D");
        ASSERT_TRUE(nullptr != sampler);
        EXPECT_EQ(StatusOK, sampler->validate());

        EXPECT_EQ(StatusOK, m_scene.destroy(texture2D));
        EXPECT_NE(StatusOK, sampler->validate());
    }

    TEST_F(TextureSamplerTest, reportsErrorWhenValidatedWithInvalidTexture3D)
    {
        Texture3D& texture3D = createObject<Texture3D>("testTexture3D");
        const TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureAddressMode::Mirror, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, texture3D, "testSampler3D");
        ASSERT_TRUE(nullptr != sampler);
        EXPECT_EQ(StatusOK, sampler->validate());

        EXPECT_EQ(StatusOK, m_scene.destroy(texture3D));
        EXPECT_NE(StatusOK, sampler->validate());
    }

    TEST_F(TextureSamplerTest, reportsErrorWhenValidatedWithInvalidTextureCube)
    {
        TextureCube& textureCube = createObject<TextureCube>("testTextureCube");
        const TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, textureCube, 1u, "testSamplerCube");
        ASSERT_TRUE(nullptr != sampler);
        EXPECT_EQ(StatusOK, sampler->validate());

        EXPECT_EQ(StatusOK, m_scene.destroy(textureCube));
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

        TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, renderBuffer);
        ASSERT_TRUE(nullptr != sampler);
        EXPECT_EQ(StatusOK, sampler->validate());

        EXPECT_EQ(StatusOK, m_scene.destroy(renderBuffer));
        EXPECT_NE(StatusOK, sampler->validate());
    }

    TEST_F(TextureSamplerTest, reportsErrorWhenValidatedWithInvalidRenderBufferMS)
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

        TextureSamplerMS* sampler = this->m_scene.createTextureSamplerMS(renderBuffer, "textureSamplerMS");
        ASSERT_TRUE(nullptr != sampler);
        EXPECT_EQ(StatusOK, sampler->validate());

        EXPECT_EQ(StatusOK, m_scene.destroy(renderBuffer));
        EXPECT_NE(StatusOK, sampler->validate());
    }

    TEST_F(TextureSamplerTest, doesNotReportErrorWhenValidatedWithExternalTexture)
    {
        TextureSamplerExternal* sampler = this->m_scene.createTextureSamplerExternal(ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear);
        ASSERT_TRUE(nullptr != sampler);
        EXPECT_EQ(StatusOK, sampler->validate());
    }

    TEST_F(TextureSamplerTest, setTextureDataToTexture2D)
    {
        const Texture3D& texture3D = createObject<Texture3D>();
        TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, texture3D);
        ASSERT_NE(nullptr, sampler);

        const Texture2D& texture2D = createObject<Texture2D>();
        EXPECT_EQ(StatusOK, sampler->setTextureData(texture2D));

        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapRMode());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(1u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType::Texture2D, sampler->getTextureType());

        const ramses_internal::TextureSamplerHandle samplerHandle = sampler->m_impl.getTextureSamplerHandle();
        ASSERT_TRUE(m_internalScene.isTextureSamplerAllocated(samplerHandle));
        EXPECT_EQ(1u, m_internalScene.getTextureSamplerCount());
        EXPECT_EQ(ramses_internal::TextureSampler::ContentType::ClientTexture, m_internalScene.getTextureSampler(samplerHandle).contentType);
        EXPECT_EQ(texture2D.m_impl.getLowlevelResourceHash(), m_internalScene.getTextureSampler(samplerHandle).textureResource);
    }

    TEST_F(TextureSamplerTest, setTextureDataFromTexture3DToAnotherTexture3D)
    {
        const Texture3D& texture3Doriginal = createObject<Texture3D>();
        TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, texture3Doriginal);
        ASSERT_NE(nullptr, sampler);

        const Texture3D& texture3D = createObject<Texture3D>();
        EXPECT_EQ(StatusOK, sampler->setTextureData(texture3D));

        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapRMode());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(1u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType::Texture3D, sampler->getTextureType());

        const ramses_internal::TextureSamplerHandle samplerHandle = sampler->m_impl.getTextureSamplerHandle();
        ASSERT_TRUE(m_internalScene.isTextureSamplerAllocated(samplerHandle));
        EXPECT_EQ(1u, m_internalScene.getTextureSamplerCount());
        EXPECT_EQ(ramses_internal::TextureSampler::ContentType::ClientTexture, m_internalScene.getTextureSampler(samplerHandle).contentType);
        EXPECT_EQ(texture3D.m_impl.getLowlevelResourceHash(), m_internalScene.getTextureSampler(samplerHandle).textureResource);
    }

    TEST_F(TextureSamplerTest, failsToSetTextureDataFromTexture2DToTexture3D)
    {
        const Texture2D& texture2D = createObject<Texture2D>();
        TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, texture2D, 1u);
        ASSERT_NE(nullptr, sampler);

        const Texture3D& texture3D = createObject<Texture3D>();
        EXPECT_NE(StatusOK, sampler->setTextureData(texture3D));
    }

    TEST_F(TextureSamplerTest, setTextureDataToTextureCube)
    {
        const Texture3D& texture3D = createObject<Texture3D>();
        TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, texture3D);
        ASSERT_NE(nullptr, sampler);

        const TextureCube& textureCube = createObject<TextureCube>();
        EXPECT_EQ(StatusOK, sampler->setTextureData(textureCube));

        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapRMode());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(1u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType::TextureCube, sampler->getTextureType());

        const ramses_internal::TextureSamplerHandle samplerHandle = sampler->m_impl.getTextureSamplerHandle();
        ASSERT_TRUE(m_internalScene.isTextureSamplerAllocated(samplerHandle));
        EXPECT_EQ(1u, m_internalScene.getTextureSamplerCount());
        EXPECT_EQ(ramses_internal::TextureSampler::ContentType::ClientTexture, m_internalScene.getTextureSampler(samplerHandle).contentType);
        EXPECT_EQ(textureCube.m_impl.getLowlevelResourceHash(), m_internalScene.getTextureSampler(samplerHandle).textureResource);
    }

    TEST_F(TextureSamplerTest, setTextureDataToTexture2DBuffer)
    {
        const Texture2D& texture2D = createObject<Texture2D>();
        TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, texture2D, 1u);
        ASSERT_NE(nullptr, sampler);

        const Texture2DBuffer& textureBuffer = createObject<Texture2DBuffer>();
        EXPECT_EQ(StatusOK, sampler->setTextureData(textureBuffer));

        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(1u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType::Texture2DBuffer, sampler->getTextureType());

        const ramses_internal::TextureSamplerHandle samplerHandle = sampler->m_impl.getTextureSamplerHandle();
        ASSERT_TRUE(m_internalScene.isTextureSamplerAllocated(samplerHandle));
        EXPECT_EQ(1u, m_internalScene.getTextureSamplerCount());
        EXPECT_EQ(ramses_internal::TextureSampler::ContentType::TextureBuffer, m_internalScene.getTextureSampler(samplerHandle).contentType);
        EXPECT_EQ(textureBuffer.m_impl.getTextureBufferHandle().asMemoryHandle(), m_internalScene.getTextureSampler(samplerHandle).contentHandle);
    }

    TEST_F(TextureSamplerTest, setTextureDataToRenderBuffer)
    {
        const Texture2D& texture2D = createObject<Texture2D>();
        TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, texture2D, 1u);
        ASSERT_NE(nullptr, sampler);

        const RenderBuffer& buffer = createObject<RenderBuffer>();
        EXPECT_EQ(StatusOK, sampler->setTextureData(buffer));

        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(1u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType::RenderBuffer, sampler->getTextureType());

        const ramses_internal::TextureSamplerHandle samplerHandle = sampler->m_impl.getTextureSamplerHandle();
        ASSERT_TRUE(m_internalScene.isTextureSamplerAllocated(samplerHandle));
        EXPECT_EQ(1u, m_internalScene.getTextureSamplerCount());
        EXPECT_EQ(ramses_internal::TextureSampler::ContentType::RenderBuffer, m_internalScene.getTextureSampler(samplerHandle).contentType);
        EXPECT_EQ(buffer.m_impl.getRenderBufferHandle().asMemoryHandle(), m_internalScene.getTextureSampler(samplerHandle).contentHandle);
    }

    TEST_F(TextureSamplerTest, failsToSetTextureDataToSamplerMarkedAsConsumer)
    {
        const auto& texture2D = createObject<Texture2D>();
        TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, texture2D);
        ASSERT_NE(nullptr, sampler);

        ASSERT_EQ(StatusOK, this->m_scene.createTextureConsumer(*sampler, dataConsumerId_t{666u}));

        const auto& texture2Dother = createObject<Texture2D>();
        EXPECT_NE(StatusOK, sampler->setTextureData(texture2Dother));
        const auto& textureCube = createObject<TextureCube>();
        EXPECT_NE(StatusOK, sampler->setTextureData(textureCube));
        const auto& textureBuffer = createObject<Texture2DBuffer>();
        EXPECT_NE(StatusOK, sampler->setTextureData(textureBuffer));
        const auto& renderBuffer = createObject<RenderBuffer>();
        EXPECT_NE(StatusOK, sampler->setTextureData(renderBuffer));
    }

    TEST_F(TextureSamplerTest, failsToSetTextureDataFromOtherSceneThanSampler)
    {
        const auto& texture2D = createObject<Texture2D>();
        TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, texture2D);
        ASSERT_NE(nullptr, sampler);

        RamsesClient& otherClient(*this->framework.createClient("other"));
        CreationHelper creationHelper(otherClient.createScene(sceneId_t(666u)), &otherClient);

        const auto& texture2Dother = *creationHelper.createObjectOfType<Texture2D>({});
        EXPECT_NE(StatusOK, sampler->setTextureData(texture2Dother));
        const auto& textureCube = *creationHelper.createObjectOfType<TextureCube>({});
        EXPECT_NE(StatusOK, sampler->setTextureData(textureCube));
        const auto& textureBuffer = *creationHelper.createObjectOfType<Texture2DBuffer>({});
        EXPECT_NE(StatusOK, sampler->setTextureData(textureBuffer));
        const auto& renderBuffer = *creationHelper.createObjectOfType<RenderBuffer>({});
        EXPECT_NE(StatusOK, sampler->setTextureData(renderBuffer));
    }
}
