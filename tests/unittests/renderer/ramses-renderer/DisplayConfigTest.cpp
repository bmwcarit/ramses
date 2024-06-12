//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/framework/ValidationReport.h"
#include "ramses/renderer/DisplayConfig.h"
#include "impl/DisplayConfigImpl.h"
#include "impl/ValidationReportImpl.h"
#include "gmock/gmock.h"

namespace ramses::internal
{
    class ADisplayConfig : public ::testing::Test
    {
    protected:
        void expectDisplayConfigValid()
        {
            ramses::ValidationReport report;
            config.validate(report);
            EXPECT_FALSE(report.hasIssue());
        }

        void expectDisplayConfigInvalid(std::string_view expectedError)
        {
            ramses::ValidationReport report;
            config.validate(report);
            EXPECT_TRUE(report.hasError());
            EXPECT_THAT(report.impl().toString(), ::testing::HasSubstr(expectedError));
        }

        ramses::DisplayConfig config;
    };

    TEST_F(ADisplayConfig, hasDefaultValuesUponConstruction)
    {
        const ramses::internal::DisplayConfigData defaultDisplayConfig;
        const auto& displayConfig = config.impl().getInternalDisplayConfig();

        EXPECT_EQ(defaultDisplayConfig.getWindowPositionX(), displayConfig.getWindowPositionX());
        EXPECT_EQ(defaultDisplayConfig.getWindowPositionY(), displayConfig.getWindowPositionY());
        EXPECT_EQ(defaultDisplayConfig.getDesiredWindowWidth(), displayConfig.getDesiredWindowWidth());
        EXPECT_EQ(defaultDisplayConfig.getDesiredWindowHeight(), displayConfig.getDesiredWindowHeight());

        EXPECT_EQ(defaultDisplayConfig.getFullscreenState(), displayConfig.getFullscreenState());

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

    TEST_F(ADisplayConfig, setsDeviceType)
    {
        EXPECT_TRUE(config.setDeviceType(ramses::EDeviceType::GL_4_2));
        EXPECT_EQ(ramses::EDeviceType::GL_4_2, config.impl().getInternalDisplayConfig().getDeviceType());
    }

    TEST_F(ADisplayConfig, setsWindowType)
    {
        EXPECT_TRUE(config.setWindowType(ramses::EWindowType::Wayland_IVI));
        EXPECT_EQ(ramses::EWindowType::Wayland_IVI, config.impl().getInternalDisplayConfig().getWindowType());
    }

    TEST_F(ADisplayConfig, setsWindowTitle)
    {
        EXPECT_TRUE(config.setWindowTitle("window title"));
        EXPECT_EQ("window title", config.impl().getInternalDisplayConfig().getWindowTitle());
    }

    TEST_F(ADisplayConfig, setsFullscreenState)
    {
        EXPECT_TRUE(config.setWindowFullscreen(true));
        EXPECT_TRUE(config.impl().getInternalDisplayConfig().getFullscreenState());
        EXPECT_TRUE(config.isWindowFullscreen());

        EXPECT_TRUE(config.setWindowFullscreen(false));
        EXPECT_FALSE(config.impl().getInternalDisplayConfig().getFullscreenState());
        EXPECT_FALSE(config.isWindowFullscreen());
    }

    TEST_F(ADisplayConfig, setsWindowRect)
    {
        EXPECT_TRUE(config.setWindowRectangle(15, 16, 123u, 345u));
        int32_t x = 0;
        int32_t y = 0;
        uint32_t width = 0;
        uint32_t height = 0;
        config.getWindowRectangle(x, y, width, height);
        EXPECT_EQ(15, x);
        EXPECT_EQ(16, y);
        EXPECT_EQ(123u, width);
        EXPECT_EQ(345u, height);
    }

    TEST_F(ADisplayConfig, failsToSetInvalidWindowRect)
    {
        EXPECT_TRUE(config.setWindowRectangle(15, 16, 1u, 2u));

        EXPECT_FALSE(config.setWindowRectangle(15, 16, 0u, 2u));
        EXPECT_FALSE(config.setWindowRectangle(15, 16, 1u, 0u));
        EXPECT_FALSE(config.setWindowRectangle(15, 16, 0u, 0u));

        EXPECT_EQ(15, config.impl().getInternalDisplayConfig().getWindowPositionX());
        EXPECT_EQ(16, config.impl().getInternalDisplayConfig().getWindowPositionY());
        EXPECT_EQ(1u, config.impl().getInternalDisplayConfig().getDesiredWindowWidth());
        EXPECT_EQ(2u, config.impl().getInternalDisplayConfig().getDesiredWindowHeight());
    }

    TEST_F(ADisplayConfig, failsToSetUnsupportedMultisampling)
    {
        EXPECT_FALSE(config.setMultiSampling(0u));
        EXPECT_FALSE(config.setMultiSampling(3u));
        EXPECT_FALSE(config.setMultiSampling(5u));
        EXPECT_FALSE(config.setMultiSampling(6u));
        EXPECT_FALSE(config.setMultiSampling(7u));
        EXPECT_FALSE(config.setMultiSampling(9u));

        EXPECT_EQ(1u, config.impl().getInternalDisplayConfig().getAntialiasingSampleCount());
    }

    TEST_F(ADisplayConfig, setsAndGetsMultisampling)
    {
        EXPECT_TRUE(config.setMultiSampling(2u));

        EXPECT_EQ(2u, config.impl().getInternalDisplayConfig().getAntialiasingSampleCount());

        uint32_t sampleCount = 0;
        config.getMultiSamplingSamples(sampleCount);
        EXPECT_EQ(2u, sampleCount);
    }

    TEST_F(ADisplayConfig, setsWindowIVIVisible)
    {
        EXPECT_TRUE(config.setWindowIviVisible());
        EXPECT_TRUE(config.impl().getInternalDisplayConfig().getStartVisibleIvi());
    }

    TEST_F(ADisplayConfig, setsAndGetsWaylandDisplay)
    {
        EXPECT_TRUE(config.setWaylandDisplay("xxx"));
        EXPECT_EQ("xxx", config.getWaylandDisplay());
        EXPECT_EQ("xxx", config.impl().getInternalDisplayConfig().getWaylandDisplay());
    }

    TEST_F(ADisplayConfig, setsAndGetsWaylandIviSurfaceId)
    {
        EXPECT_TRUE(config.setWaylandIviSurfaceID(ramses::waylandIviSurfaceId_t(25)));
        EXPECT_EQ(ramses::waylandIviSurfaceId_t(25), config.getWaylandIviSurfaceID());
        EXPECT_EQ(ramses::internal::WaylandIviSurfaceId(25), config.impl().getInternalDisplayConfig().getWaylandIviSurfaceID());
    }

    TEST_F(ADisplayConfig, setsAndGetsWaylandIviLayerId)
    {
        EXPECT_TRUE(config.setWaylandIviLayerID(ramses::waylandIviLayerId_t(36)));
        EXPECT_EQ(ramses::waylandIviLayerId_t(36), config.getWaylandIviLayerID());
        EXPECT_EQ(ramses::internal::WaylandIviLayerId(36), config.impl().getInternalDisplayConfig().getWaylandIviLayerID());
    }

    TEST_F(ADisplayConfig, setsAndGetsX11WindowHandle)
    {
        EXPECT_TRUE(config.setX11WindowHandle(ramses::X11WindowHandle(42u)));
        EXPECT_EQ(ramses::X11WindowHandle(42u), config.getX11WindowHandle());
        EXPECT_EQ(ramses::internal::X11WindowHandle(42), config.impl().getInternalDisplayConfig().getX11WindowHandle());
    }

    TEST_F(ADisplayConfig, IsValidUponConstruction)
    {
        expectDisplayConfigValid();
    }

    TEST_F(ADisplayConfig, IsValidIfWindowTypeWindowsAndDeviceTypeNotGLES30)
    {
        config.setWindowType(ramses::EWindowType::Windows);
        config.setDeviceType(ramses::EDeviceType::GL_4_5);
        expectDisplayConfigValid();
    }

    TEST_F(ADisplayConfig, IsValidIfSupportedWindowTypeUsedWithVulkan)
    {
        config.setDeviceType(ramses::EDeviceType::Vulkan);
        config.setAsyncEffectUploadEnabled(false);

        config.setWindowType(ramses::EWindowType::Windows);
        expectDisplayConfigValid();
        config.setWindowType(ramses::EWindowType::X11);
        expectDisplayConfigValid();
    }

    TEST_F(ADisplayConfig, IsNotValidIfAsyncUploadEnabledWithVulkanDevice)
    {
        config.setDeviceType(ramses::EDeviceType::Vulkan);
        config.setAsyncEffectUploadEnabled(true);

        expectDisplayConfigInvalid("Vulkan does not support async shader upload");
    }

    TEST_F(ADisplayConfig, IsValidIfWindowTypeAndroidAndDeviceTypeGLES30)
    {
        config.setWindowType(ramses::EWindowType::Android);
        config.setDeviceType(ramses::EDeviceType::GLES_3_0);
        expectDisplayConfigValid();
    }

    TEST_F(ADisplayConfig, IsValidIfWindowTypeIOSAndDeviceTypeGLES30)
    {
        config.setWindowType(ramses::EWindowType::iOS);
        config.setDeviceType(ramses::EDeviceType::GLES_3_0);
        expectDisplayConfigValid();
    }

    TEST_F(ADisplayConfig, IsValidIfWindowTypeAndroidAndExternalAndroidWindowHandleProvided)
    {
        std::uintptr_t dummyVoidPointer{ 1u };
        config.setAndroidNativeWindow(reinterpret_cast<void*>(dummyVoidPointer));
        config.setWindowType(ramses::EWindowType::Android);
        expectDisplayConfigValid();
    }

    TEST_F(ADisplayConfig, IsValidIfWindowTypeIOSAndExternalIOSWindowHandleProvided)
    {
        std::uintptr_t dummyVoidPointer{ 1u };
        config.setIOSNativeWindow(ramses::IOSNativeWindowPtr{ reinterpret_cast<void*>(dummyVoidPointer) });
        config.setWindowType(ramses::EWindowType::iOS);
        expectDisplayConfigValid();
    }

    TEST_F(ADisplayConfig, IsValidIfWindowTypeWindowsAndExternalWindowsWindowHandleProvided)
    {
        std::uintptr_t dummyVoidPointer{ 1u };
        config.setWindowsWindowHandle(reinterpret_cast<void*>(dummyVoidPointer));
        config.setWindowType(ramses::EWindowType::Windows);
        expectDisplayConfigValid();
    }

    TEST_F(ADisplayConfig, IsValidIfWindowTypeX11AndExternalX11WindowHandleProvided)
    {
        ramses::X11WindowHandle dummyX11Window{ 1u };
        config.setX11WindowHandle(dummyX11Window);
        config.setWindowType(ramses::EWindowType::X11);
        expectDisplayConfigValid();
    }

    TEST_F(ADisplayConfig, ValidationWarningIfCompositorEmbeddedCompositorDisplsayAndSocketFdAreSet)
    {
        config.setWaylandEmbeddedCompositingSocketFD(1234);
        config.setWaylandEmbeddedCompositingSocketName("abc");
        ramses::ValidationReport report;
        config.validate(report);
        EXPECT_FALSE(report.hasError());
        EXPECT_TRUE(report.hasIssue());
        EXPECT_THAT(report.impl().toString(), ::testing::HasSubstr("WARNING: Competing settings for EmbeddedCompositor are set"));
    }

    TEST_F(ADisplayConfig, ValidationErrorIfWindowTypeNotWindowsAndDeviceTypeNotGLES30)
    {
        config.setWindowType(ramses::EWindowType::Android);
        config.setDeviceType(ramses::EDeviceType::GL_4_2);
        expectDisplayConfigInvalid("ERROR: Selected window type does not support device type");
    }

    TEST_F(ADisplayConfig, ValidationErrorIfExternalHandleWindowsSetAndTypeNotWindows)
    {
        std::uintptr_t dummyVoidPointer{ 1u };
        config.setWindowsWindowHandle(reinterpret_cast<void*>(dummyVoidPointer));
        config.setWindowType(ramses::EWindowType::X11);
        expectDisplayConfigInvalid("ERROR: External Windows window handle is set and selected window type is not Windows");
    }

    TEST_F(ADisplayConfig, ValidationErrorIfExternalHandleAndroidSetAndTypeNotAndroid)
    {
        std::uintptr_t dummyVoidPointer{ 1u };
        config.setAndroidNativeWindow(reinterpret_cast<void*>(dummyVoidPointer));
        config.setWindowType(ramses::EWindowType::X11);
        expectDisplayConfigInvalid("ERROR: External Android window handle is set and selected window type is not Android");
    }

    TEST_F(ADisplayConfig, ValidationErrorIfExternalHandleIOSSetAndTypeNotIOS)
    {
        std::uintptr_t dummyVoidPointer{ 1u };
        config.setIOSNativeWindow(ramses::IOSNativeWindowPtr{ reinterpret_cast<void*>(dummyVoidPointer) });
        config.setWindowType(ramses::EWindowType::X11);
        expectDisplayConfigInvalid("ERROR: External iOS window handle is set and selected window type is not iOS");
    }

    TEST_F(ADisplayConfig, ValidationErrorIfExternalHandleX11SetAndTypeNotX11)
    {
        config.setX11WindowHandle(ramses::X11WindowHandle(123u));
        config.setWindowType(ramses::EWindowType::Android);
        expectDisplayConfigInvalid("ERROR: External X11 window handle is set and selected window type is not X11");
    }

    TEST_F(ADisplayConfig, CanBeCopyAndMoveConstructed)
    {
        config.setWindowFullscreen(true);

        ramses::DisplayConfig configCopy{ config };
        EXPECT_TRUE(configCopy.isWindowFullscreen());

        ramses::DisplayConfig configMove{ std::move(config) };
        EXPECT_TRUE(configMove.isWindowFullscreen());
    }

    TEST_F(ADisplayConfig, CanBeCopyAndMoveAssigned)
    {
        config.setWindowFullscreen(true);

        ramses::DisplayConfig configCopy;
        configCopy = config;
        EXPECT_TRUE(configCopy.isWindowFullscreen());

        ramses::DisplayConfig configMove;
        configMove = std::move(config);
        EXPECT_TRUE(configMove.isWindowFullscreen());
    }

    TEST_F(ADisplayConfig, CanBeSelfAssigned)
    {
        config.setWindowFullscreen(true);

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-move"
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif
        config = config;
        EXPECT_TRUE(config.isWindowFullscreen());
        config = std::move(config);
        // NOLINTNEXTLINE(bugprone-use-after-move)
        EXPECT_TRUE(config.isWindowFullscreen());
#ifdef __clang__
#pragma clang diagnostic pop
#endif
    }

    TEST_F(ADisplayConfig, setClearColor)
    {
        const float red   = 0.1f;
        const float green = 0.2f;
        const float blue  = 0.3f;
        const float alpha = 0.4f;
        EXPECT_TRUE(config.setClearColor({red, green, blue, alpha}));

        const glm::vec4& clearColor = config.impl().getInternalDisplayConfig().getClearColor();
        EXPECT_EQ(clearColor.r, red);
        EXPECT_EQ(clearColor.g, green);
        EXPECT_EQ(clearColor.b, blue);
        EXPECT_EQ(clearColor.a, alpha);
    }

    TEST_F(ADisplayConfig, setDepthStencilBufferType)
    {
        EXPECT_TRUE(config.setDepthStencilBufferType(ramses::EDepthBufferType::Depth));
        EXPECT_EQ(ramses::EDepthBufferType::Depth, config.impl().getInternalDisplayConfig().getDepthStencilBufferType());
    }

    TEST_F(ADisplayConfig, setAsyncEffectUploadEnabled)
    {
        EXPECT_TRUE(config.setAsyncEffectUploadEnabled(false));
        EXPECT_FALSE(config.impl().getInternalDisplayConfig().isAsyncEffectUploadEnabled());
    }

    TEST_F(ADisplayConfig, canSetEmbeddedCompositingSocketGroup)
    {
        config.setWaylandEmbeddedCompositingSocketGroup("permissionGroup");
        EXPECT_EQ("permissionGroup", config.impl().getWaylandSocketEmbeddedGroup());
    }

    TEST_F(ADisplayConfig, canSetEmbeddedCompositingSocketPermissions)
    {
        config.setWaylandEmbeddedCompositingSocketPermissions(0660);
        EXPECT_EQ(0660u, config.impl().getWaylandSocketEmbeddedPermissions());
    }

    TEST_F(ADisplayConfig, cannotSetInvalidEmbeddedCompositingSocketPermissions)
    {
        EXPECT_FALSE(config.setWaylandEmbeddedCompositingSocketPermissions(0));
    }

    TEST_F(ADisplayConfig, canSetEmbeddedCompositingSocketname)
    {
        config.setWaylandEmbeddedCompositingSocketName("wayland-x123");
        EXPECT_EQ("wayland-x123", config.getWaylandEmbeddedCompositingSocketName());
    }

    TEST_F(ADisplayConfig, canSetEmbeddedCompositingSocketFD)
    {
        config.setWaylandEmbeddedCompositingSocketFD(23);
        EXPECT_EQ(23, config.impl().getWaylandSocketEmbeddedFD());
    }

    TEST_F(ADisplayConfig, canSetPlatformRenderNode)
    {
        config.setPlatformRenderNode("abcd");
        EXPECT_EQ("abcd", config.impl().getPlatformRenderNode());
    }

    TEST_F(ADisplayConfig, canSetSwapInterval)
    {
        EXPECT_EQ(-1, config.impl().getSwapInterval());
        EXPECT_TRUE(config.setSwapInterval(0));
        EXPECT_EQ(0, config.impl().getSwapInterval());
    }

    TEST_F(ADisplayConfig, canSetScenePriority)
    {
        EXPECT_EQ(0, config.impl().getScenePriority(ramses::sceneId_t(551)));
        EXPECT_TRUE(config.setScenePriority(ramses::sceneId_t(551), 4));
        EXPECT_EQ(4, config.impl().getScenePriority(ramses::sceneId_t(551)));
    }

    TEST_F(ADisplayConfig, canSetResourceUploadBatchSize)
    {
        EXPECT_EQ(10u, config.impl().getResourceUploadBatchSize());
        EXPECT_TRUE(config.setResourceUploadBatchSize(1));
        EXPECT_EQ(1u, config.impl().getResourceUploadBatchSize());
        EXPECT_FALSE(config.setResourceUploadBatchSize(0));
        EXPECT_EQ(1u, config.impl().getResourceUploadBatchSize());
    }
}
