//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_UTILS_IOUTPUTSTREAM_H
#define RAMSES_UTILS_IOUTPUTSTREAM_H

#include <cstdint>
#include <cstddef>
#include <type_traits>
#include "PlatformAbstraction/PlatformError.h"

namespace ramses_internal
{
    class IOutputStream
    {
    public:
        virtual ~IOutputStream() = default;
        virtual IOutputStream& write(const void* data, size_t size) = 0;
        virtual EStatus getPos(size_t& position) const = 0;

        IOutputStream& operator<<(const void*) = delete;
    };

    inline IOutputStream& operator<<(IOutputStream& stream, int32_t value)
    {
        return stream.write(&value, sizeof(value));
    }

    inline IOutputStream& operator<<(IOutputStream& stream, uint32_t value)
    {
        return stream.write(&value, sizeof(value));
    }

    inline IOutputStream& operator<<(IOutputStream& stream, int64_t value)
    {
        return stream.write(&value, sizeof(value));
    }

    inline IOutputStream& operator<<(IOutputStream& stream, uint64_t value)
    {
        return stream.write(&value, sizeof(value));
    }

    inline IOutputStream& operator<<(IOutputStream& stream, int16_t value)
    {
        return stream.write(&value, sizeof(value));
    }

    inline IOutputStream& operator<<(IOutputStream& stream, uint16_t value)
    {
        return stream.write(&value, sizeof(value));
    }

    inline IOutputStream& operator<<(IOutputStream& stream, int8_t value)
    {
        return stream.write(&value, sizeof(value));
    }

    inline IOutputStream& operator<<(IOutputStream& stream, uint8_t value)
    {
        return stream.write(&value, sizeof(value));
    }

    inline IOutputStream& operator<<(IOutputStream& stream, bool  value)
    {
        return stream.write(&value, sizeof(value));
    }

    inline IOutputStream& operator<<(IOutputStream& stream, float value)
    {
        return stream.write(&value, sizeof(value));
    }

    template<typename E,
             typename = typename std::enable_if<std::is_enum<E>::value>::type>
    IOutputStream& operator<<(IOutputStream& stream, E value)
    {
        //Only strongly typed enums are allowed. Non-strongly typed enums do not have a defined size
        static_assert(!std::is_convertible<E, int>::value, "Not allowed to write non-strongly typed enum values to output stream!");

        stream << static_cast<std::underlying_type_t<E> >(value);
        return stream;
    }
}

#endif
