//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Animation/AnimationProcessing.h"
#include "Animation/SplineIterator.h"

namespace ramses_internal
{
    AnimationProcessing::AnimationProcessing(AnimationData& animationData)
        : m_processDataCache(animationData)
        , m_timeStamp(0u)
        , m_finishedAnimationProcessing(animationData)
    {
    }

    void AnimationProcessing::onAnimationStarted(AnimationHandle handle)
    {
        m_processDataCache.addProcessData(handle);
        m_finishedAnimationProcessing.onAnimationStarted(handle);
    }

    void AnimationProcessing::onAnimationFinished(AnimationHandle handle)
    {
        m_finishedAnimationProcessing.onAnimationFinished(handle);
        m_processDataCache.removeProcessData(handle);
    }

    void AnimationProcessing::onAnimationPaused(AnimationHandle handle)
    {
        resetProcessDataIfCached(handle);
        m_finishedAnimationProcessing.onAnimationPaused(handle);
    }

    void AnimationProcessing::onAnimationResumed(AnimationHandle handle)
    {
        resetProcessDataIfCached(handle);
        m_finishedAnimationProcessing.onAnimationResumed(handle);
    }

    void AnimationProcessing::onAnimationPropertiesChanged(AnimationHandle handle)
    {
        resetProcessDataIfCached(handle);
        m_finishedAnimationProcessing.onAnimationPropertiesChanged(handle);
    }

    void AnimationProcessing::onTimeChanged(const AnimationTime& time)
    {
        process(time);
    }

    void AnimationProcessing::process(const AnimationTime& timeStamp)
    {
        if (timeStamp != m_timeStamp)
        {
            m_timeStamp = timeStamp;
            processActiveAnimations();
        }
    }

    void AnimationProcessing::processActiveAnimations()
    {
        for (AnimationProcessDataCache::DataProcessMap::Iterator it = m_processDataCache.begin();
            it != m_processDataCache.end(); ++it)
        {
            AnimationProcessData& processData = it->value;
            if (processData.m_animation.isPlaying(m_timeStamp))
            {
                processAnimation(processData);
            }
        }
    }

    void AnimationProcessing::processAnimation(AnimationProcessData& processData)
    {
        const SplineBase* const pSpline = processData.m_spline;
        const SplineTimeStamp splineTime = ComputeSplineTime(processData.m_animation, m_timeStamp);
        const Bool playReverse = (processData.m_animation.m_flags & Animation::EAnimationFlags_Reverse) != 0;

        processData.m_splineIterator.setTimeStamp(splineTime, pSpline, playReverse);

        AnimationProcessDataDispatch dataDispatch(processData);
        dataDispatch.dispatch();
    }

    void AnimationProcessing::resetProcessDataIfCached(AnimationHandle handle)
    {
        if (m_processDataCache.hasProcessData(handle))
        {
            m_processDataCache.removeProcessData(handle);
            m_processDataCache.addProcessData(handle);
        }
    }

    SplineTimeStamp AnimationProcessing::ComputeSplineTime(const Animation& animation, const AnimationTime& globalTime)
    {
        const AnimationTime::Duration elapsedFromStartToNow = globalTime.getDurationSince(animation.m_startTime);
        const AnimationTime::Duration animDuration = animation.m_stopTime.getDurationSince(animation.m_startTime);
        AnimationTime::Duration elapsedTime = min(elapsedFromStartToNow, animDuration);
        elapsedTime = static_cast<AnimationTime::Duration>(elapsedTime * animation.m_playbackSpeed);
        if (animation.m_flags & Animation::EAnimationFlags_Looping)
        {
            elapsedTime = elapsedTime % animation.m_loopDuration;
        }

        return static_cast<SplineTimeStamp>(elapsedTime);
    }
}
