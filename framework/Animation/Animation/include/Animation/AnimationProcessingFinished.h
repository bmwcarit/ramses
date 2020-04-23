//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONPROCESSINGFINISHED_H
#define RAMSES_ANIMATIONPROCESSINGFINISHED_H

#include "Animation/AnimationLogicListener.h"
#include "Animation/AnimationData.h"

namespace ramses_internal
{
    /// AnimationProcessingFinished is responsible for single value change at the point when animation stopped/paused.
    class AnimationProcessingFinished : public AnimationLogicListener
    {
    public:
        explicit AnimationProcessingFinished(AnimationData& animationData);

        // AnimationStateListener interface
        virtual void onAnimationStarted(AnimationHandle handle) override;
        virtual void onAnimationFinished(AnimationHandle handle) override;
        virtual void onAnimationPaused(AnimationHandle handle) override;

    private:
        void processAnimation(AnimationProcessData& processData) const;
        void setDataBindingInitialValue(const Animation& animation);
        SplineTimeStamp computeSplineTime(const Animation& animation) const;

        AnimationData& m_animationData;
    };
}

#endif
