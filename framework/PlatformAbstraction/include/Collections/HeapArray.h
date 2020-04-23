//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CONTAINER_HEAPARRAY_H
#define RAMSES_CONTAINER_HEAPARRAY_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "PlatformAbstraction/PlatformMemory.h"
#include "PlatformAbstraction/Macros.h"
#include <memory>

namespace ramses_internal
{
    template <typename T, typename _uniqueId = void>
    class HeapArray
    {
        static_assert(std::is_integral<T>::value, "only integral types allowed");

    public:
        explicit HeapArray(UInt size = 0, const T* data = nullptr);
        HeapArray(UInt size, HeapArray&& other);

        HeapArray(const HeapArray&) = delete;
        HeapArray& operator=(const HeapArray&) = delete;

        HeapArray(HeapArray&&) noexcept;
        HeapArray& operator=(HeapArray&&) noexcept;

        UInt size() const;
        T* data();
        const T* data() const;

        void setZero();

    private:
        UInt m_size;
        std::unique_ptr<T[]> m_data;
    };

    template <typename T, typename _uniqueId>
    inline
    HeapArray<T, _uniqueId>::HeapArray(UInt size, const T* data)
        : m_size(size)
        , m_data(m_size > 0 ? new T[m_size] : nullptr)
    {
        if (m_data && data)
        {
            PlatformMemory::Copy(m_data.get(), data, size);
        }
    }

    template <typename T, typename _uniqueId>
    inline
    HeapArray<T, _uniqueId>::HeapArray(UInt size, HeapArray&& other)
        : m_size(size)
        , m_data(std::move(other.m_data))
    {
        static_assert(std::is_nothrow_move_constructible<HeapArray>::value, "HeapArray must be movable");
        other.m_size = 0;
    }

    template <typename T, typename _uniqueId>
    inline
    HeapArray<T, _uniqueId>::HeapArray(HeapArray&& o) noexcept
        : m_size(o.m_size)
        , m_data(std::move(o.m_data))
    {
        static_assert(std::is_nothrow_move_assignable<HeapArray>::value, "HeapArray must be movable");
        o.m_size = 0;
    }

    template <typename T, typename _uniqueId>
    inline
    HeapArray<T, _uniqueId>& HeapArray<T, _uniqueId>::operator=(HeapArray&& o) noexcept
    {
        if (&o != this)
        {
            m_size = o.m_size;
            m_data = std::move(o.m_data);
            o.m_size = 0;
        }
        return *this;
    }

    template <typename T, typename _uniqueId>
    inline
    UInt HeapArray<T, _uniqueId>::size() const
    {
        return m_size;
    }

    template <typename T, typename _uniqueId>
    inline
    T* HeapArray<T, _uniqueId>::data()
    {
        return m_data.get();
    }

    template <typename T, typename _uniqueId>
    inline
    const T* HeapArray<T, _uniqueId>::data() const
    {
        return m_data.get();
    }

    template <typename T, typename _uniqueId>
    inline
    void HeapArray<T, _uniqueId>::setZero()
    {
        if (m_data)
        {
            PlatformMemory::Set(m_data.get(), 0, m_size);
        }
    }
}

#endif
