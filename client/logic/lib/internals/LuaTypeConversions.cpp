//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internals/LuaTypeConversions.h"
#include "internals/SolHelper.h"

#include <cmath>

namespace rlogic::internal
{
    // extracts a plain Lua table, or one which was made read-only (e.g. module data tables)
    std::optional<sol::lua_table> LuaTypeConversions::ExtractLuaTable(const sol::object& object)
    {
        auto potentialTable = object.as<std::optional<sol::lua_table>>();

        // The missing else() statements below mean that if any of the conditions fail, the
        // result of the function will be the first cast (potentialTable)
        if (potentialTable)
        {
            // Identify read-only data (e.g. logic engine read-only module tables)
            // This is basically a reverse-test for this: https://www.lua.org/pil/13.4.4.html
            // See also LuaCompilationUtils::MakeTableReadOnly for the counterpart
            sol::lua_table table = *potentialTable;
            sol::object metaContent = table[sol::metatable_key];

            if (metaContent.valid())
            {
                auto potentialMetatable = metaContent.as<std::optional<sol::lua_table>>();
                if (potentialMetatable)
                {
                    sol::object metaIndex = (*potentialMetatable)[sol::meta_function::index];

                    if (metaIndex.valid())
                    {
                        auto potentialModuleTable = metaIndex.as<std::optional<sol::lua_table>>();
                        if (potentialModuleTable)
                        {
                            potentialTable = potentialModuleTable;
                        }
                    }
                }
            }
        }

        return potentialTable;
    }

    size_t LuaTypeConversions::GetMaxIndexForVectorType(rlogic::EPropertyType type)
    {
        switch (type)
        {
        case EPropertyType::Vec2i:
        case EPropertyType::Vec2f:
            return 2u;
        case EPropertyType::Vec3i:
        case EPropertyType::Vec3f:
            return 3u;
        case EPropertyType::Vec4f:
        case EPropertyType::Vec4i:
            return 4u;
        case EPropertyType::Struct:
        case EPropertyType::Array:
        case EPropertyType::Float:
        case EPropertyType::Int32:
        case EPropertyType::Int64:
        case EPropertyType::String:
        case EPropertyType::Bool:
            assert(false && "Should not ask about size of non-vector types!");
        }

        assert(false && "Vector type not handled!");
        return 0;
    }

    template <> DataOrError<float> LuaTypeConversions::ExtractSpecificType<float>(const sol::object& solObject)
    {
        if (!solObject.valid() || solObject.get_type() != sol::type::number)
        {
            return DataOrError<float>(
                fmt::format("Error while extracting floating point number: expected a number, received '{}'",
                    sol_helper::GetSolTypeName(solObject.get_type())));
        }

        // Extract Lua number (==double)
        const double asDouble = solObject.as<double>();

        // Integral part out of range of float type
        if (asDouble > std::numeric_limits<float>::max() || asDouble < std::numeric_limits<float>::lowest())
        {
            return DataOrError<float>(fmt::format("Error while extracting floating point number: value would cause overflow in float ('{}')", asDouble));
        }

        return DataOrError<float>(static_cast<float>(asDouble));
    }

    template <>
    DataOrError<int32_t> LuaTypeConversions::ExtractSpecificType<int32_t>(const sol::object& solObject)
    {
        if (!solObject.valid() || solObject.get_type() != sol::type::number)
        {
            return DataOrError<int32_t>(
                fmt::format("Error while extracting integer: expected a number, received '{}'",
                sol_helper::GetSolTypeName(solObject.get_type())));
        }

        // Get Lua number as double (internal format of Lua)
        const double asDouble = solObject.as<double>();
        // Rounds to closest signed integer
        const double rounded = std::round(asDouble);

        // fractional part too large -> rounding error
        const double fractPart = std::abs(asDouble - rounded);
        if (fractPart > std::numeric_limits<double>::epsilon())
        {
            return DataOrError<int32_t>(
                fmt::format("Error while extracting integer: implicit rounding (fractional part '{}' is not negligible)",
                fractPart));
        }

        // Integral part out of range
        if (rounded > std::numeric_limits<int32_t>::max() || rounded < std::numeric_limits<int32_t>::lowest())
        {
            return DataOrError<int32_t>(
                fmt::format("Error while extracting integer: integral part too large to fit in a signed 32-bit integer ('{}')", rounded));
        }

        // Static cast is well defined now
        return DataOrError<int32_t>(static_cast<int32_t>(rounded));
    }

