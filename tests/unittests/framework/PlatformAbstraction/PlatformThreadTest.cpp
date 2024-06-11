//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/PlatformAbstraction/PlatformThread.h"
#include "internal/Core/Utils/LogMacros.h"
#include "gtest/gtest.h"

namespace ramses::internal
{
    namespace
    {
        class TestRunnable : public Runnable
        {
        public:
            explicit TestRunnable(uint32_t value = 0)
                : m_ctorValue(value)
            {
            }

            void run() override
            {
                runValue = m_ctorValue;
                id = std::this_thread::get_id();
            }

            uint32_t runValue = 0;
            std::thread::id id;
        private:
            uint32_t m_ctorValue;
        };

        class ThreadCancelRunnable : public Runnable
        {
        public:
            void run() override
            {
                while (!isCancelRequested())
                {
                    PlatformThread::Sleep(10);
                }
            }
        };

        class LoggingRunnable : public Runnable
        {
        public:
            void run() override
            {
                LOG_INFO(CONTEXT_FRAMEWORK, "test");
            }
        };
    }

    class APlatformThread : public testing::Test
    {
    };


    TEST_F(APlatformThread, Constructor)
    {
        PlatformThread testThread("PlatformThrdTst");
        TestRunnable testRunnable(42);

        EXPECT_EQ(0u, testRunnable.runValue);

        testThread.start(testRunnable);
        testThread.join();

        EXPECT_EQ(42u, testRunnable.runValue);
    }

    TEST_F(APlatformThread, TwoThreadsAtOnce)
    {
        TestRunnable firstRunnable(1);
        TestRunnable secondRunnable(2);

        PlatformThread firstThread("PlatformThrdTst");
        PlatformThread secondThread("PlatformThrdTst");

        EXPECT_EQ(0u, firstRunnable.runValue);
        EXPECT_EQ(0u, secondRunnable.runValue);

        firstThread.start(firstRunnable);
        secondThread.start(secondRunnable);

        firstThread.join();
        secondThread.join();

        EXPECT_EQ(1u, firstRunnable.runValue);
        EXPECT_EQ(2u, secondRunnable.runValue);
    }

    TEST_F(APlatformThread, reuseThread)
    {
        TestRunnable runnable1(1);
        TestRunnable runnable2(42);

        {
            PlatformThread thread("");
            EXPECT_FALSE(thread.joinable());

            thread.start(runnable1);
            EXPECT_TRUE(thread.joinable());
            thread.join();

            EXPECT_FALSE(thread.joinable());
            EXPECT_EQ(1u, runnable1.runValue);

            thread.start(runnable2);
        }

        // thread should be joined
        EXPECT_EQ(42u, runnable2.runValue);
    }

    TEST_F(APlatformThread, startAndCancelTest)
    {
        PlatformThread thread("");
        {
            ThreadCancelRunnable runnable;
            thread.start(runnable);
            thread.cancel();

            // test blocks forever if cancel didn't work
            thread.join();
        }
        // should not crash
        thread.cancel();
    }

    TEST_F(APlatformThread, joinWithoutStartingIsOK)
    {
        PlatformThread thread("");
        // expect no crash
        thread.join();
    }

    TEST_F(APlatformThread, cancelWithoutRunnable)
    {
        PlatformThread thread("");
        thread.cancel();
        // no test as just no crash is expected
    }

    TEST(Thread, getId)
    {
        auto mainId = std::this_thread::get_id();
        TestRunnable firstRunnable;
        TestRunnable secondRunnable;
        PlatformThread firstThread("PlatformThrdTst");
        PlatformThread secondThread("PlatformThrdTst");

        firstThread.start(firstRunnable);
        secondThread.start(secondRunnable);
        firstThread.join();
        secondThread.join();

        EXPECT_NE(mainId, firstRunnable.id);
        EXPECT_NE(mainId, secondRunnable.id);
        EXPECT_NE(firstRunnable.id, secondRunnable.id);
    }

    TEST_F(APlatformThread, stateLifecycle)
    {
        PlatformThread thread("");
        ThreadCancelRunnable runnable;
        EXPECT_FALSE(thread.joinable());
        EXPECT_FALSE(thread.isRunning());
        thread.start(runnable);
        EXPECT_TRUE(thread.joinable());
        EXPECT_TRUE(thread.isRunning());
        thread.cancel();
        while (thread.isRunning())
            PlatformThread::Sleep(10);
        EXPECT_TRUE(thread.joinable());
        EXPECT_FALSE(thread.isRunning());
        thread.join();
        EXPECT_FALSE(thread.joinable());
        EXPECT_FALSE(thread.isRunning());
    }

    TEST_F(APlatformThread, setsNameToLogger)
    {
        std::string logMsg;
        GetRamsesLogger().setLogHandler([&logMsg](auto /*unused*/, auto /*unused*/, auto message) {
            logMsg = message;
            });

        // logging prefix name tests work with static thread_local variables, each test case needs to run in own thread to not affect other test cases
        std::thread t([&] {

            RamsesLoggerPrefixes::SetRamsesLoggerPrefixes("instName", "");

            LoggingRunnable runnable;

            PlatformThread thread{ "thrdName" };
            thread.start(runnable);
            thread.join();
            EXPECT_EQ("instName.thrdName: test", logMsg);

            PlatformThread thread2{ "thrdName2" };
            thread2.start(runnable);
            thread2.join();
            EXPECT_EQ("instName.thrdName2: test", logMsg);

            });
        t.join();

        GetRamsesLogger().setLogHandler(nullptr);
    }
}
