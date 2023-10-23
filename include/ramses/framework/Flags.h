//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/APIExport.h"
#include <type_traits>

namespace ramses
{
    /**
     * @brief Disables global Flags operators by default
     *
     * Create a template specialization to enable them for a custom enum type.
     *
     * Example:
     * \code{.cpp}
     *   enum class EFoo{Bar, Baz};
     *   template <> struct is_flag<EFoo> : std::true_type {};
     * \endcode
     */
    template <typename E> struct is_flag : std::false_type
    {
    };

    /**
    * @brief Helper class to create type safe flags from an enum
    */
    template <typename E>
    class Flags
    {
    public:
        using enum_type = E;
        using value_type = std::underlying_type_t<E>;

        explicit constexpr Flags(value_type value) noexcept
            : m_value(value)
        {
        }

        // NOLINTNEXTLINE(google-explicit-constructor) enum E shall auto-convert to Flags<E>
        constexpr Flags(E value) noexcept
            : Flags(static_cast<value_type>(value))
        {
        }

        constexpr Flags() noexcept
            : Flags(0)
        {
        }

        constexpr Flags& setFlag(E value, bool on)
        {
            if (on)
            {
                m_value |= static_cast<value_type>(value);
            }
            else
            {
                m_value &= ~static_cast<value_type>(value);
            }
            return *this;
        }

        [[nodiscard]] constexpr bool isSet(E value) const noexcept
        {
            return (static_cast<value_type>(value) & m_value) == static_cast<value_type>(value);
        }

        [[nodiscard]] constexpr bool isSet(Flags value) const noexcept
        {
            return (value.m_value & m_value) == value.m_value;
        }

        [[nodiscard]] constexpr value_type value() const noexcept
        {
            return m_value;
        }

        [[nodiscard]] constexpr Flags operator|(E other) const noexcept
        {
            return Flags(m_value | static_cast<value_type>(other));
        }

        [[nodiscard]] constexpr bool operator==(const Flags& other) const noexcept
        {
            return m_value == other.m_value;
        }

        [[nodiscard]] constexpr bool operator!=(const Flags& other) const noexcept
        {
            return m_value != other.m_value;
        }

        static_assert(std::is_enum_v<E>, "expected enum type");
        static_assert(is_flag<E>::value, "is_flag trait needs to be declared to support global operators. Example: 'template <> struct is_flag<E> : std::true_type {};'");

    private:
        value_type m_value;
    };

    template <typename E, typename = std::enable_if_t<is_flag<E>::value> >
    [[nodiscard]] constexpr inline Flags<E> operator|(E v1, E v2) noexcept
    {
        return Flags<E>(v1) | v2;
    }

    template <typename E, typename = std::enable_if_t<is_flag<E>::value> >
    [[nodiscard]] constexpr inline Flags<E> operator|(E v1, Flags<E> v2) noexcept
    {
        return v2 | v1;
    }

    template <typename E, typename = std::enable_if_t<is_flag<E>::value> >
    [[nodiscard]] constexpr inline bool operator==(E v1, Flags<E> v2) noexcept
    {
        return v2 == v1;
    }

    template <typename E, typename = std::enable_if_t<is_flag<E>::value> >
    [[nodiscard]] constexpr inline bool operator!=(E v1, Flags<E> v2) noexcept
    {
        return v2 != v1;
    }
}

