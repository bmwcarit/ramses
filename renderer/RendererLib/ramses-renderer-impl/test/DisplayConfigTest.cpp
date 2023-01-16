//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "ramses-renderer-api/DisplayConfig.h"
#include "DisplayConfigImpl.h"

class ADisplayConfig : public ::testing::Test
{
protected:
    ramses::DisplayConfig config;
};

TEST_F(ADisplayConfig, hasDefaultValuesUponConstruction)
{
    const ramses_internal::DisplayConfig defaultDisplayConfig;
    const ramses_internal::DisplayConfig& displayConfig = config.impl.getInternalDisplayConfig();

    EXPECT_EQ(defaultDisplayConfig.getWindowPositionX(), displayConfig.getWindowPositionX());
    EXPECT_EQ(defaultDisplayConfig.getWindowPositionY(), displayConfig.getWindowPositionY());
    EXPECT_EQ(defaultDisplayConfig.getDesiredWindowWidth(), displayConfig.getDesiredWindowWidth());
    EXPECT_EQ(defaultDisplayConfig.getDesiredWindowHeight(), displayConfig.getDesiredWindowHeight());

    EXPECT_EQ(defaultDisplayConfig.getFullscreenState(), displayConfig.getFullscreenState());
    EXPECT_EQ(defaultDisplayConfig.getBorderlessState(), displayConfig.getBorderlessState());
    EXPECT_EQ(defaultDisplayConfig.isWarpingEnabled(), displayConfig.isWarpingEnabled());
    EXPECT_EQ(defaultDisplayConfig.getKeepEffectsUploaded(), displayConfig.getKeepEffectsUploaded());

    EXPECT_EQ(defaultDisplayConfig.getAntialiasingSampleCount(), displayConfig.getAntialiasingSampleCount());
    EXPECT_EQ(defaultDisplayConfig.getStartVisibleIvi(), displayConfig.getStartVisibleIvi());

    EXPECT_EQ(defaultDisplayConfig.getGPUMemoryCacheSize(), displayConfig.getGPUMemoryCacheSize());
    EXPECT_EQ(defaultDisplayConfig.getClearColor(), displayConfig.getClearColor());

    EXPECT_TRUE(defaultDisplayConfig.getWaylandDisplay().empty());

    EXPECT_FALSE(defaultDisplayConfig.getWaylandIviSurfaceID().isValid());
    EXPECT_FALSE(defaultDisplayConfig.getWaylandIviLayerID().isValid());

    EXPECT_FALSE(defaultDisplayConfig.getWindowsWindowHandle().isValid());
    EXPECT_FALSE(defaultDisplayConfig.getX11WindowHandle().isValid());

    EXPECT_EQ(defaultDisplayConfig.getWaylandSocketEmbedded(), displayConfig.getWaylandSocketEmbedded());
    EXPECT_EQ(defaultDisplayConfig.getWaylandSocketEmbeddedGroup(), displayConfig.getWaylandSocketEmbeddedGroup());
    EXPECT_EQ(defaultDisplayConfig.getWaylandSocketEmbeddedFD(), displayConfig.getWaylandSocketEmbeddedFD());
    EXPECT_EQ(defaultDisplayConfig.getPlatformRenderNode(), displayConfig.getPlatformRenderNode());
}

TEST_F(ADisplayConfig, setsFullscreenState)
{
    EXPECT_EQ(ramses::StatusOK, config.setWindowFullscreen(true));
    EXPECT_TRUE(config.impl.getInternalDisplayConfig().getFullscreenState());
    EXPECT_TRUE(config.isWindowFullscreen());

    EXPECT_EQ(ramses::StatusOK, config.setWindowFullscreen(false));
    EXPECT_FALSE(config.impl.getInternalDisplayConfig().getFullscreenState());
    EXPECT_FALSE(config.isWindowFullscreen());
}

TEST_F(ADisplayConfig, setsBorderlessState)
{
    EXPECT_EQ(ramses::StatusOK, config.setWindowBorderless(true));
    EXPECT_TRUE(config.impl.getInternalDisplayConfig().getBorderlessState());

    EXPECT_EQ(ramses::StatusOK, config.setWindowBorderless(false));
    EXPECT_FALSE(config.impl.getInternalDisplayConfig().getBorderlessState());
}

TEST_F(ADisplayConfig, setsWindowRect)
{
    EXPECT_EQ(ramses::StatusOK, config.setWindowRectangle(15, 16, 123u, 345u));
    int32_t x;
    int32_t y;
    uint32_t width;
    uint32_t height;
    config.getWindowRectangle(x, y, width, height);
    EXPECT_EQ(15, x);
    EXPECT_EQ(16, y);
    EXPECT_EQ(123u, width);
    EXPECT_EQ(345u, height);
}

