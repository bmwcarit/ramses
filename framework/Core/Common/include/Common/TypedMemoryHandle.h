//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TYPEDMEMORYHANDLE_H
#define RAMSES_TYPEDMEMORYHANDLE_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "PlatformAbstraction/Hash.h"
#include "Collections/IInputStream.h"
#include "Collections/IOutputStream.h"
#include "Collections/StringOutputStream.h"
#include "Common/MemoryHandle.h"
#include "PlatformAbstraction/FmtBase.h"
#include "PlatformAbstraction/Macros.h"

namespace ramses_internal
{
    template <typename UniqueId>
    class TypedMemoryHandle final
    {
    public:
        using Type = MemoryHandle;

        static constexpr TypedMemoryHandle Invalid()
        {
            return TypedMemoryHandle();
        }

        constexpr TypedMemoryHandle()
            : m_handle(InvalidMemoryHandle)
        {}

        explicit constexpr TypedMemoryHandle(MemoryHandle handle)
            : m_handle(handle)
        {}

        RNODISCARD constexpr bool isValid() const
        {
            return m_handle != Invalid();
        }

        // conversion
        RNODISCARD constexpr inline MemoryHandle asMemoryHandle() const
        {
            return m_handle;
        }

        RNODISCARD constexpr inline MemoryHandle& asMemoryHandleReference()
        {
            return m_handle;
        }

        // operators ++/--
        constexpr inline TypedMemoryHandle& operator++()
        {
            ++m_handle;
            return *this;
        }

        constexpr inline const TypedMemoryHandle operator++(int)
        {
            TypedMemoryHandle res(m_handle);
            ++m_handle;
            return res;
        }

        constexpr inline TypedMemoryHandle& operator--()
        {
            --m_handle;
            return *this;
        }

        constexpr inline const TypedMemoryHandle operator--(int)
        {
            TypedMemoryHandle res(m_handle);
            --m_handle;
            return res;
        }

        // operators ==/!=
        RNODISCARD constexpr inline friend bool operator==(TypedMemoryHandle a, TypedMemoryHandle b)
        {
            return a.m_handle == b.m_handle;
        }

        RNODISCARD constexpr inline friend bool operator==(TypedMemoryHandle a, uintptr_t b)
        {
            return a.m_handle == b;
        }

        RNODISCARD constexpr inline friend bool operator==(uintptr_t a, TypedMemoryHandle b)
        {
            return a == b.m_handle;
        }

        RNODISCARD constexpr inline friend bool operator!=(TypedMemoryHandle a, TypedMemoryHandle b)
        {
            return a.m_handle != b.m_handle;
        }

        RNODISCARD constexpr inline friend bool operator!=(TypedMemoryHandle a, uintptr_t b)
        {
            return a.m_handle != b;
        }

        RNODISCARD constexpr inline friend bool operator!=(uintptr_t a, TypedMemoryHandle b)
        {
            return a != b.m_handle;
        }

        // operator </<=
        RNODISCARD constexpr inline friend bool operator<(TypedMemoryHandle a, TypedMemoryHandle b)
        {
            return a.m_handle < b.m_handle;
        }

        RNODISCARD constexpr inline friend bool operator<(TypedMemoryHandle a, uintptr_t b)
        {
            return a.m_handle < b;
        }

        RNODISCARD constexpr inline friend bool operator<(uintptr_t a, TypedMemoryHandle b)
        {
            return a < b.m_handle;
        }

        RNODISCARD constexpr inline friend bool operator<=(TypedMemoryHandle a, TypedMemoryHandle b)
        {
            return a.m_handle <= b.m_handle;
        }

        RNODISCARD constexpr inline friend bool operator<=(TypedMemoryHandle a, uintptr_t b)
        {
            return a.m_handle <= b;
        }

        RNODISCARD constexpr inline friend bool operator<=(uintptr_t a, TypedMemoryHandle b)
        {
            return a <= b.m_handle;
        }

        // operator >/>=
        RNODISCARD constexpr inline friend bool operator>(TypedMemoryHandle a, TypedMemoryHandle b)
        {
            return a.m_handle > b.m_handle;
        }

