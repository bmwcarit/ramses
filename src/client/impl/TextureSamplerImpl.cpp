//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/Texture2D.h"
#include "ramses/client/Texture3D.h"
#include "ramses/client/TextureCube.h"
#include "ramses/client/RenderBuffer.h"
#include "ramses/client/Texture2DBuffer.h"
#include "ramses/client/RenderTarget.h"
#include "impl/Texture2DImpl.h"
#include "impl/Texture3DImpl.h"
#include "impl/TextureCubeImpl.h"
#include "impl/TextureSamplerImpl.h"
#include "impl/RamsesClientImpl.h"
#include "impl/RenderBufferImpl.h"
#include "impl/Texture2DBufferImpl.h"
#include "impl/RenderTargetImpl.h"
#include "impl/SceneImpl.h"
#include "impl/TextureUtils.h"
#include "impl/SceneObjectRegistryIterator.h"
#include "impl/ErrorReporting.h"
#include "impl/SerializationContext.h"
#include "internal/SceneGraph/Scene/ClientScene.h"
#include "internal/DataSlotUtils.h"

namespace ramses::internal
{
    TextureSamplerImpl::TextureSamplerImpl(SceneImpl& scene, ERamsesObjectType type, std::string_view name)
        : SceneObjectImpl(scene, type, name)
        , m_textureType(ERamsesObjectType::Invalid)
    {
    }

    TextureSamplerImpl::~TextureSamplerImpl() = default;

    void TextureSamplerImpl::initializeFrameworkData(
        const ramses::internal::TextureSamplerStates& samplerStates,
        ERamsesObjectType textureType,
        ramses::internal::TextureSampler::ContentType contentType,
        ramses::internal::ResourceContentHash textureHash,
        ramses::internal::MemoryHandle contentHandle)
    {
        m_textureType = textureType;
        m_textureSamplerHandle = getIScene().allocateTextureSampler({ samplerStates, contentType, textureHash, contentHandle }, {});
    }

    void TextureSamplerImpl::deinitializeFrameworkData()
    {
        assert(m_textureSamplerHandle.isValid());
        getIScene().releaseTextureSampler(m_textureSamplerHandle);
        m_textureSamplerHandle = ramses::internal::TextureSamplerHandle::Invalid();
    }

    bool TextureSamplerImpl::setTextureData(const Texture2D& texture)
    {
        if (!isFromTheSameSceneAs(texture.impl()))
        {
            getErrorReporting().set("TextureSampler::setTextureData failed, client texture is not from the same client as this sampler.");
            return false;
        }

        return setTextureDataInternal(ERamsesObjectType::Texture2D, ramses::internal::TextureSampler::ContentType::ClientTexture, texture.impl().getLowlevelResourceHash(), ramses::internal::InvalidMemoryHandle);
    }

    bool TextureSamplerImpl::setTextureData(const Texture3D& texture)
    {
        if (m_textureType != ERamsesObjectType::Texture3D)
        {
            getErrorReporting().set("TextureSampler::setTextureData failed, changing data from non 3D texture to 3D texture is not supported. Create a new TextureSampler instead.");
            return false;
        }

        if (!isFromTheSameSceneAs(texture.impl()))
        {
            getErrorReporting().set("TextureSampler::setTextureData failed, client texture is not from the same client as this sampler.");
            return false;
        }

        return setTextureDataInternal(ERamsesObjectType::Texture3D, ramses::internal::TextureSampler::ContentType::ClientTexture, texture.impl().getLowlevelResourceHash(), ramses::internal::InvalidMemoryHandle);
    }

    bool TextureSamplerImpl::setTextureData(const TextureCube& texture)
    {
        if (!isFromTheSameSceneAs(texture.impl()))
        {
            getErrorReporting().set("TextureSampler::setTextureData failed, client texture is not from the same client as this sampler.");
            return false;
        }

        return setTextureDataInternal(ERamsesObjectType::TextureCube, ramses::internal::TextureSampler::ContentType::ClientTexture, texture.impl().getLowlevelResourceHash(), ramses::internal::InvalidMemoryHandle);
    }

    bool TextureSamplerImpl::setTextureData(const Texture2DBuffer& texture)
    {
        if (!getSceneImpl().containsSceneObject(texture.impl()))
        {
            getErrorReporting().set("TextureSampler::setTextureData failed, texture2D buffer is not from the same scene as this sampler.");
            return false;
        }

        return setTextureDataInternal(ERamsesObjectType::Texture2DBuffer, ramses::internal::TextureSampler::ContentType::TextureBuffer, {}, texture.impl().getTextureBufferHandle().asMemoryHandle());
    }

