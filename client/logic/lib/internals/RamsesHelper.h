//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-logic/EPropertyType.h"
#include "ramses-framework-api/EDataType.h"
#include <optional>

namespace ramses::internal
{
    static constexpr std::optional<EPropertyType> ConvertRamsesUniformTypeToPropertyType(ramses::EDataType type)
    {
        switch (type)
        {
        case ramses::EDataType::Int32:
            return EPropertyType::Int32;
        case ramses::EDataType::Float:
            return EPropertyType::Float;
        case ramses::EDataType::Vector2F:
            return EPropertyType::Vec2f;
        case ramses::EDataType::Vector3F:
            return EPropertyType::Vec3f;
        case ramses::EDataType::Vector4F:
            return EPropertyType::Vec4f;
        case ramses::EDataType::Vector2I:
            return EPropertyType::Vec2i;
        case ramses::EDataType::Vector3I:
            return EPropertyType::Vec3i;
        case ramses::EDataType::Vector4I:
            return EPropertyType::Vec4i;

        // unsupported property types
        case ramses::EDataType::UInt16:
        case ramses::EDataType::UInt32:
        case ramses::EDataType::Matrix22F:
        case ramses::EDataType::Matrix33F:
        case ramses::EDataType::Matrix44F:
        case ramses::EDataType::TextureSampler2D:
        case ramses::EDataType::TextureSampler3D:
        case ramses::EDataType::TextureSamplerCube:
        case ramses::EDataType::TextureSampler2DMS:
        case ramses::EDataType::TextureSamplerExternal:
        case ramses::EDataType::ByteBlob:
            return std::nullopt;
        }

        assert(false);
        return std::nullopt;
    }
}
