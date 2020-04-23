//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATION_ANIMATION_H
#define RAMSES_ANIMATION_ANIMATION_H

#include "Animation/AnimationCommon.h"
#include "Animation/AnimationTime.h"
#include "Common/BitForgeMacro.h"

namespace ramses_internal
{
    class Animation
    {
    public:
        typedef UInt32 Flags;
        enum EAnimationFlags
        {
            EAnimationFlags_Looping            = BIT(0), ///< Enable looping, must set also m_loopDuration to non-zero value
            EAnimationFlags_Relative           = BIT(1), ///< Add values from spline keys to initial value of animated property
            EAnimationFlags_ApplyInitialValue  = BIT(2), ///< Set animated data to initial value instead of spline keys data (used to rollback animation)
            EAnimationFlags_Reverse            = BIT(3)
        };

        explicit Animation(AnimationInstanceHandle animInstHandle = AnimationInstanceHandle::Invalid(), Flags flags = 0u);
        bool isPlaying(const AnimationTime& time) const;

        AnimationInstanceHandle  m_animationInstanceHandle;
        Flags                    m_flags;
        Float                    m_playbackSpeed;
        AnimationTime::Duration  m_loopDuration;
        AnimationTime            m_startTime;
        AnimationTime            m_stopTime;
        bool                     m_paused;
    };

    inline bool Animation::isPlaying(const AnimationTime& time) const
    {
        return time >= m_startTime
            && time < m_stopTime
            && !m_paused;
    }
}

#endif
