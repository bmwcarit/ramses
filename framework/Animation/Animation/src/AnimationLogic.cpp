//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Animation/AnimationLogic.h"
#include "Animation/Animation.h"
#include "Animation/AnimationData.h"
#include <algorithm>

namespace ramses_internal
{
    AnimationLogic::AnimationLogic(AnimationData& animationData)
        : m_animationData(animationData)
        , m_time(0u)
    {
        AnimationHandleVector animHandles;
        m_animationData.getAnimationHandles(animHandles);
        for (AnimationHandleVector::iterator it = animHandles.begin();
            it != animHandles.end(); ++it)
        {
            enqueueAnimation(*it);
        }

        m_animationData.addListener(this);
    }

    AnimationLogic::~AnimationLogic()
    {
        m_animationData.removeListener(this);
    }

    void AnimationLogic::preAnimationTimeRangeChange(AnimationHandle handle)
    {
        if (isAnimationActive(handle))
        {
            Animation& animation = m_animationData.getAnimationInternal(handle);
            animation.m_stopTime = m_time;
            dequeueAnimation(handle);
        }
    }

    void AnimationLogic::onAnimationTimeRangeChanged(AnimationHandle handle)
    {
        enqueueAnimation(handle);
    }

    void AnimationLogic::onAnimationPauseChanged(AnimationHandle handle, Bool pause)
    {
        if (isAnimationActive(handle))
        {
            if (pause)
            {
                notifyAnimationPaused(handle);
            }
            else
            {
                notifyAnimationResumed(handle);
            }
        }
    }

    void AnimationLogic::onAnimationPropertiesChanged(AnimationHandle handle)
    {
        notifyAnimationPropertiesChanged(handle);
        enqueueAnimation(handle);
    }

    void AnimationLogic::onAnimationInstanceChanged(AnimationInstanceHandle handle)
    {
        for (AnimationPropertiesMap::Iterator it = m_animationProperties.begin(); it != m_animationProperties.end(); ++it)
        {
            if (it->value.m_animationInstanceHandle == handle)
            {
                notifyAnimationPropertiesChanged(it->key);
            }
        }
    }

    void AnimationLogic::onSplineChanged(SplineHandle handle)
    {
        for (AnimationPropertiesMap::Iterator it = m_animationProperties.begin(); it != m_animationProperties.end(); ++it)
        {
            if (it->value.m_splineHandle == handle)
            {
                notifyAnimationPropertiesChanged(it->key);
            }
        }
    }

    void AnimationLogic::enqueueAnimation(AnimationHandle handle)
    {
        if (!m_animationData.containsAnimation(handle))
        {
            return;
        }

        if (isAnimationActive(handle) || isAnimationPending(handle))
        {
            // Update cache
            addAnimationProperties(handle);
        }
        else
        {
            addAnimationToQueue(handle);
        }
    }

    void AnimationLogic::dequeueAnimation(AnimationHandle handle)
    {
        removeAnimationFromQueue(handle);
    }

    void AnimationLogic::setTime(const AnimationTime& time)
    {
        if (time > m_time)
        {
            m_time = time;
            update();
            notifyTimeChanged(m_time);
        }
    }

    const AnimationTime& AnimationLogic::getTime() const
    {
        return m_time;
    }

    Bool AnimationLogic::isAnimationActive(AnimationHandle handle) const
    {
        return std::find(m_activeAnimations.begin(), m_activeAnimations.end(), handle) != m_activeAnimations.end();
    }

    Bool AnimationLogic::isAnimationPending(AnimationHandle handle) const
    {
        return std::find(m_pendingAnimations.begin(), m_pendingAnimations.end(), handle) != m_pendingAnimations.end();
    }

    UInt AnimationLogic::getNumActiveAnimations() const
    {
        return m_activeAnimations.size();
    }

    UInt AnimationLogic::getNumPendingAnimations() const
    {
        return m_pendingAnimations.size();
    }

    void AnimationLogic::update()
    {
        updateActiveAnimations();
        updatePendingAnimations();
    }

    void AnimationLogic::updateActiveAnimations()
    {
        bool bTilt = false;
        while (!bTilt && m_activeAnimations.size() > 0)
        {
            const AnimationHandle handle = *m_activeAnimations.begin();
            const AnimationProperties& animProps = getAnimationProperties(handle);
            const AnimationTime& stopTime = animProps.m_stopTime;
            if (m_time >= stopTime)
            {
                processFinishedAnimation(handle);
                m_activeAnimations.pop_front();
                m_animationProperties.remove(handle);
            }
            else
            {
                bTilt = true;
            }
        }
    }

