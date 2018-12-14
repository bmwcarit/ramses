//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_VECTOR2FARRAY_H
#define RAMSES_VECTOR2FARRAY_H

#include "ramses-client-api/Resource.h"

namespace ramses
{
    /**
     * @brief The Vector2fArray stores Vector2f's.
    */
    class RAMSES_API Vector2fArray : public Resource
    {
    public:

        /**
        * Stores internal data for implementation specifics of Vector2fArray.
        */
        class ArrayResourceImpl& impl;

    protected:

        /**
        * @brief RamsesClient is the factory for creating Vector2fArray instances.
        */
        friend class RamsesClientImpl;

        /**
        * @brief Constructor of Vector2fArray
        *
        * @param[in] pimpl Internal data for implementation specifics of Vector2fArray (sink - instance becomes owner)
        */
        explicit Vector2fArray(ArrayResourceImpl& pimpl);

        /**
        * @brief Destructor of the Vector2fArray
        */
        virtual ~Vector2fArray();
    };
}

#endif
