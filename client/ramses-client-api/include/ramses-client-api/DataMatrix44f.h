//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DATAMATRIX44F_H
#define RAMSES_DATAMATRIX44F_H

#include "ramses-client-api/DataObject.h"

namespace ramses
{
    class DataObjectImpl;

    /**
     * @brief The DataMatrix44f data object stores a matrix with 16 float components (4 rows, 4 columns) within a scene.
     */
    class RAMSES_API DataMatrix44f : public DataObject
    {
    public:
        /**
        * @brief Sets/updates the stored values of the matrix.
        *
        * @param[in] matrixElements new matrix values (row-wise).
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setValue(const float(&matrixElements)[16]);

        /**
        * @brief Gets all stored values of the matrix.
        *
        * @param[out] matrixElements of the matrix (row-wise).
        * @return status == 0 for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getValue(float(&matrixElements)[16]) const;

    protected:
        /**
        * @brief Scene is the factory for creating DataMatrix44f instances.
        */
        friend class SceneImpl;

        /**
        * @brief Constructor of DataMatrix44f
        *
        * @param[in] pimpl Internal data for implementation specifics of a DataObject (sink - instance becomes owner)
        */
        explicit DataMatrix44f(DataObjectImpl& pimpl);

        /**
        * @brief Destructor of the UniformDataVector4f
        */
        ~DataMatrix44f() override;
    };
}

#endif
