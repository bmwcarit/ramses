//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONDATANOTIFIER_H
#define RAMSES_ANIMATIONDATANOTIFIER_H

#include "Collections/Vector.h"
#include "Animation/AnimationDataListener.h"

namespace ramses_internal
{
    class AnimationDataNotifier
    {
    public:
        virtual ~AnimationDataNotifier() {}
        virtual void addListener(AnimationDataListener* pListener);
        virtual void removeListener(AnimationDataListener* pListener);

    protected:
        void notifyPreAnimationTimeRangeChange(AnimationHandle handle);
        void notifyAnimationTimeRangeChanged(AnimationHandle handle);
        void notifyAnimationPauseChanged(AnimationHandle handle, Bool pause);
        void notifyAnimationPropertiesChanged(AnimationHandle handle);
        void notifyAnimationInstanceChanged(AnimationInstanceHandle handle);
        void notifySplineChanged(SplineHandle handle);

        typedef std::vector<AnimationDataListener*> AnimationListenerVector;

        AnimationListenerVector m_observers;
    };
}

#endif
