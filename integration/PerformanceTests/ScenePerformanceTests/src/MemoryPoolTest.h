//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MEMORYPOOLTEST_H
#define RAMSES_MEMORYPOOLTEST_H

#include "PerformanceTestBase.h"
#include "Utils/ScopedPointer.h"
#include "Utils/MemoryPool.h"
#include "Utils/MemoryPoolExplicit.h"

class MemoryPoolTest : public PerformanceTestBase
{
public:
    enum
    {
        MemoryPoolTest_AddRemoveSequential = 0,
        MemoryPoolTest_AddRemoveSequential_Preallocated,
        MemoryPoolTest_AddRemoveShuffled,
        MemoryPoolTest_AddRemoveShuffled_Preallocated,
        MemoryPoolTest_AddRemoveInterleaved,
        MemoryPoolTest_AddRemoveInterleaved_Preallocated,
        MemoryPoolTest_Explicit_AddRemoveShuffled,
        MemoryPoolTest_Explicit_AddRemoveShuffled_Preallocated,
        MemoryPoolTest_Explicit_AddRemoveInterleaved,
        MemoryPoolTest_Explicit_AddRemoveInterleaved_Preallocated
    };

    MemoryPoolTest(ramses_internal::String testName, uint32_t testState);

    virtual void initTest(ramses::RamsesClient& client, ramses::Scene& scene) override;
    virtual void preUpdate() override;
    virtual void update() override;

private:
    static const uint32_t NumberOfElements = 10000u;

    struct DummyObject
    {
        ramses_internal::UInt64 someHandle;
        ramses_internal::UInt64 someHandle2;
        ramses_internal::UInt32 someEnum;
        bool someFlag;
    };

    class IMemoryPool
    {
    public:
        virtual ~IMemoryPool() {}
        virtual void allocate(uint32_t handle) = 0;
        virtual void release(uint32_t handle) = 0;
        virtual bool isAllocated(uint32_t handle) const = 0;
        virtual uint32_t getTotalCount() const = 0;
    };

    template <typename T>
    class MemoryPoolWrapper : public IMemoryPool
    {
    public:
        MemoryPoolWrapper(uint32_t size = 0u)
            : m_memoryPool(size)
        {
        }

        virtual void allocate(uint32_t handle) final
        {
            m_memoryPool.allocate(handle);
        }
        virtual void release(uint32_t handle) final
        {
            m_memoryPool.release(handle);
        }
        virtual bool isAllocated(uint32_t handle) const final
        {
            return m_memoryPool.isAllocated(handle);
        }
        virtual uint32_t getTotalCount() const final
        {
            return m_memoryPool.getTotalCount();
        }

    private:
        T m_memoryPool;
    };

    typedef ramses_internal::MemoryPool<DummyObject, uint32_t> PoolType;
    typedef ramses_internal::MemoryPoolExplicit<DummyObject, uint32_t> ExplicitPoolType;
    ramses_internal::ScopedPointer<IMemoryPool> m_memoryPool;

    std::vector<uint32_t> m_shuffledHandles;
};

#endif
