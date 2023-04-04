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
    * @brief   The DataObject is a base class for data container for storing data in a scene.
    * @details A concretely typed data object (see derived classes) can be bound to some inputs
    *          of some object types (e.g. #ramses::Appearance::bindInput or #ramses::Camera::bindViewportOffset).
    *          When a data object is bound to an input the data object value overrides whatever was previously
    *          set to that input using its direct setter.
    *          A single #DataObject can be bound to multiple inputs (also to other #ramses::RamsesObject types
    *          where applicable) providing a way to distribute value across instances/inputs.
    *          Using #DataObject also allows use of data linking across scenes between data object provider and consumer
    *          (#ramses::Scene::createDataProvider and #ramses::Scene::createDataConsumer) see SDK examples for typical use cases.
    *          A value set to a #DataObject is then propagated everywhere it is bound to and it is linked to.
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
        ~DataObject() override;
    };
}

#endif
