//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_GUID_H
#define RAMSES_GUID_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "PlatformAbstraction/Macros.h"
#include "Collections/String.h"
#include "Collections/IOutputStream.h"
#include "Collections/IInputStream.h"
#include "fmt/format.h"
#include <functional>

namespace ramses_internal
{
    class String;

    class Guid final
    {
    public:
        using value_type = uint64_t;

        constexpr Guid() = default;
        explicit Guid(const char* guid);
        explicit Guid(const String& guid);

        template <typename T,
                  typename = std::enable_if_t<std::is_integral<T>::value && !std::is_same<char, T>::value && !std::is_same<bool, T>::value>>
        constexpr explicit Guid(T value)
            : m_value(value)
        {}

        constexpr Guid(const Guid&) = default;
        constexpr Guid& operator=(const Guid&) = default;

        String toString() const;

        RNODISCARD constexpr uint64_t get() const;

        RNODISCARD constexpr bool isInvalid() const;
        RNODISCARD constexpr bool isValid() const;

        RNODISCARD constexpr bool operator==(const Guid& other) const;
        RNODISCARD constexpr bool operator!=(const Guid& other) const;

    private:
        static uint64_t GetFromString(const char* guid, size_t len);

        uint64_t m_value = 0;
    };

    inline Guid::Guid(const char* guid)
        : m_value(GetFromString(guid, std::strlen(guid)))
    {
    }

    inline Guid::Guid(const String& guid)
        : m_value(GetFromString(guid.c_str(), guid.size()))
    {
    }

    constexpr inline uint64_t Guid::get() const
    {
        return m_value;
    }

    constexpr inline bool Guid::isInvalid() const
    {
        return m_value == 0;
    }

    constexpr inline bool Guid::isValid() const
    {
        return m_value != 0;
    }

    constexpr inline bool Guid::operator==(const Guid& other) const
    {
        return m_value == other.m_value;
    }

    constexpr inline bool Guid::operator!=(const Guid& other) const
    {
        return m_value != other.m_value;
    }

    inline IOutputStream& operator<<(IOutputStream& stream, const Guid& guid)
    {
        const uint64_t value{guid.get()};
        return stream.write(&value, sizeof(value));
    }

    inline IInputStream& operator>>(IInputStream& stream, Guid& guid)
    {
        uint64_t value = 0;
        stream.read(&value, sizeof(value));
        guid = Guid(value);
        return stream;
    }
}

template <>
struct fmt::formatter<ramses_internal::Guid>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    constexpr auto format(const ramses_internal::Guid& guid, FormatContext& ctx)
    {
        const uint64_t v = guid.get();
        if (v < 256)
        {
            return fmt::format_to(ctx.out(), "00{:02X}", v);
        }
        else
        {
            const uint64_t upper = (v >> 48u) & 0xFFFFu;
            const uint64_t lower = v & 0xFFFFFFFFFFFFu;
            return fmt::format_to(ctx.out(), "{:04X}-{:012X}", upper, lower);
        }
    }
};

namespace std
{
    template<>
    struct hash<ramses_internal::Guid>
    {
        size_t operator()(const ramses_internal::Guid& key) const
        {
            return std::hash<uint64_t>()(key.get());
        }
    };
}

#endif
