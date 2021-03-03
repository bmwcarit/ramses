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
#include <cassert>

namespace ramses
{
    class EffectInputSemanticUtils
    {
    public:
        static ramses_internal::EFixedSemantics GetEffectInputSemanticInternal(EEffectUniformSemantic semanticType)
        {
            switch (semanticType)
            {
            case EEffectUniformSemantic::ProjectionMatrix:
                return ramses_internal::EFixedSemantics::ProjectionMatrix;
            case EEffectUniformSemantic::ModelMatrix:
                return ramses_internal::EFixedSemantics::ModelMatrix;
            case EEffectUniformSemantic::CameraWorldPosition:
                return ramses_internal::EFixedSemantics::CameraWorldPosition;
            case EEffectUniformSemantic::ViewMatrix:
                return ramses_internal::EFixedSemantics::ViewMatrix;
            case EEffectUniformSemantic::ModelViewMatrix:
                return ramses_internal::EFixedSemantics::ModelViewMatrix;
            case EEffectUniformSemantic::ModelViewMatrix33:
                return ramses_internal::EFixedSemantics::ModelViewMatrix33;
            case EEffectUniformSemantic::ModelViewProjectionMatrix:
                return ramses_internal::EFixedSemantics::ModelViewProjectionMatrix;
            case EEffectUniformSemantic::NormalMatrix:
                return ramses_internal::EFixedSemantics::NormalMatrix;
            case EEffectUniformSemantic::DisplayBufferResolution:
                return ramses_internal::EFixedSemantics::DisplayBufferResolution;
            case EEffectUniformSemantic::TextTexture:
                return ramses_internal::EFixedSemantics::TextTexture;
            case EEffectUniformSemantic::Invalid:
                return ramses_internal::EFixedSemantics::Invalid;
            }

            assert(false);
            return ramses_internal::EFixedSemantics::Invalid;
        }

        static EEffectUniformSemantic GetEffectUniformSemanticFromInternal(ramses_internal::EFixedSemantics semanticType)
        {
            switch (semanticType)
            {
            case ramses_internal::EFixedSemantics::ProjectionMatrix:
                return EEffectUniformSemantic::ProjectionMatrix;
            case ramses_internal::EFixedSemantics::ModelMatrix:
                return EEffectUniformSemantic::ModelMatrix;
            case ramses_internal::EFixedSemantics::CameraWorldPosition:
                return EEffectUniformSemantic::CameraWorldPosition;
            case ramses_internal::EFixedSemantics::ViewMatrix:
                return EEffectUniformSemantic::ViewMatrix;
            case ramses_internal::EFixedSemantics::ModelViewMatrix:
                return EEffectUniformSemantic::ModelViewMatrix;
            case ramses_internal::EFixedSemantics::ModelViewMatrix33:
                return EEffectUniformSemantic::ModelViewMatrix33;
            case ramses_internal::EFixedSemantics::ModelViewProjectionMatrix:
                return EEffectUniformSemantic::ModelViewProjectionMatrix;
            case ramses_internal::EFixedSemantics::DisplayBufferResolution:
                return EEffectUniformSemantic::DisplayBufferResolution;
            case ramses_internal::EFixedSemantics::NormalMatrix:
                return EEffectUniformSemantic::NormalMatrix;
            case ramses_internal::EFixedSemantics::TextTexture:
                return EEffectUniformSemantic::TextTexture;
            default:
                return EEffectUniformSemantic::Invalid;
            }

            return EEffectUniformSemantic::Invalid;
        }

        static ramses_internal::EFixedSemantics GetEffectInputSemanticInternal(EEffectAttributeSemantic semanticType)
        {
            switch (semanticType)
            {
            case EEffectAttributeSemantic::TextPositions:
                return ramses_internal::EFixedSemantics::TextPositionsAttribute;
            case EEffectAttributeSemantic::TextTextureCoordinates:
                return ramses_internal::EFixedSemantics::TextTextureCoordinatesAttribute;
            case EEffectAttributeSemantic::Invalid:
                return ramses_internal::EFixedSemantics::Invalid;
            }

            assert(false);
            return ramses_internal::EFixedSemantics::Invalid;
        }

        static EEffectAttributeSemantic GetEffectAttributeSemanticFromInternal(ramses_internal::EFixedSemantics semanticType)
        {
            switch (semanticType)
            {
            case ramses_internal::EFixedSemantics::TextPositionsAttribute:
                return EEffectAttributeSemantic::TextPositions;
            case ramses_internal::EFixedSemantics::TextTextureCoordinatesAttribute:
                return EEffectAttributeSemantic::TextTextureCoordinates;
            default:
                return EEffectAttributeSemantic::Invalid;
            }
        }
    };
}

#endif
