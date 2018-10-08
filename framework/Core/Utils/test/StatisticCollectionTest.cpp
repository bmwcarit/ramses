//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"
#include "Utils/StatisticCollection.h"

using namespace testing;

namespace ramses_internal
{
    class StatisticCollectionTest : public testing::Test
    {
    protected:
        StatisticCollectionTest()
        {
        }

        StatisticCollectionFramework m_statisticCollection;
    };

    TEST_F(StatisticCollectionTest, canUpdateCounter)
    {
        EXPECT_EQ(0u, m_statisticCollection.statResourcesCreated.getCounterValue());
        m_statisticCollection.statResourcesCreated.getCounterValue();
        m_statisticCollection.statResourcesCreated.incCounter(1);
        EXPECT_EQ(1u, m_statisticCollection.statResourcesCreated.getCounterValue());
    }

    TEST_F(StatisticCollectionTest, nextTimeIntervalResetsCounter)
    {
        m_statisticCollection.statResourcesCreated.incCounter(5);
        EXPECT_NE(0u, m_statisticCollection.statResourcesCreated.getCounterValue());
        m_statisticCollection.nextTimeInterval();
        EXPECT_EQ(0u, m_statisticCollection.statResourcesCreated.getCounterValue());
    }

    TEST_F(StatisticCollectionTest, collectsMinValueOfMultipleTimeframes)
    {
        m_statisticCollection.statResourcesCreated.incCounter(6);
        m_statisticCollection.nextTimeInterval();

        m_statisticCollection.statResourcesCreated.incCounter(4);
        m_statisticCollection.nextTimeInterval();

        m_statisticCollection.statResourcesCreated.incCounter(5);
        m_statisticCollection.nextTimeInterval();

        //counter in ongoing time interval is not part of summary
        m_statisticCollection.statResourcesCreated.incCounter(1);

        EXPECT_EQ(3u, m_statisticCollection.getNumberTimeIntervalsSinceLastSummaryReset());
        auto summary = m_statisticCollection.statResourcesCreated.getSummary();
        EXPECT_EQ(4u, summary.minValue);
    }

    TEST_F(StatisticCollectionTest, collectsMaxValueOfMultipleTimeframes)
    {
        m_statisticCollection.statResourcesCreated.incCounter(4);
        m_statisticCollection.nextTimeInterval();

        m_statisticCollection.statResourcesCreated.incCounter(6);
        m_statisticCollection.nextTimeInterval();

        m_statisticCollection.statResourcesCreated.incCounter(5);
        m_statisticCollection.nextTimeInterval();

        //counter in ongoing time interval is not part of summary
        m_statisticCollection.statResourcesCreated.incCounter(8);

        EXPECT_EQ(3u, m_statisticCollection.getNumberTimeIntervalsSinceLastSummaryReset());
        auto summary = m_statisticCollection.statResourcesCreated.getSummary();
        EXPECT_EQ(6u, summary.maxValue);
    }

    TEST_F(StatisticCollectionTest, collectsSumValueOfMultipleTimeframes)
    {
        m_statisticCollection.statResourcesCreated.incCounter(4);
        m_statisticCollection.nextTimeInterval();

        m_statisticCollection.statResourcesCreated.incCounter(6);
        m_statisticCollection.nextTimeInterval();

        m_statisticCollection.statResourcesCreated.incCounter(5);
        m_statisticCollection.nextTimeInterval();

        //counter in ongoing time interval is not part of summary
        m_statisticCollection.statResourcesCreated.incCounter(8);

        EXPECT_EQ(3u, m_statisticCollection.getNumberTimeIntervalsSinceLastSummaryReset());
        auto summary = m_statisticCollection.statResourcesCreated.getSummary();
        EXPECT_EQ(15u, summary.sum);
    }

    TEST_F(StatisticCollectionTest, reset)
    {
        m_statisticCollection.statResourcesCreated.incCounter(4);
        m_statisticCollection.nextTimeInterval();
        m_statisticCollection.statResourcesCreated.incCounter(6);

        EXPECT_NE(0u, m_statisticCollection.statResourcesCreated.getCounterValue());
        EXPECT_NE(std::numeric_limits<uint32_t>::max(), m_statisticCollection.statResourcesCreated.getSummary().minValue);
        EXPECT_NE(std::numeric_limits<uint32_t>::min(), m_statisticCollection.statResourcesCreated.getSummary().maxValue);
        EXPECT_NE(0u, m_statisticCollection.statResourcesCreated.getSummary().sum);

        m_statisticCollection.reset();

        EXPECT_EQ(0u, m_statisticCollection.statResourcesCreated.getCounterValue());
        EXPECT_EQ(std::numeric_limits<uint32_t>::max(), m_statisticCollection.statResourcesCreated.getSummary().minValue);
        EXPECT_EQ(std::numeric_limits<uint32_t>::min(), m_statisticCollection.statResourcesCreated.getSummary().maxValue);
        EXPECT_EQ(0u, m_statisticCollection.statResourcesCreated.getSummary().sum);
    }

    TEST_F(StatisticCollectionTest, resetSummaries)
    {
        m_statisticCollection.statResourcesCreated.incCounter(4);
        m_statisticCollection.nextTimeInterval();
        m_statisticCollection.statResourcesCreated.incCounter(6);

        EXPECT_NE(0u, m_statisticCollection.statResourcesCreated.getCounterValue());
        EXPECT_NE(std::numeric_limits<uint32_t>::max(), m_statisticCollection.statResourcesCreated.getSummary().minValue);
        EXPECT_NE(std::numeric_limits<uint32_t>::min(), m_statisticCollection.statResourcesCreated.getSummary().maxValue);
        EXPECT_NE(0u, m_statisticCollection.statResourcesCreated.getSummary().sum);

        m_statisticCollection.resetSummaries();

        EXPECT_EQ(std::numeric_limits<uint32_t>::max(), m_statisticCollection.statResourcesCreated.getSummary().minValue);
        EXPECT_EQ(std::numeric_limits<uint32_t>::min(), m_statisticCollection.statResourcesCreated.getSummary().maxValue);
        EXPECT_EQ(0u, m_statisticCollection.statResourcesCreated.getSummary().sum);

        //current counter is not reset by resetSummaries
        EXPECT_NE(0u, m_statisticCollection.statResourcesCreated.getCounterValue());
    }
}
