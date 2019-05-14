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
        assert(!contains_c(m_startedAnimations, handle));
        AnimationHandleVector::iterator iter = find_c(m_finishedAnimations, handle);
        if (iter != m_finishedAnimations.end())
        {
            m_finishedAnimations.erase(iter);
        }

        m_startedAnimations.push_back(handle);
    }

    void AnimationStateChangeCollector::onAnimationFinished(AnimationHandle handle)
    {
        assert(!contains_c(m_finishedAnimations, handle));
        AnimationHandleVector::iterator iter = find_c(m_startedAnimations, handle);
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
