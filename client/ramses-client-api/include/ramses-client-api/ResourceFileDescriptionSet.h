//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCEFILEDESCRIPTIONSET_H
#define RAMSES_RESOURCEFILEDESCRIPTIONSET_H

#include "ramses-client-api/Resource.h"

namespace ramses
{
    class ResourceFileDescription;

    /**
    * @brief A set holding multiple ResourceFileDescription ordered by index.
    */
    class RAMSES_API ResourceFileDescriptionSet
    {
    public:
        /**
        * @brief Constructor for ResourceFileDescriptionSet.
        */
        ResourceFileDescriptionSet();

        /**
        * @brief Destructor for ResourceFileDescriptionSet.
        */
        ~ResourceFileDescriptionSet();

        /**
        * @brief Adds a ResourceFileDescription to the ResourceFileDescriptionSet.
        *
        * @param[in] description The ResourceFileDescription to add.
        */
        void add(const ResourceFileDescription& description);

        /**
        * @brief Returns the number of ResourceFileDescription contained in the set.
        *
        * @return the number of ResourceFileDescription contained in the set.
        */
        uint32_t getNumberOfDescriptions() const;

        /**
        * @brief Returns the ResourceFileDescription with specified index from the ResourceFileDescriptionSet.
        *        The ResourceFileDescription objects are ordered in the ResourceFileDescriptionSet.
        *
        * @param[in] index The ResourceFileDescription index in the ResourceFileDescriptionSet.
        * @return the requested ResourceFileDescription by index.
        */
        const ResourceFileDescription& getDescription(uint32_t index) const;

        /**
        * @brief Stores internal data for implementation specifics of ResourceFileDescriptionSet.
        */
        class ResourceFileDescriptionSetImpl* impl;

        /**
        * @brief Copies a ResourceFileDesciption
        */
        ResourceFileDescriptionSet(const ResourceFileDescriptionSet&);
    private:
        ResourceFileDescriptionSet& operator=(ResourceFileDescriptionSet&);
    };
}

#endif //RAMSES_RESOURCEFILEDESCRIPTIONSET_H
