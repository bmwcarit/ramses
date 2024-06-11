//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses/renderer/DisplayConfig.h"
#include "ramses/framework/ValidationReport.h"

// internal
#include "impl/DisplayConfigImpl.h"
#include "impl/RamsesFrameworkTypesImpl.h"
#include "impl/APILoggingMacros.h"

namespace ramses
{
    DisplayConfig::DisplayConfig()
        : m_impl{ std::make_unique<internal::DisplayConfigImpl>() }
    {
    }

    DisplayConfig::~DisplayConfig() = default;

    DisplayConfig::DisplayConfig(const DisplayConfig& other)
        : m_impl{ std::make_unique<internal::DisplayConfigImpl>(*other.m_impl) }
    {
    }

    DisplayConfig::DisplayConfig(DisplayConfig&& other) noexcept = default;

    DisplayConfig& DisplayConfig::operator=(const DisplayConfig& other)
    {
        m_impl = std::make_unique<internal::DisplayConfigImpl>(*other.m_impl);
        return *this;
    }

    DisplayConfig& DisplayConfig::operator=(DisplayConfig&& other) noexcept = default;

    bool DisplayConfig::setDeviceType(EDeviceType deviceType)
    {
        return m_impl->setDeviceType(deviceType);
    }

    EDeviceType DisplayConfig::getDeviceType() const
    {
        return m_impl->getDeviceType();
    }

    bool DisplayConfig::setWindowType(EWindowType windowType)
    {
        return m_impl->setWindowType(windowType);
    }

    EWindowType DisplayConfig::getWindowType() const
    {
        return m_impl->getWindowType();
    }

    bool DisplayConfig::setWindowTitle(std::string_view title)
    {
        return m_impl->setWindowTitle(title);
    }

    bool DisplayConfig::setWindowRectangle(int32_t x, int32_t y, uint32_t width, uint32_t height)
    {
        const auto status = m_impl->setWindowRectangle(x, y, width, height);
        LOG_HL_RENDERER_API4(status, x, y, width, height);
        return status;
    }

    bool DisplayConfig::getWindowRectangle(int32_t& x, int32_t& y, uint32_t& width, uint32_t& height) const
    {
        return m_impl->getWindowRectangle(x, y, width, height);
    }

    bool DisplayConfig::setWindowFullscreen(bool fullscreen)
    {
        const auto status = m_impl->setFullscreen(fullscreen);
        LOG_HL_RENDERER_API1(status, fullscreen);
        return status;
    }

    bool DisplayConfig::isWindowFullscreen() const
    {
        return m_impl->isFullscreen();
    }

    bool DisplayConfig::setMultiSampling(uint32_t numSamples)
    {
        const auto status = m_impl->setMultiSampling(numSamples);
        LOG_HL_RENDERER_API1(status, numSamples);
        return status;
    }

    bool DisplayConfig::getMultiSamplingSamples(uint32_t& numSamples) const
    {
        return m_impl->getMultiSamplingSamples(numSamples);
    }

    bool DisplayConfig::setWaylandIviLayerID(waylandIviLayerId_t waylandIviLayerID)
    {
        const auto status = m_impl->setWaylandIviLayerID(waylandIviLayerID);
        LOG_HL_RENDERER_API1(status, waylandIviLayerID);
        return status;
    }

    waylandIviLayerId_t DisplayConfig::getWaylandIviLayerID() const
    {
        return m_impl->getWaylandIviLayerID();
    }

    bool DisplayConfig::setWaylandIviSurfaceID(waylandIviSurfaceId_t waylandIviSurfaceID)
    {
        const auto status = m_impl->setWaylandIviSurfaceID(waylandIviSurfaceID);
        LOG_HL_RENDERER_API1(status, waylandIviSurfaceID.getValue());
        return status;
    }

    waylandIviSurfaceId_t DisplayConfig::getWaylandIviSurfaceID() const
    {
        return m_impl->getWaylandIviSurfaceID();
    }

    bool DisplayConfig::setWaylandDisplay(std::string_view waylandDisplay)
    {
        return m_impl->setWaylandDisplay(waylandDisplay);
    }

    std::string_view DisplayConfig::getWaylandDisplay() const
    {
        return m_impl->getWaylandDisplay();
    }

    bool DisplayConfig::setAsyncEffectUploadEnabled(bool enabled)
    {
        const auto status = m_impl->setAsyncEffectUploadEnabled(enabled);
        LOG_HL_RENDERER_API1(status, enabled);
        return status;
    }

