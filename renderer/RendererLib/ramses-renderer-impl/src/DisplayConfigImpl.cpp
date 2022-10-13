//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DisplayConfigImpl.h"
#include "RendererLib/RendererConfigUtils.h"

namespace ramses
{
    DisplayConfigImpl::DisplayConfigImpl(int32_t argc, char const* const* argv)
        : StatusObjectImpl()
    {
        ramses_internal::CommandLineParser parser(argc, argv);
        ramses_internal::RendererConfigUtils::ApplyValuesFromCommandLine(parser, m_internalConfig);
    }

    status_t DisplayConfigImpl::setWindowRectangle(int32_t x, int32_t y, uint32_t width, uint32_t height)
    {
        if (width == 0u || height == 0u)
        {
            return addErrorEntry("DisplayConfig::setWindowRectangle failed - width and/or height cannot be 0!");
        }

        m_internalConfig.setWindowPositionX(x);
        m_internalConfig.setWindowPositionY(y);
        m_internalConfig.setDesiredWindowWidth(width);
        m_internalConfig.setDesiredWindowHeight(height);

        return StatusOK;
    }

    status_t DisplayConfigImpl::getWindowRectangle(int32_t& x, int32_t& y, uint32_t& width, uint32_t& height) const
    {
        x = m_internalConfig.getWindowPositionX();
        y = m_internalConfig.getWindowPositionY();
        width = m_internalConfig.getDesiredWindowWidth();
        height = m_internalConfig.getDesiredWindowHeight();
        return StatusOK;
    }

    status_t DisplayConfigImpl::setFullscreen(bool fullscreen)
    {
        m_internalConfig.setFullscreenState(fullscreen);
        return StatusOK;
    }

    bool DisplayConfigImpl::isFullscreen() const
    {
        return m_internalConfig.getFullscreenState();
    }

    status_t DisplayConfigImpl::setBorderless(bool borderless)
    {
        m_internalConfig.setBorderlessState(borderless);
        return StatusOK;
    }

    const ramses_internal::DisplayConfig& DisplayConfigImpl::getInternalDisplayConfig() const
    {
        return m_internalConfig;
    }

    status_t DisplayConfigImpl::setMultiSampling(uint32_t numSamples)
    {
        if (!ramses_internal::contains_c<uint32_t>({ 1u, 2u, 4u, 8u }, numSamples))
            return addErrorEntry("DisplayConfigImpl::setMultiSampling failed - sample count must be 1, 2, 4 or 8!");

        m_internalConfig.setAntialiasingSampleCount(numSamples);

        return StatusOK;
    }

    status_t DisplayConfigImpl::getMultiSamplingSamples(uint32_t& numSamples) const
    {
        const uint32_t sampleCount = m_internalConfig.getAntialiasingSampleCount();
        numSamples = sampleCount;

        return StatusOK;
    }

    status_t DisplayConfigImpl::enableWarpingPostEffect()
    {
        m_internalConfig.setWarpingEnabled(true);
        return StatusOK;
    }

    status_t DisplayConfigImpl::setWaylandIviLayerID(waylandIviLayerId_t waylandIviLayerID)
    {
        m_internalConfig.setWaylandIviLayerID(ramses_internal::WaylandIviLayerId(waylandIviLayerID.getValue()));
        return StatusOK;
    }

    waylandIviLayerId_t DisplayConfigImpl::getWaylandIviLayerID() const
    {
        return waylandIviLayerId_t(m_internalConfig.getWaylandIviLayerID().getValue());
    }

    status_t DisplayConfigImpl::setWaylandIviSurfaceID(waylandIviSurfaceId_t waylandIviSurfaceID)
    {
        m_internalConfig.setWaylandIviSurfaceID(ramses_internal::WaylandIviSurfaceId(waylandIviSurfaceID.getValue()));
        return StatusOK;
    }

    waylandIviSurfaceId_t DisplayConfigImpl::getWaylandIviSurfaceID() const
    {
        return waylandIviSurfaceId_t(m_internalConfig.getWaylandIviSurfaceID().getValue());
    }

    status_t DisplayConfigImpl::setIntegrityRGLDeviceUnit(uint32_t rglDeviceUnit)
    {
        m_internalConfig.setIntegrityRGLDeviceUnit(ramses_internal::IntegrityRGLDeviceUnit(rglDeviceUnit));
        return StatusOK;
    }

    uint32_t DisplayConfigImpl::getIntegrityRGLDeviceUnit() const
    {
        return m_internalConfig.getIntegrityRGLDeviceUnit().getValue();
    }

    void* DisplayConfigImpl::getAndroidNativeWindow() const
    {
        return m_internalConfig.getAndroidNativeWindow().getValue();
    }

    status_t DisplayConfigImpl::setAndroidNativeWindow(void * nativeWindowPtr)
    {
        m_internalConfig.setAndroidNativeWindow(ramses_internal::AndroidNativeWindowPtr(nativeWindowPtr));
        return StatusOK;
    }

    status_t DisplayConfigImpl::setWindowIviVisible(bool visible)
    {
        m_internalConfig.setStartVisibleIvi(visible);
        return StatusOK;
    }

    status_t DisplayConfigImpl::setResizable(bool resizable)
    {
        m_internalConfig.setResizable(resizable);
        return StatusOK;
    }

    status_t DisplayConfigImpl::keepEffectsUploaded(bool enable)
    {
        m_internalConfig.setKeepEffectsUploaded(enable);
        return StatusOK;
    }

    status_t DisplayConfigImpl::setGPUMemoryCacheSize(uint64_t size)
    {
        m_internalConfig.setGPUMemoryCacheSize(size);
        return StatusOK;
    }

