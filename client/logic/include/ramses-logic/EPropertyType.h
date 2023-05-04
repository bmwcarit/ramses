//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-framework-api/APIExport.h"
#include "ramses-framework-api/DataTypes.h"
#include <array>
#include <string>
#include <vector>
#include <cassert>

namespace ramses
{
    /**
    * #EPropertyType lists the types of properties created and managed by the #ramses::LogicNode
    * class and its derivates.
    */
    enum class EPropertyType : int
    {
        Float,  ///< corresponds to float
        Vec2f,  ///< corresponds to [float, float]
        Vec3f,  ///< corresponds to [float, float, float]
        Vec4f,  ///< corresponds to [float, float, float, float]
        Int32,  ///< corresponds to int32_t
        Int64,  ///< corresponds to int64_t (note that Lua cannot handle 64bit integers in full range)
        Vec2i,  ///< corresponds to [int32_t, int32_t]
        Vec3i,  ///< corresponds to [int32_t, int32_t, int32_t]
        Vec4i,  ///< corresponds to [int32_t, int32_t, int32_t, int32_t]
        Struct, ///< Has no value itself, but can have named child properties
        String, ///< corresponds to std::string
        Bool,   ///< corresponds to bool
        Array   ///< Has no value itself, but can have unnamed child properties of homogeneous types (primitive or structs).
                ///< When used in #ramses::DataArray this type refers to an array of float arrays (std::vector<float> is the element type).
    };

    /**
    * Type trait which converts C++ types to #ramses::EPropertyType enum for primitive types.
    */
    template <typename T> struct PropertyTypeToEnum;

    template <> struct PropertyTypeToEnum<float>
    {
        static const EPropertyType TYPE = EPropertyType::Float;
    };

    template <> struct PropertyTypeToEnum<vec2f>
    {
        static const EPropertyType TYPE = EPropertyType::Vec2f;
    };

    template <> struct PropertyTypeToEnum<vec3f>
    {
        static const EPropertyType TYPE = EPropertyType::Vec3f;
    };

    template <> struct PropertyTypeToEnum<vec4f>
    {
        static const EPropertyType TYPE = EPropertyType::Vec4f;
    };

    template <> struct PropertyTypeToEnum<int32_t>
    {
        static const EPropertyType TYPE = EPropertyType::Int32;
    };

    template <> struct PropertyTypeToEnum<int64_t>
    {
        static const EPropertyType TYPE = EPropertyType::Int64;
    };

    template <> struct PropertyTypeToEnum<vec2i>
    {
        static const EPropertyType TYPE = EPropertyType::Vec2i;
    };

    template <> struct PropertyTypeToEnum<vec3i>
    {
        static const EPropertyType TYPE = EPropertyType::Vec3i;
    };

    template <> struct PropertyTypeToEnum<vec4i>
    {
        static const EPropertyType TYPE = EPropertyType::Vec4i;
    };

    template <> struct PropertyTypeToEnum<std::string>
    {
        static const EPropertyType TYPE = EPropertyType::String;
    };

    template <> struct PropertyTypeToEnum<bool>
    {
        static const EPropertyType TYPE = EPropertyType::Bool;
    };

    template <> struct PropertyTypeToEnum<std::vector<float>>
    {
        static const EPropertyType TYPE = EPropertyType::Array;
    };

    /**
    * Type trait which converts #ramses::EPropertyType enum to a C++ type.
    */
    template <EPropertyType T> struct PropertyEnumToType;

    template <> struct PropertyEnumToType<EPropertyType::Float>
    {
        using TYPE = float;
    };

    template <> struct PropertyEnumToType<EPropertyType::Vec2f>
    {
        using TYPE = vec2f;
    };

    template <> struct PropertyEnumToType<EPropertyType::Vec3f>
    {
        using TYPE = vec3f;
    };

    template <> struct PropertyEnumToType<EPropertyType::Vec4f>
    {
        using TYPE = vec4f;
    };

    template <> struct PropertyEnumToType<EPropertyType::Int32>
    {
        using TYPE = int32_t;
    };

    template <> struct PropertyEnumToType<EPropertyType::Int64>
    {
        using TYPE = int64_t;
    };

    template <> struct PropertyEnumToType<EPropertyType::Vec2i>
    {
        using TYPE = vec2i;
    };

    template <> struct PropertyEnumToType<EPropertyType::Vec3i>
    {
        using TYPE = vec3i;
    };

