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
     * @brief The Resource is the base class of all resources, such as arrays and textures.
     */
    class RAMSES_API Resource : public SceneObject
    {
    public:
        /**
         * @brief Get resource Id.
         * @return the Id of the resource.
         */
        resourceId_t getResourceId() const;

        /**
        * Stores internal data for implementation specifics of Resource.
        */
        class ResourceImpl& impl;

    protected:
        /**
        * @brief RamsesClient is the factory for destroying Resource instances.
        */
        friend class RamsesClientImpl;

        /**
        * @brief Constructor for Resource.
        *
        * @param[in] pimpl Internal data for implementation specifics of Resource (sink - instance becomes owner)
        */
        explicit Resource(ResourceImpl& pimpl);

        /**
        * @brief Destructor for Resource.
        */
        virtual ~Resource();
    };
}

#endif
