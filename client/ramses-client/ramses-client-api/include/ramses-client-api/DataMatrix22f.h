//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DATAMATRIX22F_H
#define RAMSES_DATAMATRIX22F_H

#include "ramses-client-api/DataObject.h"

namespace ramses
{
    class DataObjectImpl;

    /**
    * @brief The DataMatrix22f data object stores a matrix with 4 float components (2 rows, 2 columns) within a scene.
    */
    class RAMSES_API DataMatrix22f : public DataObject
    {
    public:
        /**
        * @brief Sets/updates the stored values of the matrix.
        *
        * @param[in] matrixElements new matrix values (row-wise).
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setValue(const float(&matrixElements)[4]);

        /**
        * @brief Gets all stored values of the matrix.
        *
        * @param[out] matrixElements of the matrix (row-wise).
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getValue(float(&matrixElements)[4]) const;

    protected:
        /**
        * @brief Scene is the factory for creating DataMatrix22f instances.
        */
        friend class SceneImpl;

        /**
        * @brief Constructor of DataMatrix22f
        *
        * @param[in] pimpl Internal data for implementation specifics of a DataObject (sink - instance becomes owner)
        */
        explicit DataMatrix22f(DataObjectImpl& pimpl);

        /**
        * @brief Destructor of the UniformDataVector4f
        */
        virtual ~DataMatrix22f();
    };
}

#endif