TEST_F(ADisplayConfig, failsToSetInvalidWindowRect)
{
    EXPECT_EQ(ramses::StatusOK, config.setWindowRectangle(15, 16, 1u, 2u));

    EXPECT_NE(ramses::StatusOK, config.setWindowRectangle(15, 16, 0u, 2u));
    EXPECT_NE(ramses::StatusOK, config.setWindowRectangle(15, 16, 1u, 0u));
    EXPECT_NE(ramses::StatusOK, config.setWindowRectangle(15, 16, 0u, 0u));

    EXPECT_EQ(15, config.impl.getInternalDisplayConfig().getWindowPositionX());
    EXPECT_EQ(16, config.impl.getInternalDisplayConfig().getWindowPositionY());
    EXPECT_EQ(1u, config.impl.getInternalDisplayConfig().getDesiredWindowWidth());
    EXPECT_EQ(2u, config.impl.getInternalDisplayConfig().getDesiredWindowHeight());
}

TEST_F(ADisplayConfig, failsToSetUnsupportedMultisampling)
{
    EXPECT_NE(ramses::StatusOK, config.setMultiSampling(0u));
    EXPECT_NE(ramses::StatusOK, config.setMultiSampling(3u));
    EXPECT_NE(ramses::StatusOK, config.setMultiSampling(5u));
    EXPECT_NE(ramses::StatusOK, config.setMultiSampling(6u));
    EXPECT_NE(ramses::StatusOK, config.setMultiSampling(7u));
    EXPECT_NE(ramses::StatusOK, config.setMultiSampling(9u));

    EXPECT_EQ(1u, config.impl.getInternalDisplayConfig().getAntialiasingSampleCount());
}

TEST_F(ADisplayConfig, setsAndGetsMultisampling)
{
    EXPECT_EQ(ramses::StatusOK, config.setMultiSampling(2u));

    EXPECT_EQ(2u, config.impl.getInternalDisplayConfig().getAntialiasingSampleCount());

    uint32_t sampleCount = 0;
    config.getMultiSamplingSamples(sampleCount);
    EXPECT_EQ(2u, sampleCount);
}

TEST_F(ADisplayConfig, enablesWarping)
{
    EXPECT_EQ(ramses::StatusOK, config.enableWarpingPostEffect());
    EXPECT_TRUE(config.impl.getInternalDisplayConfig().isWarpingEnabled());
}

TEST_F(ADisplayConfig, disablesKeepingOfEffectsInVRAM)
{
    EXPECT_EQ(ramses::StatusOK, config.keepEffectsUploaded(false));
    EXPECT_FALSE(config.impl.getInternalDisplayConfig().getKeepEffectsUploaded());
}

TEST_F(ADisplayConfig, setsNativeDisplayID)
{
    EXPECT_EQ(ramses::StatusOK, config.setIntegrityRGLDeviceUnit(2u));
    EXPECT_EQ(2u, config.impl.getInternalDisplayConfig().getIntegrityRGLDeviceUnit().getValue());
}

TEST_F(ADisplayConfig, setsWindowIVIVisible)
{
    EXPECT_EQ(ramses::StatusOK, config.setWindowIviVisible());
    EXPECT_TRUE(config.impl.getInternalDisplayConfig().getStartVisibleIvi());
}

TEST_F(ADisplayConfig, setsAndGetsWaylandDisplay)
{
    EXPECT_EQ(ramses::StatusOK, config.setWaylandDisplay("xxx"));
    EXPECT_STREQ("xxx", config.getWaylandDisplay());
    EXPECT_STREQ("xxx", config.impl.getInternalDisplayConfig().getWaylandDisplay().c_str());
}

TEST_F(ADisplayConfig, setsAndGetsWaylandIviSurfaceId)
{
    EXPECT_EQ(ramses::StatusOK, config.setWaylandIviSurfaceID(ramses::waylandIviSurfaceId_t(25)));
    EXPECT_EQ(ramses::waylandIviSurfaceId_t(25), config.getWaylandIviSurfaceID());
    EXPECT_EQ(ramses_internal::WaylandIviSurfaceId(25), config.impl.getInternalDisplayConfig().getWaylandIviSurfaceID());
}

TEST_F(ADisplayConfig, setsAndGetsWaylandIviLayerId)
{
    EXPECT_EQ(ramses::StatusOK, config.setWaylandIviLayerID(ramses::waylandIviLayerId_t(36)));
    EXPECT_EQ(ramses::waylandIviLayerId_t(36), config.getWaylandIviLayerID());
    EXPECT_EQ(ramses_internal::WaylandIviLayerId(36), config.impl.getInternalDisplayConfig().getWaylandIviLayerID());
}

