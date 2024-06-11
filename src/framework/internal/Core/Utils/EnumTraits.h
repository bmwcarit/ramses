//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <type_traits>
#include <utility>
#include <string_view>
#include <cassert>

// EnumTraits implementation relies on compiler macros __FUNCSIG__ / __PRETTY_FUNCTION__
// These are non-standard and only supported for recent compilers
// Sometimes older compilers do support __PRETTY_FUNCTION__, but they do not distinguish between known and unknown enum values.
// Those compilers are not supported either (e.g. gcc-7).
#if defined(_MSC_VER) && _MSC_VER >= 1910 || defined(__GNUC__) && __GNUC__ >= 9 || defined(__clang__) && __clang_major__ >= 5
#undef RAMSES_HAS_ENUMTRAITS
#define RAMSES_HAS_ENUMTRAITS 1 // prefer ramses::internal::EnumTraits::IsSupported() where possible
#endif

namespace ramses::internal
{
    namespace EnumTraits
    {
        namespace internal
        {
            const size_t Limit = 128;

            template <typename E, E V> constexpr bool IsValid()
            {
                // __FUNCSIG__/__PRETTY_FUNCTION__ will generate different strings for known and unknown enum values.
                // - known enum values will be printed as a symbol name: "ramses::EFoo::Value1"
                // - unknown enum values will be printed as a number: "15" or with preceding type: "(enum ramses::EFoo)15"
                //
                // Exact format may vary (compiler dependent):
                //
                // Examples:
                // Possible __FUNCSIG__/__PRETTY_FUNCTION__ for a known (valid) enum value:
                // "bool ramses::internal::EnumTraits::internal::IsValid<enum ramses::EFoo,ramses::EFoo::Value1>(void)"
                //                                                                        ^-- separatorIndex
                // "bool ramses::internal::EnumTraits::internal::IsValid() [E = ramses::EFoo, V = ramses::EFoo::Value1]
                //                                                                               ^-- separatorIndex
                // "bool ramses::internal::EnumTraits::internal::IsValid() [E = ramses::(anonymous namespace)::EFoo, V = ramses::(anonymous namespace)::EFoo::Value1]
                //                                                                                                      ^-- separatorIndex
                //
                // Possible __FUNCSIG__/__PRETTY_FUNCTION__ for an unknown (invalid) enum value:
                // "bool ramses::internal::EnumTraits::internal::IsValid<enum ramses::EFoo,(enum ramses::EFoo)0xf>(void)"
                //                                                                        ^-- separatorIndex
                // "bool ramses::internal::EnumTraits::internal::IsValid() [E = ramses::EFoo, V = 15]
                //                                                                               ^-- separatorIndex
                // "bool ramses::internal::EnumTraits::internal::IsValid() [E = ramses::EFoo, V = -3]
                //                                                                               ^-- separatorIndex
                // "bool ramses::internal::EnumTraits::internal::IsValid() [E = ramses::(anonymous namespace)::EFoo, V = -3]
                //                                                                                                      ^-- separatorIndex
                //
                // separatorIndex identifies the position of the enum value (template parameter V)
#if defined(__GNUC__)
                const std::string_view name = __PRETTY_FUNCTION__;
                const auto valueIndex = name.rfind("V = ");
                if (valueIndex == std::string_view::npos)
                    return false;
                const auto separatorIndex = valueIndex + 3;
#elif defined (_MSC_VER)
                const std::string_view name = __FUNCSIG__;
                const auto separatorIndex = name.rfind(',');
#else
                // invalid for unknown compilers
                const std::string_view name = " (";
                const auto separatorIndex = 0;
#endif
                // expect a symbol name after the separatorIndex, e.g.: "ramses::EFoo::Value1"
                const auto c = name[separatorIndex + 1];
                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == ':' || c == '_')
                {
                    return true;
                }
                return false;
            }

            template <typename E> constexpr size_t CountValid()
            {
                return 0;
            }

            template <typename E, E A, E... B> constexpr size_t CountValid()
            {
                return CountValid<E, B...>() + (IsValid<E, A>() ? 1u : 0u);
            }

            template <typename E, size_t... I> constexpr size_t InternalElementCount(std::integer_sequence<size_t, I...> /* unused */)
            {
                return CountValid<E, static_cast<E>(I)...>();
            }

            template <typename E> struct ElementCount
            {
                // calculates the number of known enum values for the value range: 0..Limit
                static const size_t value = InternalElementCount<E>(std::make_integer_sequence<size_t, Limit>());
            };
        }

        constexpr bool IsSupported()
        {
#if defined(RAMSES_HAS_ENUMTRAITS)
            return true;
#else
            return false;
#endif
        }

        /**
         * Verifies if the enum has the expected number of elements.
         * This is only supported for most common compilers.
         * Unsupported compilers will always pass the check.
         */
        template <typename E> [[nodiscard]] constexpr bool VerifyElementCountIfSupported(size_t elementCount)
        {
            if (!IsSupported())
            {
                return true;
            }
            assert(elementCount < internal::Limit);
            return elementCount == internal::ElementCount<E>::value;
        }
    }
}
