//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONSYSTEMOBJECTITERATOR_H
#define RAMSES_ANIMATIONSYSTEMOBJECTITERATOR_H

#include "ramses-client-api/RamsesObject.h"

namespace ramses
{
    class AnimationSystemIteratorImpl;
    class AnimationSystem;

    /**
    * @brief The AnimationSystemObjectIterator iterates over objects of given type in an AnimationSystem.
    *
    * It provides a way to traverse all objects of given type owned by a given animation system.
    */
    class RAMSES_API AnimationSystemObjectIterator
    {
    public:
        /**
        *
        * @brief A AnimationSystemObjectIterator can iterate through objects of given type within an animation system.
        * @param[in] animationSystem AnimationSystem whose objects to iterate through.
        * @param[in] objectType Optional type of objects to iterate through.
        **/
        AnimationSystemObjectIterator(const AnimationSystem& animationSystem, ERamsesObjectType objectType = ERamsesObjectType_RamsesObject);

        /**
        *
        * @brief Destructor
        **/
        ~AnimationSystemObjectIterator();

        /**
        *
        * @brief Iterate through all objects of given type
        * @return next object, null if no more objects available
        *
        * Iterator is invalid and may no longer be used if any objects are added or removed.
        **/
        RamsesObject* getNext();

    private:
        AnimationSystemObjectIterator(const AnimationSystemObjectIterator& iterator);
        AnimationSystemObjectIterator& operator=(const AnimationSystemObjectIterator& iterator);
        AnimationSystemIteratorImpl* impl;
    };
}

#endif
