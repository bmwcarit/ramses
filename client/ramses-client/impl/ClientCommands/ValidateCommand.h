//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_VALIDATECOMMAND_H
#define RAMSES_VALIDATECOMMAND_H

#include "Ramsh/RamshCommandArguments.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"

#include <string>

namespace ramses
{
    class RamsesClientImpl;
}

namespace ramses_internal
{
    class ValidateCommand : public RamshCommandArgs<uint64_t, std::string, std::string>
    {
    public:
        explicit ValidateCommand(ramses::RamsesClientImpl& client);
        bool execute(uint64_t& sceneId, std::string& severity, std::string& objectName) const override;
    private:
        ramses::RamsesClientImpl& m_client;
    };
}

#endif
