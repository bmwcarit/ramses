//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONOBJECTIMPL_H
#define RAMSES_ANIMATIONOBJECTIMPL_H

#include "SceneObjectImpl.h"

namespace ramses_internal
{
    class IAnimationSystem;
}

namespace ramses
{
    class AnimationSystemImpl;

    class AnimationObjectImpl : public SceneObjectImpl
    {
    public:
        explicit AnimationObjectImpl(AnimationSystemImpl& animationSystem, ERamsesObjectType type, const char* name);
        virtual ~AnimationObjectImpl();

        const AnimationSystemImpl& getAnimationSystemImpl() const;
        AnimationSystemImpl&       getAnimationSystemImpl();

        const ramses_internal::IAnimationSystem& getIAnimationSystem() const;
        ramses_internal::IAnimationSystem&       getIAnimationSystem();

    private:
        AnimationSystemImpl& m_animationSystem;
    };
}

#endif
