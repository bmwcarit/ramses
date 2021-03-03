//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_API_IRAMSHCOMMAND_H
#define RAMSES_API_IRAMSHCOMMAND_H

#include "ramses-framework-api/APIExport.h"
#include <string>
#include <vector>

namespace ramses
{
    /**
    * @brief Class representing a ramses ramsh command.
    *
    * These commands can be registered with #ramses::RamsesFramework::addRamshCommand and will
    * receive callbacks triggered via console input or DLT injection.
    */
    class RAMSES_API IRamshCommand
    {
    public:
        /**
        * @brief Destructor of IRamshCommand
        */
        virtual ~IRamshCommand() = default;

        /**
         * @brief Keyword of this ramsh command. The keyword can be an alphanumeric string. Ramsh inputs
         * having this keyword as first token will call this commands execute method.
         * @returns the keyword string
         */
        virtual const std::string& keyword() const = 0;

        /**
         * @brief Helptext for this ramsh command. It will be displayed when the user invokes the 'help'
         * command. It should be a short description of the command and its expected arguments.
         * @returns the help text
         */
        virtual const std::string& help() const = 0;

        /**
         * @brief The command handler. It will be invoked with all ramsh command tokens.
         *
         * Ramsh commands are separated by spaces and each resulting token will be passed as input. The
         * command keyword itself is the first token.
         *
         * Caution: This method will be invoked asynchronously from another thread. Blocking calls during
         * execution can lead to unpredictable behavior.
         *
         * @param[in] input The command tokens including keyword
         * @returns true if the command was successful, false if not
         */
        virtual bool execute(const std::vector<std::string>& input) = 0;
    };
}

#endif
