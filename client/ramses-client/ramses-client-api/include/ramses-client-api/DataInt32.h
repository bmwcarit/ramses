//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DATAINT32_H
#define RAMSES_DATAINT32_H

#include "ramses-client-api/DataObject.h"

namespace ramses
{
    class DataObjectImpl;

    /**
     * @brief The DataInt32 data object stores an 32-bit integer value within a scene.
     */
    class RAMSES_API DataInt32 : public DataObject
    {
    public:
        /**
         * @brief Sets/updates the stored value.
         *
         * @param[in] value new value.
         * @return status == 0 for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t setValue(int32_t value);

        /**
         * @brief Gets the stored value.
         *
         * @param[out] value stored value.
         * @return status == 0 for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t getValue(int32_t& value) const;

    protected:
        /**
         * @brief Scene is the factory for creating DataInt32 instances.
         */
        friend class SceneImpl;

        /**
         * @brief Constructor of DataInt32
         *
         * @param[in] pimpl Internal data for implementation specifics of a DataObject (sink - instance becomes owner)
         */
        explicit DataInt32(DataObjectImpl& pimpl);

        /**
         * @brief Destructor of the DataInt32
         */
        virtual ~DataInt32();
    };
}

#endif
