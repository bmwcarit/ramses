//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Ramsh/RamshCommandPrintBuildConfig.h"
#include "Ramsh/Ramsh.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    RamshCommandPrintBuildConfig::RamshCommandPrintBuildConfig(const char* const configString)
        : m_configString(configString)
    {
        registerKeyword("buildConfig");
        description = "print build configuration";
    }

    Bool RamshCommandPrintBuildConfig::executeInput(const RamshInput& input)
    {
        UNUSED(input);
        LOG_INFO(CONTEXT_RAMSH, m_configString);
        return true;
    }

}