    void AnimationLogic::updatePendingAnimations()
    {
        bool bTilt = false;
        while (!bTilt && m_pendingAnimations.size() > 0)
        {
            const AnimationHandle handle = *m_pendingAnimations.begin();
            const AnimationProperties& animProps = getAnimationProperties(handle);
            if (m_time >= animProps.m_startTime)
            {
                m_pendingAnimations.pop_front();
                if (m_time < animProps.m_stopTime)
                {
                    insertToListOrderedByStopTime(m_activeAnimations, handle);
                    notifyAnimationStarted(handle);
                }
            }
            else
            {
                bTilt = true;
            }
        }
    }

    void AnimationLogic::processFinishedAnimation(AnimationHandle handle)
    {
        notifyAnimationFinished(handle);
    }

    void AnimationLogic::addAnimationProperties(AnimationHandle handle)
    {
        assert(m_animationData.containsAnimation(handle));
        const Animation& animation = m_animationData.getAnimation(handle);
        const AnimationInstance& animInst = m_animationData.getAnimationInstance(animation.m_animationInstanceHandle);

        AnimationProperties animProps;
        animProps.m_startTime = animation.m_startTime;
        animProps.m_stopTime = animation.m_stopTime;
        animProps.m_flags = animation.m_flags;
        animProps.m_animationInstanceHandle = animation.m_animationInstanceHandle;
        animProps.m_splineHandle = animInst.getSplineHandle();
        m_animationProperties.put(handle, animProps);
    }

    const AnimationLogic::AnimationProperties& AnimationLogic::getAnimationProperties(AnimationHandle handle) const
    {
        AnimationPropertiesMap::ConstIterator it = m_animationProperties.find(handle);
        assert(it != m_animationProperties.end());
        return it->value;
    }

    void AnimationLogic::insertToListOrderedByStartTime(std::deque<AnimationHandle>& list, AnimationHandle handle)
    {
        list.insert(std::upper_bound(list.begin(), list.end(), handle,
                                     [&](AnimationHandle a, AnimationHandle b) -> bool {
                                         assert(m_animationData.containsAnimation(a));
                                         assert(m_animationData.containsAnimation(b));
                                         const Animation& animation1 = m_animationData.getAnimation(a);
                                         const Animation& animation2 = m_animationData.getAnimation(b);
                                         return animation1.m_startTime < animation2.m_startTime;
                                     }),
                    handle);
    }

    void AnimationLogic::insertToListOrderedByStopTime(std::deque<AnimationHandle>& list, AnimationHandle handle)
    {
        list.insert(std::upper_bound(list.begin(), list.end(), handle,
                                     [&](AnimationHandle a, AnimationHandle b) -> bool {
                                         assert(m_animationData.containsAnimation(a));
                                         assert(m_animationData.containsAnimation(b));
                                         const Animation& animation1 = m_animationData.getAnimation(a);
                                         const Animation& animation2 = m_animationData.getAnimation(b);
                                         return animation1.m_stopTime < animation2.m_stopTime;
                                     }),
                    handle);
    }

    void AnimationLogic::addAnimationToQueue(AnimationHandle handle)
    {
        addAnimationProperties(handle);
        const AnimationProperties& animProps = getAnimationProperties(handle);
        if (!isAnimationTimeRangeValid(animProps) || m_time >= animProps.m_stopTime)
        {
            m_animationProperties.remove(handle);
        }
        else if (m_time >= animProps.m_startTime)
        {
            insertToListOrderedByStopTime(m_activeAnimations, handle);
            notifyAnimationStarted(handle);
        }
        else
        {
            insertToListOrderedByStartTime(m_pendingAnimations, handle);
        }
    }

    void AnimationLogic::removeAnimationFromQueue(AnimationHandle handle)
    {
        if (isAnimationActive(handle))
        {
            m_activeAnimations.erase(std::find(m_activeAnimations.begin(), m_activeAnimations.end(), handle));
            processFinishedAnimation(handle);
        }
        else
        {
            m_pendingAnimations.erase(std::find(m_pendingAnimations.begin(), m_pendingAnimations.end(), handle));
        }
        m_animationProperties.remove(handle);
    }

    Bool AnimationLogic::isAnimationTimeRangeValid(const AnimationProperties& animProps)
    {
        return animProps.m_startTime.isValid()
            && animProps.m_stopTime.isValid()
            && animProps.m_startTime < animProps.m_stopTime;
    }
}
