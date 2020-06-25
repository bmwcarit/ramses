//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATEDPROPERTYFACTORY_H
#define RAMSES_ANIMATEDPROPERTYFACTORY_H

#include "ramses-client-api/AnimatedProperty.h"
#include "AnimatedPropertyImpl.h"
#include "Utils/DataBindCommon.h"

namespace ramses
{
    class NodeImpl;
    class EffectInputImpl;
    class DataObjectImpl;
    class AppearanceImpl;
    class AnimationSystemImpl;

    class AnimatedPropertyFactory
    {
    public:
        explicit AnimatedPropertyFactory(AnimationSystemImpl& animationSystem);

        AnimatedProperty* createAnimatedProperty(const NodeImpl& propertyOwner, EAnimatedPropertyComponent ePropertyComponent, ramses_internal::TDataBindID bindID, const char* name);
        AnimatedProperty* createAnimatedProperty(const EffectInputImpl& propertyOwner, const AppearanceImpl& appearance, EAnimatedPropertyComponent ePropertyComponent, ramses_internal::TDataBindID bindID, const char* name);
        AnimatedProperty* createAnimatedProperty(const DataObjectImpl& propertyOwner, EAnimatedPropertyComponent ePropertyComponent, ramses_internal::TDataBindID bindID, const char* name);

    private:
        AnimatedPropertyImpl* createAnimatedPropertyImpl(const NodeImpl& propertyOwner, EAnimatedPropertyComponent ePropertyComponent, ramses_internal::TDataBindID bindID, const char* name);
        AnimatedPropertyImpl* createAnimatedPropertyImpl(const EffectInputImpl& propertyOwner, const AppearanceImpl& appearance, EAnimatedPropertyComponent ePropertyComponent, ramses_internal::TDataBindID bindID, const char* name);
        AnimatedPropertyImpl* createAnimatedPropertyImpl(const DataObjectImpl& propertyOwner, EAnimatedPropertyComponent ePropertyComponent, ramses_internal::TDataBindID bindID, const char* name);
        AnimatedPropertyImpl* createAnimatedPropertyImpl(ramses_internal::MemoryHandle handle1, ramses_internal::MemoryHandle handle2, EAnimatedPropertyComponent ePropertyComponent, ramses_internal::TDataBindID bindID, const char* name);

        AnimationSystemImpl& m_animationSystem;
    };
}

#endif
