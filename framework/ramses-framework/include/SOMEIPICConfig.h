//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SOMEIPICCONFIG_H
#define RAMSES_SOMEIPICCONFIG_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "Collections/String.h"

namespace SomeIP
{
    class ServerManager;
    class ClientManager;
}

namespace ramses
{
    class SOMEIPICConfig
    {
    public:
        SOMEIPICConfig();

        const ramses_internal::String& getIPAddress() const;
        void setIPAddress(const ramses_internal::String& ipAddress);

    private:
        ramses_internal::String m_ipAddress;
    };
}

#endif
