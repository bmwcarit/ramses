//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Ramsh/RamshCommandArguments.h"
#include "internal/PlatformAbstraction/PlatformEvent.h"

namespace ramses::internal
{
    class TestStepCommand : public RamshCommandArgs<uint8_t>
    {
    public:
        explicit TestStepCommand();
        bool execute(uint8_t& testStep) const override;

        uint8_t getCurrentTestStep();
        void waitForTestStepSetEvent(uint32_t timeoutMillisec = 0u);

    private:
        mutable std::atomic<uint8_t> m_testStep;
        mutable PlatformEvent m_testStepSetEvent;
    };
}
