//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONSTATECHANGECOLLECTOR_H
#define RAMSES_ANIMATIONSTATECHANGECOLLECTOR_H

#include "Animation/AnimationLogicListener.h"
#include "Animation/AnimationCollectionTypes.h"
#include "Collections/Vector.h"

namespace ramses_internal
{
    /// AnimationStateChangeCollector listens and collects animation state changes.
    class AnimationStateChangeCollector : public AnimationLogicListener
    {
    public:
        // AnimationStateListener interface
        virtual void onAnimationStarted(AnimationHandle handle) override;
        virtual void onAnimationFinished(AnimationHandle handle) override;

        const AnimationHandleVector& getCollectedStartedAnimations() const;
        const AnimationHandleVector& getCollectedFinishedAnimations() const;

        void resetCollections();

    private:
        AnimationHandleVector m_startedAnimations;
        AnimationHandleVector m_finishedAnimations;
    };
}

#endif
