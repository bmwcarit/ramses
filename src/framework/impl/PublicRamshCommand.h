//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Ramsh/RamshCommand.h"
#include <memory>
#include <string>
#include <vector>

namespace ramses
{
    class IRamshCommand;
}

namespace ramses::internal
{
    class PublicRamshCommand : public RamshCommand
    {
    public:
        explicit PublicRamshCommand(const std::shared_ptr<IRamshCommand>& command);

        bool executeInput(const std::vector<std::string>& input) override;

    private:
        std::weak_ptr<IRamshCommand> m_command;
    };
}
