//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_LOGCONNECTIONINFO_H
#define RAMSES_LOGCONNECTIONINFO_H

#include "Ramsh/RamshCommand.h"

namespace ramses_internal
{
    class String;
    class ICommunicationSystem;

    class LogConnectionInfo : public RamshCommand
    {
    public:
        explicit LogConnectionInfo(ICommunicationSystem& communicationSystem);
        bool executeInput(const std::vector<std::string>& input) override;

    private:
        ICommunicationSystem& m_communicationSystem;
    };
}

#endif
