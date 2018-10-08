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
        * @brief Gets the texture sampling method
        *
        * @return ETextureSamplingMethod sampling method
        */
        ETextureSamplingMethod getSamplingMethod() const;

        /**
        * @brief Gets the texture sampling anisotropy level
        *
        * @return The texture sampling anisotropy level.
        */
        uint32_t getAnisotropyLevel() const;

        /**
        * Stores internal data for implementation specifics of TextureSampler.
        */
        class TextureSamplerImpl& impl;

        /**
        * @brief Gets the type of the texture
        *
        * @return Type of the texture, see ERamsesObjectType enum for possible values.
        */
        ERamsesObjectType getTextureType() const;

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
