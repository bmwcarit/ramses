//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/Resource.h"
#include "ramses/framework/RamsesFrameworkTypes.h"
#include "ramses/framework/TextureEnums.h"

namespace ramses
{
    namespace internal
    {
        class Texture3DImpl;
    }

    /**
    * @brief Texture represents a texture resource.
    * @ingroup CoreAPI
    */
    class RAMSES_API Texture3D : public Resource
    {
    public:
        /**
        * @brief Gets texture width
        *
        * @return Texture width
        */
        [[nodiscard]] uint32_t getWidth() const;

        /**
        * @brief Gets texture height
        *
        * @return Texture height
        */
        [[nodiscard]] uint32_t getHeight() const;

        /**
        * @brief Gets texture depth
        *
        * @return Texture depth
        */
        [[nodiscard]] uint32_t getDepth() const;

        /**
        * @brief Gets texture format
        *
        * @return Texture format
        */
        [[nodiscard]] ETextureFormat getTextureFormat() const;

        /**
         * Get the internal data for implementation specifics of Texture.
         */
        [[nodiscard]] internal::Texture3DImpl& impl();

        /**
         * Get the internal data for implementation specifics of Texture.
         */
        [[nodiscard]] const internal::Texture3DImpl& impl() const;

    protected:
        /**
        * @brief Scene is the factory for creating Texture instances.
        */
        friend class internal::SceneObjectRegistry;

        /**
        * @brief Constructor of Texture
        *
        * @param[in] impl Internal data for implementation specifics of Texture (sink - instance becomes owner)
        */
        explicit Texture3D(std::unique_ptr<internal::Texture3DImpl> impl);

        /**
        * Stores internal data for implementation specifics of Texture.
        */
        internal::Texture3DImpl& m_impl;
    };
}
