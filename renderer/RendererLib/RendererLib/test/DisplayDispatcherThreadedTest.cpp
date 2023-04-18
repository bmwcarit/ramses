//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "DisplayDispatcherMock.h"
#include "RendererSceneEventSenderMock.h"
#include "Watchdog/ThreadAliveNotifierMock.h"
#include "Utils/ThreadBarrier.h"
#include <chrono>

using namespace testing;
using namespace std::chrono_literals;

namespace ramses_internal
{
    template <template<typename> class MOCK_TYPE>
    class ADisplayDispatcherThreaded : public ::testing::Test
    {
    public:
        ADisplayDispatcherThreaded()
            : m_displayDispatcher(RendererConfig{}, m_sceneEventSender, m_notifier, true)
        {
            m_displayDispatcher.setLoopMode(ELoopMode::UpdateOnly);
            m_displayDispatcher.m_expectedLoopModeForNewDisplays = ELoopMode::UpdateOnly;
            m_displayDispatcher.m_expectedMinFrameDurationForNextDisplayCreation = DefaultMinFrameDuration;
            m_displayDispatcher.m_expectNewDisplaysToStartUpdating = true;

            // put dispatcher into threaded mode - simulates user starting thread on API
            m_displayDispatcher.startDisplayThreadsUpdating();
        }

        void update()
        {
            m_displayDispatcher.dispatchCommands(m_commandBuffer);
        }

        void createDisplay(DisplayHandle displayHandle)
        {
            m_commandBuffer.enqueueCommand(RendererCommand::CreateDisplay{ displayHandle, DisplayConfig{}, nullptr });

            EXPECT_CALL(m_displayDispatcher, createDisplayBundle(displayHandle));
            update();
            ASSERT_TRUE(m_displayDispatcher.getDisplayBundleMock<MOCK_TYPE>(displayHandle) != nullptr);
            ASSERT_TRUE(m_displayDispatcher.getDisplayThreadMock<MOCK_TYPE>(displayHandle) != nullptr);
        }

        void destroyDisplay(DisplayHandle displayHandle)
        {
            m_commandBuffer.enqueueCommand(RendererCommand::DestroyDisplay{ displayHandle });

            expectCommandPushed(displayHandle, RendererCommand::DestroyDisplay{});
            update();
            EXPECT_TRUE(m_displayDispatcher.getDisplayBundleMock<MOCK_TYPE>(displayHandle) != nullptr);

            // display thread is actually destroyed only after successful destroy event emitted
            EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock<MOCK_TYPE>(displayHandle), dispatchRendererEvents(_)).WillOnce(Invoke([displayHandle](auto& evts)
            {
                RendererEvent evt{ ERendererEventType::DisplayDestroyed };
                evt.displayHandle = displayHandle;
                evts.push_back(evt);
            }));
            RendererEventVector events;
            m_displayDispatcher.dispatchRendererEvents(events);
            ASSERT_EQ(1u, events.size());
            EXPECT_EQ(ERendererEventType::DisplayDestroyed, events.front().eventType);
            EXPECT_EQ(displayHandle, events.front().displayHandle);

            EXPECT_TRUE(m_displayDispatcher.getDisplayBundleMock<MOCK_TYPE>(displayHandle) == nullptr);
            ASSERT_TRUE(m_displayDispatcher.getDisplayThreadMock<MOCK_TYPE>(displayHandle) == nullptr);
        }

