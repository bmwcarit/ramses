//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTURESAMPLERIMPL_H
#define RAMSES_TEXTURESAMPLERIMPL_H

#include "ramses-client-api/TextureEnums.h"

// internal
#include "Resource/TextureMetaInfo.h"
#include "SceneObjectImpl.h"

// ramses framework
#include "SceneAPI/Handles.h"
#include "SceneAPI/ResourceContentHash.h"
#include "SceneAPI/TextureSamplerStates.h"
#include "SceneAPI/TextureSampler.h"

namespace ramses
{
    class Texture2D;
    class Texture3D;
    class TextureCube;
    class Texture2DBuffer;
    class RenderBuffer;
    class StreamTexture;

    class TextureSamplerImpl final : public SceneObjectImpl
    {
    public:
        TextureSamplerImpl(SceneImpl& scene, const char* name);
        virtual ~TextureSamplerImpl();

        void initializeFrameworkData(
            const ramses_internal::TextureSamplerStates& samplerStates,
            ERamsesObjectType textureType,
            ramses_internal::TextureSampler::ContentType contentType,
            ramses_internal::ResourceContentHash textureHash,
            ramses_internal::MemoryHandle contentHandle);

        virtual void     deinitializeFrameworkData() override;
        virtual status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        virtual status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;
        virtual status_t validate(uint32_t indent) const override;

        ETextureAddressMode getWrapUMode() const;
        ETextureAddressMode getWrapVMode() const;
        ETextureAddressMode getWrapRMode() const;
        ETextureSamplingMethod getMinSamplingMethod() const;
        ETextureSamplingMethod getMagSamplingMethod() const;
        uint32_t getAnisotropyLevel() const;

        status_t setTextureData(const Texture2D& texture);
        status_t setTextureData(const Texture3D& texture);
        status_t setTextureData(const TextureCube& texture);
        status_t setTextureData(const Texture2DBuffer& texture);
        status_t setTextureData(const RenderBuffer& texture);
        status_t setTextureData(const StreamTexture& texture);

        ramses_internal::TextureSamplerHandle getTextureSamplerHandle() const;
        ERamsesObjectType getTextureType() const;

    private:
        status_t setTextureDataInternal(ERamsesObjectType textureType,
            ramses_internal::TextureSampler::ContentType contentType,
            ramses_internal::ResourceContentHash textureHash,
            ramses_internal::MemoryHandle contentHandle);

        status_t validateRenderBuffer(ramses_internal::RenderBufferHandle renderBufferHandle, uint32_t indent) const;
        status_t validateStreamTexture(ramses_internal::StreamTextureHandle streamTextureHandle, uint32_t indent) const;
        status_t validateTextureBuffer(ramses_internal::TextureBufferHandle textureBufferHandle, uint32_t indent) const;
        status_t validateResource(const Resource* resource, uint32_t indent) const;

        ERamsesObjectType m_textureType;

        ramses_internal::TextureSamplerHandle m_textureSamplerHandle;
    };
}

#endif