    void* DisplayConfig::getAndroidNativeWindow() const
    {
        return m_impl->getAndroidNativeWindow();
    }

    bool DisplayConfig::setAndroidNativeWindow(void* nativeWindowPtr)
    {
        return m_impl->setAndroidNativeWindow(nativeWindowPtr);
    }

    IOSNativeWindowPtr DisplayConfig::getIOSNativeWindow() const
    {
        return m_impl->getIOSNativeWindow();
    }

    bool DisplayConfig::setIOSNativeWindow(IOSNativeWindowPtr nativeWindowPtr)
    {
        return m_impl->setIOSNativeWindow(nativeWindowPtr);
    }

    bool DisplayConfig::setWindowIviVisible()
    {
        const auto status = m_impl->setWindowIviVisible(true);
        LOG_HL_RENDERER_API_NOARG(status);
        return status;
    }

    bool DisplayConfig::setGPUMemoryCacheSize(uint64_t size)
    {
        const auto status = m_impl->setGPUMemoryCacheSize(size);
        LOG_HL_RENDERER_API1(status, size);
        return status;
    }

    bool DisplayConfig::setResizable(bool resizable)
    {
        const auto status = m_impl->setResizable(resizable);
        LOG_HL_RENDERER_API_NOARG(status);
        return status;
    }

    bool DisplayConfig::setClearColor(const vec4f& color)
    {
        const auto status = m_impl->setClearColor(color);
        LOG_HL_RENDERER_API4(status, color.r, color.g, color.b, color.a);
        return status;
    }

    bool DisplayConfig::setDepthStencilBufferType(EDepthBufferType depthBufferType)
    {
        const auto status = m_impl->setDepthStencilBufferType(depthBufferType);
        LOG_HL_RENDERER_API1(status, depthBufferType);
        return status;
    }

    bool DisplayConfig::setX11WindowHandle(X11WindowHandle x11WindowHandle)
    {
        const auto status = m_impl->setX11WindowHandle(x11WindowHandle);
        LOG_HL_RENDERER_API1(status, x11WindowHandle.getValue());
        return status;
    }

    X11WindowHandle DisplayConfig::getX11WindowHandle() const
    {
        return m_impl->getX11WindowHandle();
    }

    bool DisplayConfig::setWindowsWindowHandle(void* hwnd)
    {
        const auto status = m_impl->setWindowsWindowHandle(hwnd);
        LOG_HL_RENDERER_API1(status, LOG_API_GENERIC_PTR_STRING(hwnd));
        return status;
    }

    void* DisplayConfig::getWindowsWindowHandle() const
    {
        return m_impl->getWindowsWindowHandle();
    }

    bool DisplayConfig::setWaylandEmbeddedCompositingSocketName(std::string_view socketname)
    {
        return m_impl->setWaylandEmbeddedCompositingSocketName(socketname);
    }

    std::string_view DisplayConfig::getWaylandEmbeddedCompositingSocketName() const
    {
        return m_impl->getWaylandEmbeddedCompositingSocketName();
    }

    bool DisplayConfig::setWaylandEmbeddedCompositingSocketGroup(std::string_view groupname)
    {
        return m_impl->setWaylandEmbeddedCompositingSocketGroup(groupname);
    }

    bool DisplayConfig::setWaylandEmbeddedCompositingSocketFD(int socketFileDescriptor)
    {
        return m_impl->setWaylandEmbeddedCompositingSocketFD(socketFileDescriptor);
    }

    bool DisplayConfig::setWaylandEmbeddedCompositingSocketPermissions(uint32_t permissions)
    {
        return m_impl->setWaylandEmbeddedCompositingSocketPermissions(permissions);
    }

    bool DisplayConfig::setPlatformRenderNode(std::string_view renderNode)
    {
        return m_impl->setPlatformRenderNode(renderNode);
    }

    bool DisplayConfig::setSwapInterval(int32_t interval)
    {
        return m_impl->setSwapInterval(interval);
    }

    bool DisplayConfig::setScenePriority(sceneId_t sceneId, int32_t priority)
    {
        return m_impl->setScenePriority(sceneId, priority);
    }

    bool DisplayConfig::setResourceUploadBatchSize(uint32_t batchSize)
    {
        return m_impl->setResourceUploadBatchSize(batchSize);
    }

    void DisplayConfig::validate(ValidationReport& report) const
    {
        m_impl->validate(report.impl());
    }

    internal::DisplayConfigImpl& DisplayConfig::impl()
    {
        return *m_impl;
    }

    const internal::DisplayConfigImpl& DisplayConfig::impl() const
    {
        return *m_impl;
    }
}
