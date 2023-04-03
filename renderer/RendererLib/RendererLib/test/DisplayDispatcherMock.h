//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DISPLAYDISPATCHERMOCK_H
#define RAMSES_DISPLAYDISPATCHERMOCK_H

#include "gmock/gmock.h"
#include "RendererLib/DisplayDispatcher.h"
#include "PlatformMock.h"
#include "DisplayBundleMock.h"
#include "DisplayThreadMock.h"

namespace ramses_internal
{
    class DisplayDispatcherMock : public DisplayDispatcher
    {
    public:
        DisplayDispatcherMock(const RendererConfig& config, IRendererSceneEventSender& rendererSceneSender, IThreadAliveNotifier& notifier);
        ~DisplayDispatcherMock() override;

        MOCK_METHOD(Display, createDisplayBundle, (DisplayHandle), (override));
    };

    class DisplayDispatcherFacade : public DisplayDispatcherMock
    {
    public:
        DisplayDispatcherFacade(const RendererConfig& config, IRendererSceneEventSender& rendererSceneSender, IThreadAliveNotifier& notifier ,bool threaded);
        ~DisplayDispatcherFacade() override;

        Display createDisplayBundle(DisplayHandle displayHandle) override;

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
#endif
