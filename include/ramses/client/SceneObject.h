//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/ClientObject.h"

namespace ramses
{
    namespace internal
    {
        class SceneObjectImpl;
        class SceneObjectRegistry;
    }
    class Scene;

    /**
    * @brief The SceneObject is a base class for all client API objects owned by a Scene.
    * @ingroup CoreAPI
    */
    class RAMSES_API SceneObject : public ClientObject
    {
    public:
        /**
        * @brief Returns scene object id which is automatically assigned at creation time of object and is unique within scope of one scene.
        *
        * @return Scene object id.
        */
        [[nodiscard]] sceneObjectId_t getSceneObjectId() const;

        /**
         * Get the owning #ramses::Scene.
         * @return owning #ramses::Scene
         */
        [[nodiscard]] const Scene& getScene() const;

        /**
         * Get the owning #ramses::Scene.
         * @return owning #ramses::Scene
         */
        [[nodiscard]] Scene& getScene();

        /**
         * Get the internal data for implementation specifics of SceneObject.
         */
        [[nodiscard]] internal::SceneObjectImpl& impl();

        /**
         * Get the internal data for implementation specifics of SceneObject.
         */
        [[nodiscard]] const internal::SceneObjectImpl& impl() const;

    protected:
        /**
        * @brief Constructor for SceneObject.
        *
        * @param[in] impl Internal data for implementation specifics of SceneObject (sink - instance becomes owner)
        */
        explicit SceneObject(std::unique_ptr<internal::SceneObjectImpl> impl);

        /**
        * Stores internal data for implementation specifics of SceneObject.
        */
        internal::SceneObjectImpl& m_impl;

        /**
        * @brief Scene is the factory for creating SceneReference instances.
        */
        friend class internal::SceneObjectRegistry;
    };
}
