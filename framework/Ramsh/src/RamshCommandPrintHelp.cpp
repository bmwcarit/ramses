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

    Bool RamshCommandPrintHelp::executeInput(const RamshInput& input)
    {
        UNUSED(input);
        HashSet<RamshCommand*> alreadyListed;
        KeywordToCommandMap::ConstIterator iter = m_ramsh.commands().begin();
        KeywordToCommandMap::ConstIterator end = m_ramsh.commands().end();
        for (; iter != end; ++iter)
        {
            RamshCommand* cmd = iter->value;
            if (!alreadyListed.hasElement(cmd))
            {
                LOG_INFO(CONTEXT_RAMSH,(String(iter->value->keywordString())));
                LOG_INFO(CONTEXT_RAMSH,(String("\t\t").append(iter->value->descriptionString())));

                alreadyListed.put(cmd);
            }
        }
        LOG_INFO(CONTEXT_RAMSH, (String("#")));
        LOG_INFO(CONTEXT_RAMSH, (String("\t\t").append("Browse through the last 10 used ramsh commands")));
        return true;
    }

}
