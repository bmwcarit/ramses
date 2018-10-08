//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSHFACTORY_H
#define RAMSES_RAMSHFACTORY_H

#include "RamsesFrameworkConfigImpl.h"

namespace ramses_internal
{
    class Ramsh;
    class RamshFactory
    {
    public:
        static Ramsh* ConstructRamsh(const ramses::RamsesFrameworkConfigImpl& config);
    };

}

#endif
