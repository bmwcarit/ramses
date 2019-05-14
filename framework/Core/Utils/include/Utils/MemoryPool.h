//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MEMORYPOOL_H
#define RAMSES_MEMORYPOOL_H

#include "Utils/HandlePool.h"
#include <type_traits>

namespace ramses_internal
{
    template <typename OBJECTTYPE, typename HANDLE>
    class MemoryPool final
    {
    public:
        typedef OBJECTTYPE object_type;
        typedef HANDLE handle_type;

        explicit MemoryPool(UInt32 size = 0);

        // Creation/Deletion
        HANDLE                          allocate(HANDLE handle = InvalidMemoryHandle());
        void                            release(HANDLE handle);

        // Access
        UInt32                          getTotalCount() const;
        UInt32                          getActualCount() const;
        Bool                            isAllocated(HANDLE handle) const;

        // Access to actual memory
        OBJECTTYPE*                     getMemory(HANDLE handle);
        const OBJECTTYPE*               getMemory(HANDLE handle) const;

        void                            preallocateSize(UInt32 size);

        static HANDLE                   InvalidMemoryHandle();

        static_assert(std::is_move_constructible<OBJECTTYPE>::value && std::is_move_assignable<OBJECTTYPE>::value, "OBJECTTYPE must be movable");
    protected:
        std::vector<OBJECTTYPE> m_memoryPool;
        HandlePool<HANDLE> m_handlePool;
    };

    template <typename OBJECTTYPE, typename HANDLE>
    void MemoryPool<OBJECTTYPE, HANDLE>::preallocateSize(UInt32 size)
    {
        assert(m_memoryPool.size() == m_handlePool.size());
        if (size > m_memoryPool.size())
        {
            m_handlePool.resize(size);
            m_memoryPool.resize(size);
        }
    }

    template <typename OBJECTTYPE, typename HANDLE>
    MemoryPool<OBJECTTYPE, HANDLE>::MemoryPool(UInt32 size /*= 0*/)
        : m_memoryPool(size)
        , m_handlePool(size)
    {
    }

    template <typename OBJECTTYPE, typename HANDLE>
    HANDLE MemoryPool<OBJECTTYPE, HANDLE>::InvalidMemoryHandle()
    {
        return HandlePool<HANDLE>::InvalidMemoryHandle();
    }

    template <typename OBJECTTYPE, typename HANDLE>
    inline
    HANDLE MemoryPool<OBJECTTYPE, HANDLE>::allocate(HANDLE handle)
    {
        const HANDLE actualHandle = m_handlePool.acquire(handle);
        const MemoryHandle memoryHandle = AsMemoryHandle(actualHandle);

        if (memoryHandle < m_memoryPool.size())
        {
            m_memoryPool[memoryHandle] = OBJECTTYPE();
        }
        else
        {
            m_memoryPool.resize(memoryHandle + 1u);
        }

        return actualHandle;
    }

    template <typename OBJECTTYPE, typename HANDLE>
    inline
    const OBJECTTYPE* MemoryPool<OBJECTTYPE, HANDLE>::getMemory(HANDLE handle) const
    {
        assert(handle < m_memoryPool.size());
        assert(m_handlePool.isAcquired(handle));

        return &m_memoryPool[AsMemoryHandle(handle)];
    }

    template <typename OBJECTTYPE, typename HANDLE>
    inline
    OBJECTTYPE* MemoryPool<OBJECTTYPE, HANDLE>::getMemory(HANDLE handle)
    {
        assert(handle < m_memoryPool.size());
        assert(m_handlePool.isAcquired(handle));

        return &m_memoryPool[AsMemoryHandle(handle)];
    }

    template <typename OBJECTTYPE, typename HANDLE>
    inline
    UInt32 MemoryPool<OBJECTTYPE, HANDLE>::getTotalCount() const
    {
        return static_cast<UInt32>(m_memoryPool.size());
    }

    template <typename OBJECTTYPE, typename HANDLE>
    inline
    UInt32 MemoryPool<OBJECTTYPE, HANDLE>::getActualCount() const
    {
        return m_handlePool.getNumberOfAcquired();
    }

    template <typename OBJECTTYPE, typename HANDLE>
    inline
    void MemoryPool<OBJECTTYPE, HANDLE>::release(HANDLE handle)
    {
        assert(handle < m_memoryPool.size());
        m_handlePool.release(handle);
    }

    template <typename OBJECTTYPE, typename HANDLE>
    inline
    Bool MemoryPool<OBJECTTYPE, HANDLE>::isAllocated(HANDLE handle) const
    {
        return m_handlePool.isAcquired(handle);
    }
}

#endif
