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
    ADisplayConfig()
    {
    }

    ramses::DisplayConfig config;
};

TEST_F(ADisplayConfig, hasDefaultValuesUponConstruction)
{
    const ramses_internal::DisplayConfig defaultDisplayConfig;
    const ramses_internal::DisplayConfig& displayConfig = config.impl.getInternalDisplayConfig();

    const ramses_internal::Vector3& defaultCamPos = defaultDisplayConfig.getCameraPosition();
    float camPosX = 0.f;
    float camPosY = 0.f;
    float camPosZ = 0.f;
    EXPECT_EQ(ramses::StatusOK, config.getViewPosition(camPosX, camPosY, camPosZ));
    EXPECT_FLOAT_EQ(defaultCamPos.x, camPosX);
    EXPECT_FLOAT_EQ(defaultCamPos.y, camPosY);
    EXPECT_FLOAT_EQ(defaultCamPos.z, camPosZ);

    const ramses_internal::Vector3& camPos = displayConfig.getCameraPosition();
    EXPECT_FLOAT_EQ(defaultCamPos.x, camPos.x);
    EXPECT_FLOAT_EQ(defaultCamPos.y, camPos.y);
    EXPECT_FLOAT_EQ(defaultCamPos.z, camPos.z);

    const ramses_internal::Vector3& defaultCamRot = defaultDisplayConfig.getCameraRotation();
    const ramses_internal::Vector3& camRot = displayConfig.getCameraRotation();
    EXPECT_FLOAT_EQ(defaultCamRot.x, camRot.x);
    EXPECT_FLOAT_EQ(defaultCamRot.y, camRot.y);
    EXPECT_FLOAT_EQ(defaultCamRot.z, camRot.z);

    float camRotX = 0.f;
    float camRotY = 0.f;
    float camRotZ = 0.f;
    EXPECT_EQ(ramses::StatusOK, config.getViewRotation(camRotX, camRotY, camRotZ));
    EXPECT_FLOAT_EQ(defaultCamPos.x, camRotX);
    EXPECT_FLOAT_EQ(defaultCamPos.y, camRotY);
    EXPECT_FLOAT_EQ(defaultCamPos.z, camRotZ);

    EXPECT_FLOAT_EQ(defaultDisplayConfig.getProjectionParams().leftPlane,   displayConfig.getProjectionParams().leftPlane);
    EXPECT_FLOAT_EQ(defaultDisplayConfig.getProjectionParams().rightPlane,  displayConfig.getProjectionParams().rightPlane);
    EXPECT_FLOAT_EQ(defaultDisplayConfig.getProjectionParams().bottomPlane, displayConfig.getProjectionParams().bottomPlane);
    EXPECT_FLOAT_EQ(defaultDisplayConfig.getProjectionParams().topPlane,    displayConfig.getProjectionParams().topPlane);
    EXPECT_FLOAT_EQ(defaultDisplayConfig.getProjectionParams().nearPlane,   displayConfig.getProjectionParams().nearPlane);
    EXPECT_FLOAT_EQ(defaultDisplayConfig.getProjectionParams().farPlane,    displayConfig.getProjectionParams().farPlane);

    EXPECT_EQ(defaultDisplayConfig.getWindowPositionX(), displayConfig.getWindowPositionX());
    EXPECT_EQ(defaultDisplayConfig.getWindowPositionY(), displayConfig.getWindowPositionY());
    EXPECT_EQ(defaultDisplayConfig.getDesiredWindowWidth(), displayConfig.getDesiredWindowWidth());
    EXPECT_EQ(defaultDisplayConfig.getDesiredWindowHeight(), displayConfig.getDesiredWindowHeight());

    EXPECT_EQ(defaultDisplayConfig.getFullscreenState(), displayConfig.getFullscreenState());
    EXPECT_EQ(defaultDisplayConfig.getBorderlessState(), displayConfig.getBorderlessState());
    EXPECT_EQ(defaultDisplayConfig.isWarpingEnabled(), displayConfig.isWarpingEnabled());
    EXPECT_EQ(defaultDisplayConfig.getKeepEffectsUploaded(), displayConfig.getKeepEffectsUploaded());
    EXPECT_EQ(defaultDisplayConfig.isStereoDisplay(), displayConfig.isStereoDisplay());

    EXPECT_EQ(defaultDisplayConfig.getAntialiasingMethod(), displayConfig.getAntialiasingMethod());
    EXPECT_EQ(defaultDisplayConfig.getAntialiasingSampleCount(), displayConfig.getAntialiasingSampleCount());
    EXPECT_EQ(defaultDisplayConfig.getStartVisibleIvi(), displayConfig.getStartVisibleIvi());

    EXPECT_EQ(defaultDisplayConfig.getGPUMemoryCacheSize(), displayConfig.getGPUMemoryCacheSize());
    EXPECT_EQ(defaultDisplayConfig.getClearColor(), displayConfig.getClearColor());

    EXPECT_TRUE(defaultDisplayConfig.getWaylandDisplay().empty());
}

