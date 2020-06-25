//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONSYSTEM_H
#define RAMSES_ANIMATIONSYSTEM_H

#include "ramses-client-api/AnimationTypes.h"
#include "ramses-client-api/SceneObject.h"
#include "ramses-client-api/AnimatedProperty.h"

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
    class Animation;
    class AnimationSequence;
    class Node;
    class UniformInput;
    class DataObject;
    class Appearance;
    class AnimationObject;
    class PickableObject;

    /**
    * @brief The AnimationSystem holds all animation related data.
    */
    class RAMSES_API AnimationSystem : public SceneObject
    {
    public:
        /**
        * @brief Sets the animation system to a given time.
        * Any unsigned integral values that are used in an incrementing fashion
        * can be used. This time stamp is distributed to renderer and can be used
        * as a synchronization point.
        *
        * @param[in] timeStamp The global time stamp to be set in animation system
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setTime(globalTimeStamp_t timeStamp);

        /**
        * @brief Gets the current animation system time.
        * The time stamp retrieved is the time stamp that was previously set by calling setTime.
        *
        * @return Current time stamp of the animation system
        */
        globalTimeStamp_t getTime() const;

        /**
        * @brief Creates a spline in this animation system using bool data type and step interpolation
        *
        * @param[in] name The optional name of the spline
        * @return Pointer to the created spline, null on failure
        */
        SplineStepBool* createSplineStepBool(const char* name = nullptr);

        /**
        * @brief Creates a spline in this animation system using int32_t data type and step interpolation
        *
        * @param[in] name The optional name of the spline
        * @return Pointer to the created spline, null on failure
        */
        SplineStepInt32* createSplineStepInt32(const char* name = nullptr);

        /**
        * @brief Creates a spline in this animation system using float data type and step interpolation
        *
        * @param[in] name The optional name of the spline
        * @return Pointer to the created spline, null on failure
        */
        SplineStepFloat* createSplineStepFloat(const char* name = nullptr);

        /**
        * @brief Creates a spline in this animation system using Vector2f data type and step interpolation
        *
        * @param[in] name The optional name of the spline
        * @return Pointer to the created spline, null on failure
        */
        SplineStepVector2f* createSplineStepVector2f(const char* name = nullptr);

        /**
        * @brief Creates a spline in this animation system using Vector3f data type and step interpolation
        *
        * @param[in] name The optional name of the spline
        * @return Pointer to the created spline, null on failure
        */
        SplineStepVector3f* createSplineStepVector3f(const char* name = nullptr);

        /**
        * @brief Creates a spline in this animation system using Vector4f data type and step interpolation
        *
        * @param[in] name The optional name of the spline
        * @return Pointer to the created spline, null on failure
        */
        SplineStepVector4f* createSplineStepVector4f(const char* name = nullptr);

        /**
        * @brief Creates a spline in this animation system using Vector2i data type and step interpolation
        *
        * @param[in] name The optional name of the spline
        * @return Pointer to the created spline, null on failure
        */
        SplineStepVector2i* createSplineStepVector2i(const char* name = nullptr);

        /**
        * @brief Creates a spline in this animation system using Vector3i data type and step interpolation
        *
        * @param[in] name The optional name of the spline
        * @return Pointer to the created spline, null on failure
        */
        SplineStepVector3i* createSplineStepVector3i(const char* name = nullptr);

        /**
        * @brief Creates a spline in this animation system using Vector4i data type and step interpolation
        *
        * @param[in] name The optional name of the spline
        * @return Pointer to the created spline, null on failure
        */
        SplineStepVector4i* createSplineStepVector4i(const char* name = nullptr);

        /**
        * @brief Creates a spline in this animation system using int32_t data type and linear interpolation
        *
        * @param[in] name The optional name of the spline
        * @return Pointer to the created spline, null on failure
        */
        SplineLinearInt32* createSplineLinearInt32(const char* name = nullptr);

        /**
        * @brief Creates a spline in this animation system using float data type and linear interpolation
        *
        * @param[in] name The optional name of the spline
        * @return Pointer to the created spline, null on failure
        */
        SplineLinearFloat* createSplineLinearFloat(const char* name = nullptr);

        /**
        * @brief Creates a spline in this animation system using Vector2f data type and linear interpolation
        *
        * @param[in] name The optional name of the spline
        * @return Pointer to the created spline, null on failure
        */
        SplineLinearVector2f* createSplineLinearVector2f(const char* name = nullptr);

        /**
        * @brief Creates a spline in this animation system using Vector3f data type and linear interpolation
        *
        * @param[in] name The optional name of the spline
        * @return Pointer to the created spline, null on failure
        */
        SplineLinearVector3f* createSplineLinearVector3f(const char* name = nullptr);

        /**
        * @brief Creates a spline in this animation system using Vector4f data type and linear interpolation
        *
        * @param[in] name The optional name of the spline
        * @return Pointer to the created spline, null on failure
        */
        SplineLinearVector4f* createSplineLinearVector4f(const char* name = nullptr);

        /**
        * @brief Creates a spline in this animation system using Vector2i data type and linear interpolation
        *
        * @param[in] name The optional name of the spline
        * @return Pointer to the created spline, null on failure
        */
        SplineLinearVector2i* createSplineLinearVector2i(const char* name = nullptr);

        /**
        * @brief Creates a spline in this animation system using Vector3i data type and linear interpolation
        *
        * @param[in] name The optional name of the spline
        * @return Pointer to the created spline, null on failure
        */
        SplineLinearVector3i* createSplineLinearVector3i(const char* name = nullptr);

        /**
        * @brief Creates a spline in this animation system using Vector4i data type and linear interpolation
        *
        * @param[in] name The optional name of the spline
        * @return Pointer to the created spline, null on failure
        */
        SplineLinearVector4i* createSplineLinearVector4i(const char* name = nullptr);

        /**
        * @brief Creates a spline in this animation system using int32_t data type and Bezier interpolation
        *
        * @param[in] name The optional name of the spline
        * @return Pointer to the created spline, null on failure
        */
        SplineBezierInt32* createSplineBezierInt32(const char* name = nullptr);

        /**
        * @brief Creates a spline in this animation system using float data type and Bezier interpolation
        *
        * @param[in] name The optional name of the spline
        * @return Pointer to the created spline, null on failure
        */
        SplineBezierFloat* createSplineBezierFloat(const char* name = nullptr);

        /**
        * @brief Creates a spline in this animation system using Vector2f data type and Bezier interpolation
        *
        * @param[in] name The optional name of the spline
        * @return Pointer to the created spline, null on failure
        */
        SplineBezierVector2f* createSplineBezierVector2f(const char* name = nullptr);

        /**
        * @brief Creates a spline in this animation system using Vector3f data type and Bezier interpolation
        *
        * @param[in] name The optional name of the spline
        * @return Pointer to the created spline, null on failure
        */
        SplineBezierVector3f* createSplineBezierVector3f(const char* name = nullptr);

        /**
        * @brief Creates a spline in this animation system using Vector4f data type and Bezier interpolation
        *
        * @param[in] name The optional name of the spline
        * @return Pointer to the created spline, null on failure
        */
        SplineBezierVector4f* createSplineBezierVector4f(const char* name = nullptr);

        /**
        * @brief Creates a spline in this animation system using Vector2i data type and Bezier interpolation
        *
        * @param[in] name The optional name of the spline
        * @return Pointer to the created spline, null on failure
        */
        SplineBezierVector2i* createSplineBezierVector2i(const char* name = nullptr);

        /**
        * @brief Creates a spline in this animation system using Vector3i data type and Bezier interpolation
        *
        * @param[in] name The optional name of the spline
        * @return Pointer to the created spline, null on failure
        */
        SplineBezierVector3i* createSplineBezierVector3i(const char* name = nullptr);

        /**
        * @brief Creates a spline in this animation system using Vector4i data type and Bezier interpolation
        *
        * @param[in] name The optional name of the spline
        * @return Pointer to the created spline, null on failure
        */
        SplineBezierVector4i* createSplineBezierVector4i(const char* name = nullptr);

        /**
        * @brief Creates Animation that can animate given property using given spline
        *
        * @param[in] animatedProperty AnimatedProperty to animate with this Animation
        * @param[in] spline Spline to be used for animation
        * @param[in] name The optional name of the Animation
        * @return Pointer to the created Animation, null on failure
        */
        Animation* createAnimation(const AnimatedProperty& animatedProperty, const Spline& spline, const char* name = nullptr);

        /**
        * @brief Creates AnimationSequence that can hold references to multiple animations and control them together.
        *
        * @param[in] name The optional name of the AnimationSequence
        * @return Pointer to the created AnimationSequence, null on failure
        */
        AnimationSequence* createAnimationSequence(const char* name = nullptr);

        /**
        * @brief Release an animation system object and its data.
        * The object must be owned by this animation system.
        * The reference to this object is no longer valid after it is destroyed.
        *
        * @param[in] animationObject Animation object to be released.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t destroy(AnimationObject& animationObject);

        /**
        * @brief Create a new animated property for Node
        *
        * @param[in] propertyOwner Reference to an entity that contains data to animate.
        * @param[in] property Property to animate.
        * @param[in] propertyComponent The optional component to animate in case of vector property.
        * @param[in] name The optional name of the AnimatedProperty.
        * @return AnimatedProperty referring to Node's property, null on failure
        */
        AnimatedProperty* createAnimatedProperty(const Node& propertyOwner, EAnimatedProperty property, EAnimatedPropertyComponent propertyComponent = EAnimatedPropertyComponent_All, const char* name = nullptr);

        /**
        * @brief Create a new animated property for Appearance's input
        *
        * @param[in] propertyOwner Reference to an entity that contains data to animate.
        * @param[in] appearance Reference to the appearance to which the input belongs.
        * @param[in] propertyComponent The optional component to animate in case of vector property.
        * @param[in] name The optional name of the AnimatedProperty.
        * @return AnimatedProperty referring to Appearance input, null on failure
        */
        AnimatedProperty* createAnimatedProperty(const UniformInput& propertyOwner, const Appearance& appearance, EAnimatedPropertyComponent propertyComponent = EAnimatedPropertyComponent_All, const char* name = nullptr);

        /**
        * @brief Create a new animated property for a DataObject
        *
        * @param[in] propertyOwner Reference to an entity that contains data to animate.
        * @param[in] propertyComponent The optional component to animate in case of vector property.
        * @param[in] name The optional name of the AnimatedProperty.
        * @return AnimatedProperty referring to DataObject, null on failure
        */
        AnimatedProperty* createAnimatedProperty(const DataObject& propertyOwner, EAnimatedPropertyComponent propertyComponent = EAnimatedPropertyComponent_All, const char* name = nullptr);

        /**
        * @brief Get number of animations that were finished in current update round.
        *        The animation system collects finished animations between
        *        current and previous setTime/updateLocalTime call.
        *
        * @return Number of finished animations in current update round.
        */
        uint32_t getNumberOfFinishedAnimationsSincePreviousUpdate() const;

        /**
        * @brief Get animation that was finished in current update round.
        *        The animation system collects finished animations between
        *        current and previous setTime/updateLocalTime call.
        *
        * @param[in] index Index of the finished animation.
        *                  Use getNumberOfFinishedAnimationsSincePreviousUpdate() to get the count.
        * @return Animation finished in current update round.
        */
        const Animation* getFinishedAnimationSincePreviousUpdate(uint32_t index) const;

        /**
        * @copydoc getFinishedAnimationSincePreviousUpdate(uint32_t index) const
        **/
        Animation* getFinishedAnimationSincePreviousUpdate(uint32_t index);

        /**
        * @brief Get an object from the animation system by name
        *
        * @param[in] name The name of the object to get.
        * @return Pointer to the object if found, nullptr otherwise.
        */
        const RamsesObject* findObjectByName(const char* name) const;

        /**
        * @copydoc findObjectByName(const char* name) const
        **/
        RamsesObject* findObjectByName(const char* name);

        /**
        * Stores internal data for implementation specifics of animation system.
        */
        class AnimationSystemImpl& impl;

    protected:
        /**
        * @brief SceneImpl is the factory for creating animation system instances.
        */
        friend class SceneImpl;

        /**
        * @brief Constructor of the animation system
        *
        * @param[in] pimpl Internal data for implementation specifics of AnimationSystem (sink - instance becomes owner)
        */
        explicit AnimationSystem(AnimationSystemImpl& pimpl);

        /**
        * @brief Copy constructor of animation system
        *
        * @param[in] other Other instance of animation system class
        */
        AnimationSystem(const AnimationSystem& other);

        /**
        * @brief Assignment operator of animation system.
        *
        * @param[in] other Other instance of animation system class
        * @return This instance after assignment
        */
        AnimationSystem& operator=(const AnimationSystem& other);

        /**
        * @brief Destructor of the animation system
        */
        virtual ~AnimationSystem();
    };
}

#endif
