//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEOBJECT_H
#define RAMSES_SCENEOBJECT_H

#include "ramses-client-api/ClientObject.h"

namespace ramses
{
    /**
    * @ingroup CoreAPI
    * @brief The SceneObject is a base class for all client API objects owned by a Scene.
    */
    class SceneObject : public ClientObject
    {
    public:
        /**
        * Stores internal data for implementation specifics of SceneObject.
        */
        class SceneObjectImpl& m_impl;

        /**
        * @brief Returns scene object id which is automatically assigned at creation time of object and is unique within scope of one scene.
        *
        * @return Scene object id.
        */
        [[nodiscard]] RAMSES_API sceneObjectId_t getSceneObjectId() const;

        /**
        * @brief Returns sceneid to which this object belongs to
        *
        * @return Scene id this object belongs to
        */
        [[nodiscard]] RAMSES_API sceneId_t getSceneId() const;

    protected:
        /**
        * @brief Constructor for SceneObject.
        *
        * @param[in] impl Internal data for implementation specifics of SceneObject (sink - instance becomes owner)
        */
        explicit SceneObject(std::unique_ptr<SceneObjectImpl> impl);
    };
}

#endif