    template <>
    DataOrError<int64_t> LuaTypeConversions::ExtractSpecificType<int64_t>(const sol::object& solObject)
    {
        if (!solObject.valid() || solObject.get_type() != sol::type::number)
        {
            return DataOrError<int64_t>(
                fmt::format("Error while extracting integer: expected a number, received '{}'",
                    sol_helper::GetSolTypeName(solObject.get_type())));
        }

        // Get Lua number as double (internal format of Lua)
        const double asDouble = solObject.as<double>();
        // Rounds to closest signed integer
        const double rounded = std::round(asDouble);

        // fractional part too large -> rounding error
        const double fractPart = std::abs(asDouble - rounded);
        if (fractPart > std::numeric_limits<double>::epsilon())
        {
            return DataOrError<int64_t>(
                fmt::format("Error while extracting integer: implicit rounding (fractional part '{}' is not negligible)",
                    fractPart));
        }

        // Integral part out of range - have to compare in double due to narrowing
        if (rounded > static_cast<double>(std::numeric_limits<int64_t>::max())
            || rounded < static_cast<double>(std::numeric_limits<int64_t>::lowest()))
        {
            return DataOrError<int64_t>(
                fmt::format("Error while extracting integer: integral part too large to fit in a signed 64-bit integer ('{}')", rounded));
        }

        // Static cast is well defined now
        return DataOrError<int64_t>(static_cast<int64_t>(rounded));
    }


    template <>
    DataOrError<size_t> LuaTypeConversions::ExtractSpecificType<size_t>(const sol::object& solObject)
    {
        if (!solObject.valid() || solObject.get_type() != sol::type::number)
        {
            return DataOrError<size_t>(
                fmt::format("Error while extracting integer: expected a number, received '{}'",
                sol_helper::GetSolTypeName(solObject.get_type())));
        }

        // Get Lua number as double (internal format of Lua)
        const double asDouble = solObject.as<double>();

        // Check that number is >= 0, with some tolerance
        if (asDouble < -std::numeric_limits<double>::epsilon())
        {
            return DataOrError<size_t>(
                fmt::format("Error while extracting integer: expected non-negative number, received '{}'", asDouble));
        }

        // Rounds to closest signed integer
        const double rounded = std::round(asDouble);

        // Integral part too large
        if (rounded > double(std::numeric_limits<size_t>::max()))
        {
            return DataOrError<size_t>(
                fmt::format("Error while extracting integer: integral part too large to fit in 64-bit unsigned integer ('{}')", rounded));
        }

        // fractional part too large -> rounding error
        const double fractPart = std::abs(asDouble - rounded);
        if (fractPart > std::numeric_limits<double>::epsilon())
        {
            return DataOrError<size_t>(
                fmt::format("Error while extracting integer: implicit rounding (fractional part '{}' is not negligible)",
                fractPart));
        }

        // Static cast is well defined now
        return DataOrError<size_t>(static_cast<size_t>(rounded));
    }

    template <>
    DataOrError<std::string_view> LuaTypeConversions::ExtractSpecificType<std::string_view>(const sol::object& solObject)
    {
        if (!solObject.is<std::string_view>())
        {
            return DataOrError<std::string_view>(
                fmt::format("Expected a string but got object of type {} instead!",
                sol_helper::GetSolTypeName(solObject.get_type())));
        }

        return DataOrError<std::string_view>(solObject.as<std::string_view>());
    }

    template <typename T, size_t size >
    DataOrError< std::array<T, size> > LuaTypeConversions::ExtractArray(const sol::object& solObject)
    {
        const std::optional<sol::lua_table> potentialLuaTable = ExtractLuaTable(solObject);
        if (!potentialLuaTable)
        {
            return DataOrError<std::array<T, size>>(fmt::format("Expected a Lua table with {} entries but got object of type {} instead!",
                size,
                sol_helper::GetSolTypeName(solObject.get_type())));
        }

        const sol::lua_table& solTable = *potentialLuaTable;

        const size_t tableFieldCount = solTable.size();

        if (tableFieldCount != size)
        {
            return DataOrError<std::array<T, size>>(
                fmt::format("Error while extracting array: expected {} array components in table but got {} instead!",
                            size, tableFieldCount));
        }

        std::array<T, size> data{};
        for (size_t i = 1; i <= size; ++i)
        {
            const sol::object& tableEntry = solTable[i];

            const DataOrError<T> maybeValue = ExtractSpecificType<T>(tableEntry);

            if (maybeValue.hasError())
            {
                return DataOrError<std::array<T, size>>(
                    fmt::format("Error while extracting array: unexpected value (type: '{}') at array element # {}! Reason: {}",
                                sol_helper::GetSolTypeName(tableEntry.get_type()), i, maybeValue.getError())
                );
            }

            data[i-1] = maybeValue.getData();
        }

        return DataOrError(std::move(data));
    }

    // Explicitly instantiate types we use
    template DataOrError<std::array<int32_t, 2>> LuaTypeConversions::ExtractArray<int32_t, 2>(const sol::object& solObject);
    template DataOrError<std::array<int32_t, 3>> LuaTypeConversions::ExtractArray<int32_t, 3>(const sol::object& solObject);
    template DataOrError<std::array<int32_t, 4>> LuaTypeConversions::ExtractArray<int32_t, 4>(const sol::object& solObject);
    template DataOrError<std::array<float, 2>> LuaTypeConversions::ExtractArray<float, 2>(const sol::object& solObject);
    template DataOrError<std::array<float, 3>> LuaTypeConversions::ExtractArray<float, 3>(const sol::object& solObject);
    template DataOrError<std::array<float, 4>> LuaTypeConversions::ExtractArray<float, 4>(const sol::object& solObject);
}
