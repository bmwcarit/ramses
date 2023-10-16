//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/PlatformMemory.h"
#include "internal/PlatformAbstraction/Macros.h"
#include "internal/Core/Utils/AssertMovable.h"
#include "absl/types/span.h"

#include <cstdint>
#include <memory>
#include <type_traits>

namespace ramses::internal
{
    template <typename T, typename UniqueIdT = void>
    class HeapArray final
    {
        static_assert(std::is_same_v<T, std::byte> || std::is_integral_v<T>, "only integral types or std::byte allowed");

    public:
        explicit HeapArray(size_t size = 0, const T* data = nullptr);
        HeapArray(size_t size, HeapArray&& other);

        HeapArray(const HeapArray&) = delete;
        HeapArray& operator=(const HeapArray&) = delete;

        HeapArray(HeapArray&& o) noexcept;
        HeapArray& operator=(HeapArray&& o) noexcept;

        RNODISCARD size_t size() const;
        RNODISCARD T* data();
        RNODISCARD const T* data() const;
        RNODISCARD absl::Span<const T> span() const;

        void setZero();

    private:
        size_t m_size;
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        std::unique_ptr<T[]> m_data;
    };

    template <typename T, typename UniqueIdT>
    inline
    HeapArray<T, UniqueIdT>::HeapArray(size_t size, const T* data)
        : m_size(size)
        , m_data(m_size > 0 ? new T[m_size] : nullptr)
    {
        if (m_data && data)
        {
            PlatformMemory::Copy(m_data.get(), data, size * sizeof(T));
        }
    }

    template <typename T, typename UniqueIdT>
    inline
    HeapArray<T, UniqueIdT>::HeapArray(size_t size, HeapArray&& other)
        : m_size(size)
        , m_data(std::move(other.m_data))
    {
        ASSERT_MOVABLE(HeapArray)

        other.m_size = 0;
    }

    template <typename T, typename UniqueIdT>
    inline
    HeapArray<T, UniqueIdT>::HeapArray(HeapArray&& o) noexcept
        : m_size(o.m_size)
        , m_data(std::move(o.m_data))
    {
        o.m_size = 0;
    }

    template <typename T, typename UniqueIdT>
    inline
    HeapArray<T, UniqueIdT>& HeapArray<T, UniqueIdT>::operator=(HeapArray&& o) noexcept
    {
        if (&o != this)
        {
            m_size = o.m_size;
            m_data = std::move(o.m_data);
            o.m_size = 0;
        }
        return *this;
    }

    template <typename T, typename UniqueIdT>
    inline
    size_t HeapArray<T, UniqueIdT>::size() const
    {
        return m_size;
    }

    template <typename T, typename UniqueIdT>
    inline
    T* HeapArray<T, UniqueIdT>::data()
    {
        return m_data.get();
    }

    template <typename T, typename UniqueIdT>
    inline
    const T* HeapArray<T, UniqueIdT>::data() const
    {
        return m_data.get();
    }

    template <typename T, typename UniqueIdT>
    inline
    absl::Span<const T> HeapArray<T, UniqueIdT>::span() const
    {
        return {m_data.get(), m_size};
    }

    template <typename T, typename UniqueIdT>
    inline
    void HeapArray<T, UniqueIdT>::setZero()
    {
        if (m_data)
        {
            PlatformMemory::Set(m_data.get(), 0, m_size * sizeof(T));
        }
    }
}
