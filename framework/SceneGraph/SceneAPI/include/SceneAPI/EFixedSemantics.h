//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EFFECT_EFIXEDSEMANTICS_H
#define RAMSES_EFFECT_EFIXEDSEMANTICS_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "SceneAPI/EDataType.h"

namespace ramses_internal
{
    enum EFixedSemantics
    {
        // Camera data properties
        EFixedSemantics_RendererViewMatrix = 1,
        EFixedSemantics_CameraViewMatrix,
        EFixedSemantics_ProjectionMatrix,
        EFixedSemantics_ViewMatrix,               // EFixedSemantics_RendererViewMatrix * EFixedSemantics_CameraViewMatrix

        EFixedSemantics_ModelMatrix,
        EFixedSemantics_ModelViewMatrix,
        EFixedSemantics_ModelViewMatrix33,
        EFixedSemantics_ModelViewProjectionMatrix,
        EFixedSemantics_NormalMatrix,

        EFixedSemantics_CameraWorldPosition,

        // Global data
        EFixedSemantics_RendererScreenResX,
        EFixedSemantics_RendererScreenResY,
        EFixedSemantics_RendererScreenResolution,

        EFixedSemantics_Indices,

        // Vertex attributes
        EFixedSemantics_VertexPositionAttribute,
        EFixedSemantics_VertexNormalAttribute,
        EFixedSemantics_VertexTexCoordAttribute,
        EFixedSemantics_VertexBinormalAttribute,
        EFixedSemantics_VertexTangentAttribute,
        EFixedSemantics_VertexColorAttribute,
        EFixedSemantics_VertexCustomAttribute,

        // Fragment data
        EFixedSemantics_VertexPositionOutput,
        EFixedSemantics_FragmentPositionInput,

        // Text specific
        EFixedSemantics_TextPositionsAttribute,
        EFixedSemantics_TextTextureCoordinatesAttribute,
        EFixedSemantics_TextTextureUniform,

        EFixedSemantics_Invalid,

        EFixedSemantics_Count // must be last, used for checking for dynamic semantics
    };

    inline const Char* EnumToString(EFixedSemantics semantics)
    {
        switch(semantics)
        {
        case EFixedSemantics_RendererViewMatrix:
            return "EFixedSemantics_RendererViewMatrix";
        case EFixedSemantics_CameraViewMatrix:
            return "EFixedSemantics_CameraViewMatrix";
        case EFixedSemantics_ProjectionMatrix:
            return "EFixedSemantics_CameraProjectionMatrix";
        case EFixedSemantics_ViewMatrix:
            return "EFixedSemantics_ViewMatrix";
        case EFixedSemantics_CameraWorldPosition:
            return "EFixedSemantics_CameraWorldPosition";
        case EFixedSemantics_ModelMatrix:
            return "EFixedSemantics_MeshModelMatrix";
        case EFixedSemantics_ModelViewMatrix:
            return "EFIXEDSEMANTICS_MODEL_VIEW_MATRIX";
        case EFixedSemantics_ModelViewMatrix33:
            return "EFIXEDSEMANTICS_MODEL_VIEW_MATRIX33";
        case EFixedSemantics_ModelViewProjectionMatrix:
            return "EFIXEDSEMANTICS_MODEL_VIEW_PROJECTION_MATRIX";
        case EFixedSemantics_NormalMatrix:
            return "EFIXEDSEMANTICS_NORMAL_MATRIX";
        case EFixedSemantics_RendererScreenResX:
            return "EFIXEDSEMANTICS_RENDERER_SCREEN_RESX";
        case EFixedSemantics_RendererScreenResY:
            return "EFixedSemantics_RendererScreenResY";
        case EFixedSemantics_Indices:
            return "EFixedSemantics_Indices";
        case EFixedSemantics_VertexPositionAttribute:
            return "EFixedSemantics_VertexPositionAttribute";
        case EFixedSemantics_VertexNormalAttribute:
            return "EFixedSemantics_VertexNormalAttribute";
        case EFixedSemantics_VertexTexCoordAttribute:
            return "EFixedSemantics_VertexTexCoordAttribute";
        case EFixedSemantics_VertexBinormalAttribute:
            return "EFixedSemantics_VertexBinormalAttribute";
        case EFixedSemantics_VertexTangentAttribute:
            return "EFixedSemantics_VertexTangentAttribute";
        case EFixedSemantics_VertexColorAttribute:
            return "EFixedSemantics_VertexColorAttribute";
        case EFixedSemantics_VertexCustomAttribute:
            return "EFixedSemantics_VertexCustomAttribute";
        case EFixedSemantics_RendererScreenResolution:
            return "EFixedSemantics_RendererScreenResolution";
        case EFixedSemantics_FragmentPositionInput:
            return "EFixedSemantics_FragmentPositionInput";
        case EFixedSemantics_VertexPositionOutput:
            return "EFixedSemantics_VertexPositionOutput";
        case EFixedSemantics_TextPositionsAttribute:
            return "EFixedSemantics_TextPositionsAttribute";
        case EFixedSemantics_TextTextureCoordinatesAttribute:
            return "EFixedSemantics_TextTextureCoordinatesAttribute";
        case EFixedSemantics_TextTextureUniform:
            return "EFixedSemantics_TextTextureUniform";
        default:
            return "UNKNOWN_SEMANTICS";
        }
    }

