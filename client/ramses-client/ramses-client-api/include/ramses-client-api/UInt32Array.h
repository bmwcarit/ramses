//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_UINT32ARRAY_H
#define RAMSES_UINT32ARRAY_H

#include "ramses-client-api/Resource.h"

namespace ramses
{
    /**
     * @brief The UInt32Array is a list of 16 bit unsigned values (typically used as index arrays)
    */
    class RAMSES_API UInt32Array : public Resource
    {
    public:

        /**
        * Stores internal data for implementation specifics of UInt32Array.
        */
        class ArrayResourceImpl& impl;

    protected:
        /**
        * @brief RamsesClient is the factory for creating UInt32Array instances.
        */
        friend class RamsesClientImpl;

        /**
        * @brief Constructor of UInt32Array
        *
        * @param[in] pimpl Internal data for implementation specifics of UInt32Array (sink - instance becomes owner)
        */
        explicit UInt32Array(ArrayResourceImpl& pimpl);

        /**
        * @brief Destructor of the UInt32Array
        */
        virtual ~UInt32Array();
    };
}

#endif