TEST_F(ADisplayConfig, setsFullscreenState)
{
    EXPECT_EQ(ramses::StatusOK, config.setWindowFullscreen(true));
    EXPECT_TRUE(config.impl.getInternalDisplayConfig().getFullscreenState());

    EXPECT_EQ(ramses::StatusOK, config.setWindowFullscreen(false));
    EXPECT_FALSE(config.impl.getInternalDisplayConfig().getFullscreenState());
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
    EXPECT_EQ(15, config.impl.getInternalDisplayConfig().getWindowPositionX());
    EXPECT_EQ(16, config.impl.getInternalDisplayConfig().getWindowPositionY());
    EXPECT_EQ(123u, config.impl.getInternalDisplayConfig().getDesiredWindowWidth());
    EXPECT_EQ(345u, config.impl.getInternalDisplayConfig().getDesiredWindowHeight());
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

TEST_F(ADisplayConfig, setsAndGetsViewPosition)
{
    EXPECT_EQ(ramses::StatusOK, config.setViewPosition(1, 2, 3));
    EXPECT_EQ(ramses_internal::Vector3(1, 2, 3), config.impl.getInternalDisplayConfig().getCameraPosition());

    float camPosX = 0.f;
    float camPosY = 0.f;
    float camPosZ = 0.f;
    EXPECT_EQ(ramses::StatusOK, config.getViewPosition(camPosX, camPosY, camPosZ));
    EXPECT_FLOAT_EQ(1, camPosX);
    EXPECT_FLOAT_EQ(2, camPosY);
    EXPECT_FLOAT_EQ(3, camPosZ);
}

TEST_F(ADisplayConfig, setsAndGetsViewRotation)
{
    EXPECT_EQ(ramses::StatusOK, config.setViewRotation(1, 2, 3));
    EXPECT_EQ(ramses_internal::Vector3(1, 2, 3), config.impl.getInternalDisplayConfig().getCameraRotation());

    float rotX = 1.f;
    float rotY = 2.f;
    float rotZ = 3.f;
    EXPECT_EQ(ramses::StatusOK, config.getViewRotation(rotX, rotY, rotZ));
    EXPECT_FLOAT_EQ(1, rotX);
    EXPECT_FLOAT_EQ(2, rotY);
    EXPECT_FLOAT_EQ(3, rotZ);
}

TEST_F(ADisplayConfig, failsToSetUnsupportedMultisampling)
{
    EXPECT_NE(ramses::StatusOK, config.setMultiSampling(0u));
    EXPECT_NE(ramses::StatusOK, config.setMultiSampling(3u));
    EXPECT_NE(ramses::StatusOK, config.setMultiSampling(5u));

    EXPECT_EQ(ramses_internal::EAntiAliasingMethod_PlainFramebuffer, config.impl.getInternalDisplayConfig().getAntialiasingMethod());
    EXPECT_EQ(1u, config.impl.getInternalDisplayConfig().getAntialiasingSampleCount());
}

TEST_F(ADisplayConfig, setsAndGetsMultisampling)
{
    EXPECT_EQ(ramses::StatusOK, config.setMultiSampling(2u));

    EXPECT_EQ(ramses_internal::EAntiAliasingMethod_MultiSampling, config.impl.getInternalDisplayConfig().getAntialiasingMethod());
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

TEST_F(ADisplayConfig, enablesStereoDisplay)
{
    EXPECT_EQ(ramses::StatusOK, config.enableStereoDisplay());
    EXPECT_TRUE(config.impl.getInternalDisplayConfig().isStereoDisplay());
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

TEST_F(ADisplayConfig, setsPerspectiveProjection)
{
    const ramses_internal::ProjectionParams projParams = ramses_internal::ProjectionParams::Perspective(30.f, 1.f, 1.f, 10.f);
    EXPECT_EQ(ramses::StatusOK, config.setPerspectiveProjection(30.f, 1.f, 1.f, 10.f));
    EXPECT_EQ(projParams, config.impl.getInternalDisplayConfig().getProjectionParams());
}

TEST_F(ADisplayConfig, setsAlternativePerspectiveProjection)
{
    const ramses_internal::ProjectionParams projParams = ramses_internal::ProjectionParams::Frustum(ramses_internal::ECameraProjectionType_Perspective,
        0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f);
    EXPECT_EQ(ramses::StatusOK, config.setPerspectiveProjection(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f));
    EXPECT_EQ(projParams, config.impl.getInternalDisplayConfig().getProjectionParams());
}

TEST_F(ADisplayConfig, doesNotChangeAspectRatioIfViewportIsSet)
{
    config.setWindowRectangle(0, 0, 400u, 200u);

    const ramses_internal::ProjectionParams projParams = ramses_internal::ProjectionParams::Perspective(30.f, 1.f, 1.f, 10.f);
    EXPECT_EQ(ramses::StatusOK, config.setPerspectiveProjection(30.f, 1.f, 1.f, 10.f));
    EXPECT_EQ(projParams, config.impl.getInternalDisplayConfig().getProjectionParams());

    config.setWindowRectangle(0, 0, 200u, 400u);
    EXPECT_EQ(projParams, config.impl.getInternalDisplayConfig().getProjectionParams());
}

TEST_F(ADisplayConfig, setsOrthographicProjection)
{
    const ramses_internal::ProjectionParams projParams = ramses_internal::ProjectionParams::Frustum(ramses_internal::ECameraProjectionType_Orthographic,
        30.f, 40.f, 10.f, 100.f, 0.1f, 1.f);
    EXPECT_EQ(ramses::StatusOK, config.setOrthographicProjection(30.f, 40.f, 10.f, 100.f, 0.1f, 1.f));
    EXPECT_EQ(projParams, config.impl.getInternalDisplayConfig().getProjectionParams());
}

TEST_F(ADisplayConfig, IsValidUponConstruction)
{
    EXPECT_EQ(ramses::StatusOK, config.validate());
}

TEST_F(ADisplayConfig, doesNearFarPlaneValidationOnSetting)
{
    EXPECT_NE(ramses::StatusOK, config.setPerspectiveProjection(30.f, 1.f, 0.0f, 1.f));
    EXPECT_NE(ramses::StatusOK, config.validate());

    EXPECT_NE(ramses::StatusOK, config.setPerspectiveProjection(30.f, 1.f, 10.f, 10.f));
    EXPECT_NE(ramses::StatusOK, config.validate());

    EXPECT_NE(ramses::StatusOK, config.setPerspectiveProjection(30.f, 1.f, 10.f, 1.f));
    EXPECT_NE(ramses::StatusOK, config.validate());

    EXPECT_EQ(ramses::StatusOK, config.setPerspectiveProjection(30.f, 1.f, 1.f, 10.f));
    EXPECT_EQ(ramses::StatusOK, config.validate());
}

TEST_F(ADisplayConfig, doesFovYAndAspectRatioValidationOnSetting)
{
    EXPECT_NE(ramses::StatusOK, config.setPerspectiveProjection(0.f, 1.f, 1.f, 10.f));
    EXPECT_NE(ramses::StatusOK, config.validate());

    EXPECT_NE(ramses::StatusOK, config.setPerspectiveProjection(180.f, 1.0f, 1.f, 10.f));
    EXPECT_NE(ramses::StatusOK, config.validate());

    EXPECT_NE(ramses::StatusOK, config.setPerspectiveProjection(-45.f, 1.f, 1.f, 10.f));
    EXPECT_NE(ramses::StatusOK, config.validate());

    EXPECT_NE(ramses::StatusOK, config.setPerspectiveProjection(-195.f, 1.f, 1.f, 10.f));
    EXPECT_NE(ramses::StatusOK, config.validate());

    EXPECT_NE(ramses::StatusOK, config.setPerspectiveProjection(30.f, 0.f, 1.f, 10.f));
    EXPECT_NE(ramses::StatusOK, config.validate());

    EXPECT_NE(ramses::StatusOK, config.setPerspectiveProjection(30.f, -0.5f, 1.f, 10.f));
    EXPECT_NE(ramses::StatusOK, config.validate());

    EXPECT_EQ(ramses::StatusOK, config.setPerspectiveProjection(30.f, 1.f, 1.f, 10.f));
    EXPECT_EQ(ramses::StatusOK, config.validate());
}

TEST_F(ADisplayConfig, doesOrthoProjectionParamsValidationOnSetting)
{
    EXPECT_NE(ramses::StatusOK, config.setOrthographicProjection(40.f, 40.f, 10.f, 100.f, 0.1f, 1.f));
    EXPECT_NE(ramses::StatusOK, config.validate());

    EXPECT_NE(ramses::StatusOK, config.setOrthographicProjection(10.f, 40.f, 100.f, 100.f, 0.1f, 1.f));
    EXPECT_NE(ramses::StatusOK, config.validate());

    EXPECT_NE(ramses::StatusOK, config.setOrthographicProjection(10.f, 40.f, 100.f, 10.f, 0.1f, 1.f));
    EXPECT_NE(ramses::StatusOK, config.validate());

    EXPECT_EQ(ramses::StatusOK, config.setOrthographicProjection(10.f, 40.f, 10.f, 100.f, 0.1f, 1.f));
    EXPECT_EQ(ramses::StatusOK, config.validate());
}

TEST_F(ADisplayConfig, doesPerspectiveProjectionParamsValidationOnSetting)
{
    EXPECT_NE(ramses::StatusOK, config.setPerspectiveProjection(40.f, 40.f, 10.f, 100.f, 0.1f, 1.f));
    EXPECT_NE(ramses::StatusOK, config.validate());

    EXPECT_NE(ramses::StatusOK, config.setPerspectiveProjection(10.f, 40.f, 100.f, 100.f, 0.1f, 1.f));
    EXPECT_NE(ramses::StatusOK, config.validate());

    EXPECT_NE(ramses::StatusOK, config.setPerspectiveProjection(10.f, 40.f, 100.f, 10.f, 0.1f, 1.f));
    EXPECT_NE(ramses::StatusOK, config.validate());

    EXPECT_EQ(ramses::StatusOK, config.setPerspectiveProjection(10.f, 40.f, 10.f, 100.f, 0.1f, 1.f));
    EXPECT_EQ(ramses::StatusOK, config.validate());
}

TEST_F(ADisplayConfig, validatesStereoDisplay)
{
    EXPECT_EQ(ramses::StatusOK, config.enableStereoDisplay());
    EXPECT_EQ(ramses::StatusOK, config.validate());
}

TEST_F(ADisplayConfig, failsValidationOfStereoDisplayWithWarping)
{
    EXPECT_EQ(ramses::StatusOK, config.enableStereoDisplay());
    EXPECT_EQ(ramses::StatusOK, config.enableWarpingPostEffect());
    EXPECT_NE(ramses::StatusOK, config.validate());
}

TEST_F(ADisplayConfig, failsValidationOfStereoDisplayWithAntialiasing)
{
    EXPECT_EQ(ramses::StatusOK, config.enableStereoDisplay());
    EXPECT_EQ(ramses::StatusOK, config.setMultiSampling(4u));
    EXPECT_NE(ramses::StatusOK, config.validate());
}

TEST_F(ADisplayConfig, failsValidationIfCLIParamsForPerspectiveCameraAreInvalid)
{
    const char *args[] = { "renderer", "-fov", "-45.4" };
    ramses::DisplayConfig cliConfig(3u, args);

    EXPECT_NE(ramses::StatusOK, cliConfig.validate());
}

TEST_F(ADisplayConfig, failsValidationIfNoCLIParamsForOrthoCameraAreGiven)
{
    const char *args[] = { "renderer", "-ortho" };
    ramses::DisplayConfig cliConfig(2u, args);

    EXPECT_NE(ramses::StatusOK, cliConfig.validate());
}

TEST_F(ADisplayConfig, failsValidationIfCLIParamsForOrthoCameraAreInvalid)
{
    const char *args[] = { "renderer", "-ortho", "-leftPlane", "2", "-rightPlane", "1", "-bottomPlane", "-1", "-topPlane", "1", "-np", "0.1", "-fp", "1990" };
    ramses::DisplayConfig cliConfig(14u, args);

    EXPECT_NE(ramses::StatusOK, cliConfig.validate());
}

TEST_F(ADisplayConfig, passValidationIfCLIParamsForPerspectiveCameraAreValid)
{
    const char *args[] = { "renderer", "-fov", "45.4" };
    ramses::DisplayConfig cliConfig(3u, args);

    EXPECT_EQ(ramses::StatusOK, cliConfig.validate());
}

TEST_F(ADisplayConfig, passValidationIfCLIParamsForOrthoCameraAreValid)
{
    const char *args[] = { "renderer", "-ortho", "-leftPlane", "-1", "-rightPlane", "1", "-bottomPlane", "-1", "-topPlane", "1", "-np", "0.1", "-fp", "1990" };
    ramses::DisplayConfig cliConfig(14u, args);

    EXPECT_EQ(ramses::StatusOK, cliConfig.validate());
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

TEST_F(ADisplayConfig, setOffscreen)
{
    const bool newOffscreenFlag = !config.impl.getInternalDisplayConfig().getOffscreen();

    EXPECT_EQ(ramses::StatusOK, config.setOffscreen(newOffscreenFlag));
    EXPECT_EQ(newOffscreenFlag, config.impl.getInternalDisplayConfig().getOffscreen());
}
