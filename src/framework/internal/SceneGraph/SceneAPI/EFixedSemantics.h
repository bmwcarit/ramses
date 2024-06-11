//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/EDataType.h"
#include "internal/SceneGraph/SceneAPI/SceneTypes.h"
#include "internal/Core/Utils/LoggingUtils.h"

#include <cstdint>
#include <unordered_map>
#include <variant>

namespace ramses::internal
{
    enum class EFixedSemantics
    {
        Invalid = 0,

        ProjectionMatrix,
        ViewMatrix,
        ModelMatrix,
        ModelViewMatrix,
        ModelViewMatrix33,
        ModelViewProjectionMatrix,
        NormalMatrix,
        CameraWorldPosition,
        DisplayBufferResolution,

        // Used to identify indices in geometry data fields
        Indices,

        // Text specific (used on client side only)
        TextTexture,
        TextPositionsAttribute,
        TextTextureCoordinatesAttribute,
        TimeMs,

        ModelBlock,
        CameraBlock,
        ModelCameraBlock,
        FramebufferBlock,
        SceneBlock,
    };

    using SemanticsMap = std::unordered_map<std::variant<std::string, UniformBufferBinding>, EFixedSemantics>;

    const std::array EFixedSemanticsNames =
    {
        "Invalid",
        "ProjectionMatrix",
        "ViewMatrix",
        "ModelMatrix",
        "ModelViewMatrix",
        "ModelViewMatrix33",
        "ModelViewProjectionMatrix",
        "NormalMatrix",
        "CameraWorldPosition",
        "DisplayBufferResolution",
        "Indices",
        "TextTexture",
        "TextPositionsAttribute",
        "TextTextureCoordinatesAttribute",
        "TimeMs",
        "ModelBlock",
        "CameraBlock",
        "ModelCameraBlock",
        "FramebufferBlock",
        "SceneBlock",
    };

    inline bool IsSemanticCompatibleWithDataType(EFixedSemantics semantics, EDataType dataType)
    {
        switch (semantics)
        {
        case EFixedSemantics::ModelBlock:
        case EFixedSemantics::CameraBlock:
        case EFixedSemantics::ModelCameraBlock:
        case EFixedSemantics::FramebufferBlock:
        case EFixedSemantics::SceneBlock:
            return dataType == EDataType::UniformBuffer;
        case EFixedSemantics::ProjectionMatrix:
        case EFixedSemantics::ViewMatrix:
        case EFixedSemantics::ModelMatrix:
        case EFixedSemantics::ModelViewMatrix:
        case EFixedSemantics::ModelViewProjectionMatrix:
        case EFixedSemantics::NormalMatrix:
            return dataType == EDataType::Matrix44F;
        case EFixedSemantics::ModelViewMatrix33:
            return dataType == EDataType::Matrix33F;
        case EFixedSemantics::Indices:
            return dataType == EDataType::UInt16
                || dataType == EDataType::UInt32;
        case EFixedSemantics::CameraWorldPosition:
            return dataType == EDataType::Vector3F;
        case EFixedSemantics::DisplayBufferResolution:
            return dataType == EDataType::Vector2F;
        case EFixedSemantics::TextTexture:
            return dataType == EDataType::TextureSampler2D;
        case EFixedSemantics::TextPositionsAttribute:
        case EFixedSemantics::TextTextureCoordinatesAttribute:
            return dataType == EDataType::Vector2F;
        case EFixedSemantics::TimeMs:
            return dataType == EDataType::Int32;
        case EFixedSemantics::Invalid:
            return false;
        }

        assert(false);
        return false;
    }
}

MAKE_ENUM_CLASS_PRINTABLE(ramses::internal::EFixedSemantics,
                                        "EFixedSemantics",
                                        ramses::internal::EFixedSemanticsNames,
                                        ramses::internal::EFixedSemantics::SceneBlock);
