//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FLOATARRAY_H
#define RAMSES_FLOATARRAY_H

#include "ramses-client-api/Resource.h"

namespace ramses
{
    /**
     * @brief The FloatArray stores float values.
    */
    class RAMSES_API FloatArray : public Resource
    {
    public:
        /**
        * Stores internal data for implementation specifics of FloatArray.
        */
        class ArrayResourceImpl& impl;

    protected:

        /**
        * @brief RamsesClient is the factory for creating FloatArray instances.
        */
        friend class RamsesClientImpl;

        /**
        * @brief Constructor of FloatArray
        *
        * @param[in] pimpl Internal data for implementation specifics of FloatArray (sink - instance becomes owner)
        */
        explicit FloatArray(ArrayResourceImpl& pimpl);

        /**
        * @brief Copy constructor of FloatArray
        *
        * @param[in] other Other instance of FloatArray class
        */
        FloatArray(const FloatArray& other);

        /**
        * @brief Assignment operator of FloatArray.
        *
        * @param[in] other Other instance of FloatArray class
        * @return This instance after assignment
        */
        FloatArray& operator=(const FloatArray& other);

        /**
        * @brief Destructor of the FloatArray
        */
        virtual ~FloatArray();
    };
}

#endif // RAMSES_FLOATARRAY_H
