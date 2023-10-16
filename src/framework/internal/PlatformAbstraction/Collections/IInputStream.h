//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/PlatformError.h"

#include <cstdint>
#include <type_traits>

namespace ramses::internal
{
    class IInputStream
    {
    public:
        enum class Seek
        {
            FromBeginning,
            Relative,
        };

        virtual ~IInputStream() = default;

        virtual IInputStream& read(void* data, size_t size) = 0;
        [[nodiscard]] virtual EStatus getState() const = 0;

        virtual EStatus seek(int64_t numberOfBytesToSeek, Seek origin) = 0;
        virtual EStatus getPos(size_t& position) const = 0;
    };

    template <typename T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, bool> = true>
    IInputStream& operator>>(IInputStream& stream, T& value)
    {
        return stream.read(&value, sizeof(value));
    }

    template <typename E, std::enable_if_t<std::is_enum_v<E> && !std::is_convertible_v<E, int>, bool> = true>
    IInputStream& operator>>(IInputStream& stream, E& value)
    {
        std::underlying_type_t<E> valueInUnderlyingType;
        stream >> valueInUnderlyingType;
        value = static_cast<E>(valueInUnderlyingType);

        return stream;
    }

    inline IInputStream& operator>>(IInputStream& stream, std::string& value)
    {
        uint32_t length{0};
        stream >> length; // first read the length of the string
        value.resize(length);
        if (length > 0)
        {
            stream.read(value.data(), length);
        }

        return stream;
    }
}
