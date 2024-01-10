//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/Resource.h"
#include "ramses/framework/TextureEnums.h"
#include "ramses/framework/RamsesFrameworkTypes.h"
#include "ramses/framework/TextureEnums.h"
#include "TextureSwizzle.h"


namespace ramses
{
    namespace internal
    {
        class TextureCubeImpl;
    }

    /**
     * @brief TextureCube stores pixel data with 6 equally sized quadratic faces.
     * @ingroup CoreAPI
     */
    class RAMSES_API TextureCube : public Resource
    {
    public:
        /**
        * @brief Gets cube texture edge length
        *
        * @return Texture cube edge length
        */
        [[nodiscard]] uint32_t getSize() const;

        /**
        * @brief Gets texture format
        *
        * @return Texture format
        */
        [[nodiscard]] ETextureFormat getTextureFormat() const;

        /**
        * @brief Gets swizzle description
        *
        * @return Swizzle Description
        */
        [[nodiscard]] const TextureSwizzle& getTextureSwizzle() const;

        /**
         * Get the internal data for implementation specifics of TextureCube.
         */
        [[nodiscard]] internal::TextureCubeImpl& impl();

        /**
         * Get the internal data for implementation specifics of TextureCube.
         */
        [[nodiscard]] const internal::TextureCubeImpl& impl() const;

    protected:
        /**
        * @brief Scene is the factory for creating TextureCube instances.
        */
        friend class internal::SceneObjectRegistry;

        /**
        * @brief Constructor of TextureCube
        *
        * @param[in] impl Internal data for implementation specifics of TextureCube (sink - instance becomes owner)
        */
        explicit TextureCube(std::unique_ptr<internal::TextureCubeImpl> impl);

        /**
        * Stores internal data for implementation specifics of TextureCube.
        */
        internal::TextureCubeImpl& m_impl;
    };
}
