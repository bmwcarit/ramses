//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
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

    class DumpSceneToFile : public RamshCommand
    {
    public:
        explicit DumpSceneToFile(RamsesClientImpl& client);
        bool executeInput(const std::vector<std::string>& input) override;

    private:
        RamsesClientImpl& m_client;
    };
}
