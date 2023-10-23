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
#include "ramses/client/TextureSampler.h"
#include "ramses/client/TextureSamplerMS.h"
#include "ramses/client/TextureSamplerExternal.h"
#include "ramses/client/RenderBuffer.h"
#include "ramses/client/RenderTarget.h"
#include "ramses/client/Texture2D.h"
#include "ramses/client/Texture3D.h"
#include "ramses/client/TextureCube.h"
#include "ramses/client/Texture2DBuffer.h"
#include "ramses/client/RenderBuffer.h"
#include "ramses/client/RenderTargetDescription.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/OrthographicCamera.h"
#include "impl/TextureSamplerImpl.h"
#include "impl/RenderBufferImpl.h"
#include "impl/Texture2DImpl.h"
#include "impl/Texture3DImpl.h"
#include "impl/TextureCubeImpl.h"
#include "impl/Texture2DBufferImpl.h"

using namespace testing;

namespace ramses::internal
{
    class TextureSamplerTest : public LocalTestClientWithScene, public ::testing::Test
    {
    };

    TEST_F(TextureSamplerTest, createSamplerForTexture2D)
    {
        const Texture2D& texture2D = createObject<Texture2D>("testTexture2D");
        const ramses::TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, texture2D, 1u, "testSampler2D");

        ASSERT_NE(static_cast<ramses::TextureSampler*>(nullptr), sampler);
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(1u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType::Texture2D, sampler->getTextureType());

