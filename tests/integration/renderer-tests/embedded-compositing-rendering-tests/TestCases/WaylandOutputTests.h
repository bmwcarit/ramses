//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "IEmbeddedCompositingTest.h"

namespace ramses::internal
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

        static bool CheckWaylandOutputParams(const WaylandOutputTestParams& waylandOutputParams, uint32_t expectedWidth, uint32_t expectedHeight, bool expectMode, bool expectScale, bool expectDone);
        static bool CheckWaylandOutputGeometry(const WaylandOutputTestParams& waylandOutputParams);
        static bool CheckWaylandOutputMode(const WaylandOutputTestParams& waylandOutputParams, uint32_t expectedWidth, uint32_t expectedHeight);
        static bool CheckWaylandOutputScale(const WaylandOutputTestParams& waylandOutputParams);
    };
}
