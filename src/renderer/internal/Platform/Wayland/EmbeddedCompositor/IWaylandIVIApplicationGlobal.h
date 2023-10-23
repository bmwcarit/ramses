//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <cstdint>

namespace ramses::internal
{
    class IWaylandClient;

    class IWaylandIVIApplicationGlobal
    {
    public:
        virtual ~IWaylandIVIApplicationGlobal() = default;

    protected:
        virtual void iviApplicationBind(IWaylandClient& client, uint32_t version, uint32_t id) = 0;
    };
}
