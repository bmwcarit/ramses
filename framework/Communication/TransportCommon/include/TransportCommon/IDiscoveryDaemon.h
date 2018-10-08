//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IDISCOVERYDAEMON_H
#define RAMSES_IDISCOVERYDAEMON_H

namespace ramses_internal
{
    class IDiscoveryDaemon
    {
    public:
        virtual ~IDiscoveryDaemon() {}

        virtual bool start() = 0;
        virtual bool stop() = 0;
    };
}

#endif
