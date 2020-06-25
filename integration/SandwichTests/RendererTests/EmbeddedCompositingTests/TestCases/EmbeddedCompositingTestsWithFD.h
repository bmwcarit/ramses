//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EMBEDDEDCOMPOSITINGTESTSWITHFD_H
#define RAMSES_EMBEDDEDCOMPOSITINGTESTSWITHFD_H

#include "IEmbeddedCompositingTest.h"

namespace ramses_internal
{
    class EmbeddedCompositingTestsWithFD : public IEmbeddedCompositingTest
    {
    public:
        explicit EmbeddedCompositingTestsWithFD(const String& waylandSocket)
            : m_waylandSocket(waylandSocket)
        {
        }

        virtual void setUpEmbeddedCompositingTestCases(EmbeddedCompositingTestsFramework& testFramework) final;
        virtual bool runEmbeddedCompositingTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase) final;

    private:

        enum
        {
            FDTest_CanConnectUsingWaylandSocket,
            FDTest_CanConnectUsingWaylandSocket_TwoApplicationsInSequence,
            FDTest_WaylandClientCanConnectToSocketBeforeEmbeddedCompositorIsInitialized,
            FDTest_WaylandClientCanConnectToSocketBeforeAndAfterEmbeddedCompositorIsInitialized,
            FDTest_CanConnectUsingWaylandSocketAndWaylandDisplay,
        };

        const String m_waylandSocket;
    };
}

#endif
