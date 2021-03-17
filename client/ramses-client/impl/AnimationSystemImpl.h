//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONSYSTEMIMPL_H
#define RAMSES_ANIMATIONSYSTEMIMPL_H

// internal
#include "SceneObjectImpl.h"
#include "AnimationSystemData.h"

// framework
#include "Animation/AnimationStateChangeCollector.h"

namespace ramses_internal
{
    class IAnimationSystem;
}

namespace ramses
{
    class Spline;
    class SplineStepBool;
    class SplineStepInt32;
    class SplineStepFloat;
    class SplineStepVector2f;
    class SplineStepVector3f;
    class SplineStepVector4f;
    class SplineStepVector2i;
    class SplineStepVector3i;
    class SplineStepVector4i;
    class SplineLinearInt32;
    class SplineLinearFloat;
    class SplineLinearVector2f;
    class SplineLinearVector3f;
    class SplineLinearVector4f;
    class SplineLinearVector2i;
    class SplineLinearVector3i;
    class SplineLinearVector4i;
    class SplineBezierInt32;
    class SplineBezierFloat;
    class SplineBezierVector2f;
    class SplineBezierVector3f;
    class SplineBezierVector4f;
    class SplineBezierVector2i;
    class SplineBezierVector3i;
    class SplineBezierVector4i;
    class AnimatedProperty;
    class Animation;
    class AnimationSequence;
    class SplineImpl;
    class AnimationImpl;
    class SceneImpl;
    class UniformInput;
    class DataObject;
    class Appearance;
    class NodeImpl;

    class AnimationSystemImpl final : public SceneObjectImpl
    {
    public:
        AnimationSystemImpl(SceneImpl& sceneImpl, ERamsesObjectType type, const char* name);
        virtual ~AnimationSystemImpl() override;

        void             initializeFrameworkData(ramses_internal::IAnimationSystem& animationSystem);
        virtual void     deinitializeFrameworkData() override;
        virtual status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        virtual status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;
        virtual status_t validate() const override;

        status_t          setTime(globalTimeStamp_t timeStamp);
        globalTimeStamp_t getTime() const;
        status_t          updateLocalTime(globalTimeStamp_t systemTime);

        SplineStepBool*         createSplineStepBool(const char* name);
        SplineStepInt32*        createSplineStepInt32(const char* name);
        SplineStepFloat*        createSplineStepFloat(const char* name);
        SplineStepVector2f*     createSplineStepVector2f(const char* name);
        SplineStepVector3f*     createSplineStepVector3f(const char* name);
        SplineStepVector4f*     createSplineStepVector4f(const char* name);
        SplineStepVector2i*     createSplineStepVector2i(const char* name);
        SplineStepVector3i*     createSplineStepVector3i(const char* name);
        SplineStepVector4i*     createSplineStepVector4i(const char* name);
        SplineLinearInt32*      createSplineLinearInt32(const char* name);
        SplineLinearFloat*      createSplineLinearFloat(const char* name);
        SplineLinearVector2f*   createSplineLinearVector2f(const char* name);
        SplineLinearVector3f*   createSplineLinearVector3f(const char* name);
        SplineLinearVector4f*   createSplineLinearVector4f(const char* name);
        SplineLinearVector2i*   createSplineLinearVector2i(const char* name);
        SplineLinearVector3i*   createSplineLinearVector3i(const char* name);
        SplineLinearVector4i*   createSplineLinearVector4i(const char* name);
        SplineBezierInt32*      createSplineBezierInt32(const char* name);
        SplineBezierFloat*      createSplineBezierFloat(const char* name);
        SplineBezierVector2f*   createSplineBezierVector2f(const char* name);
        SplineBezierVector3f*   createSplineBezierVector3f(const char* name);
        SplineBezierVector4f*   createSplineBezierVector4f(const char* name);
        SplineBezierVector2i*   createSplineBezierVector2i(const char* name);
        SplineBezierVector3i*   createSplineBezierVector3i(const char* name);
        SplineBezierVector4i*   createSplineBezierVector4i(const char* name);

        Animation*         createAnimation(const AnimatedProperty& animatedProperty, const Spline& spline, const char* name);
        AnimationSequence* createAnimationSequence(const char* name);

        AnimatedProperty* createAnimatedProperty(const NodeImpl& propertyOwner, EAnimatedProperty property, EAnimatedPropertyComponent propertyComponent, const char* name);
        AnimatedProperty* createAnimatedProperty(const UniformInput& propertyOwner, const Appearance& appearance, EAnimatedPropertyComponent propertyComponent, const char* name);
        AnimatedProperty* createAnimatedProperty(const DataObject& propertyOwner, EAnimatedPropertyComponent propertyComponent, const char* name);

        status_t destroy(AnimationObject& animationObject);

        ramses_internal::IAnimationSystem&        getIAnimationSystem();
        ramses_internal::AnimationSystemHandle getAnimationSystemHandle() const;

        bool                      containsAnimationObject(const AnimationObjectImpl& object) const;
        const RamsesObject*       findObjectByName(const char* name) const;
        RamsesObject*             findObjectByName(const char* name);
        const RamsesObjectRegistry& getObjectRegistry() const;

        uint32_t         getNumberOfFinishedAnimationsSincePreviousUpdate() const;
        const Animation* getFinishedAnimationSincePreviousUpdate(uint32_t index) const;
        Animation*       getFinishedAnimationSincePreviousUpdate(uint32_t index);

    private:
        const Animation* findAnimationByHandle(ramses_internal::AnimationHandle handle) const;

        static bool GetDataBindIDForDataType(ramses_internal::EDataType dataType, ramses_internal::TDataBindID& bindId);

        ramses_internal::IAnimationSystem*             m_animationSystem;
        ramses_internal::AnimationSystemHandle         m_animationSystemHandle;
        ramses_internal::AnimationStateChangeCollector m_animationStateCollector;

        AnimationSystemData m_data;
    };
}

#endif
