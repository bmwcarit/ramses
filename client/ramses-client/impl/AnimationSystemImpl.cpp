//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

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
#include "ramses-client-api/AnimatedProperty.h"
#include "ramses-client-api/Animation.h"
#include "ramses-client-api/AnimationSequence.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/DataObject.h"
#include "ramses-client-api/Appearance.h"

#include "AnimationSystemImpl.h"
#include "SceneImpl.h"
#include "SplineImpl.h"
#include "AnimationImpl.h"
#include "DataObjectImpl.h"
#include "AppearanceImpl.h"
#include "EffectImpl.h"
#include "EffectInputImpl.h"
#include "NodeImpl.h"
#include "RamsesObjectTypeUtils.h"
#include "RamsesObjectRegistryIterator.h"

#include "Scene/ClientScene.h"
#include "Scene/SceneDataBinding.h"
#include "Animation/AnimationSystem.h"
#include "AnimationAPI/IAnimationSystem.h"

#include "PlatformAbstraction/PlatformTime.h"
#include "Utils/LogMacros.h"

namespace ramses
{
    AnimationSystemImpl::AnimationSystemImpl(SceneImpl& sceneImpl, ERamsesObjectType type, const char* name)
        : SceneObjectImpl(sceneImpl, type, name)
        , m_animationSystem(nullptr)
        , m_animationSystemHandle(ramses_internal::AnimationSystemHandle::Invalid())
    {
    }

    AnimationSystemImpl::~AnimationSystemImpl()
    {
    }

    void AnimationSystemImpl::initializeFrameworkData(ramses_internal::IAnimationSystem& animationSystem)
    {
        m_data.initializeFrameworkData(*this);
        m_animationSystemHandle = getIScene().addAnimationSystem(&animationSystem);
        m_animationSystem = &animationSystem;
        m_animationSystem->registerAnimationLogicListener(&m_animationStateCollector);
    }

    void AnimationSystemImpl::deinitializeFrameworkData()
    {
        assert(m_animationSystem != nullptr);
        m_animationSystem->unregisterAnimationLogicListener(&m_animationStateCollector);
        m_data.deinitializeFrameworkData();
        getIScene().removeAnimationSystem(m_animationSystemHandle);
    }

    status_t AnimationSystemImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(SceneObjectImpl::serialize(outStream, serializationContext));

        assert(m_animationSystem != nullptr);
        outStream << m_animationSystemHandle;