    bool TextureSamplerImpl::setTextureData(const ramses::RenderBuffer& texture)
    {
        if (!getSceneImpl().containsSceneObject(texture.impl()))
        {
            getErrorReporting().set("TextureSampler::setTextureData failed, render buffer is not from the same scene as this sampler.");
            return false;
        }

        if (ERenderBufferAccessMode::WriteOnly == texture.impl().getAccessMode())
        {
            getErrorReporting().set("TextureSampler::setTextureData failed, render buffer has access mode write only.");
            return false;
        }

        return setTextureDataInternal(ERamsesObjectType::RenderBuffer, ramses::internal::TextureSampler::ContentType::RenderBuffer, {}, texture.impl().getRenderBufferHandle().asMemoryHandle());
    }

    bool TextureSamplerImpl::setTextureDataInternal(ERamsesObjectType textureType,
        ramses::internal::TextureSampler::ContentType contentType,
        ramses::internal::ResourceContentHash textureHash,
        ramses::internal::MemoryHandle contentHandle)
    {
        if (ramses::internal::DataSlotUtils::HasDataSlotIdForTextureSampler(getIScene(), m_textureSamplerHandle))
        {
            // Additional logic would have to be added on renderer side to support change of content to update consumer fallback sampler state.
            // Also more checks would be required here to make sure only 2D textures are involved.
            getErrorReporting().set("TextureSampler::setTextureData failed, changing texture sampler data for a sampler marked as texture link consumer is not supported. Create a new TextureSampler instead.");
            return false;
        }

        // With current internal logic re-creating the texture sampler instance adds minimal overhead
        // and thus extra support for change of data source on internal scene level is not worth the additional complexity.
        // This should be revisited whenever internal logic changes.

        const ramses::internal::TextureSamplerStates samplerStates = getIScene().getTextureSampler(m_textureSamplerHandle).states;
        getIScene().releaseTextureSampler(m_textureSamplerHandle);

        // re-allocate with same handle
        [[maybe_unused]] const auto handle = getIScene().allocateTextureSampler({ samplerStates, contentType, textureHash, contentHandle }, m_textureSamplerHandle);
        assert(m_textureSamplerHandle == handle);

        m_textureType = textureType;

        return true;
    }

    ETextureAddressMode TextureSamplerImpl::getWrapUMode() const
    {
        return getIScene().getTextureSampler(m_textureSamplerHandle).states.m_addressModeU;
    }

    ETextureAddressMode TextureSamplerImpl::getWrapVMode() const
    {
        return getIScene().getTextureSampler(m_textureSamplerHandle).states.m_addressModeV;
    }

    ETextureAddressMode TextureSamplerImpl::getWrapRMode() const
    {
        return getIScene().getTextureSampler(m_textureSamplerHandle).states.m_addressModeR;
    }

    ETextureSamplingMethod TextureSamplerImpl::getMinSamplingMethod() const
    {
        return getIScene().getTextureSampler(m_textureSamplerHandle).states.m_minSamplingMode;
    }

    ETextureSamplingMethod TextureSamplerImpl::getMagSamplingMethod() const
    {
        return getIScene().getTextureSampler(m_textureSamplerHandle).states.m_magSamplingMode;
    }

    uint32_t TextureSamplerImpl::getAnisotropyLevel() const
    {
        return getIScene().getTextureSampler(m_textureSamplerHandle).states.m_anisotropyLevel;
    }

    ramses::internal::TextureSamplerHandle TextureSamplerImpl::getTextureSamplerHandle() const
    {
        return m_textureSamplerHandle;
    }

    ERamsesObjectType TextureSamplerImpl::getTextureType() const
    {
        return m_textureType;
    }

    ramses::internal::EDataType TextureSamplerImpl::getTextureDataType() const
    {
        if (getIScene().getTextureSampler(m_textureSamplerHandle).contentType == ramses::internal::TextureSampler::ContentType::RenderBufferMS)
            return ramses::internal::EDataType::TextureSampler2DMS;

        switch (m_textureType)
        {
            case ERamsesObjectType::Texture2D:
            case ERamsesObjectType::RenderBuffer:
            case ERamsesObjectType::Texture2DBuffer: return ramses::internal::EDataType::TextureSampler2D;
            case ERamsesObjectType::Texture3D: return ramses::internal::EDataType::TextureSampler3D;
            case ERamsesObjectType::TextureCube: return ramses::internal::EDataType::TextureSamplerCube;
            case ERamsesObjectType::TextureSamplerExternal: return ramses::internal::EDataType::TextureSamplerExternal;
            default: break;
        }
        return ramses::internal::EDataType::Invalid;
    }

