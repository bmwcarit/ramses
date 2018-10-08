//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"
#include <PlatformAbstraction/PlatformThread.h>

namespace ramses_internal
{
    class PlatformThreadTest : public testing::Test
    {
    public:
        // Inner class which can act as a runnable for test purposes.
        class TestRunnable : public Runnable
        {
        public:
            TestRunnable()
                : m_runCount(0U)
            {
            }

            virtual void run() override
            {
                ++m_runCount;
            }

            UInt32 getRunCount() const
            {
                return m_runCount;
            }

        private:
            UInt32 m_runCount;
        };

        PlatformThreadTest()
            : m_testRunnable()
        {
        }

    protected:
        TestRunnable m_testRunnable;
    };


    TEST_F(PlatformThreadTest, Constructor)
    {
        PlatformThread testThread("PlatformThrdTst");

        EXPECT_EQ(0U, m_testRunnable.getRunCount());

        testThread.start(m_testRunnable);
        testThread.join();

        EXPECT_EQ(1U, m_testRunnable.getRunCount());
    }

    TEST_F(PlatformThreadTest, ConstructorNew)
    {
        PlatformThread pTestThread("PlatformThrdTst");

        EXPECT_EQ(0U, m_testRunnable.getRunCount());

        pTestThread.start(m_testRunnable);
        pTestThread.join();

        EXPECT_EQ(1U, m_testRunnable.getRunCount());
    }

    TEST_F(PlatformThreadTest, TwoThreadsAtOnce)
    {
        TestRunnable firstRunnable;
        TestRunnable secondRunnable;

        PlatformThread firstThread("PlatformThrdTst");
        PlatformThread secondThread("PlatformThrdTst");

        EXPECT_EQ(0U, firstRunnable.getRunCount());
        EXPECT_EQ(0U, secondRunnable.getRunCount());

        firstThread.start(firstRunnable);
        secondThread.start(secondRunnable);

        firstThread.join();
        secondThread.join();

        EXPECT_EQ(1U, firstRunnable.getRunCount());
        EXPECT_EQ(1U, secondRunnable.getRunCount());
    }
}
