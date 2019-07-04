//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DisplayManager/ConfirmationEcho.h"
#include "Ramsh/RamshInput.h"

namespace ramses_display_manager
{
    ConfirmationEcho::ConfirmationEcho(IDisplayManager& displayManager)
        : m_displayManager(displayManager)
    {
        description = "echos given text when command is executed (used for automated tests)";
        registerKeyword("confirm");

        getArgument<0>().setDescription("text");
    }

    bool ConfirmationEcho::execute(ramses_internal::String& text) const
    {
        m_displayManager.processConfirmationEchoCommand(text.c_str());
        return true;
    }
}
