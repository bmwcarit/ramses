//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "internal/Core/Utils/PeriodicLoggerHelper.h"
#include "gtest/gtest.h"

namespace ramses::internal
{
    TEST(PeriodicLoggerHelper, SummaryEntry_UInt32)
    {
        SummaryEntry<uint32_t> summary;

        {
            StringOutputStream stream{std::string{}};
            logStatisticSummaryEntry(stream, summary, 0);
            EXPECT_EQ(stream.data(), std::string{"(n.a.)"});
        }

        summary.update(0);
        summary.update(0);
        {
            StringOutputStream stream{std::string{}};
            logStatisticSummaryEntry(stream, summary, 1);
            EXPECT_EQ(stream.data(), std::string{"(0)"});
        }
        summary.update(2);
        {
            StringOutputStream stream{std::string{}};
            logStatisticSummaryEntry(stream, summary, 0);
            EXPECT_EQ(stream.data(), std::string{"(n.a.)"});
        }
        {
            StringOutputStream stream{std::string{}};
            logStatisticSummaryEntry(stream, summary, 1);
            EXPECT_EQ(stream.data(), std::string{"(0/2/2)"});
        }
        {
            StringOutputStream stream{std::string{}};
            logStatisticSummaryEntry(stream, summary, 2);
            EXPECT_EQ(stream.data(), std::string{"(0/2/1)"});
        }
    }

    TEST(PeriodicLoggerHelper, FirstFiveElements_UInt64)
    {
        FirstFiveElements<uint64_t> summary;

        {
            StringOutputStream stream{std::string{}};
            logStatisticSummaryEntry(stream, summary, 0);
            EXPECT_EQ(stream.data(), std::string{"(n.a.)"});
        }

        {
            StringOutputStream stream{std::string{}};
            logStatisticSummaryEntry(stream, summary, 1);
            EXPECT_EQ(stream.data(), std::string{"(n.a.)"});
        }

        summary.update(1);
        {
            StringOutputStream stream{std::string{}};
            logStatisticSummaryEntry(stream, summary, 0);
            EXPECT_EQ(stream.data(), std::string{"(n.a.)"});
        }

        {
            StringOutputStream stream{std::string{}};
            logStatisticSummaryEntry(stream, summary, 1);
            EXPECT_EQ(stream.data(), std::string{"(1)"});
        }

        {
            StringOutputStream stream{std::string{}};
            logStatisticSummaryEntry(stream, summary, 2);
            EXPECT_EQ(stream.data(), std::string{"(1/...)"});
        }

        summary.update(2);
        {
            StringOutputStream stream{std::string{}};
            logStatisticSummaryEntry(stream, summary, 0);
            EXPECT_EQ(stream.data(), std::string{"(n.a.)"});
        }

        {
            StringOutputStream stream{std::string{}};
            logStatisticSummaryEntry(stream, summary, 1);
            EXPECT_EQ(stream.data(), std::string{"(1/2)"});
        }

        {
            StringOutputStream stream{std::string{}};
            logStatisticSummaryEntry(stream, summary, 2);
            EXPECT_EQ(stream.data(), std::string{"(1/2)"});
        }

        {
            StringOutputStream stream{std::string{}};
            logStatisticSummaryEntry(stream, summary, 3);
            EXPECT_EQ(stream.data(), std::string{"(1/2/...)"});
        }

        summary.update(3);
        summary.update(4);
        summary.update(5);
        summary.update(6);
        {
            StringOutputStream stream{std::string{}};
            logStatisticSummaryEntry(stream, summary, 4);
            EXPECT_EQ(stream.data(), std::string{"(1/2/3/4/5)"});
        }
        {
            StringOutputStream stream{std::string{}};
            logStatisticSummaryEntry(stream, summary, 5);
            EXPECT_EQ(stream.data(), std::string{"(1/2/3/4/5)"});
        }
        {
            StringOutputStream stream{std::string{}};
            logStatisticSummaryEntry(stream, summary, 6);
            EXPECT_EQ(stream.data(), std::string{"(1/2/3/4/5/...)"});
        }
    }
}
