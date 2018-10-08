//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DATAOBJECT_H
#define RAMSES_DATAOBJECT_H

#include "ramses-client-api/SceneObject.h"

namespace ramses
{
    /**
    * @brief The DataObject is a base class for data container for storing data in a scene.
    */
    class RAMSES_API DataObject : public SceneObject
    {
    public:
        /**
        * Stores internal data for implementation specifics
        */
        class DataObjectImpl& impl;

    protected:
        /**
        * @brief Scene is the factory for creating DataObject instances.
        */
        friend class SceneImpl;

        /**
        * @brief Constructor of DataObject
        *
        * @param[in] pimpl Internal data for implementation specifics of DataObject
        */
        explicit DataObject(DataObjectImpl& pimpl);

        /**
        * @brief Destructor for a DataObject
        */
        virtual ~DataObject();
    };
}

#endif
