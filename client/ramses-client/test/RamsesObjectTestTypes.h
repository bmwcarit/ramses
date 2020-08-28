//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESOBJECTTESTTYPES_H
#define RAMSES_RAMSESOBJECTTESTTYPES_H

#include <gtest/gtest.h>

namespace ramses
{
    class RamsesClient;
    class Scene;
    class Appearance;
    class Camera;
    class RemoteCamera;
    class PerspectiveCamera;
    class OrthographicCamera;
    class Appearance;
    class Node;
    class Effect;
    class MeshNode;
    class AnimationSystem;
    class AnimationSystemRealTime;
    class ArrayBuffer;
    class Texture2DBuffer;
    class GeometryBinding;
    class RenderGroup;
    class RenderPass;
    class RenderBuffer;
    class RenderTarget;
    class DataFloat;
    class DataVector2f;
    class DataVector3f;
    class DataVector4f;
    class DataMatrix22f;
    class DataMatrix33f;
    class DataMatrix44f;
    class DataInt32;
    class DataVector2i;
    class DataVector3i;
    class DataVector4i;
    class BlitPass;
    class TextureSampler;
    class StreamTexture;
    class Texture2D;
    class Texture3D;
    class TextureCube;
    class AnimatedProperty;
    class Animation;
    class AnimationSequence;
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
    class PickableObject;
    class SceneReference;
    class ArrayResource;

    // Objects derived from Node class
    typedef ::testing::Types
        <
        Node,
        MeshNode,
        RemoteCamera,
        PerspectiveCamera,
        OrthographicCamera,
        PickableObject
        > NodeTypes;

    // Objects derived from Resource class
    typedef ::testing::Types
        <
        ArrayResource,
        Texture2D,
        Texture3D,
        TextureCube,
        Effect
        > ResourceTypes;

    // Objects owned by Scene
    typedef ::testing::Types
        <
        AnimationSystem,
        AnimationSystemRealTime,
        Node,
        MeshNode,
        RemoteCamera,
        PerspectiveCamera,
        OrthographicCamera,
        Appearance,
        GeometryBinding,
        RenderGroup,
        RenderPass,
        BlitPass,
        TextureSampler,
        RenderBuffer,
        RenderTarget,
        DataFloat,
        DataVector2f,
        DataVector3f,
        DataVector4f,
        DataMatrix22f,
        DataMatrix33f,
        DataMatrix44f,
        DataInt32,
        DataVector2i,
        DataVector3i,
        DataVector4i,
        StreamTexture,
        ArrayBuffer,
        Texture2DBuffer,
        PickableObject,
        SceneReference,
        Texture2D,
        Texture3D,
        TextureCube,
        ArrayResource,
        Effect
        > SceneObjectTypes;

    // Objects owned by AnimationSystem
    typedef ::testing::Types
        <
        AnimatedProperty,
        Animation,
        AnimationSequence,
        SplineStepBool,
        SplineStepFloat,
        SplineStepInt32,
        SplineStepVector2f,
        SplineStepVector3f,
        SplineStepVector4f,
        SplineStepVector2i,
        SplineStepVector3i,
        SplineStepVector4i,
        SplineLinearFloat,
        SplineLinearInt32,
        SplineLinearVector2f,
        SplineLinearVector3f,
        SplineLinearVector4f,
        SplineLinearVector2i,
        SplineLinearVector3i,
        SplineLinearVector4i,
        SplineBezierFloat,
        SplineBezierInt32,
        SplineBezierVector2f,
        SplineBezierVector3f,
        SplineBezierVector4f,
        SplineBezierVector2i,
        SplineBezierVector3i,
        SplineBezierVector4i
        > AnimationObjectTypes;

    // Objects owned by client
    typedef ::testing::Types
        <
        Scene
        > ClientObjectTypes;

    // All Ramses objects - set #1
    typedef ::testing::Types
        <
        RamsesClient,
        Scene,
        AnimationSystem,
        AnimationSystemRealTime,
        Node,
        MeshNode,
        RemoteCamera,
        PerspectiveCamera,
        OrthographicCamera,
        Effect,
        AnimatedProperty,
        Animation,
        AnimationSequence,
        Appearance,
        SplineStepBool,
        SplineStepFloat,
        SplineStepInt32,
        SplineStepVector2f,
        SplineStepVector3f,
        SplineStepVector4f,
        SplineStepVector2i,
        SplineStepVector3i,
        SplineStepVector4i,
        SplineLinearFloat,
        SplineLinearInt32,
        SplineLinearVector2f,
        SplineLinearVector3f,
        SplineLinearVector4f,
        SplineLinearVector2i,
        SplineLinearVector3i,
        SplineBezierFloat,
        SplineBezierInt32,
        SplineBezierVector2f,
        SplineBezierVector3f,
        SplineBezierVector4f,
        SplineBezierVector2i,
        SplineBezierVector3i,
        SplineBezierVector4i,
        Texture2D,
        Texture3D,
        TextureCube,
        StreamTexture
        > RamsesObjectTypes1;

    // All Ramses objects - set #2
    typedef ::testing::Types
        <
        ArrayResource,
        RenderGroup,
        RenderPass,
        BlitPass,
        TextureSampler,
        RenderBuffer,
        RenderTarget,
        ArrayBuffer,
        Texture2DBuffer,
        GeometryBinding,
        DataFloat,
        DataVector2f,
        DataVector3f,
        DataVector4f,
        DataMatrix22f,
        DataMatrix33f,
        DataMatrix44f,
        DataInt32,
        DataVector2i,
        DataVector3i,
        DataVector4i,
        PickableObject,
        SceneReference
        > RamsesObjectTypes2;

    // Spline types
    typedef ::testing::Types
        <
        SplineStepBool,
        SplineStepFloat,
        SplineStepInt32,
        SplineStepVector2f,
        SplineStepVector3f,
        SplineStepVector4f,
        SplineStepVector2i,
        SplineStepVector3i,
        SplineStepVector4i,
        SplineLinearFloat,
        SplineLinearInt32,
        SplineLinearVector2f,
        SplineLinearVector3f,
        SplineLinearVector4f,
        SplineLinearVector2i,
        SplineLinearVector3i,
        SplineLinearVector4i,
        SplineBezierFloat,
        SplineBezierInt32,
        SplineBezierVector2f,
        SplineBezierVector3f,
        SplineBezierVector4f,
        SplineBezierVector2i,
        SplineBezierVector3i,
        SplineBezierVector4i
        > SplineTypes;
}

#endif
