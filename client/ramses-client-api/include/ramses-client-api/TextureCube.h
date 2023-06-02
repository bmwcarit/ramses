//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTURECUBE_H
#define RAMSES_TEXTURECUBE_H

#include "ramses-client-api/Resource.h"
#include "ramses-client-api/TextureEnums.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-client-api/TextureEnums.h"
#include "TextureSwizzle.h"


namespace ramses
{
    /**
     * @ingroup CoreAPI
     * @brief TextureCube stores pixel data with 6 equally sized quadratic faces.
     */
    class TextureCube : public Resource
    {
    public:
        /**
        * @brief Gets cube texture edge length
        *
        * @return Texture cube edge length
        */
        [[nodiscard]] RAMSES_API uint32_t getSize() const;

        /**
        * @brief Gets texture format
        *
        * @return Texture format
        */
        [[nodiscard]] RAMSES_API ETextureFormat getTextureFormat() const;

        /**
        * @brief Gets swizzle description
        *
        * @return Swizzle Description
        */
        [[nodiscard]] RAMSES_API const TextureSwizzle& getTextureSwizzle() const;

        /**
        * Stores internal data for implementation specifics of TextureCube.
        */
        class TextureCubeImpl& m_impl;

    protected:
        /**
        * @brief Scene is the factory for creating TextureCube instances.
        */
        friend class RamsesObjectRegistry;

        /**
        * @brief Constructor of TextureCube
        *
        * @param[in] impl Internal data for implementation specifics of TextureCube (sink - instance becomes owner)
        */
        explicit TextureCube(std::unique_ptr<TextureCubeImpl> impl);
    };
}

#endif
