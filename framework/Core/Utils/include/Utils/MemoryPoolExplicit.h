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
#include "Utils/MemoryPoolIterator.h"
#include "Collections/Vector.h"
#include <limits>

namespace ramses_internal
{
    template <typename OBJECTTYPE, typename HANDLE>
    class MemoryPoolExplicit final
    {
    public:
        using object_type       = OBJECTTYPE;
        using handle_type       = HANDLE;

        using iterator          = memory_pool_iterator<MemoryPoolExplicit>;
        using const_iterator    = const_memory_pool_iterator<MemoryPoolExplicit>;

        explicit MemoryPoolExplicit(uint32_t size = 0);

        // Creation/Deletion
        HANDLE                          allocate(HANDLE handle);
        void                            release(HANDLE handle);

        // Access
        [[nodiscard]] uint32_t                          getTotalCount() const;
        [[nodiscard]] bool                            isAllocated(HANDLE handle) const;

        // Access to actual memory
        OBJECTTYPE*                     getMemory(HANDLE handle);
        [[nodiscard]] const OBJECTTYPE*               getMemory(HANDLE handle) const;

        void                            preallocateSize(uint32_t size);

        iterator                        begin();
        iterator                        end();
        [[nodiscard]] const_iterator                  begin() const;
        [[nodiscard]] const_iterator                  end() const;
        [[nodiscard]] const_iterator                  cbegin() const;
        [[nodiscard]] const_iterator                  cend() const;

        static_assert(std::is_move_constructible<OBJECTTYPE>::value && std::is_move_assignable<OBJECTTYPE>::value, "OBJECTTYPE must be movable");
    protected:
        std::vector<OBJECTTYPE> m_memoryPool;
        std::vector<uint8_t>      m_handlePool;
    };

    template <typename OBJECTTYPE, typename HANDLE>
    MemoryPoolExplicit<OBJECTTYPE, HANDLE>::MemoryPoolExplicit(uint32_t size /*= 0*/)
        : m_memoryPool(size)
        , m_handlePool(size)
    {
    }

    template <typename OBJECTTYPE, typename HANDLE>
    void MemoryPoolExplicit<OBJECTTYPE, HANDLE>::preallocateSize(uint32_t size)
    {
        const size_t currSize = m_memoryPool.size();
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
        m_handlePool[memoryHandle] = std::numeric_limits<uint8_t>::max();

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
    inline bool MemoryPoolExplicit<OBJECTTYPE, HANDLE>::isAllocated(HANDLE handle) const
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
    inline uint32_t MemoryPoolExplicit<OBJECTTYPE, HANDLE>::getTotalCount() const
    {
        return static_cast<uint32_t>(m_memoryPool.size());
    }

    template <typename OBJECTTYPE, typename HANDLE>
    inline
    typename MemoryPoolExplicit<OBJECTTYPE, HANDLE>::iterator MemoryPoolExplicit<OBJECTTYPE, HANDLE>::begin()
    {
        return iterator(*this, HANDLE(0u));
    }

    template <typename OBJECTTYPE, typename HANDLE>
    inline
    typename MemoryPoolExplicit<OBJECTTYPE, HANDLE>::iterator MemoryPoolExplicit<OBJECTTYPE, HANDLE>::end()
    {
        return iterator(*this, HANDLE(getTotalCount()));
    }

    template <typename OBJECTTYPE, typename HANDLE>
    inline
    typename MemoryPoolExplicit<OBJECTTYPE, HANDLE>::const_iterator MemoryPoolExplicit<OBJECTTYPE, HANDLE>::begin() const
    {
        return const_iterator(*this, HANDLE(0u));
    }

    template <typename OBJECTTYPE, typename HANDLE>
    inline
    typename MemoryPoolExplicit<OBJECTTYPE, HANDLE>::const_iterator MemoryPoolExplicit<OBJECTTYPE, HANDLE>::end() const
    {
        return const_iterator(*this, HANDLE(getTotalCount()));
    }

    template <typename OBJECTTYPE, typename HANDLE>
    inline
    typename MemoryPoolExplicit<OBJECTTYPE, HANDLE>::const_iterator MemoryPoolExplicit<OBJECTTYPE, HANDLE>::cbegin() const
    {
        return const_iterator(*this, HANDLE(0u));
    }

    template <typename OBJECTTYPE, typename HANDLE>
    inline
    typename MemoryPoolExplicit<OBJECTTYPE, HANDLE>::const_iterator MemoryPoolExplicit<OBJECTTYPE, HANDLE>::cend() const
    {
        return const_iterator(*this, HANDLE(getTotalCount()));
    }
}

#endif
