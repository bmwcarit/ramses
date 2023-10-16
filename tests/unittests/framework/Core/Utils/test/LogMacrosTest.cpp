//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Core/Utils/LogMacros.h"
#include "gtest/gtest.h"

namespace ramses::internal
{
    TEST(ALogMacro, canUseStringOutputStreamLog)
    {
        LOG_INFO(CONTEXT_FRAMEWORK, "Foo " << 123 << 456);
    }

    TEST(ALogMacro, canUseStringOutputStreamLambdaLog)
    {
        LOG_INFO_F(CONTEXT_FRAMEWORK, ([&](StringOutputStream& sos) {
            sos << "foo";
            sos << 123 << 456;
        }));
        LOG_INFO_F(CONTEXT_FRAMEWORK, ([&](auto& sos) {
            sos << "foo";
            sos << 123 << 456;
        }));
    }

    TEST(ALogMacro, canUseFmtlibLog)
    {
        LOG_INFO_P(CONTEXT_FRAMEWORK, "Foo {} {}", 123, 456.f);
    }

    TEST(ALogMacro, canUseFmtlibLambdaLog)
    {
        LOG_INFO_PF(CONTEXT_FRAMEWORK, ([&](fmt::memory_buffer& out) {
            fmt::format_to(out, "Foo {} {}", 123, 456.f);
        }));
        LOG_INFO_PF(CONTEXT_FRAMEWORK, ([&](auto& out) { fmt::format_to(out, "Foo {} {}", 123, 456.f); }));
    }
}

namespace ramses
{
    TEST(ALogMacro, canUseContextFromRamsesNamespace)
    {
        LOG_INFO(CONTEXT_FRAMEWORK, "Foo " << 123 << 456);
    }
}
