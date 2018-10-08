//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONLOGICLISTENER_H
#define RAMSES_ANIMATIONLOGICLISTENER_H

#include "Animation/AnimationCommon.h"
#include "Animation/AnimationTime.h"

namespace ramses_internal
{
    class AnimationLogicListener
    {
    public:
        virtual ~AnimationLogicListener()  {}

        virtual void onAnimationStarted(AnimationHandle handle);
        // If EAnimation_RemoveWhenFinished is set, the data is removed after this event is sent
        virtual void onAnimationFinished(AnimationHandle handle);
        // Event is sent only if animation is currently active
        virtual void onAnimationPaused(AnimationHandle handle);
        // Event is sent only if animation is currently active
        virtual void onAnimationResumed(AnimationHandle handle);
        // Flags, playback speed, loop duration changes
        virtual void onAnimationPropertiesChanged(AnimationHandle handle);
        virtual void onAnimationSplineDataChanged(AnimationHandle handle);
        virtual void onTimeChanged(const AnimationTime& time);
    };

    inline void AnimationLogicListener::onAnimationStarted(AnimationHandle handle)
    {
        UNUSED(handle);
    }
    // If EAnimation_RemoveWhenFinished is set, the data is removed after this event is sent
    inline void AnimationLogicListener::onAnimationFinished(AnimationHandle handle)
    {
        UNUSED(handle);
    }
    // Event is sent only if animation is currently active
    inline void AnimationLogicListener::onAnimationPaused(AnimationHandle handle)
    {
        UNUSED(handle);
    }
    // Event is sent only if animation is currently active
    inline void AnimationLogicListener::onAnimationResumed(AnimationHandle handle)
    {
        UNUSED(handle);
    }
    // Flags, playback speed, loop duration changes
    inline void AnimationLogicListener::onAnimationPropertiesChanged(AnimationHandle handle)
    {
        UNUSED(handle);
    }
    inline void AnimationLogicListener::onAnimationSplineDataChanged(AnimationHandle handle)
    {
        UNUSED(handle);
    }
    inline void AnimationLogicListener::onTimeChanged(const AnimationTime& time)
    {
        UNUSED(time);
    }
}

#endif