TEST_F(ADisplayConfig, setsAndGetsX11WindowHandle)
{
    EXPECT_EQ(ramses::StatusOK, config.setX11WindowHandle(42u));
    EXPECT_EQ(42u, config.getX11WindowHandle());
    EXPECT_EQ(ramses_internal::X11WindowHandle(42), config.impl.getInternalDisplayConfig().getX11WindowHandle());
}

TEST_F(ADisplayConfig, IsValidUponConstruction)
{
    EXPECT_EQ(ramses::StatusOK, config.validate());
}

TEST_F(ADisplayConfig, canBeCopyConstructed)
{
    config.enableWarpingPostEffect();
    const ramses::DisplayConfig otherConfig(config);
    EXPECT_TRUE(otherConfig.impl.getInternalDisplayConfig().isWarpingEnabled());
}

TEST_F(ADisplayConfig, setClearColor)
{
    const float red = 0.1f;
    const float green = 0.2f;
    const float blue = 0.3f;
    const float alpha = 0.4f;
    EXPECT_EQ(ramses::StatusOK, config.setClearColor(red, green, blue, alpha));

    const ramses_internal::Vector4& clearColor = config.impl.getInternalDisplayConfig().getClearColor();
    EXPECT_EQ(clearColor.r, red);
    EXPECT_EQ(clearColor.g, green);
    EXPECT_EQ(clearColor.b, blue);
    EXPECT_EQ(clearColor.a, alpha);
}

TEST_F(ADisplayConfig, setDepthStencilBufferType)
{
    EXPECT_EQ(ramses::StatusOK, ramses::DisplayConfig::setDepthStencilBufferType(config, ramses::EDepthBufferType_Depth));
    EXPECT_EQ(ramses_internal::ERenderBufferType_DepthBuffer, config.impl.getInternalDisplayConfig().getDepthStencilBufferType());
}

TEST_F(ADisplayConfig, setAsyncEffectUploadEnabled)
{
    EXPECT_EQ(ramses::StatusOK, ramses::DisplayConfig::setAsyncEffectUploadEnabled(config, false));
    EXPECT_FALSE(config.impl.getInternalDisplayConfig().isAsyncEffectUploadEnabled());
}

TEST_F(ADisplayConfig, canSetEmbeddedCompositingSocketGroup)
{
    config.setWaylandEmbeddedCompositingSocketGroup("permissionGroup");
    EXPECT_STREQ("permissionGroup", config.impl.getWaylandSocketEmbeddedGroup());
}

TEST_F(ADisplayConfig, canSetEmbeddedCompositingSocketPermissions)
{
    config.setWaylandEmbeddedCompositingSocketPermissions(0660);
    EXPECT_EQ(0660u, config.impl.getWaylandSocketEmbeddedPermissions());
}

TEST_F(ADisplayConfig, cannotSetInvalidEmbeddedCompositingSocketPermissions)
{
    EXPECT_NE(ramses::StatusOK, config.setWaylandEmbeddedCompositingSocketPermissions(0));
}

TEST_F(ADisplayConfig, canSetEmbeddedCompositingSocketname)
{
    config.setWaylandEmbeddedCompositingSocketName("wayland-x123");
    EXPECT_STREQ("wayland-x123", config.getWaylandEmbeddedCompositingSocketName());
}

TEST_F(ADisplayConfig, canSetEmbeddedCompositingSocketFD)
{
    config.setWaylandEmbeddedCompositingSocketFD(23);
    EXPECT_EQ(23, config.impl.getWaylandSocketEmbeddedFD());
}

TEST_F(ADisplayConfig, canSetPlatformRenderNode)
{
    config.setPlatformRenderNode("abcd");
    EXPECT_STREQ("abcd", config.impl.getPlatformRenderNode());
}

TEST_F(ADisplayConfig, canSetSwapInterval)
{
    EXPECT_EQ(-1, config.impl.getSwapInterval());
    EXPECT_EQ(ramses::StatusOK, config.setSwapInterval(0));
    EXPECT_EQ(0, config.impl.getSwapInterval());
}

TEST_F(ADisplayConfig, canSetScenePriority)
{
    EXPECT_EQ(0, config.impl.getScenePriority(ramses::sceneId_t(551)));
    EXPECT_EQ(ramses::StatusOK, config.setScenePriority(ramses::sceneId_t(551), 4));
    EXPECT_EQ(4, config.impl.getScenePriority(ramses::sceneId_t(551)));
}

TEST_F(ADisplayConfig, canSetResourceUploadBatchSize)
{
    EXPECT_EQ(10u, config.impl.getResourceUploadBatchSize());
    EXPECT_EQ(ramses::StatusOK, config.setResourceUploadBatchSize(1));
    EXPECT_EQ(1u, config.impl.getResourceUploadBatchSize());
    EXPECT_NE(ramses::StatusOK, config.setResourceUploadBatchSize(0));
    EXPECT_EQ(1u, config.impl.getResourceUploadBatchSize());
}
