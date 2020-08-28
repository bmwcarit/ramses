//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_COMMUNICATION_SOMEIPADAPTER_H
#define RAMSES_COMMUNICATION_SOMEIPADAPTER_H

#include "Collections/Guid.h"
#include "TransportCommon/EConnectionProtocol.h"
#ifdef HAS_SOMEIP
#include "TransportSomeIP/SomeIPCommunicationConfiguration.h"
#endif

namespace ramses_internal
{
    class SomeIPAdapter
    {
    public:
        static Guid GetGuidForCommunicationUser(uint32_t communicationUserID);
        static uint32_t GetInvalidCommunicationUser();

        static EConnectionProtocol GetUsedSomeIPStack(uint32_t communicationUserID);
        static bool IsSomeIPStackCompiled(EConnectionProtocol connectionProtocol);

        static bool GetCommunicationUserAsString(uint32_t communicationUserID, String& outStr);
    };

    inline Guid SomeIPAdapter::GetGuidForCommunicationUser(uint32_t communicationUserID)
    {
#ifdef HAS_SOMEIP
        return SomeIPCommunicationConfiguration::GetGuidForSomeIPUser(communicationUserID);
#else
        UNUSED(communicationUserID);
        return Guid();
#endif
    }

    inline uint32_t SomeIPAdapter::GetInvalidCommunicationUser()
    {
#ifdef HAS_SOMEIP
        return SomeIPCommunicationConfiguration::ERamsesCommunicationUserInternal_Invalid;
#else
        return 1000;
#endif
    }

    inline EConnectionProtocol SomeIPAdapter::GetUsedSomeIPStack(uint32_t communicationUserID)
    {
#ifdef HAS_SOMEIP
        return SomeIPCommunicationConfiguration::getUsedSomeIPStack(communicationUserID);
#else
        UNUSED(communicationUserID);
        return EConnectionProtocol::Invalid;
#endif
    }

    inline bool SomeIPAdapter::IsSomeIPStackCompiled(EConnectionProtocol connectionProtocol)
    {
#ifdef HAS_SOMEIP
        return SomeIPCommunicationConfiguration::isSomeIPStackCompiled(connectionProtocol);
#else
        UNUSED(connectionProtocol);
        return false;
#endif
    }

    inline bool SomeIPAdapter::GetCommunicationUserAsString(uint32_t communicationUserID, String& outStr)
    {
#ifdef HAS_SOMEIP
        if (SomeIPCommunicationConfiguration::getUsedSomeIPStack(communicationUserID) != EConnectionProtocol::Invalid)
        {
            outStr = SomeIPCommunicationConfiguration::getCommunicationUserIDName(communicationUserID);
            return true;
        }
#endif
        UNUSED(communicationUserID);
        UNUSED(outStr);
        return false;
    }
}

#endif
