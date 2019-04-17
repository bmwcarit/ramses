//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "gtest/gtest.h"
#include "RendererLib/DisplayConfig.h"
#include "RendererLib/RendererConfigUtils.h"
#include "Utils/CommandLineParser.h"


class AInternalDisplayConfig : public ::testing::Test
{
public:
    ramses_internal::DisplayConfig m_config;
};

TEST_F(AInternalDisplayConfig, hasDefaultValues)
{
    EXPECT_FALSE(m_config.getFullscreenState());
    EXPECT_FALSE(m_config.getBorderlessState());
    EXPECT_EQ(ramses_internal::EAntiAliasingMethod_PlainFramebuffer, m_config.getAntialiasingMethod());
    EXPECT_EQ(1u, m_config.getAntialiasingSampleCount());
    EXPECT_EQ(1280u, m_config.getDesiredWindowWidth());
    EXPECT_EQ(480u, m_config.getDesiredWindowHeight());
    EXPECT_EQ(150, m_config.getWindowPositionX());
    EXPECT_EQ(150, m_config.getWindowPositionY());
    EXPECT_EQ(ramses_internal::Vector3(0.0f), m_config.getCameraRotation());
    EXPECT_EQ(ramses_internal::Vector3(0.0f), m_config.getCameraPosition());
    EXPECT_FALSE(m_config.isWarpingEnabled());
    EXPECT_TRUE(m_config.getKeepEffectsUploaded());
    EXPECT_FALSE(m_config.isStereoDisplay());
    EXPECT_TRUE(ramses_internal::InvalidWaylandIviLayerId == m_config.getWaylandIviLayerID());
    EXPECT_TRUE(ramses_internal::InvalidIntegrityRGLDeviceUnit == m_config.getIntegrityRGLDeviceUnit());
    EXPECT_FALSE(m_config.getStartVisibleIvi());
    EXPECT_FALSE(m_config.isResizable());
    EXPECT_EQ(ramses_internal::ProjectionParams::Perspective(19.0f, 1280.f / 480.f, 0.1f, 1500.f), m_config.getProjectionParams());
    EXPECT_EQ(0u, m_config.getGPUMemoryCacheSize());
    EXPECT_EQ(ramses_internal::Vector4(0.f,0.f,0.f,1.f), m_config.getClearColor());
    EXPECT_FALSE(m_config.getOffscreen());
    EXPECT_STREQ("", m_config.getWaylandDisplay().c_str());

    // this value is used in HL API, so test that value does not change unnoticed
    EXPECT_TRUE(ramses_internal::InvalidIntegrityRGLDeviceUnit.getValue() == 0xFFFFFFFF);
}

TEST_F(AInternalDisplayConfig, setAndGetValues)
{
    m_config.setFullscreenState(true);
    EXPECT_TRUE(m_config.getFullscreenState());

    m_config.setBorderlessState(true);
    EXPECT_TRUE(m_config.getBorderlessState());

    m_config.setAntialiasingMethod(ramses_internal::EAntiAliasingMethod_MultiSampling);
    EXPECT_EQ(ramses_internal::EAntiAliasingMethod_MultiSampling, m_config.getAntialiasingMethod());

    m_config.setAntialiasingSampleCount(2u);
    EXPECT_EQ(2u, m_config.getAntialiasingSampleCount());

    m_config.setDesiredWindowWidth(100u);
    EXPECT_EQ(100u, m_config.getDesiredWindowWidth());

    m_config.setDesiredWindowHeight(200u);
    EXPECT_EQ(200u, m_config.getDesiredWindowHeight());

    m_config.setWindowPositionX(-10);
    EXPECT_EQ(-10, m_config.getWindowPositionX());

    m_config.setWindowPositionY(10);
    EXPECT_EQ(10, m_config.getWindowPositionY());

    m_config.setCameraPosition(ramses_internal::Vector3(1.f));
    EXPECT_EQ(ramses_internal::Vector3(1.f), m_config.getCameraPosition());

    m_config.setCameraRotation(ramses_internal::Vector3(2.f));
    EXPECT_EQ(ramses_internal::Vector3(2.f), m_config.getCameraRotation());

    m_config.setWaylandIviLayerID(ramses_internal::WaylandIviLayerId(102u));
    EXPECT_EQ(102u, m_config.getWaylandIviLayerID().getValue());

    m_config.setIntegrityRGLDeviceUnit(ramses_internal::IntegrityRGLDeviceUnit(33u));
    EXPECT_EQ(33u, m_config.getIntegrityRGLDeviceUnit().getValue());

    m_config.setStartVisibleIvi(true);
    EXPECT_TRUE(m_config.getStartVisibleIvi());

    m_config.setWarpingEnabled(true);
    EXPECT_TRUE(m_config.isWarpingEnabled());

    m_config.setKeepEffectsUploaded(false);
    EXPECT_FALSE(m_config.getKeepEffectsUploaded());

    m_config.setStereoDisplay(true);
    EXPECT_TRUE(m_config.isStereoDisplay());

    m_config.setGPUMemoryCacheSize(256u);
    EXPECT_EQ(256u, m_config.getGPUMemoryCacheSize());

    m_config.setResizable(false);
    EXPECT_FALSE(m_config.isResizable());

    ramses_internal::ProjectionParams projParams = ramses_internal::ProjectionParams::Frustum(ramses_internal::ECameraProjectionType_Orthographic,
        0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f);
    m_config.setProjectionParams(projParams);
    EXPECT_EQ(projParams, m_config.getProjectionParams());

    projParams = ramses_internal::ProjectionParams::Perspective(0.1f, 0.2f, 0.3f, 0.4f);
    m_config.setProjectionParams(projParams);
    EXPECT_EQ(projParams, m_config.getProjectionParams());

    const ramses_internal::Vector4 clearColor(0.1f, 0.2f, 0.3f, 0.4f);
    m_config.setClearColor(clearColor);
    EXPECT_EQ(clearColor, m_config.getClearColor());

    m_config.setOffscreen(true);
    EXPECT_TRUE(m_config.getOffscreen());

    m_config.setWaylandDisplay("ramses display");
    EXPECT_STREQ("ramses display", m_config.getWaylandDisplay().c_str());
}

