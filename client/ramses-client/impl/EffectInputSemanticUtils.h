//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EFFECTINPUTSEMANTICUTILS_H
#define RAMSES_EFFECTINPUTSEMANTICUTILS_H

#include "ramses-client-api/EffectInputSemantic.h"
#include "SceneAPI/EFixedSemantics.h"
#include <assert.h>

namespace ramses
{
    class EffectInputSemanticUtils
    {
    public:
        static ramses_internal::EFixedSemantics GetEffectInputSemanticInternal(EEffectUniformSemantic semanticType)
        {
            switch (semanticType)
            {
            case EEffectUniformSemantic_ProjectionMatrix:
                return ramses_internal::EFixedSemantics_ProjectionMatrix;
            case EEffectUniformSemantic_ModelMatrix:
                return ramses_internal::EFixedSemantics_ModelMatrix;
            case EEffectUniformSemantic_RendererViewMatrix:
                return ramses_internal::EFixedSemantics_RendererViewMatrix;
            case EEffectUniformSemantic_CameraViewMatrix:
                return ramses_internal::EFixedSemantics_CameraViewMatrix;
            case EEffectUniformSemantic_CameraWorldPosition:
                return ramses_internal::EFixedSemantics_CameraWorldPosition;
            case EEffectUniformSemantic_ViewMatrix:
                return ramses_internal::EFixedSemantics_ViewMatrix;
            case EEffectUniformSemantic_ModelViewMatrix:
                return ramses_internal::EFixedSemantics_ModelViewMatrix;
            case EEffectUniformSemantic_ModelViewMatrix33:
                return ramses_internal::EFixedSemantics_ModelViewMatrix33;
            case EEffectUniformSemantic_ModelViewProjectionMatrix:
                return ramses_internal::EFixedSemantics_ModelViewProjectionMatrix;
            case EEffectUniformSemantic_NormalMatrix:
                return ramses_internal::EFixedSemantics_NormalMatrix;
            case EEffectUniformSemantic_RendererScreenResolution:
                return ramses_internal::EFixedSemantics_RendererScreenResolution;
            case EEffectUniformSemantic_TextTexture:
                return ramses_internal::EFixedSemantics_TextTextureUniform;
            default:
                assert(false);
                return ramses_internal::EFixedSemantics_Invalid;
            }
        }

        static EEffectUniformSemantic GetEffectUniformSemanticFromInternal(ramses_internal::EFixedSemantics semanticType)
        {
            switch (semanticType)
            {
            case ramses_internal::EFixedSemantics_ProjectionMatrix:
                return EEffectUniformSemantic_ProjectionMatrix;
            case ramses_internal::EFixedSemantics_ModelMatrix:
                return EEffectUniformSemantic_ModelMatrix;
            case ramses_internal::EFixedSemantics_RendererViewMatrix:
                return EEffectUniformSemantic_RendererViewMatrix;
            case ramses_internal::EFixedSemantics_CameraViewMatrix:
                return EEffectUniformSemantic_CameraViewMatrix;
            case ramses_internal::EFixedSemantics_CameraWorldPosition:
                return EEffectUniformSemantic_CameraWorldPosition;
            case ramses_internal::EFixedSemantics_ViewMatrix:
                return EEffectUniformSemantic_ViewMatrix;
            case ramses_internal::EFixedSemantics_ModelViewMatrix:
                return EEffectUniformSemantic_ModelViewMatrix;
            case ramses_internal::EFixedSemantics_ModelViewMatrix33:
                return EEffectUniformSemantic_ModelViewMatrix33;
            case ramses_internal::EFixedSemantics_ModelViewProjectionMatrix:
                return EEffectUniformSemantic_ModelViewProjectionMatrix;
            case ramses_internal::EFixedSemantics_RendererScreenResolution:
                return EEffectUniformSemantic_RendererScreenResolution;
            case ramses_internal::EFixedSemantics_NormalMatrix:
                return EEffectUniformSemantic_NormalMatrix;
            case ramses_internal::EFixedSemantics_TextTextureUniform:
                return EEffectUniformSemantic_TextTexture;
            case ramses_internal::EFixedSemantics_Invalid:
                return EEffectUniformSemantic_Invalid;
            default:
                return EEffectUniformSemantic_Invalid;
            }
        }

        static ramses_internal::EFixedSemantics GetEffectInputSemanticInternal(EEffectAttributeSemantic semanticType)
        {
            switch (semanticType)
            {
            case EEffectAttributeSemantic_TextPositions:
                return ramses_internal::EFixedSemantics_TextPositionsAttribute;
            case EEffectAttributeSemantic_TextTextureCoordinates:
                return ramses_internal::EFixedSemantics_TextTextureCoordinatesAttribute;
            default:
                return ramses_internal::EFixedSemantics_Invalid;
            }
        }

        static EEffectAttributeSemantic GetEffectAttributeSemanticFromInternal(ramses_internal::EFixedSemantics semanticType)
        {
            switch (semanticType)
            {
            case ramses_internal::EFixedSemantics_TextPositionsAttribute:
                return EEffectAttributeSemantic_TextPositions;
            case ramses_internal::EFixedSemantics_TextTextureCoordinatesAttribute:
                return EEffectAttributeSemantic_TextTextureCoordinates;
            case ramses_internal::EFixedSemantics_Invalid:
                return EEffectAttributeSemantic_Invalid;
            default:
                return EEffectAttributeSemantic_Invalid;
            }
        }
    };
}

#endif
