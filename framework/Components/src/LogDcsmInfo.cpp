//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Components/LogDcsmInfo.h"
#include "Components/DcsmComponent.h"

namespace ramses_internal
{
    LogDcsmInfo::LogDcsmInfo(DcsmComponent& dcsmComponent)
        : m_dcsmComponent(dcsmComponent)
    {
        description = "print dcsm information";
        registerKeyword("dinfo");
        registerKeyword("printDcsmInfo");
    }

    Bool LogDcsmInfo::executeInput(const RamshInput& /*input*/)
    {
        m_dcsmComponent.logInfo();
        return true;
    }
}
