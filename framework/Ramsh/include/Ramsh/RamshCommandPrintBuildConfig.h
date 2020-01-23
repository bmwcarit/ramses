//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSHCOMMANDPRINTBUILDCONFIG_H
#define RAMSES_RAMSHCOMMANDPRINTBUILDCONFIG_H

#include "Ramsh/RamshCommand.h"

namespace ramses_internal
{
    class Ramsh;

    class RamshCommandPrintBuildConfig : public RamshCommand
    {
    public:
        explicit RamshCommandPrintBuildConfig(const char* const configStrings);
        virtual bool executeInput(const RamshInput& input) override;

    protected:
        const char* const m_configString;
    };

}// namespace ramses_internal

#endif
