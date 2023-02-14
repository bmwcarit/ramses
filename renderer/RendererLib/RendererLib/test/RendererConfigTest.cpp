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
    EXPECT_EQ(ramses_internal::String(""), config.getWaylandSocketEmbedded());
    EXPECT_EQ(ramses_internal::String(""), config.getWaylandSocketEmbeddedGroup());
    EXPECT_EQ(-1, config.getWaylandSocketEmbeddedFD());
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

TEST(AInternalRendererConfig, canGetSetWaylandSocketEmbedded)
{
    ramses_internal::RendererConfig config;
    config.setWaylandEmbeddedCompositingSocketName("wayland-11");
    EXPECT_EQ(ramses_internal::String("wayland-11"), config.getWaylandSocketEmbedded());
}

TEST(AInternalRendererConfig, canGetSetWaylandSocketEmbeddedGroupName)
{
    ramses_internal::RendererConfig config;
    config.setWaylandEmbeddedCompositingSocketGroup("groupname1");
    EXPECT_EQ(ramses_internal::String("groupname1"), config.getWaylandSocketEmbeddedGroup());

    config.setWaylandEmbeddedCompositingSocketGroup("group2");
    EXPECT_EQ(ramses_internal::String("group2"), config.getWaylandSocketEmbeddedGroup());
}

TEST(AInternalRendererConfig, canGetSetWaylandSocketEmbeddedPermissions)
{
    ramses_internal::RendererConfig config;
    config.setWaylandEmbeddedCompositingSocketPermissions(0654);
    EXPECT_EQ(0654u, config.getWaylandSocketEmbeddedPermissions());
}

TEST(AInternalRendererConfig, canGetSetWaylandSocketEmbeddedFD)
{
    ramses_internal::RendererConfig config;
    config.setWaylandEmbeddedCompositingSocketFD(42);
    EXPECT_EQ(42, config.getWaylandSocketEmbeddedFD());
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

