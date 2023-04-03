//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-logic/EPropertyType.h"

#include "ramses-client-api/EffectInputDataType.h"

#include <optional>

namespace rlogic::internal
{
    static constexpr std::optional<EPropertyType> ConvertRamsesUniformTypeToPropertyType(ramses::EEffectInputDataType type)
    {
        switch (type)
        {
        case ramses::EEffectInputDataType::EEffectInputDataType_Invalid:
            return std::nullopt;
        case ramses::EEffectInputDataType::EEffectInputDataType_Int32:
            return std::make_optional(EPropertyType::Int32);
        case ramses::EEffectInputDataType::EEffectInputDataType_UInt16:
        case ramses::EEffectInputDataType::EEffectInputDataType_UInt32:
            return std::nullopt;
        case ramses::EEffectInputDataType::EEffectInputDataType_Float:
            return std::make_optional(EPropertyType::Float);
        case ramses::EEffectInputDataType::EEffectInputDataType_Vector2F:
            return std::make_optional(EPropertyType::Vec2f);
        case ramses::EEffectInputDataType::EEffectInputDataType_Vector3F:
            return std::make_optional(EPropertyType::Vec3f);
        case ramses::EEffectInputDataType::EEffectInputDataType_Vector4F:
            return std::make_optional(EPropertyType::Vec4f);
        case ramses::EEffectInputDataType::EEffectInputDataType_Vector2I:
            return std::make_optional(EPropertyType::Vec2i);
        case ramses::EEffectInputDataType::EEffectInputDataType_Vector3I:
            return std::make_optional(EPropertyType::Vec3i);
        case ramses::EEffectInputDataType::EEffectInputDataType_Vector4I:
            return EPropertyType::Vec4i;
        case ramses::EEffectInputDataType::EEffectInputDataType_Matrix22F:
        case ramses::EEffectInputDataType::EEffectInputDataType_Matrix33F:
        case ramses::EEffectInputDataType::EEffectInputDataType_Matrix44F:
        case ramses::EEffectInputDataType::EEffectInputDataType_TextureSampler2D:
        case ramses::EEffectInputDataType::EEffectInputDataType_TextureSampler3D:
        case ramses::EEffectInputDataType::EEffectInputDataType_TextureSamplerCube:
        case ramses::EEffectInputDataType::EEffectInputDataType_TextureSampler2DMS:
        case ramses::EEffectInputDataType::EEffectInputDataType_TextureSamplerExternal:
            return std::nullopt;
        }
        return std::nullopt;
    }
}
