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
#include <thread>

using namespace testing;

namespace ramses_internal
{
    class ADisplayDispatcher : public ::testing::Test
    {
    public:
        ADisplayDispatcher()
            : m_displayDispatcher(RendererConfig{}, m_sceneEventSender, m_notifier, false)
        {
            m_displayDispatcher.setLoopMode(ELoopMode::UpdateOnly);
            m_displayDispatcher.m_expectedLoopModeForNewDisplays = ELoopMode::UpdateOnly;
        }

        void update()
        {
            m_displayDispatcher.dispatchCommands(m_commandBuffer);

            for (const auto d : m_displayDispatcher.getDisplays())
            {
                if (m_displayDispatcher.getDisplays().size() > 1u)
                    EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(d), enableContext());
                EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(d), doOneLoop(_, _));
            }
            m_displayDispatcher.doOneLoop();
        }

        void createDisplay(DisplayHandle displayHandle)
        {
            m_commandBuffer.enqueueCommand(RendererCommand::CreateDisplay{ displayHandle, DisplayConfig{}, nullptr });

            EXPECT_CALL(m_displayDispatcher, createDisplayBundle(displayHandle));
            update();
            ASSERT_TRUE(m_displayDispatcher.getDisplayBundleMock(displayHandle) != nullptr);
        }

        void destroyDisplay(DisplayHandle displayHandle)
        {
            m_commandBuffer.enqueueCommand(RendererCommand::DestroyDisplay{ displayHandle });

            expectCommandPushed(displayHandle, RendererCommand::DestroyDisplay{});
            update();
            EXPECT_TRUE(m_displayDispatcher.getDisplayBundleMock(displayHandle) != nullptr);

            // display thread is actually destroyed only after successful destroy event emitted
            EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(displayHandle), dispatchRendererEvents(_)).WillOnce(Invoke([displayHandle](auto& evts)
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

            EXPECT_TRUE(m_displayDispatcher.getDisplayBundleMock(displayHandle) == nullptr);
        }

        template <typename T>
        void expectCommandPushed(DisplayHandle displayHandle, T&& expectedCmd)
        {
            EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(displayHandle), pushAndConsumeCommands(_)).WillOnce(Invoke([c = std::forward<T>(expectedCmd)](auto& cmds)
            {
                ASSERT_EQ(1u, cmds.size());
                const RendererCommand::Variant v{ std::move(c) };
                EXPECT_EQ(v.index(), cmds.front().index());
                cmds.clear();
            }));
        }

        void expectCommandsCountPushed(DisplayHandle displayHandle, size_t cmdCount)
        {
            EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(displayHandle), pushAndConsumeCommands(_)).WillOnce(Invoke([cmdCount](auto& cmds)
            {
                EXPECT_EQ(cmdCount, cmds.size());
                cmds.clear();
            }));
        }

        void expectNoCommandPushed(DisplayHandle displayHandle)
        {
            EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(displayHandle), pushAndConsumeCommands(_)).Times(0);
        }

        void simulateSceneStateEventFromDisplay(DisplayHandle display, SceneId sceneId, RendererSceneState state)
        {
            EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display), dispatchSceneControlEvents(_)).WillOnce(Invoke([sceneId, state](auto& evts)
            {
                RendererEvent evt{ ERendererEventType::SceneStateChanged, sceneId };
                evt.state = state;
                evts.push_back(std::move(evt));
            }));
        }

        void expectEvents(const RendererEventVector& expectedEvts, const RendererEventVector& evts)
        {
            ASSERT_EQ(expectedEvts.size(), evts.size());
            for (size_t i = 0; i < evts.size(); ++i)
                EXPECT_EQ(expectedEvts[i].eventType, evts[i].eventType);
        }

        void dispatchAndExpectRendererEvents(const RendererEventVector& expectedEvents)
        {
            RendererEventVector evts;
            m_displayDispatcher.dispatchRendererEvents(evts);
            expectEvents(expectedEvents, evts);
        }

        void dispatchAndExpectSceneControlEvents(const RendererEventVector& expectedEvents)
        {
            RendererEventVector evts;
            m_displayDispatcher.dispatchSceneControlEvents(evts);
            expectEvents(expectedEvents, evts);
        }

    protected:
        StrictMock<RendererSceneEventSenderMock> m_sceneEventSender;
        RendererCommandBuffer m_commandBuffer;
        StrictMock<DisplayDispatcherFacade> m_displayDispatcher;
        NiceMock<ThreadAliveNotifierMock> m_notifier;
    };

    TEST_F(ADisplayDispatcher, canCreateDisplay)
    {
        createDisplay(DisplayHandle{ 1u });
    }

    TEST_F(ADisplayDispatcher, canDestroyDisplay)
    {
        createDisplay(DisplayHandle{ 1u });
        destroyDisplay(DisplayHandle{ 1u });
    }

    TEST_F(ADisplayDispatcher, pushesCommandToItsDestinationDisplay)
    {
        constexpr DisplayHandle display1{ 1u };
        constexpr DisplayHandle display2{ 2u };
        createDisplay(display1);
        createDisplay(display2);

        m_commandBuffer.enqueueCommand(RendererCommand::CreateOffscreenBuffer{ display1, {}, {}, {}, {}, {}, ERenderBufferType_DepthStencilBuffer });
        m_commandBuffer.enqueueCommand(RendererCommand::DestroyOffscreenBuffer{ display2, {} });

        expectCommandPushed(display1, RendererCommand::CreateOffscreenBuffer{});
        expectCommandPushed(display2, RendererCommand::DestroyOffscreenBuffer{});
        update();

        expectNoCommandPushed(display1);
        expectNoCommandPushed(display2);
        update();
    }

    TEST_F(ADisplayDispatcher, pushesCommandToDisplayWhereItsSceneIsMapped)
    {
        constexpr DisplayHandle display1{ 1u };
        constexpr DisplayHandle display2{ 2u };
        constexpr SceneId scene1{ 11u };
        constexpr SceneId scene2{ 22u };

        createDisplay(display1);
        createDisplay(display2);

        m_commandBuffer.enqueueCommand(RendererCommand::SetSceneMapping{ scene1, display1 });
        m_commandBuffer.enqueueCommand(RendererCommand::SetSceneMapping{ scene2, display2 });
        expectCommandPushed(display1, RendererCommand::SetSceneMapping{});
        expectCommandPushed(display2, RendererCommand::SetSceneMapping{});
        update();

        m_commandBuffer.enqueueCommand(RendererCommand::SetSceneState{ scene1, {} });
        expectCommandPushed(display1, RendererCommand::SetSceneState{});
        expectNoCommandPushed(display2);
        update();

        m_commandBuffer.enqueueCommand(RendererCommand::SetSceneState{ scene2, {} });
        expectNoCommandPushed(display1);
        expectCommandPushed(display2, RendererCommand::SetSceneState{});
        update();

        expectNoCommandPushed(display1);
        expectNoCommandPushed(display2);
        update();
    }

    TEST_F(ADisplayDispatcher, willNotCallPushIfNoCommands)
    {
        constexpr DisplayHandle display1{ 1u };
        constexpr DisplayHandle display2{ 2u };
        createDisplay(display1);
        createDisplay(display2);

        m_commandBuffer.enqueueCommand(RendererCommand::DestroyOffscreenBuffer{ display2, {} });

        // no push for display1
        expectNoCommandPushed(display1);
        expectCommandPushed(display2, RendererCommand::DestroyOffscreenBuffer{});
        update();

        expectNoCommandPushed(display1);
        expectNoCommandPushed(display2);
        update();
    }

    TEST_F(ADisplayDispatcher, willGracefullyFailToPushCommandForUnknownDisplay)
    {
        constexpr DisplayHandle display1{ 1u };
        constexpr DisplayHandle display2{ 2u };
        createDisplay(display1);

        m_commandBuffer.enqueueCommand(RendererCommand::DestroyOffscreenBuffer{ display2, {} });

        expectNoCommandPushed(display1);
        update();
    }

    TEST_F(ADisplayDispatcher, pushesBroadcastCommandToAllDisplays)
    {
        constexpr DisplayHandle display1{ 1u };
        constexpr DisplayHandle display2{ 2u };
        createDisplay(display1);
        createDisplay(display2);

        m_commandBuffer.enqueueCommand(RendererCommand::ScenePublished{ {}, {} });

        expectCommandPushed(display1, RendererCommand::ScenePublished{});
        expectCommandPushed(display2, RendererCommand::ScenePublished{});
        update();

        expectNoCommandPushed(display1);
        expectNoCommandPushed(display2);
        update();
    }

    TEST_F(ADisplayDispatcher, pushesOldBroadcastCommandToAllNewlyCreatedDisplays)
    {
        constexpr DisplayHandle display1{ 1u };
        createDisplay(display1);

        m_commandBuffer.enqueueCommand(RendererCommand::ScenePublished{ {}, {} });
        expectCommandPushed(display1, RendererCommand::ScenePublished{});
        update();

        // will expect this command to be pushed to any new display
        m_displayDispatcher.m_expectedBroadcastCommandsForNewDisplays.push_back(RendererCommand::ScenePublished{});

        expectNoCommandPushed(display1);
        update();

        constexpr DisplayHandle display2{ 2u };
        createDisplay(display2);
        update();

        expectNoCommandPushed(display1);
        expectNoCommandPushed(display2);
        update();

        constexpr DisplayHandle display3{ 3u };
        createDisplay(display3);
        update();

        expectNoCommandPushed(display1);
        expectNoCommandPushed(display2);
        expectNoCommandPushed(display3);
        update();
    }

    TEST_F(ADisplayDispatcher, consolidatesBroadcastCommandBeforePushingToNewlyCreatedDisplay)
    {
        constexpr DisplayHandle display1{ 1u };
        createDisplay(display1);

        constexpr SceneId scene1{ 1u };
        constexpr SceneId scene2{ 2u };
        constexpr SceneId scene3{ 3u };

        // publish/unpublish strike out
        m_commandBuffer.enqueueCommand(RendererCommand::ScenePublished{ scene1, {} });
        m_commandBuffer.enqueueCommand(RendererCommand::ScenePublished{ scene2, {} });
        m_commandBuffer.enqueueCommand(RendererCommand::SceneUnpublished{ scene1 });
        m_commandBuffer.enqueueCommand(RendererCommand::ScenePublished{ scene3, {} });
        m_commandBuffer.enqueueCommand(RendererCommand::SceneUnpublished{ scene3 });
        m_commandBuffer.enqueueCommand(RendererCommand::ScenePublished{ scene1, {} });
        m_commandBuffer.enqueueCommand(RendererCommand::SceneUnpublished{ scene1 });
        m_commandBuffer.enqueueCommand(RendererCommand::ScenePublished{ scene3, {} });
        m_commandBuffer.enqueueCommand(RendererCommand::SceneUnpublished{ scene2 });
        // not stashed
        m_commandBuffer.enqueueCommand(RendererCommand::FrameProfiler_Toggle{});
        m_commandBuffer.enqueueCommand(RendererCommand::LogInfo{});
        m_commandBuffer.enqueueCommand(RendererCommand::LogStatistics{});
        // only last one kept
        m_commandBuffer.enqueueCommand(RendererCommand::SetLimits_FrameBudgets{});
        m_commandBuffer.enqueueCommand(RendererCommand::SetLimits_FrameBudgets{});
        m_commandBuffer.enqueueCommand(RendererCommand::SetLimits_FrameBudgets{});

        expectCommandsCountPushed(display1, 15u);
        update();

        // will expect these commands to be pushed to any new display
        m_displayDispatcher.m_expectedBroadcastCommandsForNewDisplays.push_back(RendererCommand::ScenePublished{ scene3, EScenePublicationMode::EScenePublicationMode_LocalOnly });
        m_displayDispatcher.m_expectedBroadcastCommandsForNewDisplays.push_back(RendererCommand::SetLimits_FrameBudgets{});

        expectNoCommandPushed(display1);
        update();

        constexpr DisplayHandle display2{ 2u };
        createDisplay(display2);
        update();

        expectNoCommandPushed(display1);
        expectNoCommandPushed(display2);
        update();
    }

    TEST_F(ADisplayDispatcher, pushesStashedCommandToNewDisplay)
    {
        constexpr DisplayHandle display1{ 1u };
        constexpr DisplayHandle display2{ 2u };
        constexpr SceneId scene1{ 11u };
        constexpr SceneId scene2{ 22u };

        // scene mapping/state arrives before display is created
        m_commandBuffer.enqueueCommand(RendererCommand::SetSceneMapping{ scene1, display1 });
        m_commandBuffer.enqueueCommand(RendererCommand::SetSceneState{ scene1, {} });
        m_commandBuffer.enqueueCommand(RendererCommand::SetSceneMapping{ scene2, display2 });
        m_commandBuffer.enqueueCommand(RendererCommand::SetSceneState{ scene2, {} });

        // display1 will receive stashed commands right after creation
        m_displayDispatcher.m_expectedCommandsForNextCreatedDisplay.push_back(RendererCommand::SetSceneMapping{});
        m_displayDispatcher.m_expectedCommandsForNextCreatedDisplay.push_back(RendererCommand::SetSceneState{});
        createDisplay(display1);

        // same for display2
        m_displayDispatcher.m_expectedCommandsForNextCreatedDisplay.clear();
        m_displayDispatcher.m_expectedCommandsForNextCreatedDisplay.push_back(RendererCommand::SetSceneMapping{});
        m_displayDispatcher.m_expectedCommandsForNextCreatedDisplay.push_back(RendererCommand::SetSceneState{});
        createDisplay(display2);

        expectNoCommandPushed(display1);
        expectNoCommandPushed(display2);
        update();
    }

    TEST_F(ADisplayDispatcher, dispatchesRendererEventsFromAllDisplays)
    {
        constexpr DisplayHandle display1{ 1u };
        constexpr DisplayHandle display2{ 2u };
        createDisplay(display1);
        createDisplay(display2);

        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), dispatchRendererEvents(_)).WillOnce(Invoke([](auto& evts)
        {
            evts.push_back(RendererEvent{ ERendererEventType::OffscreenBufferCreated });
        }));
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display2), dispatchRendererEvents(_)).WillOnce(Invoke([](auto& evts)
        {
            evts.push_back(RendererEvent{ ERendererEventType::OffscreenBufferDestroyed });
        }));
        dispatchAndExpectRendererEvents({ ERendererEventType::OffscreenBufferCreated, ERendererEventType::OffscreenBufferDestroyed });

        // no more events
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), dispatchRendererEvents(_));
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display2), dispatchRendererEvents(_));
        dispatchAndExpectRendererEvents({});
    }

    TEST_F(ADisplayDispatcher, dispatchesSceneControlEventsFromAllDisplays)
    {
        constexpr DisplayHandle display1{ 1u };
        constexpr DisplayHandle display2{ 2u };
        createDisplay(display1);
        createDisplay(display2);

        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), dispatchSceneControlEvents(_)).WillOnce(Invoke([](auto& evts)
        {
            evts.push_back(RendererEvent{ ERendererEventType::SceneDataSlotConsumerCreated });
        }));
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display2), dispatchSceneControlEvents(_)).WillOnce(Invoke([](auto& evts)
        {
            evts.push_back(RendererEvent{ ERendererEventType::SceneDataLinked });
        }));
        dispatchAndExpectSceneControlEvents({ ERendererEventType::SceneDataSlotConsumerCreated, ERendererEventType::SceneDataLinked });

        // no more events
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), dispatchSceneControlEvents(_));
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display2), dispatchSceneControlEvents(_));
        dispatchAndExpectSceneControlEvents({});
    }

    TEST_F(ADisplayDispatcher, dispatchesInjectedRendererEvents)
    {
        constexpr DisplayHandle display1{ 1u };
        constexpr DisplayHandle display2{ 2u };
        createDisplay(display1);
        createDisplay(display2);

        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), dispatchRendererEvents(_)).WillOnce(Invoke([](auto& evts)
        {
            evts.push_back(RendererEvent{ ERendererEventType::OffscreenBufferCreated });
        }));
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display2), dispatchRendererEvents(_)).WillOnce(Invoke([](auto& evts)
        {
            evts.push_back(RendererEvent{ ERendererEventType::OffscreenBufferDestroyed });
        }));

        m_displayDispatcher.injectRendererEvent(RendererEvent{ ERendererEventType::StreamSurfaceAvailable });

        dispatchAndExpectRendererEvents({ ERendererEventType::OffscreenBufferCreated, ERendererEventType::OffscreenBufferDestroyed, ERendererEventType::StreamSurfaceAvailable });

        // no more events
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), dispatchRendererEvents(_));
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display2), dispatchRendererEvents(_));
        dispatchAndExpectRendererEvents({});
    }

    TEST_F(ADisplayDispatcher, dispatchesInjectedSceneControlEvents)
    {
        constexpr DisplayHandle display1{ 1u };
        constexpr DisplayHandle display2{ 2u };
        createDisplay(display1);
        createDisplay(display2);

        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), dispatchSceneControlEvents(_)).WillOnce(Invoke([](auto& evts)
        {
            evts.push_back(RendererEvent{ ERendererEventType::SceneDataSlotConsumerCreated });
        }));
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display2), dispatchSceneControlEvents(_)).WillOnce(Invoke([](auto& evts)
        {
            evts.push_back(RendererEvent{ ERendererEventType::SceneDataLinked });
        }));

        m_displayDispatcher.injectSceneControlEvent(RendererEvent{ ERendererEventType::StreamSurfaceAvailable });

        dispatchAndExpectSceneControlEvents({ ERendererEventType::SceneDataSlotConsumerCreated, ERendererEventType::SceneDataLinked, ERendererEventType::StreamSurfaceAvailable });

        // no more events
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), dispatchSceneControlEvents(_));
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display2), dispatchSceneControlEvents(_));
        dispatchAndExpectSceneControlEvents({});
    }

    TEST_F(ADisplayDispatcher, willGenerateFailEventIfPushingCommandForUnknownDisplay)
    {
        constexpr DisplayHandle display1{ 1u };
        constexpr DisplayHandle display2{ 2u };
        createDisplay(display1);

        m_commandBuffer.enqueueCommand(RendererCommand::DestroyOffscreenBuffer{ display2, {} });
        m_commandBuffer.enqueueCommand(RendererCommand::CreateOffscreenBuffer{ display2, {}, {}, {}, {}, {}, ERenderBufferType_DepthStencilBuffer });
        expectNoCommandPushed(display1);
        update();

        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), dispatchRendererEvents(_));
        dispatchAndExpectRendererEvents({ ERendererEventType::OffscreenBufferDestroyFailed, ERendererEventType::OffscreenBufferCreateFailed });

        // no more events
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), dispatchRendererEvents(_));
        dispatchAndExpectRendererEvents({});
    }

    TEST_F(ADisplayDispatcher, willQueryMasterSceneForRefSceneIfUpdateSceneCommandNotOwnedByDisplay)
    {
        constexpr DisplayHandle display1{ 1u };
        createDisplay(display1);

        // set master scene mapping
        constexpr SceneId masterSceneId{ 22u };
        m_commandBuffer.enqueueCommand(RendererCommand::SetSceneMapping{ masterSceneId, display1 });
        expectCommandPushed(display1, RendererCommand::SetSceneMapping{});
        update();

        // ref scene has no mapping known
        constexpr SceneId refSceneId{ 33u };
        m_commandBuffer.enqueueCommand(RendererCommand::ReceiveScene{ SceneInfo{ refSceneId } });
        // its master will be queried
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), findMasterSceneForReferencedScene(refSceneId)).WillOnce(Return(masterSceneId));
        expectCommandPushed(display1, RendererCommand::ReceiveScene{});
        update();

        // no more commands/events
        expectNoCommandPushed(display1);
        update();
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), dispatchRendererEvents(_));
        dispatchAndExpectRendererEvents({});
    }

    TEST_F(ADisplayDispatcher, gracefullyFailsToQueryMasterSceneForRefSceneIfUpdateSceneCommandNotOwnedByDisplay)
    {
        constexpr DisplayHandle display1{ 1u };
        createDisplay(display1);

        // ref scene has no mapping known
        constexpr SceneId refSceneId{ 33u };
        m_commandBuffer.enqueueCommand(RendererCommand::ReceiveScene{ SceneInfo{ refSceneId } });
        // its master will be queried and not found
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), findMasterSceneForReferencedScene(refSceneId)).WillOnce(Return(SceneId::Invalid()));
        expectNoCommandPushed(display1);
        update();

        // no more commands/events
        expectNoCommandPushed(display1);
        update();
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), dispatchRendererEvents(_));
        dispatchAndExpectRendererEvents({});
    }

    TEST_F(ADisplayDispatcher, canChangeLoopModeForAllDisplays)
    {
        constexpr DisplayHandle display1{ 1u };
        constexpr DisplayHandle display2{ 2u };
        createDisplay(display1);
        createDisplay(display2);

        m_displayDispatcher.setLoopMode(ELoopMode::UpdateAndRender);

        InSequence seq;
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), enableContext());
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), doOneLoop(ELoopMode::UpdateAndRender, _));
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display2), enableContext());
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display2), doOneLoop(ELoopMode::UpdateAndRender, _));
        m_displayDispatcher.doOneLoop();
    }

    TEST_F(ADisplayDispatcher, newDisplayWillBeUpdatedWithLastSetLoopMode)
    {
        constexpr DisplayHandle display1{ 1u };
        constexpr DisplayHandle display2{ 2u };
        createDisplay(display1);

        m_displayDispatcher.setLoopMode(ELoopMode::UpdateAndRender);
        m_displayDispatcher.m_expectedLoopModeForNewDisplays = ELoopMode::UpdateAndRender;
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), doOneLoop(ELoopMode::UpdateAndRender, _));
        m_displayDispatcher.doOneLoop();

        createDisplay(display2);

        InSequence seq;
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), enableContext());
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), doOneLoop(ELoopMode::UpdateAndRender, _));
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display2), enableContext());
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display2), doOneLoop(ELoopMode::UpdateAndRender, _));
        m_displayDispatcher.doOneLoop();
    }

    TEST_F(ADisplayDispatcher, sceneAvailableEventIsEmittedExactlyOnce)
    {
        constexpr SceneId scene1{ 123u };
        constexpr DisplayHandle display1{ 1u };
        constexpr DisplayHandle display2{ 2u };

        createDisplay(display1);
        m_commandBuffer.enqueueCommand(RendererCommand::ScenePublished{ scene1, {} });
        expectCommandPushed(display1, RendererCommand::ScenePublished{});
        m_displayDispatcher.dispatchCommands(m_commandBuffer);

        simulateSceneStateEventFromDisplay(display1, scene1, RendererSceneState::Available);
        dispatchAndExpectSceneControlEvents({ ERendererEventType::SceneStateChanged });

        // will expect this command to be pushed to any new display
        m_displayDispatcher.m_expectedBroadcastCommandsForNewDisplays.push_back(RendererCommand::ScenePublished{ scene1, {} });

        createDisplay(display2);
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), dispatchSceneControlEvents(_)); // nothing from display1
        simulateSceneStateEventFromDisplay(display2, scene1, RendererSceneState::Available);
        // expect no redundant state change emitted
        dispatchAndExpectSceneControlEvents({});

        simulateSceneStateEventFromDisplay(display1, scene1, RendererSceneState::Unavailable);
        simulateSceneStateEventFromDisplay(display2, scene1, RendererSceneState::Unavailable);
        // unavailable emitted also just once
        dispatchAndExpectSceneControlEvents({ ERendererEventType::SceneStateChanged });

        // no more events
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), dispatchSceneControlEvents(_));
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display2), dispatchSceneControlEvents(_));
        dispatchAndExpectSceneControlEvents({});
    }

    TEST_F(ADisplayDispatcher, sceneAvailableEventIsEmittedExactlyOnce_withOwnershipChange)
    {
        constexpr SceneId scene1{ 123u };
        constexpr DisplayHandle display1{ 1u };
        constexpr DisplayHandle display2{ 2u };

        createDisplay(display1);
        m_commandBuffer.enqueueCommand(RendererCommand::ScenePublished{ scene1, {} });
        m_commandBuffer.enqueueCommand(RendererCommand::SetSceneMapping{ scene1, display2 });
        expectCommandPushed(display1, RendererCommand::ScenePublished{});
        m_displayDispatcher.dispatchCommands(m_commandBuffer);

        simulateSceneStateEventFromDisplay(display1, scene1, RendererSceneState::Available);
        dispatchAndExpectSceneControlEvents({ ERendererEventType::SceneStateChanged });

        // will expect this command to be pushed to any new display
        m_displayDispatcher.m_expectedBroadcastCommandsForNewDisplays.push_back(RendererCommand::ScenePublished{ scene1, {} });
        m_displayDispatcher.m_expectedCommandsForNextCreatedDisplay.push_back(RendererCommand::SetSceneMapping{});

        // change 'ownership' of scene right before 2nd display creation
        m_commandBuffer.enqueueCommand(RendererCommand::SetSceneMapping{ scene1, display1 });
        expectCommandPushed(display1, RendererCommand::SetSceneMapping{});
        m_displayDispatcher.dispatchCommands(m_commandBuffer);
        createDisplay(display2);

        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), dispatchSceneControlEvents(_)); // nothing from display1
        simulateSceneStateEventFromDisplay(display2, scene1, RendererSceneState::Available);
        // expect no redundant state change emitted
        dispatchAndExpectSceneControlEvents({});

        simulateSceneStateEventFromDisplay(display1, scene1, RendererSceneState::Unavailable);
        simulateSceneStateEventFromDisplay(display2, scene1, RendererSceneState::Unavailable);
        // unavailable emitted also just once
        dispatchAndExpectSceneControlEvents({ ERendererEventType::SceneStateChanged });

        // no more events
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), dispatchSceneControlEvents(_));
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display2), dispatchSceneControlEvents(_));
        dispatchAndExpectSceneControlEvents({});
    }

    TEST_F(ADisplayDispatcher, sceneAvailableEventIsEmittedExactlyOnce_withLateOwnershipSet)
    {
        constexpr SceneId scene1{ 123u };
        constexpr DisplayHandle display1{ 1u };
        constexpr DisplayHandle display2{ 2u };

        createDisplay(display1);
        m_commandBuffer.enqueueCommand(RendererCommand::ScenePublished{ scene1, {} });
        expectCommandPushed(display1, RendererCommand::ScenePublished{});
        m_displayDispatcher.dispatchCommands(m_commandBuffer);

        simulateSceneStateEventFromDisplay(display1, scene1, RendererSceneState::Available);
        dispatchAndExpectSceneControlEvents({ ERendererEventType::SceneStateChanged });

        // will expect this command to be pushed to any new display
        m_displayDispatcher.m_expectedBroadcastCommandsForNewDisplays.push_back(RendererCommand::ScenePublished{ scene1, {} });

        createDisplay(display2);

        // change 'ownership' of scene after 2nd display creation
        m_commandBuffer.enqueueCommand(RendererCommand::SetSceneMapping{ scene1, display1 });
        expectCommandPushed(display1, RendererCommand::SetSceneMapping{});
        m_displayDispatcher.dispatchCommands(m_commandBuffer);

        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), dispatchSceneControlEvents(_)); // nothing from display1
        simulateSceneStateEventFromDisplay(display2, scene1, RendererSceneState::Available);
        // expect no redundant state change emitted
        dispatchAndExpectSceneControlEvents({});

        simulateSceneStateEventFromDisplay(display1, scene1, RendererSceneState::Unavailable);
        simulateSceneStateEventFromDisplay(display2, scene1, RendererSceneState::Unavailable);
        // unavailable emitted also just once
        dispatchAndExpectSceneControlEvents({ ERendererEventType::SceneStateChanged });

        // no more events
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), dispatchSceneControlEvents(_));
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display2), dispatchSceneControlEvents(_));
        dispatchAndExpectSceneControlEvents({});
    }

    TEST_F(ADisplayDispatcher, sceneAvailableEventIsEmittedAgainAfterSceneRepublishedEvenIfOwningDisplayDestroyed)
    {
        constexpr SceneId scene1{ 123u };
        constexpr DisplayHandle display1{ 1u };
        constexpr DisplayHandle display2{ 2u };

        createDisplay(display1);
        m_commandBuffer.enqueueCommand(RendererCommand::ScenePublished{ scene1, {} });
        expectCommandPushed(display1, RendererCommand::ScenePublished{});
        m_displayDispatcher.dispatchCommands(m_commandBuffer);
        m_commandBuffer.enqueueCommand(RendererCommand::SetSceneMapping{ scene1, display1 });
        expectCommandPushed(display1, RendererCommand::SetSceneMapping{});
        m_displayDispatcher.dispatchCommands(m_commandBuffer);

        simulateSceneStateEventFromDisplay(display1, scene1, RendererSceneState::Available);
        dispatchAndExpectSceneControlEvents({ ERendererEventType::SceneStateChanged });

        // will expect this command to be pushed to any new display
        m_displayDispatcher.m_expectedBroadcastCommandsForNewDisplays.push_back(RendererCommand::ScenePublished{ scene1, {} });

        simulateSceneStateEventFromDisplay(display1, scene1, RendererSceneState::Unavailable);
        dispatchAndExpectSceneControlEvents({ ERendererEventType::SceneStateChanged });

        destroyDisplay(display1);

        createDisplay(display2);
        m_commandBuffer.enqueueCommand(RendererCommand::ScenePublished{ scene1, {} });
        expectCommandPushed(display2, RendererCommand::ScenePublished{});
        m_displayDispatcher.dispatchCommands(m_commandBuffer);

        simulateSceneStateEventFromDisplay(display2, scene1, RendererSceneState::Available);
        dispatchAndExpectSceneControlEvents({ ERendererEventType::SceneStateChanged });

        simulateSceneStateEventFromDisplay(display2, scene1, RendererSceneState::Unavailable);
        dispatchAndExpectSceneControlEvents({ ERendererEventType::SceneStateChanged });

        // no more events
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display2), dispatchSceneControlEvents(_));
        dispatchAndExpectSceneControlEvents({});
    }

    TEST_F(ADisplayDispatcher, canHandleAvailableUnavailableOnTwoDisplays)
    {
        constexpr SceneId scene1{ 123u };
        constexpr DisplayHandle display1{ 1u };
        constexpr DisplayHandle display2{ 2u };

        createDisplay(display1);
        createDisplay(display2);

        m_commandBuffer.enqueueCommand(RendererCommand::SetSceneMapping{ scene1, display2 });
        expectCommandPushed(display2, RendererCommand::SetSceneMapping{});
        m_displayDispatcher.dispatchCommands(m_commandBuffer);

        m_commandBuffer.enqueueCommand(RendererCommand::ScenePublished{ scene1, {} });
        expectCommandPushed(display1, RendererCommand::ScenePublished{});
        expectCommandPushed(display2, RendererCommand::ScenePublished{});
        m_displayDispatcher.dispatchCommands(m_commandBuffer);

        simulateSceneStateEventFromDisplay(display1, scene1, RendererSceneState::Available);
        simulateSceneStateEventFromDisplay(display2, scene1, RendererSceneState::Available);
        dispatchAndExpectSceneControlEvents({ ERendererEventType::SceneStateChanged });

        simulateSceneStateEventFromDisplay(display1, scene1, RendererSceneState::Unavailable);
        simulateSceneStateEventFromDisplay(display2, scene1, RendererSceneState::Unavailable);
        dispatchAndExpectSceneControlEvents({ ERendererEventType::SceneStateChanged });

        // no more events
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), dispatchSceneControlEvents(_));
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display2), dispatchSceneControlEvents(_));
        dispatchAndExpectSceneControlEvents({});
    }

    TEST_F(ADisplayDispatcher, emitsSceneStateEventsCorrectlyForTwoScenesOnTwoDisplays)
    {
        constexpr SceneId scene1{ 123u };
        constexpr SceneId scene2{ 124u };
        constexpr DisplayHandle display1{ 1u };
        constexpr DisplayHandle display2{ 2u };

        createDisplay(display1);
        createDisplay(display2);

        // available
        m_commandBuffer.enqueueCommand(RendererCommand::ScenePublished{ scene1, {} });
        expectCommandPushed(display1, RendererCommand::ScenePublished{});
        expectCommandPushed(display2, RendererCommand::ScenePublished{});
        m_displayDispatcher.dispatchCommands(m_commandBuffer);

        m_commandBuffer.enqueueCommand(RendererCommand::ScenePublished{ scene2, {} });
        expectCommandPushed(display1, RendererCommand::ScenePublished{});
        expectCommandPushed(display2, RendererCommand::ScenePublished{});
        m_displayDispatcher.dispatchCommands(m_commandBuffer);

        simulateSceneStateEventFromDisplay(display1, scene1, RendererSceneState::Available);
        simulateSceneStateEventFromDisplay(display2, scene1, RendererSceneState::Available);
        dispatchAndExpectSceneControlEvents({ ERendererEventType::SceneStateChanged });
        simulateSceneStateEventFromDisplay(display1, scene2, RendererSceneState::Available);
        simulateSceneStateEventFromDisplay(display2, scene2, RendererSceneState::Available);
        dispatchAndExpectSceneControlEvents({ ERendererEventType::SceneStateChanged });

        // ready
        m_commandBuffer.enqueueCommand(RendererCommand::SetSceneMapping{ scene1, display1 });
        expectCommandPushed(display1, RendererCommand::SetSceneMapping{});
        m_displayDispatcher.dispatchCommands(m_commandBuffer);

        m_commandBuffer.enqueueCommand(RendererCommand::SetSceneMapping{ scene2, display2 });
        expectCommandPushed(display2, RendererCommand::SetSceneMapping{});
        m_displayDispatcher.dispatchCommands(m_commandBuffer);

        simulateSceneStateEventFromDisplay(display1, scene1, RendererSceneState::Ready);
        simulateSceneStateEventFromDisplay(display2, scene2, RendererSceneState::Ready);
        dispatchAndExpectSceneControlEvents({ ERendererEventType::SceneStateChanged, ERendererEventType::SceneStateChanged });

        // rendered
        simulateSceneStateEventFromDisplay(display1, scene1, RendererSceneState::Rendered);
        simulateSceneStateEventFromDisplay(display2, scene2, RendererSceneState::Rendered);
        dispatchAndExpectSceneControlEvents({ ERendererEventType::SceneStateChanged, ERendererEventType::SceneStateChanged });

        // unavailable
        m_commandBuffer.enqueueCommand(RendererCommand::SceneUnpublished{ scene1 });
        expectCommandPushed(display1, RendererCommand::SceneUnpublished{});
        expectCommandPushed(display2, RendererCommand::SceneUnpublished{});
        m_displayDispatcher.dispatchCommands(m_commandBuffer);

        m_commandBuffer.enqueueCommand(RendererCommand::SceneUnpublished{ scene2 });
        expectCommandPushed(display1, RendererCommand::SceneUnpublished{});
        expectCommandPushed(display2, RendererCommand::SceneUnpublished{});
        m_displayDispatcher.dispatchCommands(m_commandBuffer);

        simulateSceneStateEventFromDisplay(display1, scene1, RendererSceneState::Unavailable);
        simulateSceneStateEventFromDisplay(display2, scene1, RendererSceneState::Unavailable);
        dispatchAndExpectSceneControlEvents({ ERendererEventType::SceneStateChanged });
        simulateSceneStateEventFromDisplay(display1, scene2, RendererSceneState::Unavailable);
        simulateSceneStateEventFromDisplay(display2, scene2, RendererSceneState::Unavailable);
        dispatchAndExpectSceneControlEvents({ ERendererEventType::SceneStateChanged });

        // no more events
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), dispatchSceneControlEvents(_));
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display2), dispatchSceneControlEvents(_));
        dispatchAndExpectSceneControlEvents({});
    }
}
