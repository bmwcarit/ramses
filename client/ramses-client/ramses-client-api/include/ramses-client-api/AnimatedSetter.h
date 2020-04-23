//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATEDSETTER_H
#define RAMSES_ANIMATEDSETTER_H

#include "ramses-client-api/AnimationObject.h"
#include "ramses-client-api/AnimationTypes.h"

namespace ramses
{
    /**
    * @brief The AnimatedSetter is used to set values on AnimatedProperty which will result
    *        in smooth animation towards the new value. In order to achieve such effect,
    *        the new value will be reached with a predefined delay.
    */
    class RAMSES_API AnimatedSetter : public AnimationObject
    {
    public:

        /**
        * @brief Sets new value of animated property.
        * This value will be applied with a fixed delay (defined by AnimationSystem)
        * and it will start/continue animating the property to the new value with linear interpolation.
        *
        * @param[in] x The new value.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setValue(bool x);

        /**
        * @brief Sets new value of animated property.
        * This value will be applied with a fixed delay (defined by AnimationSystem)
        * and it will start/continue animating the property to the new value with linear interpolation.
        *
        * @param[in] x The new value.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setValue(int32_t x);

        /**
        * @brief Sets new value of animated property.
        * This value will be applied with a fixed delay (defined by AnimationSystem)
        * and it will start/continue animating the property to the new value with linear interpolation.
        *
        * @param[in] x The new value.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setValue(float x);

        /**
        * @brief Sets new value of animated property.
        * This value will be applied with a fixed delay (defined by AnimationSystem)
        * and it will start/continue animating the property to the new value with linear interpolation.
        *
        * @param[in] x The first value for the new value.
        * @param[in] y The second value for the new value.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setValue(float x, float y);

        /**
        * @brief Sets new value of animated property.
        * This value will be applied with a fixed delay (defined by AnimationSystem)
        * and it will start/continue animating the property to the new value with linear interpolation.
        *
        * @param[in] x The first component for the new value.
        * @param[in] y The second component for the new value.
        * @param[in] z The third component for the new value.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setValue(float x, float y, float z);

        /**
        * @brief Sets new value of animated property.
        * This value will be applied with a fixed delay (defined by AnimationSystem)
        * and it will start/continue animating the property to the new value with linear interpolation.
        *
        * @param[in] x The first component for the new value.
        * @param[in] y The second component for the new value.
        * @param[in] z The third component for the new value.
        * @param[in] w The fourth component for the new value.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setValue(float x, float y, float z, float w);

        /**
        * @brief Sets new value of animated property.
        * This value will be applied with a fixed delay (defined by AnimationSystem)
        * and it will start/continue animating the property to the new value with linear interpolation.
        *
        * @param[in] x The first value for the new value.
        * @param[in] y The second value for the new value.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setValue(int32_t x, int32_t y);

        /**
        * @brief Sets new value of animated property.
        * This value will be applied with a fixed delay (defined by AnimationSystem)
        * and it will start/continue animating the property to the new value with linear interpolation.
        *
        * @param[in] x The first component for the new value.
        * @param[in] y The second component for the new value.
        * @param[in] z The third component for the new value.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setValue(int32_t x, int32_t y, int32_t z);

        /**
        * @brief Sets new value of animated property.
        * This value will be applied with a fixed delay (defined by AnimationSystem)
        * and it will start/continue animating the property to the new value with linear interpolation.
        *
        * @param[in] x The first component for the new value.
        * @param[in] y The second component for the new value.
        * @param[in] z The third component for the new value.
        * @param[in] w The fourth component for the new value.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setValue(int32_t x, int32_t y, int32_t z, int32_t w);

        /**
        * @brief Stops the animation.
        * Delay can be used to postpone stop of animation.
        *
        * @param delay Delay stop in milliseconds.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t stop(timeMilliseconds_t delay = 0);

        /**
        * @brief Stores internal data for implementation specifics of AnimatedSetter.
        */
        class AnimatedSetterImpl& impl;

    protected:
        /**
        * @brief AnimationSystemData is the factory for creating AnimatedSetter.
        */
        friend class AnimationSystemData;

        /**
        * @brief Constructor of the AnimatedSetter
        *
        * @param[in] pimpl Internal data for implementation specifics of AnimatedSetter (sink - instance becomes owner)
        */
        explicit AnimatedSetter(AnimatedSetterImpl& pimpl);

        /**
        * @brief Copy constructor of AnimatedSetter
        *
        * @param[in] other Other instance of AnimatedSetter class
        */
        AnimatedSetter(const AnimatedSetter& other);

        /**
        * @brief Assignment operator of AnimatedSetter.
        *
        * @param[in] other Other instance of AnimatedSetter class
        * @return This instance after assignment
        */
        AnimatedSetter& operator=(const AnimatedSetter& other);

        /**
        * @brief Destructor of the AnimatedSetter
        */
        virtual ~AnimatedSetter();
    };
}

#endif
