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
    class DisplayManagerMock : public ramses_display_manager::IDisplayManager
    {
    public:
        DisplayManagerMock();

        MOCK_METHOD3(setSceneState, bool(sceneId_t, ramses_display_manager::SceneState, const char*));
        MOCK_METHOD3(setSceneMapping, bool(sceneId_t, displayId_t, int32_t));
        MOCK_METHOD6(setSceneOffscreenBufferMapping, bool(ramses::sceneId_t, ramses::displayId_t, uint32_t, uint32_t, ramses::sceneId_t, ramses::dataConsumerId_t));
        MOCK_METHOD4(linkData, void(sceneId_t, dataProviderId_t, sceneId_t, dataConsumerId_t));
        MOCK_METHOD1(processConfirmationEchoCommand, void(const char*));
        MOCK_METHOD2(dispatchAndFlush, void(ramses_display_manager::IEventHandler*, IRendererEventHandler*));
    };
}

#endif
