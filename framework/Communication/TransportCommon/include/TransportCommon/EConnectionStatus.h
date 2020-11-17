//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ECONNECTIONSTATUS_H
#define RAMSES_ECONNECTIONSTATUS_H

#include "PlatformAbstraction/PlatformTypes.h"
#include <assert.h>

namespace ramses_internal
{
    enum EConnectionStatus
    {
        EConnectionStatus_NotConnected = 0,
        EConnectionStatus_Connected
    };

    inline const Char* EnumToString(EConnectionStatus connectionStatus)
    {
        switch (connectionStatus)
        {
            case EConnectionStatus_NotConnected:
                return "Not connected";
            case EConnectionStatus_Connected:
                return "Connected";
            default:
                assert(false && "Unknown EConnectionStatus type");
                return "Unknown connection status type";
        }
    }
}
#endif
