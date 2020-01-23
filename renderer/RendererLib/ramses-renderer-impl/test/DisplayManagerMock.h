//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DISPLAYMANAGERMOCK_H
#define RAMSES_DISPLAYMANAGERMOCK_H

#include "gmock/gmock.h"
#include "DisplayManager/IDisplayManager.h"

namespace ramses
{
    class DisplayManagerMock : public ramses_internal::IDisplayManager
    {
    public:
        DisplayManagerMock();

        MOCK_METHOD3(setSceneState, bool(sceneId_t, ramses_internal::SceneState, const char*));
        MOCK_METHOD2(setSceneMapping, bool(sceneId_t, displayId_t));
        MOCK_METHOD3(setSceneDisplayBufferAssignment, bool(sceneId_t, displayBufferId_t, int32_t));
        MOCK_METHOD5(setDisplayBufferClearColor, bool(ramses::displayBufferId_t, float, float, float, float));
        MOCK_METHOD3(linkOffscreenBuffer, void(displayBufferId_t, sceneId_t, dataConsumerId_t));
        MOCK_METHOD4(linkData, void(sceneId_t, dataProviderId_t, sceneId_t, dataConsumerId_t));
        MOCK_METHOD1(processConfirmationEchoCommand, void(const char*));
        MOCK_METHOD2(dispatchAndFlush, void(ramses_internal::IEventHandler*, IRendererEventHandler*));
    };
}

#endif
