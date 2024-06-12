//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Core/Utils/LogMacros.h"
#include "impl/DisplayConfigImpl.h"
#include "impl/ValidationReportImpl.h"

namespace ramses::internal
{
    DisplayConfigImpl::DisplayConfigImpl() = default;

    bool DisplayConfigImpl::setDeviceType(EDeviceType deviceType)
    {
        m_internalConfig.setDeviceType(deviceType);
        return true;
    }

    EDeviceType DisplayConfigImpl::getDeviceType() const
    {
        return m_internalConfig.getDeviceType();
    }

    bool DisplayConfigImpl::setWindowType(EWindowType windowType)
    {
        m_internalConfig.setWindowType(windowType);
        return true;
    }

    EWindowType DisplayConfigImpl::getWindowType() const
    {
        return m_internalConfig.getWindowType();
    }

    bool DisplayConfigImpl::setWindowTitle(std::string_view windowTitle)
    {
        m_internalConfig.setWindowTitle(windowTitle);
        return true;
    }

    std::string_view DisplayConfigImpl::getWindowTitle() const
    {
        return m_internalConfig.getWindowTitle();
    }

    bool DisplayConfigImpl::setWindowRectangle(int32_t x, int32_t y, uint32_t width, uint32_t height)
    {
        if (width == 0u || height == 0u)
        {
            LOG_ERROR(CONTEXT_CLIENT, "DisplayConfig::setWindowRectangle failed - width and/or height cannot be 0!");
            return false;
        }

        m_internalConfig.setWindowPositionX(x);
        m_internalConfig.setWindowPositionY(y);
        m_internalConfig.setDesiredWindowWidth(width);
        m_internalConfig.setDesiredWindowHeight(height);

        return true;
    }

    bool DisplayConfigImpl::getWindowRectangle(int32_t& x, int32_t& y, uint32_t& width, uint32_t& height) const
    {
        x = m_internalConfig.getWindowPositionX();
        y = m_internalConfig.getWindowPositionY();
        width = m_internalConfig.getDesiredWindowWidth();
        height = m_internalConfig.getDesiredWindowHeight();
        return true;
    }

    bool DisplayConfigImpl::setFullscreen(bool fullscreen)
    {
        m_internalConfig.setFullscreenState(fullscreen);
        return true;
    }

    bool DisplayConfigImpl::isFullscreen() const
    {
        return m_internalConfig.getFullscreenState();
    }

    const ramses::internal::DisplayConfigData& DisplayConfigImpl::getInternalDisplayConfig() const
    {
        return m_internalConfig;
    }

    bool DisplayConfigImpl::setMultiSampling(uint32_t numSamples)
    {
        if (!ramses::internal::contains_c<uint32_t>({ 1u, 2u, 4u, 8u }, numSamples))
        {
            LOG_ERROR(CONTEXT_CLIENT, "DisplayConfigImpl::setMultiSampling failed - sample count must be 1, 2, 4 or 8!");
            return false;
        }

        m_internalConfig.setAntialiasingSampleCount(numSamples);

        return true;
    }

    bool DisplayConfigImpl::getMultiSamplingSamples(uint32_t& numSamples) const
    {
        const uint32_t sampleCount = m_internalConfig.getAntialiasingSampleCount();
        numSamples = sampleCount;

        return true;
    }

    bool DisplayConfigImpl::setWaylandIviLayerID(waylandIviLayerId_t waylandIviLayerID)
    {
        m_internalConfig.setWaylandIviLayerID(WaylandIviLayerId(waylandIviLayerID.getValue()));
        return true;
    }

    waylandIviLayerId_t DisplayConfigImpl::getWaylandIviLayerID() const
    {
        return waylandIviLayerId_t(m_internalConfig.getWaylandIviLayerID().getValue());
    }

    bool DisplayConfigImpl::setWaylandIviSurfaceID(waylandIviSurfaceId_t waylandIviSurfaceID)
    {
        m_internalConfig.setWaylandIviSurfaceID(WaylandIviSurfaceId(waylandIviSurfaceID.getValue()));
        return true;
    }

    waylandIviSurfaceId_t DisplayConfigImpl::getWaylandIviSurfaceID() const
    {
        return waylandIviSurfaceId_t(m_internalConfig.getWaylandIviSurfaceID().getValue());
    }

    void* DisplayConfigImpl::getAndroidNativeWindow() const
    {
        return m_internalConfig.getAndroidNativeWindow().getValue();
    }

