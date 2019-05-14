//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_HANDLEPOOL_H
#define RAMSES_HANDLEPOOL_H

#include "Common/TypedMemoryHandle.h"
#include "Collections/Vector.h"
#include <limits>

namespace ramses_internal
{
    template <typename HANDLE>
    class HandlePool
    {
    public:
        typedef HANDLE handle_type;

        explicit HandlePool(UInt32 size = 0);

        // Creation/Deletion
        HANDLE                          acquire(HANDLE handle = InvalidMemoryHandle());
        void                            release(HANDLE handle);
        Bool                            isAcquired(HANDLE handle) const;
        UInt32                          getNumberOfAcquired() const;

        UInt32                          size() const;
        void                            resize(UInt32 size);

        static HANDLE                   InvalidMemoryHandle();

    protected:
        HANDLE acquireInternal(MemoryHandle handle);

        std::vector<UInt8> m_handlePool;
        MemoryHandle  m_nextAvailableHint;
        UInt32        m_numberOfAcquired;
    };

    template <typename HANDLE>
    HandlePool<HANDLE>::HandlePool(UInt32 size)
        : m_handlePool(size)
        , m_nextAvailableHint(0u)
        , m_numberOfAcquired(0u)
    {
    }

    template <typename HANDLE>
    HANDLE HandlePool<HANDLE>::acquire(HANDLE handle)
    {
        const UInt poolSize = m_handlePool.size();
        if (handle == InvalidMemoryHandle())
        {
            if (m_numberOfAcquired < poolSize)
            {
                // search for available handle
                for (MemoryHandle i = m_nextAvailableHint; i < poolSize; ++i)
                {
                    if (m_handlePool[i] == 0)
                    {
                        return acquireInternal(i);
                    }
                }
                for (MemoryHandle i = 0u; i < m_nextAvailableHint && i < poolSize; ++i)
                {
                    if (m_handlePool[i] == 0)
                    {
                        return acquireInternal(i);
                    }
                }
            }

            // allocate and acquire new handle
            const MemoryHandle newHandle = static_cast<MemoryHandle>(m_handlePool.size());
            m_handlePool.resize(newHandle + 1u);
            return acquireInternal(newHandle);
        }

        const MemoryHandle memoryHandle = AsMemoryHandle(handle);
        if (memoryHandle >= poolSize)
        {
            resize(memoryHandle + 1u);
        }

        return acquireInternal(memoryHandle);
    }

    template <typename HANDLE>
    HANDLE HandlePool<HANDLE>::acquireInternal(MemoryHandle handle)
    {
        assert(handle < m_handlePool.size());
        assert(m_handlePool[handle] == 0);
        m_nextAvailableHint = handle + 1u;
        m_handlePool[handle] = std::numeric_limits<UInt8>::max();
        ++m_numberOfAcquired;
        return HANDLE(handle);
    }

    template <typename HANDLE>
    void HandlePool<HANDLE>::release(HANDLE handle)
    {
        const MemoryHandle memoryHandle = AsMemoryHandle(handle);
        assert(memoryHandle < m_handlePool.size());
        assert(m_handlePool[memoryHandle] != 0);
        m_handlePool[memoryHandle] = 0;
        m_nextAvailableHint = memoryHandle;
        assert(m_numberOfAcquired > 0u);
        --m_numberOfAcquired;
    }

    template <typename HANDLE>
    Bool HandlePool<HANDLE>::isAcquired(HANDLE handle) const
    {
        const MemoryHandle memoryHandle = AsMemoryHandle(handle);
        return (memoryHandle < m_handlePool.size()) && (m_handlePool[memoryHandle] != 0);
    }


    template <typename HANDLE>
    UInt32 ramses_internal::HandlePool<HANDLE>::getNumberOfAcquired() const
    {
        return m_numberOfAcquired;
    }

    template <typename HANDLE>
    UInt32 HandlePool<HANDLE>::size() const
    {
        return static_cast<UInt32>(m_handlePool.size());
    }

    template <typename HANDLE>
    void HandlePool<HANDLE>::resize(UInt32 size)
    {
        m_handlePool.resize(size);
    }

    template <typename HANDLE>
    HANDLE HandlePool<HANDLE>::InvalidMemoryHandle()
    {
        return std::numeric_limits<HANDLE>::max();
    }
}

#endif
