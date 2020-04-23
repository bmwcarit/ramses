//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONSYSTEMREALTIME_H
#define RAMSES_ANIMATIONSYSTEMREALTIME_H

#include "ramses-client-api/AnimationSystem.h"

namespace ramses
{
    /**
    * @brief The AnimationSystemRealTime is a special version of AnimationSystem
    * that is designed to use system time for animations.
    *
    * In a Ramses distributed system the client and the renderer have their own copies
    * of animation system. Real time animation system guarantees smooth animations without
    * stuttering by giving the control of time updates to the renderer.
    * The renderer updates the animation system using system time every frame.
    *
    * The client animation system does not have to worry about updates as long as it is not
    * making changes to the states and properties. Many animation related commands
    * rely on the current time of the local animation system, it uses it as time stamp
    * for some commands that are then distributed to the renderer.
    * Therefore it is important to properly update the time of the local animation system
    * before any change is made.
    */
    class RAMSES_API AnimationSystemRealTime : public AnimationSystem
    {
    public:

        /**
        * @brief Sets the local animation system to a given time.
        * The time used should always be system time, because the renderer uses
        * the system time implicitly. Slight offset can be used to delay or hide
        * latency of the distribution system.
        *
        * @param[in] systemTime The time stamp to be set in the local animation system.
        *                       If 0 then system time is used.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t updateLocalTime(globalTimeStamp_t systemTime = 0u);

    protected:
        /**
        * @brief SceneImpl is the factory for creating animation system instances.
        */
        friend class SceneImpl;

        /**
        * @brief Hidden method from AnimationSystem base class as time cannot be set to AnimationSystemRealTime
        *
        * @param[in] timeStamp Explicit time cannot be used with AnimationSystemRealTime
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setTime(globalTimeStamp_t timeStamp);

        /**
        * @brief Constructor of the animation system
        *
        * @param[in] pimpl Internal data for implementation specifics of AnimationSystem (sink - instance becomes owner)
        */
        explicit AnimationSystemRealTime(AnimationSystemImpl& pimpl);

        /**
        * @brief Copy constructor of animation system
        *
        * @param[in] other Other instance of animation system class
        */
        AnimationSystemRealTime(const AnimationSystemRealTime& other);

        /**
        * @brief Assignment operator of animation system.
        *
        * @param[in] other Other instance of animation system class
        * @return This instance after assignment
        */
        AnimationSystemRealTime& operator=(const AnimationSystemRealTime& other);

        /**
        * @brief Destructor of the animation system
        */
        virtual ~AnimationSystemRealTime();
    };
}

#endif
