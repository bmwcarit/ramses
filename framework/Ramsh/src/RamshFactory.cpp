//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Ramsh/Ramsh.h"
#include "ramses-sdk-build-config.h"
#include "Utils/LogMacros.h"
#include "Ramsh/RamshCommandSetContextLogLevelFilter.h"
#include "Ramsh/RamshFactory.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "Ramsh/RamshStandardSetup.h"
#include "Ramsh/RamshDLT.h"

namespace ramses_internal
{

    Ramsh* RamshFactory::ConstructRamsh(const ramses::RamsesFrameworkConfigImpl& config)
    {
        if (config.m_shellType == ramses::ERamsesShellType_Console)
        {
            return new RamshStandardSetup("ramses");
        }
        else if (config.m_shellType == ramses::ERamsesShellType_Default)
        {
            return new RamshDLT("ramses");
        }
        return new Ramsh("ramses");
    }

}
