//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ARRAYRESOURCE_H
#define RAMSES_ARRAYRESOURCE_H

#include "ramses-client-api/Resource.h"
#include "ramses-framework-api/EDataType.h"

namespace ramses
{
    /**
    * @brief The #ArrayResource stores a data array of a given type. The data is immutable.
    *        The resource can be used as input for a #ramses::GeometryBinding.
    *
    * @details If an #ArrayResource object is created with type #ramses::EDataType::ByteBlob then an element
    *          is defined as one byte, rather than a logical vertex element. Hence, number of elements is
    *          the same as size in bytes.
    */
    class RAMSES_API ArrayResource : public Resource
    {
    public:
        /**
        * Stores internal data for implementation specifics of ArrayResource.
        */
        class ArrayResourceImpl& impl;

        /**
        * @brief Returns number of elements of the array.
        */
        [[nodiscard]] uint32_t getNumberOfElements() const;

        /**
        * @brief Returns the data type of the data array.
        */
        [[nodiscard]] EDataType getDataType() const;

    protected:

        /**
        * @brief Scene is the factory for creating ArrayResource instances.
        */
        friend class SceneImpl;

        /**
        * @brief Constructor of ArrayResource
        *
        * @param[in] pimpl Internal data for implementation specifics of ArrayResource (sink - instance becomes owner)
        */
        explicit ArrayResource(ArrayResourceImpl& pimpl);

        /**
        * @brief Copy constructor of ArrayResource
        *
        * @param[in] other Other instance of ArrayResource class
        */
        ArrayResource(const ArrayResource& other);

        /**
        * @brief Assignment operator of ArrayResource.
        *
        * @param[in] other Other instance of ArrayResource class
        * @return This instance after assignment
        */
        ArrayResource& operator=(const ArrayResource& other);

        /**
        * @brief Destructor of the ArrayResource
        */
        virtual ~ArrayResource();
    };
}

#endif
