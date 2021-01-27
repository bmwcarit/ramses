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

using namespace testing;

namespace ramses_internal
{
    class ADisplayDispatcher : public ::testing::Test
    {
    public:
        ADisplayDispatcher()
            : m_displayDispatcher(RendererConfig{}, m_commandBuffer, m_sceneEventSender)
        {
        }

        void update()
        {
            for (const auto d : m_displayDispatcher.getDisplays())
                EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(d), doOneLoop(ELoopMode::UpdateOnly, _));
            m_displayDispatcher.doOneLoop(ELoopMode::UpdateOnly);
        }

        void createDisplay(DisplayHandle displayHandle)
        {
            m_commandBuffer.enqueueCommand(RendererCommand::CreateDisplay{ displayHandle, DisplayConfig{}, nullptr });

            EXPECT_CALL(m_displayDispatcher, createDisplayBundle());
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

        void expectNoCommandPushed(DisplayHandle displayHandle)
        {
            EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(displayHandle), pushAndConsumeCommands(_)).Times(0);
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

        m_commandBuffer.enqueueCommand(RendererCommand::CreateOffscreenBuffer{ display1, {}, {}, {}, {}, {} });
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
            evts.push_back(RendererEvent{ ERendererEventType::SceneStateChanged });
        }));
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display2), dispatchSceneControlEvents(_)).WillOnce(Invoke([](auto& evts)
        {
            evts.push_back(RendererEvent{ ERendererEventType::SceneDataLinked });
        }));
        dispatchAndExpectSceneControlEvents({ ERendererEventType::SceneStateChanged, ERendererEventType::SceneDataLinked });

        // no more events
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), dispatchSceneControlEvents(_));
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display2), dispatchSceneControlEvents(_));
        dispatchAndExpectSceneControlEvents({});
    }

    TEST_F(ADisplayDispatcher, rendererEventsAsResultOfBroadcastCommandAreEmittedOnlyOnceFromFirstDisplay)
    {
        constexpr DisplayHandle display1{ 1u };
        constexpr DisplayHandle display2{ 2u };
        createDisplay(display1);
        createDisplay(display2);

        // dispatch after both displays emit
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), dispatchRendererEvents(_)).WillOnce(Invoke([](auto& evts)
        {
            evts.push_back(RendererEvent{ ERendererEventType::ScenePublished });
            evts.push_back(RendererEvent{ ERendererEventType::SceneUnpublished });
        }));
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display2), dispatchRendererEvents(_)).WillOnce(Invoke([](auto& evts)
        {
            evts.push_back(RendererEvent{ ERendererEventType::ScenePublished });
            evts.push_back(RendererEvent{ ERendererEventType::SceneUnpublished });
        }));
        dispatchAndExpectRendererEvents({ ERendererEventType::ScenePublished, ERendererEventType::SceneUnpublished });

        // dispatch after 1st display emits
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), dispatchRendererEvents(_)).WillOnce(Invoke([](auto& evts)
        {
            evts.push_back(RendererEvent{ ERendererEventType::ScenePublished });
            evts.push_back(RendererEvent{ ERendererEventType::SceneUnpublished });
        }));
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display2), dispatchRendererEvents(_));
        dispatchAndExpectRendererEvents({ ERendererEventType::ScenePublished, ERendererEventType::SceneUnpublished });
        // 2nd display emits
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), dispatchRendererEvents(_));
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display2), dispatchRendererEvents(_)).WillOnce(Invoke([](auto& evts)
        {
            evts.push_back(RendererEvent{ ERendererEventType::ScenePublished });
            evts.push_back(RendererEvent{ ERendererEventType::SceneUnpublished });
        }));
        dispatchAndExpectRendererEvents({});

        // dispatch after 2nd display emits before 1st display
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), dispatchRendererEvents(_));
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display2), dispatchRendererEvents(_)).WillOnce(Invoke([](auto& evts)
        {
            evts.push_back(RendererEvent{ ERendererEventType::ScenePublished });
            evts.push_back(RendererEvent{ ERendererEventType::SceneUnpublished });
        }));
        dispatchAndExpectRendererEvents({});
        // 1st display emits
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), dispatchRendererEvents(_)).WillOnce(Invoke([](auto& evts)
        {
            evts.push_back(RendererEvent{ ERendererEventType::ScenePublished });
            evts.push_back(RendererEvent{ ERendererEventType::SceneUnpublished });
        }));
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display2), dispatchRendererEvents(_));
        dispatchAndExpectRendererEvents({ ERendererEventType::ScenePublished, ERendererEventType::SceneUnpublished });

        // no more events
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), dispatchRendererEvents(_));
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display2), dispatchRendererEvents(_));
        dispatchAndExpectRendererEvents({});
    }

    TEST_F(ADisplayDispatcher, sceneControlEventsAsResultOfBroadcastCommandAreEmittedOnlyOnceFromFirstDisplay)
    {
        constexpr DisplayHandle display1{ 1u };
        constexpr DisplayHandle display2{ 2u };
        createDisplay(display1);
        createDisplay(display2);

        // dispatch after both displays emit
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), dispatchSceneControlEvents(_)).WillOnce(Invoke([](auto& evts)
        {
            evts.push_back(RendererEvent{ ERendererEventType::ScenePublished });
            evts.push_back(RendererEvent{ ERendererEventType::SceneUnpublished });
        }));
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display2), dispatchSceneControlEvents(_)).WillOnce(Invoke([](auto& evts)
        {
            evts.push_back(RendererEvent{ ERendererEventType::ScenePublished });
            evts.push_back(RendererEvent{ ERendererEventType::SceneUnpublished });
        }));
        dispatchAndExpectSceneControlEvents({ ERendererEventType::ScenePublished, ERendererEventType::SceneUnpublished });

        // dispatch after 1st display emits
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), dispatchSceneControlEvents(_)).WillOnce(Invoke([](auto& evts)
        {
            evts.push_back(RendererEvent{ ERendererEventType::ScenePublished });
            evts.push_back(RendererEvent{ ERendererEventType::SceneUnpublished });
        }));
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display2), dispatchSceneControlEvents(_));
        dispatchAndExpectSceneControlEvents({ ERendererEventType::ScenePublished, ERendererEventType::SceneUnpublished });
        // 2nd display emits
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), dispatchSceneControlEvents(_));
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display2), dispatchSceneControlEvents(_)).WillOnce(Invoke([](auto& evts)
        {
            evts.push_back(RendererEvent{ ERendererEventType::ScenePublished });
            evts.push_back(RendererEvent{ ERendererEventType::SceneUnpublished });
        }));
        dispatchAndExpectSceneControlEvents({});

        // dispatch after 2nd display emits before 1st display
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), dispatchSceneControlEvents(_));
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display2), dispatchSceneControlEvents(_)).WillOnce(Invoke([](auto& evts)
        {
            evts.push_back(RendererEvent{ ERendererEventType::ScenePublished });
            evts.push_back(RendererEvent{ ERendererEventType::SceneUnpublished });
        }));
        dispatchAndExpectSceneControlEvents({});
        // 1st display emits
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display1), dispatchSceneControlEvents(_)).WillOnce(Invoke([](auto& evts)
        {
            evts.push_back(RendererEvent{ ERendererEventType::ScenePublished });
            evts.push_back(RendererEvent{ ERendererEventType::SceneUnpublished });
        }));
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display2), dispatchSceneControlEvents(_));
        dispatchAndExpectSceneControlEvents({ ERendererEventType::ScenePublished, ERendererEventType::SceneUnpublished });

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
            evts.push_back(RendererEvent{ ERendererEventType::SceneStateChanged });
        }));
        EXPECT_CALL(*m_displayDispatcher.getDisplayBundleMock(display2), dispatchSceneControlEvents(_)).WillOnce(Invoke([](auto& evts)
        {
            evts.push_back(RendererEvent{ ERendererEventType::SceneDataLinked });
        }));

        m_displayDispatcher.injectSceneControlEvent(RendererEvent{ ERendererEventType::StreamSurfaceAvailable });

        dispatchAndExpectSceneControlEvents({ ERendererEventType::SceneStateChanged, ERendererEventType::SceneDataLinked, ERendererEventType::StreamSurfaceAvailable });

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
        m_commandBuffer.enqueueCommand(RendererCommand::CreateOffscreenBuffer{ display2, {}, {}, {}, {}, {} });
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
}
