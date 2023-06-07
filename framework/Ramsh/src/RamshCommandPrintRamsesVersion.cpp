//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Ramsh/RamshCommandPrintRamsesVersion.h"
#include "Ramsh/Ramsh.h"
#include "Utils/LogMacros.h"
#include "ramses-sdk-build-config.h"

namespace ramses_internal
{
    RamshCommandPrintRamsesVersion::RamshCommandPrintRamsesVersion()
    {
        registerKeyword("ramsesVersion");
        description = "print RAMSES version";
    }

    bool RamshCommandPrintRamsesVersion::executeInput(const std::vector<std::string>& /*input*/)
    {
        LOG_INFO(CONTEXT_RAMSH, ::ramses_sdk::RAMSES_SDK_RAMSES_VERSION);
        return true;
    }

}
