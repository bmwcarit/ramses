//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TESTSTEPCOMMAND_H
#define RAMSES_TESTSTEPCOMMAND_H

#include "Ramsh/RamshCommandArguments.h"
#include "PlatformAbstraction/PlatformEvent.h"

namespace ramses_internal
{
    class TestStepCommand : public RamshCommandArgs<Int>
    {
    public:
        explicit TestStepCommand();
        bool execute(Int& testStep) const override;

        Int getCurrentTestStep();
        void waitForTestStepSetEvent(UInt32 timeoutMillisec = 0u);

    private:
        mutable std::atomic<Int> m_testStep;
        mutable PlatformEvent m_testStepSetEvent;
    };
}

#endif
