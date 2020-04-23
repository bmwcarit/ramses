//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONDATALISTENER_H
#define RAMSES_ANIMATIONDATALISTENER_H

#include "Animation/AnimationCommon.h"

namespace ramses_internal
{
    class AnimationDataListener
    {
    public:
        virtual ~AnimationDataListener()  {}

        virtual void preAnimationTimeRangeChange(AnimationHandle handle);
        virtual void onAnimationTimeRangeChanged(AnimationHandle handle);
        virtual void onAnimationPauseChanged(AnimationHandle handle, bool pause);
        virtual void onAnimationPropertiesChanged(AnimationHandle handle);
        virtual void onAnimationInstanceChanged(AnimationInstanceHandle handle);
        virtual void onSplineChanged(SplineHandle handle);
    };

    inline void AnimationDataListener::preAnimationTimeRangeChange(AnimationHandle handle)
    {
        UNUSED(handle);
    }
    inline void AnimationDataListener::onAnimationTimeRangeChanged(AnimationHandle handle)
    {
        UNUSED(handle);
    }
    inline void AnimationDataListener::onAnimationPauseChanged(AnimationHandle handle, bool pause)
    {
        UNUSED(handle);
        UNUSED(pause);
    }
    inline void AnimationDataListener::onAnimationPropertiesChanged(AnimationHandle handle)
    {
        UNUSED(handle);
    }
    inline void AnimationDataListener::onAnimationInstanceChanged(AnimationInstanceHandle handle)
    {
        UNUSED(handle);
    }
    inline void AnimationDataListener::onSplineChanged(SplineHandle handle)
    {
        UNUSED(handle);
    }
}

#endif