    bool TextureSamplerImpl::serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        if (!SceneObjectImpl::serialize(outStream, serializationContext))
            return false;

        outStream << static_cast<uint32_t>(m_textureType);
        outStream << m_textureSamplerHandle;

        return true;
    }

    bool TextureSamplerImpl::deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        if (!SceneObjectImpl::deserialize(inStream, serializationContext))
            return false;

        uint32_t value = 0;

        inStream >> value;
        m_textureType = static_cast<ERamsesObjectType>(value);

        serializationContext.deserializeAndMap(inStream, m_textureSamplerHandle);

        return true;
    }

    void TextureSamplerImpl::onValidate(ValidationReportImpl& report) const
    {
        SceneObjectImpl::onValidate(report);

        const Resource* resource = nullptr;
        const ramses::internal::TextureSampler& sampler = getIScene().getTextureSampler(m_textureSamplerHandle);
        switch (sampler.contentType)
        {
        case ramses::internal::TextureSampler::ContentType::ClientTexture:
            resource = getSceneImpl().scanForResourceWithHash(sampler.textureResource);
            if (resource == nullptr)
                return report.add(EIssueType::Error, "Client texture set in TextureSampler does not exist", &getRamsesObject());
            validateResource(report, resource);
            break;
        case ramses::internal::TextureSampler::ContentType::RenderBuffer:
        case ramses::internal::TextureSampler::ContentType::RenderBufferMS:
            validateRenderBuffer(report, ramses::internal::RenderBufferHandle(sampler.contentHandle));
            break;
        case ramses::internal::TextureSampler::ContentType::TextureBuffer:
            validateTextureBuffer(report, ramses::internal::TextureBufferHandle(sampler.contentHandle));
            break;
        case ramses::internal::TextureSampler::ContentType::ExternalTexture:
            break;
        case ramses::internal::TextureSampler::ContentType::OffscreenBuffer:
        case ramses::internal::TextureSampler::ContentType::StreamBuffer:
        case ramses::internal::TextureSampler::ContentType::None:
            return report.add(EIssueType::Error, "There is no valid content source set in TextureSampler", &getRamsesObject());
        }
    }

    void TextureSamplerImpl::validateRenderBuffer(ValidationReportImpl& report, ramses::internal::RenderBufferHandle renderBufferHandle) const
    {
        bool foundRenderBuffer = false;

        const bool isRenderBufferValid = getIScene().isRenderBufferAllocated(renderBufferHandle);
        if (isRenderBufferValid)
        {
            SceneObjectRegistryIterator iter(getSceneImpl().getObjectRegistry(), ERamsesObjectType::RenderBuffer);
            while (const auto* renderBuffer = iter.getNext<ramses::RenderBuffer>())
            {
                if (renderBufferHandle == renderBuffer->impl().getRenderBufferHandle())
                {
                    foundRenderBuffer = true;
                    break;
                }
            }
        }

        if (!foundRenderBuffer)
            report.add(EIssueType::Error, "Texture Sampler is using a RenderBuffer which does not exist", &getRamsesObject());
    }

    void TextureSamplerImpl::validateTextureBuffer(ValidationReportImpl& report, ramses::internal::TextureBufferHandle textureBufferHandle) const
    {
        bool foundTextureBuffer = false;

        const bool isTextureBufferValid = getIScene().isTextureBufferAllocated(textureBufferHandle);
        if (isTextureBufferValid)
        {
            SceneObjectRegistryIterator iter(getSceneImpl().getObjectRegistry(), ERamsesObjectType::Texture2DBuffer);
            while (const auto* textureBuffer = iter.getNext<Texture2DBuffer>())
            {
                if (textureBufferHandle == textureBuffer->impl().getTextureBufferHandle())
                {
                    foundTextureBuffer = true;
                    break;
                }
            }
        }

        if (!foundTextureBuffer)
            report.add(EIssueType::Error, "Texture Sampler is using a TextureBuffer which does not exist", &getRamsesObject());
    }

    void TextureSamplerImpl::validateResource(ValidationReportImpl& report, const Resource* resource) const
    {
        assert(resource);
        const ERamsesObjectType resourceType = resource->getType();
        switch (resourceType)
        {
        case ERamsesObjectType::Texture2D:
        case ERamsesObjectType::Texture3D:
        case ERamsesObjectType::TextureCube:
            report.addDependentObject(*this, resource->impl());
            break;
        default:
            assert(false);
            break;
        }
    }
}
