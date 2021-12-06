//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DMAOFFSCREENBUFFERRENDERINGTESTS_H
#define RAMSES_DMAOFFSCREENBUFFERRENDERINGTESTS_H

#include "RendererTestsFramework.h"
#include "DmaOffscreenBufferTests.h"
#include "Utils/LogMacros.h"

class DmaOffscreenBufferRenderingTests
{
public:
    DmaOffscreenBufferRenderingTests(const ramses_internal::StringVector& filterIn, const ramses_internal::StringVector& filterOut, bool generateScreenshots, const ramses::RamsesFrameworkConfig& config)
        : m_testFramework(generateScreenshots, config)
    {
        m_dmaOffscreenBufferTests.setUpTestCases(m_testFramework);
        m_testFramework.filterTestCases(filterIn, filterOut);
    }

    bool runTests()
    {
        return m_testFramework.runAllTests();
    }

    void logReport()
    {
        fmt::print("{}\n", m_testFramework.generateReport());
    }

protected:
    RendererTestsFramework m_testFramework;
    DmaOffscreenBufferTests m_dmaOffscreenBufferTests;
};

#endif
