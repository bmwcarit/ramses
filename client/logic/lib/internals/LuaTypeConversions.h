//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internals/SolWrapper.h"
#include "ramses-logic/EPropertyType.h"

#include <variant>
#include <array>

namespace rlogic::internal
{
    // Simple implementation of std::expected for the purpose of type conversion
    template<typename T>
    class DataOrError
    {
    public:
        explicit DataOrError(T data)
            : m_data(std::move(data))
        {
        }

        explicit DataOrError(const std::string& errorMessage)
            : m_data(Error{errorMessage})
        {
        }

        [[nodiscard]] bool hasError() const
        {
            return std::holds_alternative<Error>(m_data);
        }

        [[nodiscard]] T getData() const
        {
            assert(std::holds_alternative<T>(m_data));
            return std::get<T>(m_data);
        }

        [[nodiscard]] const std::string& getError() const
        {
            assert(std::holds_alternative<Error>(m_data));
            return std::get<Error>(m_data).message;
        }

    private:
        struct Error
        {
            std::string message;
        };

        std::variant<T, Error> m_data;
    };


    class LuaTypeConversions
    {
    public:
        [[nodiscard]] static std::optional<sol::lua_table> ExtractLuaTable(const sol::object& object);

        template <typename T>
        [[nodiscard]] static DataOrError<T> ExtractSpecificType(const sol::object& solObject);

        [[nodiscard]] static size_t         GetMaxIndexForVectorType(rlogic::EPropertyType type);

        template <typename T, size_t size>
        [[nodiscard]] static DataOrError<std::array<T, size>> ExtractArray(const sol::object& solObject);

        static_assert(std::is_same<LUA_NUMBER, double>::value, "This class assumes that Lua-internal numbers are double precision floats");
    };

}
