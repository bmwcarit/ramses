//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Collections/BlockingQueue.h"
#include "PlatformAbstraction/PlatformThread.h"
#include <gtest/gtest.h>
#include <random>
#include <vector>
#include <utility>

namespace ramses_internal
{
    TEST(ABlockingQueue, canPushAndPopValue)
    {
        BlockingQueue<int32_t> queue;
        queue.push(3);
        int32_t val = 0;
        EXPECT_EQ(EStatus_RAMSES_OK, queue.pop(&val));
        EXPECT_EQ(3, val);
    }

    TEST(ABlockingQueue, canPushAndPopMultipleValuesAsFifo)
    {
        BlockingQueue<int32_t> queue;
        queue.push(3);
        queue.push(4);
        queue.push(5);
        int32_t val = 0;
        EXPECT_EQ(EStatus_RAMSES_OK, queue.pop(&val));
        EXPECT_EQ(3, val);
        EXPECT_EQ(EStatus_RAMSES_OK, queue.pop(&val));
        EXPECT_EQ(4, val);
        EXPECT_EQ(EStatus_RAMSES_OK, queue.pop(&val));
        EXPECT_EQ(5, val);
    }

    TEST(ABlockingQueue, hasTimeoutWhenPoppingEmptyQueue)
    {
        BlockingQueue<int32_t> queue;
        int32_t val;
        EXPECT_EQ(EStatus_RAMSES_TIMEOUT, queue.pop(&val, 10));
    }

    TEST(ABlockingQueue, isEmptyIfNoElementsInside)
    {
        BlockingQueue<int32_t> queue;
        EXPECT_TRUE(queue.empty());
        queue.push(3);
        EXPECT_FALSE(queue.empty());
        int32_t val;
        queue.pop(&val);
        EXPECT_TRUE(queue.empty());
    }

    namespace
    {
        class Producer: public Runnable
        {
        public:
            BlockingQueue<uint32_t>& mQueue;
            int32_t mCount;
            uint32_t mId;

            static std::atomic<uint32_t> mSum;

            Producer(BlockingQueue<uint32_t>& queue, int32_t count, uint32_t id)
                : mQueue(queue), mCount(count), mId(id)
            {
            }

            inline void run()
            {
                for (int32_t i = 0; i < mCount; ++i)
                {
                    uint32_t val = i + mId * 100 + 100;
                    mQueue.push(val);
                    uint32_t sleepTime = std::uniform_int_distribution<uint32_t>(0, 100)(randGenerator);
                    PlatformThread::Sleep(sleepTime);
                    mSum += val;
                }
            }

            std::mt19937 randGenerator{std::random_device{}()};
        };

        std::atomic<uint32_t> Producer::mSum(0);

        class Consumer: public Runnable
        {
        public:
            BlockingQueue<uint32_t>& mQueue;
            int32_t mCount;
            uint32_t mId;

            static std::atomic<uint32_t> mSum;

            Consumer(BlockingQueue<uint32_t>& queue, int32_t count, uint32_t id)
                : mQueue(queue), mCount(count), mId(id)
            {
            }

            inline void run()
            {
                for (int32_t i = 0; i < mCount; ++i)
                {
                    uint32_t tmp = 0;
                    EXPECT_EQ(EStatus_RAMSES_OK, mQueue.pop(&tmp));
                    mSum += tmp;
                }
            }
        };
        std::atomic<uint32_t> Consumer::mSum(0);
    }

    TEST(ABlockingQueue, confidence_canPushAndPopFromMultipleThreads)
    {
        BlockingQueue<uint32_t> queue;

        // countProducer * countPerProducer % countConsumer == 0 must be ensured!!!
        const int32_t countProducer = 20;
        const int32_t countConsumer = 100;
        const int32_t countPerProducer = 10;
        const int32_t count = countProducer * countPerProducer;
        const int32_t countPerConsumer = count / countConsumer;

        // this ensures correct test configuration
        ASSERT_EQ(0, countProducer * countPerProducer % countConsumer);

        std::vector<std::pair<ramses_capu::Thread*, ramses_capu::Runnable*>> consumer(countConsumer);
        for (int32_t i = 0; i < countConsumer; ++i)
        {
            consumer[i].second = new Consumer(queue, countPerConsumer, i + 20);
            consumer[i].first = new ramses_capu::Thread();
            consumer[i].first->start(*consumer[i].second);
        }

        std::vector<std::pair<ramses_capu::Thread*, ramses_capu::Runnable*>> producer(countProducer);
        for (int32_t i = 0; i < countProducer; ++i)
        {
            producer[i].second = new Producer(queue, countPerProducer, i + 10);
            producer[i].first = new ramses_capu::Thread();
            producer[i].first->start(*producer[i].second);
        }

        for (int32_t i = 0; i < countConsumer; ++i)
        {
            consumer[i].first->join();
            delete consumer[i].second;
            delete consumer[i].first;
        }
        for (int32_t i = 0; i < countProducer; ++i)
        {
            producer[i].first->join();
            delete producer[i].second;
            delete producer[i].first;
        }

        EXPECT_EQ(Producer::mSum.load(), Consumer::mSum.load());
    }
}
