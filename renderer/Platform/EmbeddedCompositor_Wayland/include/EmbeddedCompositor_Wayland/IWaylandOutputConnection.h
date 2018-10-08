//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#ifndef RAMSES_IWAYLANDOUTPUTCONNECTION_H
#define RAMSES_IWAYLANDOUTPUTCONNECTION_H

namespace ramses_internal
{
    class IWaylandClient;

    class IWaylandOutputConnection
    {
    public:
        virtual ~IWaylandOutputConnection(){}
        virtual void resourceDestroyed() = 0;
    };
}

#endif
