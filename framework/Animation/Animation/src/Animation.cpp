//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Animation/Animation.h"

namespace ramses_internal
{
    Animation::Animation(AnimationInstanceHandle animInstHandle, Flags flags)
        : m_animationInstanceHandle(animInstHandle)
        , m_flags(flags)
        , m_playbackSpeed(1.f)
        , m_loopDuration(0u)
        , m_paused(false)
    {
    }
}
