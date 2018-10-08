//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DATAFLOAT_H
#define RAMSES_DATAFLOAT_H

#include "ramses-client-api/DataObject.h"

namespace ramses
{
    class DataObjectImpl;

    /**
     * @brief The DataFloat data object stores a float value within a scene.
     */
    class RAMSES_API DataFloat : public DataObject
    {
    public:
        /**
         * @brief Sets/updates the stored value.
         *
         * @param[in] value new value.
         * @return status == 0 for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t setValue(float value);

        /**
         * @brief Gets the stored value.
         *
         * @param[out] value stored value.
         * @return status == 0 for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t getValue(float& value) const;

    protected:
        /**
         * @brief Scene is the factory for creating DataFloat instances.
         */
        friend class SceneImpl;

        /**
         * @brief Constructor of DataFloat
         *
         * @param[in] pimpl Internal data for implementation specifics of a DataObject (sink - instance becomes owner)
         */
        explicit DataFloat(DataObjectImpl& pimpl);

        /**
         * @brief Destructor of the DataFloat
         */
        virtual ~DataFloat();
    };
}

#endif
