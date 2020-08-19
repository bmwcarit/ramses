//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IWAYLANDIVIAPPLICATIONGLOBAL_H
#define RAMSES_IWAYLANDIVIAPPLICATIONGLOBAL_H

#include <cstdint>

namespace ramses_internal
{
    class IWaylandClient;

    class IWaylandIVIApplicationGlobal
    {
    public:
        virtual ~IWaylandIVIApplicationGlobal(){}

    protected:
        virtual void iviApplicationBind(IWaylandClient& client, uint32_t version, uint32_t id) = 0;
    };
}

#endif
