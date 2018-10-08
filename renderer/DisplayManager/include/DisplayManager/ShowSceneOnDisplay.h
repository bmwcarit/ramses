//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DISPLAYMANAGER_SHOWSCENEONDISPLAY_H
#define RAMSES_DISPLAYMANAGER_SHOWSCENEONDISPLAY_H

#include "Ramsh/RamshCommand.h"
#include "DisplayManager/IDisplayManager.h"

namespace ramses_display_manager
{
    class ShowSceneOnDisplay : public ramses_internal::RamshCommand
    {
    public:
        ShowSceneOnDisplay(IDisplayManager& displayManager);

        virtual bool executeInput(const ramses_internal::RamshInput& input) override;

    private:
        IDisplayManager& m_displayManager;
    };

}

#endif
