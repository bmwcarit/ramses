//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MEMORYTRACKING_H
#define RAMSES_MEMORYTRACKING_H

#include "PlatformAbstraction/PlatformMath.h"

class MemoryTracking
{
public:

    void addAllocation(uint64_t size)
    {
        if (m_isEnabled)
        {
            m_allocationCount++;
            m_totalAllocationsInBytes += size;
            m_smallestAllocationInBytes = ramses_internal::min(m_smallestAllocationInBytes, size);
            m_largestAllocationInBytes = ramses_internal::max(m_largestAllocationInBytes, size);
        }
    }

    void reset()
    {
        m_allocationCount = 0;
        m_totalAllocationsInBytes = 0;
        m_smallestAllocationInBytes = std::numeric_limits<uint64_t>::max();
        m_largestAllocationInBytes = 0;
        m_isEnabled = false;
    }

    static MemoryTracking& GetInstance()
    {
        static MemoryTracking instance;
        return instance;
    }

    void setEnabled(bool enabled)
    {
        m_isEnabled = enabled;
    }

    uint64_t getAllocationCount() const
    {
        return m_allocationCount;
    }

    uint64_t getTotalAllocationsInBytes() const
    {
        return m_totalAllocationsInBytes;
    }

    uint64_t getSmallestAllocationInBytes() const
    {
        return m_smallestAllocationInBytes;
    }

    uint64_t getLargestAllocationInBytes() const
    {
        return m_largestAllocationInBytes;
    }

private:

    MemoryTracking()
    {
        reset();
    }

    uint64_t m_allocationCount;
    uint64_t m_totalAllocationsInBytes;
    uint64_t m_smallestAllocationInBytes;
    uint64_t m_largestAllocationInBytes;
    bool m_isEnabled;
};

#endif
