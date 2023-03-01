//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "gtest/gtest.h"
#include "RendererLib/RendererConfig.h"
#include "CLI/CLI.hpp"

TEST(AInternalRendererConfig, hasDefaultValues)
{
    ramses_internal::RendererConfig config;
    EXPECT_FALSE(config.getSystemCompositorControlEnabled());
    EXPECT_EQ(std::chrono::microseconds{10000u}, config.getFrameCallbackMaxPollTime());
    EXPECT_STREQ("", config.getWaylandDisplayForSystemCompositorController().c_str());
}

TEST(AInternalRendererConfig, canEnableSystemCompositorControl)
{
    ramses_internal::RendererConfig config;
    config.enableSystemCompositorControl();
    EXPECT_TRUE(config.getSystemCompositorControlEnabled());
}

TEST(AInternalRendererConfig, canSetGetMaxFramecallbackPollTime)
{
    ramses_internal::RendererConfig config;

    config.setFrameCallbackMaxPollTime(std::chrono::microseconds{123u});
    EXPECT_EQ(std::chrono::microseconds{123u}, config.getFrameCallbackMaxPollTime());
}

TEST(AInternalRendererConfig, canSetGetWaylandDisplayForSystemCompositorController)
{
    ramses_internal::RendererConfig config;

    config.setWaylandDisplayForSystemCompositorController("ramses wd");
    EXPECT_STREQ("ramses wd", config.getWaylandDisplayForSystemCompositorController().c_str());
}

