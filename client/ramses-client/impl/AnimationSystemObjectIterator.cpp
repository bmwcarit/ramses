//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/AnimationSystemObjectIterator.h"
#include "AnimationSystemIteratorImpl.h"
#include "SceneImpl.h"
#include "ramses-client-api/AnimationSystem.h"
#include "AnimationSystemImpl.h"

namespace ramses
{

    AnimationSystemObjectIterator::AnimationSystemObjectIterator(const AnimationSystem& animationSystem, ERamsesObjectType objectType)
        : impl(new AnimationSystemIteratorImpl(animationSystem.impl, objectType))
    {
    }

    AnimationSystemObjectIterator::~AnimationSystemObjectIterator()
    {
        delete impl;
    }

    RamsesObject* AnimationSystemObjectIterator::getNext()
    {
        return impl->getNext();
    }
}
