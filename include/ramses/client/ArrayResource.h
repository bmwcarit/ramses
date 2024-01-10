//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/Resource.h"
#include "ramses/framework/EDataType.h"

namespace ramses
{
    namespace internal
    {
        class ArrayResourceImpl;
    }

    /**
    * @brief The #ArrayResource stores a data array of a given type. The data is immutable.
    *        The resource can be used as an input for a #ramses::Geometry.
    *
    * @details If an #ArrayResource object is created with type #ramses::EDataType::ByteBlob then an element
    *          is defined as one byte, rather than a logical vertex element. Hence, number of elements is
    *          the same as the size in bytes.
    * @ingroup CoreAPI
    */
    class RAMSES_API ArrayResource : public Resource
    {
    public:
        /**
        * @brief Returns number of elements of the array.
        */
        [[nodiscard]] size_t getNumberOfElements() const;

        /**
        * @brief Returns the data type of the data array.
        */
        [[nodiscard]] EDataType getDataType() const;

        /**
         * Get the internal data for implementation specifics of ArrayResource.
         */
        [[nodiscard]] internal::ArrayResourceImpl& impl();

        /**
         * Get the internal data for implementation specifics of ArrayResource.
         */
        [[nodiscard]] const internal::ArrayResourceImpl& impl() const;

    protected:
        /**
        * @brief Scene is the factory for creating ArrayResource instances.
        */
        friend class internal::SceneObjectRegistry;

        /**
        * @brief Constructor of ArrayResource
        *
        * @param[in] impl Internal data for implementation specifics of ArrayResource (sink - instance becomes owner)
        */
        explicit ArrayResource(std::unique_ptr<internal::ArrayResourceImpl> impl);

        /**
        * Stores internal data for implementation specifics of ArrayResource.
        */
        internal::ArrayResourceImpl& m_impl;
    };
}
