//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DEMOLIB_DISPLAYMANAGER_H
#define RAMSES_DEMOLIB_DISPLAYMANAGER_H

#include "DisplayManager/DisplayManager.h"
#include "ramses-demoLib/IInputReceiver.h"

class DisplayManager : public ramses_display_manager::DisplayManager
{
public:
    DisplayManager(ramses::RamsesRenderer&  renderer,
                   ramses::RamsesFramework& framework,
                   IInputReceiver*          inputReceiver = nullptr);

private:
    virtual void mouseEvent(ramses::displayId_t displayId,
                            ramses::EMouseEvent eventType,
                            int32_t             mousePosX,
                            int32_t             mousePosY) override;
    virtual void touchEvent(ramses::displayId_t displayId,
                            ramses::ETouchEvent eventType,
                            int32_t             id,
                            int32_t             touchPosX,
                            int32_t             touchPosY) override;

    IInputReceiver* m_inputReceiver = nullptr;
};

#endif
