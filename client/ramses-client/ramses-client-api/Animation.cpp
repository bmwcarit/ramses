//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/Animation.h"

// internal
#include "AnimationImpl.h"

namespace ramses
{
    Animation::Animation(AnimationImpl& pimpl)
        : AnimationObject(pimpl)
        , impl(pimpl)
    {
    }

    Animation::~Animation()
    {
    }

    globalTimeStamp_t Animation::getStartTime() const
    {
        return impl.getStartTime();
    }

    globalTimeStamp_t Animation::getStopTime() const
    {
        return impl.getStopTime();
    }
}
