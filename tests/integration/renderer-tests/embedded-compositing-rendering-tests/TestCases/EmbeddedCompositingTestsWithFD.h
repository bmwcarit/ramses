//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "IEmbeddedCompositingTest.h"

namespace ramses::internal
{
    class EmbeddedCompositingTestsWithFD : public IEmbeddedCompositingTest
    {
    public:
        void setUpEmbeddedCompositingTestCases(EmbeddedCompositingTestsFramework& testFramework) final;
        bool runEmbeddedCompositingTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase) final;

    private:

        enum
        {
            FDTest_CanConnectUsingWaylandSocket,
            FDTest_CanConnectUsingWaylandSocket_TwoApplicationsInSequence,
            FDTest_WaylandClientCanConnectToSocketBeforeEmbeddedCompositorIsInitialized,
            FDTest_WaylandClientCanConnectToSocketBeforeAndAfterEmbeddedCompositorIsInitialized,
            FDTest_CanConnectUsingWaylandSocketAndWaylandDisplay,
        };

        const std::string m_waylandSocket = EmbeddedCompositingTestsFramework::TestEmbeddedCompositingDisplayName;
    };
}
