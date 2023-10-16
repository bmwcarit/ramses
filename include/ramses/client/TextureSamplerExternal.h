//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/SceneObject.h"

namespace ramses
{
    namespace internal
    {
        class TextureSamplerImpl;
    }

    /**
    * @ingroup CoreAPI
    * @brief The #ramses::TextureSamplerExternal is used to sample from external textures data when bound
    *      to a #ramses::Appearance uniform input (#ramses::Appearance::setInputTexture called with #ramses::TextureSamplerExternal)
    */
    class RAMSES_API TextureSamplerExternal : public SceneObject
    {
    public:
        /**
         * Get the internal data for implementation specifics of TextureSamplerExternal.
         */
        [[nodiscard]] internal::TextureSamplerImpl& impl();

        /**
         * Get the internal data for implementation specifics of TextureSamplerExternal.
         */
        [[nodiscard]] const internal::TextureSamplerImpl& impl() const;

    protected:
        /**
        * @brief Scene is the factory for creating TextureSamplerExternal instances.
        */
        friend class internal::SceneObjectRegistry;

        /**
        * @brief Constructor for TextureSamplerExternal.
        *
        * @param[in] impl Internal data for implementation specifics of TextureSamplerExternal (sink - instance becomes owner)
        */
        explicit TextureSamplerExternal(std::unique_ptr<internal::TextureSamplerImpl> impl);

        /**
        * Stores internal data for implementation specifics of TextureSamplerExternal.
        */
        internal::TextureSamplerImpl& m_impl;
    };
}
