//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Animation/AnimationProcessingFinished.h"
#include "Animation/SplineIterator.h"
#include "Animation/AnimationProcessData.h"

namespace ramses_internal
{
    AnimationProcessingFinished::AnimationProcessingFinished(AnimationData& animationData)
        : m_animationData(animationData)
    {
    }

    void AnimationProcessingFinished::onAnimationStarted(AnimationHandle handle)
    {
        assert(m_animationData.containsAnimation(handle));
        const Animation& animation = m_animationData.getAnimation(handle);
        if (!animation.m_paused)
        {
            setDataBindingInitialValue(animation);
        }
    }

    void AnimationProcessingFinished::onAnimationFinished(AnimationHandle handle)
    {
        AnimationProcessData processData;
        m_animationData.getAnimationProcessData(handle, processData);
        if (!processData.m_animation.m_paused)
        {
            processAnimation(processData);
        }
    }

    void AnimationProcessingFinished::onAnimationPaused(AnimationHandle handle)
    {
        AnimationProcessData processData;
        m_animationData.getAnimationProcessData(handle, processData);
        processAnimation(processData);
    }

    void AnimationProcessingFinished::processAnimation(AnimationProcessData& processData) const
    {
        const SplineBase* const pSpline = processData.m_spline;
        const SplineTimeStamp splineTime = computeSplineTime(processData.m_animation);
        const Bool playReverse = (processData.m_animation.m_flags & Animation::EAnimationFlags_Reverse) != 0;

        processData.m_splineIterator.setTimeStamp(splineTime, pSpline, playReverse);

        AnimationProcessDataDispatch dataDispatch(processData);
        dataDispatch.dispatch();
    }

    void AnimationProcessingFinished::setDataBindingInitialValue(const Animation& animation)
    {
        const AnimationInstance& animInst = m_animationData.getAnimationInstance(animation.m_animationInstanceHandle);
        const DataBindHandleVector& dataBindHandles = animInst.getDataBindings();
        const UInt numDataBinds = dataBindHandles.size();
        for (UInt i = 0u; i < numDataBinds; ++i)
        {
            AnimationDataBindBase* const pDataBind = m_animationData.getDataBinding(dataBindHandles[i]);
            assert(pDataBind != 0);
            pDataBind->setInitialValue();
        }
    }

    SplineTimeStamp AnimationProcessingFinished::computeSplineTime(const Animation& animation) const
    {
        AnimationTime::Duration animDuration = animation.m_stopTime.getDurationSince(animation.m_startTime);
        if (animation.m_flags & Animation::EAnimationFlags_Looping)
        {
            animDuration = animDuration % animation.m_loopDuration;
        }
        const Float splineTime = static_cast<Float>(animDuration);
        return static_cast<SplineTimeStamp>(splineTime * animation.m_playbackSpeed);
    }
}
