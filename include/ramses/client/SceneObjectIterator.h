//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesObject.h"

namespace ramses
{
    namespace internal
    {
        class ObjectIteratorImpl;
    }

    class Scene;

    /**
    * @ingroup CoreAPI
    * @brief The SceneObjectIterator traverses objects in a Scene.
    *
    * It provides a way to traverse all objects owned by a given scene.
    */
    class RAMSES_API SceneObjectIterator
    {
    public:
        /**
        * @brief A SceneObjectIterator can iterate through objects of given type within a scene.
        * Note that this will not iterate over #ramses::LogicObject instances created from #ramses::LogicEngine,
        * use #ramses::LogicEngine::getCollection for those.
        *
        * @param[in] scene Scene whose objects to iterate through
        * @param[in] objectType Optional type of objects to iterate through.
        **/
        explicit SceneObjectIterator(const Scene& scene, ERamsesObjectType objectType = ERamsesObjectType::RamsesObject);

        /**
        * @brief Destructor
        **/
        ~SceneObjectIterator();

        /**
        * @brief Iterate through all objects of given type
        * @return next object, null if no more objects available
        *
        * Iterator is invalid and may no longer be used if any objects are added or removed.
        **/
        RamsesObject* getNext();

    private:
        std::unique_ptr<internal::ObjectIteratorImpl> m_impl;
    };
}
