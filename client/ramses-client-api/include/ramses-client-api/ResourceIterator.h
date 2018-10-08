//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCEITERATOR_H
#define RAMSES_RESOURCEITERATOR_H

#include "ramses-client-api/RamsesObject.h"
#include "ramses-client-api/Resource.h"

namespace ramses
{
    class ResourceIteratorImpl;
    class RamsesClient;

    /**
    * @brief A ResourceIterator can iterate through resources of given type on a RamsesClient.
    **/
    class RAMSES_API ResourceIterator
    {
    public:

        /**
        * @brief A ResourceIterator can iterate through resources of given type.
        *
        * @param[in] client RamsesClient whose resources to iterate through
        * @param[in] objectType Optional type of resources to iterate through.
        **/
        ResourceIterator(const RamsesClient& client, ERamsesObjectType objectType = ERamsesObjectType_Resource);

        /**
        * @brief Destructor
        **/
        ~ResourceIterator();

        /**
        * @brief Returns the next resource while iterating.
        *
        * @return The next object, null when no more resources are available.
        * The iterator is invalid and may not be used after any resources are added or removed.
        **/
        Resource* getNext();

    private:
        ResourceIterator(const ResourceIterator& iterator);
        ResourceIterator& operator = (const ResourceIterator& iterator);
        ResourceIteratorImpl* impl;
    };
}

#endif
