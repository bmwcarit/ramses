//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/Spline.h"
#include "ramses-client-api/Animation.h"
#include "ramses-client-api/AnimatedSetter.h"
#include "ramses-client-api/AnimationSequence.h"
#include "ramses-client-api/SplineStepBool.h"
#include "ramses-client-api/SplineStepInt32.h"
#include "ramses-client-api/SplineStepFloat.h"
#include "ramses-client-api/SplineStepVector2f.h"
#include "ramses-client-api/SplineStepVector3f.h"
#include "ramses-client-api/SplineStepVector4f.h"
#include "ramses-client-api/SplineStepVector2i.h"
#include "ramses-client-api/SplineStepVector3i.h"
#include "ramses-client-api/SplineStepVector4i.h"
#include "ramses-client-api/SplineLinearInt32.h"
#include "ramses-client-api/SplineLinearFloat.h"
#include "ramses-client-api/SplineLinearVector2f.h"
#include "ramses-client-api/SplineLinearVector3f.h"
#include "ramses-client-api/SplineLinearVector4f.h"
#include "ramses-client-api/SplineLinearVector2i.h"
#include "ramses-client-api/SplineLinearVector3i.h"
#include "ramses-client-api/SplineLinearVector4i.h"
#include "ramses-client-api/SplineBezierInt32.h"
#include "ramses-client-api/SplineBezierFloat.h"
#include "ramses-client-api/SplineBezierVector2f.h"
#include "ramses-client-api/SplineBezierVector3f.h"
#include "ramses-client-api/SplineBezierVector4f.h"
#include "ramses-client-api/SplineBezierVector2i.h"
#include "ramses-client-api/SplineBezierVector3i.h"
#include "ramses-client-api/SplineBezierVector4i.h"

#include "AnimationSystemData.h"
#include "AnimationImpl.h"
#include "AnimationSystemImpl.h"
#include "AnimatedSetterImpl.h"
#include "AnimationSequenceImpl.h"
#include "SplineImpl.h"
#include "SerializationContext.h"
#include "RamsesObjectTypeUtils.h"
#include "SerializationHelper.h"
#include "RamsesObjectRegistryIterator.h"

#include "Scene/ClientScene.h"
#include "Scene/SceneDataBinding.h"
#include "AnimationAPI/IAnimationSystem.h"
#include "Animation/AnimationData.h"

namespace ramses
{
    AnimationSystemData::AnimationSystemData()
        : m_objectRegistry()
        , m_animationSystem(nullptr)
        , m_animatedSetterDelay(100u)
    {
    }

    AnimationSystemData::~AnimationSystemData()
    {
        RamsesObjectVector objects;
        m_objectRegistry.getObjectsOfType(objects, ERamsesObjectType_AnimationObject);
        for (const auto it : objects)
        {
            delete &RamsesObjectTypeUtils::ConvertTo<AnimationObject>(*it);
        }
    }

    void AnimationSystemData::initializeFrameworkData(AnimationSystemImpl& animationSystem)
    {
        m_animationSystem = &animationSystem;
        m_animatedPropertyFactory.reset(new AnimatedPropertyFactory(animationSystem));
    }

    void AnimationSystemData::deinitializeFrameworkData()
    {
    }

    status_t AnimationSystemData::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        outStream << m_animatedSetterDelay;

