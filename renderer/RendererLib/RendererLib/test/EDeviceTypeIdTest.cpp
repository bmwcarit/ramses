//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "gtest/gtest.h"
#include "RendererAPI/EDeviceTypeId.h"

using namespace ramses_internal;

TEST(EDeviceTypeIdTest, IsDeviceEnabled)
{
    EXPECT_TRUE(IsRendererDeviceEnabled(EDeviceTypeId_GL_4_2_CORE, EDeviceTypeId_GL_4_2_CORE));
    EXPECT_TRUE(IsRendererDeviceEnabled(EDeviceTypeId_ALL, EDeviceTypeId_GL_4_2_CORE));
    EXPECT_TRUE(IsRendererDeviceEnabled(EDeviceTypeId_GL_ES_3_0 | EDeviceTypeId_GL_4_2_CORE, EDeviceTypeId_GL_4_2_CORE));
    EXPECT_TRUE(IsRendererDeviceEnabled(EDeviceTypeId_GL_ES_3_0 | EDeviceTypeId_GL_4_5, EDeviceTypeId_GL_4_5));
    EXPECT_FALSE(IsRendererDeviceEnabled(EDeviceTypeId_GL_ES_3_0, EDeviceTypeId_GL_4_2_CORE));
    EXPECT_FALSE(IsRendererDeviceEnabled(EDeviceTypeId_GL_4_2_CORE, EDeviceTypeId_GL_ES_3_0));
}

TEST(EDeviceTypeIdTest, AllValueContainsGL4_2_Core)
{
    EXPECT_EQ(EDeviceTypeId_GL_4_2_CORE, EDeviceTypeId_ALL & EDeviceTypeId_GL_4_2_CORE);
}

TEST(EDeviceTypeIdTest, AllValueContainsGL4_5)
{
    EXPECT_EQ(EDeviceTypeId_GL_4_5, EDeviceTypeId_ALL & EDeviceTypeId_GL_4_5);
}

TEST(EDeviceTypeIdTest, AllValueContainsEDeviceTypeId_GL_ES_3_0)
{
    EXPECT_EQ(EDeviceTypeId_GL_ES_3_0, EDeviceTypeId_ALL & EDeviceTypeId_GL_ES_3_0);
}
