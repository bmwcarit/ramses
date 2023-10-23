//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <mutex>
#include <vector>

namespace ramses::internal
{
    class RamshCommand;
    class RamshCommandPrintHelp;
    class RamshCommandPrintBuildConfig;
    class RamshCommandPrintRamsesVersion;
    class RamshCommandSetConsoleLogLevel;
    class RamshCommandSetContextLogLevel;
    class RamshCommandSetContextLogLevelFilter;
    class RamshCommandPrintLogLevels;

    class Ramsh
    {
    public:
        Ramsh();
        virtual ~Ramsh();

        Ramsh(const Ramsh& ramsh) = delete;
        Ramsh& operator=(const Ramsh& ramsh) = delete;

        bool add(const std::shared_ptr<RamshCommand>& command, bool allowOverride = true);

        virtual bool execute(const std::vector<std::string>& input);

        std::string getFullHelp() const;

    private:
        mutable std::mutex m_lock;
        std::unordered_map<std::string, std::weak_ptr<RamshCommand>> m_commands;
        std::shared_ptr<RamshCommandPrintHelp> m_pCmdPrintHelp;
        std::shared_ptr<RamshCommandPrintBuildConfig>m_cmdPrintBuildConfig;
        std::shared_ptr<RamshCommandPrintRamsesVersion>m_cmdPrintRamsesVersion;
        std::shared_ptr<RamshCommandSetConsoleLogLevel> m_pCmdSetLogLevel;
        std::shared_ptr<RamshCommandSetContextLogLevel> m_pCmdSetContextLogLevel;
        std::shared_ptr<RamshCommandSetContextLogLevelFilter> m_pCmdSetContextLogLevelFilter;
        std::shared_ptr<RamshCommandPrintLogLevels> m_pCmdPrintLogLevels;
    };
}
