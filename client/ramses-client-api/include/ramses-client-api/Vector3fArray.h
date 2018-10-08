//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_VECTOR3FARRAY_H
#define RAMSES_VECTOR3FARRAY_H

#include "ramses-client-api/Resource.h"

namespace ramses
{
    /**
     * @brief The Vector3fArray stores Vector3f's.
    */
    class RAMSES_API Vector3fArray : public Resource
    {
    public:

        /**
        * Stores internal data for implementation specifics of Vector3fArray.
        */
        class ArrayResourceImpl& impl;

    protected:

        /**
        * @brief RamsesClient is the factory for creating Vector3fArray instances.
        */
        friend class RamsesClientImpl;

        /**
        * @brief Constructor of Vector3fArray
        *
        * @param[in] pimpl Internal data for implementation specifics of Vector3fArray (sink - instance becomes owner)
        */
        explicit Vector3fArray(ArrayResourceImpl& pimpl);

        /**
        * @brief Destructor of the Vector3fArray
        */
        virtual ~Vector3fArray();
    };
}

#endif
