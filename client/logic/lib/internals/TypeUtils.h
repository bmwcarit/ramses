//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-logic/Property.h"
#include "impl/PropertyImpl.h"
#include "ramses-framework-api/DataTypes.h"
#include <cassert>
#include <vector>

namespace ramses::internal
{
    class TypeUtils
    {
    public:
        // Enum classes should not require range checks, but EPropertyType is marshalled from Lua and from files
        // this method should be used to check if the enum is in range
        static bool IsValidType(EPropertyType type)
        {
            switch (type)
            {
            case EPropertyType::Float:
            case EPropertyType::Vec2f:
            case EPropertyType::Vec3f:
            case EPropertyType::Vec4f:
            case EPropertyType::Int32:
            case EPropertyType::Int64:
            case EPropertyType::Vec2i:
            case EPropertyType::Vec3i:
            case EPropertyType::Vec4i:
            case EPropertyType::String:
            case EPropertyType::Bool:
            case EPropertyType::Struct:
            case EPropertyType::Array:
                return true;
            default:
                return false;
            }
        }

        static bool IsPrimitiveType(EPropertyType type)
        {
            assert(IsValidType(type));

            switch (type)
            {
            case EPropertyType::Float:
            case EPropertyType::Vec2f:
            case EPropertyType::Vec3f:
            case EPropertyType::Vec4f:
            case EPropertyType::Int32:
            case EPropertyType::Int64:
            case EPropertyType::Vec2i:
            case EPropertyType::Vec3i:
            case EPropertyType::Vec4i:
            case EPropertyType::String:
            case EPropertyType::Bool:
                return true;
            case EPropertyType::Struct:
            case EPropertyType::Array:
            default:
                return false;
            }
        }

        static bool IsPrimitiveVectorType(EPropertyType type)
        {
            assert(IsValidType(type));

            switch (type)
            {
            case EPropertyType::Vec2f:
            case EPropertyType::Vec3f:
            case EPropertyType::Vec4f:
            case EPropertyType::Vec2i:
            case EPropertyType::Vec3i:
            case EPropertyType::Vec4i:
                return true;
            case EPropertyType::Float:
            case EPropertyType::Int32:
            case EPropertyType::Int64:
            case EPropertyType::String:
            case EPropertyType::Bool:
            case EPropertyType::Struct:
            case EPropertyType::Array:
            default:
                return false;
            }
        }

        // This method is for better readability in code
        static bool CanHaveChildren(EPropertyType type)
        {
            return !IsPrimitiveType(type);
        }

        static size_t ComponentsSizeForPropertyType(EPropertyType propertyType)
        {
            switch (propertyType)
            {
            case EPropertyType::Float:
            case EPropertyType::Int32:
            case EPropertyType::Int64:
                return 1u;
            case EPropertyType::Vec2f:
            case EPropertyType::Vec2i:
                return 2u;
            case EPropertyType::Vec3f:
            case EPropertyType::Vec3i:
                return 3u;
            case EPropertyType::Vec4f:
            case EPropertyType::Vec4i:
                return 4u;
            case EPropertyType::String:
            case EPropertyType::Array:
            case EPropertyType::Struct:
            case EPropertyType::Bool:
                assert(false && "This should never happen");
            }
            return 0u;
        }
    };

    /// TODO vaclav temporary till these are unified
    template <typename T> struct RlogicTypeToRamsesType  { using TYPE = T; };
    template <> struct RlogicTypeToRamsesType<vec2f> { using TYPE = ramses::vec2f; };
    template <> struct RlogicTypeToRamsesType<vec3f> { using TYPE = ramses::vec3f; };
    template <> struct RlogicTypeToRamsesType<vec4f> { using TYPE = ramses::vec4f; };
    template <> struct RlogicTypeToRamsesType<vec2i> { using TYPE = ramses::vec2i; };
    template <> struct RlogicTypeToRamsesType<vec3i> { using TYPE = ramses::vec3i; };
    template <> struct RlogicTypeToRamsesType<vec4i> { using TYPE = ramses::vec4i; };
}
