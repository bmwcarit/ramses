//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestStepCommand.h"

namespace ramses_internal
{
    TestStepCommand::TestStepCommand():
        m_testStep(0)
    {
        description = "go to next test step";
        registerKeyword("step");
        getArgument<0>().setDescription("test step nr.");
    }

    Bool TestStepCommand::execute(Int& testStep) const
    {
        m_testStep = testStep;
        m_testStepSetEvent.signal();
        return true;
    }

    Int TestStepCommand::getCurrentTestStep()
    {
        return m_testStep.load();
    }

    void TestStepCommand::waitForTestStepSetEvent(UInt32 timeoutMillisec)
    {
        m_testStepSetEvent.wait(timeoutMillisec);
    }
}
