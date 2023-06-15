//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IRAMSESOBJECTREGISTRY_H
#define RAMSES_IRAMSESOBJECTREGISTRY_H

#include "ramses-client-api/RamsesObjectTypes.h"

#include <string>

namespace ramses
{
    class RamsesObject;
    class RamsesObjectImpl;

    class IRamsesObjectRegistry
    {
    public:
        virtual ~IRamsesObjectRegistry() {};

        virtual void updateName(RamsesObject&, const std::string&) = 0;
    };
}

#endif
