//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/AnimationSystem.h"
#include "ramses-client-api/Node.h"
#include "ramses-client-api/Spline.h"
#include "ramses-client-api/DataObject.h"
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
#include "ramses-client-api/Animation.h"
#include "ramses-client-api/AnimationSequence.h"
#include "ramses-client-api/PickableObject.h"

// internal
#include "AnimationSystemImpl.h"

namespace ramses
{
    AnimationSystem::AnimationSystem(AnimationSystemImpl& pimpl)
        : SceneObject(pimpl)
        , impl(pimpl)
    {
    }

    AnimationSystem::~AnimationSystem()
    {
    }

    status_t AnimationSystem::setTime(globalTimeStamp_t timeStamp)
    {
        const status_t status = impl.setTime(timeStamp);
        LOG_HL_CLIENT_API1(status, timeStamp);
        return status;
    }

    globalTimeStamp_t AnimationSystem::getTime() const
    {
        return impl.getTime();
    }

    SplineStepBool*       AnimationSystem::createSplineStepBool(const char* name)
    {
        SplineStepBool* spline = impl.createSplineStepBool(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(spline), name);
        return spline;
    }

    SplineStepInt32*      AnimationSystem::createSplineStepInt32(const char* name)
    {
        SplineStepInt32* spline = impl.createSplineStepInt32(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(spline), name);
        return spline;
    }

    SplineStepFloat*      AnimationSystem::createSplineStepFloat(const char* name)
    {
        SplineStepFloat* spline = impl.createSplineStepFloat(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(spline), name);
        return spline;
    }

    SplineStepVector2f*   AnimationSystem::createSplineStepVector2f(const char* name)
    {
        SplineStepVector2f* spline = impl.createSplineStepVector2f(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(spline), name);
        return spline;
    }

    SplineStepVector3f*   AnimationSystem::createSplineStepVector3f(const char* name)
    {
        SplineStepVector3f* spline = impl.createSplineStepVector3f(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(spline), name);
        return spline;
    }

    SplineStepVector4f*   AnimationSystem::createSplineStepVector4f(const char* name)
    {
        SplineStepVector4f* spline = impl.createSplineStepVector4f(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(spline), name);
        return spline;
    }

    SplineStepVector2i*   AnimationSystem::createSplineStepVector2i(const char* name)
    {
        SplineStepVector2i* spline = impl.createSplineStepVector2i(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(spline), name);
        return spline;
    }

    SplineStepVector3i*   AnimationSystem::createSplineStepVector3i(const char* name)
    {
        SplineStepVector3i* spline = impl.createSplineStepVector3i(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(spline), name);
        return spline;
    }

    SplineStepVector4i*   AnimationSystem::createSplineStepVector4i(const char* name)
    {
        SplineStepVector4i* spline = impl.createSplineStepVector4i(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(spline), name);
        return spline;
    }

    SplineLinearInt32*      AnimationSystem::createSplineLinearInt32(const char* name)
    {
        SplineLinearInt32* spline = impl.createSplineLinearInt32(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(spline), name);
        return spline;
    }

    SplineLinearFloat*      AnimationSystem::createSplineLinearFloat(const char* name)
    {
        SplineLinearFloat* spline = impl.createSplineLinearFloat(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(spline), name);
        return spline;
    }

    SplineLinearVector2f*   AnimationSystem::createSplineLinearVector2f(const char* name)
    {
        SplineLinearVector2f* spline = impl.createSplineLinearVector2f(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(spline), name);
        return spline;
    }

    SplineLinearVector3f*   AnimationSystem::createSplineLinearVector3f(const char* name)
    {
        SplineLinearVector3f* spline = impl.createSplineLinearVector3f(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(spline), name);
        return spline;
    }

    SplineLinearVector4f*   AnimationSystem::createSplineLinearVector4f(const char* name)
    {
        SplineLinearVector4f* spline = impl.createSplineLinearVector4f(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(spline), name);
        return spline;
    }

    SplineLinearVector2i*   AnimationSystem::createSplineLinearVector2i(const char* name)
    {
        SplineLinearVector2i* spline = impl.createSplineLinearVector2i(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(spline), name);
        return spline;
    }

    SplineLinearVector3i*   AnimationSystem::createSplineLinearVector3i(const char* name)
    {
        SplineLinearVector3i* spline = impl.createSplineLinearVector3i(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(spline), name);
        return spline;
    }

    SplineLinearVector4i*   AnimationSystem::createSplineLinearVector4i(const char* name)
    {
        SplineLinearVector4i* spline = impl.createSplineLinearVector4i(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(spline), name);
        return spline;
    }

    SplineBezierInt32*      AnimationSystem::createSplineBezierInt32(const char* name)
    {
        SplineBezierInt32* spline = impl.createSplineBezierInt32(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(spline), name);
        return spline;
    }

    SplineBezierFloat*      AnimationSystem::createSplineBezierFloat(const char* name)
    {
        SplineBezierFloat* spline = impl.createSplineBezierFloat(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(spline), name);
        return spline;
    }

