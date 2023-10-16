//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
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
     * @brief The #ramses::TextureSamplerMS is used to sample multisampled data when bound
     *      to a #ramses::Appearance uniform input (#ramses::Appearance::setInputTexture called with #ramses::TextureSamplerMS)
     */
    class RAMSES_API TextureSamplerMS : public SceneObject
    {
    public:
        /**
         * Get the internal data for implementation specifics of TextureSamplerMS.
         */
        [[nodiscard]] internal::TextureSamplerImpl& impl();

        /**
         * Get the internal data for implementation specifics of TextureSamplerMS.
         */
        [[nodiscard]] const internal::TextureSamplerImpl& impl() const;

    protected:
        /**
        * @brief Scene is the factory for creating TextureSamplerMS instances.
        */
        friend class internal::SceneObjectRegistry;

        /**
        * @brief Constructor for TextureSamplerMS.
        *
        * @param[in] impl Internal data for implementation specifics of TextureSamplerMS (sink - instance becomes owner)
        */
        explicit TextureSamplerMS(std::unique_ptr<internal::TextureSamplerImpl> impl);

        /**
        * Stores internal data for implementation specifics of TextureSamplerMS.
        */
        internal::TextureSamplerImpl& m_impl;
    };
}
