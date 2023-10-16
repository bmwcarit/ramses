//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/EffectInputSemantic.h"
#include "internal/SceneGraph/SceneAPI/EFixedSemantics.h"
#include <cassert>

namespace ramses
{
    class EffectInputSemanticUtils
    {
    public:
        static ramses::internal::EFixedSemantics GetEffectInputSemanticInternal(EEffectUniformSemantic semanticType)
        {
            switch (semanticType)
            {
            case EEffectUniformSemantic::ProjectionMatrix:
                return ramses::internal::EFixedSemantics::ProjectionMatrix;
            case EEffectUniformSemantic::ModelMatrix:
                return ramses::internal::EFixedSemantics::ModelMatrix;
            case EEffectUniformSemantic::CameraWorldPosition:
                return ramses::internal::EFixedSemantics::CameraWorldPosition;
            case EEffectUniformSemantic::ViewMatrix:
                return ramses::internal::EFixedSemantics::ViewMatrix;
            case EEffectUniformSemantic::ModelViewMatrix:
                return ramses::internal::EFixedSemantics::ModelViewMatrix;
            case EEffectUniformSemantic::ModelViewMatrix33:
                return ramses::internal::EFixedSemantics::ModelViewMatrix33;
            case EEffectUniformSemantic::ModelViewProjectionMatrix:
                return ramses::internal::EFixedSemantics::ModelViewProjectionMatrix;
            case EEffectUniformSemantic::NormalMatrix:
                return ramses::internal::EFixedSemantics::NormalMatrix;
            case EEffectUniformSemantic::DisplayBufferResolution:
                return ramses::internal::EFixedSemantics::DisplayBufferResolution;
            case EEffectUniformSemantic::TextTexture:
                return ramses::internal::EFixedSemantics::TextTexture;
            case EEffectUniformSemantic::TimeMs:
                return ramses::internal::EFixedSemantics::TimeMs;
            case EEffectUniformSemantic::Invalid:
                return ramses::internal::EFixedSemantics::Invalid;
            }

            assert(false);
            return ramses::internal::EFixedSemantics::Invalid;
        }

        static EEffectUniformSemantic GetEffectUniformSemanticFromInternal(ramses::internal::EFixedSemantics semanticType)
        {
            switch (semanticType)
            {
            case ramses::internal::EFixedSemantics::ProjectionMatrix:
                return EEffectUniformSemantic::ProjectionMatrix;
            case ramses::internal::EFixedSemantics::ModelMatrix:
                return EEffectUniformSemantic::ModelMatrix;
            case ramses::internal::EFixedSemantics::CameraWorldPosition:
                return EEffectUniformSemantic::CameraWorldPosition;
            case ramses::internal::EFixedSemantics::ViewMatrix:
                return EEffectUniformSemantic::ViewMatrix;
            case ramses::internal::EFixedSemantics::ModelViewMatrix:
                return EEffectUniformSemantic::ModelViewMatrix;
            case ramses::internal::EFixedSemantics::ModelViewMatrix33:
                return EEffectUniformSemantic::ModelViewMatrix33;
            case ramses::internal::EFixedSemantics::ModelViewProjectionMatrix:
                return EEffectUniformSemantic::ModelViewProjectionMatrix;
            case ramses::internal::EFixedSemantics::DisplayBufferResolution:
                return EEffectUniformSemantic::DisplayBufferResolution;
            case ramses::internal::EFixedSemantics::NormalMatrix:
                return EEffectUniformSemantic::NormalMatrix;
            case ramses::internal::EFixedSemantics::TextTexture:
                return EEffectUniformSemantic::TextTexture;
            case ramses::internal::EFixedSemantics::TimeMs:
                return EEffectUniformSemantic::TimeMs;
            default:
                return EEffectUniformSemantic::Invalid;
            }

            return EEffectUniformSemantic::Invalid;
        }

        static ramses::internal::EFixedSemantics GetEffectInputSemanticInternal(EEffectAttributeSemantic semanticType)
        {
            switch (semanticType)
            {
            case EEffectAttributeSemantic::TextPositions:
                return ramses::internal::EFixedSemantics::TextPositionsAttribute;
            case EEffectAttributeSemantic::TextTextureCoordinates:
                return ramses::internal::EFixedSemantics::TextTextureCoordinatesAttribute;
            case EEffectAttributeSemantic::Invalid:
                return ramses::internal::EFixedSemantics::Invalid;
            }

            assert(false);
            return ramses::internal::EFixedSemantics::Invalid;
        }

        static EEffectAttributeSemantic GetEffectAttributeSemanticFromInternal(ramses::internal::EFixedSemantics semanticType)
        {
            switch (semanticType)
            {
            case ramses::internal::EFixedSemantics::TextPositionsAttribute:
                return EEffectAttributeSemantic::TextPositions;
            case ramses::internal::EFixedSemantics::TextTextureCoordinatesAttribute:
                return EEffectAttributeSemantic::TextTextureCoordinates;
            default:
                return EEffectAttributeSemantic::Invalid;
            }
        }
    };
}
