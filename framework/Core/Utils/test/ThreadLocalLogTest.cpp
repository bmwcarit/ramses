//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/ThreadLocalLog.h"
#include "gtest/gtest.h"

namespace ramses_internal
{
    class AThreadLocalLog : public ::testing::Test
    {
    public:
        AThreadLocalLog()
        {
            ThreadLocalLog::SetPrefix(123);
        }
    };

    TEST_F(AThreadLocalLog, canGetPrefix)
    {
        EXPECT_EQ(123, ThreadLocalLog::GetPrefix());
    }

    TEST_F(AThreadLocalLog, canChangePrefix)
    {
        ThreadLocalLog::SetPrefix(456);
        EXPECT_EQ(456, ThreadLocalLog::GetPrefix());
    }

    TEST_F(AThreadLocalLog, canUseStringOutputStreamLog)
    {
        LOG_INFO_R(CONTEXT_FRAMEWORK, "Foo " << 123 << 456);
    }

    TEST_F(AThreadLocalLog, canUseStringOutputStreamLambdaLog)
    {
        LOG_INFO_RF(CONTEXT_FRAMEWORK, ([&](ramses_internal::StringOutputStream& sos) {
            sos << "foo";
            sos << 123 << 456;
        }));
        LOG_INFO_RF(CONTEXT_FRAMEWORK, ([&](auto& sos) {
            sos << "foo";
            sos << 123 << 456;
        }));
    }

    TEST_F(AThreadLocalLog, canUseFmtlibLog)
    {
        LOG_INFO_RP(CONTEXT_FRAMEWORK, "Foo {} {}", 123, 456.f);
    }

    TEST_F(AThreadLocalLog, canUseFmtlibLambdaLog)
    {
        LOG_INFO_RPF(CONTEXT_FRAMEWORK, ([&](fmt::memory_buffer& out) {
            fmt::format_to(out, "Foo {} {}", 123, 456.f);
        }));
        LOG_INFO_RPF(CONTEXT_FRAMEWORK, ([&](auto& out) { fmt::format_to(out, "Foo {} {}", 123, 456.f); }));
    }
}

namespace ramses
{
    TEST(AThreadLocalLogRamses, canUseContextFromRamsesNamespace)
    {
        ramses_internal::ThreadLocalLog::SetPrefix(123);
        LOG_INFO_R(CONTEXT_FRAMEWORK, "Foo " << 123 << 456);
    }
}
