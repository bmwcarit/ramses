//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/TextureEnums.h"

// internal
#include "SceneObjectImpl.h"

// ramses framework
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/SceneGraph/SceneAPI/ResourceContentHash.h"
#include "internal/SceneGraph/SceneAPI/TextureSampler.h"
#include "internal/SceneGraph/SceneAPI/EDataType.h"

#include <string_view>

namespace ramses
{
    class Texture2D;
    class Texture3D;
    class TextureCube;
    class Texture2DBuffer;
    class RenderBuffer;
    class Resource;
}

namespace ramses::internal
{
    class TextureSamplerImpl final : public SceneObjectImpl
    {
    public:
        TextureSamplerImpl(SceneImpl& scene, ERamsesObjectType type, std::string_view name);
        ~TextureSamplerImpl() override;

        void initializeFrameworkData(
            const ramses::internal::TextureSamplerStates& samplerStates,
            ERamsesObjectType textureType,
            ramses::internal::TextureSampler::ContentType contentType,
            ramses::internal::ResourceContentHash textureHash,
            ramses::internal::MemoryHandle contentHandle);

        void     deinitializeFrameworkData() override;
        [[nodiscard]] bool serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        [[nodiscard]] bool deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext) override;
        void onValidate(ValidationReportImpl& report) const override;

        [[nodiscard]] ETextureAddressMode getWrapUMode() const;
        [[nodiscard]] ETextureAddressMode getWrapVMode() const;
        [[nodiscard]] ETextureAddressMode getWrapRMode() const;
        [[nodiscard]] ETextureSamplingMethod getMinSamplingMethod() const;
        [[nodiscard]] ETextureSamplingMethod getMagSamplingMethod() const;
        [[nodiscard]] uint32_t getAnisotropyLevel() const;

        [[nodiscard]] bool setTextureData(const Texture2D& texture);
        [[nodiscard]] bool setTextureData(const Texture3D& texture);
        [[nodiscard]] bool setTextureData(const TextureCube& texture);
        [[nodiscard]] bool setTextureData(const Texture2DBuffer& texture);
        [[nodiscard]] bool setTextureData(const ramses::RenderBuffer& texture);

        [[nodiscard]] ramses::internal::TextureSamplerHandle getTextureSamplerHandle() const;
        [[nodiscard]] ramses::internal::EDataType getTextureDataType() const;
        [[nodiscard]] ERamsesObjectType getTextureType() const;

    private:
        [[nodiscard]] bool setTextureDataInternal(ERamsesObjectType textureType,
            ramses::internal::TextureSampler::ContentType contentType,
            ramses::internal::ResourceContentHash textureHash,
            ramses::internal::MemoryHandle contentHandle);

        void validateRenderBuffer(ValidationReportImpl& report, ramses::internal::RenderBufferHandle renderBufferHandle) const;
        void validateTextureBuffer(ValidationReportImpl& report, ramses::internal::TextureBufferHandle textureBufferHandle) const;
        void validateResource(ValidationReportImpl& report, const Resource* resource) const;

        ERamsesObjectType m_textureType;

        ramses::internal::TextureSamplerHandle m_textureSamplerHandle;
    };
}
