//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "ClientCommands/SceneCommandBuffer.h"
#include "ClientCommands/SceneCommandTypes.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "Utils/ThreadBarrier.h"
#include "Collections/Vector.h"

using namespace testing;

namespace ramses_internal
{
    class SceneCommandBufferTest : public ::testing::Test
    {
    public:
        SceneCommandBuffer buffer;
    };

    TEST_F(SceneCommandBufferTest, BufferIsInitiallyEmpty)
    {
        SceneCommandContainer container;
        buffer.exchangeContainerData(container);
        EXPECT_EQ(0u, container.getTotalCommandCount());
    }


    TEST_F(SceneCommandBufferTest, CanAddAndGetCommands)
    {
        const String name1("cmd1");
        const String name2("cmd2");

        ForceFallbackCommand ffcCmd1;
        ffcCmd1.streamTextureName = name1;
        ffcCmd1.forceFallback     = false;

        ForceFallbackCommand ffcCmd2;
        ffcCmd2.streamTextureName = name2;
        ffcCmd2.forceFallback     = true;

        SceneCommand& cmd1 = ffcCmd1;
        SceneCommand& cmd2 = ffcCmd2;

        buffer.enqueueCommand(cmd1);
        buffer.enqueueCommand(cmd2);

        SceneCommandContainer container;
        buffer.exchangeContainerData(container);

        ASSERT_EQ(2u, container.getTotalCommandCount());
        EXPECT_EQ(ESceneCommand_ForceFallbackImage ,container.getCommandType(0));
        EXPECT_EQ(name1 ,container.getCommandData<ForceFallbackCommand>(0).streamTextureName);
        EXPECT_FALSE(container.getCommandData<ForceFallbackCommand>(0).forceFallback);
        EXPECT_EQ(ESceneCommand_ForceFallbackImage ,container.getCommandType(1));
        EXPECT_EQ(name2 ,container.getCommandData<ForceFallbackCommand>(1).streamTextureName);
        EXPECT_TRUE(container.getCommandData<ForceFallbackCommand>(1).forceFallback);
    }

    class BufferInserter : public Runnable
    {
    public:
        BufferInserter(SceneCommandBuffer& _buffer, ThreadBarrier& _barrier, uint32_t _iterations)
        : buffer(_buffer)
        , iterations(_iterations)
        , barrier(_barrier)
        {}

        BufferInserter(const BufferInserter& rhs)
        : buffer(rhs.buffer)
        , iterations(rhs.iterations)
        , barrier(rhs.barrier)
        {}

        void run()
        {
            barrier.wait();
            for(uint32_t i = 0; i < iterations; ++i)
            {
                ForceFallbackCommand cmd;
                buffer.enqueueCommand(cmd);
            }
        }

    private:
        SceneCommandBuffer& buffer;
        uint32_t            iterations;
        ThreadBarrier&      barrier;
    };

    class BufferSwapper : public Runnable
    {
    public:
        BufferSwapper(SceneCommandBuffer& _buffer, ThreadBarrier& _barrier, uint32_t _iterations = 1u)
        : buffer(_buffer)
        , iterations(_iterations)
        , barrier(_barrier)
        {}

        BufferSwapper(const BufferSwapper& rhs)
        : buffer(rhs.buffer)
        , iterations(rhs.iterations)
        , barrier(rhs.barrier)
        {}

        void run()
        {
            barrier.wait();
            while(iterations > 0u)
            {
                buffer.exchangeContainerData(container);

                if(--iterations > 0u)
                {
                    PlatformThread::Sleep(1u);
                }
            }
        }

        uint32_t getNumberOfCommands() const
        {
            return container.getTotalCommandCount();
        }

    private:
        SceneCommandBuffer&   buffer;
        uint32_t              iterations;
        ThreadBarrier&        barrier;
        SceneCommandContainer container;
    };

    TEST_F(SceneCommandBufferTest, AccessBufferFromMultipleThreads)
    {
        const uint32_t NumberOfInserterSwapperThreads    = 23u;
        const uint32_t NumberOfInsertIterationsPerThread = 500u;
        const uint32_t NumberOfSwapIterationsPerThread   = 9u;
        const uint32_t NumberOfExpectedCommands          = NumberOfInsertIterationsPerThread * NumberOfInserterSwapperThreads;
        const uint32_t TotalNumberOfThreads              = 2 * NumberOfInserterSwapperThreads;

        ThreadBarrier barrier(TotalNumberOfThreads);
        std::vector<BufferInserter> inserter(NumberOfInserterSwapperThreads, BufferInserter(buffer, barrier, NumberOfInsertIterationsPerThread));
        std::vector<BufferSwapper>  swapper(NumberOfInserterSwapperThreads, BufferSwapper(buffer, barrier, NumberOfSwapIterationsPerThread));
        PlatformThread* threads[TotalNumberOfThreads];
        for(uint32_t i = 0; i < TotalNumberOfThreads; ++i)
        {
            threads[i] = new PlatformThread("thread");
        }

        for(uint32_t i = 0; i < NumberOfInserterSwapperThreads; ++i)
        {
            threads[i*2]->start(inserter[i]);
            threads[i*2+1]->start(swapper[i]);
        }

        for(uint32_t i = 0; i < NumberOfInserterSwapperThreads; ++i)
        {
            threads[i*2]->join();
            threads[i*2+1]->join();
        }

        uint32_t numberOfCollectedCommands = 0u;
        for(uint32_t i = 0; i < NumberOfInserterSwapperThreads; ++i)
        {
            numberOfCollectedCommands += swapper[i].getNumberOfCommands();
        }

        BufferSwapper lastSwap(buffer, barrier);
        lastSwap.run();
        numberOfCollectedCommands += lastSwap.getNumberOfCommands();

        EXPECT_EQ(NumberOfExpectedCommands, numberOfCollectedCommands);

        for(uint32_t i = 0; i < TotalNumberOfThreads; ++i)
        {
            delete threads[i];
        }
    }
}