        RNODISCARD constexpr inline friend bool operator>(TypedMemoryHandle a, uintptr_t b)
        {
            return a.m_handle > b;
        }

        RNODISCARD constexpr inline friend bool operator>(uintptr_t a, TypedMemoryHandle b)
        {
            return a > b.m_handle;
        }

        RNODISCARD constexpr inline friend bool operator>=(TypedMemoryHandle a, TypedMemoryHandle b)
        {
            return a.m_handle >= b.m_handle;
        }

        RNODISCARD constexpr inline friend bool operator>=(TypedMemoryHandle a, uintptr_t b)
        {
            return a.m_handle >= b;
        }

        RNODISCARD constexpr inline friend bool operator>=(uintptr_t a, TypedMemoryHandle b)
        {
            return a >= b.m_handle;
        }

        // operator +/-
        RNODISCARD constexpr inline friend TypedMemoryHandle operator+(TypedMemoryHandle a, uint32_t b)
        {
            return TypedMemoryHandle(a.m_handle + b);
        }

        RNODISCARD constexpr inline friend TypedMemoryHandle operator+(uint32_t a, TypedMemoryHandle b)
        {
            return TypedMemoryHandle(a + b.m_handle);
        }

        RNODISCARD constexpr inline friend TypedMemoryHandle operator-(TypedMemoryHandle a, uint32_t b)
        {
            return TypedMemoryHandle(a.m_handle - b);
        }

        RNODISCARD constexpr inline friend TypedMemoryHandle operator-(uint32_t a, TypedMemoryHandle b)
        {
            return TypedMemoryHandle(a - b.m_handle);
        }

    private:
        MemoryHandle m_handle;
    };

    // conversion to handle
    template <typename T>
    struct ConversionToMemoryHandle
    {
        static MemoryHandle Convert(T handle)
        {
            return static_cast<MemoryHandle>(handle);
        }
    };

    template <typename UniqueId>
    struct ConversionToMemoryHandle< TypedMemoryHandle<UniqueId> >
    {
        static MemoryHandle Convert(TypedMemoryHandle<UniqueId> handle)
        {
            return handle.asMemoryHandle();
        }
    };

    template <typename T>
    MemoryHandle AsMemoryHandle(T handle)
    {
        return ConversionToMemoryHandle<T>::Convert(handle);
    }


    // general IInputStream/IOutputStream operators
    template <typename UniqueId>
    inline IOutputStream& operator<<(IOutputStream& os, const TypedMemoryHandle<UniqueId>& handle)
    {
        os << handle.asMemoryHandle();
        return os;
    }

    template <typename UniqueId>
    inline IInputStream& operator>>(IInputStream& is, TypedMemoryHandle<UniqueId>& handle)
    {
        is >> handle.asMemoryHandleReference();
        return is;
    }
}

template <typename UniqueId>
struct fmt::formatter<ramses_internal::TypedMemoryHandle<UniqueId>> : public ramses_internal::SimpleFormatterBase
{
    template<typename FormatContext>
    constexpr auto format(const ramses_internal::TypedMemoryHandle<UniqueId>& str, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "{}", str.asMemoryHandle());
    }
};

namespace std
{
    template <typename UniqueId>
    struct hash<::ramses_internal::TypedMemoryHandle<UniqueId>>
    {
        size_t operator()(const ::ramses_internal::TypedMemoryHandle<UniqueId>& key) const
        {
            return static_cast<size_t>(hash<ramses_internal::MemoryHandle>()(key.asMemoryHandle()));
        }
    };

    template <typename UniqueId>
    struct numeric_limits < ramses_internal::TypedMemoryHandle<UniqueId> >
    {
        static inline ramses_internal::TypedMemoryHandle<UniqueId> max()
        {
            return ramses_internal::TypedMemoryHandle<UniqueId>(std::numeric_limits<ramses_internal::MemoryHandle>::max());
        }

        static inline ramses_internal::TypedMemoryHandle<UniqueId> min()
        {
            return ramses_internal::TypedMemoryHandle<UniqueId>(std::numeric_limits<ramses_internal::MemoryHandle>::min());
        }
    };
}

#endif