TEST_F(AInternalDisplayConfig, getsValuesAssignedFromCommandLine)
{
    static const ramses_internal::Char* args[] =
    {
        "app",
        "-cpx", "1",
        "-cpy", "2",
        "-cpz", "3",
        "-crx", "4",
        "-cry", "5",
        "-crz", "6",
        "-fov", "7",
        "-np", "8",
        "-fp", "9",
        "-x", "10",
        "-y", "11",
        "-w", "12",
        "-h", "13",
        "-f",
        "-bl",
        "-warp",
        "-de",
        "-aa", "MSAA",
        "-as", "4",
        "-lid", "101",
        "-sid", "1001",
        "-rglDeviceUnit", "42",
        "-startVisible",
        "-resizableWindow",
        "-off"
    };
    ramses_internal::CommandLineParser parser(sizeof(args) / sizeof(ramses_internal::Char*), args);

    ramses_internal::DisplayConfig config;
    ramses_internal::RendererConfigUtils::ApplyValuesFromCommandLine(parser, config);

    const ramses_internal::Vector3& camPos = config.getCameraPosition();
    EXPECT_FLOAT_EQ(1.f, camPos.x);
    EXPECT_FLOAT_EQ(2.f, camPos.y);
    EXPECT_FLOAT_EQ(3.f, camPos.z);

    const ramses_internal::Vector3& camRot = config.getCameraRotation();
    EXPECT_FLOAT_EQ(4.f, camRot.x);
    EXPECT_FLOAT_EQ(5.f, camRot.y);
    EXPECT_FLOAT_EQ(6.f, camRot.z);

    const ramses_internal::ProjectionParams& projParams = config.getProjectionParams();
    EXPECT_EQ(ramses_internal::ECameraProjectionType_Perspective, projParams.getProjectionType());
    EXPECT_FLOAT_EQ(7.f, ramses_internal::ProjectionParams::GetPerspectiveFovY(projParams));
    EXPECT_FLOAT_EQ(8.f, projParams.nearPlane);
    EXPECT_FLOAT_EQ(9.f, projParams.farPlane);

    EXPECT_FLOAT_EQ(12.f / 13.f, ramses_internal::ProjectionParams::GetAspectRatio(projParams));

    EXPECT_EQ(10, config.getWindowPositionX());
    EXPECT_EQ(11, config.getWindowPositionY());
    EXPECT_EQ(12u, config.getDesiredWindowWidth());
    EXPECT_EQ(13u, config.getDesiredWindowHeight());

    EXPECT_TRUE(config.getFullscreenState());
    EXPECT_TRUE(config.getBorderlessState());
    EXPECT_TRUE(config.isWarpingEnabled());

    EXPECT_EQ(ramses_internal::EAntiAliasingMethod_MultiSampling, config.getAntialiasingMethod());
    EXPECT_EQ(4u, config.getAntialiasingSampleCount());
    EXPECT_EQ(101u, config.getWaylandIviLayerID().getValue());
    EXPECT_EQ(1001u, config.getWaylandIviSurfaceID().getValue());
    EXPECT_EQ(42u, config.getIntegrityRGLDeviceUnit().getValue());
    EXPECT_TRUE(config.getStartVisibleIvi());
    EXPECT_TRUE(config.isWarpingEnabled());
    EXPECT_FALSE(config.getKeepEffectsUploaded());
    EXPECT_TRUE(config.isResizable());
    EXPECT_TRUE(config.getOffscreen());
}

TEST_F(AInternalDisplayConfig, getsOrthoParamsAssignedFromCommandLine)
{
    static const ramses_internal::Char* args[] =
    {
        "app",
        "-ortho",
        "-leftPlane", "2",
        "-rightPlane", "3",
        "-topPlane", "4",
        "-bottomPlane", "5",
    };
    ramses_internal::CommandLineParser parser(sizeof(args) / sizeof(ramses_internal::Char*), args);

    ramses_internal::DisplayConfig config;
    ramses_internal::RendererConfigUtils::ApplyValuesFromCommandLine(parser, config);

    const ramses_internal::ProjectionParams& projParams = config.getProjectionParams();
    EXPECT_EQ(ramses_internal::ECameraProjectionType_Orthographic, projParams.getProjectionType());
    EXPECT_FLOAT_EQ(2.f, projParams.leftPlane);
    EXPECT_FLOAT_EQ(3.f, projParams.rightPlane);
    EXPECT_FLOAT_EQ(4.f, projParams.topPlane);
    EXPECT_FLOAT_EQ(5.f, projParams.bottomPlane);
}

TEST_F(AInternalDisplayConfig, canBeCompared)
{
    ramses_internal::DisplayConfig config1;
    ramses_internal::DisplayConfig config2;
    EXPECT_EQ(config1, config2);

    config1.setAntialiasingSampleCount(4u);
    EXPECT_NE(config1, config2);
}

TEST_F(AInternalDisplayConfig, canBeCompared_Offscreen)
{
    ramses_internal::DisplayConfig configDefault;
    ramses_internal::DisplayConfig configOffscreen;

    ASSERT_EQ(configDefault, configOffscreen);

    configOffscreen.setOffscreen(true);
    EXPECT_NE(configDefault, configOffscreen);
}
