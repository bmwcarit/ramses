//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "RendererLib/DisplayThread.h"
#include "DisplayBundleMock.h"
#include "Utils/ThreadBarrier.h"
#include "Watchdog/ThreadAliveNotifierMock.h"

using namespace testing;
using namespace std::chrono_literals;
namespace ramses_internal
{
    class AThreadAliveHandlerExpectingOneRegisterAndUnregister final : public StrictMock<ThreadAliveNotifierMock>
    {
    public:
        AThreadAliveHandlerExpectingOneRegisterAndUnregister()
            : StrictMock<ThreadAliveNotifierMock>()
        {
            EXPECT_CALL(*this, registerThread()).WillOnce(Return(ThreadAliveNotifierMock::dummyThreadId));
            EXPECT_CALL(*this, unregisterThread(ThreadAliveNotifierMock::dummyThreadId)).Times(1);
        }
    };

    class ADisplayThread : public ::testing::Test
    {
    public:
        ADisplayThread()
            : m_displayBundleMock{ static_cast<StrictMock<DisplayBundleMock>&>(*m_sharedDisplayBundle) }
            , m_displayThread(m_sharedDisplayBundle, DisplayHandle{ 1u }, m_aliveHandlerMock)
        {
            m_displayThread.setLoopMode(ELoopMode::UpdateAndRender);
            m_displayThread.setMinFrameDuration(1000us);
        }

    protected:
        DisplayBundleShared m_sharedDisplayBundle{ std::make_unique<StrictMock<DisplayBundleMock>>() };
        StrictMock<DisplayBundleMock>& m_displayBundleMock;
        AThreadAliveHandlerExpectingOneRegisterAndUnregister m_aliveHandlerMock;

        std::atomic_uint32_t m_loopCount{ 0 }; // must outlive thread if used in its mock
        std::atomic_uint32_t m_notifyCounter{ 0 }; // must outlive thread if used in its mock
        std::atomic_uint32_t m_timeoutCounter{ 0 }; // must outlive thread if used in its mock
        DisplayThread m_displayThread;
    };

    TEST_F(ADisplayThread, canStartStopUpdating)
    {
        EXPECT_CALL(m_aliveHandlerMock, notifyAlive(ThreadAliveNotifierMock::dummyThreadId)).Times(AtLeast(9));
        EXPECT_CALL(m_aliveHandlerMock, calculateTimeout()).Times(AtLeast(0)).WillRepeatedly(Return(20ms));
        EXPECT_CALL(m_displayBundleMock, doOneLoop(ELoopMode::UpdateAndRender, _)).Times(AnyNumber()).WillRepeatedly(Invoke([&](auto, auto)
        {
            m_loopCount++;
        }));

        m_displayThread.startUpdating();
        // let it run few loops
        while (m_loopCount < 10)
            std::this_thread::sleep_for(std::chrono::milliseconds{ 10u });
        m_displayThread.stopUpdating();
    }

    TEST_F(ADisplayThread, canStartStopRightAfterEachOther)
    {
        EXPECT_CALL(m_displayBundleMock, doOneLoop(ELoopMode::UpdateAndRender, _)).Times(AnyNumber());
        EXPECT_CALL(m_aliveHandlerMock, notifyAlive(ThreadAliveNotifierMock::dummyThreadId)).Times(AnyNumber());
        EXPECT_CALL(m_aliveHandlerMock, calculateTimeout()).Times(AnyNumber()).WillRepeatedly(Return(20ms));
        m_displayThread.startUpdating();
        m_displayThread.stopUpdating();
        m_displayThread.startUpdating();
        m_displayThread.stopUpdating();
        m_displayThread.startUpdating();
        m_displayThread.stopUpdating();
    }

    TEST_F(ADisplayThread, willStopUpdating)
    {
        EXPECT_CALL(m_aliveHandlerMock, notifyAlive(ThreadAliveNotifierMock::dummyThreadId)).Times(AtLeast(9));
        EXPECT_CALL(m_aliveHandlerMock, calculateTimeout()).Times(AtLeast(0)).WillRepeatedly(Return(20ms));
        EXPECT_CALL(m_displayBundleMock, doOneLoop(ELoopMode::UpdateAndRender, _)).Times(AnyNumber()).WillRepeatedly(Invoke([&](auto, auto)
        {
            m_loopCount++;
        }));
        m_displayThread.startUpdating();
        while (m_loopCount < 10)
            std::this_thread::sleep_for(10ms);

        // stop updating
        m_displayThread.stopUpdating();

        uint32_t loopCount1 = m_loopCount;

        // loop count should stop increasing (at most one more loop if thread overloaded)
        EXPECT_TRUE(m_loopCount <= loopCount1 + 1);
    }

    TEST_F(ADisplayThread, willUpdateWithUpdateOnlyMode)
    {
        EXPECT_CALL(m_aliveHandlerMock, notifyAlive(ThreadAliveNotifierMock::dummyThreadId)).Times(AtLeast(9));
        EXPECT_CALL(m_aliveHandlerMock, calculateTimeout()).Times(AtLeast(0)).WillRepeatedly(Return(20ms));
        EXPECT_CALL(m_displayBundleMock, doOneLoop(ELoopMode::UpdateOnly, _)).Times(AnyNumber()).WillRepeatedly(Invoke([&](auto, auto)
        {
            m_loopCount++;
        }));
        m_displayThread.setLoopMode(ELoopMode::UpdateOnly);
        m_displayThread.startUpdating();

        while (m_loopCount < 10)
            std::this_thread::sleep_for(10ms);
    }

    TEST_F(ADisplayThread, willUpdateWithUpdateAndRenderMode)
    {
        EXPECT_CALL(m_aliveHandlerMock, notifyAlive(ThreadAliveNotifierMock::dummyThreadId)).Times(AtLeast(9));
        EXPECT_CALL(m_aliveHandlerMock, calculateTimeout()).Times(AtLeast(0)).WillRepeatedly(Return(20ms));
        EXPECT_CALL(m_displayBundleMock, doOneLoop(ELoopMode::UpdateAndRender, _)).Times(AnyNumber()).WillRepeatedly(Invoke([&](auto, auto)
        {
            m_loopCount++;
        }));
        m_displayThread.setLoopMode(ELoopMode::UpdateAndRender);
        m_displayThread.startUpdating();

        while (m_loopCount < 10)
            std::this_thread::sleep_for(10ms);
    }

    TEST_F(ADisplayThread, issuesAtLeastOneNotificationPerLoopOrPerWait)
    {
        EXPECT_CALL(m_aliveHandlerMock, notifyAlive(ThreadAliveNotifierMock::dummyThreadId)).Times(AtLeast(10)).WillRepeatedly([this](auto) { m_notifyCounter++; });
        EXPECT_CALL(m_aliveHandlerMock, calculateTimeout()).Times(AtLeast(0)).WillRepeatedly([this]()
            {
                m_timeoutCounter++;
                EXPECT_GE(m_notifyCounter, m_timeoutCounter + m_loopCount);
                return 20ms;
            });
        EXPECT_CALL(m_displayBundleMock, doOneLoop(ELoopMode::UpdateAndRender, _)).Times(AnyNumber()).WillRepeatedly(Invoke([&](auto, auto)
            {
                if (++m_loopCount == 10)
                    m_displayThread.stopUpdating();
                EXPECT_GE(m_notifyCounter, m_timeoutCounter + m_loopCount);
            }));

        m_displayThread.startUpdating();
        // let it run few loops
        while (m_loopCount < 10)
            std::this_thread::sleep_for(1ms);
    }
}