        return m_data.serialize(outStream, serializationContext);
    }

    status_t AnimationSystemImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(SceneObjectImpl::deserialize(inStream, serializationContext));

        inStream >> m_animationSystemHandle;

        m_animationSystem = getIScene().getAnimationSystem(m_animationSystemHandle);
        if (m_animationSystem == nullptr)
        {
            return addErrorEntry("AnimationSystem deserialize failed: could not retrieve animation system data!");
        }
        m_animationSystem->registerAnimationLogicListener(&m_animationStateCollector);

        m_data.initializeFrameworkData(*this);
        CHECK_RETURN_ERR(m_data.deserialize(inStream, serializationContext));

        return StatusOK;
    }

    status_t AnimationSystemImpl::validate(uint32_t indent, StatusObjectSet& visitedObjects) const
    {
        status_t status = ClientObjectImpl::validate(indent, visitedObjects);
        indent += IndentationStep;

        uint32_t objectCount[ERamsesObjectType_NUMBER_OF_TYPES];
        for (uint32_t i = 0u; i < ERamsesObjectType_NUMBER_OF_TYPES; ++i)
        {
            const ERamsesObjectType type = static_cast<ERamsesObjectType>(i);
            if (RamsesObjectTypeUtils::IsTypeMatchingBaseType(type, ERamsesObjectType_AnimationObject) &&
                RamsesObjectTypeUtils::IsConcreteType(type))
            {
                objectCount[i] = 0u;
                RamsesObjectRegistryIterator iter(getObjectRegistry(), ERamsesObjectType(i));
                while (const RamsesObject* obj = iter.getNext())
                {
                    if (addValidationOfDependentObject(indent, obj->impl, visitedObjects) != StatusOK)
                    {
                        status = getValidationErrorStatus();
                    }
                    ++objectCount[i];
                }
            }
        }

        for (uint32_t i = 0u; i < ERamsesObjectType_NUMBER_OF_TYPES; ++i)
        {
            const ERamsesObjectType type = static_cast<ERamsesObjectType>(i);
            if (RamsesObjectTypeUtils::IsTypeMatchingBaseType(type, ERamsesObjectType_AnimationObject) &&
                RamsesObjectTypeUtils::IsConcreteType(type))
            {
                ramses_internal::StringOutputStream msg;
                msg << "Number of " << RamsesObjectTypeUtils::GetRamsesObjectTypeName(type) << " instances: " << objectCount[i];
                addValidationMessage(EValidationSeverity_Info, indent, msg.release());
            }
        }

        return status;
    }

    status_t AnimationSystemImpl::setTime(globalTimeStamp_t timeStamp)
    {
        m_animationStateCollector.resetCollections();
        assert(m_animationSystem != nullptr);
        m_animationSystem->setTime(ramses_internal::AnimationTime(timeStamp));
        return StatusOK;
    }

    globalTimeStamp_t AnimationSystemImpl::getTime() const
    {
        assert(m_animationSystem != nullptr);
        return m_animationSystem->getTime().getTimeStamp();
    }

    status_t AnimationSystemImpl::updateLocalTime(globalTimeStamp_t systemTime)
    {
        ramses_internal::AnimationTime timeStamp = ramses_internal::AnimationTime(systemTime);
        if (systemTime == 0u)
        {
            timeStamp = ramses_internal::PlatformTime::GetMillisecondsAbsolute();
        }

        m_animationStateCollector.resetCollections();
        assert(m_animationSystem != nullptr);
        ramses_internal::AnimationSystem& nonDistributedAnimationSystem = static_cast<ramses_internal::AnimationSystem&>(*m_animationSystem);
        nonDistributedAnimationSystem.ramses_internal::AnimationSystem::setTime(timeStamp);

        return StatusOK;
    }

    SplineStepBool*       AnimationSystemImpl::createSplineStepBool(const char* name)
    {
        return m_data.createSpline<SplineStepBool>(ramses_internal::EInterpolationType_Step, ramses_internal::EDataTypeID_Boolean, ERamsesObjectType_SplineStepBool, name);
    }

    SplineStepInt32*      AnimationSystemImpl::createSplineStepInt32(const char* name)
    {
        return m_data.createSpline<SplineStepInt32>(ramses_internal::EInterpolationType_Step, ramses_internal::EDataTypeID_Int32, ERamsesObjectType_SplineStepInt32, name);
    }

    SplineStepFloat*      AnimationSystemImpl::createSplineStepFloat(const char* name)
    {
        return m_data.createSpline<SplineStepFloat>(ramses_internal::EInterpolationType_Step, ramses_internal::EDataTypeID_Float, ERamsesObjectType_SplineStepFloat, name);
    }

    SplineStepVector2f*   AnimationSystemImpl::createSplineStepVector2f(const char* name)
    {
        return m_data.createSpline<SplineStepVector2f>(ramses_internal::EInterpolationType_Step, ramses_internal::EDataTypeID_Vector2f, ERamsesObjectType_SplineStepVector2f, name);
    }

    SplineStepVector3f*   AnimationSystemImpl::createSplineStepVector3f(const char* name)
    {
        return m_data.createSpline<SplineStepVector3f>(ramses_internal::EInterpolationType_Step, ramses_internal::EDataTypeID_Vector3f, ERamsesObjectType_SplineStepVector3f, name);
    }

    SplineStepVector4f*   AnimationSystemImpl::createSplineStepVector4f(const char* name)
    {
        return m_data.createSpline<SplineStepVector4f>(ramses_internal::EInterpolationType_Step, ramses_internal::EDataTypeID_Vector4f, ERamsesObjectType_SplineStepVector4f, name);
    }

    SplineStepVector2i*   AnimationSystemImpl::createSplineStepVector2i(const char* name)
    {
        return m_data.createSpline<SplineStepVector2i>(ramses_internal::EInterpolationType_Step, ramses_internal::EDataTypeID_Vector2i, ERamsesObjectType_SplineStepVector2i, name);
    }

    SplineStepVector3i*   AnimationSystemImpl::createSplineStepVector3i(const char* name)
    {
        return m_data.createSpline<SplineStepVector3i>(ramses_internal::EInterpolationType_Step, ramses_internal::EDataTypeID_Vector3i, ERamsesObjectType_SplineStepVector3i, name);
    }

    SplineStepVector4i*   AnimationSystemImpl::createSplineStepVector4i(const char* name)
    {
        return m_data.createSpline<SplineStepVector4i>(ramses_internal::EInterpolationType_Step, ramses_internal::EDataTypeID_Vector4i, ERamsesObjectType_SplineStepVector4i, name);
    }

    SplineLinearInt32*      AnimationSystemImpl::createSplineLinearInt32(const char* name)
    {
        return m_data.createSpline<SplineLinearInt32>(ramses_internal::EInterpolationType_Linear, ramses_internal::EDataTypeID_Int32, ERamsesObjectType_SplineLinearInt32, name);
    }

    SplineLinearFloat*      AnimationSystemImpl::createSplineLinearFloat(const char* name)
    {
        return m_data.createSpline<SplineLinearFloat>(ramses_internal::EInterpolationType_Linear, ramses_internal::EDataTypeID_Float, ERamsesObjectType_SplineLinearFloat, name);
    }

    SplineLinearVector2f*   AnimationSystemImpl::createSplineLinearVector2f(const char* name)
    {
        return m_data.createSpline<SplineLinearVector2f>(ramses_internal::EInterpolationType_Linear, ramses_internal::EDataTypeID_Vector2f, ERamsesObjectType_SplineLinearVector2f, name);
    }

    SplineLinearVector3f*   AnimationSystemImpl::createSplineLinearVector3f(const char* name)
    {
        return m_data.createSpline<SplineLinearVector3f>(ramses_internal::EInterpolationType_Linear, ramses_internal::EDataTypeID_Vector3f, ERamsesObjectType_SplineLinearVector3f, name);
    }

    SplineLinearVector4f*   AnimationSystemImpl::createSplineLinearVector4f(const char* name)
    {
        return m_data.createSpline<SplineLinearVector4f>(ramses_internal::EInterpolationType_Linear, ramses_internal::EDataTypeID_Vector4f, ERamsesObjectType_SplineLinearVector4f, name);
    }

    SplineLinearVector2i*   AnimationSystemImpl::createSplineLinearVector2i(const char* name)
    {
        return m_data.createSpline<SplineLinearVector2i>(ramses_internal::EInterpolationType_Linear, ramses_internal::EDataTypeID_Vector2i, ERamsesObjectType_SplineLinearVector2i, name);
    }

    SplineLinearVector3i*   AnimationSystemImpl::createSplineLinearVector3i(const char* name)
    {
        return m_data.createSpline<SplineLinearVector3i>(ramses_internal::EInterpolationType_Linear, ramses_internal::EDataTypeID_Vector3i, ERamsesObjectType_SplineLinearVector3i, name);
    }

    SplineLinearVector4i*   AnimationSystemImpl::createSplineLinearVector4i(const char* name)
    {
        return m_data.createSpline<SplineLinearVector4i>(ramses_internal::EInterpolationType_Linear, ramses_internal::EDataTypeID_Vector4i, ERamsesObjectType_SplineLinearVector4i, name);
    }

    SplineBezierInt32*      AnimationSystemImpl::createSplineBezierInt32(const char* name)
    {
        return m_data.createSpline<SplineBezierInt32>(ramses_internal::EInterpolationType_Bezier, ramses_internal::EDataTypeID_Int32, ERamsesObjectType_SplineBezierInt32, name);
    }

    SplineBezierFloat*      AnimationSystemImpl::createSplineBezierFloat(const char* name)
    {
        return m_data.createSpline<SplineBezierFloat>(ramses_internal::EInterpolationType_Bezier, ramses_internal::EDataTypeID_Float, ERamsesObjectType_SplineBezierFloat, name);
    }

    SplineBezierVector2f*   AnimationSystemImpl::createSplineBezierVector2f(const char* name)
    {
        return m_data.createSpline<SplineBezierVector2f>(ramses_internal::EInterpolationType_Bezier, ramses_internal::EDataTypeID_Vector2f, ERamsesObjectType_SplineBezierVector2f, name);
    }

    SplineBezierVector3f*   AnimationSystemImpl::createSplineBezierVector3f(const char* name)
    {
        return m_data.createSpline<SplineBezierVector3f>(ramses_internal::EInterpolationType_Bezier, ramses_internal::EDataTypeID_Vector3f, ERamsesObjectType_SplineBezierVector3f, name);
    }

    SplineBezierVector4f*   AnimationSystemImpl::createSplineBezierVector4f(const char* name)
    {
        return m_data.createSpline<SplineBezierVector4f>(ramses_internal::EInterpolationType_Bezier, ramses_internal::EDataTypeID_Vector4f, ERamsesObjectType_SplineBezierVector4f, name);
    }

    SplineBezierVector2i*   AnimationSystemImpl::createSplineBezierVector2i(const char* name)
    {
        return m_data.createSpline<SplineBezierVector2i>(ramses_internal::EInterpolationType_Bezier, ramses_internal::EDataTypeID_Vector2i, ERamsesObjectType_SplineBezierVector2i, name);
    }

    SplineBezierVector3i*   AnimationSystemImpl::createSplineBezierVector3i(const char* name)
    {
        return m_data.createSpline<SplineBezierVector3i>(ramses_internal::EInterpolationType_Bezier, ramses_internal::EDataTypeID_Vector3i, ERamsesObjectType_SplineBezierVector3i, name);
    }

    SplineBezierVector4i*   AnimationSystemImpl::createSplineBezierVector4i(const char* name)
    {
        return m_data.createSpline<SplineBezierVector4i>(ramses_internal::EInterpolationType_Bezier, ramses_internal::EDataTypeID_Vector4i, ERamsesObjectType_SplineBezierVector4i, name);
    }

    Animation* AnimationSystemImpl::createAnimation(const AnimatedProperty& animatedProperty, const Spline& spline, const char* name)
    {
        if (!containsAnimationObject(animatedProperty.impl) ||
            !containsAnimationObject(spline.impl))
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "AnimationSystem::createAnimation: failed to create Animation, provided property and/or spline belong to another AnimationSystem!");
            return nullptr;
        }

        return m_data.createAnimation(animatedProperty.impl, spline, name);
    }

    AnimationSequence* AnimationSystemImpl::createAnimationSequence(const char* name)
    {
        return m_data.createAnimationSequence(name);
    }

    AnimatedProperty* AnimationSystemImpl::createAnimatedProperty(const NodeImpl& propertyOwner, EAnimatedProperty property, EAnimatedPropertyComponent propertyComponent, const char* name)
    {
        typedef ramses_internal::DataBindContainerToTraitsSelector<ramses_internal::IScene>::ContainerTraitsClassType ContainerTraitsClass;
        ramses_internal::TDataBindID dataBindID(std::numeric_limits<ramses_internal::TDataBindID>::max());
        switch (property)
        {
        case EAnimatedProperty_Translation:
            dataBindID = ContainerTraitsClass::TransformNode_Translation;
            break;
        case EAnimatedProperty_Rotation:
            dataBindID = ContainerTraitsClass::TransformNode_Rotation;
            break;
        case EAnimatedProperty_Scaling:
            dataBindID = ContainerTraitsClass::TransformNode_Scaling;
            break;
        }

        // (Violin) this cast is necessary because the animation needs to force creation of the internal
        // transform so that it tries to change it by direct access to LL handle
        // The actual problem is that the propertyOwner is const - it shouldn't (it is being changed in the animation)
        const_cast<NodeImpl&>(propertyOwner).initializeTransform();

        return m_data.createAnimatedProperty(propertyOwner, propertyComponent, dataBindID, name);
    }

    AnimatedProperty* AnimationSystemImpl::createAnimatedProperty(const UniformInput& propertyOwner, const Appearance& appearance, EAnimatedPropertyComponent propertyComponent, const char* name)
    {
        if (!propertyOwner.isValid() || propertyOwner.impl.getEffectHash() != appearance.impl.getEffectImpl()->getLowlevelResourceHash())
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "AnimationSystem::createAnimatedProperty:  failed to create AnimatedProperty, uniform input invalid or does not match provided Appearance!");
            return nullptr;
        }

        ramses_internal::TDataBindID bindId = 0u;
        if (!GetDataBindIDForDataType(propertyOwner.impl.getDataType(), bindId))
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "AnimationSystem::createAnimatedProperty: failed to create AnimatedProperty, unsupported data type to animate!");
            return nullptr;
        }

        return m_data.createAnimatedProperty(propertyOwner.impl, appearance.impl, propertyComponent, bindId, name);
    }

    ramses::AnimatedProperty* AnimationSystemImpl::createAnimatedProperty(const DataObject& propertyOwner, EAnimatedPropertyComponent propertyComponent, const char* name)
    {
        ramses_internal::TDataBindID bindId = 0u;
        if (!GetDataBindIDForDataType(propertyOwner.impl.getDataType(), bindId))
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "AnimationSystem::createAnimatedProperty: failed to create AnimatedProperty, unsupported data type to animate!");
            return nullptr;
        }

        return m_data.createAnimatedProperty(propertyOwner.impl, propertyComponent, bindId, name);
    }

    status_t AnimationSystemImpl::destroy(AnimationObject& animationObject)
    {
        return m_data.destroy(animationObject);
    }

    ramses_internal::IAnimationSystem& AnimationSystemImpl::getIAnimationSystem()
    {
        assert(m_animationSystem != nullptr);
        return *m_animationSystem;
    }

    ramses_internal::AnimationSystemHandle AnimationSystemImpl::getAnimationSystemHandle() const
    {
        assert(m_animationSystemHandle.isValid());
        return m_animationSystemHandle;
    }

    bool AnimationSystemImpl::containsAnimationObject(const AnimationObjectImpl& object) const
    {
        return &object.getAnimationSystemImpl() == this;
    }

    const RamsesObject* AnimationSystemImpl::findObjectByName(const char* name) const
    {
        return m_data.findObjectByName(name);
    }

    RamsesObject* AnimationSystemImpl::findObjectByName(const char* name)
    {
        return m_data.findObjectByName(name);
    }

    const RamsesObjectRegistry& AnimationSystemImpl::getObjectRegistry() const
    {
        return m_data.getObjectRegistry();
    }

    uint32_t AnimationSystemImpl::getNumberOfFinishedAnimationsSincePreviousUpdate() const
    {
        return static_cast<uint32_t>(m_animationStateCollector.getCollectedFinishedAnimations().size());
    }

    const Animation* AnimationSystemImpl::getFinishedAnimationSincePreviousUpdate(uint32_t index) const
    {
        const ramses_internal::AnimationHandleVector& animHandles = m_animationStateCollector.getCollectedFinishedAnimations();
        if (index < animHandles.size())
        {
            return findAnimationByHandle(animHandles[index]);
        }

        return nullptr;
    }

    Animation* AnimationSystemImpl::getFinishedAnimationSincePreviousUpdate(uint32_t index)
    {
        // non-const version of getFinishedAnimationSincePreviousUpdate cast to its const version to avoid duplicating code
        return const_cast<Animation*>((const_cast<const AnimationSystemImpl&>(*this)).getFinishedAnimationSincePreviousUpdate(index));
    }

    const Animation* AnimationSystemImpl::findAnimationByHandle(ramses_internal::AnimationHandle handle) const
    {
        RamsesObjectRegistryIterator iter(getObjectRegistry(), ERamsesObjectType_Animation);
        while (const Animation* animation = iter.getNext<Animation>())
        {
            if (animation->impl.getAnimationHandle() == handle)
            {
                return animation;
            }
        }

        return nullptr;
    }

    bool AnimationSystemImpl::GetDataBindIDForDataType(ramses_internal::EDataType dataType, ramses_internal::TDataBindID& bindId)
    {
        typedef ramses_internal::DataBindContainerToTraitsSelector<ramses_internal::IScene>::ContainerTraitsClassType ContainerTraitsClass;

        switch (dataType)
        {
        case ramses_internal::EDataType_Float:
            bindId = ContainerTraitsClass::DataField_Float;
            break;
        case ramses_internal::EDataType_Vector2F:
            bindId = ContainerTraitsClass::DataField_Vector2f;
            break;
        case ramses_internal::EDataType_Vector3F:
            bindId = ContainerTraitsClass::DataField_Vector3f;
            break;
        case ramses_internal::EDataType_Vector4F:
            bindId = ContainerTraitsClass::DataField_Vector4f;
            break;
        case ramses_internal::EDataType_Int32:
            bindId = ContainerTraitsClass::DataField_Integer;
            break;
        case ramses_internal::EDataType_Vector2I:
            bindId = ContainerTraitsClass::DataField_Vector2i;
            break;
        case ramses_internal::EDataType_Vector3I:
            bindId = ContainerTraitsClass::DataField_Vector3i;
            break;
        case ramses_internal::EDataType_Vector4I:
            bindId = ContainerTraitsClass::DataField_Vector4i;
            break;
        default:
            return false;
        }

        return true;
    }
}
