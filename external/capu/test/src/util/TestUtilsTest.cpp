/*
 * Copyright (C) 2012 BMW Car IT GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "TestUtilsTest.h"
#include "ramses-capu/util/Runnable.h"
#include "ramses-capu/os/Thread.h"

namespace ramses_capu
{

    class TestUtilsTestRunnable: public Runnable
    {
    public:

        TestUtilsTestRunnable()
            : executed(false)
        {

        }

        void run()
        {
            Thread::Sleep(500); // wait a while before release
            executed = true;
            runnableExecuted.release();
        }

        ThreadSynchronizer runnableExecuted;
        bool executed;
    };

    class ITestUtilsTestInterface
    {
    public:
        virtual ~ITestUtilsTestInterface()
        {

        }
        virtual void doSomething() = 0;
    };

    class MockTestUtilsTestInterface: public ITestUtilsTestInterface
    {
    public:
        MOCK_METHOD0(doSomething, void());

        ThreadSynchronizer doSomethingCall;
    };


    class TestUtilsInterfaceRunnable: public Runnable
    {
    public:

        TestUtilsInterfaceRunnable(ITestUtilsTestInterface& testUtilsInterface)
            : somethingDone(false)
            , m_testInterface(testUtilsInterface)

        {

        }

        void run()
        {
            Thread::Sleep(500); // wait a while before release
            m_testInterface.doSomething();
            somethingDone = true;
        }

        bool somethingDone;

    private:
        ITestUtilsTestInterface& m_testInterface;

    };

    TestUtilsTest::TestUtilsTest()
    {

    }

    TestUtilsTest::~TestUtilsTest()
    {

    }

    void TestUtilsTest::SetUp()
    {

    }

    void TestUtilsTest::TearDown()
    {

    }

    TEST_F(TestUtilsTest, SyncWithThread)
    {
        TestUtilsTestRunnable testRunnable;
        Thread thread;

        thread.start(testRunnable);

        testRunnable.runnableExecuted.wait();

        thread.join();

        EXPECT_EQ(true, testRunnable.executed);
    }

    TEST_F(TestUtilsTest, SyncWithInterfaceCall)
    {
        MockTestUtilsTestInterface testInterface;
        TestUtilsInterfaceRunnable testRunnable(testInterface);
        Thread thread;

        EXPECT_CALL(testInterface, doSomething())
        .Times(testing::Exactly(1))
        .WillOnce(ReleaseSyncCall(&testInterface.doSomethingCall)); // register release of sync call

        thread.start(testRunnable);

        testInterface.doSomethingCall.wait();

        thread.join();

        EXPECT_EQ(true, testRunnable.somethingDone);
    }
}
