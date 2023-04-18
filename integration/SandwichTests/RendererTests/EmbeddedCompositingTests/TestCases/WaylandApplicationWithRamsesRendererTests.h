//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WAYLANDAPPLICATIONWITHRAMSESRENDERERTESTS_H
#define RAMSES_WAYLANDAPPLICATIONWITHRAMSESRENDERERTESTS_H

#include "IEmbeddedCompositingTest.h"

namespace ramses_internal
{
    class WaylandApplicationWithRamsesRendererTests : public IEmbeddedCompositingTest
    {
    public:
        void setUpEmbeddedCompositingTestCases(EmbeddedCompositingTestsFramework& testFramework) final;
        bool runEmbeddedCompositingTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase) final;

    private:
        enum
        {
            CanRunRamsesRendererWithinExistingWaylandApplication,
        };
    };
}

#endif
