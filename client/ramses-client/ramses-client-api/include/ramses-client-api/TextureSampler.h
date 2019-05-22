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
    class StreamTexture;

    /**
     * @brief The TextureSampler holds a texture and its sampling parameters
     */
    class RAMSES_API TextureSampler : public SceneObject
    {
    public:
        /**
        * @brief Gets the texture wrap mode for the u axis
        *
        * @return ETextureAddressMode wrap mode for u axis
        */
        ETextureAddressMode getWrapUMode() const;

        /**
        * @brief Gets the texture wrap mode for the v axis
        *
        * @return ETextureAddressMode wrap mode for v axis
        */
        ETextureAddressMode getWrapVMode() const;

        /**
        * @brief Gets the texture wrap mode for the r axis
        *
        * @return ETextureAddressMode wrap mode for r axis
        */
        ETextureAddressMode getWrapRMode() const;

        /**
        * @brief Gets the texture min sampling method
        *
        * @return ETextureSamplingMethod min sampling method
        */
        ETextureSamplingMethod getMinSamplingMethod() const;

        /**
        * @brief Gets the texture mag sampling method
        *
        * @return ETextureSamplingMethod mag sampling method
        */
        ETextureSamplingMethod getMagSamplingMethod() const;

        /**
        * @brief Gets the texture sampling anisotropy level
        *
        * @return The texture sampling anisotropy level.
        */
        uint32_t getAnisotropyLevel() const;

        /**
        * @brief Gets the type of the texture
        *
        * @return Type of the texture, see ERamsesObjectType enum for possible values.
        */
        ERamsesObjectType getTextureType() const;

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
        status_t setTextureData(const Texture2D& dataSource);
        /// @copydoc setTextureData(const Texture2D&)
        status_t setTextureData(const Texture3D& dataSource);
        /// @copydoc setTextureData(const Texture2D&)
        status_t setTextureData(const TextureCube& dataSource);
        /// @copydoc setTextureData(const Texture2D&)
        status_t setTextureData(const Texture2DBuffer& dataSource);
        /// @copydoc setTextureData(const Texture2D&)
        status_t setTextureData(const RenderBuffer& dataSource);
        /// @copydoc setTextureData(const Texture2D&)
        status_t setTextureData(const StreamTexture& dataSource);

        /**
        * Stores internal data for implementation specifics of TextureSampler.
        */
        class TextureSamplerImpl& impl;

    protected:
        /**
        * @brief Scene is the factory for creating TextureSampler instances.
        */
        friend class SceneImpl;

        /**
        * @brief Constructor for TextureSampler.
        *
        * @param[in] pimpl Internal data for implementation specifics of TextureSampler (sink - instance becomes owner)
        */
        explicit TextureSampler(TextureSamplerImpl& pimpl);

        /**
        * @brief Destructor of the TextureSampler
        */
        virtual ~TextureSampler();
    };
}

#endif
