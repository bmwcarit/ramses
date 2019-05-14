//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Animation/AnimationDataNotifier.h"

namespace ramses_internal
{
    void AnimationDataNotifier::addListener(AnimationDataListener* pListener)
    {
        if (contains_c(m_observers, pListener))
        {
            return;
        }

        if (pListener != 0)
        {
            m_observers.push_back(pListener);
        }
    }

    void AnimationDataNotifier::removeListener(AnimationDataListener* pListener)
    {
        auto it = find_c(m_observers, pListener);
        if (it != m_observers.end())
        {
            m_observers.erase(it);
        }
    }

    void AnimationDataNotifier::notifyPreAnimationTimeRangeChange(AnimationHandle handle)
    {
        for (auto obs : m_observers)
        {
            obs->preAnimationTimeRangeChange(handle);
        }
    }

    void AnimationDataNotifier::notifyAnimationTimeRangeChanged(AnimationHandle handle)
    {
        for (auto obs : m_observers)
        {
            obs->onAnimationTimeRangeChanged(handle);
        }
    }

    void AnimationDataNotifier::notifyAnimationPauseChanged(AnimationHandle handle, Bool pause)
    {
        for (auto obs : m_observers)
        {
            obs->onAnimationPauseChanged(handle, pause);
        }
    }

    void AnimationDataNotifier::notifyAnimationPropertiesChanged(AnimationHandle handle)
    {
        for (auto obs : m_observers)
        {
            obs->onAnimationPropertiesChanged(handle);
        }
    }

    void AnimationDataNotifier::notifyAnimationInstanceChanged(AnimationInstanceHandle handle)
    {
        for (auto obs : m_observers)
        {
            obs->onAnimationInstanceChanged(handle);
        }
    }

    void AnimationDataNotifier::notifySplineChanged(SplineHandle handle)
    {
        for (auto obs : m_observers)
        {
            obs->onSplineChanged(handle);
        }
    }
}
