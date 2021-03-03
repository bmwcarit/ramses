//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Ramsh/RamshCommandPrintHelp.h"
#include "Ramsh/Ramsh.h"
#include "Collections/HashSet.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    RamshCommandPrintHelp::RamshCommandPrintHelp(const Ramsh& ramsh) : m_ramsh(ramsh)
    {
        registerKeyword("help");
        registerKeyword("?");
        description = "help command. lists all available commands.";
    }

    bool RamshCommandPrintHelp::executeInput(const std::vector<std::string>& /*input*/)
    {
        LOG_INFO(CONTEXT_RAMSH, m_ramsh.getFullHelp());
        return true;
    }

}