    template <> struct PropertyEnumToType<EPropertyType::Vec4i>
    {
        using TYPE = vec4i;
    };

    template <> struct PropertyEnumToType<EPropertyType::String>
    {
        using TYPE = std::string;
    };

    template <> struct PropertyEnumToType<EPropertyType::Bool>
    {
        using TYPE = bool;
    };

    template <> struct PropertyEnumToType<EPropertyType::Array>
    {
        using TYPE = std::vector<float>;
    };

    /**
    * Type trait which can be used to check if a type is primitive or not.
    * "primitive" in this context means the type can be used as a template for
    * #ramses::Property::set() and #ramses::Property::get(), i.e. it has a
    * value which can be directly set/obtained, as opposed to non-primitive types
    * like structs or arrays which don't have a singular settable value.
    */
    template <typename T> struct IsPrimitiveProperty
    {
        static const bool value = false;
    };

    template <> struct IsPrimitiveProperty<float>
    {
        static const bool value = true;
    };

    template <> struct IsPrimitiveProperty<vec2f>
    {
        static const bool value = true;
    };

    template <> struct IsPrimitiveProperty<vec3f>
    {
        static const bool value = true;
    };

    template <> struct IsPrimitiveProperty<vec4f>
    {
        static const bool value = true;
    };

    template <> struct IsPrimitiveProperty<int32_t>
    {
        static const bool value = true;
    };

    template <> struct IsPrimitiveProperty<int64_t>
    {
        static const bool value = true;
    };

    template <> struct IsPrimitiveProperty<vec2i>
    {
        static const bool value = true;
    };

    template <> struct IsPrimitiveProperty<vec3i>
    {
        static const bool value = true;
    };

    template <> struct IsPrimitiveProperty<vec4i>
    {
        static const bool value = true;
    };

    template <> struct IsPrimitiveProperty<std::string>
    {
        static const bool value = true;
    };

    template <> struct IsPrimitiveProperty<bool>
    {
        static const bool value = true;
    };

    /**
    * Helper to determine if given property type can be stored in a #ramses::DataArray.
    */
    constexpr bool CanPropertyTypeBeStoredInDataArray(EPropertyType type)
    {
        switch (type)
        {
        case EPropertyType::Float:
        case EPropertyType::Vec2f:
        case EPropertyType::Vec3f:
        case EPropertyType::Vec4f:
        case EPropertyType::Int32:
        case EPropertyType::Vec2i:
        case EPropertyType::Vec3i:
        case EPropertyType::Vec4i:
        /**
            Arrays can be nested in other arrays, but only if their element type is Float
        */
        case EPropertyType::Array:
            return true;
        case EPropertyType::Bool:
        case EPropertyType::Struct:
        case EPropertyType::String:
        case EPropertyType::Int64:
            return false;
        }

        return false;
    }

    /**
    * Helper to determine if given property type can be animated using #ramses::AnimationNode.
    */
    constexpr bool CanPropertyTypeBeAnimated(EPropertyType type)
    {
        // all data types that can be stored in DataArray can be animated
        return CanPropertyTypeBeStoredInDataArray(type);
    }

    /**
    * Returns the string representation of a property type. This string corresponds to the syntax
    * that has to be used in the Lua source code used to create scripts with properties with
    * the corresponding type. If a type is declared as Type:T() in Lua, this function will return 'T' as string
    */
    constexpr const char* GetLuaPrimitiveTypeName(EPropertyType type)
    {
        switch (type)
        {
        case EPropertyType::Float:
            return "Float";
        case EPropertyType::Vec2f:
            return "Vec2f";
        case EPropertyType::Vec3f:
            return "Vec3f";
        case EPropertyType::Vec4f:
            return "Vec4f";
        case EPropertyType::Int32:
            return "Int32";
        case EPropertyType::Int64:
            return "Int64";
        case EPropertyType::Vec2i:
            return "Vec2i";
        case EPropertyType::Vec3i:
            return "Vec3i";
        case EPropertyType::Vec4i:
            return "Vec4i";
        case EPropertyType::Struct:
            return "Struct";
        case EPropertyType::String:
            return "String";
        case EPropertyType::Bool:
            return "Bool";
        case EPropertyType::Array:
            return "Array";
        }
        return "Struct";
    }

    constexpr size_t MaxArrayPropertySize = 255u;
}
