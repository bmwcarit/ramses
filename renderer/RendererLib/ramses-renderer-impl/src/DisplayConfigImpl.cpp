//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DisplayConfigImpl.h"
#include "RendererLib/RendererConfigUtils.h"
#include "Math3d/CameraMatrixHelper.h"

namespace ramses
{
    DisplayConfigImpl::DisplayConfigImpl(int32_t argc, char const* const* argv)
        : StatusObjectImpl()
    {
        ramses_internal::CommandLineParser parser(argc, argv);
        ramses_internal::RendererConfigUtils::ApplyValuesFromCommandLine(parser, m_internalConfig);
    }

    status_t DisplayConfigImpl::setViewPosition(float x, float y, float z)
    {
        m_internalConfig.setCameraPosition(ramses_internal::Vector3(x, y, z));
        return StatusOK;
    }

    status_t DisplayConfigImpl::getViewPosition(float& x, float& y, float& z) const
    {
        const ramses_internal::Vector3 position = m_internalConfig.getCameraPosition();
        x = position.x;
        y = position.y;
        z = position.z;
        return StatusOK;
    }

    status_t DisplayConfigImpl::setViewRotation(float x, float y, float z)
    {
        m_internalConfig.setCameraRotation(ramses_internal::Vector3(x, y, z));
        return StatusOK;
    }

    status_t DisplayConfigImpl::getViewRotation(float& x, float& y, float& z) const
    {
        const ramses_internal::Vector3 position = m_internalConfig.getCameraRotation();
        x = position.x;
        y = position.y;
        z = position.z;
        return StatusOK;
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

    status_t DisplayConfigImpl::setFullscreen(bool fullscreen)
    {
        m_internalConfig.setFullscreenState(fullscreen);
        return StatusOK;
    }

    status_t DisplayConfigImpl::setBorderless(bool borderless)
    {
        m_internalConfig.setBorderlessState(borderless);
        return StatusOK;
    }

    status_t DisplayConfigImpl::setPerspectiveProjection(float fieldOfViewY, float aspectRatio, float nearPlane, float farPlane)
    {
        if (fieldOfViewY > 0.0f && fieldOfViewY < 180.0f && aspectRatio > 0.0f && nearPlane > 0.0f && nearPlane < farPlane)
        {

            const ramses_internal::ProjectionParams projParams = ramses_internal::ProjectionParams::Perspective(fieldOfViewY, aspectRatio, nearPlane, farPlane);
            m_internalConfig.setProjectionParams(projParams);
        }
        else
        {
            return addErrorEntry("DisplayConfig::setPerspectiveProjection failed - Invalid parameters!");
        }

        return StatusOK;
    }

    status_t DisplayConfigImpl::setProjection(float leftPlane, float rightPlane, float bottomPlane, float topPlane, float nearPlane, float farPlane, bool isOrthographic)
    {
        if (leftPlane < rightPlane && bottomPlane < topPlane && nearPlane < farPlane)
        {
            const ramses_internal::ProjectionParams projParams = ramses_internal::ProjectionParams::Frustum(
                (isOrthographic ? ramses_internal::ECameraProjectionType_Orthographic : ramses_internal::ECameraProjectionType_Perspective),
                leftPlane, rightPlane, bottomPlane, topPlane, nearPlane, farPlane);
            m_internalConfig.setProjectionParams(projParams);
        }
        else
        {
            return addErrorEntry("DisplayConfig::setProjection failed - Invalid plane parameters!");
        }
        return StatusOK;
    }

    const ramses_internal::DisplayConfig& DisplayConfigImpl::getInternalDisplayConfig() const
    {
        return m_internalConfig;
    }

    status_t DisplayConfigImpl::setMultiSampling(uint32_t numSamples)
    {
        if (numSamples != 1u &&
            numSamples != 2u &&
            numSamples != 4u)
        {
            return addErrorEntry("DisplayConfig::setMultiSampling failed - currently the only valid sample count is 1, 2 or 4!");
        }

        if (numSamples > 1u)
        {
            m_internalConfig.setAntialiasingMethod(ramses_internal::EAntiAliasingMethod_MultiSampling);
        }
        else
        {
            m_internalConfig.setAntialiasingMethod(ramses_internal::EAntiAliasingMethod_PlainFramebuffer);
        }
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

    status_t DisplayConfigImpl::enableStereoDisplay()
    {
        m_internalConfig.setStereoDisplay(true);
        return StatusOK;
    }

    status_t DisplayConfigImpl::setWaylandIviLayerID(uint32_t waylandIviLayerID)
    {
        m_internalConfig.setWaylandIviLayerID(ramses_internal::WaylandIviLayerId(waylandIviLayerID));
        return StatusOK;
    }

    uint32_t DisplayConfigImpl::getWaylandIviLayerID() const
    {
        return m_internalConfig.getWaylandIviLayerID().getValue();
    }

    status_t DisplayConfigImpl::setWaylandIviSurfaceID(uint32_t waylandIviSurfaceID)
    {
        m_internalConfig.setWaylandIviSurfaceID(ramses_internal::WaylandIviSurfaceId(waylandIviSurfaceID));
        return StatusOK;
    }

    uint32_t DisplayConfigImpl::getWaylandIviSurfaceID() const
    {
        return m_internalConfig.getWaylandIviSurfaceID().getValue();
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

    status_t DisplayConfigImpl::setWindowIviVisible()
    {
        m_internalConfig.setStartVisibleIvi(true);
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

    status_t DisplayConfigImpl::setOffscreen(bool offscreenFlag)
    {
        m_internalConfig.setOffscreen(offscreenFlag);
        return StatusOK;
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

    status_t DisplayConfigImpl::validate(uint32_t indent) const
    {
        status_t status = StatusObjectImpl::validate(indent);
        indent += IndentationStep;

        const ramses_internal::ProjectionParams& projParams = m_internalConfig.getProjectionParams();
        if (projParams.nearPlane <= 0 || projParams.nearPlane >= projParams.farPlane)
        {
            addValidationMessage(EValidationSeverity_Error, indent, "near plane can not be zero or negative, greater than or equal to far plane");
            status = getValidationErrorStatus();
        }
        if (projParams.leftPlane >= projParams.rightPlane)
        {
            addValidationMessage(EValidationSeverity_Error, indent, "left plane can not be greater than or equal to right plane");
            status = getValidationErrorStatus();
        }
        if (projParams.bottomPlane >= projParams.topPlane)
        {
            addValidationMessage(EValidationSeverity_Error, indent, "bottom plane can not be greater than or equal to top plane");
            status = getValidationErrorStatus();
        }

        if (m_internalConfig.isStereoDisplay())
        {
            if (m_internalConfig.isWarpingEnabled())
            {
                addValidationMessage(EValidationSeverity_Error, indent, "warping is not supported for stereo display");
                status = getValidationErrorStatus();
            }
            if (m_internalConfig.getAntialiasingMethod() != ramses_internal::EAntiAliasingMethod_PlainFramebuffer)
            {
                addValidationMessage(EValidationSeverity_Error, indent, "anti aliasing is not supported for stereo display");
                status = getValidationErrorStatus();
            }
        }

        return status;
    }
}
