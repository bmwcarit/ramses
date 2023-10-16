//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Ramsh/RamshCommand.h"

namespace ramses::internal
{
    class Ramsh;

    class RamshCommandPrintBuildConfig : public RamshCommand
    {
    public:
        RamshCommandPrintBuildConfig();
        bool executeInput(const std::vector<std::string>& input) override;
    };

}// namespace ramses::internal