        return SerializationHelper::SerializeObjectsInRegistry<AnimationObject>(outStream, serializationContext, m_objectRegistry);
    }

    template <typename T>
    T& AnimationSystemData::createImplHelper(ERamsesObjectType)
    {
        return *new T(*m_animationSystem, "");
    }
    template <>
    SplineImpl& AnimationSystemData::createImplHelper<SplineImpl>(ERamsesObjectType type)
    {
        return *new SplineImpl(*m_animationSystem, type, "");
    }
    template <>
    AnimatedSetterImpl& AnimationSystemData::createImplHelper<AnimatedSetterImpl>(ERamsesObjectType)
    {
        return *new AnimatedSetterImpl(*m_animationSystem, m_animatedSetterDelay, "");
    }

    template <typename ObjectType, typename ObjectImplType>
    status_t AnimationSystemData::createAndDeserializeObjectImpls(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext, uint32_t count)
    {
        for (uint32_t i = 0u; i < count; ++i)
        {
            ObjectImplType& impl = createImplHelper<ObjectImplType>(TYPE_ID_OF_RAMSES_OBJECT<ObjectType>::ID);
            ObjectIDType objectID = DeserializationContext::GetObjectIDNull();
            const status_t status = SerializationHelper::DeserializeObjectImpl(inStream, serializationContext, impl, objectID);
            if (status != StatusOK)
            {
                delete &impl;
                return status;
            }
            m_objectRegistry.addObject(*new ObjectType(impl));
        }

        return StatusOK;
    }

    status_t AnimationSystemData::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        inStream >> m_animatedSetterDelay;

        uint32_t totalCount = 0u;
        uint32_t typesCount = 0u;
        SerializationHelper::DeserializeNumberOfObjectTypes(inStream, totalCount, typesCount);

        m_objectRegistry.reserveAdditionalGeneralCapacity(totalCount);
        for (uint32_t i = 0u; i < typesCount; ++i)
        {
            uint32_t count = 0u;
            const ERamsesObjectType type = SerializationHelper::DeserializeObjectTypeAndCount(inStream, count);
            assert(m_objectRegistry.getNumberOfObjects(type) == 0u);
            m_objectRegistry.reserveAdditionalObjectCapacity(type, count);

            status_t status = StatusOK;
            switch (type)
            {
            case ERamsesObjectType_SplineStepBool:
                status = createAndDeserializeObjectImpls<SplineStepBool, SplineImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_SplineStepFloat:
                status = createAndDeserializeObjectImpls<SplineStepFloat, SplineImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_SplineStepInt32:
                status = createAndDeserializeObjectImpls<SplineStepInt32, SplineImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_SplineStepVector2f:
                status = createAndDeserializeObjectImpls<SplineStepVector2f, SplineImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_SplineStepVector3f:
                status = createAndDeserializeObjectImpls<SplineStepVector3f, SplineImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_SplineStepVector4f:
                status = createAndDeserializeObjectImpls<SplineStepVector4f, SplineImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_SplineStepVector2i:
                status = createAndDeserializeObjectImpls<SplineStepVector2i, SplineImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_SplineStepVector3i:
                status = createAndDeserializeObjectImpls<SplineStepVector3i, SplineImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_SplineStepVector4i:
                status = createAndDeserializeObjectImpls<SplineStepVector4i, SplineImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_SplineLinearFloat:
                status = createAndDeserializeObjectImpls<SplineLinearFloat, SplineImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_SplineLinearInt32:
                status = createAndDeserializeObjectImpls<SplineLinearInt32, SplineImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_SplineLinearVector2f:
                status = createAndDeserializeObjectImpls<SplineLinearVector2f, SplineImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_SplineLinearVector3f:
                status = createAndDeserializeObjectImpls<SplineLinearVector3f, SplineImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_SplineLinearVector4f:
                status = createAndDeserializeObjectImpls<SplineLinearVector4f, SplineImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_SplineLinearVector2i:
                status = createAndDeserializeObjectImpls<SplineLinearVector2i, SplineImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_SplineLinearVector3i:
                status = createAndDeserializeObjectImpls<SplineLinearVector3i, SplineImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_SplineLinearVector4i:
                status = createAndDeserializeObjectImpls<SplineLinearVector4i, SplineImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_SplineBezierFloat:
                status = createAndDeserializeObjectImpls<SplineBezierFloat, SplineImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_SplineBezierInt32:
                status = createAndDeserializeObjectImpls<SplineBezierInt32, SplineImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_SplineBezierVector2f:
                status = createAndDeserializeObjectImpls<SplineBezierVector2f, SplineImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_SplineBezierVector3f:
                status = createAndDeserializeObjectImpls<SplineBezierVector3f, SplineImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_SplineBezierVector4f:
                status = createAndDeserializeObjectImpls<SplineBezierVector4f, SplineImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_SplineBezierVector2i:
                status = createAndDeserializeObjectImpls<SplineBezierVector2i, SplineImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_SplineBezierVector3i:
                status = createAndDeserializeObjectImpls<SplineBezierVector3i, SplineImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_SplineBezierVector4i:
                status = createAndDeserializeObjectImpls<SplineBezierVector4i, SplineImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_AnimatedProperty:
                status = createAndDeserializeObjectImpls<AnimatedProperty, AnimatedPropertyImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_Animation:
                status = createAndDeserializeObjectImpls<Animation, AnimationImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_AnimationSequence:
                status = createAndDeserializeObjectImpls<AnimationSequence, AnimationSequenceImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_AnimatedSetter:
                status = createAndDeserializeObjectImpls<AnimatedSetter, AnimatedSetterImpl>(inStream, serializationContext, count);
                break;
            default:
                return m_animationSystem->addErrorEntry("AnimationSystem::deserialize failed, unexpected object type in file stream.");
            }

            CHECK_RETURN_ERR(status);
        }

        return StatusOK;
    }

    Animation* AnimationSystemData::createAnimation(const AnimatedPropertyImpl& animatedProperty, const Spline& spline, const char* name)
    {
        assert(m_animationSystem != nullptr);
        const ramses_internal::EDataTypeID dataBindDataTypeID = animatedProperty.getDataTypeID();
        const ramses_internal::EDataTypeID splineDataTypeID = SplineImpl::GetDataTypeForSplineType(spline.getType());
        if (ramses_internal::AnimationData::CheckDataTypeCompatibility(splineDataTypeID, dataBindDataTypeID, animatedProperty.getVectorComponent()))
        {
            AnimationImpl& pimpl = *new AnimationImpl(*m_animationSystem, name);

            const ramses_internal::EInterpolationType interpolationType = SplineImpl::GetInterpolationTypeForSplineType(spline.impl.getType());
            pimpl.initializeFrameworkData(animatedProperty, spline.impl.getSplineHandle(), interpolationType);
            Animation* animation = new Animation(pimpl);
            m_objectRegistry.addObject(*animation);

            return animation;
        }

        return nullptr;
    }

    AnimatedSetter* AnimationSystemData::createAnimatedSetter(const AnimatedPropertyImpl& animatedProperty, const char* name)
    {
        assert(m_animationSystem != nullptr);
        AnimatedSetterImpl& pimpl = *new AnimatedSetterImpl(*m_animationSystem, m_animatedSetterDelay, name);

        pimpl.initializeFrameworkData(animatedProperty);
        AnimatedSetter* animatedSetter = new AnimatedSetter(pimpl);
        m_objectRegistry.addObject(*animatedSetter);

        return animatedSetter;
    }

    AnimationSequence* AnimationSystemData::createAnimationSequence(const char* name)
    {
        assert(m_animationSystem != nullptr);
        AnimationSequenceImpl& pimpl = *new AnimationSequenceImpl(*m_animationSystem, name);
        AnimationSequence* sequence = new AnimationSequence(pimpl);
        m_objectRegistry.addObject(*sequence);
        return sequence;
    }

    status_t AnimationSystemData::destroy(AnimationObject& object)
    {
        if (&object.impl.getAnimationSystemImpl() != m_animationSystem)
        {
            return m_animationSystem->addErrorEntry("AnimationSystem::destroy failed, object is not in this animation system.");
        }

        object.impl.deinitializeFrameworkData();
        m_objectRegistry.removeObject(object);
        delete &object;

        return StatusOK;
    }

    status_t AnimationSystemData::setDelayForAnimatedSetters(timeMilliseconds_t delay)
    {
        m_animatedSetterDelay = delay;
        return StatusOK;
    }

    timeMilliseconds_t AnimationSystemData::getDelayForAnimatedSetters() const
    {
        return m_animatedSetterDelay;
    }

    const RamsesObject* AnimationSystemData::findObjectByName(const char* name) const
    {
        return m_objectRegistry.findObjectByName(name);
    }

    RamsesObject* AnimationSystemData::findObjectByName(const char* name)
    {
        return m_objectRegistry.findObjectByName(name);
    }

    const RamsesObjectRegistry& AnimationSystemData::getObjectRegistry() const
    {
        return m_objectRegistry;
    }

    AnimatedProperty* AnimationSystemData::createAnimatedProperty(const EffectInputImpl& propertyOwner, const AppearanceImpl& appearance, EAnimatedPropertyComponent ePropertyComponent, ramses_internal::TDataBindID bindID, const char* name)
    {
        AnimatedProperty* animProperty = m_animatedPropertyFactory->createAnimatedProperty(propertyOwner, appearance, ePropertyComponent, bindID, name);
        if (animProperty != nullptr)
        {
            m_objectRegistry.addObject(*animProperty);
        }

        return animProperty;
    }
}
