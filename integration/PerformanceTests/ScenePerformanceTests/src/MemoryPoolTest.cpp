//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "MemoryPoolTest.h"
#include "PerformanceTestUtils.h"

MemoryPoolTest::MemoryPoolTest(ramses_internal::String testName, uint32_t testState)
    : PerformanceTestBase(testName, testState)
    , m_shuffledHandles(NumberOfElements)
{
    for (uint32_t i = 0u; i < NumberOfElements; ++i)
    {
        m_shuffledHandles[i] = i;
    }
    PerformanceTestUtils::ShuffleObjectList(m_shuffledHandles);

    switch (m_testState)
    {
    case MemoryPoolTest_Explicit_AddRemoveShuffled:
    case MemoryPoolTest_Explicit_AddRemoveInterleaved:
    case MemoryPoolTest_Explicit_AddRemoveShuffled_Preallocated:
    case MemoryPoolTest_Explicit_AddRemoveInterleaved_Preallocated:
        m_memoryPool.reset(new MemoryPoolWrapper<ExplicitPoolType>(NumberOfElements));
        break;
    default:
        m_memoryPool.reset(new MemoryPoolWrapper<PoolType>);
        break;
    }
}

void MemoryPoolTest::initTest(ramses::RamsesClient& client, ramses::Scene& scene)
{
    UNUSED(client);
    UNUSED(scene);
}

void MemoryPoolTest::preUpdate()
{
    switch (m_testState)
    {
    case MemoryPoolTest_AddRemoveSequential:
    case MemoryPoolTest_AddRemoveShuffled:
    case MemoryPoolTest_AddRemoveInterleaved:
        m_memoryPool.reset(new MemoryPoolWrapper<PoolType>);
        break;
    case MemoryPoolTest_Explicit_AddRemoveShuffled:
    case MemoryPoolTest_Explicit_AddRemoveInterleaved:
        m_memoryPool.reset(new MemoryPoolWrapper<ExplicitPoolType>(NumberOfElements));
        break;
    default:
        const uint32_t count = m_memoryPool->getTotalCount();
        for (uint32_t i = 0u; i < count; ++i)
        {
            if (m_memoryPool->isAllocated(i))
            {
                m_memoryPool->release(i);
            }
        }
        break;
    }
}

void MemoryPoolTest::update()
{
    IMemoryPool& pool = *m_memoryPool;

    switch (m_testState)
    {
    case MemoryPoolTest_AddRemoveSequential:
    case MemoryPoolTest_AddRemoveSequential_Preallocated:
    {
        for (uint32_t i = 0u; i < NumberOfElements; ++i)
        {
            pool.allocate(PoolType::InvalidMemoryHandle());
        }
        for (uint32_t i = 0u; i < NumberOfElements; ++i)
        {
            pool.release(i);
        }
    }
        break;
    case MemoryPoolTest_AddRemoveShuffled:
    case MemoryPoolTest_AddRemoveShuffled_Preallocated:
    case MemoryPoolTest_Explicit_AddRemoveShuffled:
    case MemoryPoolTest_Explicit_AddRemoveShuffled_Preallocated:
    {
        for (uint32_t i = 0u; i < NumberOfElements; ++i)
        {
            pool.allocate(m_shuffledHandles[i]);
        }
        for (uint32_t i = 0u; i < NumberOfElements; ++i)
        {
            pool.release(m_shuffledHandles[i]);
        }
    }
        break;
    case MemoryPoolTest_AddRemoveInterleaved:
    case MemoryPoolTest_AddRemoveInterleaved_Preallocated:
    case MemoryPoolTest_Explicit_AddRemoveInterleaved:
    case MemoryPoolTest_Explicit_AddRemoveInterleaved_Preallocated:
    {
        uint32_t addIdx = 0u;
        uint32_t remIdx = 0u;

        for (uint32_t ii = 0u; ii < NumberOfElements / 50u; ++ii)
        {
            for (uint32_t i = 0u; i < 50u; ++i)
            {
                pool.allocate(m_shuffledHandles[addIdx++]);
            }
            for (uint32_t i = 0u; i < 30u; ++i)
            {
                pool.release(m_shuffledHandles[remIdx++]);
            }
        }
    }
        break;
    default:
        assert(false);
        break;
    }
}
