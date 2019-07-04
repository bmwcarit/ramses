//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_LOGDCSMINFO_H
#define RAMSES_LOGDCSMINFO_H

#include "Ramsh/RamshCommand.h"

namespace ramses_internal
{
    class DcsmComponent;

    class LogDcsmInfo : public RamshCommand
    {
    public:
        explicit LogDcsmInfo(DcsmComponent& dcsmComponent);
        virtual Bool executeInput(const RamshInput& input) override;

    private:
        DcsmComponent& m_dcsmComponent;
    };
}

#endif
