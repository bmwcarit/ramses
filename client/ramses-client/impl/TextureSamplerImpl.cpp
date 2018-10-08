//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Texture3D.h"
#include "ramses-client-api/TextureCube.h"
#include "ramses-client-api/RenderBuffer.h"
#include "ramses-client-api/Texture2DBuffer.h"
#include "ramses-client-api/RenderTarget.h"
#include "ramses-client-api/StreamTexture.h"
#include "Texture2DImpl.h"
#include "Texture3DImpl.h"
#include "TextureCubeImpl.h"
#include "TextureSamplerImpl.h"
#include "RamsesClientImpl.h"
#include "RenderBufferImpl.h"
#include "Texture2DBufferImpl.h"
#include "RenderTargetImpl.h"
#include "StreamTextureImpl.h"
#include "SceneImpl.h"
#include "Scene/ClientScene.h"
#include "TextureUtils.h"
#include "RamsesObjectRegistryIterator.h"

namespace ramses
{
    TextureSamplerImpl::TextureSamplerImpl(SceneImpl& scene, const char* name)
        : SceneObjectImpl(scene, ERamsesObjectType_TextureSampler, name)
        , m_textureType(ERamsesObjectType_Invalid)
    {
    }

    TextureSamplerImpl::~TextureSamplerImpl()
    {
    }

    void TextureSamplerImpl::initializeFrameworkData(
        const ramses_internal::TextureSamplerStates& samplerStates,
        ERamsesObjectType textureType,
        ramses_internal::ResourceContentHash textureHash,
        ramses_internal::MemoryHandle contentHandle)
    {
        m_textureType = textureType;

        ramses_internal::TextureSampler::ContentType contentType = ramses_internal::TextureSampler::ContentType::None;
        switch (textureType)
        {
        case ramses::ERamsesObjectType_Texture2D:
        case ramses::ERamsesObjectType_Texture3D:
        case ramses::ERamsesObjectType_TextureCube:
            contentType = ramses_internal::TextureSampler::ContentType::ClientTexture;
            break;
        case ramses::ERamsesObjectType_RenderBuffer:
            contentType = ramses_internal::TextureSampler::ContentType::RenderBuffer;
            break;
        case ramses::ERamsesObjectType_Texture2DBuffer:
            contentType = ramses_internal::TextureSampler::ContentType::TextureBuffer;
            break;
        case ramses::ERamsesObjectType_StreamTexture:
            contentType = ramses_internal::TextureSampler::ContentType::StreamTexture;
            break;
        default:
            assert(false);
            break;
        }
        m_textureSamplerHandle = getIScene().allocateTextureSampler({ samplerStates, contentType, textureHash, contentHandle });
    }

    void TextureSamplerImpl::deinitializeFrameworkData()
    {
        assert(m_textureSamplerHandle.isValid());
        getIScene().releaseTextureSampler(m_textureSamplerHandle);
        m_textureSamplerHandle = ramses_internal::TextureSamplerHandle::Invalid();
    }

    ETextureAddressMode TextureSamplerImpl::getWrapUMode() const
    {
        return TextureUtils::GetTextureAddressModeFromInternal(getIScene().getTextureSampler(m_textureSamplerHandle).states.m_addressModeU);
    }

    ETextureAddressMode TextureSamplerImpl::getWrapVMode() const
    {
        return TextureUtils::GetTextureAddressModeFromInternal(getIScene().getTextureSampler(m_textureSamplerHandle).states.m_addressModeV);
    }

    ETextureAddressMode TextureSamplerImpl::getWrapRMode() const
    {
        return TextureUtils::GetTextureAddressModeFromInternal(getIScene().getTextureSampler(m_textureSamplerHandle).states.m_addressModeR);
    }

    ETextureSamplingMethod TextureSamplerImpl::getSamplingMethod() const
    {
        return TextureUtils::GetTextureSamplingFromInternal(getIScene().getTextureSampler(m_textureSamplerHandle).states.m_samplingMode);
    }

    uint32_t TextureSamplerImpl::getAnisotropyLevel() const
    {
        return getIScene().getTextureSampler(m_textureSamplerHandle).states.m_anisotropyLevel;
    }

    ramses_internal::TextureSamplerHandle TextureSamplerImpl::getTextureSamplerHandle() const
    {
        return m_textureSamplerHandle;
    }

    ERamsesObjectType TextureSamplerImpl::getTextureType() const
    {
        return m_textureType;
    }

    status_t TextureSamplerImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(SceneObjectImpl::serialize(outStream, serializationContext));

        outStream << static_cast<ramses_internal::UInt32>(m_textureType);
        outStream << m_textureSamplerHandle;

