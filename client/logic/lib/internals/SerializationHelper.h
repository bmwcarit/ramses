//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-logic/EPropertyType.h"

#include "generated/PropertyGen.h"

#include <optional>

namespace rlogic::internal
{
    static std::optional<EPropertyType> ConvertSerializationTypeToEPropertyType(rlogic_serialization::EPropertyRootType propertyRootType, rlogic_serialization::PropertyValue valueType)
    {
        switch (propertyRootType)
        {
        case rlogic_serialization::EPropertyRootType::Struct:
            return EPropertyType::Struct;
        case rlogic_serialization::EPropertyRootType::Array:
            return EPropertyType::Array;
        case rlogic_serialization::EPropertyRootType::Primitive:
            switch (valueType)
            {
            case rlogic_serialization::PropertyValue::bool_s:
                return EPropertyType::Bool;
            case rlogic_serialization::PropertyValue::float_s:
                return EPropertyType::Float;
            case rlogic_serialization::PropertyValue::vec2f_s:
                return EPropertyType::Vec2f;
            case rlogic_serialization::PropertyValue::vec3f_s:
                return EPropertyType::Vec3f;
            case rlogic_serialization::PropertyValue::vec4f_s:
                return EPropertyType::Vec4f;
            case rlogic_serialization::PropertyValue::int32_s:
                return EPropertyType::Int32;
            case rlogic_serialization::PropertyValue::int64_s:
                return EPropertyType::Int64;
            case rlogic_serialization::PropertyValue::vec2i_s:
                return EPropertyType::Vec2i;
            case rlogic_serialization::PropertyValue::vec3i_s:
                return EPropertyType::Vec3i;
            case rlogic_serialization::PropertyValue::vec4i_s:
                return EPropertyType::Vec4i;
            case rlogic_serialization::PropertyValue::string_s:
                return EPropertyType::String;
            case rlogic_serialization::PropertyValue::NONE:
                return std::nullopt;
            }
        }
        return std::nullopt;
    }
}
