//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "PublicRamshCommand.h"
#include "ramses/framework/IRamshCommand.h"
#include <cassert>

namespace ramses::internal
{
    PublicRamshCommand::PublicRamshCommand(const std::shared_ptr<ramses::IRamshCommand>& command)
        : m_command(command)
    {
        assert(command);
        registerKeyword(command->keyword());
        description = command->help();
    }

    bool PublicRamshCommand::executeInput(const std::vector<std::string>& input)
    {
        if (auto command = m_command.lock())
            return command->execute(input);
        return false;
    }
}