    bool DisplayConfigImpl::setAndroidNativeWindow(void * nativeWindowPtr)
    {
        m_internalConfig.setAndroidNativeWindow(ramses::internal::AndroidNativeWindowPtr(nativeWindowPtr));
        return true;
    }

    IOSNativeWindowPtr DisplayConfigImpl::getIOSNativeWindow() const
    {
        return m_internalConfig.getIOSNativeWindow();
    }

    bool DisplayConfigImpl::setIOSNativeWindow(IOSNativeWindowPtr nativeWindowPtr)
    {
        m_internalConfig.setIOSNativeWindow(nativeWindowPtr);
        return true;
    }

    bool DisplayConfigImpl::setWindowIviVisible(bool visible)
    {
        m_internalConfig.setStartVisibleIvi(visible);
        return true;
    }

    bool DisplayConfigImpl::setResizable(bool resizable)
    {
        m_internalConfig.setResizable(resizable);
        return true;
    }

    bool DisplayConfigImpl::setGPUMemoryCacheSize(uint64_t size)
    {
        m_internalConfig.setGPUMemoryCacheSize(size);
        return true;
    }

    bool DisplayConfigImpl::setClearColor(const vec4f& color)
    {
        m_internalConfig.setClearColor(color);
        return true;
    }

    bool DisplayConfigImpl::setDepthStencilBufferType(EDepthBufferType depthBufferType)
    {
        m_internalConfig.setDepthStencilBufferType(depthBufferType);
        return true;
    }

    bool DisplayConfigImpl::setX11WindowHandle(X11WindowHandle x11WindowHandle)
    {
        m_internalConfig.setX11WindowHandle(x11WindowHandle);
        return true;
    }

    X11WindowHandle DisplayConfigImpl::getX11WindowHandle() const
    {
        return m_internalConfig.getX11WindowHandle();
    }

    bool DisplayConfigImpl::setWindowsWindowHandle(void* hwnd)
    {
        m_internalConfig.setWindowsWindowHandle(ramses::internal::WindowsWindowHandle(hwnd));
        return true;
    }

    void* DisplayConfigImpl::getWindowsWindowHandle() const
    {
        return m_internalConfig.getWindowsWindowHandle().getValue();
    }

    bool DisplayConfigImpl::setWaylandDisplay(std::string_view waylandDisplay)
    {
        m_internalConfig.setWaylandDisplay(waylandDisplay);
        return true;
    }

    std::string_view DisplayConfigImpl::getWaylandDisplay() const
    {
        return m_internalConfig.getWaylandDisplay();
    }

    bool DisplayConfigImpl::setAsyncEffectUploadEnabled(bool enabled)
    {
        m_internalConfig.setAsyncEffectUploadEnabled(enabled);
        return true;
    }

    bool DisplayConfigImpl::setWaylandEmbeddedCompositingSocketGroup(std::string_view groupname)
    {
        m_internalConfig.setWaylandEmbeddedCompositingSocketGroup(groupname);
        return true;
    }

    std::string_view DisplayConfigImpl::getWaylandSocketEmbeddedGroup() const
    {
        return m_internalConfig.getWaylandSocketEmbeddedGroup();
    }

    bool DisplayConfigImpl::setWaylandEmbeddedCompositingSocketPermissions(uint32_t permissions)
    {
        if (m_internalConfig.setWaylandEmbeddedCompositingSocketPermissions(permissions))
            return true;
        LOG_ERROR(CONTEXT_CLIENT, "DisplayConfig::setWaylandEmbeddedCompositingSocketPermissions failed");
        return false;
    }

    uint32_t DisplayConfigImpl::getWaylandSocketEmbeddedPermissions() const
    {
        return m_internalConfig.getWaylandSocketEmbeddedPermissions();
    }

    bool DisplayConfigImpl::setWaylandEmbeddedCompositingSocketName(std::string_view socketname)
    {
        m_internalConfig.setWaylandEmbeddedCompositingSocketName(socketname);
        return true;
    }

    std::string_view DisplayConfigImpl::getWaylandEmbeddedCompositingSocketName() const
    {
        return m_internalConfig.getWaylandSocketEmbedded();
    }

    bool DisplayConfigImpl::setWaylandEmbeddedCompositingSocketFD(int fd)
    {
        m_internalConfig.setWaylandEmbeddedCompositingSocketFD(fd);
        return true;
    }

    int DisplayConfigImpl::getWaylandSocketEmbeddedFD() const
    {
        return m_internalConfig.getWaylandSocketEmbeddedFD();
    }

