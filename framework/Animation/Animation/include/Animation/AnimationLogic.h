//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONLOGIC_H
#define RAMSES_ANIMATIONLOGIC_H

#include "Animation/AnimationDataListener.h"
#include "Animation/AnimationLogicNotifier.h"
#include "Animation/Animation.h"
#include "Collections/HashMap.h"
#include <deque>

namespace ramses_internal
{
    class AnimationData;

    class AnimationLogic : public AnimationDataListener, public AnimationLogicNotifier
    {
    public:
        explicit AnimationLogic(AnimationData& animationData);
        virtual ~AnimationLogic();

        // AnimationDataListener interface
        virtual void preAnimationTimeRangeChange(AnimationHandle handle) override;
        virtual void onAnimationTimeRangeChanged(AnimationHandle handle) override;
        virtual void onAnimationPauseChanged(AnimationHandle handle, bool pause) override;
        virtual void onAnimationPropertiesChanged(AnimationHandle handle) override;
        virtual void onAnimationInstanceChanged(AnimationInstanceHandle handle) override;
        virtual void onSplineChanged(SplineHandle handle) override;

        void enqueueAnimation(AnimationHandle handle);
        void dequeueAnimation(AnimationHandle handle);
        void setTime(const AnimationTime& time);
        const AnimationTime& getTime() const;

        bool isAnimationActive(AnimationHandle handle) const;
        bool isAnimationPending(AnimationHandle handle) const;
        UInt getNumActiveAnimations() const;
        UInt getNumPendingAnimations() const;

    private:
        // Subset of Animation data used by AnimationLogic
        struct AnimationProperties
        {
            AnimationTime m_startTime;
            AnimationTime m_stopTime;
            Animation::Flags m_flags;
            AnimationInstanceHandle m_animationInstanceHandle;
            SplineHandle m_splineHandle;
        };

        void update();
        void updateActiveAnimations();
        void updatePendingAnimations();
        void processFinishedAnimation(AnimationHandle handle);

        void addAnimationProperties(AnimationHandle handle);
        const AnimationProperties& getAnimationProperties(AnimationHandle handle) const;

        void insertToListOrderedByStartTime(std::deque<AnimationHandle>& list, AnimationHandle handle);
        void insertToListOrderedByStopTime(std::deque<AnimationHandle>& list, AnimationHandle handle);
        void addAnimationToQueue(AnimationHandle handle);
        void removeAnimationFromQueue(AnimationHandle handle);

        static bool isAnimationTimeRangeValid(const AnimationProperties& animProps);

        typedef HashMap<AnimationHandle, AnimationProperties> AnimationPropertiesMap;

        AnimationData& m_animationData;

        std::deque<AnimationHandle> m_pendingAnimations;
        std::deque<AnimationHandle> m_activeAnimations;
        AnimationPropertiesMap m_animationProperties;

        AnimationTime m_time;
    };
}

#endif
