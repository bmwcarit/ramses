//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/AnimationSystemRealTime.h"

// internal
#include "AnimationSystemImpl.h"

namespace ramses
{
    AnimationSystemRealTime::AnimationSystemRealTime(AnimationSystemImpl& pimpl)
        : AnimationSystem(pimpl)
    {
    }

    AnimationSystemRealTime::~AnimationSystemRealTime()
    {
    }

    status_t AnimationSystemRealTime::updateLocalTime(globalTimeStamp_t systemTime)
    {
        const status_t status = impl.updateLocalTime(systemTime);
        LOG_HL_CLIENT_API1(status, systemTime)
        return status;
    }
}
