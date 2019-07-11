//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATEDPROPERTY_H
#define RAMSES_ANIMATEDPROPERTY_H

#include "ramses-client-api/AnimationObject.h"

namespace ramses
{
    /**
    * @brief The AnimatedProperty holds a reference to data that can be animated.
    */
    class RAMSES_API AnimatedProperty : public AnimationObject
    {
    public:
        /**
        * Stores internal data for implementation specifics of AnimatedProperty.
        */
        class AnimatedPropertyImpl& impl;

        /**
         * @brief Deleted copy constructor
         * @param other unused
         */
        AnimatedProperty(const AnimatedProperty& other) = delete;

        /**
         * @brief Deleted copy assignment
         * @param other unused
         * @return unused
         */
        AnimatedProperty& operator=(const AnimatedProperty& other) = delete;

    protected:
        /**
        * @brief AnimatedPropertyFactory is the factory for creating animated properties.
        */
        friend class AnimatedPropertyFactory;
        /**
        * @brief AnimationSystemData is the factory for creating animated properties.
        */
        friend class AnimationSystemData;

        /**
        * @brief Constructor of the animated property
        *
        * @param[in] pimpl Internal data for implementation specifics of AnimatedProperty (sink - instance becomes owner)
        */
        explicit AnimatedProperty(AnimatedPropertyImpl& pimpl);

        /**
        * @brief Destructor of the animated property
        */
        virtual ~AnimatedProperty() override;
    };

    /// Vector component ID for binding single/multi component data
    enum EAnimatedPropertyComponent
    {
        EAnimatedPropertyComponent_X = 0,
        EAnimatedPropertyComponent_Y,
        EAnimatedPropertyComponent_Z,
        EAnimatedPropertyComponent_W,
        EAnimatedPropertyComponent_All
    };

    /// Property to animate for objects that have more than one property that can be animated
    enum EAnimatedProperty
    {
        EAnimatedProperty_Translation = 0,
        EAnimatedProperty_Rotation,
        EAnimatedProperty_Scaling
    };
}

#endif
