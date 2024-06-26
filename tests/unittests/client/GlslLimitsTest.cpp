//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "internal/glslEffectBlock/GLSlang.h"
#include "internal/glslEffectBlock/GlslLimits.h"

using namespace testing;

namespace ramses::internal
{
    class GlslLimitsTest : public ::testing::Test
    {
    };

    TEST_F(GlslLimitsTest, parseGlslVersionFromString)
    {
        EXPECT_EQ(100u, GlslLimits::GetVersionFromString("#version 100"));
        EXPECT_EQ(330u, GlslLimits::GetVersionFromString("#version 330"));
        EXPECT_EQ(0u, GlslLimits::GetVersionFromString("Some invalid string"));
    }

    TEST_F(GlslLimitsTest, testLimitsOpenGL_20)
    {
        TBuiltInResource res{};
        GlslLimits::InitCompilationResources(res, "#version 110");

        EXPECT_EQ(res.maxClipPlanes, 6);
        EXPECT_EQ(res.maxTextureUnits, 2);
        EXPECT_EQ(res.maxAtomicCounterBindings, 0); // Should not be present
    }

    TEST_F(GlslLimitsTest, testLimitsOpenGL_ES_30)
    {
        TBuiltInResource res{};
        GlslLimits::InitCompilationResources(res, "#version 300 es");

        EXPECT_EQ(res.maxFragmentInputVectors, 15);
        EXPECT_EQ(res.maxTessEvaluationUniformComponents, 0); // Should not be present
    }

    TEST_F(GlslLimitsTest, testLimitsOpenGL_42)
    {
        TBuiltInResource res{};
        GlslLimits::InitCompilationResources(res, "#version 420");

        EXPECT_EQ(res.maxAtomicCounterBufferSize, 16384);
        EXPECT_EQ(res.minProgramTexelOffset, -8);
    }
}