        const ramses::internal::TextureSamplerHandle samplerHandle = sampler->impl().getTextureSamplerHandle();
        const auto texHash = texture2D.impl().getLowlevelResourceHash();
        ASSERT_TRUE(m_internalScene.isTextureSamplerAllocated(samplerHandle));
        EXPECT_EQ(ramses::internal::TextureSampler::ContentType::ClientTexture, m_internalScene.getTextureSampler(samplerHandle).contentType);
        EXPECT_EQ(texHash, m_internalScene.getTextureSampler(samplerHandle).textureResource);
    }

    TEST_F(TextureSamplerTest, createSamplerForTexture2DWithAnisotropy)
    {
        const Texture2D& texture2D = createObject<Texture2D>("testTexture2D");
        const ramses::TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, texture2D, 16u, "testSampler2D");

        ASSERT_NE(static_cast<ramses::TextureSampler*>(nullptr), sampler);
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
        const ramses::TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureAddressMode::Mirror, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, texture3D, "testSampler3D");

        ASSERT_NE(static_cast<ramses::TextureSampler*>(nullptr), sampler);
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
        const ramses::TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, textureCube, 1u, "testSamplerCube");

        ASSERT_NE(static_cast<ramses::TextureSampler*>(nullptr), sampler);
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
        const ramses::TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, textureCube, 16u, "testSamplerCube");

        ASSERT_NE(static_cast<ramses::TextureSampler*>(nullptr), sampler);
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(16u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType::TextureCube, sampler->getTextureType());
    }

    TEST_F(TextureSamplerTest, createSamplerForRenderBuffer)
    {
        const ramses::RenderBuffer& renderBuffer = createObject<ramses::RenderBuffer>();
        ramses::TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, renderBuffer, 16u);

        ASSERT_NE(static_cast<ramses::TextureSampler*>(nullptr), sampler);
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(16u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType::RenderBuffer, sampler->getTextureType());

        const ramses::internal::TextureSamplerHandle samplerHandle = sampler->impl().getTextureSamplerHandle();
        const ramses::internal::RenderBufferHandle renderBufferHandle = renderBuffer.impl().getRenderBufferHandle();
        ASSERT_TRUE(m_internalScene.isTextureSamplerAllocated(samplerHandle));
        EXPECT_EQ(ramses::internal::TextureSampler::ContentType::RenderBuffer, m_internalScene.getTextureSampler(samplerHandle).contentType);
        EXPECT_EQ(renderBufferHandle.asMemoryHandle(), m_internalScene.getTextureSampler(samplerHandle).contentHandle);
    }

    TEST_F(TextureSamplerTest, createSamplerMS)
    {
        const ramses::RenderBuffer& renderBuffer = createObject<ramses::RenderBuffer>();
        TextureSamplerMS* sampler = this->m_scene.createTextureSamplerMS(renderBuffer, "renderBuffer");

        ASSERT_NE(static_cast<TextureSamplerMS*>(nullptr), sampler);

        const ramses::internal::TextureSamplerHandle samplerHandle = sampler->impl().getTextureSamplerHandle();
        const ramses::internal::RenderBufferHandle renderBufferHandle = renderBuffer.impl().getRenderBufferHandle();
        ASSERT_TRUE(m_internalScene.isTextureSamplerAllocated(samplerHandle));
        EXPECT_EQ(ramses::internal::TextureSampler::ContentType::RenderBufferMS, m_internalScene.getTextureSampler(samplerHandle).contentType);
        EXPECT_EQ(renderBufferHandle.asMemoryHandle(), m_internalScene.getTextureSampler(samplerHandle).contentHandle);
    }

    TEST_F(TextureSamplerTest, createSamplerForTextureExternal)
    {
        const TextureSamplerExternal* sampler = this->m_scene.createTextureSamplerExternal(ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, "testSampler2DExternal");

        ASSERT_NE(static_cast<TextureSamplerExternal*>(nullptr), sampler);
        const ramses::internal::TextureSamplerHandle samplerHandle = sampler->impl().getTextureSamplerHandle();

        ASSERT_TRUE(m_internalScene.isTextureSamplerAllocated(samplerHandle));
        const auto& samplerInternal = m_internalScene.getTextureSampler(samplerHandle);

        EXPECT_EQ(ramses::internal::TextureSampler::ContentType::ExternalTexture, samplerInternal.contentType);
        EXPECT_EQ(ramses::internal::InvalidMemoryHandle, samplerInternal.contentHandle);
        EXPECT_FALSE(samplerInternal.textureResource.isValid());

        EXPECT_EQ(ETextureAddressMode::Clamp, samplerInternal.states.m_addressModeU);
        EXPECT_EQ(ETextureAddressMode::Clamp, samplerInternal.states.m_addressModeV);
        EXPECT_EQ(ETextureAddressMode::Clamp, samplerInternal.states.m_addressModeR);
        EXPECT_EQ(ETextureSamplingMethod::Linear, samplerInternal.states.m_minSamplingMode);
        EXPECT_EQ(ETextureSamplingMethod::Linear, samplerInternal.states.m_magSamplingMode);
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
        EXPECT_TRUE(m_scene.createTextureConsumer(*sampler1, consumerId));
        ValidationReport report;
        m_scene.validate(report);
        EXPECT_FALSE(report.hasIssue());

        const auto sampler2 = m_scene.createTextureSamplerExternal(ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear);
        EXPECT_TRUE(m_scene.createTextureConsumer(*sampler2, consumerId));
        report.clear();
        m_scene.validate(report);
        EXPECT_TRUE(report.hasError());
        EXPECT_THAT(report.impl().toString(), HasSubstr("Duplicate texture consumer ID '123'"));

        // validates fine again after duplicate removed
        EXPECT_TRUE(m_scene.destroy(*sampler1));
        report.clear();
        m_scene.validate(report);
        EXPECT_FALSE(report.hasIssue());
    }

    TEST_F(TextureSamplerTest, cannotCreateSamplerForRenderBufferWithSamples)
    {
        const ramses::RenderBuffer& renderBuffer = *m_scene.createRenderBuffer(4u, 4u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite, 4u);
        ramses::TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, renderBuffer, 16u);

        EXPECT_EQ(nullptr, sampler);
    }

    TEST_F(TextureSamplerTest, cannotCreateSamplerMSForWriteOnlyRenderBuffer)
    {
        const ramses::RenderBuffer& renderBuffer = *m_scene.createRenderBuffer(4u, 4u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::WriteOnly, 4u);
        TextureSamplerMS* sampler = this->m_scene.createTextureSamplerMS(renderBuffer, "renderBuffer");

        EXPECT_EQ(nullptr, sampler);
    }

    TEST_F(TextureSamplerTest, reportsErrorWhenValidatedWithInvalidTexture2D)
    {
        auto& texture2D = createObject<Texture2D>("testTexture2D");
        const ramses::TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, texture2D, 1u, "testSampler2D");
        ASSERT_TRUE(nullptr != sampler);
        ValidationReport report;
        sampler->validate(report);
        EXPECT_FALSE(report.hasIssue());

        EXPECT_TRUE(m_scene.destroy(texture2D));
        report.clear();
        sampler->validate(report);
        EXPECT_TRUE(report.hasError());
    }

    TEST_F(TextureSamplerTest, reportsErrorWhenValidatedWithInvalidTexture3D)
    {
        auto& texture3D = createObject<Texture3D>("testTexture3D");
        const ramses::TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureAddressMode::Mirror, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, texture3D, "testSampler3D");
        ASSERT_TRUE(nullptr != sampler);
        ValidationReport report;
        sampler->validate(report);
        EXPECT_FALSE(report.hasIssue());

        EXPECT_TRUE(m_scene.destroy(texture3D));
        report.clear();
        sampler->validate(report);
        EXPECT_TRUE(report.hasError());
    }

    TEST_F(TextureSamplerTest, reportsErrorWhenValidatedWithInvalidTextureCube)
    {
        auto& textureCube = createObject<TextureCube>("testTextureCube");
        const ramses::TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, textureCube, 1u, "testSamplerCube");
        ASSERT_TRUE(nullptr != sampler);
        ValidationReport report;
        sampler->validate(report);
        EXPECT_FALSE(report.hasIssue());

        EXPECT_TRUE(m_scene.destroy(textureCube));
        report.clear();
        sampler->validate(report);
        EXPECT_TRUE(report.hasError());
    }

    TEST_F(TextureSamplerTest, reportsErrorWhenValidatedWithInvalidRenderBuffer)
    {
        auto& renderBuffer = createObject<ramses::RenderBuffer>();
        RenderTargetDescription rtDesc;
        rtDesc.addRenderBuffer(renderBuffer);
        ramses::RenderTarget* rt = this->m_scene.createRenderTarget(rtDesc);
        auto& renderPass = createObject<ramses::RenderPass>();
        OrthographicCamera* orthoCam = this->m_scene.createOrthographicCamera("camera");
        orthoCam->setFrustum(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f);
        orthoCam->setViewport(0, 0, 100, 200);
        renderPass.setCamera(*orthoCam);
        renderPass.setRenderTarget(rt);

        ramses::TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, renderBuffer);
        ASSERT_TRUE(nullptr != sampler);
        ValidationReport report;
        sampler->validate(report);
        EXPECT_FALSE(report.hasIssue());

        EXPECT_TRUE(m_scene.destroy(renderBuffer));
        report.clear();
        sampler->validate(report);
        EXPECT_TRUE(report.hasError());
    }

    TEST_F(TextureSamplerTest, reportsErrorWhenValidatedWithInvalidRenderBufferMS)
    {
        auto& renderBuffer = createObject<ramses::RenderBuffer>();
        RenderTargetDescription rtDesc;
        rtDesc.addRenderBuffer(renderBuffer);
        ramses::RenderTarget* rt = this->m_scene.createRenderTarget(rtDesc);
        auto& renderPass = createObject<ramses::RenderPass>();
        OrthographicCamera* orthoCam = this->m_scene.createOrthographicCamera("camera");
        orthoCam->setFrustum(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f);
        orthoCam->setViewport(0, 0, 100, 200);
        renderPass.setCamera(*orthoCam);
        renderPass.setRenderTarget(rt);

        TextureSamplerMS* sampler = this->m_scene.createTextureSamplerMS(renderBuffer, "textureSamplerMS");
        ASSERT_TRUE(nullptr != sampler);
        ValidationReport report;
        sampler->validate(report);
        EXPECT_FALSE(report.hasIssue());

        EXPECT_TRUE(m_scene.destroy(renderBuffer));
        report.clear();
        sampler->validate(report);
        EXPECT_TRUE(report.hasError());
    }

    TEST_F(TextureSamplerTest, doesNotReportErrorWhenValidatedWithExternalTexture)
    {
        TextureSamplerExternal* sampler = this->m_scene.createTextureSamplerExternal(ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear);
        ASSERT_TRUE(nullptr != sampler);
        ValidationReport report;
        sampler->validate(report);
        EXPECT_FALSE(report.hasIssue());
    }

    TEST_F(TextureSamplerTest, setTextureDataToTexture2D)
    {
        const Texture3D& texture3D = createObject<Texture3D>();
        ramses::TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, texture3D);
        ASSERT_NE(nullptr, sampler);

        const Texture2D& texture2D = createObject<Texture2D>();
        EXPECT_TRUE(sampler->setTextureData(texture2D));

        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapRMode());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(1u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType::Texture2D, sampler->getTextureType());

        const ramses::internal::TextureSamplerHandle samplerHandle = sampler->impl().getTextureSamplerHandle();
        ASSERT_TRUE(m_internalScene.isTextureSamplerAllocated(samplerHandle));
        EXPECT_EQ(1u, m_internalScene.getTextureSamplerCount());
        EXPECT_EQ(ramses::internal::TextureSampler::ContentType::ClientTexture, m_internalScene.getTextureSampler(samplerHandle).contentType);
        EXPECT_EQ(texture2D.impl().getLowlevelResourceHash(), m_internalScene.getTextureSampler(samplerHandle).textureResource);
    }

    TEST_F(TextureSamplerTest, setTextureDataFromTexture3DToAnotherTexture3D)
    {
        const Texture3D& texture3Doriginal = createObject<Texture3D>();
        ramses::TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, texture3Doriginal);
        ASSERT_NE(nullptr, sampler);

        const Texture3D& texture3D = createObject<Texture3D>();
        EXPECT_TRUE(sampler->setTextureData(texture3D));

        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapRMode());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(1u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType::Texture3D, sampler->getTextureType());

        const ramses::internal::TextureSamplerHandle samplerHandle = sampler->impl().getTextureSamplerHandle();
        ASSERT_TRUE(m_internalScene.isTextureSamplerAllocated(samplerHandle));
        EXPECT_EQ(1u, m_internalScene.getTextureSamplerCount());
        EXPECT_EQ(ramses::internal::TextureSampler::ContentType::ClientTexture, m_internalScene.getTextureSampler(samplerHandle).contentType);
        EXPECT_EQ(texture3D.impl().getLowlevelResourceHash(), m_internalScene.getTextureSampler(samplerHandle).textureResource);
    }

    TEST_F(TextureSamplerTest, failsToSetTextureDataFromTexture2DToTexture3D)
    {
        const Texture2D& texture2D = createObject<Texture2D>();
        ramses::TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, texture2D, 1u);
        ASSERT_NE(nullptr, sampler);

        const Texture3D& texture3D = createObject<Texture3D>();
        EXPECT_FALSE(sampler->setTextureData(texture3D));
    }

    TEST_F(TextureSamplerTest, setTextureDataToTextureCube)
    {
        const Texture3D& texture3D = createObject<Texture3D>();
        ramses::TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, texture3D);
        ASSERT_NE(nullptr, sampler);

        const TextureCube& textureCube = createObject<TextureCube>();
        EXPECT_TRUE(sampler->setTextureData(textureCube));

        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapRMode());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(1u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType::TextureCube, sampler->getTextureType());

        const ramses::internal::TextureSamplerHandle samplerHandle = sampler->impl().getTextureSamplerHandle();
        ASSERT_TRUE(m_internalScene.isTextureSamplerAllocated(samplerHandle));
        EXPECT_EQ(1u, m_internalScene.getTextureSamplerCount());
        EXPECT_EQ(ramses::internal::TextureSampler::ContentType::ClientTexture, m_internalScene.getTextureSampler(samplerHandle).contentType);
        EXPECT_EQ(textureCube.impl().getLowlevelResourceHash(), m_internalScene.getTextureSampler(samplerHandle).textureResource);
    }

    TEST_F(TextureSamplerTest, setTextureDataToTexture2DBuffer)
    {
        const Texture2D& texture2D = createObject<Texture2D>();
        ramses::TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, texture2D, 1u);
        ASSERT_NE(nullptr, sampler);

        const Texture2DBuffer& textureBuffer = createObject<Texture2DBuffer>();
        EXPECT_TRUE(sampler->setTextureData(textureBuffer));

        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(1u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType::Texture2DBuffer, sampler->getTextureType());

        const ramses::internal::TextureSamplerHandle samplerHandle = sampler->impl().getTextureSamplerHandle();
        ASSERT_TRUE(m_internalScene.isTextureSamplerAllocated(samplerHandle));
        EXPECT_EQ(1u, m_internalScene.getTextureSamplerCount());
        EXPECT_EQ(ramses::internal::TextureSampler::ContentType::TextureBuffer, m_internalScene.getTextureSampler(samplerHandle).contentType);
        EXPECT_EQ(textureBuffer.impl().getTextureBufferHandle().asMemoryHandle(), m_internalScene.getTextureSampler(samplerHandle).contentHandle);
    }

    TEST_F(TextureSamplerTest, setTextureDataToRenderBuffer)
    {
        const Texture2D& texture2D = createObject<Texture2D>();
        ramses::TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, texture2D, 1u);
        ASSERT_NE(nullptr, sampler);

        const ramses::RenderBuffer& buffer = createObject<ramses::RenderBuffer>();
        EXPECT_TRUE(sampler->setTextureData(buffer));

        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(1u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType::RenderBuffer, sampler->getTextureType());

        const ramses::internal::TextureSamplerHandle samplerHandle = sampler->impl().getTextureSamplerHandle();
        ASSERT_TRUE(m_internalScene.isTextureSamplerAllocated(samplerHandle));
        EXPECT_EQ(1u, m_internalScene.getTextureSamplerCount());
        EXPECT_EQ(ramses::internal::TextureSampler::ContentType::RenderBuffer, m_internalScene.getTextureSampler(samplerHandle).contentType);
        EXPECT_EQ(buffer.impl().getRenderBufferHandle().asMemoryHandle(), m_internalScene.getTextureSampler(samplerHandle).contentHandle);
    }

    TEST_F(TextureSamplerTest, failsToSetTextureDataToSamplerMarkedAsConsumer)
    {
        const auto& texture2D = createObject<Texture2D>();
        ramses::TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, texture2D);
        ASSERT_NE(nullptr, sampler);

        ASSERT_TRUE(this->m_scene.createTextureConsumer(*sampler, dataConsumerId_t{666u}));

        const auto& texture2Dother = createObject<Texture2D>();
        EXPECT_FALSE(sampler->setTextureData(texture2Dother));
        const auto& textureCube = createObject<TextureCube>();
        EXPECT_FALSE(sampler->setTextureData(textureCube));
        const auto& textureBuffer = createObject<Texture2DBuffer>();
        EXPECT_FALSE(sampler->setTextureData(textureBuffer));
        const auto& renderBuffer = createObject<ramses::RenderBuffer>();
        EXPECT_FALSE(sampler->setTextureData(renderBuffer));
    }

    TEST_F(TextureSamplerTest, failsToSetTextureDataFromOtherSceneThanSampler)
    {
        const auto& texture2D = createObject<Texture2D>();
        ramses::TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, texture2D);
        ASSERT_NE(nullptr, sampler);

        RamsesClient& otherClient(*this->framework.createClient("other"));
        CreationHelper creationHelper(otherClient.createScene(sceneId_t(666u)), &otherClient);

        const auto& texture2Dother = *creationHelper.createObjectOfType<Texture2D>({});
        EXPECT_FALSE(sampler->setTextureData(texture2Dother));
        const auto& textureCube = *creationHelper.createObjectOfType<TextureCube>({});
        EXPECT_FALSE(sampler->setTextureData(textureCube));
        const auto& textureBuffer = *creationHelper.createObjectOfType<Texture2DBuffer>({});
        EXPECT_FALSE(sampler->setTextureData(textureBuffer));
        const auto& renderBuffer = *creationHelper.createObjectOfType<ramses::RenderBuffer>({});
        EXPECT_FALSE(sampler->setTextureData(renderBuffer));
    }
}
