//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Animation/AnimationLogicNotifier.h"
#include "Animation/AnimationLogicListener.h"

namespace ramses_internal
{
    void AnimationLogicNotifier::addListener(AnimationLogicListener* pListener)
    {
        assert((pListener != NULL) && !contains_c(m_observers, pListener));
        m_observers.push_back(pListener);
    }

    void AnimationLogicNotifier::removeListener(AnimationLogicListener* pListener)
    {
        const AnimationListenerVector::iterator it = find_c(m_observers, pListener);
        assert(it != m_observers.end());
        m_observers.erase(it);
    }

    void AnimationLogicNotifier::notifyAnimationStarted(AnimationHandle handle)
    {
        for (auto observer : m_observers)
        {
            observer->onAnimationStarted(handle);
        }
    }

    void AnimationLogicNotifier::notifyAnimationFinished(AnimationHandle handle)
    {
        for (auto observer : m_observers)
        {
            observer->onAnimationFinished(handle);
        }
    }

    void AnimationLogicNotifier::notifyAnimationPaused(AnimationHandle handle)
    {
        for (auto observer : m_observers)
        {
            observer->onAnimationPaused(handle);
        }
    }

    void AnimationLogicNotifier::notifyAnimationResumed(AnimationHandle handle)
    {
        for (auto observer : m_observers)
        {
            observer->onAnimationResumed(handle);
        }
    }

    void AnimationLogicNotifier::notifyAnimationPropertiesChanged(AnimationHandle handle)
    {
        for (auto observer : m_observers)
        {
            observer->onAnimationPropertiesChanged(handle);
        }
    }

    void AnimationLogicNotifier::notifyTimeChanged(const AnimationTime& time)
    {
        for (auto observer : m_observers)
        {
            observer->onTimeChanged(time);
        }
    }
}
