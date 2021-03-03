//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Ramsh/Ramsh.h"
#include "Collections/StringOutputStream.h"
#include "Collections/HashSet.h"
#include "Utils/LogMacros.h"
#include "Ramsh/RamshCommandSetContextLogLevelFilter.h"
#include "Ramsh/RamshCommandPrintHelp.h"
#include "Ramsh/RamshCommandPrintBuildConfig.h"
#include "Ramsh/RamshCommandPrintRamsesVersion.h"
#include "Ramsh/RamshCommandSetConsoleLogLevel.h"
#include "Ramsh/RamshCommandSetContextLogLevel.h"
#include "Ramsh/RamshCommandSetContextLogLevelFilter.h"
#include "Ramsh/RamshCommandPrintLogLevels.h"
#include <mutex>

namespace ramses_internal
{
    Ramsh::Ramsh()
    {
        m_cmdPrintBuildConfig = std::make_shared<RamshCommandPrintBuildConfig>();
        add(m_cmdPrintBuildConfig);

        m_cmdPrintRamsesVersion = std::make_shared<RamshCommandPrintRamsesVersion>();
        add(m_cmdPrintRamsesVersion);

        m_pCmdPrintHelp = std::make_shared<RamshCommandPrintHelp>(*this);
        add(m_pCmdPrintHelp);

        m_pCmdSetLogLevel = std::make_shared<RamshCommandSetConsoleLogLevel>(*this);
        add(m_pCmdSetLogLevel);

        m_pCmdSetContextLogLevel = std::make_shared<RamshCommandSetContextLogLevel>(*this);
        add(m_pCmdSetContextLogLevel);

        m_pCmdSetContextLogLevelFilter = std::make_shared<RamshCommandSetContextLogLevelFilter>(*this);
        add(m_pCmdSetContextLogLevelFilter);

        m_pCmdPrintLogLevels = std::make_shared<RamshCommandPrintLogLevels>(*this);
        add(m_pCmdPrintLogLevels);
    }

    Ramsh::~Ramsh() = default;

    bool Ramsh::add(const std::shared_ptr<RamshCommand>& command, bool allowOverride)
    {
        std::lock_guard<std::mutex> l(m_lock);
        assert(command);

        // check keywords
        if (command->keywords().empty())
        {
            LOG_WARN_P(CONTEXT_RAMSH, "Ramsh::add: Command has no keywords");
            return false;
        }
        for (const auto& kw : command->keywords())
        {
            if (kw.empty())
            {
                LOG_WARN(CONTEXT_RAMSH, "Ramsh::add: Empty keyword not allowed");
                return false;
            }
            auto isValidChar = [&](char c) {
                return isprint(c) == 0 || isspace(c) != 0 || c == '"';
            };
            if (std::find_if(kw.begin(), kw.end(), isValidChar) != kw.end())
            {
                LOG_WARN_P(CONTEXT_RAMSH, "Ramsh::add: Command keyword '{}' is invalid", kw);
                return false;
            }
            if (!allowOverride)
            {
                if (m_commands.find(kw) != m_commands.end())
                {
                    LOG_WARN_P(CONTEXT_RAMSH, "Ramsh::add: Command with keyword '{}' already exists", kw);
                    return false;
                }
            }
        }

        for (const auto& kw : command->keywords())
            m_commands[kw] = command;

        return true;
    }

    bool Ramsh::execute(const std::vector<std::string>& input)
    {
        if (input.empty())
        {
            return false;
        }

        const std::string& keyword = input[0];
        std::shared_ptr<RamshCommand> cmd;
        {
            std::lock_guard<std::mutex> l(m_lock);

            auto iter = m_commands.find(keyword);
            if (iter == m_commands.end())
            {
                LOG_ERROR_P(CONTEXT_RAMSH, "unknown command: '{}'", keyword);
                return false;
            }

            cmd = iter->second.lock();
        }
        if (!cmd)
        {
            LOG_ERROR_P(CONTEXT_RAMSH, "command implementation missing: '{}'", keyword);
            return false;
        }

        LOG_DEBUG_P(CONTEXT_RAMSH, "triggering cmd '{}'", fmt::join(input, " "));
        const bool cmdResult = cmd->executeInput(input);
        if (!cmdResult)
        {
            LOG_WARN_P(CONTEXT_RAMSH, "cmd '{}' returned false", fmt::join(input, " "));
        }

        return cmdResult;
    }

    std::string Ramsh::getFullHelp() const
    {
        std::lock_guard<std::mutex> l(m_lock);

        StringOutputStream sos;
        HashSet<RamshCommand*> alreadyListed;
        for (const auto& p : m_commands)
        {
            auto cmd = p.second.lock();
            if (!cmd)
                continue;
            if (!alreadyListed.contains(cmd.get()))
            {
                alreadyListed.put(cmd.get());
                sos << cmd->keywordString() << "\n"
                    << "\t\t" << cmd->descriptionString() << "\n";
            }
        }
        sos << "#\n"
            << "\t\tBrowse through the last 10 used ramsh commands\n";
        return sos.release();
    }
}
