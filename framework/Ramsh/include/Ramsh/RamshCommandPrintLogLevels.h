//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSHCOMMANDPRINTLOGLEVELS_H
#define RAMSES_RAMSHCOMMANDPRINTLOGLEVELS_H

#include "Ramsh/RamshCommand.h"

namespace ramses_internal
{
    class Ramsh;

    class RamshCommandPrintLogLevels : public RamshCommand
    {
    public:
        explicit RamshCommandPrintLogLevels(const Ramsh& ramsh);
        bool executeInput(const std::vector<std::string>& input) override;

    protected:
        const Ramsh& m_ramsh;
    };

}// namespace ramses_internal

#endif
