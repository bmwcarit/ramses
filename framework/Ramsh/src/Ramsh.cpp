//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Ramsh/Ramsh.h"
#include "ramses-sdk-build-config.h"
#include "Utils/LogMacros.h"
#include "Ramsh/RamshCommandSetContextLogLevelFilter.h"

namespace ramses_internal
{
    Ramsh::Ramsh(String prompt)
    : m_prompt(prompt.append(">"))
    , m_cmdPrintBuildConfig(::ramses_sdk::RAMSES_SDK_BUILD_CONFIG)
    , m_cmdPrintRamsesVersion(::ramses_sdk::RAMSES_SDK_PROJECT_VERSION_STRING)
    {
        add(m_cmdPrintBuildConfig);
        add(m_cmdPrintRamsesVersion);

        m_pCmdPrintHelp = new RamshCommandPrintHelp(*this);
        add(*m_pCmdPrintHelp);

        m_pCmdSetLogLevel = new RamshCommandSetConsoleLogLevel(*this);
        add(*m_pCmdSetLogLevel);

        m_pCmdSetContextLogLevel = new RamshCommandSetContextLogLevel(*this);
        add(*m_pCmdSetContextLogLevel);

        m_pCmdSetContextLogLevelFilter = new RamshCommandSetContextLogLevelFilter(*this);
        add(*m_pCmdSetContextLogLevelFilter);

        m_pCmdPrintLogLevels = new RamshCommandPrintLogLevels(*this);
        add(*m_pCmdPrintLogLevels);
    }

    Ramsh::~Ramsh()
    {
        delete m_pCmdPrintHelp;
        delete m_pCmdSetLogLevel;
        delete m_pCmdSetContextLogLevel;
        delete m_pCmdSetContextLogLevelFilter;
        delete m_pCmdPrintLogLevels;
    }

    void Ramsh::add(RamshCommand& command)
    {
        for (const auto& kw : command.keywords())
        {
            m_commands.put(kw, &command);
        }
    }

    bool Ramsh::start()
    {
        return true;
    }

    bool Ramsh::stop()
    {
        return true;
    }

    const String& Ramsh::getPrompt() const
    {
        return m_prompt;
    }

    bool Ramsh::execute(RamshInput& input)
    {
        if (!input.isValid())
        {
            return false;
        }

        String commandString(input.toString());

        const String& keyword = input[0];
        auto iter = m_commands.find(keyword);

        if (iter == m_commands.end())
        {
            LOG_ERROR(CONTEXT_RAMSH, String("unknown command: '").append(keyword).append("'"));
            return false;
        }

        RamshCommand* cmd = iter->value;
        if (!cmd)
        {
            LOG_ERROR(CONTEXT_RAMSH, String("command implementation missing: '").append(keyword).append("'"));
            return false;
        }

        LOG_DEBUG(CONTEXT_RAMSH, String("triggering cmd '").append(commandString).append("'"));
        const bool cmdResult = cmd->executeInput(input);
        if (!cmdResult)
        {
            LOG_WARN(CONTEXT_RAMSH, "cmd '" << commandString << "' returned false");
        }

        return cmdResult;
    }

    const KeywordToCommandMap& Ramsh::commands() const
    {
        return m_commands;
    }
}
