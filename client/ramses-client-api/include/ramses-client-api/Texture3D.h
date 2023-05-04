//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTURE3D_H
#define RAMSES_TEXTURE3D_H

#include "ramses-client-api/Resource.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "TextureEnums.h"

namespace ramses
{
    /**
    * @ingroup CoreAPI
    * @brief Texture represents a texture resource
    */
    class Texture3D : public Resource
    {
    public:
        /**
        * @brief Gets texture width
        *
        * @return Texture width
        */
        [[nodiscard]] RAMSES_API uint32_t getWidth() const;

        /**
        * @brief Gets texture height
        *
        * @return Texture height
        */
        [[nodiscard]] RAMSES_API uint32_t getHeight() const;

        /**
        * @brief Gets texture depth
        *
        * @return Texture depth
        */
        [[nodiscard]] RAMSES_API uint32_t getDepth() const;

        /**
        * @brief Gets texture format
        *
        * @return Texture format
        */
        [[nodiscard]] RAMSES_API ETextureFormat getTextureFormat() const;

        /**
        * Stores internal data for implementation specifics of Texture.
        */
        class Texture3DImpl& m_impl;

    protected:
        /**
        * @brief Scene is the factory for creating Texture instances.
        */
        friend class RamsesObjectRegistry;

        /**
        * @brief Constructor of Texture
        *
        * @param[in] impl Internal data for implementation specifics of Texture (sink - instance becomes owner)
        */
        explicit Texture3D(std::unique_ptr<Texture3DImpl> impl);
    };
}

#endif
