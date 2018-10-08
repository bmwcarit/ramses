//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSHCOMMANDSETCONTEXTLOGLEVEL_H
#define RAMSES_RAMSHCOMMANDSETCONTEXTLOGLEVEL_H

#include "Ramsh/RamshCommand.h"


namespace ramses_internal
{
    class Ramsh;

    class RamshCommandSetContextLogLevel : public RamshCommand
    {
    public:
        explicit RamshCommandSetContextLogLevel(const Ramsh& ramsh);
        virtual Bool executeInput(const RamshInput& input) override;

    protected:
        const Ramsh& m_ramsh;
    };

}// namespace ramses_internal

#endif