    status_t DisplayConfigImpl::setClearColor(float red, float green, float blue, float alpha)
    {
        m_internalConfig.setClearColor(ramses_internal::Vector4(red, green, blue, alpha));
        return StatusOK;
    }

    status_t DisplayConfigImpl::setDepthStencilBufferType(EDepthBufferType depthBufferType)
    {
        switch (depthBufferType)
        {
        case EDepthBufferType_None:
            m_internalConfig.setDepthStencilBufferType(ramses_internal::ERenderBufferType_InvalidBuffer);
            break;
        case EDepthBufferType_Depth:
            m_internalConfig.setDepthStencilBufferType(ramses_internal::ERenderBufferType_DepthBuffer);
            break;
        case EDepthBufferType_DepthStencil:
            m_internalConfig.setDepthStencilBufferType(ramses_internal::ERenderBufferType_DepthStencilBuffer);
            break;
        }

        return StatusOK;
    }

    status_t DisplayConfigImpl::setX11WindowHandle(unsigned long x11WindowHandle)
    {
        m_internalConfig.setX11WindowHandle(ramses_internal::X11WindowHandle(x11WindowHandle));
        return StatusOK;
    }

    unsigned long DisplayConfigImpl::getX11WindowHandle() const
    {
        return m_internalConfig.getX11WindowHandle().getValue();
    }

    status_t DisplayConfigImpl::setWindowsWindowHandle(void* hwnd)
    {
        m_internalConfig.setWindowsWindowHandle(ramses_internal::WindowsWindowHandle(hwnd));
        return StatusOK;
    }

    void* DisplayConfigImpl::getWindowsWindowHandle() const
    {
        return m_internalConfig.getWindowsWindowHandle().getValue();
    }

    status_t DisplayConfigImpl::setWaylandDisplay(const char* waylandDisplay)
    {
        m_internalConfig.setWaylandDisplay(waylandDisplay);
        return StatusOK;
    }

    const char* DisplayConfigImpl::getWaylandDisplay() const
    {
        return m_internalConfig.getWaylandDisplay().c_str();
    }

    status_t DisplayConfigImpl::setAsyncEffectUploadEnabled(bool enabled)
    {
        m_internalConfig.setAsyncEffectUploadEnabled(enabled);
        return StatusOK;
    }

    status_t DisplayConfigImpl::setWaylandEmbeddedCompositingSocketGroup(const char* groupname)
    {
        m_internalConfig.setWaylandEmbeddedCompositingSocketGroup(groupname);
        return StatusOK;
    }

    const char* DisplayConfigImpl::getWaylandSocketEmbeddedGroup() const
    {
        return m_internalConfig.getWaylandSocketEmbeddedGroup().c_str();
    }

    status_t DisplayConfigImpl::setWaylandEmbeddedCompositingSocketPermissions(uint32_t permissions)
    {
        if (m_internalConfig.setWaylandEmbeddedCompositingSocketPermissions(permissions))
            return StatusOK;
        return addErrorEntry("DisplayConfig::setWaylandEmbeddedCompositingSocketPermissions failed");
    }

    uint32_t DisplayConfigImpl::getWaylandSocketEmbeddedPermissions() const
    {
        return m_internalConfig.getWaylandSocketEmbeddedPermissions();
    }

    status_t DisplayConfigImpl::setWaylandEmbeddedCompositingSocketName(const char* socketname)
    {
        m_internalConfig.setWaylandEmbeddedCompositingSocketName(socketname);
        return StatusOK;
    }

    const char* DisplayConfigImpl::getWaylandEmbeddedCompositingSocketName() const
    {
        return m_internalConfig.getWaylandSocketEmbedded().c_str();
    }

    status_t DisplayConfigImpl::setWaylandEmbeddedCompositingSocketFD(int fd)
    {
        m_internalConfig.setWaylandEmbeddedCompositingSocketFD(fd);
        return StatusOK;
    }

    int DisplayConfigImpl::getWaylandSocketEmbeddedFD() const
    {
        return m_internalConfig.getWaylandSocketEmbeddedFD();
    }

    status_t DisplayConfigImpl::setPlatformRenderNode(const char* renderNode)
    {
        m_internalConfig.setPlatformRenderNode(renderNode);
        return StatusOK;
    }

    const char* DisplayConfigImpl::getPlatformRenderNode() const
    {
        return m_internalConfig.getPlatformRenderNode().c_str();
    }

    status_t DisplayConfigImpl::setSwapInterval(int32_t interval)
    {
        m_internalConfig.setSwapInterval(interval);
        return StatusOK;
    }

    int32_t  DisplayConfigImpl::getSwapInterval() const
    {
        return m_internalConfig.getSwapInterval();
    }

    status_t DisplayConfigImpl::validate() const
    {
        status_t status = StatusObjectImpl::validate();

        const ramses_internal::String& embeddedCompositorFilename = m_internalConfig.getWaylandSocketEmbedded();
        int embeddedCompositorFileDescriptor                      = m_internalConfig.getWaylandSocketEmbeddedFD();

        if(embeddedCompositorFilename.size() == 0u && embeddedCompositorFileDescriptor < 0)
            status = addValidationMessage(EValidationSeverity_Info, "no socket information for EmbeddedCompositor set (neither file descriptor nor file name). No embedded compositor available.");
        else if(embeddedCompositorFilename.size() > 0u && embeddedCompositorFileDescriptor >= 0)
            status = addValidationMessage(EValidationSeverity_Info, "Competing settings for EmbeddedCompositor are set (file descriptor and file name). File descriptor setting will be preferred.");

        return status;
    }
}
