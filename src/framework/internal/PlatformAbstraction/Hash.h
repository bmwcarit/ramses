//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <functional>
#include <cstddef>

namespace ramses::internal
{
    // HashCombine and HashVal(ue) (modeled according to standard proposal n3876)
    template <typename T>
    inline void HashCombine(std::size_t& seed, const T& value)
    {
        seed ^= std::hash<T>()(value)
            + 0x9e3779b9u + (seed << 6u) + (seed >> 2u);
    }

    template <typename T, typename... Types>
    inline void HashCombine(std::size_t& seed, const T& value, const Types&... args)
    {
        HashCombine(seed, value);
        HashCombine(seed, args...);
    }

    template <typename... Types>
    inline std::size_t HashValue(const Types&... args)
    {
        std::size_t seed = 0;
        HashCombine(seed, args...);
        return seed;
    }

    namespace internal
    {
        template <std::size_t N>
        struct FnvHash;

        template <>
        struct FnvHash<4>
        {
            uint32_t operator()(const void* key, const std::size_t len)
            {
                constexpr const uint32_t prime = 16777619UL;
                constexpr const uint32_t offset_base = 2166136261UL;

                const auto* ptr = static_cast<const std::byte*>(key);
                uint32_t result = offset_base;
                for (std::size_t i = 0; i < len; ++i)
                    result = (result ^ std::to_integer<uint8_t>(ptr[i])) * prime;
                return result;
            }
        };


        template <>
        struct FnvHash<8>
        {
            uint64_t operator()(const void* key, std::size_t len)
            {
                constexpr const uint64_t prime = 1099511628211ULL;
                constexpr const uint64_t offset_base = 14695981039346656037ULL;

                const auto* ptr = static_cast<const std::byte*>(key);
                uint64_t result = offset_base;
                for (std::size_t i = 0; i < len; ++i)
                    result = (result ^ std::to_integer<uint8_t>(ptr[i])) * prime;
                return result;
            }
        };
    }

    // HashMemoryRange for hashing arbitrary data blob
    inline std::size_t HashMemoryRange(const void* ptr, const std::size_t size)
    {
        return internal::FnvHash<sizeof(std::size_t)>()(ptr, size);
    }
}
