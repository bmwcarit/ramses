//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-renderer-api/DisplayConfig.h"

// internal
#include "DisplayConfigImpl.h"
#include "RamsesFrameworkTypesImpl.h"

namespace ramses
{
    DisplayConfig::DisplayConfig()
        : StatusObject{ std::make_unique<DisplayConfigImpl>() }
        , m_impl{ static_cast<DisplayConfigImpl&>(*StatusObject::m_impl) }
    {
    }

    DisplayConfig::~DisplayConfig() = default;

    DisplayConfig::DisplayConfig(const DisplayConfig& other)
        : StatusObject{ std::make_unique<DisplayConfigImpl>(other.m_impl) }
        , m_impl{ static_cast<DisplayConfigImpl&>(*StatusObject::m_impl) }
    {
    }

    DisplayConfig::DisplayConfig(DisplayConfig&& other) noexcept
        : StatusObject{ std::move(other.StatusObject::m_impl) }
        , m_impl{ static_cast<DisplayConfigImpl&>(*StatusObject::m_impl) }
    {
    }

    DisplayConfig& DisplayConfig::operator=(const DisplayConfig& other)
    {
        StatusObject::m_impl = std::make_unique<DisplayConfigImpl>(other.m_impl);
        m_impl = static_cast<DisplayConfigImpl&>(*StatusObject::m_impl);
        return *this;
    }

    DisplayConfig& DisplayConfig::operator=(DisplayConfig&& other) noexcept
    {
        StatusObject::m_impl = std::move(other.StatusObject::m_impl);
        m_impl = static_cast<DisplayConfigImpl&>(*StatusObject::m_impl);
        return *this;
    }

    status_t DisplayConfig::setDeviceType(EDeviceType deviceType)
    {
        return m_impl.get().setDeviceType(deviceType);
    }

    EDeviceType DisplayConfig::getDeviceType() const
    {
        return m_impl.get().getDeviceType();
    }

    status_t DisplayConfig::setWindowType(EWindowType windowType)
    {
        return m_impl.get().setWindowType(windowType);
    }

    EWindowType DisplayConfig::getWindowType() const
    {
        return m_impl.get().getWindowType();
    }

    status_t DisplayConfig::setWindowRectangle(int32_t x, int32_t y, uint32_t width, uint32_t height)
    {
        const status_t status = m_impl.get().setWindowRectangle(x, y, width, height);
        LOG_HL_RENDERER_API4(status, x, y, width, height);
        return status;
    }

    status_t DisplayConfig::getWindowRectangle(int32_t& x, int32_t& y, uint32_t& width, uint32_t& height) const
    {
        return m_impl.get().getWindowRectangle(x, y, width, height);
    }

    status_t DisplayConfig::setWindowFullscreen(bool fullscreen)
    {
        const status_t status = m_impl.get().setFullscreen(fullscreen);
        LOG_HL_RENDERER_API1(status, fullscreen);
        return status;
    }

    bool DisplayConfig::isWindowFullscreen() const
    {
        return m_impl.get().isFullscreen();
    }

    status_t DisplayConfig::setWindowBorderless(bool borderless)
    {
        const status_t status = m_impl.get().setBorderless(borderless);
        LOG_HL_RENDERER_API1(status, borderless);
        return status;
    }

    status_t DisplayConfig::setMultiSampling(uint32_t numSamples)
    {
        const status_t status = m_impl.get().setMultiSampling(numSamples);
        LOG_HL_RENDERER_API1(status, numSamples);
        return status;
    }

    status_t DisplayConfig::getMultiSamplingSamples(uint32_t& numSamples) const
    {
        return m_impl.get().getMultiSamplingSamples(numSamples);
    }

    status_t DisplayConfig::setWaylandIviLayerID(waylandIviLayerId_t waylandIviLayerID)
    {
        const status_t status = m_impl.get().setWaylandIviLayerID(waylandIviLayerID);
        LOG_HL_RENDERER_API1(status, waylandIviLayerID);
        return status;
    }

    waylandIviLayerId_t DisplayConfig::getWaylandIviLayerID() const
    {
        return m_impl.get().getWaylandIviLayerID();
    }

    status_t DisplayConfig::setWaylandIviSurfaceID(waylandIviSurfaceId_t waylandIviSurfaceID)
    {
        const status_t status = m_impl.get().setWaylandIviSurfaceID(waylandIviSurfaceID);
        LOG_HL_RENDERER_API1(status, waylandIviSurfaceID.getValue());
        return status;
    }

    waylandIviSurfaceId_t DisplayConfig::getWaylandIviSurfaceID() const
    {
        return m_impl.get().getWaylandIviSurfaceID();
    }

    status_t DisplayConfig::setWaylandDisplay(std::string_view waylandDisplay)
    {
        return m_impl.get().setWaylandDisplay(waylandDisplay);
    }

    std::string_view DisplayConfig::getWaylandDisplay() const
    {
        return m_impl.get().getWaylandDisplay();
    }

