//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONOBJECT_H
#define RAMSES_ANIMATIONOBJECT_H

#include "ramses-client-api/SceneObject.h"

namespace ramses
{
    /**
    * @brief The AnimationObject is a base class for all client API objects owned by an AnimationSystem.
    */
    class RAMSES_API AnimationObject : public SceneObject
    {
    public:
        /**
        * Stores internal data for implementation specifics of AnimationObject.
        */
        class AnimationObjectImpl& impl;

    protected:
        /**
        * @brief Constructor for AnimationObject.
        *
        * @param[in] pimpl Internal data for implementation specifics of AnimationObject (sink - instance becomes owner)
        */
        explicit AnimationObject(AnimationObjectImpl& pimpl);

        /**
        * @brief Destructor of the AnimationObject
        */
        virtual ~AnimationObject();

        /**
        * @brief AnimationSystemData is the factory for creating animated properties.
        */
        friend class AnimationSystemData;

    private:
        /**
        * @brief Copy constructor of AnimationObject
        */
        AnimationObject(const AnimationObject& other);

        /**
        * @brief Assignment operator of AnimationObject.
        *
        * @param[in] other Instance to assign from
        * @return This instance after assignment
        */
        AnimationObject& operator=(const AnimationObject& other);
    };
}

#endif

