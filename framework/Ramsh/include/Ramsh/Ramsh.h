//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSH_H
#define RAMSES_RAMSH_H

#include "Ramsh/RamshCommandPrintHelp.h"
#include "Collections/Vector.h"
#include "Collections/HashMap.h"
#include "Collections/String.h"
#include "Ramsh/RamshCommandPrintBuildConfig.h"
#include "Ramsh/RamshCommandPrintRamsesVersion.h"
#include "Ramsh/RamshCommandSetLogLevel.h"
#include "Ramsh/RamshCommandSetContextLogLevel.h"
#include "Ramsh/RamshCommandSetContextLogLevelFilter.h"
#include "Ramsh/RamshCommandPrintLogLevels.h"

namespace ramses_internal
{
    class RamshCommand;
    class RamshInput;
    class RamshCommunicationChannel;

    typedef HashMap<String, RamshCommand*> KeywordToCommandMap;

    class Ramsh
    {
    public:
        Ramsh(String prompt = "noname");

        virtual ~Ramsh();

        void add(RamshCommand& command);

        virtual bool start();
        virtual bool stop();

        virtual Bool execute(RamshInput& input);

        const String& getPrompt() const;

        const KeywordToCommandMap& commands() const;

    protected:
        String m_prompt;
        KeywordToCommandMap m_commands;
        RamshCommandPrintHelp* m_pCmdPrintHelp;
        RamshCommandPrintBuildConfig m_cmdPrintBuildConfig;
        RamshCommandPrintRamsesVersion m_cmdPrintRamsesVersion;
        RamshCommandSetLogLevel* m_pCmdSetLogLevel;
        RamshCommandSetContextLogLevel* m_pCmdSetContextLogLevel;
        RamshCommandSetContextLogLevelFilter* m_pCmdSetContextLogLevelFilter;
        RamshCommandPrintLogLevels* m_pCmdPrintLogLevels;

    private:
        Ramsh(const Ramsh& ramsh);
    };

}// namespace ramses_internal

#endif
