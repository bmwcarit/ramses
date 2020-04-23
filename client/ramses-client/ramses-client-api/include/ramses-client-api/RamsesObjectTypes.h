//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESOBJECTTYPES_H
#define RAMSES_RAMSESOBJECTTYPES_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"

namespace ramses
{
    /// RamsesObject type ID
    enum ERamsesObjectType
    {
        ERamsesObjectType_Invalid = 0,
        ERamsesObjectType_ClientObject, // base type
        ERamsesObjectType_RamsesObject, // base type
        ERamsesObjectType_SceneObject, // base type
        ERamsesObjectType_AnimationObject, // base type
        ERamsesObjectType_Client,
        ERamsesObjectType_Scene,
        ERamsesObjectType_AnimationSystem, // base type and concrete type
        ERamsesObjectType_AnimationSystemRealTime,
        ERamsesObjectType_Node, // base type
        ERamsesObjectType_MeshNode,
        ERamsesObjectType_Camera,
        ERamsesObjectType_RemoteCamera,
        ERamsesObjectType_LocalCamera,
        ERamsesObjectType_PerspectiveCamera,
        ERamsesObjectType_OrthographicCamera,
        ERamsesObjectType_Effect,
        ERamsesObjectType_AnimatedProperty,
        ERamsesObjectType_Animation,
        ERamsesObjectType_AnimationSequence,
        ERamsesObjectType_AnimatedSetter,
        ERamsesObjectType_Appearance,
        ERamsesObjectType_GeometryBinding,
        ERamsesObjectType_PickableObject,
        ERamsesObjectType_Spline, // base type
        ERamsesObjectType_SplineStepBool,
        ERamsesObjectType_SplineStepFloat,
        ERamsesObjectType_SplineStepInt32,
        ERamsesObjectType_SplineStepVector2f,
        ERamsesObjectType_SplineStepVector3f,
        ERamsesObjectType_SplineStepVector4f,
        ERamsesObjectType_SplineStepVector2i,
        ERamsesObjectType_SplineStepVector3i,
        ERamsesObjectType_SplineStepVector4i,
        ERamsesObjectType_SplineLinearFloat,
        ERamsesObjectType_SplineLinearInt32,
        ERamsesObjectType_SplineLinearVector2f,
        ERamsesObjectType_SplineLinearVector3f,
        ERamsesObjectType_SplineLinearVector4f,
        ERamsesObjectType_SplineLinearVector2i,
        ERamsesObjectType_SplineLinearVector3i,
        ERamsesObjectType_SplineLinearVector4i,
        ERamsesObjectType_SplineBezierFloat,
        ERamsesObjectType_SplineBezierInt32,
        ERamsesObjectType_SplineBezierVector2f,
        ERamsesObjectType_SplineBezierVector3f,
        ERamsesObjectType_SplineBezierVector4f,
        ERamsesObjectType_SplineBezierVector2i,
        ERamsesObjectType_SplineBezierVector3i,
        ERamsesObjectType_SplineBezierVector4i,
        ERamsesObjectType_Resource, // base type
        ERamsesObjectType_Texture2D,
        ERamsesObjectType_Texture3D,
        ERamsesObjectType_TextureCube,
        ERamsesObjectType_UInt16Array,
        ERamsesObjectType_UInt32Array,
        ERamsesObjectType_FloatArray,
        ERamsesObjectType_Vector2fArray,
        ERamsesObjectType_Vector2iArray,
        ERamsesObjectType_Vector3fArray,
        ERamsesObjectType_Vector3iArray,
        ERamsesObjectType_Vector4fArray,
        ERamsesObjectType_Vector4iArray,
        ERamsesObjectType_RenderGroup,
        ERamsesObjectType_RenderPass,
        ERamsesObjectType_BlitPass,
        ERamsesObjectType_TextureSampler,
        ERamsesObjectType_RenderBuffer,
        ERamsesObjectType_RenderTarget,
        ERamsesObjectType_IndexDataBuffer,
        ERamsesObjectType_VertexDataBuffer,
        ERamsesObjectType_Texture2DBuffer,
        ERamsesObjectType_DataObject, // base type
        ERamsesObjectType_DataFloat,
        ERamsesObjectType_DataVector2f,
        ERamsesObjectType_DataVector3f,
        ERamsesObjectType_DataVector4f,
        ERamsesObjectType_DataMatrix22f,
        ERamsesObjectType_DataMatrix33f,
        ERamsesObjectType_DataMatrix44f,
        ERamsesObjectType_DataInt32,
        ERamsesObjectType_DataVector2i,
        ERamsesObjectType_DataVector3i,
        ERamsesObjectType_DataVector4i,
        ERamsesObjectType_StreamTexture,
        ERamsesObjectType_SceneReference,

        // Whenever new type of object is added
        // its traits must be registered in RamsesObjectTypeTraits.h using helper macros
        // and added to appropriate test type list(s) in RamsesObjectTestTypes.h
        // and added a conversion template instantiation in RamsesObjectTypeUtils.cpp
        ERamsesObjectType_NUMBER_OF_TYPES
    };
}

#endif
