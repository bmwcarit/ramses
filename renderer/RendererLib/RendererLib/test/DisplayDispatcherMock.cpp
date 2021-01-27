//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DisplayDispatcherMock.h"

namespace ramses_internal
{
    DisplayDispatcherMock::DisplayDispatcherMock(const RendererConfig& config, RendererCommandBuffer& commandBuffer, IRendererSceneEventSender& rendererSceneSender)
        : DisplayDispatcher(config, commandBuffer, rendererSceneSender)
    {
    }
    DisplayDispatcherMock::~DisplayDispatcherMock() = default;


    DisplayDispatcherFacade::DisplayDispatcherFacade(const RendererConfig& config, RendererCommandBuffer& commandBuffer, IRendererSceneEventSender& rendererSceneSender)
        : DisplayDispatcherMock(config, commandBuffer, rendererSceneSender)
    {
    }
    DisplayDispatcherFacade::~DisplayDispatcherFacade() = default;

    DisplayDispatcher::Display DisplayDispatcherFacade::createDisplayBundle()
    {
        DisplayDispatcherMock::createDisplayBundle();

        auto platform = std::make_unique<::testing::StrictMock<PlatformStrictMock>>();
        auto displayBundle = std::make_unique<::testing::StrictMock<DisplayBundleMock>>();

        // these expectations cannot be in test case initiating the display creation
        // because the mock is only created here immediately followed by these calls
        InSequence seq;
        // stashed broadcast commands pushed to new display
        EXPECT_CALL(*displayBundle, pushAndConsumeCommands(_)).WillOnce(Invoke([&](auto& cmds)
        {
            ASSERT_EQ(m_expectedBroadcastCommandsForNewDisplays.size(), cmds.size());
            for (size_t i = 0; i < cmds.size(); ++i)
                EXPECT_EQ(m_expectedBroadcastCommandsForNewDisplays[i].index(), cmds[i].index());
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
            EXPECT_TRUE(absl::holds_alternative<RendererCommand::CreateDisplay>(cmds.front()));
            cmds.clear();
        }));
        EXPECT_CALL(*displayBundle, doOneLoop(_, _));

        return { std::move(platform), std::move(displayBundle), {} };
    }

    ::testing::StrictMock<DisplayBundleMock>* DisplayDispatcherFacade::getDisplayBundleMock(DisplayHandle display)
    {
        if (m_displays.count(display) == 0)
            return nullptr;

        return static_cast<::testing::StrictMock<DisplayBundleMock>*>(m_displays[display].m_displayBundle.get());
    }

    DisplayHandleVector DisplayDispatcherFacade::getDisplays() const
    {
        DisplayHandleVector displays;
        displays.reserve(m_displays.size());
        for (const auto& d : m_displays)
            displays.push_back(d.first);

        return displays;
    }
}
