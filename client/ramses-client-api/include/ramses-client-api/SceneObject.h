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
    * @brief The SceneObject is a base class for all client API objects owned by a Scene.
    */
    class RAMSES_API SceneObject : public ClientObject
    {
    public:
        /**
        * Stores internal data for implementation specifics of SceneObject.
        */
        class SceneObjectImpl& impl;

        /**
        * @brief Returns scene object id which is automatically assigned at creation time of object and is unique within scope of one scene.
        *
        * @return Scene object id.
        */
        [[nodiscard]] sceneObjectId_t getSceneObjectId() const;

        /**
        * @brief Returns sceneid to which this object belongs to
        *
        * @return Scene id this object belongs to
        */
        [[nodiscard]] sceneId_t getSceneId() const;

    protected:
        /**
        * @brief Constructor for SceneObject.
        *
        * @param[in] pimpl Internal data for implementation specifics of SceneObject (sink - instance becomes owner)
        */
        explicit SceneObject(SceneObjectImpl& pimpl);

        /**
        * @brief Destructor of the SceneObject
        */
        ~SceneObject() override;

        /**
        * SceneImpl is the factory for creating object instances which derive from SceneObject.
        */
        friend class SceneImpl;

    private:
        /**
        * @brief Copy constructor of SceneObject
        */
        SceneObject(const SceneObject& other);

        /**
        * @brief Assignment operator of SceneObject.
        *
        * @param[in] other Instance to assign from
        * @return This instance after assignment
        */
        SceneObject& operator=(const SceneObject& other);
    };
}

#endif
