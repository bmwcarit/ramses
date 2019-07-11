//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Window_Integrity_RGL/Window_Integrity_RGL.h"
#include "RendererLib/DisplayConfig.h"
#include "Utils/LogMacros.h"
#include <EGL/eglext.h>

namespace ramses_internal
{
    Window_Integrity_RGL::Window_Integrity_RGL(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler)
        : Window_Base(displayConfig, windowEventHandler, 0u)
        , m_deviceUnit(displayConfig.getIntegrityRGLDeviceUnit().getValue())
        , m_backgroundColor(displayConfig.getClearColor())
    {
    }

    Window_Integrity_RGL::~Window_Integrity_RGL()
    {
        LOG_INFO(CONTEXT_RENDERER, "Window_Integrity_RGL::~Window_Integrity_RGL(): Destroying Integrity RGL window");

        if(InvalidIntegrityRGLDeviceUnit.getValue() == m_deviceUnit)
        {
            LOG_WARN(CONTEXT_RENDERER, "Window_Integrity_RGL::~Window_Integrity_RGL(): nothing to do because device unit was not set");
            return;
        }

        const auto windowDisableErrorStatus = R_WM_WindowDisable(m_deviceUnit, &m_rglWindow);
        if(R_WM_ERR_OK != windowDisableErrorStatus)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Window_Integrity_RGL::init(): R_WM_WindowDisable failed with error status : " << windowDisableErrorStatus);
        }

        const auto windowDestroyErrorStatus = R_WM_WindowDelete(m_deviceUnit, &m_rglWindow);
        if(R_WM_ERR_OK != windowDestroyErrorStatus)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Window_Integrity_RGL::init(): R_WM_WindowDelete failed with error status : " << windowDestroyErrorStatus);
        }

        const auto screenDisableErrorStatus = R_WM_ScreenDisable(m_deviceUnit);
        if(R_WM_ERR_OK != screenDisableErrorStatus)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Window_Integrity_RGL::init(): R_WM_ScreenDisable failed with error status : " << screenDisableErrorStatus);
        }

        const auto deviceDeinitErrorStatus = R_WM_DevDeinit(m_deviceUnit);
        if(R_WM_ERR_OK != deviceDeinitErrorStatus)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Window_Integrity_RGL::~Window_Integrity_RGL(): R_WM_DevDeinit failed with error status :" << deviceDeinitErrorStatus);
        }

        LOG_INFO(CONTEXT_RENDERER, "Window_Integrity_RGL::~Window_Integrity_RGL(): Integrity RGL window destroyed");
    }

    Bool Window_Integrity_RGL::init()
    {
        LOG_INFO(CONTEXT_RENDERER, "Window_Integrity_RGL::init(): Constructing Window Integrity RGL");

        if(InvalidIntegrityRGLDeviceUnit.getValue() == m_deviceUnit)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Window_Integrity_RGL::init(): RGL device unit is not set in display config!");
            return false;
        }

        const auto deviceInitErrorStatus = R_WM_DevInit(m_deviceUnit);
        if(R_WM_ERR_OK != deviceInitErrorStatus)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Window_Integrity_RGL::init(): R_WM_DevInit failed with error status :" << deviceInitErrorStatus);
            return false;
        }

        const auto displaySetBackgroundColorErrorStatus = R_WM_ScreenBgColorSet(m_deviceUnit, static_cast<UInt8>(255 * m_backgroundColor.x), static_cast<UInt8>(255 * m_backgroundColor.y), static_cast<UInt8>(255 * m_backgroundColor.z));
        if( R_WM_ERR_OK != displaySetBackgroundColorErrorStatus)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Window_Integrity_RGL::init(): R_WM_ScreenBgColorSet failed with error status :" << displaySetBackgroundColorErrorStatus);
            return false;
        }

        const auto screenEnableErrorStatus = R_WM_ScreenEnable(m_deviceUnit);
        if(R_WM_ERR_OK != screenEnableErrorStatus)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Window_Integrity_RGL::init(): R_WM_ScreenEnable failed with error status : " << screenEnableErrorStatus);
            return false;
        }

        m_rglWindow.ColorFmt         = m_colorFormat;
        m_rglWindow.PosX             = getPosX();
        m_rglWindow.PosY             = getPosY();
        m_rglWindow.PosZ             = m_layer;
        m_rglWindow.Pitch            = getWidth();
        m_rglWindow.Width            = getWidth();
        m_rglWindow.Height           = getHeight();
        m_rglWindow.Surface.BufNum   = m_numberOfBuffers;
        m_rglWindow.Surface.BufMode  = m_bufferAllocationMode;
        m_rglWindow.Surface.Type     = m_surfaceType;

        const auto windowCreateErrorStatus = R_WM_WindowCreate(m_deviceUnit, &m_rglWindow);
        if(R_WM_ERR_OK != windowCreateErrorStatus)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Window_Integrity_RGL::init(): R_WM_WindowCreate failed with error status : " << windowCreateErrorStatus);
            return false;
        }

        const auto deviceEventRegisterErrorStatus = R_WM_DevEventRegister(m_deviceUnit, R_WM_EVENT_VBLANK, 0);
        if(R_WM_ERR_OK != deviceEventRegisterErrorStatus)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Window_Integrity_RGL::init(): R_WM_DevEventRegister failed with error status : " << windowCreateErrorStatus);
            return false;
        }

        const auto windowEnableErrorStatus = R_WM_WindowEnable(m_deviceUnit, &m_rglWindow);
        if(R_WM_ERR_OK != windowEnableErrorStatus)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Window_Integrity_RGL::init(): R_WM_WindowEnable failed with error status : " << windowCreateErrorStatus);
            return false;
        }

        LOG_INFO(CONTEXT_RENDERER, "Window_Integrity_RGL::init(): Integrity RGL Window creation succeeded");
        return true;
    }

    const EGLNativeDisplayType Window_Integrity_RGL::getNativeDisplayHandle() const
    {
        return m_nativeDisplayHandle;
    }

    const EGLNativeWindowType Window_Integrity_RGL::getNativeWindowHandle() const
    {
        return reinterpret_cast<EGLNativeWindowType>(const_cast<r_wm_Window_t*>(&m_rglWindow));
    }

    const EGLint* Window_Integrity_RGL::getSurfaceAttributes() const
    {
        static const EGLint surfaceAttributes[] =
        {
            EGL_BUFFER_SIZE,          32,
            EGL_ALPHA_SIZE,           8,
            EGL_BLUE_SIZE,            8,
            EGL_GREEN_SIZE,           8,
            EGL_RED_SIZE,             8,
            EGL_DEPTH_SIZE,           8,
            EGL_SURFACE_TYPE,         EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE,      EGL_OPENGL_ES3_BIT_KHR,
            EGL_NONE
        };
        return surfaceAttributes;
    }

    bool Window_Integrity_RGL::hasTitle() const
    {
        return false;
    }

    Bool Window_Integrity_RGL::setFullscreen(Bool)
    {
        return true;
    }

    void Window_Integrity_RGL::handleEvents()
    {
    }
}
