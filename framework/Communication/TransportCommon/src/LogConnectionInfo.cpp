//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "TransportCommon/LogConnectionInfo.h"
#include "TransportCommon/ICommunicationSystem.h"

namespace ramses_internal
{
    LogConnectionInfo::LogConnectionInfo(const ICommunicationSystem& communicationSystem)
        : m_communicationSystem(communicationSystem)
    {
        description = "print connection information";
        registerKeyword("cinfo");
        registerKeyword("printConnectionInfo");
    }

    Bool LogConnectionInfo::executeInput(const RamshInput& /*input*/)
    {
        m_communicationSystem.logConnectionInfo();
        return true;
    }
}
