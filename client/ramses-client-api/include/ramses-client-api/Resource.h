//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCE_H
#define RAMSES_RESOURCE_H

#include "ramses-client-api/SceneObject.h"

namespace ramses
{
    /**
     * @ingroup CoreAPI
     * @brief The Resource is the base class of all resources, such as arrays and textures.
     */
    class Resource : public SceneObject
    {
    public:
        /**
         * @brief Get resource Id.
         * @return the Id of the resource.
         */
        [[nodiscard]] RAMSES_API resourceId_t getResourceId() const;

        /**
        * Stores internal data for implementation specifics of Resource.
        */
        class ResourceImpl& m_impl;

    protected:
        /**
        * @brief Constructor for Resource.
        *
        * @param[in] impl Internal data for implementation specifics of Resource (sink - instance becomes owner)
        */
        explicit Resource(std::unique_ptr<ResourceImpl> impl);
    };
}

#endif
