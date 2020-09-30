//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "ramses-renderer-api/BinaryShaderCache.h"
#include "ramses-renderer-api/RendererConfig.h"
#include "RendererConfigImpl.h"

TEST(ARendererConfig, hasDefaultValuesUponConstruction)
{
    const ramses_internal::RendererConfig defaultConfig;
    ramses::RendererConfig config;
    EXPECT_EQ(nullptr, config.impl.getBinaryShaderCache());

    const ramses_internal::RendererConfig& internalConfig = config.impl.getInternalRendererConfig();

    EXPECT_EQ(defaultConfig.getWaylandSocketEmbedded(), internalConfig.getWaylandSocketEmbedded());
    EXPECT_EQ(defaultConfig.getWaylandSocketEmbeddedGroup(), internalConfig.getWaylandSocketEmbeddedGroup());
    EXPECT_EQ(defaultConfig.getWaylandSocketEmbeddedFD(), internalConfig.getWaylandSocketEmbeddedFD());
}

TEST(ARendererConfig, canEnableSystemCompositor)
{
    ramses::RendererConfig config;
    EXPECT_EQ(ramses::StatusOK, config.enableSystemCompositorControl());
    EXPECT_TRUE(config.impl.getInternalRendererConfig().getSystemCompositorControlEnabled());
}

TEST(ARendererConfig, canBeCopyConstructed)
{
    ramses::RendererConfig config;
    EXPECT_EQ(ramses::StatusOK, config.setWaylandEmbeddedCompositingSocketFD(333));
    ramses::RendererConfig configOther(config);
    EXPECT_EQ(333, configOther.impl.getInternalRendererConfig().getWaylandSocketEmbeddedFD());
}

TEST(ARendererConfig, canSetBinaryShaderCache)
{
    ramses::RendererConfig config;

    ramses::BinaryShaderCache cache;
    config.setBinaryShaderCache(cache);
    EXPECT_EQ(&cache, config.impl.getBinaryShaderCache());
}

TEST(ARendererConfig, canSetEmbeddedCompositingSocketGroup)
{
    ramses::RendererConfig config;

    config.setWaylandEmbeddedCompositingSocketGroup("permissionGroup");
    EXPECT_STREQ("permissionGroup", config.impl.getWaylandSocketEmbeddedGroup());
}

TEST(ARendererConfig, canSetEmbeddedCompositingSocketPermissions)
{
    ramses::RendererConfig config;
    config.setWaylandEmbeddedCompositingSocketPermissions(0660);
    EXPECT_EQ(0660u, config.impl.getWaylandSocketEmbeddedPermissions());
}

TEST(ARendererConfig, cannotSetInvalidEmbeddedCompositingSocketPermissions)
{
    ramses::RendererConfig config;
    EXPECT_NE(ramses::StatusOK, config.setWaylandEmbeddedCompositingSocketPermissions(0));
}

TEST(ARendererConfig, canSetEmbeddedCompositingSocketname)
{
    ramses::RendererConfig config;

    config.setWaylandEmbeddedCompositingSocketName("wayland-x123");
    EXPECT_STREQ("wayland-x123", config.getWaylandEmbeddedCompositingSocketName());
}

TEST(ARendererConfig, canSetEmbeddedCompositingSocketFD)
{
    ramses::RendererConfig config;

    config.setWaylandEmbeddedCompositingSocketFD(23);
    EXPECT_EQ(23, config.impl.getWaylandSocketEmbeddedFD());
}

TEST(ARendererConfig, defaultRendererConfigDoesntValidate)
{
    ramses::RendererConfig config;
    EXPECT_NE(ramses::StatusOK, config.validate());
}

TEST(ARendererConfig, settingEmbeddedCompositingSocketnameValidatesTrue)
{
    ramses::RendererConfig config;
    config.setWaylandEmbeddedCompositingSocketName("wayland-x123");
    EXPECT_EQ(ramses::StatusOK, config.validate());
}

TEST(ARendererConfig, settingEmbeddedCompositingSocketFDValidatesTrue)
{
    ramses::RendererConfig config;
    config.setWaylandEmbeddedCompositingSocketFD(23);
    EXPECT_EQ(ramses::StatusOK, config.validate());
}

TEST(ARendererConfig, setsAndGetsWaylandDisplay)
{
    ramses::RendererConfig config;
    EXPECT_EQ(ramses::StatusOK, config.setSystemCompositorWaylandDisplay("xxx"));
    EXPECT_STREQ("xxx", config.getSystemCompositorWaylandDisplay());
    EXPECT_STREQ("xxx", config.impl.getInternalRendererConfig().getWaylandDisplayForSystemCompositorController().c_str());
}

TEST(ARendererConfig, setsAndGetsLoopCountPeriod)
{
    ramses::RendererConfig config;
    EXPECT_EQ(ramses::StatusOK, config.setRenderThreadLoopTimingReportingPeriod(std::chrono::milliseconds(1234)));
    EXPECT_EQ(std::chrono::milliseconds(1234), config.getRenderThreadLoopTimingReportingPeriod());
}
