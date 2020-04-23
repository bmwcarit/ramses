//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONSYSTEMDATA_H
#define RAMSES_ANIMATIONSYSTEMDATA_H

// internal
#include "AnimatedPropertyFactory.h"
#include "SplineImpl.h"
#include "SceneImpl.h"

// framework
#include "RamsesObjectRegistry.h"
#include <memory>

namespace ramses
{
    class Spline;
    class AnimatedProperty;
    class Animation;
    class AnimatedSetter;
    class AnimationSequence;
    class EffectInputImpl;
    class AppearanceImpl;

    class AnimationSystemData
    {
    public:
        AnimationSystemData();
        ~AnimationSystemData();

        void initializeFrameworkData(AnimationSystemImpl& animationSystem);
        void deinitializeFrameworkData();

        status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const;
        status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext);

        template <typename SplineType>
        SplineType*        createSpline(ramses_internal::EInterpolationType interpolationType, ramses_internal::EDataTypeID dataType, ERamsesObjectType objectType, const char* name);
        Animation*         createAnimation(const AnimatedPropertyImpl& animatedProperty, const Spline& spline, const char* name);
        AnimatedSetter*    createAnimatedSetter(const AnimatedPropertyImpl& animatedProperty, const char* name);
        AnimationSequence* createAnimationSequence(const char* name);
        template <typename PropertyOwnerType>
        AnimatedProperty*  createAnimatedProperty(const PropertyOwnerType& propertyOwner, EAnimatedPropertyComponent ePropertyComponent, ramses_internal::TDataBindID bindID, const char* name);
        AnimatedProperty*  createAnimatedProperty(const EffectInputImpl& propertyOwner, const AppearanceImpl& appearance, EAnimatedPropertyComponent ePropertyComponent, ramses_internal::TDataBindID bindID, const char* name);

        status_t           setDelayForAnimatedSetters(timeMilliseconds_t delay);
        timeMilliseconds_t getDelayForAnimatedSetters() const;

        status_t destroy(AnimationObject& object);

        const RamsesObject*       findObjectByName(const char* name) const;
        RamsesObject*             findObjectByName(const char* name);
        const RamsesObjectRegistry& getObjectRegistry() const;

    private:
        template <typename T>
        T& createImplHelper(ERamsesObjectType type);
        template <typename ObjectType, typename ObjectImplType>
        status_t createAndDeserializeObjectImpls(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext, uint32_t count);

        RamsesObjectRegistry               m_objectRegistry;
        AnimationSystemImpl*               m_animationSystem;
        timeMilliseconds_t                 m_animatedSetterDelay;

        std::unique_ptr<AnimatedPropertyFactory> m_animatedPropertyFactory;
    };

    template <typename SplineType>
    SplineType* AnimationSystemData::createSpline(ramses_internal::EInterpolationType interpolationType, ramses_internal::EDataTypeID dataType, ERamsesObjectType objectType, const char* name)
    {
        assert(m_animationSystem != nullptr);
        SplineImpl& splineImpl = *new SplineImpl(*m_animationSystem, objectType, name);
        splineImpl.initializeFrameworkData(interpolationType, dataType);
        SplineType* spline = new SplineType(splineImpl);
        m_objectRegistry.addObject(*spline);
        return spline;
    }

    template <typename PropertyOwnerType>
    AnimatedProperty* AnimationSystemData::createAnimatedProperty(const PropertyOwnerType& propertyOwner, EAnimatedPropertyComponent ePropertyComponent, ramses_internal::TDataBindID bindID, const char* name)
    {
        AnimatedProperty* animProperty = m_animatedPropertyFactory->createAnimatedProperty(propertyOwner, ePropertyComponent, bindID, name);
        if (animProperty != nullptr)
        {
            m_objectRegistry.addObject(*animProperty);
        }

        return animProperty;
    }
}

#endif
