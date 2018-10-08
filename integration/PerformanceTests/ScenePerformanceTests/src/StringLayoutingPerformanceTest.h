//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_STRINGLAYOUTINGPERFORMANCETEST_H
#define RAMSES_STRINGLAYOUTINGPERFORMANCETEST_H

#include "PerformanceTestBase.h"
#include "ramses-text-api/FontRegistry.h"
#include "ramses-text-api/GlyphMetrics.h"
#include <string>

class StringLayoutingPerformanceTest : public PerformanceTestBase
{
public:
    enum
    {
        StringLayoutingPerformanceTest_LayoutBigString = 0,
    };

    StringLayoutingPerformanceTest(ramses_internal::String testName, uint32_t testState);

    virtual void initTest(ramses::RamsesClient& client, ramses::Scene& scene) override;
    virtual void update() override;

private:
    std::u32string m_string;
    ramses::GlyphMetricsVector m_glyphs;
    ramses::FontRegistry m_fontRegistry;
    ramses::GlyphMetricsVector::const_iterator m_currentIter;
};
#endif
