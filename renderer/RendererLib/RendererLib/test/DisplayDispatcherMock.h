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

namespace ramses_internal
{
    class DisplayDispatcherMock : public DisplayDispatcher
    {
    public:
        DisplayDispatcherMock(const RendererConfig& config, RendererCommandBuffer& commandBuffer, IRendererSceneEventSender& rendererSceneSender);
        virtual ~DisplayDispatcherMock();

        MOCK_METHOD(Display, createDisplayBundle, (), (override));
    };

    class DisplayDispatcherFacade : public DisplayDispatcherMock
    {
    public:
        DisplayDispatcherFacade(const RendererConfig& config, RendererCommandBuffer& commandBuffer, IRendererSceneEventSender& rendererSceneSender);
        virtual ~DisplayDispatcherFacade();

        virtual Display createDisplayBundle() override;

        ::testing::StrictMock<DisplayBundleMock>* getDisplayBundleMock(DisplayHandle display);
        DisplayHandleVector getDisplays() const;

        RendererCommands m_expectedBroadcastCommandsForNewDisplays;
        RendererCommands m_expectedCommandsForNextCreatedDisplay;
    };
}
#endif