    bool DisplayConfigImpl::setPlatformRenderNode(std::string_view renderNode)
    {
        m_internalConfig.setPlatformRenderNode(renderNode);
        return true;
    }

    std::string_view DisplayConfigImpl::getPlatformRenderNode() const
    {
        return m_internalConfig.getPlatformRenderNode();
    }

    bool DisplayConfigImpl::setSwapInterval(int32_t interval)
    {
        m_internalConfig.setSwapInterval(interval);
        return true;
    }

    int32_t  DisplayConfigImpl::getSwapInterval() const
    {
        return m_internalConfig.getSwapInterval();
    }

    bool DisplayConfigImpl::setScenePriority(sceneId_t sceneId, int32_t priority)
    {
        m_internalConfig.setScenePriority(SceneId(sceneId.getValue()), priority);
        return true;
    }

    int32_t DisplayConfigImpl::getScenePriority(sceneId_t sceneId) const
    {
        return m_internalConfig.getScenePriority(SceneId(sceneId.getValue()));
    }

    bool DisplayConfigImpl::setResourceUploadBatchSize(uint32_t batchSize)
    {
        if (batchSize == 0)
        {
            LOG_ERROR(CONTEXT_CLIENT, "DisplayConfig::setResourceUploadBatchSize failed - batchSize cannot be 0!");
            return false;
        }
        m_internalConfig.setResourceUploadBatchSize(batchSize);
        return true;
    }

    uint32_t DisplayConfigImpl::getResourceUploadBatchSize() const
    {
        return m_internalConfig.getResourceUploadBatchSize();
    }

    void DisplayConfigImpl::validate(ValidationReportImpl& report) const
    {
        const auto embeddedCompositorFilename = m_internalConfig.getWaylandSocketEmbedded();
        int embeddedCompositorFileDescriptor  = m_internalConfig.getWaylandSocketEmbeddedFD();

        if (!embeddedCompositorFilename.empty() && embeddedCompositorFileDescriptor >= 0)
        {
            report.add(EIssueType::Warning, "Competing settings for EmbeddedCompositor are set (file descriptor and file name). File descriptor setting will be preferred.", nullptr);
        }

        const auto requestedWindow = m_internalConfig.getWindowType();
        const auto requestedDevice = m_internalConfig.getDeviceType();

        const std::vector<std::pair<EWindowType, EDeviceType>> supportedDeviceWindowCombinations {{
            {EWindowType::Windows, EDeviceType::GLES_3_0} , {EWindowType::Windows, EDeviceType::GL_4_2}, {EWindowType::Windows, EDeviceType::GL_4_5}, {EWindowType::Windows, EDeviceType::Vulkan},
            {EWindowType::X11, EDeviceType::GLES_3_0}, {EWindowType::X11, EDeviceType::Vulkan},
            {EWindowType::Wayland_IVI, EDeviceType::GLES_3_0},
            {EWindowType::Wayland_Shell, EDeviceType::GLES_3_0},
            {EWindowType::Android, EDeviceType::GLES_3_0},
            {EWindowType::iOS, EDeviceType::GLES_3_0},
            }};

        const bool supported = std::any_of(supportedDeviceWindowCombinations.cbegin(), supportedDeviceWindowCombinations.cend(),
                                                    [requestedWindow, requestedDevice](const auto& e){ return e.first == requestedWindow && e.second == requestedDevice; });
        if (!supported)
            report.add(EIssueType::Error, "Selected window type does not support device type", nullptr);

        if(m_internalConfig.getWindowsWindowHandle().isValid() && m_internalConfig.getWindowType() != EWindowType::Windows)
            report.add(EIssueType::Error, "External Windows window handle is set and selected window type is not Windows", nullptr);

        if (m_internalConfig.getX11WindowHandle().isValid() && m_internalConfig.getWindowType() != EWindowType::X11)
            report.add(EIssueType::Error, "External X11 window handle is set and selected window type is not X11", nullptr);

        if (m_internalConfig.getAndroidNativeWindow().isValid() && m_internalConfig.getWindowType() != EWindowType::Android)
            report.add(EIssueType::Error, "External Android window handle is set and selected window type is not Android", nullptr);

        if (m_internalConfig.getIOSNativeWindow().isValid() && m_internalConfig.getWindowType() != EWindowType::iOS)
            report.add(EIssueType::Error, "External iOS window handle is set and selected window type is not iOS", nullptr);

        if(requestedDevice == EDeviceType::Vulkan && m_internalConfig.isAsyncEffectUploadEnabled())
            report.add(EIssueType::Error, "Vulkan does not support async shader upload", nullptr);
    }
}
