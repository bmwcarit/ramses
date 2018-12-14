//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATION_H
#define RAMSES_ANIMATION_H

#include "ramses-client-api/AnimationObject.h"
#include "ramses-client-api/AnimationTypes.h"

namespace ramses
{
    /**
    * @brief The Animation combines spline with one or more AnimatedProperty instances
    * and allows control of the animation.
    */
    class RAMSES_API Animation : public AnimationObject
    {
    public:
        /**
        * @brief Gets global time stamp for when animation is set to start.
        *
        * @return Start global time stamp.
        */
        globalTimeStamp_t  getStartTime() const;

        /**
        * @brief Gets global time stamp for when animation is set to stop.
        *
        * @return Stop global time stamp.
        */
        globalTimeStamp_t  getStopTime() const;

        /**
        * @brief Stores internal data for implementation specifics of Animation.
        */
        class AnimationImpl& impl;

    protected:
        /**
        * @brief AnimationSystemData is the factory for creating Animation.
        */
        friend class AnimationSystemData;

        /**
        * @brief Constructor of the Animation
        *
        * @param[in] pimpl Internal data for implementation specifics of Animation (sink - instance becomes owner)
        */
        explicit Animation(AnimationImpl& pimpl);

        /**
        * @brief Copy constructor of Animation
        *
        * @param[in] other Other instance of Animation class
        */
        Animation(const Animation& other);

        /**
        * @brief Assignment operator of Animation.
        *
        * @param[in] other Other instance of Animation class
        * @return This instance after assignment
        */
        Animation& operator=(const Animation& other);

        /**
        * @brief Destructor of the Animation
        */
        virtual ~Animation();
    };
}

#endif
