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
    EXPECT_EQ(1u, m_config.getAntialiasingSampleCount());
    EXPECT_EQ(1280u, m_config.getDesiredWindowWidth());
    EXPECT_EQ(480u, m_config.getDesiredWindowHeight());
    EXPECT_EQ(0, m_config.getWindowPositionX());
    EXPECT_EQ(0, m_config.getWindowPositionY());
    EXPECT_FALSE(m_config.isWarpingEnabled());
    EXPECT_TRUE(m_config.getKeepEffectsUploaded());
    EXPECT_TRUE(!m_config.getWaylandIviLayerID().isValid());
    EXPECT_TRUE(!m_config.getIntegrityRGLDeviceUnit().isValid());
    EXPECT_FALSE(m_config.getStartVisibleIvi());
    EXPECT_FALSE(m_config.isResizable());
    EXPECT_EQ(0u, m_config.getGPUMemoryCacheSize());
    EXPECT_EQ(ramses_internal::Vector4(0.f,0.f,0.f,1.f), m_config.getClearColor());
    EXPECT_STREQ("", m_config.getWaylandDisplay().c_str());
    EXPECT_EQ(ramses_internal::ERenderBufferType_DepthStencilBuffer, m_config.getDepthStencilBufferType());
    EXPECT_TRUE(m_config.isAsyncEffectUploadEnabled());
    EXPECT_EQ(ramses_internal::String(""), m_config.getWaylandSocketEmbedded());
    EXPECT_EQ(ramses_internal::String(""), m_config.getWaylandSocketEmbeddedGroup());
    EXPECT_EQ(-1, m_config.getWaylandSocketEmbeddedFD());
    EXPECT_EQ(ramses_internal::String(""), m_config.getPlatformRenderNode());

    // this value is used in HL API, so test that value does not change unnoticed
    EXPECT_TRUE(ramses_internal::IntegrityRGLDeviceUnit::Invalid().getValue() == 0xFFFFFFFF);
}

TEST_F(AInternalDisplayConfig, setAndGetValues)
{
    m_config.setFullscreenState(true);
    EXPECT_TRUE(m_config.getFullscreenState());

    m_config.setBorderlessState(true);
    EXPECT_TRUE(m_config.getBorderlessState());

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

    m_config.setGPUMemoryCacheSize(256u);
    EXPECT_EQ(256u, m_config.getGPUMemoryCacheSize());

    m_config.setResizable(false);
    EXPECT_FALSE(m_config.isResizable());

    const ramses_internal::Vector4 clearColor(0.1f, 0.2f, 0.3f, 0.4f);
    m_config.setClearColor(clearColor);
    EXPECT_EQ(clearColor, m_config.getClearColor());

    m_config.setDepthStencilBufferType(ramses_internal::ERenderBufferType_DepthBuffer);
    EXPECT_EQ(ramses_internal::ERenderBufferType_DepthBuffer, m_config.getDepthStencilBufferType());

    m_config.setWaylandDisplay("ramses display");
    EXPECT_STREQ("ramses display", m_config.getWaylandDisplay().c_str());

    m_config.setAsyncEffectUploadEnabled(false);
    EXPECT_FALSE(m_config.isAsyncEffectUploadEnabled());

    m_config.setWaylandEmbeddedCompositingSocketName("wayland-11");
    EXPECT_EQ(ramses_internal::String("wayland-11"), m_config.getWaylandSocketEmbedded());

    m_config.setWaylandEmbeddedCompositingSocketFD(42);
    EXPECT_EQ(42, m_config.getWaylandSocketEmbeddedFD());

    m_config.setWaylandEmbeddedCompositingSocketGroup("groupname1");
    EXPECT_EQ(ramses_internal::String("groupname1"), m_config.getWaylandSocketEmbeddedGroup());

    m_config.setWaylandEmbeddedCompositingSocketPermissions(0654);
    EXPECT_EQ(0654u, m_config.getWaylandSocketEmbeddedPermissions());

    m_config.setPlatformRenderNode("/some/render/node");
    EXPECT_EQ(ramses_internal::String("/some/render/node"), m_config.getPlatformRenderNode());
}

TEST_F(AInternalDisplayConfig, getsValuesAssignedFromCommandLine)
{
    static const ramses_internal::Char* args[] =
    {
        "app",
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

    EXPECT_EQ(10, config.getWindowPositionX());
    EXPECT_EQ(11, config.getWindowPositionY());
    EXPECT_EQ(12u, config.getDesiredWindowWidth());
    EXPECT_EQ(13u, config.getDesiredWindowHeight());

    EXPECT_TRUE(config.getFullscreenState());
    EXPECT_TRUE(config.getBorderlessState());
    EXPECT_TRUE(config.isWarpingEnabled());

    EXPECT_EQ(4u, config.getAntialiasingSampleCount());
    EXPECT_EQ(101u, config.getWaylandIviLayerID().getValue());
    EXPECT_EQ(1001u, config.getWaylandIviSurfaceID().getValue());
    EXPECT_EQ(42u, config.getIntegrityRGLDeviceUnit().getValue());
    EXPECT_TRUE(config.getStartVisibleIvi());
    EXPECT_TRUE(config.isWarpingEnabled());
    EXPECT_FALSE(config.getKeepEffectsUploaded());
    EXPECT_TRUE(config.isResizable());
}

TEST_F(AInternalDisplayConfig, willUseMSAA2IfMSAAMethodSpecifiedWithoutSamples)
{
    static const char* args[] =
    {
        "app",
        "-aa", "MSAA",
    };
    ramses_internal::CommandLineParser parser(sizeof(args) / sizeof(char*), args);

    ramses_internal::DisplayConfig config;
    ramses_internal::RendererConfigUtils::ApplyValuesFromCommandLine(parser, config);

    EXPECT_EQ(2u, config.getAntialiasingSampleCount());
}

TEST_F(AInternalDisplayConfig, canBeCompared)
{
    ramses_internal::DisplayConfig config1;
    ramses_internal::DisplayConfig config2;
    EXPECT_EQ(config1, config2);

    config1.setAntialiasingSampleCount(4u);
    EXPECT_NE(config1, config2);
}
