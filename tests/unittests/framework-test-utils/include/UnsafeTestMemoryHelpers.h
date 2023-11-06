//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "absl/types/span.h"

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <type_traits>

namespace ramses::internal
{
    namespace UnsafeTestMemoryHelpers
    {
        inline void* ForgeArbitraryPointer(size_t pointingTo)
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) construct dummy pointer
            return reinterpret_cast<void*>(pointingTo);
        }

        inline std::uintptr_t PointerToInteger(const void* ptr)
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) needed for conversion
            return reinterpret_cast<std::uintptr_t>(ptr);
        }

        template <typename T>
        inline void WriteToMemoryBlob(T value, std::byte* destBlob, size_t index = 0)
        {
            static_assert(std::is_arithmetic<T>::value, "T must be arithmetic type");
            std::memcpy(destBlob + index*sizeof(T), &value, sizeof(T));
        }

        template <typename T>
        inline T GetTypedValueFromMemoryBlob(const std::byte* data, size_t index = 0)
        {
            static_assert(std::is_arithmetic<T>::value, "T must be arithmetic type");
            T result;
            std::memcpy(&result, data + index*sizeof(T), sizeof(T));
            return result;
        }

        inline bool CompareMemoryBlobToSpan(const void* dataBlob, size_t blobSize, absl::Span<const std::byte> theSpan)
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) safe to get std::byte*
            return absl::MakeSpan(reinterpret_cast<const std::byte*>(dataBlob), blobSize) == theSpan;
        }

        inline const std::byte* ConvertToBytes(std::initializer_list<uint32_t> values)
        {
            return reinterpret_cast<const std::byte*>(std::data(values));
        }
    }
}
