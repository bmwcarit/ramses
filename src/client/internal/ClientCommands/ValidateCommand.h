//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Ramsh/RamshCommandArguments.h"
#include "ramses/framework/RamsesFrameworkTypes.h"

#include <string>

namespace ramses::internal
{
    class RamsesClientImpl;

    class ValidateCommand : public RamshCommandArgs<uint64_t, std::string, std::string>
    {
    public:
        explicit ValidateCommand(RamsesClientImpl& client);
        bool execute(uint64_t& sceneId, std::string& severity, std::string& objectName) const override;
    private:
        RamsesClientImpl& m_client;
    };
}
