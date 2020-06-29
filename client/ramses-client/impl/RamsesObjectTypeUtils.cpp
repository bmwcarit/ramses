//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RamsesObjectTypeUtils.h"
#include "Utils/LoggingUtils.h"

namespace ramses
{
    const char* RamsesObjectTypeNames[] =
    {
        "ERamsesObjectType_Invalid",
        "ERamsesObjectType_RamsesObject",
        "ERamsesObjectType_ClientObject",
        "ERamsesObjectType_SceneObject",
        "ERamsesObjectType_AnimationObject",
        "ERamsesObjectType_Client",
        "ERamsesObjectType_Scene",
        "ERamsesObjectType_AnimationSystem",
        "ERamsesObjectType_AnimationSystemRealTime",
        "ERamsesObjectType_Node",
        "ERamsesObjectType_MeshNode",
        "ERamsesObjectType_Camera",
        "ERamsesObjectType_RemoteCamera",
        "ERamsesObjectType_LocalCamera",
        "ERamsesObjectType_PerspectiveCamera",
        "ERamsesObjectType_OrthographicCamera",
        "ERamsesObjectType_Effect",
        "ERamsesObjectType_AnimatedProperty",
        "ERamsesObjectType_Animation",
        "ERamsesObjectType_AnimationSequence",
        "ERamsesObjectType_Appearance",
        "ERamsesObjectType_Geometry",
        "ERamsesObjectType_PickableObject",
        "ERamsesObjectType_Spline",
        "ERamsesObjectType_SplineStepBool",
        "ERamsesObjectType_SplineStepFloat",
        "ERamsesObjectType_SplineStepInt32",
        "ERamsesObjectType_SplineStepVector2f",
        "ERamsesObjectType_SplineStepVector3f",
        "ERamsesObjectType_SplineStepVector4f",
        "ERamsesObjectType_SplineStepVector2i",
        "ERamsesObjectType_SplineStepVector3i",
        "ERamsesObjectType_SplineStepVector4i",
        "ERamsesObjectType_SplineLinearFloat",
        "ERamsesObjectType_SplineLinearInt32",
        "ERamsesObjectType_SplineLinearVector2f",
        "ERamsesObjectType_SplineLinearVector3f",
        "ERamsesObjectType_SplineLinearVector4f",
        "ERamsesObjectType_SplineLinearVector2i",
        "ERamsesObjectType_SplineLinearVector3i",
        "ERamsesObjectType_SplineLinearVector4i",
        "ERamsesObjectType_SplineBezierFloat",
        "ERamsesObjectType_SplineBezierInt32",
        "ERamsesObjectType_SplineBezierVector2f",
        "ERamsesObjectType_SplineBezierVector3f",
        "ERamsesObjectType_SplineBezierVector4f",
        "ERamsesObjectType_SplineBezierVector2i",
        "ERamsesObjectType_SplineBezierVector3i",
        "ERamsesObjectType_SplineBezierVector4i",
        "ERamsesObjectType_Resource",
        "ERamsesObjectType_Texture2D",
        "ERamsesObjectType_Texture3D",
        "ERamsesObjectType_TextureCube",
        "ERamsesObjectType_UInt16Array",
        "ERamsesObjectType_UInt32Array",
        "ERamsesObjectType_FloatArray",
        "ERamsesObjectType_Vector2fArray",
        "ERamsesObjectType_Vector2iArray",
        "ERamsesObjectType_Vector3fArray",
        "ERamsesObjectType_Vector3iArray",
        "ERamsesObjectType_Vector4fArray",
        "ERamsesObjectType_Vector4iArray",
        "ERamsesObjectType_RenderGroup",
        "ERamsesObjectType_RenderPass",
        "ERamsesObjectType_BlitPass",
        "ERamsesObjectType_TextureSampler",
        "ERamsesObjectType_RenderBuffer",
        "ERamsesObjectType_RenderTarget",
        "ERamsesObjectType_IndexDataBuffer",
        "ERamsesObjectType_VertexDataBuffer",
        "ERamsesObjectType_Texture2DBuffer",
        "ERamsesObjectType_DataObject",
        "ERamsesObjectType_DataFloat",
        "ERamsesObjectType_DataVector2f",
        "ERamsesObjectType_DataVector3f",
        "ERamsesObjectType_DataVector4f",
        "ERamsesObjectType_DataMatrix22f",
        "ERamsesObjectType_DataMatrix33f",
        "ERamsesObjectType_DataMatrix44f",
        "ERamsesObjectType_DataInt32",
        "ERamsesObjectType_DataVector2i",
        "ERamsesObjectType_DataVector3i",
        "ERamsesObjectType_DataVector4i",
        "ERamsesObjectType_StreamTexture",
        "ERamsesObjectType_SceneReference"
    };

    ENUM_TO_STRING(ERamsesObjectType, RamsesObjectTypeNames, ERamsesObjectType_NUMBER_OF_TYPES);

    static_assert(ERamsesObjectType_NUMBER_OF_TYPES == (sizeof(RamsesObjectTraits) / sizeof(RamsesObjectTraits[0])), "Every RamsesObject type must register its traits!");

    const char* RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType type)
    {
        return EnumToString(type);
    }

    bool RamsesObjectTypeUtils::IsTypeMatchingBaseType(ERamsesObjectType type, ERamsesObjectType baseType)
    {
        while (type != ERamsesObjectType_Invalid)
        {
            if (type == baseType)
            {
                return true;
            }
            assert(RamsesObjectTraits[type].typeID == type && "Wrong order of RamsesObject traits!");
            type = RamsesObjectTraits[type].baseClassTypeID;
        }

        return false;
    }

    bool RamsesObjectTypeUtils::IsConcreteType(ERamsesObjectType type)
    {
        assert(RamsesObjectTraits[type].typeID == type && "Wrong order of RamsesObject traits!");
        return RamsesObjectTraits[type].isConcreteType;
    }
}
