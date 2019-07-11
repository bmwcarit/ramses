//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DEFAULTRENDERERCACHETEST_H
#define RAMSES_DEFAULTRENDERERCACHETEST_H

#include "PerformanceTestBase.h"
#include "ramses-renderer-api/DefaultRendererResourceCache.h"

class DefaultRendererCacheTest : public PerformanceTestBase
{
public:

    enum
    {
        DefaultRendererCacheTest_HasResource_Positive = 0,
        DefaultRendererCacheTest_HasResource_Negative,
        DefaultRendererCacheTest_GetResource,
        DefaultRendererCacheTest_StoreResource,
    };

    DefaultRendererCacheTest(ramses_internal::String testName, uint32_t testState);

    virtual void initTest(ramses::RamsesClient& client, ramses::Scene& scene) override;
    virtual void update() override;

private:

    static void DummyMethod();

    uint32_t m_newResourceId = 5000;
    ramses::DefaultRendererResourceCache m_cache{10 * 1000 * 1000};
};
#endif
