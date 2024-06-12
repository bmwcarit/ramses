//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "gmock/gmock.h"
#include "internal/RendererLib/DisplayDispatcher.h"
#include "PlatformMock.h"
#include "DisplayBundleMock.h"
#include "DisplayThreadMock.h"

namespace ramses::internal
{
    class DisplayDispatcherMock : public DisplayDispatcher
    {
    public:
        DisplayDispatcherMock(const RendererConfigData& config, IRendererSceneEventSender& rendererSceneSender, IThreadAliveNotifier& notifier);
        ~DisplayDispatcherMock() override;

        MOCK_METHOD(Display, createDisplayBundle, (DisplayHandle, const DisplayConfigData&), (override));
    };

    class DisplayDispatcherFacade : public DisplayDispatcherMock
    {
    public:
        DisplayDispatcherFacade(const RendererConfigData& config, IRendererSceneEventSender& rendererSceneSender, IThreadAliveNotifier& notifier ,bool threaded);
        ~DisplayDispatcherFacade() override;

        Display createDisplayBundle(DisplayHandle displayHandle, const DisplayConfigData& dispConfig) override;

        template <template<typename> class MOCK_TYPE = ::testing::StrictMock>
        MOCK_TYPE<DisplayBundleMock>* getDisplayBundleMock(DisplayHandle display);
        template <template<typename> class MOCK_TYPE = ::testing::StrictMock>
        MOCK_TYPE<DisplayThreadMock>* getDisplayThreadMock(DisplayHandle display);
        [[nodiscard]] DisplayHandleVector getDisplays() const;

        RendererCommands m_expectedBroadcastCommandsForNewDisplays;
        RendererCommands m_expectedCommandsForNextCreatedDisplay;
        ELoopMode m_expectedLoopModeForNewDisplays = ELoopMode::UpdateOnly;
        std::chrono::microseconds m_expectedMinFrameDurationForNextDisplayCreation{ 0 };
        bool m_expectNewDisplaysToStartUpdating = true;
        bool m_useNiceMock = false;

    private:
        template <template<typename> class MOCK_TYPE>
        Display createDisplayBundleMocks();

        bool m_threaded;
    };
}