        return StatusOK;
    }

    status_t TextureSamplerImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(SceneObjectImpl::deserialize(inStream, serializationContext));

        ramses_internal::UInt32 value;

        inStream >> value;
        m_textureType = static_cast<ERamsesObjectType>(value);

        inStream >> m_textureSamplerHandle;

        return StatusOK;
    }

    status_t TextureSamplerImpl::validate(uint32_t indent) const
    {
        status_t status = SceneObjectImpl::validate(indent);
        indent += IndentationStep;

        status_t contentStatus = StatusOK;
        const Resource* resource = nullptr;
        const ramses_internal::TextureSampler& sampler = getIScene().getTextureSampler(m_textureSamplerHandle);
        switch (sampler.contentType)
        {
        case ramses_internal::TextureSampler::ContentType::ClientTexture:
            resource = getClientImpl().scanForResourceWithHash(sampler.textureResource);
            if (resource == nullptr)
            {
                addValidationMessage(EValidationSeverity_Error, indent, "Client texture set in TextureSampler does not exist");
                return getValidationErrorStatus();
            }
            contentStatus = validateResource(resource, indent);
            break;
        case ramses_internal::TextureSampler::ContentType::RenderBuffer:
            contentStatus = validateRenderBuffer(ramses_internal::RenderBufferHandle(sampler.contentHandle), indent);
            break;
        case ramses_internal::TextureSampler::ContentType::TextureBuffer:
            contentStatus = validateTextureBuffer(ramses_internal::TextureBufferHandle(sampler.contentHandle), indent);
            break;
        case ramses_internal::TextureSampler::ContentType::StreamTexture:
            contentStatus = validateStreamTexture(ramses_internal::StreamTextureHandle(sampler.contentHandle), indent);
            break;
        default:
            addValidationMessage(EValidationSeverity_Error, indent, "There is no valid content source set in TextureSampler");
            return getValidationErrorStatus();
        }

        return (contentStatus != StatusOK ? contentStatus : status);
    }

    ramses::status_t TextureSamplerImpl::validateRenderBuffer(ramses_internal::RenderBufferHandle renderBufferHandle, uint32_t indent) const
    {
        status_t status = StatusOK;
        bool foundRenderBuffer = false;

        const bool isRenderBufferValid = getIScene().isRenderBufferAllocated(renderBufferHandle);
        if (isRenderBufferValid)
        {
            RamsesObjectRegistryIterator iter(getSceneImpl().getObjectRegistry(), ERamsesObjectType_RenderBuffer);
            while (const RenderBuffer* renderBuffer = iter.getNext<RenderBuffer>())
            {
                if (renderBufferHandle == renderBuffer->impl.getRenderBufferHandle())
                {
                    foundRenderBuffer = true;
                    const status_t renderBufferStatus = addValidationOfDependentObject(indent, renderBuffer->impl);
                    if (StatusOK != renderBufferStatus)
                    {
                        status = renderBufferStatus;
                    }
                    break;
                }
            }
        }

        if (!foundRenderBuffer)
        {
            addValidationMessage(EValidationSeverity_Error, indent, "Texture Sampler is using a RenderBuffer which does not exist");
            return getValidationErrorStatus();
        }

        return status;
    }

    ramses::status_t TextureSamplerImpl::validateTextureBuffer(ramses_internal::TextureBufferHandle textureBufferHandle,
                                                               uint32_t                             indent) const
    {
        status_t status             = StatusOK;
        bool     foundTextureBuffer = false;

        const bool isTextureBufferValid = getIScene().isTextureBufferAllocated(textureBufferHandle);
        if (isTextureBufferValid)
        {
            RamsesObjectRegistryIterator iter(getSceneImpl().getObjectRegistry(), ERamsesObjectType_Texture2DBuffer);
            while (const Texture2DBuffer* textureBuffer = iter.getNext<Texture2DBuffer>())
            {
                if (textureBufferHandle == textureBuffer->impl.getTextureBufferHandle())
                {
                    foundTextureBuffer                 = true;
                    const status_t textureBufferStatus = addValidationOfDependentObject(indent, textureBuffer->impl);
                    if (StatusOK != textureBufferStatus)
                    {
                        status = textureBufferStatus;
                    }
                    break;
                }
            }
        }

        if (!foundTextureBuffer)
        {
            addValidationMessage(
                EValidationSeverity_Error, indent, "Texture Sampler is using a TextureBuffer which does not exist");
            return getValidationErrorStatus();
        }

        return status;
    }

    ramses::status_t TextureSamplerImpl::validateStreamTexture(ramses_internal::StreamTextureHandle streamTextureHandle, uint32_t indent) const
    {
        status_t status = StatusOK;
        bool foundStreamTexture = false;

        const bool isStreamTextureValid = getIScene().isStreamTextureAllocated(streamTextureHandle);
        if (isStreamTextureValid)
        {
            RamsesObjectRegistryIterator iter(getSceneImpl().getObjectRegistry(), ERamsesObjectType_StreamTexture);
            while (const StreamTexture* streamTexture = iter.getNext<StreamTexture>())
            {
                if (streamTextureHandle == streamTexture->impl.getHandle())
                {
                    foundStreamTexture = true;
                    status = addValidationOfDependentObject(indent, streamTexture->impl);
                    break;
                }
            }
        }

        if (!foundStreamTexture)
        {
            addValidationMessage(EValidationSeverity_Error, indent, "Texture Sampler is using a StreamTexture which does not exist");
            return getValidationErrorStatus();
        }

        return status;
    }

    ramses::status_t TextureSamplerImpl::validateResource(const Resource* resource, uint32_t indent) const
    {
        if (!resource)
        {
            return StatusOK;
        }

        status_t textureStatus = StatusOK;
        const ERamsesObjectType resourceType = resource->getType();
        switch (resourceType)
        {
        case ERamsesObjectType_Texture2D:
        {
            const Texture2D& texture = RamsesObjectTypeUtils::ConvertTo<Texture2D>(*resource);
            textureStatus = addValidationOfDependentObject(indent, texture.impl);
            break;
        }
        case ERamsesObjectType_Texture3D:
        {
            const Texture3D& texture = RamsesObjectTypeUtils::ConvertTo<Texture3D>(*resource);
            textureStatus = addValidationOfDependentObject(indent, texture.impl);
            break;
        }
        case ERamsesObjectType_TextureCube:
        {
            const TextureCube& texture = RamsesObjectTypeUtils::ConvertTo<TextureCube>(*resource);
            textureStatus = addValidationOfDependentObject(indent, texture.impl);
            break;
        }
        default:
            assert(false);
            break;
        }

        return textureStatus;
    }

}