        template <typename T>
        void expectCommandPushed(DisplayHandle displayHandle, T&& expectedCmd)
        {
            EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock<MOCK_TYPE>(displayHandle), pushAndConsumeCommands(_)).WillOnce(Invoke([c = std::forward<T>(expectedCmd)](auto& cmds)
            {
                ASSERT_EQ(1u, cmds.size());
                const RendererCommand::Variant v{ std::move(c) };
                EXPECT_EQ(v.index(), cmds.front().index());
                cmds.clear();
            }));
        }

    protected:
        StrictMock<RendererSceneEventSenderMock> m_sceneEventSender;
        RendererCommandBuffer m_commandBuffer;
        StrictMock<DisplayDispatcherFacade> m_displayDispatcher;
        NiceMock<ThreadAliveNotifierMock> m_notifier;
    };

    using ADisplayDispatcherThreadedStrict = ADisplayDispatcherThreaded<StrictMock>;
    using ADisplayDispatcherThreadedNice = ADisplayDispatcherThreaded<NiceMock>;

    TEST_F(ADisplayDispatcherThreadedStrict, canCreateDisplay)
    {
        createDisplay(DisplayHandle{ 1u });
    }

    TEST_F(ADisplayDispatcherThreadedStrict, canDestroyDisplay)
    {
        createDisplay(DisplayHandle{ 1u });
        destroyDisplay(DisplayHandle{ 1u });
    }

    TEST_F(ADisplayDispatcherThreadedStrict, canChangeLoopModeForAllDisplays)
    {
        constexpr DisplayHandle display1{ 1u };
        constexpr DisplayHandle display2{ 2u };
        createDisplay(display1);
        createDisplay(display2);

        EXPECT_CALL(*m_displayDispatcher.getDisplayThreadMock(display1), setLoopMode(ELoopMode::UpdateAndRender));
        EXPECT_CALL(*m_displayDispatcher.getDisplayThreadMock(display2), setLoopMode(ELoopMode::UpdateAndRender));
        m_displayDispatcher.setLoopMode(ELoopMode::UpdateAndRender);
    }

    TEST_F(ADisplayDispatcherThreadedStrict, newDisplayWillUseLastSetLoopMode)
    {
        constexpr DisplayHandle display1{ 1u };
        constexpr DisplayHandle display2{ 2u };
        createDisplay(display1);

        EXPECT_CALL(*m_displayDispatcher.getDisplayThreadMock(display1), setLoopMode(ELoopMode::UpdateAndRender));
        m_displayDispatcher.setLoopMode(ELoopMode::UpdateAndRender);
        m_displayDispatcher.m_expectedLoopModeForNewDisplays = ELoopMode::UpdateAndRender;

        createDisplay(display2);
    }

    TEST_F(ADisplayDispatcherThreadedStrict, newDisplaysUseDefaultMinFrameDuration)
    {
        m_displayDispatcher.m_expectedMinFrameDurationForNextDisplayCreation = DefaultMinFrameDuration;

        constexpr DisplayHandle display1{ 1u };
        constexpr DisplayHandle display2{ 2u };
        createDisplay(display1);
        createDisplay(display2);

        EXPECT_EQ(DefaultMinFrameDuration, m_displayDispatcher.getMinFrameDuration(display1));
        EXPECT_EQ(DefaultMinFrameDuration, m_displayDispatcher.getMinFrameDuration(display2));
    }

    TEST_F(ADisplayDispatcherThreadedStrict, canChangeMinFrameDuration)
    {
        constexpr DisplayHandle display1{ 1u };
        constexpr DisplayHandle display2{ 2u };
        createDisplay(display1);
        createDisplay(display2);
        EXPECT_EQ(DefaultMinFrameDuration, m_displayDispatcher.getMinFrameDuration(display1));
        EXPECT_EQ(DefaultMinFrameDuration, m_displayDispatcher.getMinFrameDuration(display2));

        constexpr std::chrono::microseconds minFrame1{ 50 };
        constexpr std::chrono::microseconds minFrame2{ 20 };

        EXPECT_CALL(*m_displayDispatcher.getDisplayThreadMock(display1), setMinFrameDuration(minFrame1));
        m_displayDispatcher.setMinFrameDuration(minFrame1, display1);
        EXPECT_CALL(*m_displayDispatcher.getDisplayThreadMock(display2), setMinFrameDuration(minFrame2));
        m_displayDispatcher.setMinFrameDuration(minFrame2, display2);

        EXPECT_EQ(minFrame1, m_displayDispatcher.getMinFrameDuration(display1));
        EXPECT_EQ(minFrame2, m_displayDispatcher.getMinFrameDuration(display2));
    }

    TEST_F(ADisplayDispatcherThreadedStrict, newDisplayWillUsePreviouslySetFrameDuration)
    {
        constexpr DisplayHandle display1{ 1u };
        constexpr DisplayHandle display2{ 2u };

        constexpr std::chrono::microseconds minFrame1{ 50 };
        constexpr std::chrono::microseconds minFrame2{ 20 };

        // displays not created yet but can set FPS limit already, and independently from each other
        m_displayDispatcher.setMinFrameDuration(minFrame1, display1);
        m_displayDispatcher.setMinFrameDuration(minFrame2, display2);
        EXPECT_EQ(minFrame1, m_displayDispatcher.getMinFrameDuration(display1));
        EXPECT_EQ(minFrame2, m_displayDispatcher.getMinFrameDuration(display2));

        m_displayDispatcher.m_expectedMinFrameDurationForNextDisplayCreation = minFrame1;
        createDisplay(display1);

        m_displayDispatcher.m_expectedMinFrameDurationForNextDisplayCreation = minFrame2;
        createDisplay(display2);

        EXPECT_EQ(minFrame1, m_displayDispatcher.getMinFrameDuration(display1));
        EXPECT_EQ(minFrame2, m_displayDispatcher.getMinFrameDuration(display2));
    }

    TEST_F(ADisplayDispatcherThreadedStrict, canGetMinFrameDurationEvenIfDisplayDestroyed) // not a useful feature but documented behavior
    {
        constexpr DisplayHandle display1{ 1u };
        constexpr std::chrono::microseconds minFrame1{ 50 };

        m_displayDispatcher.m_expectedMinFrameDurationForNextDisplayCreation = DefaultMinFrameDuration;
        createDisplay(display1);

        EXPECT_CALL(*m_displayDispatcher.getDisplayThreadMock(display1), setMinFrameDuration(minFrame1));
        m_displayDispatcher.setMinFrameDuration(minFrame1, display1);
        EXPECT_EQ(minFrame1, m_displayDispatcher.getMinFrameDuration(display1));

        destroyDisplay(display1);
        EXPECT_EQ(minFrame1, m_displayDispatcher.getMinFrameDuration(display1));
    }

    TEST_F(ADisplayDispatcherThreadedStrict, canStartStopUpdatingAllDisplays)
    {
        constexpr DisplayHandle display1{ 1u };
        constexpr DisplayHandle display2{ 2u };
        createDisplay(display1);
        createDisplay(display2);

        EXPECT_CALL(*m_displayDispatcher.getDisplayThreadMock(display1), stopUpdating());
        EXPECT_CALL(*m_displayDispatcher.getDisplayThreadMock(display2), stopUpdating());
        m_displayDispatcher.stopDisplayThreadsUpdating();

        EXPECT_CALL(*m_displayDispatcher.getDisplayThreadMock(display1), startUpdating());
        EXPECT_CALL(*m_displayDispatcher.getDisplayThreadMock(display2), startUpdating());
        m_displayDispatcher.startDisplayThreadsUpdating();
    }

    TEST_F(ADisplayDispatcherThreadedStrict, willNotStartUpdatingForNewDisplayWhenUpdatingStopped)
    {
        constexpr DisplayHandle display1{ 1u };
        constexpr DisplayHandle display2{ 2u };
        createDisplay(display1);

        EXPECT_CALL(*m_displayDispatcher.getDisplayThreadMock(display1), stopUpdating());
        m_displayDispatcher.stopDisplayThreadsUpdating();
        m_displayDispatcher.m_expectNewDisplaysToStartUpdating = false;

        createDisplay(display2);
    }

    TEST_F(ADisplayDispatcherThreadedNice, canCollectEventsFromAnotherThreadWhileCreatingNewDisplays)
    {
        ThreadBarrier started(2);

        // due to threading there will be calls right after creation, not purpose of this test, just ignore them
        m_displayDispatcher.m_useNiceMock = true;
        EXPECT_CALL(m_displayDispatcher, createDisplayBundle(_)).Times(AnyNumber());

        std::thread t([&]
        {
            started.wait();
            for (int i = 0; i < 20; ++i)
            {
                RendererEventVector evts;
                m_displayDispatcher.dispatchRendererEvents(evts);
            }
        });

        started.wait();

        DisplayHandle display{ 1u };
        for (int i = 0; i < 20; ++i)
        {
            m_commandBuffer.enqueueCommand(RendererCommand::CreateDisplay{ display++, DisplayConfig{}, nullptr });
            m_displayDispatcher.dispatchCommands(m_commandBuffer);
        }

        t.join();
    }

    TEST_F(ADisplayDispatcherThreadedNice, canControlUpdatingFromAnotherThreadWhileCreatingNewDisplays)
    {
        ThreadBarrier started(2);

        // due to threading there will be calls right after creation, not purpose of this test, just ignore them
        m_displayDispatcher.m_useNiceMock = true;
        EXPECT_CALL(m_displayDispatcher, createDisplayBundle(_)).Times(AnyNumber());

        std::thread t([&]
        {
            started.wait();
            for (int i = 0; i < 20; ++i)
            {
                m_displayDispatcher.stopDisplayThreadsUpdating();
                m_displayDispatcher.startDisplayThreadsUpdating();
                m_displayDispatcher.setMinFrameDuration(DefaultMinFrameDuration, DisplayHandle{ 1u });
                m_displayDispatcher.setLoopMode(ELoopMode::UpdateOnly);
            }
        });

        started.wait();

        DisplayHandle display{ 1u };
        for (int i = 0; i < 20; ++i)
        {
            m_commandBuffer.enqueueCommand(RendererCommand::CreateDisplay{ display++, DisplayConfig{}, nullptr });
            m_displayDispatcher.dispatchCommands(m_commandBuffer);
        }

        t.join();
    }
}