    status_t DisplayConfig::setAsyncEffectUploadEnabled(bool enabled)
    {
        const status_t status = m_impl.get().setAsyncEffectUploadEnabled(enabled);
        LOG_HL_RENDERER_API1(status, enabled);
        return status;
        }

    void* DisplayConfig::getAndroidNativeWindow() const
    {
        return m_impl.get().getAndroidNativeWindow();
    }

    status_t DisplayConfig::setAndroidNativeWindow(void* nativeWindowPtr)
    {
        return m_impl.get().setAndroidNativeWindow(nativeWindowPtr);
    }


    void* DisplayConfig::getIOSNativeWindow() const
    {
        return m_impl.get().getIOSNativeWindow();
    }

    status_t DisplayConfig::setIOSNativeWindow(void* nativeWindowPtr)
    {
        return m_impl.get().setIOSNativeWindow(nativeWindowPtr);
    }

    status_t DisplayConfig::setWindowIviVisible()
    {
        const status_t status = m_impl.get().setWindowIviVisible(true);
        LOG_HL_RENDERER_API_NOARG(status);
        return status;
    }

    status_t DisplayConfig::keepEffectsUploaded(bool enable)
    {
        const status_t status = m_impl.get().keepEffectsUploaded(enable);
        LOG_HL_RENDERER_API1(status, enable);
        return status;
    }

    status_t DisplayConfig::setGPUMemoryCacheSize(uint64_t size)
    {
        const status_t status = m_impl.get().setGPUMemoryCacheSize(size);
        LOG_HL_RENDERER_API1(status, size);
        return status;
    }

    status_t DisplayConfig::setResizable(bool resizable)
    {
        const status_t status = m_impl.get().setResizable(resizable);
        LOG_HL_RENDERER_API_NOARG(status);
        return status;
    }

    status_t DisplayConfig::setClearColor(const vec4f& color)
    {
        const status_t status = m_impl.get().setClearColor(color);
        LOG_HL_RENDERER_API4(status, color.r, color.g, color.b, color.a);
        return status;
    }

    status_t DisplayConfig::setDepthStencilBufferType(EDepthBufferType depthBufferType)
    {
        const auto status = m_impl.get().setDepthStencilBufferType(depthBufferType);
        LOG_HL_RENDERER_API1(status, depthBufferType);
        return status;
    }

    status_t DisplayConfig::setX11WindowHandle(unsigned long x11WindowHandle)
    {
        const status_t status = m_impl.get().setX11WindowHandle(x11WindowHandle);
        LOG_HL_RENDERER_API1(status, x11WindowHandle);
        return status;
    }

    unsigned long DisplayConfig::getX11WindowHandle() const
    {
        return m_impl.get().getX11WindowHandle();
    }

    status_t DisplayConfig::setWindowsWindowHandle(void* hwnd)
    {
        const status_t status = m_impl.get().setWindowsWindowHandle(hwnd);
        LOG_HL_RENDERER_API1(status, LOG_API_GENERIC_PTR_STRING(hwnd));
        return status;
    }

    void* DisplayConfig::getWindowsWindowHandle() const
    {
        return m_impl.get().getWindowsWindowHandle();
    }

    status_t DisplayConfig::setWaylandEmbeddedCompositingSocketName(std::string_view socketname)
    {
        return m_impl.get().setWaylandEmbeddedCompositingSocketName(socketname);
    }

    std::string_view DisplayConfig::getWaylandEmbeddedCompositingSocketName() const
    {
        return m_impl.get().getWaylandEmbeddedCompositingSocketName();
    }

    status_t DisplayConfig::setWaylandEmbeddedCompositingSocketGroup(std::string_view groupname)
    {
        return m_impl.get().setWaylandEmbeddedCompositingSocketGroup(groupname);
    }

    status_t DisplayConfig::setWaylandEmbeddedCompositingSocketFD(int socketFileDescriptor)
    {
        return m_impl.get().setWaylandEmbeddedCompositingSocketFD(socketFileDescriptor);
    }

    status_t DisplayConfig::setWaylandEmbeddedCompositingSocketPermissions(uint32_t permissions)
    {
        return m_impl.get().setWaylandEmbeddedCompositingSocketPermissions(permissions);
    }

    status_t DisplayConfig::setPlatformRenderNode(std::string_view renderNode)
    {
        return m_impl.get().setPlatformRenderNode(renderNode);
    }

    status_t DisplayConfig::setSwapInterval(int32_t interval)
    {
        return m_impl.get().setSwapInterval(interval);
    }

    status_t DisplayConfig::setScenePriority(sceneId_t sceneId, int32_t priority)
    {
        return m_impl.get().setScenePriority(sceneId, priority);
    }

    status_t DisplayConfig::setResourceUploadBatchSize(uint32_t batchSize)
    {
        return m_impl.get().setResourceUploadBatchSize(batchSize);
    }
}
