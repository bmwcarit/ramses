//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MEMORYPOOLEXPLICIT_H
#define RAMSES_MEMORYPOOLEXPLICIT_H

#include "Common/TypedMemoryHandle.h"
#include "Collections/Vector.h"
#include <limits>

namespace ramses_internal
{
    template <typename OBJECTTYPE, typename HANDLE>
    class MemoryPoolExplicit final
    {
    public:
        typedef OBJECTTYPE object_type;
        typedef HANDLE handle_type;

        explicit MemoryPoolExplicit(UInt32 size = 0);

        // Creation/Deletion
        HANDLE                          allocate(HANDLE handle);
        void                            release(HANDLE handle);

        // Access
        UInt32                          getTotalCount() const;
        Bool                            isAllocated(HANDLE handle) const;

        // Access to actual memory
        OBJECTTYPE*                     getMemory(HANDLE handle);
        const OBJECTTYPE*               getMemory(HANDLE handle) const;

        void                            preallocateSize(UInt32 size);

        static_assert(std::is_move_constructible<OBJECTTYPE>::value && std::is_move_assignable<OBJECTTYPE>::value, "OBJECTTYPE must be movable");
    protected:
        std::vector<OBJECTTYPE> m_memoryPool;
        std::vector<UInt8>      m_handlePool;
    };

    template <typename OBJECTTYPE, typename HANDLE>
    MemoryPoolExplicit<OBJECTTYPE, HANDLE>::MemoryPoolExplicit(UInt32 size /*= 0*/)
        : m_memoryPool(size)
        , m_handlePool(size)
    {
    }

    template <typename OBJECTTYPE, typename HANDLE>
    void MemoryPoolExplicit<OBJECTTYPE, HANDLE>::preallocateSize(UInt32 size)
    {
        const UInt currSize = m_memoryPool.size();
        assert(currSize == m_handlePool.size());
        if (size > currSize)
        {
            m_handlePool.resize(size);
            m_memoryPool.resize(size);
        }
    }

    template <typename OBJECTTYPE, typename HANDLE>
    inline HANDLE MemoryPoolExplicit<OBJECTTYPE, HANDLE>::allocate(HANDLE handle)
    {
        const MemoryHandle memoryHandle = AsMemoryHandle(handle);
        assert(memoryHandle < m_memoryPool.size());
        assert(m_handlePool[memoryHandle] == 0);

        m_memoryPool[memoryHandle] = OBJECTTYPE();
        m_handlePool[memoryHandle] = std::numeric_limits<UInt8>::max();

        return handle;
    }

    template <typename OBJECTTYPE, typename HANDLE>
    inline void MemoryPoolExplicit<OBJECTTYPE, HANDLE>::release(HANDLE handle)
    {
        const MemoryHandle memoryHandle = AsMemoryHandle(handle);
        assert(memoryHandle < m_handlePool.size());
        assert(m_handlePool[memoryHandle] != 0);
        m_handlePool[memoryHandle] = 0;
    }

    template <typename OBJECTTYPE, typename HANDLE>
    inline Bool MemoryPoolExplicit<OBJECTTYPE, HANDLE>::isAllocated(HANDLE handle) const
    {
        const MemoryHandle memoryHandle = AsMemoryHandle(handle);
        assert(memoryHandle < m_handlePool.size());
        return m_handlePool[memoryHandle] != 0;
    }

    template <typename OBJECTTYPE, typename HANDLE>
    inline const OBJECTTYPE* MemoryPoolExplicit<OBJECTTYPE, HANDLE>::getMemory(HANDLE handle) const
    {
        const MemoryHandle memoryHandle = AsMemoryHandle(handle);
        assert(memoryHandle < m_memoryPool.size());
        assert(m_handlePool[memoryHandle] != 0);

        return &m_memoryPool[memoryHandle];
    }

    template <typename OBJECTTYPE, typename HANDLE>
    inline OBJECTTYPE* MemoryPoolExplicit<OBJECTTYPE, HANDLE>::getMemory(HANDLE handle)
    {
        const MemoryHandle memoryHandle = AsMemoryHandle(handle);
        assert(memoryHandle < m_memoryPool.size());
        assert(m_handlePool[memoryHandle] != 0);

        return &m_memoryPool[memoryHandle];
    }

    template <typename OBJECTTYPE, typename HANDLE>
    inline UInt32 MemoryPoolExplicit<OBJECTTYPE, HANDLE>::getTotalCount() const
    {
        return static_cast<UInt32>(m_memoryPool.size());
    }
}

#endif
