//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCEFILEDESCRIPTION_H
#define RAMSES_RESOURCEFILEDESCRIPTION_H

#include "ramses-client-api/Resource.h"

namespace ramses
{
    /**
    * @brief The resource file description holds meta information about a file containing resources (resource count, hashes of contained resources etc.)
    */
    class RAMSES_API ResourceFileDescription
    {
    public:
        /**
        * @brief Constructor for ResourceFileDescription.
        *
        * @param[in] filename Name of the file containing resources.
        */
        explicit ResourceFileDescription(const char* filename);

        /**
        * @brief Destructor for ResourceFileDescription.
        */
        ~ResourceFileDescription();

        /**
        * @brief Returns the file containing resources.
        *
        * @return filename Name of the file containing resources.
        */
        const char* getFilename() const;

        /**
        * @brief Adds a resource to the file description for storing.
        *
        * @param[in] resourceObject The resource object to store.
        */
        void add(const Resource* resourceObject);

        /**
        * @brief Returns true if the resource is contained in the ResourceFileDescription
        *
        * @param[in] resourceObject the object in question
        * @return true if the resource is contained in the ResourceFileDescription.
        */
        bool contains(const Resource* resourceObject) const;

        /**
        * @brief Returns the total number of resources contained in the ResourceFileDescription
        *
        * @return the total number of resources contained in the ResourceFileDescription.
        */
        uint32_t getNumberOfResources() const;

        /**
        * @brief Returns the resource object with specified index from the ResourceFileDescription.
        *        The resource objects are ordered in the ResourceFileDescription.
        *
        * @param[in] index The resource index in the ResourceFileDescription.
        * @return the requested resource by index.
        */
        const Resource& getResource(uint32_t index) const;

        /**
        * @brief Stores internal data for implementation specifics of ResourceFileDescription.
        */
        class ResourceFileDescriptionImpl* impl;

        /**
        * @brief Copies a ResourceFileDesciption
        */
        ResourceFileDescription(const ResourceFileDescription&);

        /**
        * @brief Assigns a ResourceFileDesciption
        * @return This after assignment
        */
        ResourceFileDescription& operator=(const ResourceFileDescription&);
    };
}

#endif
