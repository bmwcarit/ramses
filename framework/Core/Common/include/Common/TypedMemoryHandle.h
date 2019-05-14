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
#include "Collections/IInputStream.h"
#include "Collections/IOutputStream.h"
#include "Collections/StringOutputStream.h"
#include "Common/MemoryHandle.h"

#include "ramses-capu/util/Traits.h"
#include "ramses-capu/container/Hash.h"

namespace ramses_internal
{
    template <typename UniqueId>
    class TypedMemoryHandle final
    {
    public:
        typedef MemoryHandle Type;

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

        Bool isValid() const
        {
            return m_handle != Invalid();
        }

        // conversion
        inline
        MemoryHandle asMemoryHandle() const
        {
            return m_handle;
        }

        inline
        MemoryHandle& asMemoryHandleReference()
        {
            return m_handle;
        }

        // operators ++/--
        inline TypedMemoryHandle& operator++()
        {
            ++m_handle;
            return *this;
        }

        inline TypedMemoryHandle operator++(int)
        {
            TypedMemoryHandle res(m_handle);
            ++m_handle;
            return res;
        }

        inline TypedMemoryHandle& operator--()
        {
            --m_handle;
            return *this;
        }

        inline TypedMemoryHandle operator--(int)
        {
            TypedMemoryHandle res(m_handle);
            --m_handle;
            return res;
        }

        // operators ==/!=
        inline friend Bool operator==(TypedMemoryHandle a, TypedMemoryHandle b)
        {
            return a.m_handle == b.m_handle;
        }

        inline friend Bool operator==(TypedMemoryHandle a, UInt b)
        {
            return a.m_handle == b;
        }

        inline friend Bool operator==(UInt a, TypedMemoryHandle b)
        {
            return a == b.m_handle;
        }

        inline friend Bool operator!=(TypedMemoryHandle a, TypedMemoryHandle b)
        {
            return a.m_handle != b.m_handle;
        }

        inline friend Bool operator!=(TypedMemoryHandle a, UInt b)
        {
            return a.m_handle != b;
        }

        inline friend Bool operator!=(UInt a, TypedMemoryHandle b)
        {
            return a != b.m_handle;
        }

        // operator </<=
        inline friend Bool operator<(TypedMemoryHandle a, TypedMemoryHandle b)
        {
            return a.m_handle < b.m_handle;
        }

        inline friend Bool operator<(TypedMemoryHandle a, UInt b)
        {
            return a.m_handle < b;
        }

        inline friend Bool operator<(UInt a, TypedMemoryHandle b)
        {
            return a < b.m_handle;
        }

        inline friend Bool operator<=(TypedMemoryHandle a, TypedMemoryHandle b)
        {
            return a.m_handle <= b.m_handle;
        }

        inline friend Bool operator<=(TypedMemoryHandle a, UInt b)
        {
            return a.m_handle <= b;
        }

        inline friend Bool operator<=(UInt a, TypedMemoryHandle b)
        {
            return a <= b.m_handle;
        }

        // operator >/>=
        inline friend Bool operator>(TypedMemoryHandle a, TypedMemoryHandle b)
        {
            return a.m_handle > b.m_handle;
        }

        inline friend Bool operator>(TypedMemoryHandle a, UInt b)
        {
            return a.m_handle > b;
        }

        inline friend Bool operator>(UInt a, TypedMemoryHandle b)
        {
            return a > b.m_handle;
        }

        inline friend Bool operator>=(TypedMemoryHandle a, TypedMemoryHandle b)
        {
            return a.m_handle >= b.m_handle;
        }

        inline friend Bool operator>=(TypedMemoryHandle a, UInt b)
        {
            return a.m_handle >= b;
        }

        inline friend Bool operator>=(UInt a, TypedMemoryHandle b)
        {
            return a >= b.m_handle;
        }

        // operator +/-
        inline friend TypedMemoryHandle operator+(TypedMemoryHandle a, UInt32 b)
        {
            return TypedMemoryHandle(a.m_handle + b);
        }

        inline friend TypedMemoryHandle operator+(UInt32 a, TypedMemoryHandle b)
        {
            return TypedMemoryHandle(a + b.m_handle);
        }

        inline friend TypedMemoryHandle operator-(TypedMemoryHandle a, UInt32 b)
        {
            return TypedMemoryHandle(a.m_handle - b);
        }

        inline friend TypedMemoryHandle operator-(UInt32 a, TypedMemoryHandle b)
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

    // StringOutputStream operators
    template <typename UniqueId>
    inline StringOutputStream& operator<<(StringOutputStream& os, const TypedMemoryHandle<UniqueId>& handle)
    {
        os << handle.asMemoryHandle();
        return os;
    }
}

// make TypedMemoryHandle hash exactly equal to MemoryHandle
namespace ramses_capu
{
    template <typename UniqueId>
    struct Hash<::ramses_internal::TypedMemoryHandle<UniqueId>>
    {
        uint_t operator()(const ::ramses_internal::TypedMemoryHandle<UniqueId>& key)
        {
            // hasher for primitives
            return Hash<ramses_internal::MemoryHandle>()(key.asMemoryHandle());
        }
    };
}

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
