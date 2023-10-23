//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestStepCommand.h"

namespace ramses::internal
{
    TestStepCommand::TestStepCommand():
        m_testStep(0)
    {
        description = "go to next test step";
        registerKeyword("step");
        getArgument<0>().setDescription("test step nr.");
    }

    bool TestStepCommand::execute(uint8_t& testStep) const
    {
        m_testStep = testStep;
        m_testStepSetEvent.signal();
        return true;
    }

    uint8_t TestStepCommand::getCurrentTestStep()
    {
        return m_testStep.load();
    }

    void TestStepCommand::waitForTestStepSetEvent(uint32_t timeoutMillisec)
    {
        m_testStepSetEvent.wait(timeoutMillisec);
    }
}
