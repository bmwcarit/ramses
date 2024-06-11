//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DisplayDispatcherMock.h"
#include "PlatformFactoryMock.h"

namespace ramses::internal
{
    DisplayDispatcherMock::DisplayDispatcherMock(const RendererConfigData& config, IRendererSceneEventSender& rendererSceneSender, IThreadAliveNotifier& notifier)
        : DisplayDispatcher(std::make_unique<PlatformFactoryNiceMock>(), config, rendererSceneSender, notifier, EFeatureLevel_Latest)
    {
    }
    DisplayDispatcherMock::~DisplayDispatcherMock() = default;

    DisplayDispatcherFacade::DisplayDispatcherFacade(const RendererConfigData& config, IRendererSceneEventSender& rendererSceneSender, IThreadAliveNotifier& notifier, bool threaded)
        : DisplayDispatcherMock(config, rendererSceneSender, notifier)
        , m_threaded{ threaded }
    {
    }
    DisplayDispatcherFacade::~DisplayDispatcherFacade() = default;

    template <>
    DisplayDispatcher::Display DisplayDispatcherFacade::createDisplayBundleMocks<StrictMock>()
    {
        auto platform = std::make_unique<::testing::StrictMock<PlatformStrictMock>>();
        auto displayBundle = std::make_unique<StrictMock<DisplayBundleMock>>();

        // these expectations cannot be in test case initiating the display creation
        // because the mock is only created here immediately followed by these calls
        InSequence seq;

        auto displayThread = std::make_unique<StrictMock<DisplayThreadMock>>();
        if (m_threaded)
        {
            EXPECT_CALL(*displayThread, setLoopMode(m_expectedLoopModeForNewDisplays));
            EXPECT_CALL(*displayThread, setMinFrameDuration(m_expectedMinFrameDurationForNextDisplayCreation));
            if (m_expectNewDisplaysToStartUpdating)
                EXPECT_CALL(*displayThread, startUpdating());
        }

        // stashed broadcast commands pushed to new display
        EXPECT_CALL(*displayBundle, pushAndConsumeCommands(_)).WillOnce(Invoke([&](auto& cmds)
        {
            ASSERT_EQ(m_expectedBroadcastCommandsForNewDisplays.size(), cmds.size());
            for (size_t i = 0; i < cmds.size(); ++i)
            {
                const auto& expCmd = m_expectedBroadcastCommandsForNewDisplays[i];
                const auto& cmd = cmds[i];
                EXPECT_EQ(expCmd.index(), cmd.index());
                if (std::holds_alternative<RendererCommand::ScenePublished>(expCmd))
                {
                    EXPECT_EQ(std::get<RendererCommand::ScenePublished>(expCmd).scene, std::get<RendererCommand::ScenePublished>(cmd).scene);
                }
            }
            cmds.clear();
        }));
        // stashed commands specific for this display pushed to it
        EXPECT_CALL(*displayBundle, pushAndConsumeCommands(_)).WillOnce(Invoke([&](auto& cmds)
        {
            ASSERT_EQ(m_expectedCommandsForNextCreatedDisplay.size(), cmds.size());
            for (size_t i = 0; i < cmds.size(); ++i)
                EXPECT_EQ(m_expectedCommandsForNextCreatedDisplay[i].index(), cmds[i].index());
            cmds.clear();
        }));
        // create cmd pushed to new display (to be handled internally)
        EXPECT_CALL(*displayBundle, pushAndConsumeCommands(_)).WillOnce(Invoke([](auto& cmds)
        {
            ASSERT_EQ(1u, cmds.size());
            EXPECT_TRUE(std::holds_alternative<RendererCommand::CreateDisplay>(cmds.front()));
            cmds.clear();
        }));

        Display bundle;
        bundle.platform = std::move(platform);
        bundle.displayBundle = DisplayBundleShared{ std::move(displayBundle) };
        if (m_threaded)
            bundle.displayThread = std::move(displayThread);

        return bundle;
    }

    template <>
    DisplayDispatcher::Display DisplayDispatcherFacade::createDisplayBundleMocks<NiceMock>()
    {
        auto platform = std::make_unique<::testing::StrictMock<PlatformStrictMock>>();
        auto displayBundle = std::make_unique<NiceMock<DisplayBundleMock>>();
        auto displayThread = std::make_unique<NiceMock<DisplayThreadMock>>();

        Display bundle;
        bundle.platform = std::move(platform);
        bundle.displayBundle = DisplayBundleShared{ std::move(displayBundle) };
        if (m_threaded)
            bundle.displayThread = std::move(displayThread);

        return bundle;
    }

    DisplayDispatcher::Display DisplayDispatcherFacade::createDisplayBundle(DisplayHandle displayHandle, const DisplayConfigData& dispConfig)
    {
        DisplayDispatcherMock::createDisplayBundle(displayHandle, dispConfig);
        return m_useNiceMock ? createDisplayBundleMocks<::testing::NiceMock>() : createDisplayBundleMocks<::testing::StrictMock>();
    }

    template <template<typename> class MOCK_TYPE>
    MOCK_TYPE<DisplayBundleMock>* DisplayDispatcherFacade::getDisplayBundleMock(DisplayHandle display)
    {
        if (m_displays.count(display) == 0)
            return nullptr;

        return &static_cast<MOCK_TYPE<DisplayBundleMock>&>(*m_displays[display].displayBundle);
    }

    template <template<typename> class MOCK_TYPE>
    MOCK_TYPE<DisplayThreadMock>* DisplayDispatcherFacade::getDisplayThreadMock(DisplayHandle display)
    {
        if (m_displays.count(display) == 0)
            return nullptr;

        return &static_cast<MOCK_TYPE<DisplayThreadMock>&>(*m_displays[display].displayThread);
    }

    DisplayHandleVector DisplayDispatcherFacade::getDisplays() const
    {
        DisplayHandleVector displays;
        displays.reserve(m_displays.size());
        for (const auto& d : m_displays)
            displays.push_back(d.first);

        return displays;
    }

    template ::testing::StrictMock<DisplayBundleMock>* DisplayDispatcherFacade::getDisplayBundleMock<::testing::StrictMock>(DisplayHandle);
    template ::testing::NiceMock<DisplayBundleMock>* DisplayDispatcherFacade::getDisplayBundleMock<::testing::NiceMock>(DisplayHandle);
    template ::testing::StrictMock<DisplayThreadMock>* DisplayDispatcherFacade::getDisplayThreadMock<::testing::StrictMock>(DisplayHandle);
    template ::testing::NiceMock<DisplayThreadMock>* DisplayDispatcherFacade::getDisplayThreadMock<::testing::NiceMock>(DisplayHandle);
}