    inline bool IsSemanticCompatibleWithDataType(EFixedSemantics semantics, EDataType dataType)
    {
        switch (semantics)
        {
        case EFixedSemantics_RendererViewMatrix:
        case EFixedSemantics_CameraViewMatrix:
        case EFixedSemantics_ProjectionMatrix:
        case EFixedSemantics_ViewMatrix:
        case EFixedSemantics_ModelMatrix:
        case EFixedSemantics_ModelViewMatrix:
        case EFixedSemantics_ModelViewProjectionMatrix:
        case EFixedSemantics_NormalMatrix:
            return dataType == EDataType_Matrix44F;
        case EFixedSemantics_ModelViewMatrix33:
            return dataType == EDataType_Matrix33F;
        case EFixedSemantics_RendererScreenResX:
        case EFixedSemantics_RendererScreenResY:
            return dataType == EDataType_Float;
        case EFixedSemantics_Indices:
            return dataType == EDataType_UInt16
                || dataType == EDataType_UInt32;
        case EFixedSemantics_TextPositionsAttribute:
            return dataType == EDataType_Vector2F;
        case EFixedSemantics_TextTextureCoordinatesAttribute:
            return dataType == EDataType_Vector2F;
        case EFixedSemantics_VertexPositionAttribute:
        case EFixedSemantics_VertexNormalAttribute:
        case EFixedSemantics_VertexTexCoordAttribute:
        case EFixedSemantics_VertexBinormalAttribute:
        case EFixedSemantics_VertexTangentAttribute:
        case EFixedSemantics_VertexColorAttribute:
        case EFixedSemantics_VertexCustomAttribute:
            return dataType == EDataType_Float
                || dataType == EDataType_Vector2F
                || dataType == EDataType_Vector3F
                || dataType == EDataType_Vector4F;
        case EFixedSemantics_VertexPositionOutput:
        case EFixedSemantics_FragmentPositionInput:
            return dataType == EDataType_Vector4F;
        case EFixedSemantics_CameraWorldPosition:
            return dataType == EDataType_Vector3F;
        case EFixedSemantics_RendererScreenResolution:
            return dataType == EDataType_Vector2F;
        case EFixedSemantics_TextTextureUniform:
            return dataType == EDataType_TextureSampler;
        case EFixedSemantics_Invalid:
            return false;
        default:
            assert(0);
            return false;
        }
    }
}

#endif
