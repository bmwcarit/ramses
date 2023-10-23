//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Ramsh/RamshCommand.h"

#include "internal/PlatformAbstraction/PlatformEvent.h"
#include <atomic>

namespace ramses::internal
{
    class Ramsh;

    class RamshCommandExit : public RamshCommand
    {
    public:
        explicit RamshCommandExit();
        bool executeInput(const std::vector<std::string>& input) override;

        bool exitRequested();
        void waitForExitRequest(uint32_t timeoutMillisec = 0u);

    private:
        std::atomic<bool> m_exitRequested;
        PlatformEvent m_exitEvent;
    };

}
