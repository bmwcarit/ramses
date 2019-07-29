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
    EXPECT_EQ(ramses::StatusOK, config.setWaylandSocketEmbeddedFD(333));
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

TEST(ARendererConfig, canSetEmbeddedCompositingSocketPermissionsGroup)
{
    ramses::RendererConfig config;

    config.setWaylandSocketEmbeddedGroup("permissionGroup");
    EXPECT_STREQ("permissionGroup", config.impl.getWaylandSocketEmbeddedGroup());
}

TEST(ARendererConfig, canSetEmbeddedCompositingSocketname)
{
    ramses::RendererConfig config;

    config.setWaylandSocketEmbedded("wayland-x123");
    EXPECT_STREQ("wayland-x123", config.impl.getWaylandSocketEmbedded());
}

TEST(ARendererConfig, canSetEmbeddedCompositingSocketFD)
{
    ramses::RendererConfig config;

    config.setWaylandSocketEmbeddedFD(23);
    EXPECT_EQ(23, config.impl.getWaylandSocketEmbeddedFD());
}

TEST(ARendererConfig, defaultRendererConfigDoesntValidate)
{
    ramses::RendererConfig config;
    EXPECT_NE(ramses::StatusOK, config.impl.validate(0u));
}

TEST(ARendererConfig, settingEmbeddedCompositingSocketnameValidatesTrue)
{
    ramses::RendererConfig config;
    config.setWaylandSocketEmbedded("wayland-x123");
    EXPECT_EQ(ramses::StatusOK, config.impl.validate(0u));
}

TEST(ARendererConfig, settingEmbeddedCompositingSocketFDValidatesTrue)
{
    ramses::RendererConfig config;
    config.setWaylandSocketEmbeddedFD(23);
    EXPECT_EQ(ramses::StatusOK, config.impl.validate(0u));
}

TEST(ARendererConfig, setsAndGetsWaylandDisplay)
{
    ramses::RendererConfig config;
    EXPECT_EQ(ramses::StatusOK, config.setSystemCompositorWaylandDisplay("xxx"));
    EXPECT_STREQ("xxx", config.getSystemCompositorWaylandDisplay());
    EXPECT_STREQ("xxx", config.impl.getInternalRendererConfig().getWaylandDisplayForSystemCompositorController().c_str());
}
