//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "internal/RendererLib/RendererConfigData.h"

namespace ramses::internal
{
    TEST(AInternalRendererConfig, hasDefaultValues)
    {
        ramses::internal::RendererConfigData config;
        EXPECT_FALSE(config.getSystemCompositorControlEnabled());
        EXPECT_EQ(std::chrono::microseconds{10000u}, config.getFrameCallbackMaxPollTime());
        EXPECT_EQ("", config.getWaylandDisplayForSystemCompositorController());
    }

    TEST(AInternalRendererConfig, canEnableSystemCompositorControl)
    {
        ramses::internal::RendererConfigData config;
        config.enableSystemCompositorControl();
        EXPECT_TRUE(config.getSystemCompositorControlEnabled());
    }

    TEST(AInternalRendererConfig, canSetGetMaxFramecallbackPollTime)
    {
        ramses::internal::RendererConfigData config;

        config.setFrameCallbackMaxPollTime(std::chrono::microseconds{123u});
        EXPECT_EQ(std::chrono::microseconds{123u}, config.getFrameCallbackMaxPollTime());
    }

    TEST(AInternalRendererConfig, canSetGetWaylandDisplayForSystemCompositorController)
    {
        ramses::internal::RendererConfigData config;

        config.setWaylandDisplayForSystemCompositorController("ramses wd");
        EXPECT_EQ("ramses wd", config.getWaylandDisplayForSystemCompositorController());
    }
}
