//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SOMEIPICConfig.h"

namespace ramses
{
    SOMEIPICConfig::SOMEIPICConfig()
        : m_ipAddress("127.0.0.1")
    {
    }

    const ramses_internal::String& SOMEIPICConfig::getIPAddress() const
    {
        return m_ipAddress;
    }

    void SOMEIPICConfig::setIPAddress(const ramses_internal::String& ipAddress)
    {
        m_ipAddress = ipAddress;
    }
}
