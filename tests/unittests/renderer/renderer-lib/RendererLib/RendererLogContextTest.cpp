//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "internal/RendererLib/RendererLogContext.h"

namespace ramses::internal
{
    class ARendererLogContext : public ::testing::Test
    {
    public:
        ARendererLogContext()
            : contextError  (ERendererLogLevelFlag_Error)
            , contextWarn   (ERendererLogLevelFlag_Warn)
            , contextInfo   (ERendererLogLevelFlag_Info)
            , contextDetails(ERendererLogLevelFlag_Details)
        {
        }

        ~ARendererLogContext() override = default;

        RendererLogContext contextError;
        RendererLogContext contextWarn;
        RendererLogContext contextInfo;
        RendererLogContext contextDetails;
    };

    TEST_F(ARendererLogContext, activatesLogLevelFlagsErrorCorrectly)
    {
        EXPECT_TRUE (contextError.isLogLevelFlagEnabled(ERendererLogLevelFlag_Error));
        EXPECT_FALSE(contextError.isLogLevelFlagEnabled(ERendererLogLevelFlag_Warn));
        EXPECT_FALSE(contextError.isLogLevelFlagEnabled(ERendererLogLevelFlag_Info));
        EXPECT_FALSE(contextError.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details));
    }
    TEST_F(ARendererLogContext, activatesLogLevelFlagsWarnCorrectly)
    {
        EXPECT_TRUE (contextWarn.isLogLevelFlagEnabled(ERendererLogLevelFlag_Error));
        EXPECT_TRUE (contextWarn.isLogLevelFlagEnabled(ERendererLogLevelFlag_Warn));
        EXPECT_FALSE(contextWarn.isLogLevelFlagEnabled(ERendererLogLevelFlag_Info));
        EXPECT_FALSE(contextWarn.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details));
    }

    TEST_F(ARendererLogContext, activatesLogLevelFlagsInfoCorrectly)
    {
        EXPECT_TRUE (contextInfo.isLogLevelFlagEnabled(ERendererLogLevelFlag_Error));
        EXPECT_TRUE (contextInfo.isLogLevelFlagEnabled(ERendererLogLevelFlag_Warn));
        EXPECT_TRUE (contextInfo.isLogLevelFlagEnabled(ERendererLogLevelFlag_Info));
        EXPECT_FALSE(contextInfo.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details));
    }

    TEST_F(ARendererLogContext, activatesLogLevelFlagsDetailsCorrectly)
    {
        EXPECT_TRUE(contextDetails.isLogLevelFlagEnabled(ERendererLogLevelFlag_Error));
        EXPECT_TRUE(contextDetails.isLogLevelFlagEnabled(ERendererLogLevelFlag_Warn));
        EXPECT_TRUE(contextDetails.isLogLevelFlagEnabled(ERendererLogLevelFlag_Info));
        EXPECT_TRUE(contextDetails.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details));
    }

    TEST_F(ARendererLogContext, matchesAllNodesByDefault)
    {
        EXPECT_TRUE(contextInfo.isMatchingNodeHandeFilter(NodeHandle(0u)));
        EXPECT_TRUE(contextInfo.isMatchingNodeHandeFilter(NodeHandle(123u)));
        EXPECT_TRUE(contextInfo.isMatchingNodeHandeFilter(NodeHandle::Invalid()));
    }

    TEST_F(ARendererLogContext, matchesExactHandle)
    {
        contextInfo.setNodeHandleFilter(NodeHandle(123u));
        EXPECT_TRUE(contextInfo.isMatchingNodeHandeFilter(NodeHandle(123u)));
        EXPECT_FALSE(contextInfo.isMatchingNodeHandeFilter(NodeHandle(0u)));
        EXPECT_FALSE(contextInfo.isMatchingNodeHandeFilter(NodeHandle::Invalid()));
    }

    TEST_F(ARendererLogContext, matchesAllNodesWithWildcard)
    {
        contextInfo.setNodeHandleFilter(NodeHandle::Invalid());
        EXPECT_TRUE(contextInfo.isMatchingNodeHandeFilter(NodeHandle(0u)));
        EXPECT_TRUE(contextInfo.isMatchingNodeHandeFilter(NodeHandle(123u)));
        EXPECT_TRUE(contextInfo.isMatchingNodeHandeFilter(NodeHandle::Invalid()));
    }
}
