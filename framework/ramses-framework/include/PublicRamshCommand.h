//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PUBLICRAMSHCOMMAND_H
#define RAMSES_PUBLICRAMSHCOMMAND_H

#include "Ramsh/RamshCommand.h"
#include <memory>
#include <string>
#include <vector>

namespace ramses
{
    class IRamshCommand;
}

namespace ramses_internal
{
    class PublicRamshCommand : public RamshCommand
    {
    public:
        explicit PublicRamshCommand(const std::shared_ptr<ramses::IRamshCommand>& command);

        bool executeInput(const std::vector<std::string>& input) override;

    private:
        std::weak_ptr<ramses::IRamshCommand> m_command;
    };
}

#endif
