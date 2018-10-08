//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DefaultRendererCacheTest.h"

DefaultRendererCacheTest::DefaultRendererCacheTest(ramses_internal::String testName, uint32_t testState) : PerformanceTestBase(testName, testState) {};

void DefaultRendererCacheTest::initTest(ramses::RamsesClient& client, ramses::Scene& scene)
{
    UNUSED(client);
    UNUSED(scene);

    static const uint8_t buffer[10000] = {0};

    for (auto i = 0; i < 1000; i++)
    {
        m_cache.storeResource(ramses::rendererResourceId_t(i, 0), buffer, sizeof(buffer), ramses::resourceCacheFlag_t(123), ramses::sceneId_t(1));
    }
}

void DefaultRendererCacheTest::update()
{
    switch (m_testState)
    {
    case DefaultRendererCacheTest_HasResource_Positive:
    {
        for (auto i = 0; i < 1000; i++)
        {
            // These resource ids will always exist
            uint32_t size = 0;
            const bool found = m_cache.hasResource(ramses::rendererResourceId_t(i, 0), size);

            if (!found)
            {
                DummyMethod();
            }
        }

        break;
    }
    case DefaultRendererCacheTest_HasResource_Negative:
    {
        for (auto i = 0; i < 1000; i++)
        {
            // These resource ids will never exist
            uint32_t size = 0;
            const bool found = m_cache.hasResource(ramses::rendererResourceId_t(i + 50000, 0), size);

            if (found)
            {
                DummyMethod();
            }
        }

        break;
    }
    case DefaultRendererCacheTest_GetResource:
    {
        static uint8_t buffer[10000];

        for (auto i = 0; i < 1000; i++)
        {
            const bool success = m_cache.getResourceData(ramses::rendererResourceId_t(i, 0), buffer, sizeof(buffer));

            if (!success)
            {
                DummyMethod();
            }
        }

        break;
    }
    case DefaultRendererCacheTest_StoreResource:
    {
        static const uint8_t data[10000] = {0};

        // The cache is already maxed out, so each call to will result in a old item being deleted + the new item copied in
        for (auto i = 0; i < 1000; i++)
        {
            m_cache.storeResource(ramses::rendererResourceId_t(m_newResourceId, 0), data, sizeof(data), ramses::resourceCacheFlag_t(123), ramses::sceneId_t(321));
            m_newResourceId++; // This is will ensure a new id for each new item
        }

        break;
    }
    default:
    {
        assert(false);
        break;
    }
    }
}

// Never called. This is only here to make sure things are not optimized away.
void DefaultRendererCacheTest::DummyMethod()
{
    assert(false);
    ramses::DefaultRendererResourceCache temp(123);
    temp.saveToFile("");
}
