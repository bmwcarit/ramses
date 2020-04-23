//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "AnimationObjectImpl.h"
#include "AnimationSystemImpl.h"

namespace ramses
{
    AnimationObjectImpl::AnimationObjectImpl(AnimationSystemImpl& animationSystem, ERamsesObjectType type, const char* name)
        : SceneObjectImpl(animationSystem.getSceneImpl(), type, name)
        , m_animationSystem(animationSystem)
    {
    }

    AnimationObjectImpl::~AnimationObjectImpl()
    {
    }

    const AnimationSystemImpl& AnimationObjectImpl::getAnimationSystemImpl() const
    {
        return m_animationSystem;
    }

    AnimationSystemImpl& AnimationObjectImpl::getAnimationSystemImpl()
    {
        return m_animationSystem;
    }

    const ramses_internal::IAnimationSystem& AnimationObjectImpl::getIAnimationSystem() const
    {
        return m_animationSystem.getIAnimationSystem();
    }

    ramses_internal::IAnimationSystem& AnimationObjectImpl::getIAnimationSystem()
    {
        return m_animationSystem.getIAnimationSystem();
    }
}
