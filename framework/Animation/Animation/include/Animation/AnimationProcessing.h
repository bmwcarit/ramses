//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONPROCESSING_H
#define RAMSES_ANIMATIONPROCESSING_H

#include "Animation/AnimationLogicListener.h"
#include "Animation/AnimationData.h"
#include "Animation/AnimationProcessingFinished.h"
#include "Animation/AnimationProcessDataCache.h"

namespace ramses_internal
{
    class AnimationProcessing : public AnimationLogicListener
    {
    public:
        explicit AnimationProcessing(AnimationData& animationData);

        // AnimationStateListener interface
        virtual void onAnimationStarted(AnimationHandle handle) override;
        virtual void onAnimationFinished(AnimationHandle handle) override;
        virtual void onAnimationPaused(AnimationHandle handle) override;
        virtual void onAnimationResumed(AnimationHandle handle) override;
        virtual void onAnimationPropertiesChanged(AnimationHandle handle) override;
        virtual void onTimeChanged(const AnimationTime& time) override;

        static SplineTimeStamp ComputeSplineTime(const Animation& animation, const AnimationTime& globalTime);

    private:
        void process(const AnimationTime& timeStamp);
        void processActiveAnimations();
        void processAnimation(AnimationProcessData& processData);
        void resetProcessDataIfCached(AnimationHandle handle);

        AnimationProcessDataCache m_processDataCache;
        AnimationTime m_timeStamp;

        AnimationProcessingFinished m_finishedAnimationProcessing;
    };
}

#endif
