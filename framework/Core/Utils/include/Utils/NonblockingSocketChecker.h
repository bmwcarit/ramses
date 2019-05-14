//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_NONBLOCKINGSOCKETCHECKER_H
#define RAMSES_NONBLOCKINGSOCKETCHECKER_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "ramses-capu/os/NonBlockSocketChecker.h"
#include "Collections/Vector.h"

namespace ramses_internal
{

    typedef ramses_capu::os::SocketDescription SocketDescription;
    typedef std::vector<ramses_capu::os::SocketInfoPair> SocketInfoVector;
    typedef ramses_capu::os::SocketDelegate SocketDelegate;

    typedef ramses_capu::os::SocketInfoPair SocketInfoPair;

    class NonBlockingSocketChecker : private ramses_capu::NonBlockSocketChecker
    {
    public:
        static void CheckSocketsForIncomingData(const SocketInfoVector& socketsToCheck);
        static void CheckSocketsForIncomingData(const SocketInfoVector& socketsToCheck, UInt32 timeout);
    };

    inline void NonBlockingSocketChecker::CheckSocketsForIncomingData(const SocketInfoVector& socketsToCheck)
    {
        ramses_capu::NonBlockSocketChecker::CheckSocketsForIncomingData(socketsToCheck);
    }

    inline void NonBlockingSocketChecker::CheckSocketsForIncomingData(const SocketInfoVector& socketsToCheck, UInt32 timeout)
    {
        ramses_capu::NonBlockSocketChecker::CheckSocketsForIncomingData(socketsToCheck, timeout);
    }

}


#endif