    SplineBezierVector2f*   AnimationSystem::createSplineBezierVector2f(const char* name)
    {
        SplineBezierVector2f* spline = impl.createSplineBezierVector2f(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(spline), name);
        return spline;
    }

    SplineBezierVector3f*   AnimationSystem::createSplineBezierVector3f(const char* name)
    {
        SplineBezierVector3f* spline = impl.createSplineBezierVector3f(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(spline), name);
        return spline;
    }

    SplineBezierVector4f*   AnimationSystem::createSplineBezierVector4f(const char* name)
    {
        SplineBezierVector4f* spline = impl.createSplineBezierVector4f(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(spline), name);
        return spline;
    }

    SplineBezierVector2i*   AnimationSystem::createSplineBezierVector2i(const char* name)
    {
        SplineBezierVector2i* spline = impl.createSplineBezierVector2i(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(spline), name);
        return spline;
    }

    SplineBezierVector3i*   AnimationSystem::createSplineBezierVector3i(const char* name)
    {
        SplineBezierVector3i* spline = impl.createSplineBezierVector3i(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(spline), name);
        return spline;
    }

    SplineBezierVector4i*   AnimationSystem::createSplineBezierVector4i(const char* name)
    {
        SplineBezierVector4i* spline = impl.createSplineBezierVector4i(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(spline), name);
        return spline;
    }

    Animation* AnimationSystem::createAnimation(const AnimatedProperty& animatedProperty, const Spline& spline, const char* name)
    {
        Animation* animation = impl.createAnimation(animatedProperty, spline, name);
        LOG_HL_CLIENT_API3(LOG_API_RAMSESOBJECT_PTR_STRING(animation), LOG_API_RAMSESOBJECT_STRING(animatedProperty), LOG_API_RAMSESOBJECT_STRING(spline), name);
        return animation;
    }

    AnimationSequence* AnimationSystem::createAnimationSequence(const char* name)
    {
        AnimationSequence* animationSequence = impl.createAnimationSequence(name);
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(animationSequence), name);
        return animationSequence;
    }

    status_t AnimationSystem::destroy(AnimationObject& animationObject)
    {
        const status_t status = impl.destroy(animationObject);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(animationObject));
        return status;
    }

    AnimatedProperty* AnimationSystem::createAnimatedProperty(const Node& propertyOwner, EAnimatedProperty property, EAnimatedPropertyComponent propertyComponent, const char* name)
    {
        AnimatedProperty* animatedProperty = impl.createAnimatedProperty(propertyOwner.impl, property, propertyComponent, name);
        LOG_HL_CLIENT_API4(LOG_API_RAMSESOBJECT_PTR_STRING(animatedProperty), LOG_API_RAMSESOBJECT_STRING(propertyOwner), property, propertyComponent, name);
        return animatedProperty;
    }

    AnimatedProperty* AnimationSystem::createAnimatedProperty(const UniformInput& propertyOwner, const Appearance& appearance, EAnimatedPropertyComponent propertyComponent, const char* name)
    {
        AnimatedProperty* animatedProperty = impl.createAnimatedProperty(propertyOwner, appearance, propertyComponent, name);
        LOG_HL_CLIENT_API4(LOG_API_RAMSESOBJECT_PTR_STRING(animatedProperty), LOG_API_GENERIC_OBJECT_STRING(propertyOwner), LOG_API_RAMSESOBJECT_STRING(appearance), propertyComponent, name);
        return animatedProperty;
    }

    AnimatedProperty* AnimationSystem::createAnimatedProperty(const DataObject& propertyOwner, EAnimatedPropertyComponent propertyComponent, const char* name)
    {
        AnimatedProperty* animatedProperty = impl.createAnimatedProperty(propertyOwner, propertyComponent, name);
        LOG_HL_CLIENT_API3(LOG_API_RAMSESOBJECT_PTR_STRING(animatedProperty), LOG_API_RAMSESOBJECT_STRING(propertyOwner), propertyComponent, name);
        return animatedProperty;
    }

    uint32_t AnimationSystem::getNumberOfFinishedAnimationsSincePreviousUpdate() const
    {
        return impl.getNumberOfFinishedAnimationsSincePreviousUpdate();
    }

    const Animation* AnimationSystem::getFinishedAnimationSincePreviousUpdate(uint32_t index) const
    {
        return impl.getFinishedAnimationSincePreviousUpdate(index);
    }

    Animation* AnimationSystem::getFinishedAnimationSincePreviousUpdate(uint32_t index)
    {
        return impl.getFinishedAnimationSincePreviousUpdate(index);
    }

    const RamsesObject* AnimationSystem::findObjectByName(const char* name) const
    {
        return impl.findObjectByName(name);
    }

    RamsesObject* AnimationSystem::findObjectByName(const char* name)
    {
        return impl.findObjectByName(name);
    }
}
