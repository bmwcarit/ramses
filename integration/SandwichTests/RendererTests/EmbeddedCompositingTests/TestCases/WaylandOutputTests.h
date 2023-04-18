//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WAYLANDOUTPUTTESTS_H
#define RAMSES_WAYLANDOUTPUTTESTS_H

#include "IEmbeddedCompositingTest.h"

namespace ramses_internal
{
    class WaylandOutputTests : public IEmbeddedCompositingTest
    {
    public:
        WaylandOutputTests();
        void setUpEmbeddedCompositingTestCases(EmbeddedCompositingTestsFramework& testFramework) final;
        bool runEmbeddedCompositingTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase) final;

    private:
        enum
        {
            WaylandOutput_Version1,
            WaylandOutput_Version2,
            WaylandOutput_Version3,
        };

        bool checkWaylandOutputParams(const WaylandOutputTestParams& waylandOutputParams, uint32_t expectedWidth, uint32_t expectedHeight, bool expectMode, bool expectScale, bool expectDone);
        bool checkWaylandOutputGeometry(const WaylandOutputTestParams& waylandOutputParams);
        bool checkWaylandOutputMode(const WaylandOutputTestParams& waylandOutputParams, uint32_t expectedWidth, uint32_t expectedHeight);
        bool checkWaylandOutputScale(const WaylandOutputTestParams& waylandOutputParams);
    };
}

#endif
