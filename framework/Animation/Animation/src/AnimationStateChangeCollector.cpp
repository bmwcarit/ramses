//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Animation/AnimationStateChangeCollector.h"

namespace ramses_internal
{
    void AnimationStateChangeCollector::onAnimationStarted(AnimationHandle handle)
    {
        assert(!m_startedAnimations.contains(handle));
        AnimationHandleVector::Iterator iter = m_finishedAnimations.find(handle);
        if (iter != m_finishedAnimations.end())
        {
            m_finishedAnimations.erase(iter);
        }

        m_startedAnimations.push_back(handle);
    }

    void AnimationStateChangeCollector::onAnimationFinished(AnimationHandle handle)
    {
        assert(!m_finishedAnimations.contains(handle));
        AnimationHandleVector::Iterator iter = m_startedAnimations.find(handle);
        if (iter != m_startedAnimations.end())
        {
            m_startedAnimations.erase(iter);
        }

        m_finishedAnimations.push_back(handle);
    }

    const AnimationHandleVector& AnimationStateChangeCollector::getCollectedStartedAnimations() const
    {
        return m_startedAnimations;
    }

    const AnimationHandleVector& AnimationStateChangeCollector::getCollectedFinishedAnimations() const
    {
        return m_finishedAnimations;
    }

    void AnimationStateChangeCollector::resetCollections()
    {
        m_startedAnimations.clear();
        m_finishedAnimations.clear();
    }
}
