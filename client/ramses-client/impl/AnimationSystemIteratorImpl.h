//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONSYSTEMITERATORIMPL_H
#define RAMSES_ANIMATIONSYSTEMITERATORIMPL_H

#include "ramses-client-api/RamsesObjectTypes.h"
#include "ObjectIteratorImpl.h"
#include "AnimationSystemImpl.h"

namespace ramses
{
    class RamsesObject;

    class AnimationSystemIteratorImpl : public ObjectIteratorImpl
    {
    public:
        AnimationSystemIteratorImpl(const AnimationSystemImpl& animSystem, ERamsesObjectType filterType)
            : ObjectIteratorImpl(animSystem.getObjectRegistry(), filterType)
        {
        }
    };
}

#endif
