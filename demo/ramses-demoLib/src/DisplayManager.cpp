//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-demoLib/DisplayManager.h"

DisplayManager::DisplayManager(ramses::RamsesRenderer&  renderer,
                               ramses::RamsesFramework& framework,
                               IInputReceiver*          inputReceiver)
    : ramses_display_manager::DisplayManager(renderer, framework, false)
    , m_inputReceiver(inputReceiver)
{
}

void DisplayManager::mouseEvent(ramses::displayId_t displayId,
                                ramses::EMouseEvent eventType,
                                int32_t             mousePosX,
                                int32_t             mousePosY)
{
    UNUSED(displayId)
    if (m_inputReceiver)
    {
        m_inputReceiver->mouseEvent(eventType, mousePosX, mousePosY);
    }
}

void DisplayManager::touchEvent(
    ramses::displayId_t displayId, ramses::ETouchEvent eventType, int32_t id, int32_t touchPosX, int32_t touchPosY)
{
    UNUSED(displayId)
    if (m_inputReceiver)
    {
        m_inputReceiver->touchEvent(eventType, id, touchPosX, touchPosY);
    }
}
