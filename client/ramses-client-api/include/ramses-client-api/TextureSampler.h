//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTURESAMPLER_H
#define RAMSES_TEXTURESAMPLER_H

#include "ramses-client-api/SceneObject.h"

#include "TextureEnums.h"

namespace ramses
{
    class Texture2D;
    class Texture3D;
    class TextureCube;
    class Texture2DBuffer;
    class RenderBuffer;

    /**
     * @ingroup CoreAPI
     * @brief The TextureSampler holds a texture and its sampling parameters
     */
    class TextureSampler : public SceneObject
    {
    public:
        /**
        * @brief Gets the texture wrap mode for the u axis
        *
        * @return ETextureAddressMode wrap mode for u axis
        */
        [[nodiscard]] RAMSES_API ETextureAddressMode getWrapUMode() const;

        /**
        * @brief Gets the texture wrap mode for the v axis
        *
        * @return ETextureAddressMode wrap mode for v axis
        */
        [[nodiscard]] RAMSES_API ETextureAddressMode getWrapVMode() const;

        /**
        * @brief Gets the texture wrap mode for the r axis
        *
        * @return ETextureAddressMode wrap mode for r axis
        */
        [[nodiscard]] RAMSES_API ETextureAddressMode getWrapRMode() const;

        /**
        * @brief Gets the texture min sampling method
        *
        * @return ETextureSamplingMethod min sampling method
        */
        [[nodiscard]] RAMSES_API ETextureSamplingMethod getMinSamplingMethod() const;

        /**
        * @brief Gets the texture mag sampling method
        *
        * @return ETextureSamplingMethod mag sampling method
        */
        [[nodiscard]] RAMSES_API ETextureSamplingMethod getMagSamplingMethod() const;

        /**
        * @brief Gets the texture sampling anisotropy level
        *
        * @return The texture sampling anisotropy level.
        */
        [[nodiscard]] RAMSES_API uint32_t getAnisotropyLevel() const;

        /**
        * @brief Gets the type of the texture
        *
        * @return Type of the texture, see ERamsesObjectType enum for possible values.
        */
        [[nodiscard]] RAMSES_API ERamsesObjectType getTextureType() const;

        /**
        * @brief Replaces current texture content source with a new one.
        *        Texture data can be changed from/to any type/format with these exceptions:
        *          - Texture3D can only be changed to another Texture3D
        *          - texture data cannot be changed if this TextureSampler is marked as texture consumer
        *            to be used for data linking (see ramses::Scene::createTextureConsumer)
        *
        * @param[in] dataSource Texture data source to be used with this sampler.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t setTextureData(const Texture2D& dataSource);
        /// @copybrief setTextureData(const Texture2D&)
        RAMSES_API status_t setTextureData(const Texture3D& dataSource);
        /// @copybrief setTextureData(const Texture2D&)
        RAMSES_API status_t setTextureData(const TextureCube& dataSource);
        /// @copybrief setTextureData(const Texture2D&)
        RAMSES_API status_t setTextureData(const Texture2DBuffer& dataSource);
        /// @copybrief setTextureData(const Texture2D&)
        RAMSES_API status_t setTextureData(const RenderBuffer& dataSource);

        /**
        * Stores internal data for implementation specifics of TextureSampler.
        */
        class TextureSamplerImpl& m_impl;

    protected:
        /**
        * @brief Scene is the factory for creating TextureSampler instances.
        */
        friend class RamsesObjectRegistry;

        /**
        * @brief Constructor for TextureSampler.
        *
        * @param[in] impl Internal data for implementation specifics of TextureSampler (sink - instance becomes owner)
        */
        explicit TextureSampler(std::unique_ptr<TextureSamplerImpl> impl);
    };
}

#endif
