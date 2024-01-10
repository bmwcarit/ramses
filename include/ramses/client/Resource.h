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
        class ResourceImpl;
    }

    /**
     * @brief The Resource is the base class of all resources, such as arrays and textures.
     * @ingroup CoreAPI
     */
    class RAMSES_API Resource : public SceneObject
    {
    public:
        /**
         * @brief Get resource Id.
         * @return the Id of the resource.
         */
        [[nodiscard]] resourceId_t getResourceId() const;

        /**
         * Get the internal data for implementation specifics of Resource.
         */
        [[nodiscard]] internal::ResourceImpl& impl();

        /**
         * Get the internal data for implementation specifics of Resource.
         */
        [[nodiscard]] const internal::ResourceImpl& impl() const;

    protected:
        /**
        * @brief Constructor for Resource.
        *
        * @param[in] impl Internal data for implementation specifics of Resource (sink - instance becomes owner)
        */
        explicit Resource(std::unique_ptr<internal::ResourceImpl> impl);

        /**
        * Stores internal data for implementation specifics of Resource.
        */
        internal::ResourceImpl& m_impl;
    };
}
